#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "crypto/base58.h"
#include "crypto/hash_wrapper.hpp"
#include "crypto/verus_transaction.hpp"
#include "crypto/verushash/arith_uint256.h"
#include "crypto/verushash/verus_hash.h"
#include "daemon/daemon_rpc.hpp"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/rapidjson.h"
#include "sock_addr.hpp"
#include "stratum/stratum_server.hpp"

#define CONFIG_PATH_VRSC                                              \
    "//home/sickguy/Documents/Projects/SickPool/server/config/coins/" \
    "VRSC.json"
// #define CONFIG_PATH_VRSC \
//     "C:\\projects\\pool\\pool-server\\config\\coins\\VRSC.json"
// #define CONFIG_PATH_VRSC                                                      \
//     "/media/sickguy/B0A8A4E4A8A4A9F4/Projects/Pool/pool-server/config/coins/" \
//     "VRSC.json"

void ParseCoinConfig(CoinConfig* cnfg, const char* path);

// int valid(const char* s)
// {
//     unsigned char dec[32], d1[SHA256_DIGEST_LENGTH],
//     d2[SHA256_DIGEST_LENGTH];

//     coin_err = "";
//     if (!unbase58(s, dec)) return 0;

//     SHA256(SHA256(dec, 21, d1), SHA256_DIGEST_LENGTH, d2);

//     if (memcmp(dec + 21, d2, 4)) return 0;

//     return 1;
// }

int main(int argc, char** argv)
{
    // const char* s[] = {"1Q1pE5vPGEEMqRcVRMbtBK842Y6Pzo6nK9",
    //                    "1AGNa15ZQXAZUgFiqJ2i7Z2DPU2J6hW62i",
    //                    "1Q1pE5vPGEEMqRcVRMbtBK842Y6Pzo6nJ9",
    //                    "1AGNa15ZQXAZUgFiqJ2i7Z2DPU2J6hW62I", 0};
    // int i;
    // for (i = 0; s[i]; i++)
    // {
    //     int status = valid(s[i]);
    //     printf("%s: %s\n", s[i], status ? "Ok" : "NO OK");
    // }
    // return 0;

    CoinConfig coinConfig;

    try
    {
        ParseCoinConfig(&coinConfig, CONFIG_PATH_VRSC);
        StratumServer stratumServer(coinConfig);
        stratumServer.StartListening();
    }
    catch (std::runtime_error e)
    {
        std::cerr << "START-UP ERROR: " << e.what() << "." << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void AssignJson(const char* name, std::string& obj, Document& doc)
{
    if (doc.HasMember(name) && doc[name].IsString())
        obj = doc[name].GetString();
    else
        throw std::runtime_error(std::string("Invalid or no \"") + name +
                                 "\" (string) variable in config file");
}

void AssignJson(const char* name, u_short& obj, Document& doc)
{
    if (doc.HasMember(name) && doc[name].IsUint())
        obj = doc[name].GetUint();
    else
        throw std::runtime_error(std::string("Invalid or no \"") + name +
                                 "\" (uint) variable in config file");
}

void AssignJson(const char* name, double& obj, Document& doc)
{
    if (doc.HasMember(name) && doc[name].IsDouble())
        obj = doc[name].GetDouble();
    else
        throw std::runtime_error(std::string("Invalid or no \"") + name +
                                 "\" (double) variable in config file");
}

void ParseCoinConfig(CoinConfig* cnfg, const char* path)
{
    std::ifstream configfStream;
    configfStream.open(CONFIG_PATH_VRSC);
    if (!configfStream.is_open())
    {
        throw std::runtime_error(
            std::string("Failed to open coin config file ") + CONFIG_PATH_VRSC);
    }

    IStreamWrapper is(configfStream);

    rapidjson::Document configDoc(rapidjson::kObjectType);
    configDoc.ParseStream(is);

    AssignJson("name", cnfg->name, configDoc);
    AssignJson("symbol", cnfg->symbol, configDoc);
    AssignJson("algo", cnfg->algo, configDoc);
    AssignJson("stratum_port", cnfg->stratum_port, configDoc);
    AssignJson("pow_fee", cnfg->pow_fee, configDoc);
    AssignJson("pos_fee", cnfg->pos_fee, configDoc);
    AssignJson("default_diff", cnfg->default_diff, configDoc);
    AssignJson("pool_addr", cnfg->pool_addr, configDoc);
    AssignJson("redis_host", cnfg->redis_host, configDoc);

    if (!configDoc.HasMember("rpcs") || !configDoc["rpcs"].IsArray())
        throw std::runtime_error(
            std::string("Invalid or no \"") + "rpcs" +
            "\"; ([string host, string auth]) variable in config file");
    auto rpcs = configDoc["rpcs"].GetArray();

    for (int i = 0; i < rpcs.Size(); i++)
    {
        cnfg->rpcs[i].host = rpcs[i]["host"].GetString();
        cnfg->rpcs[i].auth = rpcs[i]["auth"].GetString();
    }

    configfStream.close();
}