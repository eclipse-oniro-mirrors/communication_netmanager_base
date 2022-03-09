/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "napi_net_conn.h"
#include <memory>
#include "dns_resolver_client.h"
#include "base_context.h"
#include "event_context.h"
#include "napi_common.h"
#include "napi_net_connection.h"
#include "net_conn_constants.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
napi_value netConnectionObject;
template<typename T>
napi_value ParseTypesArray(napi_env env, napi_value obj, std::set<T> &typeArray)
{
    bool result = false;
    napi_status status = napi_is_array(env, obj, &result);
    if (status != napi_ok || !result) {
        NETMGR_LOG_E("Invalid input parameter type!");
        return nullptr;
    }

    napi_value elementValue = nullptr;
    int32_t element = ERROR_DEFAULT;
    uint32_t arrayLength = 0;
    NAPI_CALL(env, napi_get_array_length(env, obj, &arrayLength));
    for (uint32_t i = 0; i < arrayLength; ++i) {
        NAPI_CALL(env, napi_get_element(env, obj, i, &elementValue));
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, elementValue, &valueType);
        if (valueType == napi_number) {
            NAPI_CALL(env, napi_get_value_int32(env, elementValue, &element));
            typeArray.insert(static_cast<T>(element));
        } else {
            NETMGR_LOG_E("Invalid parameter type of array element!");
            return nullptr;
        }
    }
    return NapiCommon::CreateUndefined(env);
}

napi_value ParseCapabilities(napi_env env, napi_value obj, NetAllCapabilities &capabilities)
{
    capabilities.linkUpBandwidthKbps_ = NapiCommon::GetNapiInt32Value(env, obj, "linkUpBandwidthKbps");
    capabilities.linkDownBandwidthKbps_ = NapiCommon::GetNapiInt32Value(env, obj, "linkDownBandwidthKbps");

    napi_value networkCap = NapiCommon::GetNamedProperty(env, obj, "networkCap");
    if (networkCap) {
        if (ParseTypesArray(env, networkCap, capabilities.netCaps_) == nullptr) {
            return nullptr;
        }
    }

    napi_value bearerTypes = NapiCommon::GetNamedProperty(env, obj, "bearerTypes");
    if (bearerTypes) {
        if (ParseTypesArray(env, bearerTypes, capabilities.bearerTypes_) == nullptr) {
            return nullptr;
        }
    }
    return NapiCommon::CreateUndefined(env);
}

napi_status ParseNetSpecifier(napi_env env, napi_value obj, NetSpecifier &specifier)
{
    napi_value capabilitiesObj = NapiCommon::GetNamedProperty(env, obj, "netCapabilities");
    if (capabilitiesObj) {
        napi_value result = ParseCapabilities(env, capabilitiesObj, specifier.netCapabilities_);
        if (result == nullptr) {
            return napi_invalid_arg;
        }
    }
    specifier.ident_ = NapiCommon::GetNapiStringValue(env, obj, "bearerPrivateIdentifier");
    return napi_ok;
}

napi_value JS_Constructor(napi_env env, napi_callback_info cbinfo)
{
    NETMGR_LOG_I("netConnection JS_Constructor");
    size_t argc = 2;
    napi_value argv[] = {nullptr, nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));

    NapiNetConnection *netConnection = new NapiNetConnection();
    if (argc == ARGV_INDEX_1) {
        napi_valuetype valueType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[ARGV_INDEX_0], &valueType));
        if (valueType == napi_object) {
            if (ParseNetSpecifier(env, argv[ARGV_INDEX_0], netConnection->netSpecifier_) == napi_ok) {
                netConnection->timeout_ = 0;
                netConnection->hasSpecifier = true;
                netConnection->hasTimeout = true;
            }
            NETMGR_LOG_I("netConnection hasSpecifier:%{public}d, hasTimeout:%{public}d",
                netConnection->hasSpecifier, netConnection->hasTimeout);
        } else if (valueType == napi_number) {
            std::string msg("The parameter 'timeout' is only valid when the parameter 'netSpecifier' is input!");
            NETMGR_LOG_E("%{public}s", msg.c_str());
            napi_throw_error(env, "1", msg.c_str());
            return nullptr;
        } else {
            NETMGR_LOG_E("invalid data type!");
            return nullptr;
        }
    } else if (argc == ARGV_INDEX_2) {
        NAPI_CALL(env, ParseNetSpecifier(env, argv[ARGV_INDEX_0], netConnection->netSpecifier_));
        NAPI_CALL(env, napi_get_value_uint32(env, argv[ARGV_INDEX_1], &netConnection->timeout_));
        netConnection->hasSpecifier = true;
        netConnection->hasTimeout = true;
    } else {
        if (argc != 0) {
            NETMGR_LOG_E("Invalid number of arguments");
            return nullptr;
        }
    }

    napi_wrap(
        env, thisVar, netConnection,
        [](napi_env env, void *data, void *hint) {
            NapiNetConnection *netConnection = (NapiNetConnection *)data;
            delete netConnection;
        },
        nullptr, nullptr);

    return thisVar;
}

