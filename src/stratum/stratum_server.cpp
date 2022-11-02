#include "stratum_server.hpp"

StratumServer::StratumServer(const CoinConfig &conf)
    : Server<StratumClient>(conf.stratum_port),
      coin_config(conf),
      redis_manager("127.0.0.1", &conf),
      diff_manager(&clients, &clients_mutex, coin_config.target_shares_rate),
      round_manager(redis_manager, "pow"),
      stats_manager(&redis_manager, &diff_manager, &round_manager,
                    &conf.stats),
      daemon_manager(coin_config.rpcs),
      payment_manager(&redis_manager, &daemon_manager, coin_config.pool_addr,
                      coin_config.payment_interval_seconds,
                      coin_config.min_payout_threshold),
      job_manager(&daemon_manager, &payment_manager, coin_config.pool_addr),
      block_submitter(&redis_manager, &daemon_manager,
                      &round_manager)
{
    auto error = httpParser.allocate(HTTP_REQ_ALLOCATE, MAX_HTTP_JSON_DEPTH);

    if (error != simdjson::SUCCESS)
    {
        logger.Log<LogType::Critical>( 
                    "Failed to allocate http parser buffer: {}",
                    simdjson::error_message(error));
        exit(EXIT_FAILURE);
    }

    // init hash functions if needed
    HashWrapper::InitSHA256();
#if POW_ALGO == POW_ALGO_VERUSHASH
    HashWrapper::InitVerusHash();
#endif

    // redis_manager.UpdatePoS(0, GetCurrentTimeMs());

    stats_thread =
        std::jthread(std::bind_front(&StatsManager::Start, &stats_manager));
    stats_thread.detach();

    control_server.Start(coin_config.control_port);
    control_thread = std::jthread(
        std::bind_front(&StratumServer::HandleControlCommands, this));
    control_thread.detach();
}

StratumServer::~StratumServer()
{
    Stop();
    logger.Log<LogType::Info>( "Stratum destroyed.");
}

void StratumServer::HandleControlCommands(std::stop_token st)
{
    char buff[256] = {0};
    while (!st.stop_requested())
    {
        ControlCommands cmd = control_server.GetNextCommand(buff, sizeof(buff));
        HandleControlCommand(cmd, buff);
        logger.Log<LogType::Info>( 
                    "Processed control command: {}", buff);
    }
}

void StratumServer::HandleControlCommand(ControlCommands cmd, char buff[])
{
    switch (cmd)
    {
        case ControlCommands::BLOCK_NOTIFY:
            HandleBlockNotify();
            break;
        // case ControlCommands::WALLET_NOTFIY:
        // {
        //     // format: %b%s%w (block hash, txid, wallet address)
        //     WalletNotify *wallet_notify =
        //         reinterpret_cast<WalletNotify *>(buff + 2);
        //     // HandleWalletNotify(wallet_notify);
        //     break;
        // }
        default:
            logger.Log<LogType::Warn>(
                        "Unknown control command {} received.", (int)cmd);
            break;
    }
}

void StratumServer::Listen()
{
    // const auto worker_amount = 2;
    const auto worker_amount = std::thread::hardware_concurrency();
    processing_threads.reserve(worker_amount);

    for (auto i = 0; i < worker_amount; i++)
    {
        processing_threads.emplace_back(std::bind_front(
            &StratumServer::ServiceSockets, (StratumServer *)this));
    }

    HandleBlockNotify();

    for (auto &t : processing_threads)
    {
        t.join();
    }
    stats_thread.join();
    control_thread.join();
}

void StratumServer::Stop()
{
    stats_thread.request_stop();
    control_thread.request_stop();
    for (auto &t : processing_threads)
    {
        t.request_stop();
    }
}

void StratumServer::ServiceSockets(std::stop_token st)
{
    logger.Log<LogType::Info>(
                "Starting servicing sockets on thread {}", gettid());

    while (!st.stop_requested())
    {
        Service();
    }
}

// TODO: test buffer too little
// TODO: test buffer flooded

