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

#ifndef NET_TETHER_CONTROLLER_H
#define NET_TETHER_CONTROLLER_H

#include <string>
#include <unordered_map>
#include <iservice_registry.h>
#include <iremote_object.h>

namespace OHOS {
namespace NetManagerStandard {
class NetTetherController : public virtual RefBase {
public:
    explicit NetTetherController(uint32_t netAbilityId);
    bool Init(const std::unordered_map<std::string, uint32_t> &callCode);
    void SetRemoteDeath(bool bStatus);
    int32_t OpenTether();
    int32_t CloseTether();

private:
    class NetTetherControllerDeath : public IRemoteObject::DeathRecipient {
    public:
        explicit NetTetherControllerDeath(NetTetherController &object);
        void OnRemoteDied(const wptr<IRemoteObject> &object);

    private:
        NetTetherController &mNetTetherController_;
    };

private:
    uint32_t netAbilityId_;
    bool remoteDeath_;
    sptr<IRemoteObject> remote_;
    sptr<NetTetherControllerDeath> death_;
    std::unordered_map<std::string, uint32_t> callCode_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_TETHER_CONTROLLER_H