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

#include "net_tether_service.h"
#include "system_ability_definition.h"
#include "net_mgr_log_wrapper.h"
#include "net_tethering.h"

namespace OHOS {
namespace NetManagerStandard {
const bool REGISTER_LOCAL_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetTetherService>::GetInstance().get());

NetTetherService::NetTetherService()
    : SystemAbility(COMM_NET_TETHERING_MANAGER_SYS_ABILITY_ID, true), registerToService_(false),
      state_(STATE_STOPPED) {}

NetTetherService::~NetTetherService()
{
    NetTethering::ReleaseInstance();
}

void NetTetherService::OnStart()
{
    if (state_ == STATE_RUNNING) {
        NETMGR_LOG_D("the state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_LOG_E("init failed");
        return;
    }
    state_ = STATE_RUNNING;
}

void NetTetherService::OnStop()
{
    state_ = STATE_STOPPED;
    registerToService_ = false;
}

bool NetTetherService::Init()
{
    if (!REGISTER_LOCAL_RESULT) {
        NETMGR_LOG_E("Register to local sa manager failed");
        registerToService_ = false;
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<NetTetherService>::GetInstance().get())) {
            NETMGR_LOG_E("Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }
    return true;
}

int32_t NetTetherService::TetherByIface(const std::string &iface)
{
    NETMGR_LOG_D("NetTetherService::TetherByIface, iface: [%{public}s]", iface.c_str());
    return NetTethering::GetInstance()->TetherByIface(iface);
}

int32_t NetTetherService::UntetherByIface(const std::string &iface)
{
    NETMGR_LOG_D("NetTetherService::UntetherByIface, iface: [%{public}s]", iface.c_str());
    return NetTethering::GetInstance()->UntetherByIface(iface);
}

int32_t NetTetherService::TetherByType(TetheringType type)
{
    NETMGR_LOG_D("NetTetherService::TetherByType, type: [%{public}d]", static_cast<int32_t>(type));
    return NetTethering::GetInstance()->TetherByType(type);
}

int32_t NetTetherService::UntetherByType(TetheringType type)
{
    NETMGR_LOG_D("NetTetherService::UntetherByType, type: [%{public}d]", static_cast<int32_t>(type));
    return NetTethering::GetInstance()->UntetherByType(type);
}

int32_t NetTetherService::RegisterTetheringEventCallback(const sptr<INetTetherCallback> &callback)
{
    NETMGR_LOG_D("NetTetherService::RegisterTetheringEventCallback");
    return NetTethering::GetInstance()->RegisterTetheringEventCallback(callback);
}
} // namespace NetManagerStandard
} // namespace OHOS