void StratumServer::HandleBlockNotify()
{
    int64_t curtime_ms = GetCurrentTimeMs();

    const job_t *new_job = job_manager.GetNewJob();

    std::stop_token st = control_thread.get_stop_token();

    while (new_job == nullptr)
    {
        if (st.stop_requested())
        {
            return;
        }
        new_job = job_manager.GetNewJob();

        logger.Log<
            LogType::Critical>(
            "Block update error: Failed to generate new job! retrying...");

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    // save it to process round
    {
        std::shared_lock clients_read_lock(clients_mutex);
        for (const auto &[conn, _] : clients)
        {
            auto cli = conn->ptr.get();
            if (cli->GetIsAuthorized())
            {
                if (cli->GetIsPendingDiff())
                {
                    cli->ActivatePendingDiff();
                    UpdateDifficulty(conn.get());
                }

                BroadcastJob(conn.get(), new_job);
            }
        }

        // after we broadcasted new job:
        // > kick clients with below min difficulty
        // > update the map according to difficulties (notify best miners first)
        // > reset duplicate shares map

        for (auto it = clients.begin(); it != clients.end();)
        {
            const auto cli = (*it).first->ptr;
            const auto cli_diff = cli->GetDifficulty();
            if (cli_diff < coin_config.minimum_difficulty)
            {
                DisconnectClient(it->first);
                it++;
            }
            else
            {
                ++it;
            }

            cli->ResetShareSet();
            // update the difficulty in the map
            it->second = cli_diff;
        }
        // TODO: reset shares
    }

    // payment_manager.UpdatePayouts(&round_manager, curtime_ms);

    // the estimated share amount is supposed to be meet at block time
    const double net_est_hr = new_job->expected_hashes / BLOCK_TIME;
    round_manager.netwrok_hr = net_est_hr;
    round_manager.difficulty = new_job->target_diff;

    // submission_manager.CheckImmatureSubmissions();

    redis_manager.SetNewBlockStats(chain, curtime_ms, net_est_hr,
                                   new_job->target_diff);

    logger.Log<LogType::Info>( 
                "Broadcasted new job: \n"
                "┌{0:─^{8}}┐\n"
                "│{1: ^{8}}│\n"
                "│{2: <{8}}│\n"
                "│{3: <{8}}│\n"
                "│{4: <{8}}│\n"
                "│{5: <{8}}│\n"
                "│{6: <{8}}│\n"
                "│{7: <{8}}│\n"
                // "│{8: <{9}}│\n"
                // "└{0:─^{9}}┘\n",
                "└{0:─^{8}}┘\n",
                "", fmt::format("Job #{}", new_job->GetId()),
                fmt::format("Height: {}", new_job->height),
                // fmt::format("Min time: {}", new_job->min_time),
                fmt::format("Difficulty: {}", new_job->target_diff),
                fmt::format("Est. shares: {}", new_job->expected_hashes),
                fmt::format("Block reward: {}", new_job->block_reward),
                fmt::format("Transaction count: {}", new_job->tx_count),
                fmt::format("Block size: {}", new_job->block_size * 2), 40);
}

// use wallletnotify with "%b %s %w" arg (block hash, txid, wallet address),
// check if block hash is smaller than current job's difficulty to check whether
// its pos block.
// void StratumServer::HandleWalletNotify(WalletNotify *wal_notify)
// {
//     using namespace simdjson;
//     std::string_view block_hash(wal_notify->block_hash,
//                                 sizeof(wal_notify->block_hash));
//     std::string_view tx_id(wal_notify->txid, sizeof(wal_notify->txid));
//     std::string_view address(wal_notify->wallet_address,
//                              sizeof(wal_notify->wallet_address));

//     auto bhash256 = UintToArith256(uint256S(block_hash.data()));
//     double bhashDiff = BitsToDiff(bhash256.GetCompact());

//     if (job_manager.GetLastJob() == nullptr) return;

//     double pow_diff = job_manager.GetLastJob()->target_diff;

//     logger.Log<LogType::Info>(
//                 "Received PoS TxId: {}, block hash: {}", tx_id, block_hash);

//     if (bhashDiff >= pow_diff)
//     {
//         return;  // pow block
//     }

//     if (address != coin_config.pool_addr)
//     {
//         logger.Log<LogType::Error>(
//                     "CheckAcceptedBlock error: Wrong address: {}, block: {}",
//                     address, block_hash);
//         return;
//     }

//     // now make sure we actually staked the PoS block and not just received a tx
//     // inside one.

//     std::string resBody;
//     int64_t reward_value;

//     int resCode =
//         daemon_manager.SendRpcReq(resBody, 1, "getrawtransaction",
//                                   DaemonRpc::GetParamsStr(tx_id,
//                                                           1));  // verboose

//     if (resCode != 200)
//     {
//         logger.Log<LogType::Error>(
//                     "CheckAcceptedBlock error: Failed to getrawtransaction, "
//                     "http code: %d",
//                     resCode);
//         return;
//     }

//     try
//     {
//         ondemand::document doc = httpParser.iterate(
//             resBody.data(), resBody.size(), resBody.capacity());

//         ondemand::object res = doc["result"].get_object();

//         ondemand::array vout = res["vout"].get_array();
//         auto output1 = (*vout.begin());
//         reward_value = output1["valueSat"].get_int64();

//         auto addresses = output1["scriptPubKey"]["addresses"];
//         address = (*addresses.begin()).get_string();
//     }
//     catch (const simdjson_error &err)
//     {
//         logger.Log<LogType::Error>(
//                     "HandleWalletNotify: Failed to parse json, error: {}",
//                     err.what());
//         return;
//     }

//     // double check
//     if (address != coin_config.pool_addr)
//     {
//         logger.Log<LogType::Error>(
//                     "CheckAcceptedBlock error: Wrong address: {}, block: {}",
//                     address, block_hash);
//     }

//     BlockRes block_res;
//     bool getblock_res =
//         daemon_manager.GetBlock(block_res, httpParser, block_hash);

//     if (!getblock_res)
//     {
//         logger.Log<LogType::Error>( "Failed to getblock {}!",
//                     block_hash);
//         return;
//     }

//     if (block_res.validation_type != ValidationType::STAKE)
//     {
//         logger.Log<LogType::Critical>( 
//                     "Double PoS block check failed! block hash: {}",
//                     block_hash);
//         return;
//     }
//     if (!block_res.tx_ids.size() || block_res.tx_ids[0] != tx_id)
//     {
//         logger.Log<LogType::Critical>( 
//                     "TxId is not coinbase, block hash: {}", block_hash);
//         return;
//     }
//     // we have verified:
//     //  block is PoS (twice),
//     // the txid is ours (got its value),
//     // the txid is indeed the coinbase tx

//     Round round = round_manager_pos.GetChainRound();
//     const auto now_ms = GetCurrentTimeMs();
//     const uint64_t duration_ms = now_ms - round.round_start_ms;
//     const double effort_percent =
//         round.total_effort / round.estimated_effort * 100.f;

//     uint8_t block_hash_bin[HASH_SIZE];
//     uint8_t tx_id_bin[HASH_SIZE];

//     Unhexlify(block_hash_bin, block_hash.data(), block_hash.size());

//     Unhexlify(tx_id_bin, tx_id.data(), tx_id.size());

//     auto submission = std::make_unique<ExtendedSubmission>(
//         std::string_view(chain), std::string_view(coin_config.pool_addr),
//         BlockType::POS, block_res.height, reward_value, duration_ms, now_ms,
//         SubmissionManager::block_number, 0.d, 0.d, block_hash_bin, tx_id_bin);

//     submission_manager.AddImmatureBlock(std::move(submission),
//                                         coin_config.pos_fee);

//     logger.Log<LogType::Info>(
//                 "Added immature PoS Block! hash: {}", block_hash);
// }

RpcResult StratumServer::HandleShare(StratumClient *cli, WorkerContext *wc,
                                     share_t &share)
{
    int64_t time = GetCurrentTimeMs();
    // Benchmark<std::chrono::microseconds> share_bench("Share process");
    auto start = TIME_NOW();

    ShareResult share_res;
    RpcResult rpc_res(ResCode::OK);
    // const job_t *job = job_manager.GetJob(share.jobId);
    const job_t *job = job_manager.GetLastJob();
    // to make sure the job isn't removed while we are using it,
    // and at the same time allow multiple threads to use same job
    std::shared_lock<std::shared_mutex> job_read_lock;

    if (job == nullptr)
    {
        share_res.code = ResCode::JOB_NOT_FOUND;
        share_res.message = "Job not found";
        share_res.difficulty = static_cast<double>(BadDiff::STALE_SHARE_DIFF);
    }
    else
    {
        job_read_lock = std::shared_lock<std::shared_mutex>(job->job_mutex);
        ShareProcessor::Process(share_res, cli, wc, job, share, time);
        // share_res.code = ResCode::VALID_BLOCK;
    }

    if (unlikely_cond(share_res.code == ResCode::VALID_BLOCK))
    {
        // > add share stats before submission to have accurate effort (its
        // fast)
        round_manager.AddRoundShare(cli->GetAddress(),
                                        share_res.difficulty);

        std::size_t blockSize = job->block_size * 2;
        std::string blockData;
        blockData.reserve(blockSize);

#ifdef STRATUM_PROTOCOL_ZEC
        job->GetBlockHex(blockData, wc->block_header);
#elif defined(STRATUM_PROTOCOL_BTC)
        job->GetBlockHex(blockData, wc->block_header, cli->extra_nonce_sv,
                         share.extranonce2);
#elif defined(STRATUM_PROTOCOL_CN)
        // TODO: find way to upgrade to exclusive lock
        //  std::unique_lock<std::shared_mutex> job_write(job->job_mutex);
        job_t *job_mutable = const_cast<job_t *>(job);
        job_mutable->GetBlockHex(blockData, wc->nonce);
#else

#endif

#ifndef STRATUM_PROTOCOL_CN
        if (job->is_payment)
        {
            payment_manager.finished_payment.reset(
                payment_manager.pending_payment.release());
        }
#endif

        // submit ASAP
        auto block_hex = std::string_view(blockData.data(), blockSize);
        block_submitter.TrySubmit(chain, block_hex, httpParser);

        logger.Log<LogType::Info>( "Block hex: {}",
                    std::string_view(blockData.data(), blockSize));

        // job will remain safe thanks to the lock.
        const std::string_view worker_full(cli->GetFullWorkerName());
        const auto chain_round = round_manager.GetChainRound();
        const uint64_t duration_ms = time - chain_round.round_start_ms;
        const BlockType type = BlockType::POW;
#ifndef STRATUM_PROTOCOL_CN
        if (job->is_payment)
        {
            type = BlockType::POW_PAYMENT;
        }
#else

#endif
        const double dur = time - chain_round.round_start_ms;
        const double effort_percent =
            ((chain_round.total_effort - share_res.difficulty) /
             job->target_diff) *
            100.f;

#if HASH_ALGO == HASH_ALGO_X25X
        HashWrapper::X22I(share_res.hash_bytes.data(), wc->block_header);
        // std::reverse(share_res.hash_bytes.begin(),
        //              share_res.hash_bytes.end());
#endif

        auto submission = std::make_unique<ExtendedSubmission>(
            chain, worker_full, type, job->height, job->block_reward,
            duration_ms, time, redis_manager.GetBlockNumber(),
            share_res.difficulty, effort_percent, share_res.hash_bytes.data(),
            job->coinbase_tx_id);

        block_submitter.AddImmatureBlock(std::move(submission),
                                            coin_config.pow_fee);
    }
    else if (likely_cond(share_res.code == ResCode::VALID_SHARE))
    {
        round_manager.AddRoundShare(cli->GetAddress(),
                                        share_res.difficulty);
    }
    else
    {
        // logger.Log<LogType::Warn>(
        //             "Received bad share for job id: {}", share.jobId);

        rpc_res = RpcResult(share_res.code, share_res.message);
    }

    stats_manager.AddShare(cli->GetFullWorkerName(), cli->GetAddress(),
                           share_res.difficulty);

    auto end = TIME_NOW();

    // logger.Log<
    //     LogType::Debug>( 
    //     "Share processed in {}us, diff: {}, res: {}",
    //     std::chrono::duration_cast<std::chrono::microseconds>(end - start)
    //         .count(),
    //     share_res.difficulty, (int)share_res.code);

    return rpc_res;
}

// the shared pointer makes sure the client won't be freed as long as we are
// processing it

void StratumServer::HandleConsumeable(connection_it *it)
{
    static thread_local WorkerContext wc;

    std::shared_ptr<Connection<StratumClient>> conn = *(*it);
    const auto sockfd = conn->sock;

    // bigger than 0
    size_t req_len = 0;
    size_t next_req_len = 0;
    const char *last_req_end = nullptr;
    const char *req_start = nullptr;
    char *req_end = nullptr;
    char *buffer = (char *)conn->req_buff;

    req_end = std::strchr(buffer, '\n');

    // there can be multiple messages in 1 recv
    // {1}\n{2}\n
    // strchr(buffer, '{');  // "should" be first char
    req_start = &buffer[0];
    while (req_end)
    {
        req_len = req_end - req_start;
        HandleReq(conn.get(), &wc, std::string_view(req_start, req_len));

        last_req_end = req_end;
        req_start = req_end + 1;
        req_end = std::strchr(req_end + 1, '\n');
    }

    // if we haven't received a full request then don't touch the buffer
    if (last_req_end)
    {
        next_req_len =
            conn->req_pos - (last_req_end - buffer + 1);  // don't inlucde \n

        std::memmove(buffer, last_req_end + 1, next_req_len);
        buffer[next_req_len] = '\0';

        conn->req_pos = next_req_len;
        last_req_end = nullptr;
    }
}
void StratumServer::HandleConnected(connection_it *it)
{
    std::shared_ptr<Connection<StratumClient>> conn = *(*it);
    conn->ptr = std::make_shared<StratumClient>(GetCurrentTimeMs(),
                                                coin_config.default_difficulty);
    std::unique_lock lock(clients_mutex);
    clients.emplace(conn, 0);
}

void StratumServer::HandleDisconnected(connection_it *conn)
{
    auto conn_ptr = (*(*conn));
    DisconnectClient(conn_ptr);
}

void StratumServer::DisconnectClient(
    const std::shared_ptr<Connection<StratumClient>> conn_ptr)
{
    auto sock = conn_ptr->sock;
    stats_manager.PopWorker(conn_ptr->ptr->GetFullWorkerName(),
                            conn_ptr->ptr->GetAddress());
    std::unique_lock lock(clients_mutex);
    clients.erase(conn_ptr);

    logger.Log<LogType::Info>(
                "Stratum client disconnected. sock: {}", sock);
}

// TODO: add EPOLLERR, EPOLLHUP