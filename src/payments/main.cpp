#include <errno.h>
#include <simdjson/simdjson.h>

#include "config_parser.hpp"
#include "daemon_manager_t.hpp"
#include "fmt/format.h"
#include "redis_payment.hpp"

using enum Prefix;

static constexpr std::string_view logger_field = "PaymentManager";
const Logger<logger_field> logger;

// on each matured pool block: check if the updated
// balances meets the payout threshold if yes
// put miner on pending, stop waiting for replies if its time for
// payout

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        logger.Log<LogType::Critical>("Not enough parameters.");
        return -1;
    }

    simdjson::ondemand::parser parser;
    Logger<config_field_str> conf_logger;
    CoinConfig coinConfig;
    simdjson::padded_string json = simdjson::padded_string::load(argv[1]);
    ParseCoinConfig(json, coinConfig, conf_logger);

    if (coinConfig.payment_interval_seconds <= 0)
    {
        logger.Log<LogType::Critical>("No payment interval.");
        return -1;
    }

    const PersistenceLayer rm(coinConfig);
    PersistenceBlock persistence_block(rm);
    daemon_manager_t daemon_manager(coinConfig.payment_rpcs);

    persistence_block.SubscribeToMaturityChannel();
    time_t curtime = time(nullptr);
    time_t next_payment = curtime + coinConfig.payment_interval_seconds -
                          curtime % coinConfig.payment_interval_seconds;

    while (true)
    {
        auto [res, rep] = persistence_block.GetOneReply(next_payment - curtime);

        // payout time!
        if (res == REDIS_ERR)
        {
            // REDIS_ERR_TIMEOUT is for windows...
            if (RedisManager::GetError() != REDIS_ERR_IO || errno != EAGAIN)
            {
                logger.Log<LogType::Error>("Unexpected redis error, code: {}",
                                           RedisManager::GetError());
                return -1;
            }

            // timeout reached, payment time!
            logger.Log<LogType::Info>("Payment time! {}", next_payment);

            // immediate payment time!
            std::vector<Payee> payees;
            if (!persistence_block.LoadUnpaidRewards(payees))
            {
                logger.Log<LogType::Error>("Failed to get unpaid rewards");
                return -2;
            }
            logger.Log<LogType::Info>("Payees peding payment: {}",
                                      payees.size());

            // min fee
            // substract fees
            PayoutInfo payout_info;
            payout_info.tx_fee = 10000000000;
            payout_info.time = next_payment;

            if (payees.empty()) continue;

            const uint64_t individual_fee = payout_info.tx_fee / payees.size();
            uint64_t total = 0;
            for (auto& [_id, amount, addr] : payees)
            {
                int64_t amount_txfeed = amount - individual_fee;
                total += amount_txfeed;
                amount = amount_txfeed;
            }


            TransferResCn transfer_res;
            if (!daemon_manager.Transfer(transfer_res, payees, payout_info.tx_fee,
                                         parser))
            {
                logger.Log<LogType::Info>("Failed to transfer funds!");
            }
            payout_info.txid = transfer_res.txid;
            persistence_block.AddPayout(payout_info, payees, total);

            next_payment += coinConfig.payment_interval_seconds;
        }
        else
        {
            // pool block matured
        }

        // free consumed message
    }

    return 0;
}