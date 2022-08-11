#include "redis_manager.hpp"

RedisManager::RedisManager(const std::string &ip, int port)
    : rc(redisConnect(ip.c_str(), port))
{
    using namespace std::string_view_literals;

    if (rc->err)
    {
        Logger::Log(LogType::Critical, LogField::Redis,
                    "Failed to connect to redis: {}", rc->errstr);
        redisFree(rc);
        throw std::runtime_error("Failed to connect to redis");
    }

    // AppendCommand({
    //     "round_effort_percent"sv,
    // });
    AppendCommand(
        {"TS.CREATE"sv, fmt::format("{}:{}", WORKER_COUNT_KEY, "pool")});
    AppendCommand(
        {"TS.CREATE"sv, fmt::format("{}:{}", MINER_COUNT_KEY, "pool")});
    AppendCommand({"TS.CREATE"sv, fmt::format("{}:{}", HASHRATE_KEY, "pool"),
                   "RETENTION"sv, "0"sv, "LABELS"sv, "type"sv,
                   "pool-hashrate"sv});

    if (!GetReplies())
    {
        Logger::Log(LogType::Critical, LogField::Redis,
                    "Failed to connect to add pool timeserieses", rc->errstr);
        throw std::runtime_error("Failed to connect to add pool timeserieses");
    }
}

RedisManager::~RedisManager() { redisFree(rc); }

// TODO: think of smt else
bool RedisManager::DoesAddressExist(std::string_view addrOrId,
                                    std::string &valid_addr)
{
    std::scoped_lock lock(rc_mutex);

    redisReply *reply;
    bool isId = addrOrId.ends_with("@");

    if (isId)
    {
        reply =
            (redisReply *)redisCommand(rc, "TS.QUERYINDEX id=%b type=hashrate",
                                       addrOrId.data(), addrOrId.size());
    }
    else
    {
        reply = (redisReply *)redisCommand(
            rc, "TS.QUERYINDEX address=%b type=hashrate", addrOrId.data(),
            addrOrId.size());
    }

    bool exists = reply && reply->elements == 1;
    bool res = false;
    if (exists)
    {
        valid_addr =
            reply->element[0]->str + sizeof(COIN_SYMBOL ":hashrate:") - 1;
        res = true;
    }
    freeReplyObject(reply);
    return res;
}

int RedisManager::AddNetworkHr(std::string_view chain, int64_t time, double hr)
{
    std::scoped_lock lock(rc_mutex);

    std::string key = fmt::format("{}:{}", chain, "network_difficulty");
    AppendTsAdd(std::string_view(key), time, hr);
    return GetReplies();
}

bool RedisManager::UpdateImmatureRewards(std::string_view chain,
                                         uint32_t block_num,
                                         int64_t matured_time, bool matured)
{
    using namespace std::string_view_literals;
    using namespace std::string_literals;
    std::scoped_lock lock(rc_mutex);

    auto reply = Command(
        {"HGETALL"sv, fmt::format("{}:{}", IMMATURE_REWARDS, block_num)});

    double matured_funds = 0;
    // either mature everything or nothing
    {
        RedisTransaction update_rewards_tx(this);

        for (int i = 0; i < reply->elements; i += 2)
        {
            double minerReward = 0;
            std::string_view addr(reply->element[i]->str,
                                  reply->element[i]->len);

            RoundShare *miner_share = (RoundShare *)reply->element[i + 1]->str;

            // if a block has been orphaned only remove the immature
            // if there are sub chains add chain:

            if (!matured)
            {
                miner_share->reward = 0;
            }

            // used to show round share statistics
            uint8_t
                round_share_block_num[sizeof(miner_share) + sizeof(block_num)];
            memcpy(round_share_block_num, miner_share, sizeof(miner_share));
            memcpy(round_share_block_num + sizeof(miner_share), &block_num,
                   sizeof(block_num));

            std::string_view mature_shares_key =
                fmt::format("mature-shares:{}", addr);
            AppendCommand({"LPUSH"sv, mature_shares_key,
                           std::string_view((char *)round_share_block_num,
                                            sizeof(round_share_block_num))});

            AppendCommand({"LTRIM"sv, mature_shares_key, "0"sv,
                           std::to_string(ROUND_SHARES_LIMIT)});

            std::string reward_str = std::to_string(miner_share->reward);
            std::string_view reward_sv(reward_str);

            // // used for payments
            // AppendCommand({"HSET"sv, fmt::format("mature-rewards:{}", addr),
            //                std::to_string(block_num), reward_sv});

            AppendCommand({"ZINCRBY"sv,
                           fmt::format("solver-index:{}", MATURE_BALANCE_KEY),
                           reward_sv, addr});

            AppendCommand({"HINCRBY"sv, fmt::format("solver:{}", addr),
                           MATURE_BALANCE_KEY, reward_sv});

            matured_funds += miner_share->reward;
        }

        // we pushed it to mature round shares list
        AppendCommand(
            {"UNLINK"sv, fmt::format("{}:{}", IMMATURE_REWARDS, block_num)});
    }

    Logger::Log(LogType::Info, LogField::Redis, "{} funds have matured!",
                matured_funds);
    return GetReplies();
}

