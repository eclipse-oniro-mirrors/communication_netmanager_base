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

#ifndef NETSYS_WEARABLE_DISTRIBUTED_NET_SERVICE_MANAGER_H
#define NETSYS_WEARABLE_DISTRIBUTED_NET_SERVICE_MANAGER_H

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
class DistributeNetManager {
public:
    enum RULES_TYPE {
        TCP_ADD_RULE,
        UDP_ADD_RULE,
        INPUT_ADD_RULE,
        INPUT_DEL_RULE,
        DEFAULT_RULE
    };

    /**
     * @brief Sets the IP tables based on the provided TCP and UDP port IDs
     *
     * @param tcpPortId TCP port ID
     * @param udpPortId UDP port ID
     * @return A uint32_t value indicating the result of the operation
     */
    int32_t EnableWearbleDistributedNetForward(const int32_t tcpPortId, const int32_t udpPortId);

    /**
     * @brief Clears the IP tables
     *
     * @return A uint32_t value indicating the result of the operation
     */
    int32_t DisableWearbleDistributedNetForward();
private:
    std::string GenerateRule(const char *inputRules, const int32_t portId);
    void SetTcpPort(const int32_t tcpPortId);
    void SetUdpPort(const int32_t udpPortId);
    int32_t AddTcpIpRules();
    int32_t AddUdpIpRules();
    int32_t DealRule(const RULES_TYPE type, const int32_t portId);
private:
    int32_t tcpPort_;
    int32_t udpPort_;
};
} // namespace nmd
} // namespace OHOS// namespace OHOS::nmd
#endif // NETSYS_WEARABLE_DISTRIBUTED_NET_SERVICE_MANAGER_H
