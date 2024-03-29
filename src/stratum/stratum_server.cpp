#include "stratum_server.hpp"
template class StratumServer<ZanoStatic>;
template class StratumServer<VrscStatic>;

template <StaticConf confs>
StratumServer<confs>::StratumServer(CoinConfig &&conf)
    : StratumBase(std::move(conf)),
      daemon_manager(coin_config.rpcs),
      job_manager(&daemon_manager, coin_config.pool_addr),
      block_submitter(&daemon_manager, &round_manager),
      stats_manager(persistence_layer, &round_manager, &conf.stats,
                    GetHashMultiplier<confs>())
{
    static_assert(confs.DIFF1 != 0, "DIFF1 can't be zero!");
    job_manager.GetFirstJob();
    persistence_layer.Init();

    if (auto error =
            httpParser.allocate(HTTP_REQ_ALLOCATE, MAX_HTTP_JSON_DEPTH);
        error != simdjson::SUCCESS)
    {
        logger.template Log<LogType::Critical>(
            "Failed to allocate http parser buffer: {}",
            simdjson::error_message(error));
        exit(EXIT_FAILURE);
    }

    // init hash functions if needed
    HashWrapper::InitSHA256();

    if constexpr (confs.HASH_ALGO == HashAlgo::VERUSHASH_V2b2)
    {
        HashWrapper::InitVerusHash();
    }

    stats_thread =
        std::jthread(std::bind_front(&StatsManager::Start, &stats_manager));
}

template <StaticConf confs>
StratumServer<confs>::~StratumServer()
{
    stats_thread.request_stop();
    this->logger.template Log<LogType::Info>("Stratum destroyed.");
}

// TODO: test buffer too little
// TODO: test buffer flooded
template <StaticConf confs>
void StratumServer<confs>::HandleBlockNotify()
{
    const std::shared_ptr<JobT> new_job = job_manager.GetNewJob();

    if (!new_job)
    {
        // logger.template Log<LogType::Critical>(
        //     "Block update error: Failed to generate new job! retrying...");
        return;
    }

    HandleNewJob(std::move(new_job));
}

template <StaticConf confs>
void StratumServer<confs>::HandleNewJob()
{
    HandleNewJob(job_manager.GetLastJob());
}

// ONLY UPDATE DIFFICULTY ON NEW JOB AS MINERS WILL IGNORE IT OTHERWISE...
template <StaticConf confs>
void StratumServer<confs>::HandleNewJob(const std::shared_ptr<JobT> new_job)
{
    {
        std::shared_lock clients_read_lock(clients_mutex);
        for (const auto &[conn, _] : clients)
        {
            auto cli = conn->ptr.get();
            if (cli->GetHasAuthorized())
            {
                double diff;
                std::optional<double> new_diff = cli->GetPendingDifficulty();
                if (new_diff)
                {
                    diff = new_diff.value();
                    UpdateDifficulty(conn.get());
                }
                else
                {
                    diff = cli->GetDifficulty();
                }

                BroadcastJob(conn.get(), diff, new_job.get());

                if (new_diff)
                {
                    cli->ActivatePendingDiff();
                }
            }
        }

        // after we broadcasted new job:
        // > kick clients with below min difficulty
        // > update the map according to difficulties (notify best miners
        // first) > reset duplicate shares map

        for (auto it = clients.begin(); it != clients.end();)
        {
            const auto cli = (*it).first->ptr;
            const auto cli_diff = cli->GetDifficulty();
            if (cli_diff < coin_config.diff_config.minimum_diff)
            {
                this->DisconnectClient(it->first);
                it++;
            }
            else
            {
                ++it;
            }

            // update the difficulty in the map
            it->second = cli_diff;
        }

        // only reset share set if we invalidated the old jobs!
        if (new_job->clean)
        {
            round_manager.SetNewBlockStats(coin_config.symbol, new_job->height,
                                           new_job->target_diff);
            for (const auto &[cli, _] : clients)
            {
                cli->ptr->ResetShareSet();
            }
        }
    }

    // the estimated share amount is supposed to be meet at block time
    const double net_est_hr = new_job->expected_hashes / confs.BLOCK_TIME;
    stats_manager.SetNetworkStats(NetworkStats{
        .network_hr = net_est_hr, .difficulty = new_job->target_diff});

    logger.template Log<LogType::Info>(
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
        "", fmt::format("Job #{}", new_job->id),
        fmt::format("Height: {}", new_job->height),
        // fmt::format("Min time: {}", new_job->min_time),
        fmt::format("Difficulty: {}", new_job->target_diff),
        fmt::format("Est. shares: {}", new_job->expected_hashes),
        fmt::format("Block reward: {}", new_job->coinbase_value),
        fmt::format("Transaction count: {}", new_job->tx_count),
        fmt::format("Block size: {}", new_job->block_size * 2),
        std::max(int(new_job->id.size()), 40));
}

