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

#ifndef NET_HANDLER_H
#define NET_HANDLER_H

#include <string>
#include <list>

#include "parcel.h"
#include "singleton.h"
#include "i_net_conn_service.h"

#include "inet_addr.h"

namespace OHOS {
namespace NetManagerStandard {
class NetHandler {
public:
    NetHandler(int32_t netId);
    NetHandler();
    ~NetHandler();

    int32_t BindSocket(int socket_fd);
    int32_t GetAddressesByName(const std::string &host, std::list<INetAddr> &addrList);
    int32_t GetAddressByName(const std::string &host, INetAddr &addr);

    void SetNetId(int32_t netId)
    {
        netId_ = netId;
    }

    int32_t GetNetId() const
    {
        return netId_;
    }

private:
    class NetConnDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit NetConnDeathRecipient(NetHandler &client) : client_(client) {}
        ~NetConnDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        NetHandler &client_;
    };

private:
    sptr<INetConnService> getProxy();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutex_;
    sptr<INetConnService> NetConnService_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    uint32_t netId_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_HANDLER_H