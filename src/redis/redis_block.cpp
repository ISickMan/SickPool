#ifndef REDIS_BLOCK_HPP
#define REDIS_BLOCK_HPP

#include "redis_manager.hpp"

using enum Prefix;

void RedisManager::AppendAddBlockSubmission(
    const ExtendedSubmission *submission)
{
    using namespace std::string_view_literals;

    uint32_t block_id = submission->number;
    auto chain = std::string(submission->chain_sv);
    std::string block_id_str = std::to_string(block_id);
    std::string_view block_id_sv(block_id_str);

    {
        RedisTransaction add_block_tx(this);

        // serialize the block submission to save space and net
        // bandwidth, as the indexes are added manually anyway no need for
        // hash
        AppendCommand(
            {"SET"sv, fmt::format("{}:{}", block_id, PrefixKey<BLOCK>()),
             std::string_view((char *)submission, sizeof(BlockSubmission))});
        /* sortable indexes */
        // block no. and block time will always be same order
        // so only one index is required to sort by either of them
        // (block num value is smaller)
        AppendCommand({"ZADD"sv, PrefixKey<BLOCK, INDEX, NUMBER>(),
                       std::to_string(submission->number), block_id_sv});

        AppendCommand({"ZADD"sv, PrefixKey<BLOCK, INDEX, REWARD>(),
                       std::to_string(submission->block_reward), block_id_sv});

        AppendCommand({"ZADD"sv, PrefixKey<BLOCK, INDEX, DIFFICULTY>(),
                       std::to_string(submission->difficulty), block_id_sv});

        AppendCommand({"ZADD"sv, PrefixKey<BLOCK, INDEX, EFFORT>(),
                       std::to_string(submission->effort_percent),
                       block_id_sv});

        AppendCommand({"ZADD"sv, PrefixKey<BLOCK, INDEX, DURATION>(),
                       std::to_string(submission->duration_ms), block_id_sv});

        /* non-sortable indexes */
        AppendCommand({"SADD"sv,
                       fmt::format("{}:{}", PrefixKey<BLOCK, INDEX, CHAIN>(),
                                   submission->chain_sv),
                       block_id_sv});

        AppendCommand(
            {"SADD"sv, PrefixKey<BLOCK, INDEX, TYPE, POW>(), block_id_sv});

        AppendCommand({"SADD"sv,
                       fmt::format("{}:{}", PrefixKey<BLOCK, INDEX, SOLVER>(),
                                   submission->miner_sv),
                       block_id_sv});

        /* other block statistics */
        AppendTsAdd(PrefixKey<BLOCK, STATS, EFFORT_PERCENT>(),
                    submission->time_ms, submission->effort_percent);

        // block number is written on interval.
        // AppendTsAdd(PrefixKey<BLOCK, STATS, DURATION>(), submission->time_ms,
        //             submission->duration_ms);

    }
}

bool RedisManager::UpdateBlockConfirmations(std::string_view block_id,
                                            int32_t confirmations)
{
    using namespace std::string_view_literals;
    std::scoped_lock lock(rc_mutex);
    // redis bitfield uses be so gotta swap em
    return Command({"BITFIELD"sv,
                    fmt::format("{}:{}", PrefixKey<BLOCK>(), block_id), "SET"sv,
                    "i32"sv, "0"sv, std::to_string(bswap_32(confirmations))})
               .get() == nullptr;
}

bool RedisManager::LoadImmatureBlocks(
    std::vector<std::unique_ptr<ExtendedSubmission>> &submissions)

{
    auto reply =
        Command({"KEYS", fmt::format("{}*", PrefixKey<IMMATURE, REWARD>())});

    for (int i = 0; i < reply->elements; i++)
    {
        std::string_view block_id(reply->element[i]->str,
                                  reply->element[i]->len);
        block_id =
            block_id.substr(block_id.find_last_of(":") + 1, block_id.size());

        auto block_reply = Command(
            {"GET", fmt::format("{}:{}", PrefixKey<BLOCK>(), block_id)});

        if (block_reply->type != REDIS_REPLY_STRING)
        {
            continue;
        }

        auto submission = (BlockSubmission *)block_reply->str;
        std::unique_ptr<ExtendedSubmission> extended =
            std::make_unique<ExtendedSubmission>(*submission);

        submissions.emplace_back(std::move(extended));
    }

    return true;
}
#endif