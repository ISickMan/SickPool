#include "stratum_server.hpp"

StratumServer::StratumServer(const CoinConfig &conf)
    : coin_config(conf),
      redis_manager("127.0.0.1", (int)conf.redis_port),
      diff_manager(&clients, &clients_mutex, coin_config.target_shares_rate),
      stats_manager(&redis_manager, &diff_manager,
                    (int)coin_config.hashrate_interval_seconds,
                    (int)coin_config.effort_interval_seconds,
                    (int)coin_config.average_hashrate_interval_seconds,
                    (int)coin_config.diff_adjust_seconds,
                    (int)coin_config.hashrate_ttl_seconds),
      daemon_manager(coin_config.rpcs),
      job_manager(&daemon_manager, coin_config.pool_addr),
      round_manager(&redis_manager, &stats_manager),
      submission_manager(&redis_manager, &daemon_manager, &round_manager)
{
    auto error = httpParser.allocate(MAX_HTTP_REQ_SIZE, MAX_HTTP_JSON_DEPTH);
    if (error != simdjson::SUCCESS)
    {
        Logger::Log(LogType::Critical, LogField::Stratum,
                    "Failed to allocate http parser buffer: {}",
                    simdjson::error_message(error));
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    // struct timeval timeout;
    // timeout.tv_sec = coin_config.socket_recv_timeout_seconds;
    // timeout.tv_usec = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
        throw std::runtime_error("Failed to set stratum socket options");

    // if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
    // sizeof(timeout)))
    //     throw std::runtime_error("Failed to set stratum socket options");

    if (sockfd == -1)
        throw std::runtime_error("Failed to create stratum socket");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(coin_config.stratum_port));

    if (bind(sockfd, (const sockaddr *)&addr, sizeof(addr)) != 0)
    {
        throw std::runtime_error("Stratum server failed to bind to port " +
                                 std::to_string(coin_config.stratum_port));
    }

    // init hash functions if needed
    HashWrapper::InitSHA256();
#if POW_ALGO == POW_ALGO_VERUSHASH
    HashWrapper::InitVerusHash();
#endif

    // redis_manager.UpdatePoS(0, GetCurrentTimeMs());

    std::thread stats_thread(&StatsManager::Start, &stats_manager);
    stats_thread.detach();

    control_server.Start(coin_config.control_port);
    std::thread control_thread(&StratumServer::HandleControlCommands, this);
    control_thread.detach();
}

StratumServer::~StratumServer()
{
    // for (std::unique_ptr<StratumClient> cli : this->clients){
    //     close(cli.get()->GetSock());
    // }
    close(this->sockfd);
}

void StratumServer::HandleControlCommands()
{
    while (true)
    {
        ControlCommands cmd = control_server.GetNextCommand();
        HandleControlCommand(cmd);
    }
}

void StratumServer::HandleControlCommand(ControlCommands cmd)
{
    auto val = simdjson::ondemand::array();
    switch (cmd)
    {
        case ControlCommands::BLOCK_NOTIFY:
            HandleBlockNotify(val);
            break;
        case ControlCommands::WALLET_NOTFIY:
            HandleWalletNotify(val);
            break;
    }
}

void StratumServer::StartListening()
{
    if (listen(this->sockfd, 1024) != 0)
        throw std::runtime_error(
            "Stratum server failed to enter listenning state.");

    auto val = simdjson::ondemand::array();
    HandleBlockNotify(val);

    std::thread listeningThread(&StratumServer::Listen, this);
    listeningThread.join();
}

void StratumServer::Listen()
{
    Logger::Log(LogType::Info, LogField::Stratum,
                "Started listenning on port: {}", ntohs(this->addr.sin_port));

    while (true)
    {
        int conn_fd;
        struct sockaddr_in conn_addr;
        socklen_t addr_len = sizeof(conn_addr);
        conn_fd = accept(sockfd, (sockaddr *)&conn_addr, &addr_len);

        char ip_str[INET_ADDRSTRLEN];
        struct in_addr ip_addr = conn_addr.sin_addr;
        inet_ntop(AF_INET, &ip_addr, ip_str, sizeof(ip_str));

        Logger::Log(LogType::Info, LogField::Stratum,
                    "Tcp client connected, ip: {}, starting new thread...",
                    ip_str);

        if (conn_fd <= 0)
        {
            Logger::Log(
                LogType::Warn, LogField::Stratum,
                "Invalid connecting socket accepted errno: {}. Ignoring...",
                errno);
            continue;
        }

        std::thread cliHandler(&StratumServer::HandleSocket, this, conn_fd);
        // SetHighPriorityThrea d(cliHandler);
        cliHandler.detach();
    }
}

