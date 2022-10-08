#ifndef DAEMON_MANAGER_ZANO_HPP
#define DAEMON_MANAGER_ZANO_HPP

#include "daemon_manager.hpp"
#include "charconv"
#include "daemon_responses_cryptonote.hpp"

class DaemonManagerZano : public DaemonManager
{
   public:
    using DaemonManager::DaemonManager;
    bool GetBlockTemplate(BlockTemplateResCn& templateRes,
                          std::string_view addr, std::string_view extra_data,
                          simdjson::ondemand::parser& parser);
};
#endif