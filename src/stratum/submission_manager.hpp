#ifndef SUBMISSION_MANAGER_HPP
#define SUBMISSION_MANAGER_HPP

#include <simdjson/simdjson.h>
#include <memory>

#include "block_submission.hpp"
#include "daemon_manager.hpp"
#include "redis_manager.hpp"
#include "stats_manager.hpp"

class SubmissionManager
{
   public:
    SubmissionManager(RedisManager* redis_manager,
                      DaemonManager* daemon_manager, StatsManager* stats_manager)
        : redis_manager(redis_manager),
          daemon_manager(daemon_manager),
          block_number(redis_manager->GetBlockNumber())
    {
        Logger::Log(LogType::Info, LogField::SubmissionManager,
                    "Submission manager started, block number: %u",
                    block_number);
    }

    inline bool TrySubmit(const std::string_view chain,
                          const std::string_view block_hex)
    {
        bool added = false;
        for (int i = 0; i < submis_retries; ++i)
        {
            added = SubmitBlock(block_hex);
            if (added)
            {
                break;
            }
        }
        return added;
    }

    bool AddImmatureBlock(std::unique_ptr<BlockSubmission> submission);

    void CheckImmatureSubmissions(const double pow_fee);

   private:
    bool SubmitBlock(std::string_view block_hex); // TODO: make const when we wrapped rpc func, same for trysubmit
    const int submis_retries = 10;
    RedisManager* redis_manager;
    DaemonManager* daemon_manager;
    StatsManager* stats_manager;

    uint32_t block_number;
    simdjson::ondemand::parser httpParser;

    // pointer as it is not assignable for erase
    std::vector<std::unique_ptr<BlockSubmission>> immature_block_submissions;
};

#endif

//TODO: wrap all rpc methods we use