napi_value RegisternetConnectionObject(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("register", NapiNetConnection::Register),
        DECLARE_NAPI_FUNCTION("unregister", NapiNetConnection::Unregister),
        DECLARE_NAPI_FUNCTION("on", NapiNetConnection::On),
    };

    NAPI_CALL(env,
        napi_define_class(env, "NetConnection", NAPI_AUTO_LENGTH, JS_Constructor, nullptr,
            sizeof(desc) / sizeof(desc[0]), desc, &netConnectionObject));
    return exports;
}

napi_value CreateNetConnection(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("netConnection CreateNetConnection");
    std::size_t argc = 2;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_new_instance(env, netConnectionObject, argc, argv, &result));

    return result;
}
} // namespace
NapiNetConn::NapiNetConn() {}

napi_value NapiNetConn::DeclareNetConnInterface(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getDefaultNet", GetDefaultNet),
        DECLARE_NAPI_FUNCTION("getAllNets", GetAllNets),
        DECLARE_NAPI_FUNCTION("getConnectionProperties", GetConnectionProperties),
        DECLARE_NAPI_FUNCTION("getNetCapabilities", GetNetCapabilities),
        DECLARE_NAPI_FUNCTION("hasDefaultNet", HasDefaultNet),
        DECLARE_NAPI_FUNCTION("enableAirplaneMode", EnableAirplaneMode),
        DECLARE_NAPI_FUNCTION("disableAirplaneMode", DisableAirplaneMode),
        DECLARE_NAPI_FUNCTION("reportNetConnected", NetDetection),
        DECLARE_NAPI_FUNCTION("reportNetDisconnected", NetDetection),
        DECLARE_NAPI_FUNCTION("createNetConnection", CreateNetConnection),
        DECLARE_NAPI_FUNCTION("getAddressesByName", GetAddressesByName),
        DECLARE_NAPI_FUNCTION("restoreFactoryData", RestoreFactoryData),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiNetConn::DeclareNetConnNew(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("DeclareNetConnNew");
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    int32_t netId = 0;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc == ARGV_NUM_1) {
        napi_get_value_int32(env, argv[ARGV_INDEX_0], &netId);
    }
    napi_value target = nullptr;
    napi_get_new_target(env, info, &target);
    if (target == nullptr) {
        NETMGR_LOG_E("Failed to get target.");
        return nullptr;
    }
    NapiCommon::SetPropertyInt32(env, thisVar, "netId", netId);
    sptr<NetHandle> *handlerPtr = new sptr<NetHandle>(std::make_unique<NetHandle>(netId).release());
    napi_status status = napi_wrap(env, thisVar, reinterpret_cast<void *>(handlerPtr),
        NapiNetConn::DeclareNetConnDestructor, nullptr, nullptr);
    if (status != napi_ok) {
        NETMGR_LOG_E("Failed to wrap DeclareNetConnNew.");
        delete handlerPtr;
        return nullptr;
    }
    return thisVar;
}

napi_value NapiNetConn::DeclareNetConnConstructor(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("bindSocket", BindSocket),
        DECLARE_NAPI_FUNCTION("getAddressesByName", GetAddressesByName),
        DECLARE_NAPI_FUNCTION("getAddressByName", GetAddressByName),
    };
    napi_value constructor;
    napi_status status = napi_define_class(env, "NetHandle", NAPI_AUTO_LENGTH, DeclareNetConnNew, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        NETMGR_LOG_E("Failed to define class at Init");
        return nullptr;
    }
    status = napi_set_named_property(env, exports, "NetHandle", constructor);
    if (status != napi_ok) {
        NETMGR_LOG_E("Failed to set property at init");
        return nullptr;
    }
    napi_value netId;
    napi_create_int32(env, 0, &netId);
    if (napi_set_named_property(env, exports, "netId", netId) != napi_ok) {
        NETMGR_LOG_E("Failed to set netId at init");
        return nullptr;
    }
    g_constructor = new (std::nothrow) napi_ref;
    if (g_constructor == nullptr) {
        NETMGR_LOG_E("Failed to create ref at init");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, g_constructor);
    if (status != napi_ok) {
        NETMGR_LOG_E("Failed to create reference at init");
        return nullptr;
    }
    return exports;
}

void NapiNetConn::DeclareNetConnDestructor(napi_env env, void *nativeObject, void *hint)
{
    sptr<NetHandle> *addonPtr = static_cast<sptr<NetHandle> *>(nativeObject);
    delete addonPtr;
}

