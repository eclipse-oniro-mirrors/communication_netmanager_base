/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "dns_result_call_back.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "net_conn_service.h"


namespace OHOS {
namespace NetManagerStandard {
int32_t NetDnsResultCallback::OnDnsResultReport(uint32_t size, const std::list<NetsysNative::NetDnsResultReport> netDnsResultReport)
{
    NETMGR_LOG_I("Dns Result Report interface is called");
    netDnsResult_.clear();
    std::list<NetsysNative::NetDnsResultReport> report(netDnsResultReport);
    int32_t defaultNetid = 0;
    int32_t dnsHealthSuccess = 0;
    int32_t result = NetConnService::GetInstance()->GetDefaultNet(defaultNetid);
    NETMGR_LOG_I("GetDefaultNet result: %{public}d, defaultNetid: %{public}d", result, defaultNetid);
    for (auto &it : report) {
        NETMGR_LOG_I("netId_: %{public}d, queryResult_: %{public}d, pid_ : %{public}d", it.netid_, it.queryresult_, it.pid_);
        if (netDnsResult_.find(it.netid_) == netDnsResult_.end() && it.netid_ == 0) {
            netDnsResult_[defaultNetid].totalReports_++;
            netDnsResult_[defaultNetid].failReports_ += it.queryresult_ == 0 ? 0 : 1;
        } else if (netDnsResult_.find(it.netid_) == netDnsResult_.end()) {
            netDnsResult_[it.netid_].totalReports_ = 1;
            netDnsResult_[it.netid_].failReports_ = it.queryresult_ == 0 ? 0 : 1;
        } else {
            netDnsResult_[it.netid_].totalReports_++;
            netDnsResult_[it.netid_].failReports_ += it.queryresult_ == 0 ? 0 : 1;
        }
    }
    for (auto &item : netDnsResult_) {
        NETMGR_LOG_I("netId_: %{public}d, totalReports_: %{public}d, failReports_: %{public}d", 
            item.first, item.second.totalReports_, item.second.failReports_);
        if (item.second.failReports_/item.second.totalReports_ > 0.2) {
            NETMGR_LOG_I("start netdetection for dns fail, netId:%{public}d,totalReports:%{public}d,failReports:%{public}d",item.first,item.second.totalReports_,item.second.failReports_);
            dnsHealthSuccess = 1;
            int result = NetConnService::GetInstance()->NetDetectionForDnsHealth(item.first, dnsHealthSuccess);
            if (result != 0) {
                NETMGR_LOG_E("NetDetectionForDnsHealth failed");
            }
        } else {
            NETMGR_LOG_I("start netdetection for dns success, netId:%{public}d,totalReports:%{public}d,failReports:%{public}d",item.first,item.second.totalReports_,item.second.failReports_);
            dnsHealthSuccess = 0;
            int result = NetConnService::GetInstance()->NetDetectionForDnsHealth(item.first, dnsHealthSuccess);
            if (result != 0) {
                NETMGR_LOG_E("NetDetectionForDnsHealth failed");
            }
        }
    }
    return 0;
}

void NetDnsResultCallback::GetDumpMessageForDnsResult(std::string &message)
{
    message.append("Dns result Info:\n");
    for (auto &item : netDnsResult_) {
        message.append("\tnetId: " + std::to_string(item.first) + "\n");
        message.append("\ttotalReports: " + std::to_string(item.second.totalReports_) + "\n");
        message.append("\tfailReports: " + std::to_string(item.second.failReports_) + "\n");
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