void StratumServer::HandleSocket(int conn_fd)
{
    auto client = std::make_unique<StratumClient>(conn_fd, GetCurrentTimeMs(),
                                                  coin_config.default_diff);
    auto cli_ptr = client.get();

    {
        std::scoped_lock cli_lock(clients_mutex);
        // client will be null after move, so assign the new ref to it
        this->clients.push_back(std::move(client));
        // client is moved, do not use
    }

    // simdjson requires some extra bytes, so don't write to the last bytes
    char buffer[REQ_BUFF_SIZE];
    char *reqEnd = nullptr;
    char *reqStart = nullptr;
    const char *lastReqEnd = nullptr;
    std::size_t total = 0;
    std::size_t reqLen = 0;
    std::size_t nextReqLen = 0;
    std::size_t recvRes = 0;
    bool isBuffMaxed = false;

    while (true)
    {
        do
        {
            recvRes = recv(conn_fd, buffer + total,
                           REQ_BUFF_SIZE_REAL - total - 1, 0);
            total += recvRes;
            buffer[total] = '\0';  // for strchr
            reqEnd = std::strchr(buffer, '\n');
            isBuffMaxed = total >= REQ_BUFF_SIZE_REAL;
        } while (reqEnd == nullptr && recvRes && !isBuffMaxed);

        // exit loop, miner disconnected
        if (!recvRes)
        {
            stats_manager.PopWorker(cli_ptr->GetFullWorkerName(),
                                    cli_ptr->GetAddress());
            close(cli_ptr->GetSock());

            {
                std::scoped_lock cli_lock(clients_mutex);
                clients.erase(
                    std::find(clients.begin(), clients.end(), client));
            }

            Logger::Log(LogType::Info, LogField::Stratum,
                        "Client disconnected. res: {}, errno: {}", recvRes,
                        errno);
            break;
        }
        // std::cout << total << std::endl;
        // std::cout << std::string_view(buffer, total) << std::endl;

        if (isBuffMaxed && !reqEnd)
        {
            Logger::Log(LogType::Critical, LogField::Stratum,
                        "Request too big. {}", std::string_view(buffer, total));

            total = 0;
            continue;
        }

        // there can be multiple messages in 1 recv
        // {1}\n{2}
        reqStart = strchr(buffer, '{');
        while (reqEnd != nullptr)
        {
            reqLen = reqEnd - reqStart;
            HandleReq(cli_ptr, reqStart, reqLen);

            lastReqEnd = reqEnd;
            reqStart = strchr(reqEnd + 1, '{');
            reqEnd = strchr(reqEnd + 1, '\n');
        }

        nextReqLen = total - (lastReqEnd - buffer + 1);  // don't inlucde \n
        std::memmove(buffer, lastReqEnd + 1, nextReqLen);
        buffer[nextReqLen] = '\0';

        total = nextReqLen;
    }
}

void StratumServer::HandleReq(StratumClient *cli, char *buffer,
                              std::size_t reqSize)
{
    int id = 0;
    std::string_view method;
    simdjson::ondemand::array params;

    auto start = std::chrono::steady_clock::now();

    // std::cout << "last char -> " << (int)buffer[]
    simdjson::ondemand::document doc;
    try
    {
        doc = cli->GetParser()->iterate(buffer, reqSize,
                                        reqSize + simdjson::SIMDJSON_PADDING);

        simdjson::ondemand::object req = doc.get_object();
        id = static_cast<int>(req["id"].get_int64());
        method = req["method"].get_string();
        params = req["params"].get_array();
    }
    catch (const simdjson::simdjson_error &err)
    {
        SendReject(cli, id, (int)ShareCode::UNKNOWN, "Bad request");
        Logger::Log(LogType::Error, LogField::Stratum,
                    "Request JSON parse error: {}\nRequest: {}\n", err.what(),
                    std::string_view(buffer, reqSize));
        return;
    }
    auto end = std::chrono::steady_clock::now();
    auto dur =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count();
    // std::cout << "req parse took: " << dur << "micro seconds." << std::endl;

    if (method == "mining.submit")
    {
        HandleSubmit(cli, id, params);
    }
    else if (method == "mining.subscribe")
    {
        HandleSubscribe(cli, id, params);
    }
    else if (method == "mining.authorize")
    {
        HandleAuthorize(cli, id, params);
    }
    else
    {
        SendReject(cli, id, (int)ShareCode::UNKNOWN, "Unknown method");
        Logger::Log(LogType::Warn, LogField::Stratum,
                    "Unknown request method: {}", method);
    }
}