bool RedisManager::LoadUnpaidRewards(
    std::vector<std::pair<std::string, PayeeInfo>> &rewards,
    const efforts_map_t &efforts, std::mutex *efforts_mutex)
{
    using namespace std::string_view_literals;

    rewards.reserve(efforts.size());

    {
        RedisTransaction tx(this);
        std::scoped_lock lock(*efforts_mutex);

        for (const auto &[addr, _] : efforts)
        {
            AppendCommand({"HMGET"sv, fmt::format("solver:{}", addr),
                           MATURE_BALANCE_KEY, PAYOUT_THRESHOLD_KEY,
                           SCRIPT_PUB_KEY_KEY});
            rewards.emplace_back(addr, 0);
        }
    }

    redis_unique_ptr reply;
    if (!GetReplies(&reply)) return false;

    for (int i = 0; i < reply->elements; i += 3)
    {
        rewards[i].second.amount =
            std::strtoll(reply->element[i]->str, nullptr, 10);

        rewards[i].second.settings.threshold =
            std::strtoll(reply->element[i + 1]->str, nullptr, 10);

        const auto script_len = reply->element[i + 2]->len;

        rewards[i].second.script_pub_key.reserve(script_len / 2);
        Unhexlify(rewards[i].second.script_pub_key.data(),
                  reply->element[i + 2]->str, script_len * 2);
    }
    return true;
}

void RedisManager::AppendTsAdd(std::string_view key_name, int64_t time,
                               double value)
{
    using namespace std::string_view_literals;
    AppendCommand(
        {"TS.ADD"sv, key_name, std::to_string(time), std::to_string(value)});
}

void RedisManager::AppendTsCreate(std::string_view key, std::string_view prefix,
                                  std::string_view type,
                                  std::string_view address, std::string_view id,
                                  uint64_t retention_ms)
{
    using namespace std::string_view_literals;
    AppendCommand({"TS.CREATE"sv, key, "RETENTION"sv,
                   std::to_string(retention_ms), "LABELS"sv, "type"sv, type,
                   "prefix"sv, prefix, "address"sv, address, "identity"sv, id});
}

