#include "payment_manager.hpp"

Logger<LogField::PaymentManager> PaymentManager::logger;

int PaymentManager::payout_age_seconds;
int64_t PaymentManager::minimum_payout_threshold;
int64_t PaymentManager::last_payout_ms;
uint32_t PaymentManager::payment_counter = 0;

PaymentManager::PaymentManager(RedisManager* rm, daemon_manager_t* dm,
                               const std::string& pool_addr, int payout_age_s,
                               int64_t min_threshold)
    : redis_manager(rm), daemon_manager(dm), pool_addr(pool_addr)
{
    PaymentManager::payout_age_seconds = payout_age_s;
    PaymentManager::minimum_payout_threshold = min_threshold;
}

bool PaymentManager::GetRewardsProp(round_shares_t& miner_shares,
                                    int64_t block_reward,
                                    efforts_map_t& miner_efforts,
                                    double total_effort, double fee)
{
    int64_t substracted_reward =
        static_cast<int64_t>(static_cast<double>(block_reward) * (1.f - fee));

    if (total_effort == 0)
    {
        logger.Log<LogType::Critical>(
                    "Round effort is 0!");
        return false;
    }
    miner_shares.reserve(miner_efforts.size());

    for (auto& [addr, effort] : miner_efforts)
    {
        RoundShare round_share;
        round_share.effort = effort;
        round_share.share = round_share.effort / total_effort;
        round_share.reward = static_cast<int64_t>(
            round_share.share * static_cast<double>(substracted_reward));
        miner_shares.emplace_back(addr, round_share);

        logger.Log<LogType::Info>(
                    "Miner round share: {}, effort: {}, share: {}, reward: "
                    "{}, total effort: {}",
                    addr, round_share.effort, round_share.share,
                    round_share.reward, total_effort);
    }

    return true;
}

bool PaymentManager::GeneratePayout(RoundManager* round_manager,
                                    int64_t time_now_ms)
{
    // add new payments
    std::vector<uint8_t> payout_bytes;
    std::vector<std::pair<std::string, PayeeInfo>> rewards;
    round_manager->LoadUnpaidRewards(rewards);

    for (const auto& [addr, info] : rewards)
    {
        logger.Log<LogType::Info>(
                    "Pending reward to {} -> {}, threshold: {}.", addr,
                    info.amount, info.settings.threshold);
    }

    if (!GeneratePayoutTx(payout_bytes, rewards))
    {
        return false;
    }

    char bytes_hex[payout_bytes.size() * 2];
    Hexlify(bytes_hex, payout_bytes.data(), payout_bytes.size());
    std::string_view unfunded_rawtx(bytes_hex, sizeof(bytes_hex));

    FundRawTransactionRes fund_res;
    // 0 fees
    if (!daemon_manager->FundRawTransaction(fund_res, parser, unfunded_rawtx, 0, pool_addr))
        {
            logger.Log<LogType::Info>(
                        "Failed to fund rawtransaction. {}", unfunded_rawtx);
            return false;
        }

    SignRawTransactionRes sign_res;
    if (!daemon_manager->SignRawTransaction(sign_res, parser, fund_res.hex))
    {
        logger.Log<LogType::Info>(
                    "Failed to sign rawtransaction.");
        return false;
    }

    pending_payment->raw_transaction_hex = sign_res.hex;
    pending_payment->td = TransactionData(pending_payment->raw_transaction_hex);

    return true;
}

bool PaymentManager::GeneratePayoutTx(
    std::vector<uint8_t>& bytes,
    const std::vector<std::pair<std::string, PayeeInfo>>& rewards)
{
    // inputs will be added in fundrawtransaction
    // change will be auto added in fundrawtransaction
    pending_payment = std::make_unique<PaymentInfo>(payment_counter);

    for (auto& [addr, reward_info] : rewards)
    {
        // check if passed threshold
        if (reward_info.amount < reward_info.settings.threshold ||
            reward_info.amount == 0)
        {
            logger.Log<LogType::Info>(
                        "Skipping {} in payment insufficient threshold: {}/{}",
                        addr, reward_info.amount,
                        reward_info.settings.threshold);
            continue;
        }

        // leave 50000 bytes for funding inputs and change and other
        // transactions in the block
        if (pending_payment->tx.GetTxLen() + 50000 > MAX_BLOCK_SIZE)
        {
            break;
        }
        pending_payment->rewards.emplace_back(addr, reward_info.amount);
        // allows us to support more than just p2pkh transaction
        pending_payment->tx.AddOutput(reward_info.script_pub_key,
                                      reward_info.amount);

        pending_payment->total_paid += reward_info.amount;
    }

    if (pending_payment->tx.vout.empty())
    {
        return false;
    }

    pending_payment->tx.GetBytes(bytes);

    return true;
}

void PaymentManager::UpdatePayouts(RoundManager* round_manager,
                                   int64_t curtime_ms)
{
    if (finished_payment)
    {
        // will be freed after here, since we moved
        auto complete_tx = std::move(finished_payment);

        redis_manager->AddPayout(complete_tx.get());

        last_payout_ms = curtime_ms;
        payment_counter++;

        finished_payment.release();

        logger.Log<
            LogType::Info>(
            "Payment has been included in block and added to database!");
    }

    // make sure there is no currently pending payment or an unprocessed
    // finished payment
    if (!pending_payment.get() && !finished_payment.get() &&
        curtime_ms > last_payout_ms + (payout_age_seconds * 1000))
    {
        if (GeneratePayout(round_manager, curtime_ms))
        {
            logger.Log<
                LogType::Info>(
                "Generated payout txid: {}, data: {}",
                std::string_view(pending_payment->td.hash_hex, HASH_SIZE_HEX),
                pending_payment->td.data_hex);
        }
        else
        {
            pending_payment.release();
            logger.Log<LogType::Info>(
                        "Failed to generate payout Tx.");
            // std::string_view(pending_tx->td.hash_hex, HASH_SIZE_HEX),
            // pending_tx->td.data_hex);
        }
    }
}