void StratumServer::HandleBlockNotify(const simdjson::ondemand::array &params)
{
    using namespace simdjson;
    int64_t curtimeMs = GetCurrentTimeMs();

    const job_t *newJob = job_manager.GetNewJob();

    while (newJob == nullptr)
    {
        newJob = job_manager.GetNewJob();

        Logger::Log(
            LogType::Critical, LogField::Stratum,
            "Block update error: Failed to generate new job! retrying...");

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    // save it to process round
    {
        std::scoped_lock clients_lock(clients_mutex);
        for (auto &cli : clients)
        {
            // (*it)->SetDifficulty(1000000, curtimeMs);
            // (*it)->SetDifficulty(job->GetTargetDiff(), curtimeMs);
            // UpdateDifficulty(*it);
            if (cli->GetIsPendingDiff())
            {
                cli->ActivatePendingDiff();
                UpdateDifficulty(cli.get());
            }
            BroadcastJob(cli.get(), newJob);
            cli->ResetShareSet();
        }
    }

    // in case of a crash without shares, we need to update round start time

    std::scoped_lock redis_lock(redis_mutex);

    // while (jobs.size() > 1)
    // {
    //     // TODO: fix
    //     //  delete jobs[0];
    //     //  jobs.erase();
    // }
    submission_manager.CheckImmatureSubmissions();
    redis_manager.AddNetworkHr(chain, curtimeMs, newJob->GetTargetDiff());

    redis_manager.SetRoundEstimatedEffort(chain, "pow",
                                          newJob->GetEstimatedShares());
    // TODO: combine all redis functions one new block to one pipelined

    Logger::Log(LogType::Info, LogField::JobManager,
                "Broadcasted new job: #{}", newJob->GetId());
    Logger::Log(LogType::Info, LogField::JobManager, "Height: {}",
                newJob->GetHeight());
    Logger::Log(LogType::Info, LogField::JobManager, "Min time: {}",
                newJob->GetMinTime());
    Logger::Log(LogType::Info, LogField::JobManager, "Difficutly: {}",
                newJob->GetTargetDiff());
    Logger::Log(LogType::Info, LogField::JobManager, "Est. shares: {}",
                newJob->GetEstimatedShares());
    // Logger::Log(LogType::Info, LogField::JobManager, "Target: %s",
    //             job->GetTarget()->GetHex().c_str());
    Logger::Log(LogType::Info, LogField::JobManager, "Block reward: {}",
                newJob->GetBlockReward());
    Logger::Log(LogType::Info, LogField::JobManager, "Transaction count: {}",
                newJob->GetTransactionCount());
    Logger::Log(LogType::Info, LogField::JobManager, "Block size: {}",
                newJob->GetBlockSize());
}

// use wallletnotify with "%b %s %w" arg (block hash, txid, wallet address),
// check if block hash is smaller than current job's difficulty to check whether
// its pos block.
void StratumServer::HandleWalletNotify(simdjson::ondemand::array &params)
{
    using namespace simdjson;
    /// TODO: exception handle this and everything like this
    std::string_view blockHash;
    std::string_view txId;
    std::string_view address;

    try
    {
        auto it = params.begin();
        blockHash = (*it).get_string();
        txId = (*++it).get_string();
        address = (*++it).get_string();
    }
    catch (const simdjson::simdjson_error &err)
    {
        Logger::Log(LogType::Warn, LogField::Stratum,
                    "Failed to parse wallet notify, txid: {}, error: {}", txId,
                    err.what());
        return;
    }

    auto bhash256 = UintToArith256(uint256S(blockHash.data()));
    double bhashDiff = BitsToDiff(bhash256.GetCompact());
    double pow_diff = job_manager.GetLastJob()->GetTargetDiff();

    if (bhashDiff >= pow_diff)
    {
        return;  // pow block
    }

    if (address != coin_config.pool_addr)
    {
        Logger::Log(LogType::Error, LogField::Stratum,
                    "CheckAcceptedBlock error: Wrong address: {}, block: {}",
                    address, blockHash);
        return;
    }

    Logger::Log(LogType::Info, LogField::Stratum,
                "Received PoS TxId: {}, block hash: {}", txId, blockHash);

    // now make sure we actually staked the PoS block and not just received a tx
    // inside one.

    std::string resBody;
    int64_t value;

    int resCode = daemon_manager.SendRpcReq<std::any>(
        resBody, 1, "getrawtransaction", std::any(txId),
        std::any(1));  // verboose

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
        value = output1["valueSat"].get_int64();

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
                    address, blockHash);
    }

    resCode = daemon_manager.SendRpcReq<std::any>(resBody, 1, "getblock",
                                                  std::any(blockHash));

    if (resCode != 200)
    {
        Logger::Log(LogType::Error, LogField::Stratum,
                    "CheckAcceptedBlock error: Failed to getblock, "
                    "http code: {}",
                    resCode);
        return;
    }

    std::string_view validationType;
    std::string_view coinbaseTxId;
    try
    {
        ondemand::document doc = httpParser.iterate(
            resBody.data(), resBody.size(), resBody.capacity());

        ondemand::object res = doc["result"].get_object();
        validationType = res["validationtype"];
        coinbaseTxId = (*res["tx"].get_array().begin()).get_string();
    }
    catch (const simdjson_error &err)
    {
        Logger::Log(LogType::Error, LogField::Stratum,
                    "HandleWalletNotify (1): Failed to parse json, error: {}",
                    err.what());
        return;
    }

    if (validationType != "stake")
    {
        Logger::Log(LogType::Critical, LogField::Stratum,
                    "Double PoS block check failed! block hash: {}", blockHash);
        return;
    }
    if (coinbaseTxId != txId)
    {
        Logger::Log(LogType::Critical, LogField::Stratum,
                    "TxId is not coinbase, block hash: {}", blockHash);
        return;
    }
    // we have verified:
    //  block is PoS (twice),
    // the txid is ours (got its value),
    // the txid is indeed the coinbase tx

    Logger::Log(LogType::Info, LogField::Stratum, "Found PoS Block! hash: {}",
                blockHash);
}

