/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "net_policy_listener.h"

#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "netmanager_base_common_utils.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace OHOS::EventFwk;
constexpr const char* UID = "uid";
};

NetPolicyListener::NetPolicyListener(const EventFwk::CommonEventSubscribeInfo &sp,
                                     std::shared_ptr<NetPolicyService> NetPolicy)
    : CommonEventSubscriber(sp)
{
    netPolicyService_ = NetPolicy;
}

void NetPolicyListener::OnReceiveEvent(const CommonEventData &data)
{
    auto &want = data.GetWant();
    std::string wantAction = want.GetAction();
    if (wantAction == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        uint32_t uid = want.GetIntParam(UID, 0);
        netPolicyService_->GetNetAccessPolicyDBHandler().DeleteByUid(uid);
        netPolicyService_->DeleteNetworkAccessPolicy(uid);
        NETMGR_LOG_I("packet remove uid:[%{public}d]", uid);
    }
}
} // namespace NetManagerStandard
} // namespace OHOS
