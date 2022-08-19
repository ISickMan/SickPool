#include <signal.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "static_config.hpp"

#include "crypto/base58.h"
#include "crypto/hash_wrapper.hpp"
#include "crypto/verus_transaction.hpp"
#include "crypto/verushash/arith_uint256.h"
#include "crypto/verushash/verus_hash.h"
#include "daemon/daemon_rpc.hpp"
#include "logger.hpp"
#include "sock_addr.hpp"
#include "stratum/stratum_server.hpp"

// #define CONFIG_PATH                                                   \
//     "//home/sickguy/Documents/Projects/SickPool/server/config/coins/" \
//     "VRSC.json"
// #define CONFIG_PATH_VRSC \
//     "C:\\projects\\pool\\pool-server\\config\\coins\\VRSC.json"

void ParseCoinConfig(const simdjson::padded_string& json, CoinConfig& cnfg);

stratum_server_t *stratum_server_ptr;

void SigintHandler(int sig)
{
    Logger::Log(LogType::Info, LogField::Config, "Enter password:");
    // std::string pass;
    // std::cin >> pass;

    // if(pass == "1234"){
        Logger::Log(LogType::Info, LogField::Config, "Stopping stratum server...");

        stratum_server_ptr->Stop();
    // }
}

int main(int argc, char** argv)
{

    // std::cout << std::hex
    //           <<
    //           UintToArith256(uint256S("00000000000184c09e98da047ab3260fca551"
    //                                      "c8c476551b63140d99b634aea2d"))
    //                  .GetCompact();
    // std::cout << "Block Submission Size: " << sizeof(BlockSubmission);
    Logger::Log(LogType::Info, LogField::Config, "Starting SickPool!");

    Logger::Log(LogType::Info, LogField::Config, "Static config:");
    Logger::Log(LogType::Info, LogField::Config, "Coin symbol: {}",
                COIN_SYMBOL);

    Logger::Log(LogType::Info, LogField::Config, "Loading dynamic config...");

    if (signal(SIGINT, SigintHandler) == SIG_ERR)
    {
        Logger::Log(LogType::Error, LogField::Config,
                    "Failed to register SIGINT...");
    }

    CoinConfig coinConfig;
    try
    {
        if (argc < 2 || !std::ifstream(argv[1]).good())
        {
            throw std::runtime_error("Bad config file specified");
        }

        simdjson::padded_string json = simdjson::padded_string::load(argv[1]);
        ParseCoinConfig(json, coinConfig);

        Logger::Log(LogType::Info, LogField::Config, "Coin config loaded:");
        Logger::Log(LogType::Info, LogField::Config, "Pool address: {}",
                    coinConfig.pool_addr);
        Logger::Log(LogType::Info, LogField::Config, "Redis port: {}",
                    coinConfig.redis_port);
        Logger::Log(LogType::Info, LogField::Config, "PoW fee: {}",
                    coinConfig.pow_fee);
        Logger::Log(LogType::Info, LogField::Config, "PoS fee: {}",
                    coinConfig.pos_fee);
        Logger::Log(LogType::Info, LogField::Config, "Hashrate retention: {}s",
                    coinConfig.hashrate_interval_seconds);
        Logger::Log(LogType::Info, LogField::Config, "Hashrate ttl: {}s",
                    coinConfig.hashrate_ttl_seconds);
        Logger::Log(LogType::Info, LogField::Config, "Effort retention: {}s",
                    coinConfig.effort_interval_seconds);

        stratum_server_t stratum_server(coinConfig);
        stratum_server_ptr = &stratum_server;
        stratum_server.Listen();
    }
    catch (std::runtime_error e)
    {
        Logger::Log(LogType::Critical, LogField::Config, "START-UP ERROR: {}.",
                    e.what());
        return EXIT_FAILURE;
    }
    return EXIT_FAILURE;  // should never finish!
}

void AssignJson(const char* name, std::string& obj,
                simdjson::ondemand::document& doc)
{
    try
    {
        std::string_view sv = doc[name].get_string();

        obj = std::string(sv);
    }
    catch (...)
    {
        throw std::runtime_error(std::string("Invalid or no \"") + name +
                                 "\" (string) variable in config file");
    }
}

template <typename T>
void AssignJson(const char* name, T& obj, simdjson::ondemand::document& doc)
{
    try
    {
        obj = doc[name].get<T>();
    }
    catch (...)
    {
        throw std::runtime_error(std::string("Invalid or no \"") + name +
                                 "\" () variable in config file");
    }
}

void ParseCoinConfig(const simdjson::padded_string& json, CoinConfig& cnfg)
{
    simdjson::ondemand::parser confParser;
    simdjson::ondemand::document configDoc = confParser.iterate(json);

    AssignJson("stratum_port", cnfg.stratum_port, configDoc);
    AssignJson("control_port", cnfg.control_port, configDoc);
    AssignJson("redis_port", cnfg.redis_port, configDoc);
    AssignJson("hashrate_interval_seconds", cnfg.hashrate_interval_seconds,
               configDoc);
    AssignJson("effort_interval_seconds", cnfg.effort_interval_seconds,
               configDoc);
    AssignJson("average_hashrate_interval_seconds",
               cnfg.average_hashrate_interval_seconds, configDoc);
    AssignJson("hashrate_ttl", cnfg.hashrate_ttl_seconds, configDoc);
    AssignJson("socket_recv_timeout_seconds", cnfg.socket_recv_timeout_seconds,
               configDoc);
    AssignJson("diff_adjust_seconds", cnfg.diff_adjust_seconds, configDoc);
    AssignJson("pow_fee", cnfg.pow_fee, configDoc);
    AssignJson("pos_fee", cnfg.pos_fee, configDoc);
    AssignJson("default_diff", cnfg.default_diff, configDoc);
    AssignJson("target_shares_rate", cnfg.target_shares_rate, configDoc);
    AssignJson("pool_addr", cnfg.pool_addr, configDoc);
    AssignJson("payment_interval_seconds", cnfg.payment_interval_seconds,
               configDoc);
    AssignJson("min_payout_threshold", cnfg.min_payout_threshold, configDoc);

    try
    {
        auto rpcs = configDoc["rpcs"].get_array();
        for (auto rpc : rpcs)
        {
            RpcConfig rpcConf;
            std::string_view host_sv = rpc["host"].get_string();
            std::string_view auth_sv = rpc["auth"].get_string();
            rpcConf.host = std::string(host_sv);
            rpcConf.auth = std::string(auth_sv);
            cnfg.rpcs.push_back(rpcConf);
        }
    }
    catch (...)
    {
        throw std::runtime_error(
            std::string("Invalid or no \"") + "rpcs" +
            "\"; ([string host, string auth]) variable in config file");
    }
}