template <StaticConf confs>
RpcResult StratumServer<confs>::HandleShare(Connection<StratumClient> *con,
                                            WorkerContextT *wc, ShareT &share)
{
    const auto cli = con->ptr.get();
    uint64_t time = GetCurrentTimeMs();

    std::optional<FullId> authorized_id_opt = cli->GetAuthorizedId(share.worker);
    if (!authorized_id_opt)
    {
        return RpcResult(ResCode::UNAUTHORIZED_WORKER, "Unauthorized worker");
    }
    FullId authorized_id = authorized_id_opt.value();

    ShareResult share_res;
    RpcResult rpc_res(ResCode::OK);
    const std::shared_ptr<JobT> job = job_manager.GetJob(share.job_id);

    if (job == nullptr)
    {
        stats_manager.AddStaleShare(cli->stats_it);

        return RpcResult(ResCode::JOB_NOT_FOUND,
                         "Job not found");  // copy ellision
    }
    else
    {
        ShareProcessor::Process<confs>(share_res, cli, wc, job.get(), share,
                                       time);
    }

    if (share_res.code == ResCode::VALID_BLOCK) [[unlikely]]
    {
        std::size_t blockSize = job->block_size * 2;
        std::string blockData;
        blockData.reserve(blockSize);

        if constexpr (confs.STRATUM_PROTOCOL == StratumProtocol::ZEC)
        {
            job->GetBlockHex(blockData, wc->block_header.data());
        }
        else if constexpr (confs.STRATUM_PROTOCOL == StratumProtocol::BTC)
        {
            job->GetBlockHex(blockData, wc->block_header, cli->extra_nonce_sv,
                             share.extranonce2);
        }
        else if constexpr (confs.STRATUM_PROTOCOL == StratumProtocol::CN)
        {
            job->GetBlockHex(blockData, share.nonce);
            // special hash
            share_res.hash_bytes = job->GetBlockHash(share.nonce);
        }

        // submit ASAP
        auto block_hex = std::string_view(blockData.data(), blockSize);
        block_submitter.TrySubmit(block_hex, httpParser);

        logger.template Log<LogType::Info>("Block hex: {}", block_hex);

        // job will remain safe thanks to the lock.
        const auto chain_round = round_manager.GetChainRound();
        const uint64_t duration_ms = time - chain_round.round_start_ms;

        const double effort_percent =
            ((chain_round.total_effort) / job->expected_hashes) * 100.f;

        auto bs = BlockSubmission{
            .id = 0,  // QUERY LATER
            .miner_id = authorized_id.miner_id,
            .worker_id = authorized_id.worker_id,
            .block_type = static_cast<uint8_t>(BlockStatus::PENDING),
            .chain = 0,
            .reward = job->coinbase_value,
            .time_ms = time,
            .duration_ms = duration_ms,
            .height = job->height,
            .difficulty = share_res.difficulty,
            .effort_percent = effort_percent,
            .hash_bin = share_res.hash_bytes};

        auto submission = std::make_unique<BlockSubmission>(bs);

        block_submitter.AddImmatureBlock(std::move(submission),
                                         coin_config.pow_fee);

        // take into account in PPLNS but not in round effort.
        round_manager.AddRoundSharePPLNS(authorized_id.miner_id,
                                         share_res.difficulty);

        stats_manager.AddValidShare(cli->stats_it, cli->GetDifficulty());
        return RpcResult(ResCode::OK);
    }
    else if (share_res.code == ResCode::VALID_SHARE) [[likely]]
    {
        round_manager.AddRoundShare(authorized_id.miner_id,
                                    share_res.difficulty);
        round_manager.AddRoundSharePPLNS(authorized_id.miner_id,
                                         cli->GetDifficulty());
        stats_manager.AddValidShare(cli->stats_it, cli->GetDifficulty());
        return RpcResult(ResCode::OK);
    }

    stats_manager.AddInvalidShare(cli->stats_it);

    return RpcResult(share_res.code, share_res.message);
}

