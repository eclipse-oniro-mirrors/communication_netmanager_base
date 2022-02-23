/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "connection_exec.h"

#include "constant.h"
#include "net_conn_callback_observer.h"
#include "net_conn_client.h"
#include "netconnection.h"
#include "netmanager_base_log.h"
#include "netmanager_base_napi_utils.h"
#include "securec.h"

static constexpr const int MAX_HOST_LEN = 256;

namespace OHOS::NetManagerStandard {
bool ConnectionExec::ExecGetAddressByName(GetAddressByNameContext *context)
{
    return NetHandleExec::ExecGetAddressesByName(context);
}

napi_value ConnectionExec::GetAddressByNameCallback(GetAddressByNameContext *context)
{
    return NetHandleExec::GetAddressesByNameCallback(context);
}

bool ConnectionExec::ExecGetDefaultNet(GetDefaultNetContext *context)
{
    return DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultNet(context->netHandle) == 0;
}

napi_value ConnectionExec::GetDefaultNetCallback(GetDefaultNetContext *context)
{
    return NetConnCallbackObserver::CreateNetHandle(context->GetEnv(), new NetHandle(context->netHandle));
}

bool ConnectionExec::ExecHasDefaultNet(HasDefaultNetContext *context)
{
    return DelayedSingleton<NetConnClient>::GetInstance()->HasDefaultNet(context->hasDefaultNet) == 0;
}

napi_value ConnectionExec::HasDefaultNetCallback(HasDefaultNetContext *context)
{
    return NapiUtils::GetBoolean(context->GetEnv(), context->hasDefaultNet);
}

bool ConnectionExec::ExecGetNetCapabilities(GetNetCapabilitiesContext *context)
{
    return DelayedSingleton<NetConnClient>::GetInstance()->GetNetCapabilities(context->netHandle,
                                                                              context->capabilities) == 0;
}

napi_value ConnectionExec::GetNetCapabilitiesCallback(GetNetCapabilitiesContext *context)
{
    return NetConnCallbackObserver::CreateNetCapabilities(context->GetEnv(),
                                                          new NetAllCapabilities(context->capabilities));
}

bool ConnectionExec::ExecGetConnectProperties(GetConnectPropertiesContext *context)
{
    return DelayedSingleton<NetConnClient>::GetInstance()->GetConnectionProperties(context->netHandle,
                                                                                   context->linkInfo) == 0;
}

napi_value ConnectionExec::GetConnectPropertiesCallback(GetConnectPropertiesContext *context)
{
    return NetConnCallbackObserver::CreateConnectionProperties(context->GetEnv(), new NetLinkInfo(context->linkInfo));
}

bool ConnectionExec::NetHandleExec::ExecGetAddressesByName(GetAddressByNameContext *context)
{
    addrinfo *res = nullptr;
    int status = getaddrinfo(context->host.c_str(), nullptr, nullptr, &res);
    if (status < 0) {
        NETMANAGER_BASE_LOGE("getaddrinfo errno %{public}d %{public}s", errno, strerror(errno));
        context->SetErrorCode(errno);
        return false;
    }

    char host[MAX_HOST_LEN] = {0};
    for (addrinfo *tmp = res; tmp != nullptr; tmp = tmp->ai_next) {
        (void)memset_s(host, sizeof(host), 0, sizeof(host));
        if (getnameinfo(tmp->ai_addr, tmp->ai_addrlen, host, sizeof(host), nullptr, 0, 0) < 0) {
            continue;
        }
        NETMANAGER_BASE_LOGI("host ip: %{public}s", host);

        NetAddress address;
        SetAddressInfo(host, tmp, address);

        context->addresses.emplace_back(address);
    }
    freeaddrinfo(res);
    return true;
}

napi_value ConnectionExec::NetHandleExec::GetAddressesByNameCallback(GetAddressByNameContext *context)
{
    napi_value addresses = NapiUtils::CreateArray(context->GetEnv(), context->addresses.size());
    for (uint32_t index = 0; index < context->addresses.size(); ++index) {
        napi_value obj = MakeNetAddressJsValue(context->GetEnv(), context->addresses[index]);
        NapiUtils::SetArrayElement(context->GetEnv(), addresses, index, obj);
    }
    return addresses;
}

bool ConnectionExec::NetHandleExec::ExecGetAddressByName(GetAddressByNameContext *context)
{
    addrinfo *res = nullptr;
    int status = getaddrinfo(context->host.c_str(), nullptr, nullptr, &res);
    if (status < 0) {
        NETMANAGER_BASE_LOGE("getaddrinfo errno %{public}d %{public}s", errno, strerror(errno));
        context->SetErrorCode(errno);
        return false;
    }

    char host[MAX_HOST_LEN] = {0};
    if (res != nullptr) {
        if (getnameinfo(res->ai_addr, res->ai_addrlen, host, sizeof(host), nullptr, 0, 0) < 0) {
            context->SetErrorCode(errno);
            return false;
        }
        NETMANAGER_BASE_LOGI("host ip: %{public}s", host);

        NetAddress address;
        SetAddressInfo(host, res, address);

        context->addresses.emplace_back(address);
    }
    freeaddrinfo(res);
    return true;
}

napi_value ConnectionExec::NetHandleExec::GetAddressByNameCallback(GetAddressByNameContext *context)
{
    if (context->addresses.empty()) {
        return NapiUtils::GetUndefined(context->GetEnv());
    }
    return MakeNetAddressJsValue(context->GetEnv(), context->addresses[0]);
}

napi_value ConnectionExec::NetHandleExec::MakeNetAddressJsValue(napi_env env, const NetAddress &address)
{
    napi_value obj = NapiUtils::CreateObject(env);
    if (NapiUtils::GetValueType(env, obj) != napi_object) {
        return NapiUtils::GetUndefined(env);
    }

    NapiUtils::SetStringPropertyUtf8(env, obj, KEY_ADDRESS, address.GetAddress());
    NapiUtils::SetUint32Property(env, obj, KEY_FAMILY, address.GetJsValueFamily());
    NapiUtils::SetUint32Property(env, obj, KEY_PORT, address.GetPort());
    return obj;
}

void ConnectionExec::NetHandleExec::SetAddressInfo(const char *host, addrinfo *info, NetAddress &address)
{
    address.SetAddress(host);
    address.SetFamilyBySaFamily(info->ai_addr->sa_family);
    if (info->ai_addr->sa_family == AF_INET) {
        auto addr4 = reinterpret_cast<sockaddr_in *>(info->ai_addr);
        address.SetPort(addr4->sin_port);
    } else if (info->ai_addr->sa_family == AF_INET6) {
        auto addr6 = reinterpret_cast<sockaddr_in6 *>(info->ai_addr);
        address.SetPort(addr6->sin6_port);
    }
}

bool ConnectionExec::NetConnectionExec::ExecRegister(RegisterContext *context)
{
    NETMANAGER_BASE_LOGI("ConnectionExec::NetConnectionExec::ExecRegister");

    EventManager *manager = context->GetManager();
    auto conn = static_cast<NetConnection *>(manager->GetData());
    sptr<INetConnCallback> callback = conn->GetObserver();

    if (conn->hasNetSpecifier && conn->hasTimeout) {
        sptr<NetSpecifier> specifier = new NetSpecifier(conn->netSpecifier);
        int32_t ret =
            DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(specifier, callback, conn->timeout);
        NETMANAGER_BASE_LOGI("Register result hasNetSpecifier and hasTimeout %{public}d", ret);
        return ret == 0;
    }

    if (conn->hasNetSpecifier) {
        sptr<NetSpecifier> specifier = new NetSpecifier(conn->netSpecifier);
        int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(specifier, callback, 0);
        NETMANAGER_BASE_LOGI("Register result hasNetSpecifier %{public}d", ret);
        return ret == 0;
    }

    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(callback);
    NETMANAGER_BASE_LOGI("Register result %{public}d", ret);
    return ret == 0;
}

napi_value ConnectionExec::NetConnectionExec::RegisterCallback(RegisterContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::NetConnectionExec::ExecUnregister(UnregisterContext *context)
{
    NETMANAGER_BASE_LOGI("ConnectionExec::NetConnectionExec::ExecUnregister");

    EventManager *manager = context->GetManager();
    auto conn = static_cast<NetConnection *>(manager->GetData());
    sptr<INetConnCallback> callback = conn->GetObserver();

    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(callback);
    NETMANAGER_BASE_LOGI("Unregister result %{public}d", ret);
    return ret == 0;
}

napi_value ConnectionExec::NetConnectionExec::UnregisterCallback(RegisterContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}
} // namespace OHOS::NetManagerStandard