#include "block_submission_manager.hpp"

uint32_t SubmissionManager::block_number;

void SubmissionManager::CheckImmatureSubmissions()
{
    using namespace simdjson;
    std::scoped_lock lock(blocks_lock);

    std::string resBody;

    Logger::Log(LogType::Info, LogField::SubmissionManager,
                "Checking confirmations for {} blocks", immature_block_submissions.size());

    for (int i = 0; i < immature_block_submissions.size(); i++)
    {
        const auto& submission = immature_block_submissions[i];
        auto hashHex = std::string_view((char*)submission->hash_hex,
                                        sizeof(submission->hash_hex));
        auto chain = submission->chain_sv;

        int res = daemon_manager->SendRpcReq(
            resBody, 1, "getblockheader", DaemonRpc::GetParamsStr(hashHex));

        if (res != 200)
        {
            Logger::Log(LogType::Info, LogField::SubmissionManager,
                        "Failed to get confirmations for block {}, parse "
                        "error: {}, http code: {}",
                        hashHex, resBody, res);
            continue;
        }

        int32_t confirmations = -1;
        try
        {
            ondemand::document doc = httpParser.iterate(
                resBody.data(), resBody.size(), resBody.capacity());

            confirmations = (int32_t)doc["result"]["confirmations"].get_int64();
        }
        catch (const simdjson_error& err)
        {
            confirmations = 0;
            Logger::Log(LogType::Info, LogField::SubmissionManager,
                        "Failed to get confirmations for block {}, parse "
                        "error: {}",
                        hashHex, err.what());
            continue;
        }

        redis_manager->UpdateBlockConfirmations(
            std::string_view(std::to_string(submission->number)),
            confirmations);

        Logger::Log(LogType::Info, LogField::SubmissionManager,
                    "Block {} has {} confirmations", hashHex, confirmations);

        int64_t confirmation_time = GetCurrentTimeMs();

        if (confirmations == -1)
        {
            Logger::Log(LogType::Info, LogField::SubmissionManager,
                        "Block {} has been orphaned! :(", hashHex);

            redis_manager->UpdateImmatureRewards(chain, submission->number,
                                                 confirmation_time, false);
            immature_block_submissions.erase(
                immature_block_submissions.begin() + i);
            i--;
        }
        else if (confirmations > COINBASE_MATURITY)
        {
            Logger::Log(LogType::Info, LogField::SubmissionManager,
                        "Block {} has matured!", hashHex);

            //     int64_t duration_ms = confirmation_time - last_matured_time;
            //     redis_manager->AddStakingPoints(chain, duration_ms);
            // }

            redis_manager->UpdateImmatureRewards(chain, submission->number,
                                                 confirmation_time, true);

            immature_block_submissions.erase(
                immature_block_submissions.begin() + i);

            last_matured_time = confirmation_time;
            i--;
        }
    }
}

bool SubmissionManager::AddImmatureBlock(
    std::unique_ptr<ExtendedSubmission> submission, const double pow_fee)
{
    std::scoped_lock lock(blocks_lock);

    if (submission->block_type == BlockType::POW ||
        submission->block_type == BlockType::POW_PAYMENT)
    {
        round_manager_pow->CloseRound(submission.get(), pow_fee);
    }
    else if (submission->block_type == BlockType::POS)
    {
        round_manager_pos->CloseRound(submission.get(), pow_fee);
    }
    // TODO: incrblockcount somewhere
    //  redis_manager->IncrBlockCount();
    block_number++;

    Logger::Log(
        LogType::Info, LogField::SubmissionManager,
        "Added new block submission: \n"
        "┌{0:─^{12}}┐\n"
        "│{1: ^{12}}│\n"
        "│{2: <{12}}│\n"
        "│{3: <{12}}│\n"
        "│{4: <{12}}│\n"
        "│{5: <{12}}│\n"
        "│{6: <{12}}│\n"
        "│{7: <{12}}│\n"
        "│{8: <{12}}│\n"
        "│{9: <{12}}│\n"
        "│{10: <{12}}│\n"
        "│{11: <{12}}│\n"
        "└{0:─^{12}}┘\n",
        "", fmt::format("Block Submission #{}", submission->number),
        fmt::format("Type: {}", (int)submission->block_type),
        fmt::format("Reward: {}", submission->block_reward),
        fmt::format("Found time: {}", submission->time_ms),
        fmt::format("Duration (ms): {}", submission->duration_ms),
        fmt::format("Height: {}", submission->height),
        fmt::format("Difficulty: {}", submission->difficulty),
        fmt::format("Effort percent: {}", submission->effort_percent),
        fmt::format("Miner: {}",
                    std::string_view((char*)submission->miner, ADDRESS_LEN)),
        fmt::format("Worker: {}",
                    std::string((char*)submission->worker, MAX_WORKER_NAME_LEN)
                        .c_str()),
        fmt::format("Hash: {}", std::string_view((char*)submission->hash_hex,
                                                 HASH_SIZE_HEX)),
        70);

    Logger::Log(LogType::Info, LogField::SubmissionManager,
                "Closed round for block submission no {} (immature).",
                submission->number);
    immature_block_submissions.push_back(std::move(submission));

    return true;
}

bool SubmissionManager::SubmitBlock(std::string_view block_hex)
{
    std::string resultBody;
    int resCode = daemon_manager->SendRpcReq(
        resultBody, 1, "submitblock", DaemonRpc::GetParamsStr(block_hex));

    if (resCode != 200)
    {
        Logger::Log(LogType::Critical, LogField::SubmissionManager,
                    "Failed to send block submission, http code: {}, res: {}",
                    resCode, resultBody);
        return false;
    }

    try
    {
        using namespace simdjson;
        ondemand::document doc = httpParser.iterate(
            resultBody.data(), resultBody.size(), resultBody.capacity());

        ondemand::object res = doc.get_object();
        ondemand::value resultField = res["result"];
        ondemand::value errorField = res["error"];

        if (!errorField.is_null())
        {
            std::string_view res = errorField.get_string();
            Logger::Log(LogType::Critical, LogField::SubmissionManager,
                        "Block submission rejected, rpc error: {}", res);
            return false;
        }

        if (!resultField.is_null())
        {
            std::string_view result = resultField.get_string();
            Logger::Log(LogType::Critical, LogField::SubmissionManager,
                        "Block submission rejected, rpc result: {}", result);

            if (result == "inconclusive")
            {
                Logger::Log(
                    LogType::Warn, LogField::SubmissionManager,
                    "Submitted inconclusive block, waiting for result...");
                return true;
            }
            return false;
        }
    }
    catch (const simdjson::simdjson_error& err)
    {
        // Logger::Log(LogType::Critical, LogField::SubmissionManager,
        //             "Submit block response parse error: {}", err.what());
        return false;
    }

    return true;
}

// think of what happens if they deposit at the same time as the block was found