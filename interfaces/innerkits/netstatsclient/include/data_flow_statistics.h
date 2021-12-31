/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef DATA_FLOW_STATISTICS_H
#define DATA_FLOW_STATISTICS_H

#include <string>

namespace OHOS {
namespace NetManagerStandard {
class DataFlowStatistics {
public:

    DataFlowStatistics();
    ~DataFlowStatistics();

    /**
     * @brief Obtains the bytes received over the cellular network.
     *
     * @return The number of received bytes.
     */
    int64_t GetCellularRxBytes();

    /**
     * @brief Obtains the bytes sent over the cellular network.
     *
     * @return The number of sent bytes.
     */
    int64_t GetCellularTxBytes();

    /**
     * @brief Obtains the bytes received through all NICs.
     *
     * @return The number of received bytes.
     */
    int64_t GetAllRxBytes();

    /**
     * @brief Obtains the bytes sent through all NICs.
     *
     * @return The number of sent bytes.
     */
    int64_t GetAllTxBytes();

    /**
     * @brief Obtains the bytes received through a specified UID.
     *
     * @param uid
     * @return The number of received bytes.
     */
    int64_t GetUidRxBytes(uint32_t uid);

    /**
     * @brief Obtains the bytes sent through a specified UID.
     *
     * @param uid
     * @return The number of sent bytes.
     */
    int64_t GetUidTxBytes(uint32_t uid);

    /**
     * @brief Obtains the bytes received through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of received bytes.
     */
    int64_t GetIfaceRxBytes(const std::string &interfaceName);

    /**
     * @brief Obtains the bytes sent through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of sent bytes.
     */
    int64_t GetIfaceTxBytes(const std::string &interfaceName);

    /**
     * @brief Obtains the packets received through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of received packets.
     */
    int64_t GetIfaceRxPackets(const std::string &interfaceName);

    /**
     * @brief Obtains the packets sent through a specified NIC.
     *
     * @param iface The name of the interface.
     * @return The number of sent packets.
     */
    int64_t GetIfaceTxPackets(const std::string &interfaceName);

    /**
     * @brief Obtains the NIC list.
     *
     * @return The list of interface.
     */
    std::vector<std::string> InterfaceGetList();
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif  // DATA_FLOW_STATISTICS_H