void ReadNetLinkInfo(napi_env env, napi_value &info, NetLinkInfo &netLinkInfo)
{
    napi_create_object(env, &info);
    NapiCommon::SetPropertyString(env, info, "ifaceName", netLinkInfo.ifaceName_);
    NapiCommon::SetPropertyString(env, info, "domain", netLinkInfo.domain_);
    // insert netAddr list to js return value.
    int netAddrListNum = 0;
    napi_value netAddrList = nullptr;
    napi_create_array_with_length(env, netLinkInfo.netAddrList_.size(), &netAddrList);
    for_each(netLinkInfo.netAddrList_.begin(), netLinkInfo.netAddrList_.end(), [&](INetAddr &val) {
        napi_value obj = nullptr;
        napi_create_object(env, &obj);
        NapiCommon::SetPropertyInt32(env, obj, "type", val.type_);
        NapiCommon::SetPropertyInt32(env, obj, "prefixLen", val.prefixlen_);
        NapiCommon::SetPropertyString(env, obj, "netMask", val.netMask_);
        NapiCommon::SetPropertyString(env, obj, "address", val.address_);
        NapiCommon::SetPropertyString(env, obj, "hostName", val.hostName_);
        napi_set_element(env, netAddrList, netAddrListNum++, obj);
    });
    napi_set_named_property(env, info, "netAddrList", netAddrList);
    // insert dns list to js return value.
    int dnsListNum = 0;
    napi_value dnsList = nullptr;
    napi_create_array_with_length(env, netLinkInfo.dnsList_.size(), &dnsList);
    for_each(netLinkInfo.dnsList_.begin(), netLinkInfo.dnsList_.end(), [&](INetAddr &val) {
        napi_value obj = nullptr;
        napi_create_string_utf8(env, val.address_.c_str(), NAPI_AUTO_LENGTH, &obj);
        napi_set_element(env, dnsList, dnsListNum++, obj);
    });
    napi_set_named_property(env, info, "dnsList", dnsList);
    // insert route list to js return value.
    int routeListNum = 0;
    napi_value routeList = nullptr;
    napi_create_array_with_length(env, netLinkInfo.routeList_.size(), &routeList);
    for_each(netLinkInfo.routeList_.begin(), netLinkInfo.routeList_.end(), [&](Route &val) {
        napi_value obj = nullptr;
        napi_create_object(env, &obj);
        NapiCommon::SetPropertyString(env, obj, "iface", val.iface_);
        NapiCommon::SetPropertyString(env, obj, "destination", val.destination_.address_);
        NapiCommon::SetPropertyString(env, obj, "gateway", val.gateway_.address_);
        NapiCommon::SetPropertyInt32(env, obj, "rtn_type", val.rtnType_);
        napi_set_element(env, routeList, routeListNum++, obj);
    });
    napi_set_named_property(env, info, "routeList", routeList);
    NapiCommon::SetPropertyInt32(env, info, "mtu", netLinkInfo.mtu_);
}

void ReadNetCapabilities(napi_env env, napi_value &info, const NetAllCapabilities &netAllCap)
{
    napi_create_object(env, &info);
    NapiCommon::SetPropertyUint32(env, info, "linkUpBandwidthKbps", netAllCap.linkUpBandwidthKbps_);
    NapiCommon::SetPropertyUint32(env, info, "linkDownBandwidthKbps_", netAllCap.linkDownBandwidthKbps_);
    // insert netCaps_ set to js return value.
    int32_t netCapsNum = 0;
    napi_value netCapsSet = nullptr;
    napi_create_array_with_length(env, netAllCap.netCaps_.size(), &netCapsSet);
    for_each(netAllCap.netCaps_.begin(), netAllCap.netCaps_.end(), [&](NetCap val) {
        napi_value obj = nullptr;
        napi_create_object(env, &obj);
        NapiCommon::SetPropertyUint32(env, obj, "netCap", static_cast<uint32_t>(val));
        napi_set_element(env, netCapsSet, netCapsNum++, obj);
    });
    napi_set_named_property(env, info, "netCaps_", netCapsSet);

    // insert bearerTypes_ set to js return value.
    int32_t bearerTypeNum = 0;
    napi_value bearerTypeSet = nullptr;
    napi_create_array_with_length(env, netAllCap.bearerTypes_.size(), &bearerTypeSet);
    for_each(netAllCap.bearerTypes_.begin(), netAllCap.bearerTypes_.end(), [&](NetBearType val) {
        napi_value obj = nullptr;
        napi_create_object(env, &obj);
        NapiCommon::SetPropertyUint32(env, obj, "bearerType", static_cast<uint32_t>(val));
        napi_set_element(env, bearerTypeSet, bearerTypeNum++, obj);
    });
    napi_set_named_property(env, info, "bearerTypes_", bearerTypeSet);
}

