/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "bindsocket_context.h"
#include "connection_async_work.h"
#include "connection_exec.h"
#include "constant.h"
#include "getaddressbyname_context.h"
#include "getappnet_context.h"
#include "getdefaultnet_context.h"
#include "gethttpproxy_context.h"
#include "napi_constant.h"
#include "net_all_capabilities.h"
#include "netconnection.h"
#include "netinterface.h"
#include "netmanager_base_log.h"
#include "none_params_context.h"
#include "module_template.h"
#include "parse_nethandle_context.h"
#include "register_context.h"
#include "interfaceregister_context.h"
#include "setappnet_context.h"
#include "setglobalhttpproxy_context.h"
#include "setcustomdnsrule_context.h"
#include "deletecustomdnsrule_context.h"
#include "deletecustomdnsrules_context.h"
#include "getinterfaceconfig_context.h"
#include "registernetsupplier_context.h"
#include "unregisternetsupplier_context.h"

static constexpr const char *CONNECTION_MODULE_NAME = "net.connection";
static thread_local uint64_t g_moduleId;

#define DECLARE_NET_CAP(cap) \
    DECLARE_NAPI_STATIC_PROPERTY(#cap, NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetCap::cap)))

#define DECLARE_NET_BEAR_TYPE(type) \
    DECLARE_NAPI_STATIC_PROPERTY(#type, NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetBearType::type)))

namespace OHOS::NetManagerStandard {

template <typename T> static bool ParseTypesArray(napi_env env, napi_value obj, std::set<T> &typeArray,
    std::function<bool(uint32_t)> isValid)
{
    if (!NapiUtils::IsArray(env, obj)) {
        return false;
    }
    uint32_t arrayLength =
        NapiUtils::GetArrayLength(env, obj) > MAX_ARRAY_LENGTH ? MAX_ARRAY_LENGTH : NapiUtils::GetArrayLength(env, obj);
    for (uint32_t i = 0; i < arrayLength; ++i) {
        napi_value val = NapiUtils::GetArrayElement(env, obj, i);
        if (NapiUtils::GetValueType(env, val) == napi_number) {
            uint32_t value = NapiUtils::GetUint32FromValue(env, val);
            if (!isValid(value)) {
                NETMANAGER_BASE_LOGE("Invalid parameter value of array element!");
                return false;
            }
            typeArray.insert(static_cast<T>(value));
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
    (void)ParseTypesArray<NetCap>(env, networkCap, capabilities.netCaps_, [](uint32_t value) {
        return value >= 0 && value <= static_cast<uint32_t>(NetCap::NET_CAPABILITY_END);
    });

    napi_value bearerTypes = NapiUtils::GetNamedProperty(env, obj, KEY_BEARER_TYPE);
    bool ret = ParseTypesArray<NetBearType>(env, bearerTypes, capabilities.bearerTypes_, [](uint32_t value) {
        return value >= 0 && value <= static_cast<uint32_t>(NetBearType::BEARER_DEFAULT);
    });
    return ret;
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

static NetConnectionType GetNetConnectionType(napi_env env, size_t argc, napi_value *argv)
{
    if (argc == ARG_NUM_0) {
        return NetConnectionType::PARAMETER_ZERO;
    }
    if (argc == ARG_NUM_1) {
        if (NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) == napi_undefined) {
            return NetConnectionType::PARAMETER_ZERO;
        }
        if (NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) == napi_object) {
            return NetConnectionType::PARAMETER_SPECIFIER;
        }
        return NetConnectionType::PARAMETER_ERROR;
    }
    if (argc == ARG_NUM_2) {
        if (NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) == napi_object &&
            NapiUtils::GetValueType(env, argv[ARG_INDEX_1]) == napi_number) {
            return NetConnectionType::PARAMETER_TIMEOUT;
        }
        if (NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) == napi_undefined &&
            NapiUtils::GetValueType(env, argv[ARG_INDEX_1]) == napi_undefined) {
            return NetConnectionType::PARAMETER_ZERO;
        }
        if (NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) == napi_object &&
            NapiUtils::GetValueType(env, argv[ARG_INDEX_1]) == napi_undefined) {
            return NetConnectionType::PARAMETER_SPECIFIER;
        }
    }
    return NetConnectionType::PARAMETER_ERROR;
}

static void *ParseNetConnectionParams(napi_env env, size_t argc, napi_value *argv,
    std::shared_ptr<EventManager>& manager)
{
    std::unique_ptr<NetConnection, decltype(&NetConnection::DeleteNetConnection)> netConnection(
        NetConnection::MakeNetConnection(manager), NetConnection::DeleteNetConnection);
    netConnection->moduleId_ = g_moduleId;

    auto netConnType = GetNetConnectionType(env, argc, argv);

    switch (netConnType) {
        case NetConnectionType::PARAMETER_ZERO: {
            NETMANAGER_BASE_LOGI("ParseNetConnectionParams no params");
            return netConnection.release();
        }
        case NetConnectionType::PARAMETER_SPECIFIER: {
            if (!ParseNetSpecifier(env, argv[ARG_INDEX_0], netConnection->netSpecifier_)) {
                NETMANAGER_BASE_LOGE("ParseNetSpecifier failed");
                return nullptr;
            }
            netConnection->hasNetSpecifier_ = true;
            return netConnection.release();
        }
        case NetConnectionType::PARAMETER_TIMEOUT: {
            if (!ParseNetSpecifier(env, argv[ARG_INDEX_0], netConnection->netSpecifier_)) {
                NETMANAGER_BASE_LOGE("ParseNetSpecifier failed, do not use params");
                return nullptr;
            }
            netConnection->hasNetSpecifier_ = true;
            netConnection->hasTimeout_ = true;
            netConnection->timeout_ = NapiUtils::GetUint32FromValue(env, argv[ARG_INDEX_1]);
            return netConnection.release();
        }
        default:
            NETMANAGER_BASE_LOGE("constructor params invalid, should be none or specifier or specifier+timeout_");
            return nullptr;
    }
}

static void *ParseNetInterfaceParams(napi_env env, size_t argc, napi_value *argv,
    std::shared_ptr<EventManager>& manager)
{
    std::unique_ptr<NetInterface, decltype(&NetInterface::DeleteNetInterface)> netInterface(
        NetInterface::MakeNetInterface(manager), NetInterface::DeleteNetInterface);
    netInterface->moduleId_ = g_moduleId;

    if (argc == ARG_NUM_0) {
        return netInterface.release();
    }
    NETMANAGER_BASE_LOGE("constructor params invalid, should be none");
    return nullptr;
}

static void AddCleanupHook(napi_env env)
{
    NapiUtils::SetEnvValid(env);
    auto envWrapper = new (std::nothrow) napi_env;
    if (envWrapper == nullptr) {
        NETMANAGER_BASE_LOGE("EnvWrapper create fail!");
        return;
    }
    *envWrapper = env;
    napi_add_env_cleanup_hook(env, NapiUtils::HookForEnvCleanup, envWrapper);
}

std::initializer_list<napi_property_descriptor> ConnectionModule::createPropertyList()
{
    std::initializer_list<napi_property_descriptor> functions = {
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_DEFAULT_NET, GetDefaultNet),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_DEFAULT_NET_SYNC, GetDefaultNetSync),
        DECLARE_NAPI_FUNCTION(FUNCTION_CREATE_NET_CONNECTION, CreateNetConnection),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_ADDRESSES_BY_NAME, GetAddressesByName),
        DECLARE_NAPI_FUNCTION(FUNCTION_HAS_DEFAULT_NET, HasDefaultNet),
        DECLARE_NAPI_FUNCTION(FUNCTION_HAS_DEFAULT_NET_SYNC, HasDefaultNetSync),
        DECLARE_NAPI_FUNCTION(FUNCTION_IS_DEFAULT_NET_METERED, IsDefaultNetMetered),
        DECLARE_NAPI_FUNCTION(FUNCTION_IS_DEFAULT_NET_METERED_SYNC, IsDefaultNetMeteredSync),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_NET_CAPABILITIES, GetNetCapabilities),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_NET_CAPABILITIES_SYNC, GetNetCapabilitiesSync),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_CONNECTION_PROPERTIES, GetConnectionProperties),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_CONNECTION_PROPERTIES_SYNC, GetConnectionPropertiesSync),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_ALL_NETS, GetAllNets),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_ALL_NETS_SYNC, GetAllNetsSync),
        DECLARE_NAPI_FUNCTION(FUNCTION_ENABLE_AIRPLANE_MODE, EnableAirplaneMode),
        DECLARE_NAPI_FUNCTION(FUNCTION_DISABLE_AIRPLANE_MODE, DisableAirplaneMode),
        DECLARE_NAPI_FUNCTION(FUNCTION_REPORT_NET_CONNECTED, ReportNetConnected),
        DECLARE_NAPI_FUNCTION(FUNCTION_REPORT_NET_DISCONNECTED, ReportNetDisconnected),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_DEFAULT_HTTP_PROXY, GetDefaultHttpProxy),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_GLOBAL_HTTP_PROXY, GetGlobalHttpProxy),
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_GLOBAL_HTTP_PROXY, SetGlobalHttpProxy),
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_CUSTOM_DNS_RULE, AddCustomDnsRule),
        DECLARE_NAPI_FUNCTION(FUNCTION_DELETE_CUSTOM_DNS_RULE, RemoveCustomDnsRule),
        DECLARE_NAPI_FUNCTION(FUNCTION_DELETE_CUSTOM_DNS_RULES, ClearCustomDnsRules),
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_APP_HTTP_PROXY, SetAppHttpProxy),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_APP_NET, GetAppNet),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_APP_NET_SYNC, GetAppNetSync),
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_APP_NET, SetAppNet),
        DECLARE_NAPI_FUNCTION(FUNCTION_FACTORY_RESET_NETWORK, FactoryResetNetwork),
        DECLARE_NAPI_FUNCTION(FUNCTION_FACTORY_RESET_NETWORK_SYNC, FactoryResetNetworkSync),
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_PAC_URL, SetPacUrl),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_PAC_URL, GetPacUrl),
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_INTERFACE_UP, SetInterfaceUp),
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_INTERFACE_IP_ADDRESS, SetNetInterfaceIpAddress),
        DECLARE_NAPI_FUNCTION(FUNCTION_ADD_NETWORK_ROUTE, AddNetworkRoute),
        DECLARE_NAPI_FUNCTION(FUNCTION_CREATE_NET_INTERFACE, CreateNetInterface),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_INTERFACE_CONFIG, GetNetInterfaceConfiguration),
        DECLARE_NAPI_FUNCTION(FUNCTION_REGISTER_NET_SUPPLIER, RegisterNetSupplier),
        DECLARE_NAPI_FUNCTION(FUNCTION_UNREGISTER_NET_SUPPLIER, UnregisterNetSupplier),
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_NET_EXT_ATTRIBUTE, SetNetExtAttribute),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_NET_EXT_ATTRIBUTE, GetNetExtAttribute),
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_NET_EXT_ATTRIBUTE_SYNC, SetNetExtAttributeSync),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_NET_EXT_ATTRIBUTE_SYNC, GetNetExtAttributeSync),
    };
    return functions;
}