void StratumServer::HandleSubscribe(const StratumClient *cli, int id,
                                    simdjson::ondemand::array &params) const
{
    // Mining software info format: "SickMiner/6.9"
    // if (params.IsArray() && params[0].IsString())
    //     std::string software_info = params[0].GetString();
    // if (params[1].IsString())
    //     std::string last_session_id = params[1].GetString();
    // if (params[2].IsString()) std::string host = params[2].GetString();
    // if (params[3].IsInt()) int port = params[3].GetInt();

    // REQ
    //{"id": 1, "method": "mining.subscribe", "params": ["MINER_USER_AGENT",
    //"SESSION_ID", "CONNECT_HOST", CONNECT_PORT]} \n

    // RES
    //{"id": 1, "result": ["SESSION_ID", "NONCE_1"], "error": null} \n

    // we don't send session id

    char response[128];
    int len =
        snprintf(response, sizeof(response),
                 "{\"id\":%d,\"result\":[null,\"%.*s\"],\"error\":null}\n", id,
                 EXTRANONCE_SIZE * 2, cli->GetExtraNonce().data());
    SendRaw(cli->GetSock(), response, len);

    Logger::Log(LogType::Info, LogField::Stratum, "client subscribed!");
}

void StratumServer::HandleAuthorize(StratumClient *cli, int id,
                                    simdjson::ondemand::array &params)
{
    using namespace simdjson;

    std::size_t split = 0;
    int resCode = 0;
    std::string valid_addr;
    std::string idTag = "null";
    std::string_view given_addr;
    std::string_view worker;
    bool isValid = false;
    bool isIdentity = false;

    std::string resultBody;
    ondemand::document doc;
    ondemand::object res;

    std::string_view worker_full;
    try
    {
        auto it = params.begin();
        worker_full = (*it).get_string();
    }
    catch (const simdjson_error &err)
    {
        Logger::Log(LogType::Error, LogField::Stratum,
                    "No worker name provided in authorization. err: {}",
                    err.what());
        return;
    }

    // worker name format: address.worker_name
    split = worker_full.find('.');

    if (split == std::string_view::npos)
    {
        SendReject(cli, id, (int)ShareCode::UNAUTHORIZED_WORKER,
                   "invalid worker name format, use: address/id@.worker");
        return;
    }
    else if (worker_full.size() > MAX_WORKER_NAME_LEN + ADDRESS_LEN + 1)
    {
        SendReject(
            cli, id, (int)ShareCode::UNAUTHORIZED_WORKER,
            "Worker name too long! (max " STRM(MAX_WORKER_NAME_LEN) " chars)");
        return;
    }

    given_addr = worker_full.substr(0, split);
    worker = worker_full.substr(split + 1, worker_full.size() - 1);

    bool oldAddress = redis_manager.DoesAddressExist(given_addr, valid_addr);

    if (!oldAddress)
    {
        resCode = daemon_manager.SendRpcReq<std::any>(
            resultBody, 1, "validateaddress", std::any(given_addr));
        try
        {
            doc = httpParser.iterate(resultBody.data(), resultBody.size(),
                                     resultBody.capacity());

            res = doc["result"].get_object();

            isValid = res["isvalid"].get_bool();
            if (!isValid)
            {
                SendReject(cli, id, (int)ShareCode::UNAUTHORIZED_WORKER,
                           "Invalid address!");
                return;
            }

            std::string_view addr_sv = res["address"].get_string();
            valid_addr = std::string(addr_sv);
            isIdentity = valid_addr[0] == 'i';
        }
        catch (const simdjson_error &err)
        {
            SendReject(cli, id, (int)ShareCode::UNAUTHORIZED_WORKER,
                       "Server error: Failed to validate address!");
            Logger::Log(LogType::Critical, LogField::Stratum,
                        "Authorize RPC (validateaddress) failed: {}",
                        err.what());
            return;
        }

        if (isIdentity)
        {
            if (given_addr == valid_addr)
            {
                // we were given an identity address (i not @), get the id@
                resCode = daemon_manager.SendRpcReq<std::any>(
                    resultBody, 1, "getidentity", std::any(valid_addr));

                try
                {
                    doc =
                        httpParser.iterate(resultBody.data(), resultBody.size(),
                                           resultBody.capacity());

                    res = doc["result"].get_object();
                    std::string_view id_sv = res["name"].get_string();
                    idTag = std::string(id_sv);
                }
                catch (const simdjson_error &err)
                {
                    SendReject(cli, id, (int)ShareCode::UNAUTHORIZED_WORKER,
                               "Server error: Failed to get id!");
                    Logger::Log(LogType::Critical, LogField::Stratum,
                                "Authorize RPC (getidentity) failed: {}",
                                err.what());
                    return;
                }
            }
            else
            {
                // we were given an id@
                idTag = std::string(given_addr);
            }
        }
    }

    std::string worker_full_str = fmt::format("{}.{}", valid_addr, worker);

    cli->SetAddress(worker_full_str, valid_addr);

    // string-views to non-local string
    bool added_to_db = stats_manager.AddWorker(
        cli->GetAddress(), cli->GetFullWorkerName(), idTag, std::time(nullptr));

    if (!added_to_db)
    {
        SendReject(cli, id, (int)ShareCode::UNAUTHORIZED_WORKER,
                   "Failed to add worker to database!");
        return;
    }
    cli->SetAuthorized();

    Logger::Log(LogType::Info, LogField::Stratum,
                "Authorized worker: {}, address: {}, id: {}", worker,
                valid_addr, idTag);

    SendAccept(cli, id);
    this->UpdateDifficulty(cli);

    const job_t *job = job_manager.GetLastJob();

    if (job == nullptr)
    {
        Logger::Log(LogType::Critical, LogField::Stratum,
                    "No jobs to broadcast!");
        return;
    }

    this->BroadcastJob(cli, job);
}

