#include "static_config.hpp"
#ifdef STRATUM_PROTOCOL_ZEC
#include "stratum_server_zec.hpp"

void StratumServerZec::HandleReq(StratumClient *cli, WorkerContext *wc,
                                 std::string_view req)
{
    int id = 0;
    const int sock = cli->sock;

    std::string_view method;
    simdjson::ondemand::array params;

    auto start = std::chrono::steady_clock::now();

    // std::cout << "last char -> " << (int)buffer[]
    simdjson::ondemand::document doc;
    try
    {
        doc = wc->json_parser.iterate(req.data(), req.size(),
                                      req.size() + simdjson::SIMDJSON_PADDING);

        simdjson::ondemand::object req = doc.get_object();
        id = static_cast<int>(req["id"].get_int64());
        method = req["method"].get_string();
        params = req["params"].get_array();
    }
    catch (const simdjson::simdjson_error &err)
    {
        SendRes(sock, id, RpcResult(ResCode::UNKNOWN, "Bad request"));
        Logger::Log(LogType::Error, LogField::Stratum,
                    "Request JSON parse error: {}\nRequest: {}\n", err.what(),
                    req);
        return;
    }
    auto end = std::chrono::steady_clock::now();
    auto dur =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count();
    // std::cout << "req parse took: " << dur << "micro seconds." << std::endl;

    RpcResult res(ResCode::UNKNOWN);

    if (method == "mining.submit")
    {
        res = HandleSubmit(cli, wc, params);
    }
    else if (method == "mining.subscribe")
    {
        res = HandleSubscribe(cli, params);
    }
    else if (method == "mining.authorize")
    {
        res = HandleAuthorize(cli, params);
        if (res.code == ResCode::OK)
        {
            SendRes(sock, id, res);
            UpdateDifficulty(cli);

            const job_t *job = job_manager.GetLastJob();

            if (job == nullptr)
            {
                Logger::Log(LogType::Critical, LogField::Stratum,
                            "No jobs to broadcast!");
                return;
            }

            BroadcastJob(cli, job);
            return;
        }
    }
    else
    {
        res = RpcResult(ResCode::UNKNOWN, "Unknown method");
        Logger::Log(LogType::Warn, LogField::Stratum,
                    "Unknown request method: {}", method);
    }

    SendRes(sock, id, res);
}

// https://zips.z.cash/zip-0301#mining-subscribe
RpcResult StratumServerZec::HandleSubscribe(StratumClient *cli,
                                    simdjson::ondemand::array &params) const
{
    // Mining software info format: "SickMiner/6.9"
    // REQ
    //{"id": 1, "method": "mining.subscribe", "params": ["MINER_USER_AGENT",
    //"SESSION_ID", "CONNECT_HOST", CONNECT_PORT]} \n

    // RES
    //{"id": 1, "result": ["SESSION_ID", "NONCE_1"], "error": null} \n

    // we don't send session id

    Logger::Log(LogType::Info, LogField::Stratum, "client subscribed!");
    
    return RpcResult(ResCode::OK,
                     fmt::format("[null,\"{}\"]", cli->GetExtraNonce()));
}