napi_value ConnectionModule::InitConnectionModule(napi_env env, napi_value exports)
{
    g_moduleId = NapiUtils::CreateUvHandlerQueue(env);
    std::initializer_list<napi_property_descriptor> functions = ConnectionModule::createPropertyList();
    NapiUtils::DefineProperties(env, exports, functions);
    InitClasses(env, exports);
    InitProperties(env, exports);
    AddCleanupHook(env);
    return exports;
}

void ConnectionModule::InitClasses(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> netConnectionFunctions = {
        DECLARE_NAPI_FUNCTION(NetConnectionInterface::FUNCTION_ON, NetConnectionInterface::On),
        DECLARE_NAPI_FUNCTION(NetConnectionInterface::FUNCTION_REGISTER, NetConnectionInterface::Register),
        DECLARE_NAPI_FUNCTION(NetConnectionInterface::FUNCTION_UNREGISTER, NetConnectionInterface::Unregister),
    };
    ModuleTemplate::DefineClass(env, exports, netConnectionFunctions, INTERFACE_NET_CONNECTION);

    std::initializer_list<napi_property_descriptor> netInterfaceFunctions = {
        DECLARE_NAPI_FUNCTION(NetInterfaceInterface::FUNCTION_ON, NetInterfaceInterface::On),
        DECLARE_NAPI_FUNCTION(NetInterfaceInterface::FUNCTION_REGISTER, NetInterfaceInterface::Register),
        DECLARE_NAPI_FUNCTION(NetInterfaceInterface::FUNCTION_UNREGISTER, NetInterfaceInterface::Unregister),
    };
    ModuleTemplate::DefineClass(env, exports, netInterfaceFunctions, INTERFACE_NET_INTERFACE);
}

