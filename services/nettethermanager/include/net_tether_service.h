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

#ifndef NET_TETHER_SERVICE_H
#define NET_TETHER_SERVICE_H

#include "singleton.h"
#include "system_ability.h"

#include "net_tether_service_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class NetTetherService
    : public SystemAbility, public NetTetherServiceStub, public std::enable_shared_from_this<NetTetherService> {
    DECLARE_DELAYED_SINGLETON(NetTetherService)
    DECLARE_SYSTEM_ABILITY(NetTetherService)
public:
    void OnStart() override;
    void OnStop() override;
    int32_t TetherByIface(const std::string &iface) override;
    int32_t UntetherByIface(const std::string &iface) override;
    int32_t TetherByType(TetheringType type) override;
    int32_t UntetherByType(TetheringType type) override;
    int32_t RegisterTetheringEventCallback(const sptr<INetTetherCallback> &callback) override;

private:
    bool Init();

private:
    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };
    bool registerToService_;
    ServiceRunningState state_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_TETHER_SERVICE_H
