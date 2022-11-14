#ifndef ROUND_SHARE_HPP_
#define ROUND_SHARE_HPP_

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include "transaction.hpp"
#include "block_template.hpp"
#include "stats.hpp"

using reward_map_t = std::vector<std::pair<std::string, int64_t>>;

#pragma pack(push, 1)

struct RoundReward
{
    double effort;
    double share;
    int64_t reward;
};
#pragma pack(pop)
struct PaymentSettings
{
    int64_t threshold;
    bool pool_block_only;
};

struct PayeeInfo
{
    int64_t amount;
    std::vector<uint8_t> script_pub_key;
    PaymentSettings settings;
};

// struct PaymentInfo{
//     explicit PaymentInfo(uint32_t id) : id(id) {}
//     uint32_t id;
//     int64_t total_paid = 0;
//     reward_map_t rewards;
//     std::string raw_transaction_hex;
//     char block_hash_hex[HASH_SIZE_HEX];
//     TransactionData td;
//     transaction_t tx;
// };

#pragma pack(push, 1)
struct FinishedPayment
{
    uint32_t id;
    // char hash_hex[HASH_SIZE_HEX];
    int64_t total_paid_amount;
    int64_t time_ms;
    int64_t fee;
    size_t total_payees;
};

// struct UserPayment
// {
//     uint32_t id;
//     char hash_hex[HASH_SIZE_HEX];
//     int64_t amount;
//     int64_t time;
// };
#pragma pack(pop)

using round_shares_t = std::unordered_map<MinerIdHex, RoundReward>;

#endif