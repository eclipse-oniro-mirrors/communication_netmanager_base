/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef NET_CONNECTION_ANI_H
#define NET_CONNECTION_ANI_H
#include "cxx.h"
#include "i_net_conn_callback.h"
#include "net_conn_callback_stub.h"
#include "net_conn_client.h"
#include "net_handle.h"
#include "refbase.h"
#include <memory>

namespace OHOS {

namespace NetManagerAni {

struct NetHandle;
struct NetCapabilities;
struct HttpProxy;
struct ConnectionProperties;
struct LinkAddress;
struct NetAddress;
struct ConnCallback;
struct NetBlockStatusInfo;
struct NetCapabilityInfo;
struct NetConnectionPropertyInfo;

NetHandle GetDefaultNetHandle(int32_t &ret);

rust::vec<NetHandle> GetAllNets(int32_t &ret);

bool HasDefaultNet(int32_t &ret);

NetCapabilities GetNetCapabilities(NetHandle const &netHandle, int32_t &ret);

HttpProxy GetDefaultHttpProxy(int32_t &ret);

HttpProxy GetGlobalHttpProxy(int32_t &ret);

int32_t SetGlobalHttpProxy(const HttpProxy &httpProxy);
int32_t SetAppHttpProxy(const HttpProxy &httpProxy);

NetManagerStandard::NetConnClient &GetNetConnClient(int32_t &nouse);

int32_t IsDefaultNetMetered(bool &isMetered);

ConnectionProperties GetConnectionProperties(int32_t net_id, int32_t &ret);

rust::vec<NetAddress> GetAddressesByName(const std::string &host, int32_t netId, int32_t &ret);

NetAddress GetAddressByName(const std::string &host, int32_t netId, int32_t &ret);

void NetDetection(int32_t net_id, int32_t &ret);

class NetCoonCallback : public NetManagerStandard::NetConnCallbackStub {
public:
    NetCoonCallback(rust::Box<ConnCallback> callback);
    ~NetCoonCallback() = default;

    int32_t NetAvailable(sptr<NetManagerStandard::NetHandle> &netHandle) override;
    int32_t NetCapabilitiesChange(sptr<NetManagerStandard::NetHandle> &netHandle,
                                  const sptr<NetManagerStandard::NetAllCapabilities> &netAllCap) override;
    int32_t NetConnectionPropertiesChange(sptr<NetManagerStandard::NetHandle> &netHandle,
                                          const sptr<NetManagerStandard::NetLinkInfo> &info) override;
    int32_t NetLost(sptr<NetManagerStandard::NetHandle> &netHandle) override;
    int32_t NetUnavailable() override;
    int32_t NetBlockStatusChange(sptr<NetManagerStandard::NetHandle> &netHandle, bool blocked) override;

private:
    rust::Box<ConnCallback> inner_;
};

class UnregisterHandle {
public:
    UnregisterHandle(sptr<NetCoonCallback> callback);
    ~UnregisterHandle() = default;

    int32_t Unregister();

private:
    sptr<NetCoonCallback> callback_;
};

std::unique_ptr<UnregisterHandle> RegisterNetConnCallback(rust::Box<ConnCallback> Connection, int32_t &ret);

} // namespace NetManagerAni
} // namespace OHOS
#endif