napi_value NapiNetConn::CreateNetHandle(napi_env env, sptr<NetHandle> &net)
{
    napi_value constructor;
    napi_status status = napi_get_reference_value(env, *g_constructor, &constructor);
    if (status != napi_ok) {
        NETMGR_LOG_E("Failed to get CreateNetHandle");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value arg = nullptr;
    napi_create_int32(env, net->GetNetId(), &arg);
    napi_value *argv = &arg;
    size_t argc = ARGV_NUM_1;
    status = napi_new_instance(env, constructor, argc, argv, &result);
    if (status != napi_ok) {
        NETMGR_LOG_E("Failed to create CreateNetHandle");
        return nullptr;
    }
    return result;
}

void NapiNetConn::ExecNetDetection(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecNetDetection");
    NetConnAsyncContext *context = (NetConnAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->result = DelayedSingleton<NetConnClient>::GetInstance()->NetDetection(*(context->addon));
    NETMGR_LOG_D(
        "ExecNetDetection netId =[%{public}d], result =[%{public}d]", context->addon->GetNetId(), context->result);
}

void NapiNetConn::CompleteNetDetection(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteNetDetection");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value infoFail = nullptr;
    napi_create_int32(env, context->result, &infoFail);
    napi_get_undefined(env, &info);
    if (context->callbackRef == nullptr) {
        // promiss return
        if (context->result != ERR_NONE) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, infoFail));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        // call back return
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result != ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_0] = infoFail;
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_1] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiNetConn::NetDetection(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("NetDetection");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("NetDetection agvc = [%{public}d]", static_cast<int32_t>(argc));
    sptr<NetHandle> *addonPtr = nullptr;
    napi_status status = napi_unwrap(env, argv[ARGV_INDEX_0], reinterpret_cast<void **>(&addonPtr));
    if (status != napi_ok) {
        NETMGR_LOG_E("NetDetection Failed to unwrap.");
        return nullptr;
    }
    NetConnAsyncContext *context = std::make_unique<NetConnAsyncContext>().release();
    context->addon = *addonPtr;
    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("NetDetection  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "NetDetection", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(
            env, resource, resourceName, ExecNetDetection, CompleteNetDetection, (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

void NapiNetConn::ExecBindSocket(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecBindSocket");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->result = context->addon->BindSocket(context->socketId);
    NETMGR_LOG_D(
        "ExecBindSocket netId =[%{public}d], result =[%{public}d]", context->addon->GetNetId(), context->result);
}

void NapiNetConn::CompleteBindSocket(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteBindSocket");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_create_int32(env, context->result, &info);
    if (context->callbackRef == nullptr) {
        // promiss return
        if (context->result != ERR_NONE) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, info));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        // call back return
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result != ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_0] = info;
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_1] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiNetConn::BindSocket(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("BindSocket");
    size_t argc = ARGV_NUM_2;
    napi_value thisVar = nullptr;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NETMGR_LOG_I("BindSocket agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext *context = std::make_unique<NetConnAsyncContext>().release();
    sptr<NetHandle> *addonPtr = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&addonPtr));
    if (status != napi_ok) {
        NETMGR_LOG_E("BindSocket Failed to unwrap.");
        return nullptr;
    }
    context->addon = *addonPtr;
    NAPI_CALL(env, napi_get_value_int32(env, argv[ARGV_INDEX_0], &context->socketId));
    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("BindSocket  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "BindSocket", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(
            env, resource, resourceName, ExecBindSocket, CompleteBindSocket, (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

void NapiNetConn::ExecGetAddressesByName(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecGetAddressesByName");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    std::vector<INetAddr> addrList;
    std::string hostName = context->host;
    if (context->useDnsResolver) {
        context->result =
            DelayedSingleton<DnsResolverClient>::GetInstance()->GetAddressesByName(hostName, addrList);
    } else {
        context->result = context->addon->GetAddressesByName(hostName, addrList);
    }
    for (auto val : addrList) {
        context->addr.push_back(val.address_);
    }
    NETMGR_LOG_D("ExecGetAddressesByName result =[%{public}d], addr.size =[%{public}d]", context->result,
        static_cast<int32_t>(context->addr.size()));
}

void NapiNetConn::CompleteGetAddressesByName(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetAddressesByName");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value infoFail = nullptr;
    napi_value infoAttay = nullptr;
    napi_create_int32(env, context->result, &infoFail);
    napi_create_array_with_length(env, context->addr.size(), &infoAttay);
    for (size_t index = 0; index < context->addr.size(); index++) {
        napi_value info = nullptr;
        napi_create_string_utf8(env, context->addr[index].c_str(), NAPI_AUTO_LENGTH, &info);
        napi_set_element(env, infoAttay, index, info);
    }
    if (context->callbackRef == nullptr) {
        // promiss return
        if (context->result != ERR_NONE) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, infoFail));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, infoAttay));
        }
    } else {
        // call back return
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result != ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_0] = infoFail;
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_1] = infoAttay;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiNetConn::GetAddressesByName(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("GetAddressesByName");
    size_t argc = ARGV_NUM_2;
    napi_value thisVar = nullptr;
    napi_value propertyName = nullptr;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NETMGR_LOG_I("GetAddressesByName agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext *context = std::make_unique<NetConnAsyncContext>().release();
    napi_create_string_utf8(env, "NetHandle", NAPI_AUTO_LENGTH, &propertyName);
    NAPI_CALL(env, napi_has_own_property(env, thisVar, propertyName, &(context->useDnsResolver)));
    if (!context->useDnsResolver) {
        sptr<NetHandle> *addonPtr = nullptr;
        napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&addonPtr));
        if (status != napi_ok) {
            NETMGR_LOG_E("GetAddressesByName Failed to unwrap.");
            return nullptr;
        }
        context->addon = *addonPtr;
        NETMGR_LOG_I("GetAddressesByName find NetHandle netId =[%{public}d].", context->addon->GetNetId());
    }
    NAPI_CALL(env,
        napi_get_value_string_utf8(
            env, argv[ARGV_INDEX_0], context->host, HOST_MAX_BYTES, &(context->hostRealBytes)));
    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("GetAddressesByName  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetAddressesByName", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetAddressesByName, CompleteGetAddressesByName,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

void NapiNetConn::ExecGetAddressByName(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecGetAddressByName");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    INetAddr addr;
    std::string hostName = context->host;
    context->result = context->addon->GetAddressByName(hostName, addr);
    context->hostAddress = addr.address_;
    NETMGR_LOG_D("ExecGetAddressByName netId =[%{public}d], result =[%{public}d], addr =[%{public}s]",
        context->addon->GetNetId(), context->result, context->hostAddress.c_str());
}

void NapiNetConn::CompleteGetAddressByName(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetAddressByName");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value infoFail = nullptr;
    napi_value info = nullptr;
    napi_create_int32(env, context->result, &infoFail);
    napi_create_string_utf8(env, context->hostAddress.c_str(), NAPI_AUTO_LENGTH, &info);
    if (context->callbackRef == nullptr) {
        // promiss return
        if (context->result != ERR_NONE) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, infoFail));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        // call back return
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result != ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_0] = infoFail;
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_1] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiNetConn::GetAddressByName(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("GetAddressByName");
    size_t argc = ARGV_NUM_2;
    napi_value thisVar = nullptr;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NETMGR_LOG_I("GetAddressByName agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext *context = std::make_unique<NetConnAsyncContext>().release();
    sptr<NetHandle> *addonPtr = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&addonPtr));
    if (status != napi_ok) {
        NETMGR_LOG_E("GetAddressByName Failed to unwrap.");
        return nullptr;
    }
    context->addon = *addonPtr;
    NAPI_CALL(env,
        napi_get_value_string_utf8(
            env, argv[ARGV_INDEX_0], context->host, HOST_MAX_BYTES, &(context->hostRealBytes)));
    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("GetAddressByName  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetAddressByName", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetAddressByName, CompleteGetAddressByName,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

void NapiNetConn::ExecGetDefaultNet(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecGetDefaultNet");
    NetConnAsyncContext *context = (NetConnAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->addon = std::make_unique<NetHandle>().release();
    context->result = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultNet(*(context->addon));
    NETMGR_LOG_D("ExecGetDefaultNet result =[%{public}d], netId =[%{public}d]", context->result,
        context->addon->GetNetId());
}

void NapiNetConn::CompleteGetDefaultNet(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetDefaultNet");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value infoFail = nullptr;
    if (context->result == ERR_NONE) {
        info = CreateNetHandle(env, context->addon);
        if (info == nullptr) {
            context->result = -1;
        }
    }
    napi_create_int32(env, context->result, &infoFail);
    if (context->callbackRef == nullptr) {
        // promiss return
        if (context->result != ERR_NONE) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, infoFail));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        // call back return
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result != ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_0] = infoFail;
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_1] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiNetConn::GetDefaultNet(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("GetDefaultNet");
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("GetDefaultNet agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext *context = std::make_unique<NetConnAsyncContext>().release();
    napi_value result = nullptr;
    if (argc == ARGV_NUM_0) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_1) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("GetDefaultNet  exception");
    }
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetDefaultNet", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetDefaultNet, CompleteGetDefaultNet,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

