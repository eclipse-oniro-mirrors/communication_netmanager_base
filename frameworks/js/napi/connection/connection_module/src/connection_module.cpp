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

#include "connection_module.h"

#include "connection_async_work.h"
#include "constant.h"
#include "getaddressbyname_context.h"
#include "getdefaultnet_context.h"
#include "net_all_capabilities.h"
#include "netconnection.h"
#include "netmanager_base_log.h"
#include "netmanager_base_module_template.h"
#include "register_context.h"

static constexpr const char *CONNECTION_MODULE_NAME = "net.connection";

namespace OHOS::NetManagerStandard {

template <typename T> static bool ParseTypesArray(napi_env env, napi_value obj, std::set<T> &typeArray)
{
    if (!NapiUtils::IsArray(env, obj)) {
        return false;
    }

    for (uint32_t i = 0; i < NapiUtils::GetArrayLength(env, obj); ++i) {
        napi_value val = NapiUtils::GetArrayElement(env, obj, i);
        if (NapiUtils::GetValueType(env, val) == napi_number) {
            typeArray.insert(static_cast<T>(NapiUtils::GetUint32FromValue(env, val)));
        } else {
            NETMANAGER_BASE_LOGE("Invalid parameter type of array element!");
            return false;
        }
    }
    return true;
}

static bool ParseCapabilities(napi_env env, napi_value obj, NetAllCapabilities &capabilities)
{
    if (NapiUtils::GetValueType(env, obj) != napi_object) {
        return false;
    }

    capabilities.linkUpBandwidthKbps_ = NapiUtils::GetUint32Property(env, obj, KEY_LINK_UP_BAND_WIDTH_KPS);
    capabilities.linkDownBandwidthKbps_ = NapiUtils::GetUint32Property(env, obj, KEY_LINK_DOWN_BAND_WIDTH_KPS);

    napi_value networkCap = NapiUtils::GetNamedProperty(env, obj, KEY_NETWORK_CAP);
    (void)ParseTypesArray<NetCap>(env, networkCap, capabilities.netCaps_);

    napi_value bearerTypes = NapiUtils::GetNamedProperty(env, obj, KEY_BEARER_TYPE);
    if (!ParseTypesArray<NetBearType>(env, bearerTypes, capabilities.bearerTypes_)) {
        return false;
    }

    return true;
}

static bool ParseNetSpecifier(napi_env env, napi_value obj, NetSpecifier &specifier)
{
    napi_value capabilitiesObj = NapiUtils::GetNamedProperty(env, obj, KEY_NET_CAPABILITIES);
    if (!ParseCapabilities(env, capabilitiesObj, specifier.netCapabilities_)) {
        return false;
    }
    specifier.ident_ = NapiUtils::GetStringPropertyUtf8(env, obj, KEY_BEARER_PRIVATE_IDENTIFIER);
    return true;
}

static void *ParseNetConnectionParams(napi_env env, size_t argc, napi_value *argv, EventManager *manager)
{
    std::unique_ptr<NetConnection, decltype(&NetConnection::DeleteNetConnection)> netConnection(
        NetConnection::MakeNetConnection(manager), NetConnection::DeleteNetConnection);

    if (argc == ARG_NUM_0) {
        NETMANAGER_BASE_LOGI("ParseNetConnectionParams no params");
        return netConnection.release();
    }

    if (argc == ARG_NUM_1 && NapiUtils::GetValueType(env, argv[0]) == napi_object) {
        if (!ParseNetSpecifier(env, argv[0], netConnection->netSpecifier)) {
            NETMANAGER_BASE_LOGE("ParseNetSpecifier failed");
            return nullptr;
        }
        netConnection->hasNetSpecifier = true;
        return netConnection.release();
    }

    if (argc == ARG_NUM_2 && NapiUtils::GetValueType(env, argv[0]) == napi_object &&
        NapiUtils::GetValueType(env, argv[1]) == napi_number) {
        if (!ParseNetSpecifier(env, argv[0], netConnection->netSpecifier)) {
            NETMANAGER_BASE_LOGE("ParseNetSpecifier failed, do not use params");
            return nullptr;
        }
        netConnection->hasNetSpecifier = true;
        netConnection->hasTimeout = true;
        netConnection->timeout = NapiUtils::GetUint32FromValue(env, argv[1]);
        return netConnection.release();
    }

    NETMANAGER_BASE_LOGE("constructor params invalid, should be none or specifier or specifier+timeout");
    return nullptr;
}

napi_value ConnectionModule::InitConnectionModule(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> functions = {
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_DEFAULT_NET, GetDefaultNet),
        DECLARE_NAPI_FUNCTION(FUNCTION_CREATE_NET_CONNECTION, CreateNetConnection),
    };
    NapiUtils::DefineProperties(env, exports, functions);