// https://zips.z.cash/zip-0301#mining-submit
void StratumServer::HandleSubmit(StratumClient *cli, int id,
                                 simdjson::ondemand::array &params)
{
    // parsing takes 0-1 us
    Share share;
    try
    {
        auto it = params.begin();
        share.worker = (*it).get_string();
        share.jobId = (*++it).get_string();
        share.time = (*++it).get_string();
        share.nonce2 = (*++it).get_string();
        share.solution = (*++it).get_string();
    }
    catch (const simdjson::simdjson_error &err)
    {
        SendReject(cli, id, (int)ShareCode::UNKNOWN, "invalid params");
        Logger::Log(LogType::Critical, LogField::Stratum,
                    "Failed to parse submit: {}", err.what());
        return;
    }

    HandleShare(cli, id, share);
}

void StratumServer::HandleShare(StratumClient *cli, int id, Share &share)
{
    int64_t time = GetCurrentTimeMs();
    Benchmark<std::chrono::microseconds> share_bench("Share process");
    auto start = TIME_NOW();

    ShareResult share_res;
    const job_t *job = job_manager.GetJob(share.jobId);

    ShareProcessor::Process(*cli, job, share, share_res, time);

    // > add share stats before submission to have accurate effort (its fast)
    // > possible that a timeseries wasn't created yet, so don't add shares
    if (share_res.code != ShareCode::UNAUTHORIZED_WORKER)
    {
        round_manager.AddRoundEffort(COIN_SYMBOL, share_res.difficulty);
        stats_manager.AddShare(cli->GetFullWorkerName(), cli->GetAddress(),
                               share_res.difficulty);
    }

    switch (share_res.code)
    {
        case ShareCode::VALID_BLOCK:
        {
            std::size_t blockSize = job->GetBlockSize();
            char blockData[blockSize];

            job->GetBlockHex(cli->GetBlockheaderBuff(), blockData);

            // submit ASAP
            auto block_hex = std::string_view(blockData, blockSize);
            submission_manager.TrySubmit(chain, block_hex);

            const int confirmations = 0;
            const std::string_view worker_full(cli->GetFullWorkerName());
            const auto chainRound = round_manager.GetChainRound(chain);
            const auto type = BlockType::POW;
            const int64_t duration_ms = time - chainRound.round_start_ms;
            const double effort_percent =
                chainRound.total_effort / job->GetEstimatedShares();

            auto submission = std::make_unique<BlockSubmission>(
                confirmations, type, job->GetBlockReward(), time, duration_ms,
                job->GetHeight(), SubmissionManager::block_number,
                share_res.difficulty, effort_percent);

            memcpy(submission->chain, chain.data(), chain.size());
            memcpy(submission->miner, worker_full.data(), ADDRESS_LEN);
            memcpy(submission->worker, worker_full.data() + ADDRESS_LEN,
                   worker_full.size() - ADDRESS_LEN);
            Hexlify((char *)submission->hashHex, share_res.hash_bytes.data(),
                    HASH_SIZE);
            ReverseHex((char *)submission->hashHex, (char *)submission->hashHex,
                       HASH_SIZE_HEX);

            submission_manager.AddImmatureBlock(std::move(submission),
                                                coin_config.pow_fee);
            SendAccept(cli, id);
            break;
        }
        case ShareCode::VALID_SHARE:
            SendAccept(cli, id);
            break;
        case ShareCode::JOB_NOT_FOUND:
            Logger::Log(LogType::Warn, LogField::Stratum,
                        "Received share for unknown job id: {}", share.jobId);
        default:
            SendReject(cli, id, (int)share_res.code, share_res.message.c_str());
            break;
    }

    auto end = TIME_NOW();

    // Logger::Log(LogType::Debug, LogField::Stratum,
    //             "Share processed in {}us, diff: {}, res: {}",
    //             std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(), share_res.difficulty, (int)share_res.code);
}