void NapiNetConn::ExecGetAllNets(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecGetAllNets");
    NetConnAsyncContext *context = (NetConnAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->result = DelayedSingleton<NetConnClient>::GetInstance()->GetAllNets(context->netList);
    NETMGR_LOG_D("ExecGetAllNets result =[%{public}d]", context->result);
}

void NapiNetConn::CompleteGetAllNets(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetAllNets");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value infoAttay = nullptr;
    napi_value infoFail = nullptr;
    int32_t listLen = context->netList.size();
    NETMGR_LOG_I("CompleteGetAllNets netList =[%{public}d]", listLen);
    napi_create_array_with_length(env, listLen, &infoAttay);
    int32_t netListNum = 0;
    for (auto val : context->netList) {
        info = CreateNetHandle(env, val);
        if (info == nullptr) {
            context->result = -1;
            break;
        }
        napi_set_element(env, infoAttay, netListNum++, info);
    }
    napi_create_int32(env, context->result, &infoFail);
    if (context->callbackRef == nullptr) {
        if (context->result != ERR_NONE) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, infoFail));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, infoAttay));
        }
    } else {
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result != ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_0] = infoFail;
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_1] = infoAttay;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiNetConn::GetAllNets(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("GetAllNets");
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("GetAllNets agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext *context = std::make_unique<NetConnAsyncContext>().release();
    napi_value result = nullptr;
    if (argc == ARGV_NUM_0) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_1) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("GetAllNets  exception");
    }
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetAllNets", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(
            env, resource, resourceName, ExecGetAllNets, CompleteGetAllNets, (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

void NapiNetConn::ExecGetConnectionProperties(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecGetConnectionProperties");
    NetConnAsyncContext *context = (NetConnAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->result = DelayedSingleton<NetConnClient>::GetInstance()->GetConnectionProperties(
        *(context->addon), context->netLinkInfo);
    NETMGR_LOG_D("ExecGetConnectionProperties netId =[%{public}d], result =[%{public}d]",
        context->addon->GetNetId(), context->result);
}

void NapiNetConn::CompleteGetConnectionProperties(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetConnectionProperties");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    ReadNetLinkInfo(env, info, context->netLinkInfo);
    napi_value infoFail = nullptr;
    napi_create_int32(env, context->result, &infoFail);
    if (context->callbackRef == nullptr) {
        // promiss return
        if (context->result != ERR_NONE) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, infoFail));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        // call back return
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result != ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_0] = infoFail;
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_1] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiNetConn::GetConnectionProperties(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("GetConnectionProperties");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("GetConnectionProperties agvc = [%{public}d]", static_cast<int32_t>(argc));
    sptr<NetHandle> *addonPtr = nullptr;
    napi_status status = napi_unwrap(env, argv[ARGV_INDEX_0], reinterpret_cast<void **>(&addonPtr));
    if (status != napi_ok) {
        NETMGR_LOG_E("GetConnectionProperties Failed to unwrap.");
        return nullptr;
    }
    NetConnAsyncContext *context = std::make_unique<NetConnAsyncContext>().release();
    context->addon = *addonPtr;
    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("GetConnectionProperties  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetConnectionProperties", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetConnectionProperties,
            CompleteGetConnectionProperties, (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

void NapiNetConn::ExecGetNetCapabilities(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecGetNetCapabilities");
    NetConnAsyncContext *context = (NetConnAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->result =
        DelayedSingleton<NetConnClient>::GetInstance()->GetNetCapabilities(*(context->addon), context->netAllCap);
    NETMGR_LOG_D("ExecGetNetCapabilities netId =[%{public}d], result =[%{public}d]", context->addon->GetNetId(),
        context->result);
}

void NapiNetConn::CompleteGetNetCapabilities(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetNetCapabilities");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    ReadNetCapabilities(env, info, context->netAllCap);
    napi_value infoFail = nullptr;
    napi_create_int32(env, context->result, &infoFail);
    if (context->callbackRef == nullptr) {
        if (context->result != ERR_NONE) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, infoFail));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result != ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_0] = infoFail;
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_1] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiNetConn::GetNetCapabilities(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("GetNetCapabilities");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("GetNetCapabilities agvc = [%{public}d]", static_cast<int32_t>(argc));
    sptr<NetHandle> *addonPtr = nullptr;
    napi_status status = napi_unwrap(env, argv[ARGV_INDEX_0], reinterpret_cast<void **>(&addonPtr));
    if (status != napi_ok) {
        NETMGR_LOG_E("GetNetCapabilities Failed to unwrap.");
        return nullptr;
    }
    NetConnAsyncContext *context = std::make_unique<NetConnAsyncContext>().release();
    context->addon = *addonPtr;
    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("GetNetCapabilities  exception");
    }
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetNetCapabilities", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecGetNetCapabilities, CompleteGetNetCapabilities,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

void NapiNetConn::ExecHasDefaultNet(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecHasDefaultNet");
    NetConnAsyncContext *context = (NetConnAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->result = DelayedSingleton<NetConnClient>::GetInstance()->HasDefaultNet(context->flag);
    NETMGR_LOG_D("ExecHasDefaultNet result =[%{public}d], flag =[%{public}d]", context->result, context->flag);
}

void NapiNetConn::CompleteHasDefaultNet(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteHasDefaultNet");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value infoFail = nullptr;
    napi_get_boolean(env, context->flag, &info);
    napi_create_int32(env, context->result, &infoFail);
    if (context->callbackRef == nullptr) {
        if (context->result != ERR_NONE) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, infoFail));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        // call back return
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result != ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_0] = infoFail;
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_1] = info;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiNetConn::HasDefaultNet(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("HasDefaultNet");
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("HasDefaultNet agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext *context = std::make_unique<NetConnAsyncContext>().release();
    napi_value result = nullptr;
    if (argc == ARGV_NUM_0) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_1) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("HasDefaultNet  exception");
    }
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "HasDefaultNet", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env,
        napi_create_async_work(env, resource, resourceName, ExecHasDefaultNet, CompleteHasDefaultNet,
            (void *)context, &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

bool MatchSetAirplaneModeInputParam(napi_env env, const napi_value parameters[], size_t parameterCount)
{
    switch (parameterCount) {
        case ARGV_NUM_0:
            return true;
        case ARGV_NUM_1:
            return NapiCommon::MatchValueType(env, parameters[0], napi_function);
        default:
            return false;
    }
}

bool MatchRestoreFactoryDataParam(napi_env env, const napi_value parameters[], size_t parameterCount)
{
    switch (parameterCount) {
        case ARGV_NUM_0:
            return true;
        case ARGV_NUM_1:
            return NapiCommon::MatchValueType(env, parameters[0], napi_function);
        default:
            return false;
    }
}

void NativeSetAirplaneMode(napi_env env, void *data)
{
    auto context = static_cast<BooleanValueContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->SetAirplaneMode(context->value);
    context->resolved = result == 0;
    context->errorCode = result;
}

void SetAirplaneModeCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<BooleanValueContext *>(data);
    napi_value callbackValue = nullptr;
    std::string tip = context->value ? "EnableAirplaneMode" : "DisableAirplaneMode";
    if (status == napi_ok) {
        if (context->resolved) {
            napi_get_undefined(env, &callbackValue);
        } else {
            tip.append(" failed");
            callbackValue = NapiCommon::CreateCodeMessage(env, tip, context->errorCode);
        }
    } else {
        tip.append(" error,napi_status = ");
        callbackValue = NapiCommon::CreateErrorMessage(env, tip + std::to_string(status));
    }
    NapiCommon::Handle1ValueCallback(env, context, callbackValue);
}

napi_value NapiNetConn::EnableAirplaneMode(napi_env env, napi_callback_info info)
{
    size_t paramsCount = ARGV_NUM_1;
    napi_value params[ARGV_NUM_1] = {0};
    napi_value arg = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &paramsCount, params, &arg, &data);
    NAPI_ASSERT(env, MatchSetAirplaneModeInputParam(env, params, paramsCount),
        "EnableAirplaneMode input param type mismatch");
    auto context = std::make_unique<BooleanValueContext>().release();
    context->value = true;
    if (paramsCount == ARGV_NUM_1) {
        napi_create_reference(env, params[0], CALLBACK_REF_CNT, &context->callbackRef);
    }
    napi_value result = NapiCommon::HandleAsyncWork(
        env, context, "EnableAirplaneMode", NativeSetAirplaneMode, SetAirplaneModeCallback);
    return result;
}

