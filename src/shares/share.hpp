#ifndef SHARE_HPP_
#define SHARE_HPP_
#include <string_view>
#include <memory>
#include <vector>
#include <simdjson.h>

#include "static_config.hpp"
#include "verushash/verus_hash.h"

// as in order of appearance
#ifdef STRATUM_PROTOCOL_ZEC
struct ShareZec
{
    std::string_view worker;
    std::string_view jobId;
    std::string_view time;
    std::string_view nonce2;
    std::string_view solution;
};
typedef ShareZec share_t;
// as in order of appearance
#endif

#ifdef STRATUM_PROTOCOL_BTC
struct ShareBtc
{
    std::string_view worker;
    std::string_view jobId;
    std::string_view extranonce2;
    std::string_view time;
    std::string_view nonce;
};
typedef ShareBtc share_t;

#endif

struct WorkerContext
{
    uint32_t current_height;
    uint8_t block_header[BLOCK_HEADER_SIZE];
    simdjson::ondemand::parser json_parser;

    CVerusHashV2 hasher = CVerusHashV2(SOLUTION_VERUSHHASH_V2_2);
};

enum class ResCode
{
    OK = 1,
    UNKNOWN = 20,
    JOB_NOT_FOUND = 21,
    DUPLICATE_SHARE = 22,
    LOW_DIFFICULTY_SHARE = 23,
    UNAUTHORIZED_WORKER = 24,
    NOT_SUBSCRIBED = 25,

    // non-standard
    VALID_SHARE = 30,
    VALID_BLOCK = 31,
};
struct RpcResult
{
    RpcResult(ResCode e, std::string m = "true") : code(e), msg(m){}

    // static RpcResult Ok = RpcResult(ResCode::OK);

    ResCode code;
    std::string msg;
};
struct ShareResult
{
    ResCode code;
    std::string message;
    double difficulty;
    // uint256 takes vector as param
    std::vector<uint8_t> hash_bytes = std::vector<uint8_t>(32);
};

#endif