// https://zips.z.cash/zip-0301#mining-authorize
RpcResult StratumServerZec::HandleAuthorize(StratumClient *cli,
                                         simdjson::ondemand::array &params)
{
    using namespace simdjson;

    std::size_t split = 0;
    int resCode = 0;
    std::string id_tag = "null";
    std::string_view given_addr;
    std::string_view worker;
    bool isIdentity = false;

    std::string_view worker_full;
    try
    {
        worker_full = params.at(0).get_string();
    }
    catch (const simdjson_error &err)
    {
        Logger::Log(LogType::Error, LogField::Stratum,
                    "No worker name provided in authorization. err: {}",
                    err.what());

        return RpcResult(ResCode::UNAUTHORIZED_WORKER, "Bad request");
    }

    // worker name format: address.worker_name
    split = worker_full.find('.');

    if (split == std::string_view::npos)
    {
        return RpcResult(ResCode::UNAUTHORIZED_WORKER,
                         "invalid worker name format, use: address/id@.worker");
    }
    else if (worker_full.size() > MAX_WORKER_NAME_LEN + ADDRESS_LEN + 1)
    {
        return RpcResult(
            ResCode::UNAUTHORIZED_WORKER,
            "Worker name too long! (max " STRM(MAX_WORKER_NAME_LEN) " chars)");
    }

    given_addr = worker_full.substr(0, split);
    worker = worker_full.substr(split + 1, worker_full.size() - 1);
    ValidateAddressRes va_res;

    // bool oldAddress = redis_manager.DoesAddressExist(given_addr,
    // va_res.valid_addr);

    // if (!oldAddress)
    // {
    if (!daemon_manager.ValidateAddress(va_res, httpParser, given_addr))
    {
        return RpcResult(ResCode::UNAUTHORIZED_WORKER,
                         "Failed to validate address!");
    }

    if (!va_res.is_valid)
    {
        return RpcResult(ResCode::UNAUTHORIZED_WORKER,
                         fmt::format("Invalid address {}!", given_addr));
    }

    isIdentity = va_res.valid_addr[0] == 'i';

    if (isIdentity)
    {
        if (given_addr == va_res.valid_addr)
        {
            // we were given an identity address (i not @), get the id@
            GetIdentityRes id_res;
            if (!daemon_manager.GetIdentity(id_res, httpParser, given_addr))
            {
                Logger::Log(LogType::Critical, LogField::Stratum,
                            "Authorize RPC (getidentity) failed!");

                return RpcResult(
                    ResCode::UNAUTHORIZED_WORKER,
                    fmt::format("Server error: Failed to get id! {}", given_addr));
            }

            id_tag = id_res.name;
        }
        else
        {
            // we were given an id@
            id_tag = std::string(given_addr);
        }
    }
    // }

    std::string worker_full_str =
        fmt::format("{}.{}", va_res.valid_addr, worker);

    cli->SetAddress(worker_full_str, va_res.valid_addr);

    // string-views to non-local string
    bool added_to_db = stats_manager.AddWorker(
        cli->GetAddress(), cli->GetFullWorkerName(),
        va_res.script_pub_key, std::time(nullptr), id_tag);

    if (!added_to_db)
    {
        return RpcResult(ResCode::UNAUTHORIZED_WORKER,
                         "Failed to add worker to database!");
    }
    cli->SetAuthorized();

    Logger::Log(LogType::Info, LogField::Stratum,
                "Authorized worker: {}, address: {}, id: {}", worker,
                va_res.valid_addr, id_tag);

    return RpcResult(ResCode::OK);
}

// https://zips.z.cash/zip-0301#mining-submit
RpcResult StratumServerZec::HandleSubmit(StratumClient *cli, WorkerContext *wc,
                                    simdjson::ondemand::array &params)
{
    using namespace simdjson;
    // parsing takes 0-1 us
    ShareZec share;
    std::string parse_error = "";

    const auto end = params.end();
    auto it = params.begin();
    error_code error;

    if (it == end || (error = (*it).get_string().get(share.worker)))
    {
        parse_error = "Bad worker.";
    }
    else if (++it == end || (error = (*it).get_string().get(share.jobId)) ||
             share.jobId.size() != JOBID_SIZE * 2)
    {
        parse_error = "Bad job id.";
    }
    else if (++it == end || (error = (*it).get_string().get(share.time)) ||
             share.time.size() != JOBID_SIZE * 2)
    {
        parse_error = "Bad time.";
    }
    else if (++it == end || (error = (*it).get_string().get(share.nonce2)) ||
             share.nonce2.size() != EXTRANONCE2_SIZE * 2)
    {
        parse_error = "Bad nonce2.";
    }
    else if (++it == end || (error = (*it).get_string().get(share.solution)) ||
             share.solution.size() !=
                 (SOLUTION_SIZE + SOLUTION_LENGTH_SIZE) * 2)
    {
        parse_error = "Bad solution.";
    }

    if (!parse_error.empty())
    {
        Logger::Log(LogType::Critical, LogField::Stratum,
                    "Failed to parse submit: {}", parse_error);
        return RpcResult(ResCode::UNKNOWN, parse_error);
    }

    return HandleShare(cli, wc, share);
}

void StratumServerZec::UpdateDifficulty(StratumClient *cli)
{
    uint32_t diffBits = DiffToBits(cli->GetDifficulty());
    uint256 diff256;
    arith_uint256 arith256 = UintToArith256(diff256).SetCompact(diffBits);

    char request[512];
    int len = fmt::format_to_n(
        request, sizeof(request),
        "{{\"id\":null,\"method\":\"mining.set_difficulty\",\"params\":[\"{}\"]}}\n",
        arith256.GetHex()).size;
    // arith256.GetHex().c_str());

    SendRaw(cli->sock, request, len);

    Logger::Log(LogType::Debug, LogField::Stratum,
                "Set difficulty for {} to {}", cli->GetFullWorkerName(),
                arith256.GetHex());
}

#endif