    std::initializer_list<napi_property_descriptor> netConnectionFunctions = {
        DECLARE_NAPI_FUNCTION(NetConnectionInterface::FUNCTION_ON, NetConnectionInterface::On),
        DECLARE_NAPI_FUNCTION(NetConnectionInterface::FUNCTION_REGISTER, NetConnectionInterface::Register),
        DECLARE_NAPI_FUNCTION(NetConnectionInterface::FUNCTION_UNREGISTER, NetConnectionInterface::Unregister),
    };
    ModuleTemplate::DefineClass(env, exports, netConnectionFunctions, INTERFACE_NET_CONNECTION);

    return exports;
}

napi_value ConnectionModule::CreateNetConnection(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::NewInstance(env, info, INTERFACE_NET_CONNECTION, ParseNetConnectionParams,
                                       [](napi_env, void *data, void *) {
                                           auto manager = static_cast<EventManager *>(data);
                                           auto netConnection = static_cast<NetConnection *>(manager->GetData());
                                           delete manager;
                                           NetConnection::DeleteNetConnection(netConnection);
                                       });
}

napi_value ConnectionModule::GetDefaultNet(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetDefaultNetContext>(env, info, FUNCTION_GET_DEFAULT_NET, nullptr,
                                                           ConnectionAsyncWork::ExecGetDefaultNet,
                                                           ConnectionAsyncWork::GetDefaultNetCallback);
}

napi_value ConnectionModule::NetHandleInterface::GetAddressesByName(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAddressByNameContext>(
        env, info, FUNCTION_GET_ADDRESSES_BY_NAME, nullptr,
        ConnectionAsyncWork::NetHandleAsyncWork::ExecGetAddressesByName,
        ConnectionAsyncWork::NetHandleAsyncWork::GetAddressesByNameCallback);
}

napi_value ConnectionModule::NetHandleInterface::GetAddressByName(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAddressByNameContext>(
        env, info, FUNCTION_GET_ADDRESSES_BY_NAME, nullptr,
        ConnectionAsyncWork::NetHandleAsyncWork::ExecGetAddressByName,
        ConnectionAsyncWork::NetHandleAsyncWork::GetAddressByNameCallback);
}

napi_value ConnectionModule::NetConnectionInterface::On(napi_env env, napi_callback_info info)
{
    std::initializer_list<std::string> events = {EVENT_NET_AVAILABLE,
                                                 EVENT_NET_BLOCK_STATUS_CHANGE,
                                                 EVENT_NET_CAPABILITIES_CHANGE,
                                                 EVENT_NET_CONNECTION_PROPERTIES_CHANGE,
                                                 EVENT_NET_LOST,
                                                 EVENT_NET_UNAVAILABLE};
    return ModuleTemplate::On(env, info, events, false);
}

napi_value ConnectionModule::NetConnectionInterface::Register(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<RegisterContext>(env, info, FUNCTION_REGISTER, nullptr,
                                                      ConnectionAsyncWork::NetConnectionAsyncWork::ExecRegister,
                                                      ConnectionAsyncWork::NetConnectionAsyncWork::RegisterCallback);
}

napi_value ConnectionModule::NetConnectionInterface::Unregister(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<UnregisterContext>(
        env, info, FUNCTION_UNREGISTER, nullptr, ConnectionAsyncWork::NetConnectionAsyncWork::ExecUnregister,
        ConnectionAsyncWork::NetConnectionAsyncWork::UnregisterCallback);
}

static napi_module g_connectionModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ConnectionModule::InitConnectionModule,
    .nm_modname = CONNECTION_MODULE_NAME,
    .nm_priv = nullptr,
    .reserved = {nullptr},
};

extern "C" __attribute__((constructor)) void RegisterConnectionModule(void)
{
    napi_module_register(&g_connectionModule);
}
} // namespace OHOS::NetManagerStandard