napi_value NapiNetConn::DisableAirplaneMode(napi_env env, napi_callback_info info)
{
    size_t paramsCount = ARGV_NUM_1;
    napi_value params[ARGV_NUM_1] = {0};
    napi_value arg = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &paramsCount, params, &arg, &data);
    NAPI_ASSERT(env, MatchSetAirplaneModeInputParam(env, params, paramsCount),
        "DisableAirplaneMode input param type mismatch");
    auto context = std::make_unique<BooleanValueContext>().release();
    context->value = false;
    if (paramsCount == ARGV_NUM_1) {
        napi_create_reference(env, params[0], CALLBACK_REF_CNT, &context->callbackRef);
    }
    napi_value result = NapiCommon::HandleAsyncWork(
        env, context, "DisableAirplaneMode", NativeSetAirplaneMode, SetAirplaneModeCallback);
    return result;
}

void NapiNetConn::NativeRestoreFactoryData(napi_env env, void *data)
{
    auto context = static_cast<RestoreFactoryDataContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    int32_t result = DelayedSingleton<NetConnClient>::GetInstance()->RestoreFactoryData();
    context->resolved = result == 0;
    context->errorCode = result;
}

void NapiNetConn::RestoreFactoryDataCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<RestoreFactoryDataContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok) {
        if (context->resolved) {
            napi_get_undefined(env, &callbackValue);
        } else {
            callbackValue = NapiCommon::CreateCodeMessage(env, "Failed to RestoreFactoryData", context->errorCode);
        }
    } else {
        callbackValue = NapiCommon::CreateErrorMessage(
            env, "RestoreFactoryData error,napi_status = " + std::to_string(status));
    }
    NapiCommon::Handle1ValueCallback(env, context, callbackValue);
}

