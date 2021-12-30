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
#include "net_conn_constants.h"
#include "net_mgr_log_wrapper.h"
#include "event_listener_manager.h"
#include "event_context.h"
#include "net_conn_client.h"
#include "net_handler.h"
#include "napi_common.h"

namespace OHOS {
namespace NetManagerStandard {
NapiNetConn::NapiNetConn() {}

napi_value NapiNetConn::DeclareNetConnInterface(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getDefaultNet", GetDefaultNet),
        DECLARE_NAPI_FUNCTION("getSpecificNet", GetSpecificNet),
        DECLARE_NAPI_FUNCTION("getAllNets", GetAllNets),
        DECLARE_NAPI_FUNCTION("getConnectionProperties", GetConnectionProperties),
        DECLARE_NAPI_FUNCTION("getNetCapabilities", GetNetCapabilities),
        DECLARE_NAPI_FUNCTION("deactivateNetwork", DeactivateNetwork),
        DECLARE_NAPI_FUNCTION("bindSocket", BindSocket),
        DECLARE_NAPI_FUNCTION("getAddressesByName", GetAddressesByName),
        DECLARE_NAPI_FUNCTION("getAddressByName", GetAddressByName),
        DECLARE_NAPI_FUNCTION("getSpecificUidNet", GetSpecificUidNet),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_value ReadNetLinkInfo(napi_env env, NetLinkInfo &netLinkInfo)
{
    napi_value info = nullptr;
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
        NapiCommon::SetPropertyInt32(env, obj, "rtn_type", val.rtn_type_);
        napi_set_element(env, routeList, routeListNum++, obj);
    });
    napi_set_named_property(env, info, "routeList", routeList);
    NapiCommon::SetPropertyInt32(env, info, "mtu", netLinkInfo.mtu_);
    return info;
}

void NapiNetConn::ExecOn(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecOn");
    NetConnAsyncContext* context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    EventListener event;
    event.callbackRef = context->callbackRef;
    event.env = env;
    event.event = context->eventContent;
    event.identifier = context->identifier;
    event.netType = context->netType;
    event.netCapabilities = context->netCapabilities;
    context->result = EventListenerManager::GetInstance().AddEventListener(event);
    NETMGR_LOG_D("ExecOn result = [%{public}d]", context->result);
}

void NapiNetConn::ExecOff(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecOff");
    NetConnAsyncContext* context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    EventListener event;
    event.callbackRef = context->callbackRef;
    event.env = env;
    event.event = context->eventContent;
    event.identifier = context->identifier;
    event.netType = context->netType;
    event.netCapabilities = context->netCapabilities;
    context->result = EventListenerManager::GetInstance().RemoveEventListener(event);
    NETMGR_LOG_D("ExecOff result = [%{public}d]", context->result);
}

void NapiNetConn::CompleteOn(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteOn");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context->result != 0) {
        NETMGR_LOG_E("ExecOn context->result = [%{public}d]", context->result);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiNetConn::CompleteOff(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteOff");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context->result != 0) {
        NETMGR_LOG_E("ExecOff context->result = [%{public}d]", context->result);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiNetConn::On(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_D("On");
    size_t argc = ARGV_NUM_3;
    napi_value argv[] = {nullptr, nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_D("On agvc = %{public}d", static_cast<int32_t>(argc));
    NetConnAsyncContext* context = std::make_unique<NetConnAsyncContext>().release();
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARGV_INDEX_0], context->eventContent,
        EVENT_CONTENT_MAX_BYTE, &(context->eventContentRealBytes)));
    NETMGR_LOG_D("On eventContent = [%{public}s]", context->eventContent);
    napi_value result = nullptr;
    if (argc == ARGV_NUM_3) {
        napi_valuetype type;
        NAPI_CALL(env, napi_typeof(env, argv[ARGV_INDEX_1], &type));
        NETMGR_LOG_D("On = %{public}d", type);
        if (type == napi_object) { // callback call
            NapiCommon::GetPropertyString(env, argv[ARGV_INDEX_1], "identifier", context->identifier);
            NapiCommon::GetPropertyInt32(env, argv[ARGV_INDEX_1], "netType", context->netType);
            NapiCommon::GetPropertyInt32(env, argv[ARGV_INDEX_1], "netCapabilities", context->netCapabilities);
            NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_2], CALLBACK_REF_CNT, &context->callbackRef));
        }
    } else if (argc == ARGV_NUM_2) { // callback call
        NETMGR_LOG_D("callback call = ARGV_NUM_2");
        NAPI_CALL(env, napi_create_reference(env, argv[1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("NapiNetConn::On exception");
    }
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "On", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecOn,
        CompleteOn,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetConn::Off(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_D("Off");
    size_t argc = ARGV_NUM_3;
    napi_value argv[] = {nullptr, nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_D("On agvc = %{public}d", static_cast<int32_t>(argc));
    NetConnAsyncContext* context = std::make_unique<NetConnAsyncContext>().release();
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARGV_INDEX_0], context->eventContent,
        EVENT_CONTENT_MAX_BYTE, &(context->eventContentRealBytes)));
    NETMGR_LOG_D("Off eventContent = [%{public}s]", context->eventContent);
    napi_value result = nullptr;
    if (argc == ARGV_NUM_3) {
        napi_valuetype type;
        NAPI_CALL(env, napi_typeof(env, argv[ARGV_INDEX_1], &type));
        NETMGR_LOG_D("Off = %{public}d", type);
        if (type == napi_object) { // callback call
            NapiCommon::GetPropertyString(env, argv[ARGV_INDEX_1], "identifier", context->identifier);
            NapiCommon::GetPropertyInt32(env, argv[ARGV_INDEX_1], "netType", context->netType);
            NapiCommon::GetPropertyInt32(env, argv[ARGV_INDEX_1], "netCapabilities", context->netCapabilities);
            NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_2], CALLBACK_REF_CNT, &context->callbackRef));
        }
    } else if (argc == ARGV_NUM_2) { // callback call
        NETMGR_LOG_D("callback call = ARGV_NUM_2");
        NAPI_CALL(env, napi_create_reference(env, argv[1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("NapiNetConn::Off exception");
    }
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "Off", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecOff,
        CompleteOff,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

void NapiNetConn::ExecDeactivateNetwork(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecDeactivateNetwork");
    NetConnAsyncContext *context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->result = DelayedSingleton<NetConnClient>::GetInstance()->DeactivateNetwork(context->reqId);
    NETMGR_LOG_D("ExecDeactivateNetwork result =[%{public}d]", context->result);
}

void NapiNetConn::CompleteDeactivateNetwork(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteDeactivateNetwork");
    NetConnAsyncContext* context = static_cast<NetConnAsyncContext *>(data);
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

napi_value NapiNetConn::DeactivateNetwork(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("DeactivateNetwork");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("DeactivateNetwork agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext* context = std::make_unique<NetConnAsyncContext>().release();
    // Parse Js argv;
    NAPI_CALL(env, napi_get_value_uint32(env, argv[ARGV_INDEX_0], &context->reqId));
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
        NETMGR_LOG_E("DeactivateNetwork  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "DeactivateNetwork", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecDeactivateNetwork,
        CompleteDeactivateNetwork,
        (void *)context,
        &context->work));
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
    NetHandler handler(context->netId);
    context->result = handler.BindSocket(context->socketId);
    NETMGR_LOG_D("ExecBindSocket result =[%{public}d]", context->result);
}

void NapiNetConn::CompleteBindSocket(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteBindSocket");
    NetConnAsyncContext* context = static_cast<NetConnAsyncContext *>(data);
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
    size_t argc = ARGV_NUM_3;
    napi_value argv[] = {nullptr, nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("BindSocket agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext* context = std::make_unique<NetConnAsyncContext>().release();
    // Parse Js argv;
    NAPI_CALL(env, napi_get_value_int32(env, argv[ARGV_INDEX_0], &context->socketId));
    NAPI_CALL(env, napi_get_value_int32(env, argv[ARGV_INDEX_1], &context->netId));
    napi_value result = nullptr;
    if (argc == ARGV_NUM_2) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_3) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_2], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("BindSocket  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "BindSocket", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecBindSocket,
        CompleteBindSocket,
        (void *)context,
        &context->work));
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
    std::list<INetAddr> addrList;
    std::string hostName = context->host;
    NetHandler handler(context->netId);
    context->result = handler.GetAddressesByName(hostName, addrList);
    for_each(addrList.begin(), addrList.end(), [&](INetAddr &val) {
        context->addr.push_back(val.address_);
    });
    NETMGR_LOG_D("ExecGetAddressesByName result =[%{public}d], addr.size =[%{public}d]",
        context->result, static_cast<int32_t>(context->addr.size()));
}

void NapiNetConn::CompleteGetAddressesByName(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetAddressesByName");
    NetConnAsyncContext* context = static_cast<NetConnAsyncContext *>(data);
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
    size_t argc = ARGV_NUM_3;
    napi_value argv[] = {nullptr, nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("GetAddressesByName agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext* context = std::make_unique<NetConnAsyncContext>().release();
    // Parse Js argv;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARGV_INDEX_0],
        context->host, HOST_MAX_BYTES, &(context->hostRealBytes)));
    NAPI_CALL(env, napi_get_value_int32(env, argv[ARGV_INDEX_1], &context->netId));
    napi_value result = nullptr;
    if (argc == ARGV_NUM_2) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_3) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_2], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("GetAddressesByName  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetAddressesByName", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetAddressesByName,
        CompleteGetAddressesByName,
        (void *)context,
        &context->work));
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
    NetHandler handler(context->netId);
    context->result = handler.GetAddressByName(hostName, addr);
    context->hostAddress = addr.address_;
    NETMGR_LOG_D("ExecGetAddressByName result =[%{public}d], addr =[%{public}s]",
        context->result, context->hostAddress.c_str());
}