void ConnectionModule::InitProperties(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> netCaps = {
        DECLARE_NET_CAP(NET_CAPABILITY_MMS),
        DECLARE_NET_CAP(NET_CAPABILITY_NOT_METERED),
        DECLARE_NET_CAP(NET_CAPABILITY_INTERNET),
        DECLARE_NET_CAP(NET_CAPABILITY_NOT_VPN),
        DECLARE_NET_CAP(NET_CAPABILITY_VALIDATED),
        DECLARE_NET_CAP(NET_CAPABILITY_PORTAL),
        DECLARE_NET_CAP(NET_CAPABILITY_INTERNAL_DEFAULT),
        DECLARE_NET_CAP(NET_CAPABILITY_CHECKING_CONNECTIVITY),
    };
    napi_value caps = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, caps, netCaps);
    NapiUtils::SetNamedProperty(env, exports, INTERFACE_NET_CAP, caps);

    std::initializer_list<napi_property_descriptor> netBearTypes = {
        DECLARE_NET_BEAR_TYPE(BEARER_CELLULAR),  DECLARE_NET_BEAR_TYPE(BEARER_WIFI),
        DECLARE_NET_BEAR_TYPE(BEARER_BLUETOOTH), DECLARE_NET_BEAR_TYPE(BEARER_ETHERNET),
        DECLARE_NET_BEAR_TYPE(BEARER_VPN),       DECLARE_NET_BEAR_TYPE(BEARER_WIFI_AWARE),
        DECLARE_NET_BEAR_TYPE(BEARER_DEFAULT),
    };
    napi_value types = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, types, netBearTypes);
    NapiUtils::SetNamedProperty(env, exports, INTERFACE_NET_BEAR_TYPE, types);
}