napi_value NapiNetConn::RestoreFactoryData(napi_env env, napi_callback_info info)
{
    size_t paramsCount = ARGV_NUM_1;
    napi_value params[ARGV_NUM_1] = {0};
    napi_value arg = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &paramsCount, params, &arg, &data);
    NAPI_ASSERT(env, MatchRestoreFactoryDataParam(env, params, paramsCount),
        "RestoreFactoryData input param type mismatch");
    auto context = std::make_unique<RestoreFactoryDataContext>().release();
    if (paramsCount == 1) {
        NAPI_CALL(env, napi_create_reference(env, params[0], 1, &context->callbackRef));
    }
    napi_value result = NapiCommon::HandleAsyncWork(
        env, context, "RestoreFactoryData", NativeRestoreFactoryData, RestoreFactoryDataCallback);
    return result;
}

napi_value NapiNetConn::DeclareNetworkTypeData(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("BEARER_CELLULAR",
            NapiCommon::NapiValueByInt32(env, static_cast<uint32_t>(NetBearType::BEARER_CELLULAR))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "BEARER_WIFI", NapiCommon::NapiValueByInt32(env, static_cast<uint32_t>(NetBearType::BEARER_WIFI))),
        DECLARE_NAPI_STATIC_PROPERTY("BEARER_BLUETOOTH",
            NapiCommon::NapiValueByInt32(env, static_cast<uint32_t>(NetBearType::BEARER_BLUETOOTH))),
        DECLARE_NAPI_STATIC_PROPERTY("BEARER_ETHERNET",
            NapiCommon::NapiValueByInt32(env, static_cast<uint32_t>(NetBearType::BEARER_ETHERNET))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "BEARER_VPN", NapiCommon::NapiValueByInt32(env, static_cast<uint32_t>(NetBearType::BEARER_VPN))),
        DECLARE_NAPI_STATIC_PROPERTY("BEARER_WIFI_AWARE",
            NapiCommon::NapiValueByInt32(env, static_cast<uint32_t>(NetBearType::BEARER_WIFI_AWARE))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiNetConn::DeclareNetCapabilityData(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("NET_CAPABILITY_MMS",
            NapiCommon::NapiValueByInt32(env, static_cast<uint32_t>(NetCap::NET_CAPABILITY_MMS))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_CAPABILITY_NOT_METERED",
            NapiCommon::NapiValueByInt32(env, static_cast<uint32_t>(NetCap::NET_CAPABILITY_NOT_METERED))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_CAPABILITY_INTERNET",
            NapiCommon::NapiValueByInt32(env, static_cast<uint32_t>(NetCap::NET_CAPABILITY_INTERNET))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_CAPABILITY_NOT_VPN",
            NapiCommon::NapiValueByInt32(env, static_cast<uint32_t>(NetCap::NET_CAPABILITY_NOT_VPN))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_CAPABILITY_VALIDATED",
            NapiCommon::NapiValueByInt32(env, static_cast<uint32_t>(NetCap::NET_CAPABILITY_VALIDATED))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_CAPABILITY_CAPTIVE_PORTAL",
            NapiCommon::NapiValueByInt32(env, static_cast<uint32_t>(NetCap::NET_CAPABILITY_CAPTIVE_PORTAL))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiNetConn::RegisterNetConnInterface(napi_env env, napi_value exports)
{
    RegisternetConnectionObject(env, exports);
    DeclareNetConnInterface(env, exports);
    DeclareNetworkTypeData(env, exports);
    DeclareNetCapabilityData(env, exports);
    DeclareNetConnConstructor(env, exports);
    return nullptr;
}

static napi_module _netConnModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = NapiNetConn::RegisterNetConnInterface,
    .nm_modname = "net.connection",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterNetConnModule(void)
{
    napi_module_register(&_netConnModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
