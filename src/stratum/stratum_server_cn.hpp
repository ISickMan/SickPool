#ifndef STRATUM_SERVER_ZEC_HPP_
#define STRATUM_SERVER_ZEC_HPP_
#include <type_traits>

#include "cn/common/base58.h"
#include "cn/currency_protocol/blobdatatype.h"
#include "job_cryptonote.hpp"
#include "share.hpp"
#include "static_config.hpp"
#include "stratum_server.hpp"

static constexpr std::string_view field_str_cn = "StratumServerCn";

template <StaticConf confs>
class StratumServerCn : public StratumServer<confs>
{
   public:
    using WorkerContextT = StratumServer<confs>::WorkerContextT;
    using job_t = StratumServer<confs>::job_t;

    explicit StratumServerCn(CoinConfig&& conf)
        : StratumServer<confs>(std::move(conf))
    {
    }

   private:
    const Logger<field_str_cn> logger;

    RpcResult HandleAuthorize(StratumClient* cli,
                              simdjson::ondemand::array& params,
                              std::string_view worker);
    RpcResult HandleSubscribe(StratumClient* cli,
                              simdjson::ondemand::array& params) const;
    RpcResult HandleSubmit(StratumClient* cli, WorkerContextT* wc,
                           simdjson::ondemand::array& params,
                           std::string_view worker);

    void HandleReq(Connection<StratumClient>* conn, WorkerContextT* wc,
                   std::string_view req) override;

    void UpdateDifficulty(Connection<StratumClient>* conn) override;

    void BroadcastJob(Connection<StratumClient>* conn, const job_t* job,
                      int id) const;

    void BroadcastJob(Connection<StratumClient>* conn,
                      const job_t* job) const override;
};

// using stratum_server_t = StratumServerCn;

#endif