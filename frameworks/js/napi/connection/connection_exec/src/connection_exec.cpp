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

#include <netdb.h>

#include "connection_module.h"
#include "constant.h"
#include "netmanager_base_log.h"
#include "netmanager_base_napi_utils.h"
#include "securec.h"

static constexpr const int MAX_HOST_LEN = 256;

namespace OHOS::NetManagerBase {
bool ConnectionExec::ExecGetDefaultNet(GetDefaultNetContext *context)
{
    (void)context;

    return true;
}

napi_value ConnectionExec::GetDefaultNetCallback(GetDefaultNetContext *context)
{
    napi_value netHandle = NapiUtils::CreateObject(context->GetEnv());
    if (NapiUtils::GetValueType(context->GetEnv(), netHandle) != napi_object) {
        return NapiUtils::GetUndefined(context->GetEnv());
    }

    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_FUNCTION(ConnectionModule::NetHandle::FUNCTION_GET_ADDRESSES_BY_NAME,
                              ConnectionModule::NetHandle::GetAddressesByName),
        DECLARE_NAPI_FUNCTION(ConnectionModule::NetHandle::FUNCTION_GET_ADDRESS_BY_NAME,
                              ConnectionModule::NetHandle::GetAddressByName),
    };
    NapiUtils::DefineProperties(context->GetEnv(), netHandle, properties);
    return netHandle;
}

bool ConnectionExec::NetHandleExec::ExecGetAddressesByName(GetAddressByNameContext *context)
{
    addrinfo *res = nullptr;
    int status = getaddrinfo(context->host.c_str(), nullptr, nullptr, &res);
    if (status < 0) {
        NETMANAGER_BASE_LOGE("getaddrinfo errno %{public}d %{public}s", errno, strerror(errno));
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
        address.SetAddress(host);
        address.SetFamilyBySaFamily(tmp->ai_addr->sa_family);
        if (tmp->ai_addr->sa_family == AF_INET) {
            auto addr4 = reinterpret_cast<sockaddr_in *>(tmp->ai_addr);
            address.SetPort(addr4->sin_port);
        } else if (tmp->ai_addr->sa_family == AF_INET6) {
            auto addr6 = reinterpret_cast<sockaddr_in6 *>(tmp->ai_addr);
            address.SetPort(addr6->sin6_port);
        }

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
    return ExecGetAddressesByName(context);
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
} // namespace OHOS::NetManagerBase
