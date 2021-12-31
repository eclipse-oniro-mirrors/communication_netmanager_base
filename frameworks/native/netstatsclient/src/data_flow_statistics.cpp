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

#include "data_flow_statistics.h"
#include "netd_controller.h"

namespace OHOS {
namespace NetManagerStandard {
DataFlowStatistics::DataFlowStatistics()
{}

DataFlowStatistics::~DataFlowStatistics() {}

int64_t DataFlowStatistics::GetCellularRxBytes()
{
    return NetdController::GetInstance().GetCellularRxBytes();
}

int64_t DataFlowStatistics::GetCellularTxBytes()
{
    return NetdController::GetInstance().GetCellularTxBytes();
}

int64_t DataFlowStatistics::GetAllRxBytes()
{
    return NetdController::GetInstance().GetAllRxBytes();
}

int64_t DataFlowStatistics::GetAllTxBytes()
{
    return NetdController::GetInstance().GetAllTxBytes();
}

int64_t DataFlowStatistics::GetUidRxBytes(uint32_t uid)
{
    return NetdController::GetInstance().GetUidRxBytes(uid);
}

int64_t DataFlowStatistics::GetUidTxBytes(uint32_t uid)
{
    return NetdController::GetInstance().GetUidTxBytes(uid);
}

int64_t DataFlowStatistics::GetIfaceRxBytes(const std::string &interfaceName)
{
    return NetdController::GetInstance().GetIfaceRxBytes(interfaceName);
}

int64_t DataFlowStatistics::GetIfaceTxBytes(const std::string &interfaceName)
{
    return NetdController::GetInstance().GetIfaceTxBytes(interfaceName);
}

int64_t DataFlowStatistics::GetIfaceRxPackets(const std::string &interfaceName)
{
    return NetdController::GetInstance().GetIfaceRxPackets(interfaceName);
}

int64_t DataFlowStatistics::GetIfaceTxPackets(const std::string &interfaceName)
{
    return NetdController::GetInstance().GetIfaceTxPackets(interfaceName);
}

std::vector<std::string> DataFlowStatistics::InterfaceGetList()
{
    return NetdController::GetInstance().InterfaceGetList();
}
} // namespace NetManagerStandard
} // namespace OHOS