napi_value ConnectionModule::GetAddressesByName(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAddressByNameContext>(env, info, FUNCTION_GET_ADDRESSES_BY_NAME, nullptr,
                                                              ConnectionAsyncWork::ExecGetAddressesByName,
                                                              ConnectionAsyncWork::GetAddressesByNameCallback);
}

napi_value ConnectionModule::HasDefaultNet(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<HasDefaultNetContext>(env, info, FUNCTION_HAS_DEFAULT_NET, nullptr,
                                                           ConnectionAsyncWork::ExecHasDefaultNet,
                                                           ConnectionAsyncWork::HasDefaultNetCallback);
}

napi_value ConnectionModule::HasDefaultNetSync(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::InterfaceSync<HasDefaultNetContext>(env, info, FUNCTION_HAS_DEFAULT_NET, nullptr,
                                                               ConnectionExec::ExecHasDefaultNet,
                                                               ConnectionExec::HasDefaultNetCallback);
}

napi_value ConnectionModule::IsDefaultNetMetered(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<IsDefaultNetMeteredContext>(env, info, FUNCTION_IS_DEFAULT_NET_METERED, nullptr,
                                                                 ConnectionAsyncWork::ExecIsDefaultNetMetered,
                                                                 ConnectionAsyncWork::IsDefaultNetMeteredCallback);
}

napi_value ConnectionModule::IsDefaultNetMeteredSync(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::InterfaceSync<IsDefaultNetMeteredContext>(env, info, FUNCTION_IS_DEFAULT_NET_METERED,
        nullptr, ConnectionExec::ExecIsDefaultNetMetered, ConnectionExec::IsDefaultNetMeteredCallback);
}

napi_value ConnectionModule::GetNetCapabilities(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetNetCapabilitiesContext>(env, info, FUNCTION_GET_NET_CAPABILITIES, nullptr,
                                                                ConnectionAsyncWork::ExecGetNetCapabilities,
                                                                ConnectionAsyncWork::GetNetCapabilitiesCallback);
}