void StratumServer::SendReject(const StratumClient *cli, int id, int err,
                               const char *msg) const
{
    char buffer[512];
    int len =
        snprintf(buffer, sizeof(buffer),
                 "{\"id\":%d,\"result\":null,\"error\":[%d,\"%s\",null]}\n", id,
                 err, msg);
    SendRaw(cli->GetSock(), buffer, len);
}

void StratumServer::SendAccept(const StratumClient *cli, int id) const
{
    char buff[512];
    int len = snprintf(buff, sizeof(buff),
                       "{\"id\":%d,\"result\":true,\"error\":null}\n", id);
    SendRaw(cli->GetSock(), buff, len);
}

void StratumServer::UpdateDifficulty(StratumClient *cli)
{
    uint32_t diffBits = DiffToBits(cli->GetDifficulty());
    uint256 diff256;
    arith_uint256 arith256 = UintToArith256(diff256).SetCompact(diffBits);

    char request[512];
    int len = snprintf(
        request, sizeof(request),
        "{\"id\":null,\"method\":\"mining.set_target\",\"params\":[\"%s\"]}\n",
        arith256.GetHex().c_str());
    // arith256.GetHex().c_str());

    SendRaw(cli->GetSock(), request, len);

    Logger::Log(LogType::Debug, LogField::Stratum,
                "Set difficulty for {} to {}", cli->GetFullWorkerName(),
                arith256.GetHex());
}

void StratumServer::BroadcastJob(const StratumClient *cli, const Job *job) const
{
    // auto res =
    auto notifyMsg = job->GetNotifyMessage();
    SendRaw(cli->GetSock(), notifyMsg.data(), notifyMsg.size());
}