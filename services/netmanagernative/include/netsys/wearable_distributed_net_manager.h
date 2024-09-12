/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NETSYS_WEARABLE_DISTRIBUTED_NET_MANAGER_H
#define NETSYS_WEARABLE_DISTRIBUTED_NET_MANAGER_H

#include <string>
#include <vector>
#include <cstdint>

#define TCP_ADD16 "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -p tcp -j REDIRECT "\
    "--to-ports %d"
#define UDP_ADD16 "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -p udp -j TPROXY "\
    "--tproxy-mark 0x1/0x1 --on-port %d"
#define INPUT_ADD "-w -A INPUT -p tcp -s 127.0.0.1 --destination-port %d -j REJECT"
#define INPUT_DEL "-w -D INPUT -p tcp -s 127.0.0.1 --destination-port %d -j REJECT"

namespace OHOS {
namespace nmd {
class WearableDistributedNet {
public:
    enum RULES_TYPE {
        TCP_ADD_RULE,
        UDP_ADD_RULE,
        INPUT_ADD_RULE,
        INPUT_DEL_RULE,
        DEFAULT_RULE
    };

    /**
    * @brief Enables the wearable distributed network forwarding by configuring TCP and UDP ports.
    *
    * @param tcpPortId The TCP port ID to enable forwarding for.
    * @param udpPortId The UDP port ID to enable forwarding for.
    * @return NETMANAGER_SUCCESS if successful, NETMANAGER_ERROR if any of the operations fail.
    */  
    int32_t EnableWearableDistributedNetForward(const int32_t tcpPortId, const int32_t udpPortId);

    /**
    * @brief Disables the wearable distributed network forwarding by removing configured rules.
    *
    * @return NETMANAGER_SUCCESS if successful, NETMANAGER_ERROR if any of the operations fail.
    */
    int32_t DisableWearableDistributedNetForward();
private:
    int32_t EstablishTcpIpRulesForNetworkDistribution();
    int32_t EstablishUdpIpRulesForNetworkDistribution(const int32_t udpPortId);
    int32_t ExecuteIptablesCommands(const char** commands);
    std::string GenerateRule(const char *inputRules, const int32_t portId);
    int32_t ApplyRule(const RULES_TYPE type, const int32_t portId);
    void SetTcpPort(const int32_t tcpPortId);
    int32_t GetTcpPort();
private:
    int32_t tcpPort_;
};
} // namespace nmd
} // namespace OHOS// namespace OHOS::nmd
#endif // NETSYS_WEARABLE_DISTRIBUTED_NET_MANAGER_H