napi_value ConnectionModule::GetNetCapabilitiesSync(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::InterfaceSync<GetNetCapabilitiesContext>(env, info, FUNCTION_GET_NET_CAPABILITIES, nullptr,
                                                                    ConnectionExec::ExecGetNetCapabilities,
                                                                    ConnectionExec::GetNetCapabilitiesCallback);
}

napi_value ConnectionModule::GetConnectionProperties(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetConnectionPropertiesContext>(
        env, info, FUNCTION_GET_CONNECTION_PROPERTIES, nullptr, ConnectionAsyncWork::ExecGetConnectionProperties,
        ConnectionAsyncWork::GetConnectionPropertiesCallback);
}

napi_value ConnectionModule::GetConnectionPropertiesSync(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::InterfaceSync<GetConnectionPropertiesContext>(
        env, info, FUNCTION_GET_CONNECTION_PROPERTIES, nullptr, ConnectionExec::ExecGetConnectionProperties,
        ConnectionExec::GetConnectionPropertiesCallback);
}

napi_value ConnectionModule::CreateNetConnection(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::NewInstance(env, info, INTERFACE_NET_CONNECTION, ParseNetConnectionParams,
        [](napi_env, void *data, void *) {
            NETMANAGER_BASE_LOGI("finalize netConnection");
            auto sharedManager = static_cast<std::shared_ptr<EventManager> *>(data);
            if (sharedManager == nullptr || *sharedManager == nullptr) {
                return;
            }
            auto manager = *sharedManager;
            auto netConnection = static_cast<NetConnection *>(manager->GetData());
            delete sharedManager;
            NetConnection::DeleteNetConnection(netConnection);
        });
}

napi_value ConnectionModule::CreateNetInterface(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::NewInstance(env, info, INTERFACE_NET_INTERFACE, ParseNetInterfaceParams,
        [](napi_env, void *data, void *) {
            NETMANAGER_BASE_LOGI("finalize netInterface");
            auto sharedManager = static_cast<std::shared_ptr<EventManager> *>(data);
            if (sharedManager == nullptr || *sharedManager == nullptr) {
                return;
            }
            auto manager = *sharedManager;
            auto netInterface = static_cast<NetInterface *>(manager->GetData());
            delete sharedManager;
            NetInterface::DeleteNetInterface(netInterface);
        });
}

napi_value ConnectionModule::GetDefaultNet(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetDefaultNetContext>(env, info, FUNCTION_GET_DEFAULT_NET, nullptr,
                                                           ConnectionAsyncWork::ExecGetDefaultNet,
                                                           ConnectionAsyncWork::GetDefaultNetCallback);
}

napi_value ConnectionModule::GetDefaultNetSync(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::InterfaceSync<GetDefaultNetContext>(env, info, FUNCTION_GET_DEFAULT_NET, nullptr,
                                                               ConnectionExec::ExecGetDefaultNet,
                                                               ConnectionExec::GetDefaultNetCallback);
}

napi_value ConnectionModule::GetAllNets(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAllNetsContext>(env, info, FUNCTION_GET_ALL_NETS, nullptr,
                                                        ConnectionAsyncWork::ExecGetAllNets,
                                                        ConnectionAsyncWork::GetAllNetsCallback);
}

napi_value ConnectionModule::GetAllNetsSync(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::InterfaceSync<GetAllNetsContext>(env, info, FUNCTION_GET_ALL_NETS, nullptr,
                                                            ConnectionExec::ExecGetAllNets,
                                                            ConnectionExec::GetAllNetsCallback);
}

napi_value ConnectionModule::EnableAirplaneMode(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<EnableAirplaneModeContext>(env, info, FUNCTION_ENABLE_AIRPLANE_MODE, nullptr,
                                                                ConnectionAsyncWork::ExecEnableAirplaneMode,
                                                                ConnectionAsyncWork::EnableAirplaneModeCallback);
}

napi_value ConnectionModule::DisableAirplaneMode(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<DisableAirplaneModeContext>(env, info, FUNCTION_DISABLE_AIRPLANE_MODE, nullptr,
                                                                 ConnectionAsyncWork::ExecDisableAirplaneMode,
                                                                 ConnectionAsyncWork::DisableAirplaneModeCallback);
}

