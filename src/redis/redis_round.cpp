#include "redis_manager.hpp"

void RedisManager::AppendSetMinerEffort(std::string_view chain,
                                        std::string_view miner,
                                        std::string_view type, double effort)
{
    AppendHset(fmt::format("{}:round:{}:effort", chain, type), miner,
               std::to_string(effort));
}
int64_t RedisManager::GetRoundTime(std::string_view chain,
                                   std::string_view type)
{
    std::scoped_lock lock(rc_mutex);

    return hgeti(fmt::format("{}:round:{}", chain, type), "start");
}

double RedisManager::GetRoundEffort(std::string_view chain,
                                    std::string_view type)
{
    std::scoped_lock lock(rc_mutex);

    return hgetd(fmt::format("{}:round:{}", chain, type), "total_effort");
}

bool RedisManager::SetRoundEstimatedEffort(std::string_view chain,
                                           std::string_view type, double effort)
{
    std::scoped_lock lock(rc_mutex);

    return hset(fmt::format("{}:round:{}", chain, type), "estimated",
                std::to_string(effort));
}

bool RedisManager::SetRoundStartTime(std::string_view chain,
                                     std::string_view type, int64_t val)
{
    std::scoped_lock lock(rc_mutex);

    return hset(fmt::format("{}:round:{}", chain, type), "start",
                std::to_string(val));
}
bool RedisManager::SetRoundEffort(std::string_view chain, std::string_view type,
                                  double effort)
{
    std::scoped_lock lock(rc_mutex);

    AppendSetRoundEffort(chain, type, effort);
    return GetReplies();
}

void RedisManager::AppendSetRoundEffort(std::string_view chain,
                                        std::string_view type, double effort)
{
    AppendHset(fmt::format("{}:round:{}", chain, type), "total_effort",
               std::to_string(effort));
}

// reset effort of miners + total round effort
bool RedisManager::ResetRoundEfforts(std::string_view chain,
                                     std::string_view type)
{
    std::scoped_lock lock(rc_mutex);

    // AppendCommand("DEL %s", fmt::format("{}:round:{}:effort", chain, type).c_str());
    AppendSetRoundEffort(chain, type, 0);
    return GetReplies();
}

bool RedisManager::AddRoundShares(
    std::string_view chain, const BlockSubmission *submission,
    const std::vector<std::pair<std::string, RoundShare>> &miner_shares)
{
    std::scoped_lock lock(rc_mutex);

    uint32_t height = submission->height;

    // redis transaction, so either all balances are added or none
    {
        RedisTransaction close_round_tx(this);

        for (const auto &miner_share : miner_shares)
        {
            AppendCommand("HSET immature-rewards:%u %b %b", submission->number,
                          miner_share.first.data(), miner_share.first.size(),
                          &miner_share.second, sizeof(RoundShare));

            AppendCommand("HINCRBY %b:balance-immature %b %" PRId64,
                          chain.data(), chain.size(), miner_share.first.data(),
                          miner_share.first.size(), miner_share.second.reward);
        }
    }

    return GetReplies();
}
