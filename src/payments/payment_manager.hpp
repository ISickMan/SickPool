#ifndef PAYMENT_MANAGER_HPP
#define PAYMENT_MANAGER_HPP
#include <deque>
#include <unordered_map>

#include "block_template.hpp"
#include "daemon_manager_t.hpp"
#include "logger.hpp"
#include "redis_manager.hpp"
#include "round_manager.hpp"
#include "round_share.hpp"
#include "stats.hpp"
#include "transaction.hpp"

class RoundManager;
class RedisManager;

enum class PaymentStatus
{
    PENDING_INDEPENDENT,
    BROADCASTED
};

class PaymentManager
{
   public:
    PaymentManager(RedisManager* rm, daemon_manager_t* dm,
                   const std::string& pool_addr, int payout_age_s,
                   int64_t minimum_payout_threshold);

    static bool GetRewardsPPLNS(round_shares_t& miner_shares,
                                        const std::span<Share> shares,
                                        const int64_t block_reward,
                                        const double n, double fee);
    static bool GetRewardsPROP(round_shares_t& miner_shares,
                               int64_t block_reward,
                               const efforts_map_t& miner_efforts,
                               double total_effort, double fee);

    static uint32_t payment_counter;
    static int payout_age_seconds;
    static int64_t last_payout_ms;
    static int64_t minimum_payout_threshold;

    // std::unique_ptr<PaymentInfo> pending_payment;
    // std::unique_ptr<PaymentInfo> finished_payment;

    // void UpdatePayouts(RoundManager* round_manager, int64_t curtime_ms);

   private:
    bool GeneratePayoutTx(std::vector<uint8_t>& bytes,
                          const payees_info_t& rewards);
    // void ResetPayment();
    static constexpr std::string_view field_str = "PaymentManager";
    static Logger<field_str> logger;
    RedisManager* redis_manager;
    daemon_manager_t* daemon_manager;
    std::string pool_addr;
    // block id -> block height, maturity time, pending to be paid
    simdjson::ondemand::parser parser;
};
#endif