napi_value ConnectionModule::ReportNetConnected(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<ReportNetConnectedContext>(env, info, FUNCTION_REPORT_NET_CONNECTED, nullptr,
                                                                ConnectionAsyncWork::ExecReportNetConnected,
                                                                ConnectionAsyncWork::ReportNetConnectedCallback);
}

napi_value ConnectionModule::ReportNetDisconnected(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<ReportNetDisconnectedContext>(env, info, FUNCTION_REPORT_NET_DISCONNECTED, nullptr,
                                                                   ConnectionAsyncWork::ExecReportNetDisconnected,
                                                                   ConnectionAsyncWork::ReportNetDisconnectedCallback);
}

napi_value ConnectionModule::GetDefaultHttpProxy(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetHttpProxyContext>(env, info, FUNCTION_GET_DEFAULT_HTTP_PROXY, nullptr,
                                                          ConnectionAsyncWork::ExecGetDefaultHttpProxy,
                                                          ConnectionAsyncWork::GetDefaultHttpProxyCallback);
}

napi_value ConnectionModule::GetGlobalHttpProxy(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetHttpProxyContext>(env, info, FUNCTION_GET_GLOBAL_HTTP_PROXY, nullptr,
                                                          ConnectionAsyncWork::ExecGetGlobalHttpProxy,
                                                          ConnectionAsyncWork::GetGlobalHttpProxyCallback);
}

napi_value ConnectionModule::SetGlobalHttpProxy(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<SetGlobalHttpProxyContext>(env, info, FUNCTION_SET_GLOBAL_HTTP_PROXY, nullptr,
                                                                ConnectionAsyncWork::ExecSetGlobalHttpProxy,
                                                                ConnectionAsyncWork::SetGlobalHttpProxyCallback);
}

napi_value ConnectionModule::SetAppHttpProxy(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::InterfaceSync<SetAppHttpProxyContext>(env, info, FUNCTION_SET_APP_HTTP_PROXY, nullptr,
                                                                 ConnectionExec::ExecSetAppHttpProxy,
                                                                 ConnectionExec::SetAppHttpProxyCallback);
}

napi_value ConnectionModule::GetAppNet(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAppNetContext>(env, info, FUNCTION_GET_APP_NET, nullptr,
                                                       ConnectionAsyncWork::ExecGetAppNet,
                                                       ConnectionAsyncWork::GetAppNetCallback);
}

napi_value ConnectionModule::GetAppNetSync(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::InterfaceSync<GetAppNetContext>(env, info, FUNCTION_GET_APP_NET, nullptr,
                                                           ConnectionExec::ExecGetAppNet,
                                                           ConnectionExec::GetAppNetCallback);
}

napi_value ConnectionModule::SetAppNet(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<SetAppNetContext>(env, info, FUNCTION_SET_APP_NET, nullptr,
                                                       ConnectionAsyncWork::ExecSetAppNet,
                                                       ConnectionAsyncWork::SetAppNetCallback);
}

napi_value ConnectionModule::SetInterfaceUp(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<SetInterfaceUpContext>(env, info, FUNCTION_SET_INTERFACE_UP, nullptr,
                                                            ConnectionAsyncWork::ExecSetInterfaceUp,
                                                            ConnectionAsyncWork::SetInterfaceUpCallback);
}

napi_value ConnectionModule::SetNetInterfaceIpAddress(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<SetInterfaceIpAddrContext>(env, info, FUNCTION_SET_INTERFACE_IP_ADDRESS, nullptr,
                                                                ConnectionAsyncWork::ExecSetInterfaceIpAddr,
                                                                ConnectionAsyncWork::SetInterfaceIpAddrCallback);
}

napi_value ConnectionModule::AddNetworkRoute(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<AddNetworkRouteContext>(env, info, FUNCTION_ADD_NETWORK_ROUTE, nullptr,
                                                             ConnectionAsyncWork::ExecAddNetworkRoute,
                                                             ConnectionAsyncWork::AddNetworkRouteCallback);
}

napi_value ConnectionModule::GetNetInterfaceConfiguration(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetNetInterfaceConfigurationContext>(
        env, info, FUNCTION_GET_INTERFACE_CONFIG, nullptr,
        ConnectionAsyncWork::ExecGetNetInterfaceConfiguration,
        ConnectionAsyncWork::GetNetInterfaceConfigurationCallback);
}