void NapiNetConn::CompleteGetAddressByName(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetAddressByName");
    NetConnAsyncContext* context = static_cast<NetConnAsyncContext *>(data);
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
    size_t argc = ARGV_NUM_3;
    napi_value argv[] = {nullptr, nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("GetAddressByName agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext* context = std::make_unique<NetConnAsyncContext>().release();
    // Parse Js argv;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARGV_INDEX_0],
        context->host, HOST_MAX_BYTES, &(context->hostRealBytes)));
    NAPI_CALL(env, napi_get_value_int32(env, argv[ARGV_INDEX_1], &context->netId));
    napi_value result = nullptr;
    if (argc == ARGV_NUM_2) {
        if (context->callbackRef == nullptr) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_3) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_2], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("GetAddressByName  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetAddressByName", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetAddressByName,
        CompleteGetAddressByName,
        (void *)context,
        &context->work));
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
    context->result = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultNet(context->netId);
    NETMGR_LOG_D("ExecGetDefaultNet result =[%{public}d], netId =[%{public}d]", context->result, context->netId);
}

void NapiNetConn::CompleteGetDefaultNet(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetDefaultNet");
    NetConnAsyncContext* context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value infoFail = nullptr;
    napi_create_int32(env, context->netId, &info);
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
    napi_value argv[] = {nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("GetDefaultNet agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext* context = std::make_unique<NetConnAsyncContext>().release();
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
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetDefaultNet", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetDefaultNet,
        CompleteGetDefaultNet,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

void NapiNetConn::ExecGetSpecificNet(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecGetSpecificNet");
    NetConnAsyncContext *context = (NetConnAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->result = DelayedSingleton<NetConnClient>::GetInstance()->GetSpecificNet(context->type, context->netIdList);
    NETMGR_LOG_D("ExecGetSpecificNet result =[%{public}d], netId =[%{public}u]", context->result, context->type);
}

void NapiNetConn::CompleteGetSpecificNet(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetSpecificNet");
    NetConnAsyncContext* context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value infoAttay = nullptr;
    napi_value infoFail = nullptr;
    napi_create_int32(env, context->result, &infoFail);
    int32_t listLen = context->netIdList.size();
    NETMGR_LOG_I("CompleteGetSpecificNet netIdList =[%{public}d]", listLen);
    napi_create_array_with_length(env, listLen, &infoAttay);
    int32_t netIdListNum = 0;
    for_each(context->netIdList.begin(), context->netIdList.end(), [&](int32_t val) {
        napi_create_int32(env, val, &info);
        napi_set_element(env, infoAttay, netIdListNum++, info);
    });
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

napi_value NapiNetConn::GetSpecificNet(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("GetSpecificNet");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("GetSpecificNet agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext* context = std::make_unique<NetConnAsyncContext>().release();
    // Parse Js argv;
    NAPI_CALL(env, napi_get_value_uint32(env, argv[ARGV_INDEX_0], &context->type));
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
        NETMGR_LOG_E("GetSpecificNet  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetSpecificNet", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetSpecificNet,
        CompleteGetSpecificNet,
        (void *)context,
        &context->work));
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
    context->result = DelayedSingleton<NetConnClient>::GetInstance()->GetAllNets(context->netIdList);
    NETMGR_LOG_D("ExecGetAllNets result =[%{public}d]", context->result);
}

void NapiNetConn::CompleteGetAllNets(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetAllNets");
    NetConnAsyncContext* context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value infoAttay = nullptr;
    napi_value infoFail = nullptr;
    napi_create_int32(env, context->result, &infoFail);
    int32_t listLen = context->netIdList.size();
    NETMGR_LOG_I("CompleteGetAllNets netIdList =[%{public}d]", listLen);
    napi_create_array_with_length(env, listLen, &infoAttay);
    int32_t netIdListNum = 0;
    for_each(context->netIdList.begin(), context->netIdList.end(), [&](int32_t val) {
        napi_create_int32(env, val, &info);
        napi_set_element(env, infoAttay, netIdListNum++, info);
    });
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

napi_value NapiNetConn::GetAllNets(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("GetAllNets");
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("GetAllNets agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext* context = std::make_unique<NetConnAsyncContext>().release();
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
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetAllNets", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetAllNets,
        CompleteGetAllNets,
        (void *)context,
        &context->work));
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
    NetLinkInfo info;
    context->result = DelayedSingleton<NetConnClient>::GetInstance()->GetConnectionProperties(
        context->netId, info);
    NETMGR_LOG_D("ExecGetConnectionProperties result =[%{public}d]", context->result);
    context->netLinkInfo = ReadNetLinkInfo(env, info);
}

void NapiNetConn::CompleteGetConnectionProperties(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetConnectionProperties");
    NetConnAsyncContext* context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = context->netLinkInfo;
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
    napi_value argv[] = {nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("GetConnectionProperties agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext* context = std::make_unique<NetConnAsyncContext>().release();
    // Parse Js argv;
    NAPI_CALL(env, napi_get_value_int32(env, argv[ARGV_INDEX_0], &context->netId));
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
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetConnectionProperties,
        CompleteGetConnectionProperties,
        (void *)context,
        &context->work));
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
    context->result = DelayedSingleton<NetConnClient>::GetInstance()->GetNetCapabilities(context->netId, context->cap);
    NETMGR_LOG_D("ExecGetNetCapabilities result =[%{public}d], netId =[%{public}" PRIu64 "]", context->result,
        context->cap);
}

void NapiNetConn::CompleteGetNetCapabilities(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetNetCapabilities");
    NetConnAsyncContext* context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value infoFail = nullptr;
    napi_create_int32(env, context->result, &infoFail);
    napi_create_bigint_uint64(env, context->cap, &info);
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

napi_value NapiNetConn::GetNetCapabilities(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("GetNetCapabilities");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("GetNetCapabilities agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext* context = std::make_unique<NetConnAsyncContext>().release();
    // Parse Js argv;
    NAPI_CALL(env, napi_get_value_int32(env, argv[ARGV_INDEX_0], &context->netId));
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
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetNetCapabilities", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetNetCapabilities,
        CompleteGetNetCapabilities,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

void NapiNetConn::ExecGetSpecificUidNet(napi_env env, void *data)
{
    NETMGR_LOG_D("ExecGetSpecificUidNet");
    NetConnAsyncContext *context = (NetConnAsyncContext *)data;
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->result = DelayedSingleton<NetConnClient>::GetInstance()->GetSpecificUidNet(context->uid, context->netId);
    NETMGR_LOG_D("ExecGetSpecificUidNet result =[%{public}d], netId =[%{public}d]", context->result, context->netId);
}

void NapiNetConn::CompleteGetSpecificUidNet(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteGetSpecificUidNet");
    NetConnAsyncContext* context = static_cast<NetConnAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_value infoFail = nullptr;
    napi_create_int32(env, context->netId, &info);
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

napi_value NapiNetConn::GetSpecificUidNet(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_I("GetSpecificUidNet");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NETMGR_LOG_I("GetSpecificUidNet agvc = [%{public}d]", static_cast<int32_t>(argc));
    NetConnAsyncContext* context = std::make_unique<NetConnAsyncContext>().release();
    // Parse Js argv;
    NAPI_CALL(env, napi_get_value_int32(env, argv[ARGV_INDEX_0], &context->uid));
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
        NETMGR_LOG_E("GetSpecificUidNet  exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "GetSpecificUidNet", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetSpecificUidNet,
        CompleteGetSpecificUidNet,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetConn::DeclareNetworkTypeData(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("NET_TYPE_UNKNOWN",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetworkType::NET_TYPE_UNKNOWN))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_TYPE_CELLULAR",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetworkType::NET_TYPE_CELLULAR))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_TYPE_ETHERNET",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetworkType::NET_TYPE_ETHERNET))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_TYPE_WIFI",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetworkType::NET_TYPE_WIFI))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_TYPE_MAX",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetworkType::NET_TYPE_MAX))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiNetConn::DeclareNetCapabilityData(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("NET_CAPABILITIES_NONE",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetCapabilities::NET_CAPABILITIES_NONE))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_CAPABILITIES_INTERNET",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetCapabilities::NET_CAPABILITIES_INTERNET))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_CAPABILITIES_MMS",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetCapabilities::NET_CAPABILITIES_MMS))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiNetConn::RegisterNetConnInterface(napi_env env, napi_value exports)
{
    DeclareNetConnInterface(env, exports);
    DeclareNetworkTypeData(env, exports);
    DeclareNetCapabilityData(env, exports);
    return nullptr;
}

static napi_module _netConnModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = NapiNetConn::RegisterNetConnInterface,
    .nm_modname = "netmanager.netconn",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterNetConnModule(void)
{
    napi_module_register(&_netConnModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
