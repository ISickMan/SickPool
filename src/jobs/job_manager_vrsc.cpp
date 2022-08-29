#include "static_config.hpp"
#if COIN == VRSC
#include "job_manager_vrsc.hpp"

const job_t* JobManagerVrsc::GetNewJob(const std::string& json_template)
{
    using namespace simdjson;
    try
    {
        ondemand::document doc =
            jsonParser.iterate(json_template.data(), json_template.size(),
                               json_template.capacity());

        blockTemplate = BlockTemplateVrsc();
        ondemand::object res = doc["result"].get_object();

        // this must be in the order they appear in the result for simdjson
        blockTemplate.version =
            static_cast<uint32_t>(res["version"].get_int64());
        blockTemplate.prev_block_hash = res["previousblockhash"].get_string();
        blockTemplate.finals_root_hash = res["finalsaplingroothash"].get_string();
        blockTemplate.solution = res["solution"].get_string();
        // can't iterate after we get the string_view
        ondemand::array txs = res["transactions"].get_array();

        int64_t additional_fee = 0;
        bool includes_payment =
            payment_manager->ShouldIncludePayment(blockTemplate.prev_block_hash);
        if (includes_payment)
        {
            std::string& payment_hash_hex =
                payment_manager->pending_tx->info.tx_hash_hex;

            if (!payment_hash_hex.empty() &&
                payment_hash_hex == blockTemplate.prev_block_hash)
            {
                payment_manager->payment_included = true;
            }
            else
            {
                // either we have yet to create jobs with this payment or a
                // block with this payment was orphaned.
                TransactionData td(payment_hash_hex);
                payment_hash_hex.reserve(HASH_SIZE_HEX);
                Hexlify(payment_hash_hex.data(), td.hash, HASH_SIZE_HEX);

                blockTemplate.tx_list.AddTxData(td);
                additional_fee += payment_manager->pending_tx->info.fee;
            }
        }

        for (auto tx : txs)
        {
            std::string_view tx_data_hex = tx["data"].get_string();
            std::string_view tx_hash_hex = tx["hash"].get_string();
            TransactionData td(tx_data_hex, tx_hash_hex);
            td.fee = tx["fee"].get_double();

            // std::cout << "tx data: " << td.data_hex << std::endl;

            int txSize = td.data_hex.size() / 2;
            td.data = std::vector<unsigned char>(txSize);
            Unhexlify(td.data.data(), td.data_hex.data(), td.data_hex.size());
            Unhexlify(td.hash, tx_hash_hex.data(), tx_hash_hex.size());  // hash
            // is reversed std::reverse(td.hash, td.hash + 32);

            if (!blockTemplate.tx_list.AddTxData(td))
            {
                Logger::Log(LogType::Warn, LogField::JobManager,
                            "Block template is full! block size is {} bytes",
                            blockTemplate.tx_list.byteCount);
                break;
            }
        }

        blockTemplate.coinbase_hex = res["coinbasetxn"]["data"].get_string();
        blockTemplate.coinbase_value =
            res["coinbasetxn"]["coinbasevalue"].get_int64();

        blockTemplate.target = res["target"].get_string();
        blockTemplate.min_time = res["mintime"].get_int64();
        std::string_view bits_sv = res["bits"].get_string();
        blockTemplate.bits = HexToUint(bits_sv.data(), BITS_SIZE * 2);

        blockTemplate.height =
            static_cast<uint32_t>(res["height"].get_int64());

        TransactionData coinbaseTx = GetCoinbaseTxData(
            blockTemplate.coinbase_value + additional_fee, blockTemplate.height,
            blockTemplate.min_time, blockTemplate.coinbase_hex);

        // we need to hexlify here otherwise hex will be garbage
        char coinbaseHex[coinbaseTx.data.size() * 2];
        Hexlify(coinbaseHex, coinbaseTx.data.data(), coinbaseTx.data.size());
        coinbaseTx.data_hex =
            std::string_view(coinbaseHex, sizeof(coinbaseHex));
        blockTemplate.tx_list.AddCoinbaseTxData(coinbaseTx);

        std::string jobIdHex = fmt::format("{:08x}", job_count);

        job_t job(jobIdHex, blockTemplate, includes_payment);

        last_job_id_hex = jobIdHex;
        job_count++;

        return &jobs.emplace_back(std::move(job));
    }
    catch (const simdjson::simdjson_error& err)
    {
        Logger::Log(LogType::Critical, LogField::JobManager,
                    "Failed to parse block template: {}, json: {}", err.what(),
                    json_template);
    }
    return nullptr;
}
#endif