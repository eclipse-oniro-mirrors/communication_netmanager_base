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

#ifndef NET_TETHER_CLIENT_H
#define NET_TETHER_CLIENT_H

#include <string>

#include "singleton.h"
#include "i_net_tether_service.h"

namespace OHOS {
namespace NetManagerStandard {
class NetTetherClient {
    DECLARE_DELAYED_SINGLETON(NetTetherClient)

public:
    int32_t TetherByIface(const std::string &iface);
    int32_t UntetherByIface(const std::string &iface);
    int32_t TetherByType(TetheringType type);
    int32_t UntetherByType(TetheringType type);
    int32_t RegisterTetheringEventCallback(const sptr<INetTetherCallback> &callback);

private:
    class NetTetherDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit NetTetherDeathRecipient(NetTetherClient &client) : client_(client) {}
        ~NetTetherDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        NetTetherClient &client_;
    };

private:
    sptr<INetTetherService> GetProxy();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutex_;
    sptr<INetTetherService> netTetherService_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_TETHER_CLIENT_H