napi_value ConnectionModule::RegisterNetSupplier(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<RegisterNetSupplierContext>(
        env, info, FUNCTION_REGISTER_NET_SUPPLIER, nullptr,
        ConnectionAsyncWork::ExecRegisterNetSupplier,
        ConnectionAsyncWork::RegisterNetSupplierCallback);
}

napi_value ConnectionModule::UnregisterNetSupplier(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<UnregisterNetSupplierContext>(
        env, info, FUNCTION_UNREGISTER_NET_SUPPLIER, nullptr,
        ConnectionAsyncWork::ExecUnregisterNetSupplier,
        ConnectionAsyncWork::UnregisterNetSupplierCallback);
}

napi_value ConnectionModule::AddCustomDnsRule(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<SetCustomDNSRuleContext>(env, info, FUNCTION_SET_CUSTOM_DNS_RULE, nullptr,
                                                              ConnectionAsyncWork::ExecSetCustomDNSRule,
                                                              ConnectionAsyncWork::SetCustomDNSRuleCallback);
}

napi_value ConnectionModule::RemoveCustomDnsRule(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<DeleteCustomDNSRuleContext>(env, info, FUNCTION_DELETE_CUSTOM_DNS_RULE, nullptr,
                                                                 ConnectionAsyncWork::ExecDeleteCustomDNSRule,
                                                                 ConnectionAsyncWork::DeleteCustomDNSRuleCallback);
}

napi_value ConnectionModule::ClearCustomDnsRules(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<DeleteCustomDNSRulesContext>(env, info, FUNCTION_DELETE_CUSTOM_DNS_RULES, nullptr,
                                                                 ConnectionAsyncWork::ExecDeleteCustomDNSRules,
                                                                 ConnectionAsyncWork::DeleteCustomDNSRulesCallback);
}

napi_value ConnectionModule::FactoryResetNetwork(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<FactoryResetNetworkContext>(env, info, FUNCTION_FACTORY_RESET_NETWORK, nullptr,
                                                                 ConnectionAsyncWork::ExecFactoryResetNetwork,
                                                                 ConnectionAsyncWork::FactoryResetNetworkCallback);
}

napi_value ConnectionModule::FactoryResetNetworkSync(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::InterfaceSync<FactoryResetNetworkContext>(env, info, FUNCTION_FACTORY_RESET_NETWORK, nullptr,
                                                                     ConnectionExec::ExecFactoryResetNetwork,
                                                                     ConnectionExec::FactoryResetNetworkCallback);
}

napi_value ConnectionModule::SetPacUrl(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::InterfaceSync<SetPacUrlContext>(env, info, FUNCTION_SET_PAC_URL, nullptr,
                                                                     ConnectionExec::ExecSetPacUrl,
                                                                     ConnectionExec::SetPacUrlCallback);
}

napi_value ConnectionModule::GetPacUrl(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::InterfaceSync<GetPacUrlContext>(env, info, FUNCTION_GET_PAC_URL, nullptr,
                                                                     ConnectionExec::ExecGetPacUrl,
                                                                     ConnectionExec::GetPacUrlCallback);
}

napi_value ConnectionModule::GetNetExtAttributeSync(napi_env env, napi_callback_info info)
{
    NETMANAGER_BASE_LOGI("js invoke getNetExtAttributeSync");
    return ModuleTemplate::InterfaceSync<GetNetExtAttributeContext>(env, info, FUNCTION_GET_NET_EXT_ATTRIBUTE_SYNC,
        nullptr, ConnectionExec::ExecGetNetExtAttribute, ConnectionExec::GetNetExtAttributeCallback);
}

napi_value ConnectionModule::SetNetExtAttributeSync(napi_env env, napi_callback_info info)
{
    NETMANAGER_BASE_LOGI("js invoke setNetExtAttributeSync");
    return ModuleTemplate::InterfaceSync<SetNetExtAttributeContext>(env, info, FUNCTION_SET_NET_EXT_ATTRIBUTE_SYNC,
        nullptr, ConnectionExec::ExecSetNetExtAttribute, ConnectionExec::SetNetExtAttributeCallback);
}

