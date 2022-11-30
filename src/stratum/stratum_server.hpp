#ifndef STRATUM_SERVER_HPP_
#define STRATUM_SERVER_HPP_
#include <simdjson.h>

#include <chrono>
#include <deque>
#include <functional>
#include <iterator>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "job.hpp"
#include "block_submitter.hpp"
#include "connection.hpp"
#include "jobs/job_manager.hpp"
#include "logger.hpp"
#include "server.hpp"
#include "shares/share_processor.hpp"
#include "static_config/static_config.hpp"
#include "stratum_client.hpp"
#include "stratum_server_base.hpp"
static constexpr std::string_view field_str_stratum = "StratumServer";

template <StaticConf confs>
class StratumServer : public StratumBase
{
   public:
    explicit StratumServer(CoinConfig&& conf);
    ~StratumServer() override;
    using WorkerContextT = WorkerContext<confs.BLOCK_HEADER_SIZE>;
    using JobT = Job<confs.STRATUM_PROTOCOL>;

   private:
    using ShareT = StratumShare<confs.STRATUM_PROTOCOL>;

    const Logger<field_str_stratum> logger;

    std::jthread stats_thread;

   protected:
    simdjson::ondemand::parser httpParser =
        simdjson::ondemand::parser(HTTP_REQ_ALLOCATE);

    daemon_manager_t daemon_manager;
    PaymentManager payment_manager;
    job_manager_t job_manager;
    BlockSubmitter block_submitter;
    StatsManager stats_manager;

    virtual void HandleReq(Connection<StratumClient>* conn,
                           WorkerContextT* wc,
                           std::string_view req) = 0;

    virtual RpcResult HandleAuthorize(StratumClient* cli,
                                      std::string_view miner,
                                      std::string_view worker);
    RpcResult HandleAuthorize(StratumClient* cli,
                              simdjson::ondemand::array& params);

    void HandleBlockNotify() override;

    RpcResult HandleShare(StratumClient* cli, WorkerContextT* wc,
                          ShareT& share);

    virtual void UpdateDifficulty(Connection<StratumClient>* conn) = 0;

    virtual void BroadcastJob(Connection<StratumClient>* conn,
                              const JobT* job) const = 0;
    void HandleConsumeable(connection_it* conn) override;
    bool HandleConnected(connection_it* conn) override;

    void DisconnectClient(
        const std::shared_ptr<Connection<StratumClient>> conn_ptr) override;
};

#ifdef STRATUM_PROTOCOL_ZEC
#include "stratum_server_zec.hpp"
#elif STRATUM_PROTOCOL_BTC
#include "stratum_server_btc.hpp"
#elif STRATUM_PROTOCOL_CN
#include "stratum_server_cn.hpp"
#endif

#endif