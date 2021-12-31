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

#include "net_tether_controller.h"

#include <string_ex.h>

#include "net_mgr_log_wrapper.h"
#include "iremote_broker.h"
#include "message_parcel.h"
#include "message_option.h"
#include "net_tether_define.h"

namespace OHOS {
namespace NetManagerStandard {
NetTetherController::NetTetherControllerDeath::NetTetherControllerDeath(NetTetherController &object)
    : mNetTetherController_(object) {}

void NetTetherController::NetTetherControllerDeath::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    NETMGR_LOG_D("Remote service is died!");
    mNetTetherController_.SetRemoteDeath(true);
}

NetTetherController::NetTetherController(uint32_t netAbilityId) : netAbilityId_(netAbilityId), remoteDeath_(false) {}

bool NetTetherController::Init(const std::unordered_map<std::string, uint32_t> &callCode)
{
    sptr<ISystemAbilityManager> sa_mgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sa_mgr == nullptr) {
        NETMGR_LOG_E("failed to get SystemAbilityManager");
        return false;
    }

    remote_ = sa_mgr->GetSystemAbility(netAbilityId_);
    if (remote_ == nullptr) {
        NETMGR_LOG_E("failed to get sa proxy, ability id is [%{public}u]", netAbilityId_);
        return false;
    }
    death_ = (std::make_unique<NetTetherControllerDeath>(*this)).release();
    remote_->AddDeathRecipient(death_);
    callCode_ = callCode;
    return true;
}

void NetTetherController::SetRemoteDeath(bool bStatus)
{
    remoteDeath_ = bStatus;
}

int32_t NetTetherController::OpenTether()
{
    if (remoteDeath_) {
        NETMGR_LOG_E("Remote network supplier does not exist!");
        return -1;
    }
    auto iter = callCode_.find("OpenTether");
    if (iter == callCode_.end()) {
        return -1;
    }
    MessageParcel data;
    switch (netAbilityId_) {
        case WIFI_SA_ID:
            data.WriteInt32(0);
            break;
        default:
            break;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote_->SendRequest(iter->second, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("OpenTether call remote failed, ret [%{public}d]", error);
        return -1;
    }
    int32_t replyRes = 0;
    if (!reply.ReadInt32(replyRes)) {
        return -1;
    }
    return replyRes;
}

int32_t NetTetherController::CloseTether()
{
    if (remoteDeath_) {
        NETMGR_LOG_E("Remote network supplier does not exist!");
        return -1;
    }
    auto iter = callCode_.find("CloseTether");
    if (iter == callCode_.end()) {
        return -1;
    }
    MessageParcel data;
    switch (netAbilityId_) {
        case WIFI_SA_ID:
            data.WriteInt32(0);
            break;
        default:
            break;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote_->SendRequest(iter->second, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOG_E("CloseTether call remote failed, ret [%{public}d]", error);
        return -1;
    }
    int32_t replyRes = 0;
    if (!reply.ReadInt32(replyRes)) {
        return -1;
    }
    return replyRes;
}
} // namespace NetManagerStandard
} // namespace OHOS