napi_value ConnectionModule::GetNetExtAttribute(napi_env env, napi_callback_info info)
{
    NETMANAGER_BASE_LOGI("js invoke getNetExtAttribute");
    return ModuleTemplate::Interface<GetNetExtAttributeContext>(env, info, FUNCTION_GET_NET_EXT_ATTRIBUTE, nullptr,
                                                                 ConnectionAsyncWork::ExecGetNetExtAttribute,
                                                                 ConnectionAsyncWork::GetNetExtAttributeCallback);
}

napi_value ConnectionModule::SetNetExtAttribute(napi_env env, napi_callback_info info)
{
    NETMANAGER_BASE_LOGI("js invoke setNetExtAttribute");
    return ModuleTemplate::Interface<SetNetExtAttributeContext>(env, info, FUNCTION_SET_NET_EXT_ATTRIBUTE, nullptr,
                                                                 ConnectionAsyncWork::ExecSetNetExtAttribute,
                                                                 ConnectionAsyncWork::SetNetExtAttributeCallback);
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
    return ModuleTemplate::Interface<RegisterContext>(
        env, info, FUNCTION_REGISTER,
        [](napi_env theEnv, napi_value thisVal, RegisterContext *context) -> bool {
            if (context && context->GetManager() && !context->GetManager()->GetRef()) {
                context->GetManager()->SetRef(NapiUtils::CreateReference(theEnv, thisVal));
            }
            return true;
        },
        ConnectionAsyncWork::NetConnectionAsyncWork::ExecRegister,
        ConnectionAsyncWork::NetConnectionAsyncWork::RegisterCallback);
}

napi_value ConnectionModule::NetConnectionInterface::Unregister(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<UnregisterContext>(
        env, info, FUNCTION_UNREGISTER,
        [](napi_env theEnv, napi_value thisVal, UnregisterContext *context) -> bool {
            if (context && context->GetManager()) {
                if (context->GetManager()->GetRef()) {
                    NapiUtils::DeleteReference(theEnv, context->GetManager()->GetRef());
                    context->GetManager()->SetRef(nullptr);
                }
                context->GetManager()->DeleteAllListener();
            }
            return true;
        },
        ConnectionAsyncWork::NetConnectionAsyncWork::ExecUnregister,
        ConnectionAsyncWork::NetConnectionAsyncWork::UnregisterCallback);
}

napi_value ConnectionModule::NetInterfaceInterface::On(napi_env env, napi_callback_info info)
{
    std::initializer_list<std::string> events = {EVENT_IFACE_ADDRESS_UPDATED,
                                                 EVENT_IFACE_ADDRESS_REMOVED,
                                                 EVENT_IFACE_ADDED,
                                                 EVENT_IFACE_REMOVED,
                                                 EVENT_IFACE_CHANGED,
                                                 EVENT_IFACE_LINK_STATE_CHANGED,
                                                 EVENT_IFACE_ROUTE_CHANGED};
    return ModuleTemplate::On(env, info, events, false);
}

napi_value ConnectionModule::NetInterfaceInterface::Register(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<IfaceRegisterContext>(
        env, info, FUNCTION_REGISTER,
        [](napi_env theEnv, napi_value thisVal, IfaceRegisterContext *context) -> bool {
            if (context && context->GetManager() && !context->GetManager()->GetRef()) {
                context->GetManager()->SetRef(NapiUtils::CreateReference(theEnv, thisVal));
            }
            return true;
        },
        ConnectionAsyncWork::NetInterfaceAsyncWork::ExecIfaceRegister,
        ConnectionAsyncWork::NetInterfaceAsyncWork::IfaceRegisterCallback);
}

napi_value ConnectionModule::NetInterfaceInterface::Unregister(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<IfaceUnregisterContext>(
        env, info, FUNCTION_UNREGISTER,
        [](napi_env theEnv, napi_value thisVal, IfaceUnregisterContext *context) -> bool {
            if (context && context->GetManager()) {
                if (context->GetManager()->GetRef()) {
                    NapiUtils::DeleteReference(theEnv, context->GetManager()->GetRef());
                    context->GetManager()->SetRef(nullptr);
                }
                context->GetManager()->DeleteAllListener();
            }
            return true;
        },
        ConnectionAsyncWork::NetInterfaceAsyncWork::ExecIfaceUnregister,
        ConnectionAsyncWork::NetInterfaceAsyncWork::IfaceUnregisterCallback);
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
