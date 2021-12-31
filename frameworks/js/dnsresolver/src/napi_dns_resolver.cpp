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

#include "napi_dns_resolver.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "net_mgr_log_wrapper.h"
#include "i_dns_resolver_service.h"
#include "dns_resolver_client.h"
#include "napi_common.h"

namespace OHOS {
namespace NetManagerStandard {
NapiDnsResolver::NapiDnsResolver() {}

void NapiDnsResolver::ExecDnsResolverCallback(napi_env env, void *data)
{
    DnsResolverAsyncContext* context = static_cast<DnsResolverAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    std::string hostName = context->host;
    std::vector<INetAddr> addrInfo;
    NETMGR_LOG_D("ExecDnsResolverCallback [%{public}s]", hostName.c_str());
    context->result = DelayedSingleton<DnsResolverClient>::GetInstance()->GetAddressesByName(hostName, addrInfo);
    NETMGR_LOG_D("ExecDnsResolverCallback, addrInfo.size = [%{public}d]", static_cast<int32_t>(addrInfo.size()));
    for (auto it = addrInfo.begin(); it != addrInfo.end(); ++it) {
        context->hostAddress.push_back(it->address_);
    }
}

void NapiDnsResolver::CompleteDnsResolverCallback(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_D("CompleteDnsResolverCallback");
    DnsResolverAsyncContext* context = static_cast<DnsResolverAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    // creat function return
    napi_value infoAttay = nullptr;
    napi_value infoFail = nullptr;
    napi_value info = nullptr;
    napi_create_int32(env, context->result, &infoFail);
    if (context->hostAddress.size() > 0) {
        napi_create_array_with_length(env, context->hostAddress.size(), &infoAttay);
        for (size_t index = 0; index < context->hostAddress.size(); index++) {
            napi_create_string_utf8(env, context->hostAddress[index].c_str(), NAPI_AUTO_LENGTH, &info);
            napi_set_element(env, infoAttay, index, info);
        }
    } else {
        napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &infoAttay);
    }
    if (context->callbackRef == nullptr) {
        // promiss return
        if (context->result == ERR_NONE) {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, infoAttay));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, infoFail));
        }
    } else {
        // call back return
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->result == ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_1] = infoAttay;
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_0] = infoFail;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

napi_value NapiDnsResolver::GetAddressesByName(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_D("NapiDnsResolver GetAddressesByName");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    DnsResolverAsyncContext* context = std::make_unique<DnsResolverAsyncContext>().release();
    // Parse Js argv;
    NAPI_CALL(env, napi_get_value_string_utf8(
        env, argv[ARGV_INDEX_0], context->host, HOST_MAX_BYTES, &(context->hostRealBytes)));
    NETMGR_LOG_D("GetAddressesByName = [%{public}s]", context->host);
    NETMGR_LOG_D("GetAddressesByName argc = [%{public}d]", static_cast<int32_t>(argc));
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
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecDnsResolverCallback,
        CompleteDnsResolverCallback,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiDnsResolver::DeclareNapiDnsResolverInterface(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getAddressesByName", GetAddressesByName),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiDnsResolver::RegisterDnsResolverInterface(napi_env env, napi_value exports)
{
    DeclareNapiDnsResolverInterface(env, exports);
    return nullptr;
}

static napi_module _dnsResolverModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = NapiDnsResolver::RegisterDnsResolverInterface,
    .nm_modname = "netmanager.dnsresolver",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterDnsResolverModule(void)
{
    napi_module_register(&_dnsResolverModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