bool RedisManager::AddNewMiner(std::string_view address,
                               std::string_view worker_full,
                               std::string_view id_tag,
                               std::string_view script_pub_key, int64_t curtime)
{
    using namespace std::string_view_literals;

    std::scoped_lock lock(rc_mutex);

    std::string addr_lowercase(address);
    std::transform(addr_lowercase.begin(), addr_lowercase.end(),
                   addr_lowercase.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    {
        RedisTransaction add_worker_tx(this);

        // 100% new, as we loaded all existing
        // miners
        auto chain = std::string_view(COIN_SYMBOL);

        // create worker count ts
        AppendTsCreate(fmt::format("{}:{}", WORKER_COUNT_KEY, address),
                       "miner"sv, WORKER_COUNT_KEY, address, id_tag,
                       StatsManager::hashrate_ttl_seconds * 1000);

        // reset worker count
        AppendTsAdd(fmt::format("{}:{}", WORKER_COUNT_KEY, address), curtime,
                    0);

        // add to pool miner counter
        AppendCommand({"TS.INCRBY"sv,
                       fmt::format("{}:{}", MINER_COUNT_KEY, "pool"), "1"sv});

        std::string_view curtime_sv = std::to_string(curtime);
        // don't override if it exists
        AppendCommand({"ZADD"sv, fmt::format("solver-index:{}", JOIN_TIME_KEY),
                       "NX", curtime_sv, address});

        // reset all indexes of new miner
        for (std::string_view index :
             {WORKER_COUNT_KEY, HASHRATE_KEY, MATURE_BALANCE_KEY})
        {
            AppendCommand({"ZADD"sv, fmt::format("solver-index:{}", index), "0",
                           address});
        }

        AppendCommand({"HSET"sv, fmt::format("solver:{}", address),
                       IDENTITY_KEY, id_tag, JOIN_TIME_KEY, curtime_sv,
                       SCRIPT_PUB_KEY_KEY, script_pub_key, PAYOUT_THRESHOLD_KEY,
                       std::to_string(PaymentManager::minimum_payout_threshold),
                       HASHRATE_KEY, "0"sv, MATURE_BALANCE_KEY, "0"sv,
                       IMMATURE_BALANCE_KEY, "0"sv, WORKER_COUNT_KEY, "0"sv});

        AppendCommand({"HSET"sv, ADDRESS_MAP_KEY, addr_lowercase, address});
        if (!id_tag.empty())
        {
            AppendCommand({"HSET"sv, ADDRESS_MAP_KEY, id_tag, address});
        }

        // set round effort to 0
        AppendSetMinerEffort(chain, TOTAL_EFFORT_KEY, "pow", 0);
        AppendSetMinerEffort(chain, TOTAL_EFFORT_KEY, "pos", 0);

        AppendCreateStatsTs(address, id_tag, "miner"sv);
    }

    return GetReplies();
}

bool RedisManager::AddNewWorker(std::string_view address,
                                std::string_view worker_full,
                                std::string_view id_tag)
{
    using namespace std::string_view_literals;

    std::scoped_lock lock(rc_mutex);

    {
        RedisTransaction add_worker_tx(this);

        AppendCreateStatsTs(worker_full, id_tag, "worker"sv);
        AppendUpdateWorkerCount(address, 1);
    }

    return GetReplies();
}

void RedisManager::AppendUpdateWorkerCount(std::string_view address, int amount)
{
    using namespace std::string_view_literals;
    std::string amount_str = std::to_string(amount);

    AppendCommand({"ZINCRBY"sv,
                   fmt::format("solver-index:{}", WORKER_COUNT_KEY), address,
                   amount_str});

    AppendCommand({"HINCRBY"sv, fmt::format("solver:{}", address),
                   WORKER_COUNT_KEY, amount_str});

    AppendCommand({"TS.INCRBY"sv,
                   fmt::format("{}:{}", WORKER_COUNT_KEY, address),
                   amount_str});

    AppendCommand({"TS.INCRBY"sv,
                   fmt::format("{}:{}", WORKER_COUNT_KEY, "pool"), amount_str});
}

bool RedisManager::PopWorker(std::string_view address)
{
    std::scoped_lock lock(rc_mutex);

    AppendUpdateWorkerCount(address, -1);

    return GetReplies();
}

void RedisManager::AppendHset(std::string_view key, std::string_view field,
                              std::string_view val)
{
    AppendCommand({"HSET", key, field, val});
}

bool RedisManager::hset(std::string_view key, std::string_view field,
                        std::string_view val)
{
    AppendHset(key, field, val);
    return GetReplies();
}

std::string RedisManager::hget(std::string_view key, std::string_view field)
{
    using namespace std::string_view_literals;
    auto reply = Command({"HGET"sv, key, field});

    if (reply && reply->type == REDIS_REPLY_STRING)
    {
        // returns 0 if failed
        return std::string(reply->str, reply->len);
    }
    return std::string();
}

void RedisManager::AppendCreateStatsTs(std::string_view addrOrWorker,
                                       std::string_view id,
                                       std::string_view prefix)
{
    using namespace std::literals;

    std::string_view address = addrOrWorker.substr(0, ADDRESS_LEN);

    auto retention = StatsManager::hashrate_ttl_seconds * 1000;

    for (auto key_type : {HASHRATE_KEY, "hashrate:average"sv, "shares:valid"sv,
                          "shares:stale"sv, "shares:invalid"sv})
    {
        auto key = fmt::format("{}:{}:{}", key_type, prefix, addrOrWorker);
        AppendTsCreate(key, prefix, key_type, address, id, retention);
    }
}