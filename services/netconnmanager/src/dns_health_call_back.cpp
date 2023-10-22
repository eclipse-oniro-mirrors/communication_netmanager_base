/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "dns_health_call_back.h"
#include "net_conn_service.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t NetDnsHealthCallback::OnDnsHealthReport(const NetsysNative::NetDnsHealthReport &dnsHealthReport)
{
    NETMGR_LOG_I(" Dns Health Report in, netid:[%{public}d] uid:[%{public}d] appid:[%{public}d] host_:[%{public}s] type:[%{public}d]", dnsHealthReport.netid_, dnsHealthReport.uid_, dnsHealthReport.appid_, dnsHealthReport.host_.c_str(), dnsHealthReport.type_);
    if (dnsHealthReport.result_ == 0) {
        NETMGR_LOG_I("Dns Health Report fail, start NetDetection");
    }
    return NETMANAGER_SUCCESS; 
};

} // namespace NetManagerStandard
} // namespace OHOS