// the shared pointer makes sure the client won't be freed as long as we are
// processing it

template <StaticConf confs>
void StratumServer<confs>::HandleConsumeable(connection_it *it)
{
    static thread_local WorkerContext<confs.BLOCK_HEADER_SIZE> wc;

    std::shared_ptr<Connection<StratumClient>> conn = *(*it);

    // bigger than 0
    size_t req_len = 0;
    size_t next_req_len = 0;
    const char *last_req_end = nullptr;
    const char *req_start = nullptr;
    char *req_end = nullptr;
    char *buffer = conn->req_buff;

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

template <StaticConf confs>
bool StratumServer<confs>::HandleConnected(connection_it *it)
{
    std::shared_ptr<Connection<StratumClient>> conn = *(*it);

    conn->ptr = std::make_shared<StratumClient>(
        GetCurrentTimeMs(), coin_config.diff_config.default_diff,
        coin_config.diff_config.target_shares_rate,
        coin_config.diff_config.retarget_interval);

    if (job_manager.GetLastJob() == nullptr)
    {
        // disconnect if we don't have any jobs to not cause a crash, more
        // efficient to check here than everytime we broadcast a job (there will
        // not be case where job is empty after the first job.)
        logger.template Log<LogType::Warn>(
            "Rejecting client connection, as there isn't a job!");
        return false;
    }

    std::unique_lock lock(clients_mutex);
    clients.try_emplace(conn, 0);

    return true;
}

template <StaticConf confs>
void StratumServer<confs>::DisconnectClient(
    const std::shared_ptr<Connection<StratumClient>> conn_ptr)
{
    stats_manager.PopWorker(conn_ptr->ptr->stats_it);
    std::unique_lock lock(clients_mutex);
    clients.erase(conn_ptr);

    // auto worker_name = conn_ptr->ptr->GetFullWorkerName();
    // logger.template Log<LogType::Info>("Stratum worker {} disconnected.",
    //                                    worker_name);
}

template <StaticConf confs>
RpcResult StratumServer<confs>::HandleAuthorize(StratumClient *cli,
                                                std::string_view address,
                                                std::string_view worker)
{
    using namespace simdjson;

    bool is_alias = address.starts_with("@");
    std::string_view alias;

    currency::blobdata addr_blob(address.data(), address.size());

    if (worker.size() > MAX_WORKER_NAME_LEN)
    {
        return RpcResult(
            ResCode::UNAUTHORIZED_WORKER,
            "Worker name too long! (max " xSTRR(MAX_WORKER_NAME_LEN) " chars)");
    }

    if (is_alias)
    {
        alias = address.substr(1);
        AliasRes alias_res;
        if (!daemon_manager.ValidateAliasEncoding(alias))
        {
            return RpcResult(ResCode::UNAUTHORIZED_WORKER,
                             "Invalid alias name!");
        }

        if (daemon_manager.GetAliasAddress(alias_res, alias, httpParser))
        {
            address = alias_res.address;
        }
        else
        {
            return RpcResult(ResCode::UNAUTHORIZED_WORKER,
                             "Alias does not exist!");
        }
    }
    else
    {
        std::vector<uint8_t> data;
        // DECODEBASE58CHECK
        if (!DecodeBase58(std::string(address), data))
        {
            return RpcResult(ResCode::UNAUTHORIZED_WORKER,
                             fmt::format("Invalid address {} !", address));
        }
    }

    // if the miner address is already in the database we have already
    // validated the address, let it through
    auto [miner_id, db_alias] = PersistenceLayer::GetMinerId(address, alias);

    if (miner_id == -1)
    {
        logger.template Log<LogType::Info>("New miner has joined the pool: {}",
                                           address);
    }
    // always try to add timeserieses
    if (!stats_manager.AddMiner(miner_id, address, alias,
                                this->coin_config.min_payout_threshold))
    {
        return RpcResult(
            ResCode::UNAUTHORIZED_WORKER,
            "Failed to add miner to database, please contact support!");
    }

    else if (is_alias && db_alias != alias &&
             !PersistenceLayer::UpdateAlias(miner_id, alias))
    {
        return RpcResult(
            ResCode::UNAUTHORIZED_WORKER,
            "Failed to update miner identity, please contact support!");
    }

    int64_t worker_id =
        PersistenceLayer::GetWorkerId(static_cast<MinerId>(miner_id), worker);

    if (worker_id == -1)
    {
        logger.template Log<LogType::Info>("New worker has joined the pool: ",
                                           worker);
    }

    if (!stats_manager.AddWorker(worker_id, miner_id, address, worker, alias))
    {
        return RpcResult(
            ResCode::UNAUTHORIZED_WORKER,
            "Failed to add worker to database, please contact support!");
    }

    const auto fid = FullId{static_cast<MinerId>(miner_id),
                            static_cast<WorkerId>(worker_id)};

    // std::string worker_full_str = fmt::format("{}.{}", address, worker);
    worker_map::iterator stats_it = stats_manager.AddExistingWorker(fid);

    cli->AuthorizeWorker(fid, worker, stats_it);

    logger.template Log<LogType::Info>("Authorized worker: {}, address: {}",
                                       worker, address);

    return RpcResult(ResCode::OK);
}

template <StaticConf confs>
RpcResult StratumServer<confs>::HandleAuthorize(
    StratumClient *cli, simdjson::ondemand::array &params)
{
    using namespace simdjson;

    std::string_view worker_full;
    try
    {
        worker_full = params.at(0).get_string();
    }
    catch (const simdjson_error &err)
    {
        logger.template Log<LogType::Error>(
            "No worker name provided in authorization. err: {}", err.what());

        return RpcResult(ResCode::UNAUTHORIZED_WORKER,
                         "Bad request, no worker name!");
    }

    const size_t sep = worker_full.find('.');
    if (sep == std::string::npos)
    {
        logger.template Log<LogType::Error>("Bad worker name format: {}",
                                            worker_full);

        return RpcResult(ResCode::UNAUTHORIZED_WORKER,
                         "Bad request, bad worker format!");
    }

    std::string_view miner = worker_full.substr(0, sep);
    std::string_view worker =
        worker_full.substr(sep + 1, worker_full.size() - 1);

    return HandleAuthorize(cli, miner, worker);
}

template <StaticConf confs>
bool StratumServer<confs>::HandleTimeout(connection_it *conn,
                                         uint64_t timeout_streak)
{
    auto conn_ptr = *(*conn);

    logger.template Log<LogType::Info>(
        "Worker with ip {} has timed out, expiration streak: {}", conn_ptr->ip,
        timeout_streak);

    // disconnect unauthorized miner after timeout, will call handle
    // disconnected

    if (auto cli = conn_ptr->ptr; !cli->GetHasAuthorized())
    {
        logger.template Log<LogType::Warn>(
            "Disconnecting worker with ip {}, hasn't authorized.",
            conn_ptr->ip);

        return false;
    }
    else
    {
        cli->HandleAdjust(GetCurrentTimeMs());
    }
    return true;
}