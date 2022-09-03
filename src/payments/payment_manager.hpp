#ifndef PAYMENT_MANAGER_HPP
#define PAYMENT_MANAGER_HPP
#include <deque>
#include <unordered_map>

#include "daemon_manager.hpp"
#include "logger.hpp"
#include "redis_manager.hpp"
#include "round_manager.hpp"
#include "round_share.hpp"
#include "stats.hpp"
#include "block_template.hpp"

class RoundManager;
class RedisManager;

class PaymentManager
{
   public:
    PaymentManager(RedisManager* rm, DaemonManager* dm, const std::string& pool_addr, int payout_age_s,
                   int64_t minimum_payout_threshold);
    static bool GetRewardsProp(round_shares_t& miner_shares,
                               int64_t block_reward,
                               efforts_map_t& miner_efforts,
                               double total_effort, double fee);
    bool GeneratePayout(RoundManager* round_manager, int64_t time_now);

    static uint32_t payment_counter;
    static int payout_age_seconds;
    static int64_t last_payout_ms;
    static int64_t minimum_payout_threshold;

    bool payment_included = false;
    std::unique_ptr<PendingPayment> pending_tx;

    bool ShouldIncludePayment(std::string_view prevblockhash);
    void UpdatePayouts(RoundManager* round_manager, int64_t curtime_ms);

   private:
    bool GeneratePayoutTx(
        std::vector<uint8_t>& bytes,
        const std::vector<std::pair<std::string, PayeeInfo>>& rewards);
    // void ResetPayment();
    RedisManager* redis_manager;
    DaemonManager* daemon_manager;
    std::string pool_addr;
    // block id -> block height, maturity time, pending to be paid
    simdjson::ondemand::parser parser;
};
#endif