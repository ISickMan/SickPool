#include "stratum_server.hpp"

StratumServer::StratumServer(const CoinConfig &conf)
    : Server<StratumClient>(conf.stratum_port),
      coin_config(conf),
      redis_manager("127.0.0.1", (int)conf.redis_port),
      clients_mutex(),
      clients(),
      diff_manager(&clients, &clients_mutex, coin_config.target_shares_rate),
      round_manager_pow(&redis_manager, "pow"),
      round_manager_pos(&redis_manager, "pos"),
      stats_manager(&redis_manager, &diff_manager, &round_manager_pow,
                    (int)coin_config.hashrate_interval_seconds,
                    (int)coin_config.effort_interval_seconds,
                    (int)coin_config.average_hashrate_interval_seconds,
                    (int)coin_config.diff_adjust_seconds,
                    (int)coin_config.hashrate_ttl_seconds),
      daemon_manager(coin_config.rpcs),
      payment_manager(&redis_manager, &daemon_manager, coin_config.pool_addr,
                      coin_config.payment_interval_seconds,
                      coin_config.min_payout_threshold),
      job_manager(&daemon_manager, &payment_manager, coin_config.pool_addr),
      submission_manager(&redis_manager, &daemon_manager, &payment_manager,
                         &round_manager_pow, &round_manager_pos)
{
    auto error = httpParser.allocate(HTTP_REQ_ALLOCATE, MAX_HTTP_JSON_DEPTH);

    if (error != simdjson::SUCCESS)
    {
        Logger::Log(LogType::Critical, LogField::Stratum,
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
    Logger::Log(LogType::Info, LogField::Stratum, "Stratum destroyed.");
}

void StratumServer::HandleControlCommands(std::stop_token st)
{
    char buff[256] = {0};
    while (!st.stop_requested())
    {
        ControlCommands cmd = control_server.GetNextCommand(buff, sizeof(buff));
        HandleControlCommand(cmd, buff);
        Logger::Log(LogType::Info, LogField::ControlServer,
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
        case ControlCommands::WALLET_NOTFIY:
        {
            // format: %b%s%w (block hash, txid, wallet address)
            WalletNotify *wallet_notify =
                reinterpret_cast<WalletNotify *>(buff + 2);
            HandleWalletNotify(wallet_notify);
            break;
        }
        default:
            Logger::Log(LogType::Warn, LogField::StatsManager,
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
    Logger::Log(LogType::Info, LogField::Stratum,
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

        Logger::Log(
            LogType::Critical, LogField::Stratum,
            "Block update error: Failed to generate new job! retrying...");

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    // save it to process round
    {
        std::scoped_lock clients_lock(clients_mutex);
        for (auto &[con, _] : clients)
        {
            auto cli = con->ptr.get();
            if (cli->GetIsAuthorized())
            {
                if (cli->GetIsPendingDiff())
                {
                    cli->ActivatePendingDiff();
                    UpdateDifficulty(cli);
                }

                BroadcastJob(cli, new_job);
            }
        }
        // TODO: reset shares
    }

    payment_manager.UpdatePayouts(&round_manager_pow, curtime_ms);

    // the estimated share amount is supposed to be meet at block time
    const double net_est_hr = new_job->GetEstimatedShares() / BLOCK_TIME;

    submission_manager.CheckImmatureSubmissions();

    redis_manager.SetNewBlockStats(chain, curtime_ms, net_est_hr);

    Logger::Log(
        LogType::Info, LogField::JobManager,
        "Broadcasted new job: \n"
        "┌{0:─^{9}}┐\n"
        "│{1: ^{9}}│\n"
        "│{2: <{9}}│\n"
        "│{3: <{9}}│\n"
        "│{4: <{9}}│\n"
        "│{5: <{9}}│\n"
        "│{6: <{9}}│\n"
        "│{7: <{9}}│\n"
        "│{8: <{9}}│\n"
        "└{0:─^{9}}┘\n",
        "", fmt::format("Job #{}", new_job->GetId()),
        fmt::format("Height: {}", new_job->GetHeight()),
        fmt::format("Min time: {}", new_job->GetMinTime()),
        fmt::format("Difficulty: {}", new_job->GetTargetDiff()),
        fmt::format("Est. shares: {}", new_job->GetEstimatedShares()),
        fmt::format("Block reward: {}", new_job->GetBlockReward()),
        fmt::format("Transaction count: {}", new_job->GetTransactionCount()),
        fmt::format("Block size: {}", new_job->GetBlockSizeHex()), 40);
}

// use wallletnotify with "%b %s %w" arg (block hash, txid, wallet address),
// check if block hash is smaller than current job's difficulty to check whether
// its pos block.
void StratumServer::HandleWalletNotify(WalletNotify *wal_notify)
{
    using namespace simdjson;
    std::string_view block_hash(wal_notify->block_hash,
                                sizeof(wal_notify->block_hash));
    std::string_view tx_id(wal_notify->txid, sizeof(wal_notify->txid));
    std::string_view address(wal_notify->wallet_address,
                             sizeof(wal_notify->wallet_address));

    auto bhash256 = UintToArith256(uint256S(block_hash.data()));
    double bhashDiff = BitsToDiff(bhash256.GetCompact());
    double pow_diff = job_manager.GetLastJob()->GetTargetDiff();

    Logger::Log(LogType::Info, LogField::Stratum,
                "Received PoS TxId: {}, block hash: {}", tx_id, block_hash);

    if (bhashDiff >= pow_diff)
    {
        return;  // pow block
    }

    if (address != coin_config.pool_addr)
    {
        Logger::Log(LogType::Error, LogField::Stratum,
                    "CheckAcceptedBlock error: Wrong address: {}, block: {}",
                    address, block_hash);
        return;
    }

    // now make sure we actually staked the PoS block and not just received a tx
    // inside one.

    std::string resBody;
    int64_t reward_value;

    int resCode =
        daemon_manager.SendRpcReq(resBody, 1, "getrawtransaction",
                                  DaemonRpc::GetParamsStr(tx_id,
                                                          1));  // verboose

    if (resCode != 200)
    {
        Logger::Log(LogType::Error, LogField::Stratum,
                    "CheckAcceptedBlock error: Failed to getrawtransaction, "
                    "http code: %d",
                    resCode);
        return;
    }

    try
    {
        ondemand::document doc = httpParser.iterate(
            resBody.data(), resBody.size(), resBody.capacity());

        ondemand::object res = doc["result"].get_object();

        ondemand::array vout = res["vout"].get_array();
        auto output1 = (*vout.begin());
        reward_value = output1["valueSat"].get_int64();

        auto addresses = output1["scriptPubKey"]["addresses"];
        address = (*addresses.begin()).get_string();
    }
    catch (const simdjson_error &err)
    {
        Logger::Log(LogType::Error, LogField::Stratum,
                    "HandleWalletNotify: Failed to parse json, error: {}",
                    err.what());
        return;
    }

    // double check
    if (address != coin_config.pool_addr)
    {
        Logger::Log(LogType::Error, LogField::Stratum,
                    "CheckAcceptedBlock error: Wrong address: {}, block: {}",
                    address, block_hash);
    }

    BlockRes block_res;
    bool getblock_res =
        daemon_manager.GetBlock(block_res, httpParser, block_hash);

    if (!getblock_res)
    {
        Logger::Log(LogType::Error, LogField::Stratum, "Failed to getblock {}!",
                    block_hash);
        return;
    }

    if (block_res.validation_type != ValidationType::STAKE)
    {
        Logger::Log(LogType::Critical, LogField::Stratum,
                    "Double PoS block check failed! block hash: {}",
                    block_hash);
        return;
    }
    if (!block_res.tx_ids.size() || block_res.tx_ids[0] != tx_id)
    {
        Logger::Log(LogType::Critical, LogField::Stratum,
                    "TxId is not coinbase, block hash: {}", block_hash);
        return;
    }
    // we have verified:
    //  block is PoS (twice),
    // the txid is ours (got its value),
    // the txid is indeed the coinbase tx

    Round round = round_manager_pos.GetChainRound();
    const auto now_ms = GetCurrentTimeMs();
    const double effort_percent =
        round.total_effort / round.estimated_effort * 100.f;

    uint8_t block_hash_bin[HASH_SIZE];
    uint8_t tx_id_bin[HASH_SIZE];

    Unhexlify(block_hash_bin, block_hash.data(), block_hash.size());

    Unhexlify(tx_id_bin, tx_id.data(), tx_id.size());

    auto submission = std::make_unique<ExtendedSubmission>(
        std::string_view(chain), std::string_view(coin_config.pool_addr),
        BlockType::POS, block_res.height, reward_value, round, now_ms,
        SubmissionManager::block_number, 0.d, 0.d, block_hash_bin, tx_id_bin);

    submission_manager.AddImmatureBlock(std::move(submission),
                                        coin_config.pos_fee);

    Logger::Log(LogType::Info, LogField::Stratum,
                "Added immature PoS Block! hash: {}", block_hash);
}

RpcResult StratumServer::HandleShare(StratumClient *cli, WorkerContext *wc,
                                     share_t &share)
{
    int64_t time = GetCurrentTimeMs();
    // Benchmark<std::chrono::microseconds> share_bench("Share process");
    auto start = TIME_NOW();

    ShareResult share_res;
    const job_t *job = job_manager.GetJob(share.jobId);

    ShareProcessor::Process(share_res, cli, wc, job, share, time);

    // > add share stats before submission to have accurate effort (its fast)
    // > possible that a timeseries wasn't created yet, so don't add shares
    // already authorized here
    const double expected_shares = GetExpectedHashes(share_res.difficulty);
    round_manager_pow.AddRoundShare(cli->GetAddress(), expected_shares);
    stats_manager.AddShare(cli->GetFullWorkerName(), cli->GetAddress(),
                           expected_shares);
    // share_res.code = ResCode::VALID_BLOCK;

    auto end = TIME_NOW();

    Logger::Log(
        LogType::Debug, LogField::Stratum,
        "Share processed in {}us, diff: {}, shares: {}, res: {}",
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count(),
        share_res.difficulty, expected_shares, (int)share_res.code);

    if (unlikely(share_res.code == ResCode::VALID_BLOCK))
    {
        std::size_t blockSize = job->GetBlockSizeHex();
        char blockData[blockSize];

#ifndef STRATUM_PROTOCOL_BTC
        job->GetBlockHex(blockData, wc->block_header);
#else
        job->GetBlockHex(blockData, wc->block_header, cli->GetExtraNonce(),
                         share.extranonce2);
#endif

        if (job->GetIsPayment())
        {
            payment_manager.payment_included = true;
        }

        // submit ASAP
        auto block_hex = std::string_view(blockData, blockSize);
        submission_manager.TrySubmit(chain, block_hex);

        Logger::Log(LogType::Info, LogField::StatsManager, "Block hex: {}",
                    std::string_view(blockData, blockSize));

        const std::string_view worker_full(cli->GetFullWorkerName());
        const auto chainRound = round_manager_pow.GetChainRound();
        const auto type =
            job->GetIsPayment() ? BlockType::POW_PAYMENT : BlockType::POW;
        const double effort_percent =
            (chainRound.total_effort / job->GetEstimatedShares()) * 100.f;

        if constexpr (HASH_ALGO == HashAlgo::X25X)
        {
            HashWrapper::X22I(share_res.hash_bytes.data(), wc->block_header);
            // std::reverse(share_res.hash_bytes.begin(),
            //              share_res.hash_bytes.end());
        }

        auto submission = std::make_unique<ExtendedSubmission>(
            chain, worker_full, type, job->GetHeight(), job->GetBlockReward(),
            chainRound, time, SubmissionManager::block_number,
            share_res.difficulty, effort_percent, share_res.hash_bytes.data(),
            job->coinbase_tx_id);

        submission_manager.AddImmatureBlock(std::move(submission),
                                            coin_config.pow_fee);
        return RpcResult(ResCode::OK);
    }

    if (likely(share_res.code == ResCode::VALID_SHARE))
    {
        return RpcResult(ResCode::OK);
    }

    Logger::Log(LogType::Warn, LogField::Stratum,
                "Received bad share for job id: {}", share.jobId);

    return RpcResult(share_res.code, share_res.message);
}

void StratumServer::BroadcastJob(StratumClient *cli, const job_t *job) const
{
    // auto res =
    auto notifyMsg = job->GetNotifyMessage();
    SendRaw(cli->sock, notifyMsg.data(), notifyMsg.size());
}

void StratumServer::HandleConsumeable(connection_it *it)
{
    static thread_local WorkerContext wc;

    Connection<StratumClient> *conn = (*it)->get();
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
        HandleReq(conn->ptr.get(), &wc, std::string_view(req_start, req_len));

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
    Connection<StratumClient> *conn = (*(*it)).get();
    conn->ptr = std::make_unique<StratumClient>(conn->sock, "", 0, coin_config.default_diff);
    std::scoped_lock lock(clients_mutex);
    clients.emplace(conn, 0);
}
void StratumServer::HandleDisconnected(connection_it *conn)
{
    StratumClient *cli = (*(*conn))->ptr.get();
    stats_manager.PopWorker(cli->GetFullWorkerName(), cli->GetAddress());
    std::scoped_lock lock(clients_mutex);
    clients.erase((*conn)->get());
}

// void StratumServer::EraseClient(
//     int sockfd, std::list<std::unique_ptr<StratumClient>>::iterator *it)
// {
//     if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sockfd, nullptr) == -1)
//     {
//         Logger::Log(LogType::Warn, LogField::Stratum,
//                     "Failed to remove socket {} from epoll list errno: {} "
//                     "-> errno: {}. ",
//                     sockfd, errno, std::strerror(errno));
//     }
//     if (close(sockfd) == -1)
//     {
//         Logger::Log(LogType::Warn, LogField::Stratum,
//                     "Failed to close socket {} errno: {} "
//                     "-> errno: {}. ",
//                     sockfd, errno, std::strerror(errno));
//     }

//     std::unique_lock l(clients_mutex);
//     clients.erase(*it);
// }

// TODO: switch unordered map to linked list
// TODO: add EPOLLERR, EPOLLHUP