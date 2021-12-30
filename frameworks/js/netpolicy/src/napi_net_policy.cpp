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

#include "napi_net_policy.h"
#include <memory>
#include <cinttypes>
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "net_mgr_log_wrapper.h"
#include "i_net_policy_service.h"
#include "net_policy_client.h"
#include "napi_common.h"
#include "base_context.h"

namespace OHOS {
namespace NetManagerStandard {
const int32_t DEFAULT_REF_COUNT = 1;
void NapiNetPolicy::ExecSetUidPolicy(napi_env env, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    NetUidPolicy policy = static_cast<NetUidPolicy>(context->policy);
    context->policyResult =
        static_cast<int32_t>(DelayedSingleton<NetPolicyClient>::GetInstance()->SetUidPolicy(context->uid, policy));
    NETMGR_LOG_I("ExecSetUidPolicy, policy = [%{public}d], policyResult = [%{public}d]",
        context->policy, context->policyResult);
}

void NapiNetPolicy::ExecGetUids(napi_env env, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    NetUidPolicy policy = static_cast<NetUidPolicy>(context->policy);
    context->uidTogether = DelayedSingleton<NetPolicyClient>::GetInstance()->GetUids(policy);
    NETMGR_LOG_I("ExecGetUids, policy = [%{public}d], res.length = [%{public}d]",
        context->policy, static_cast<int32_t>(context->uidTogether.size()));
}

void NapiNetPolicy::ExecGetUidPolicy(napi_env env, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->policyResult =
        static_cast<int32_t>(DelayedSingleton<NetPolicyClient>::GetInstance()->GetUidPolicy(context->uid));
    NETMGR_LOG_I("ExecGetUidPolicy, uid = [%{public}d], policyResult = [%{public}d]",
        context->uid, context->policyResult);
}

void NapiNetPolicy::ExecIsUidNetAccess(napi_env env, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    if (context->interfaceName.length() > 0) {
        context->result =
            DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAccess(context->uid, context->interfaceName);
    } else {
        context->result =
            DelayedSingleton<NetPolicyClient>::GetInstance()->IsUidNetAccess(context->uid, context->metered);
    }
}

void NapiNetPolicy::ExecSetNetPolicys(napi_env env, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->resultCode = DelayedSingleton<NetPolicyClient>::GetInstance()->SetNetPolicys(context->quotaPolicys);
}

void NapiNetPolicy::ExecSnoozePolicy(napi_env env, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->resultCode = DelayedSingleton<NetPolicyClient>::GetInstance()->SnoozePolicy(context->quotaPolicy);
}

void NapiNetPolicy::ExecSetIdleWhitelist(napi_env env, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->resultCode = DelayedSingleton<NetPolicyClient>::GetInstance()->SetIdleWhitelist(context->uid,
        context->isWhiteList);
    NETMGR_LOG_D("context->resultCode == [%{public}d]", static_cast<int>(context->resultCode));
}

void NapiNetPolicy::ExecGetIdleWhitelist(napi_env env, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    context->resultCode = DelayedSingleton<NetPolicyClient>::GetInstance()->GetIdleWhitelist(context->uids);
}

void NapiNetPolicy::CompleteSetUidPolicy(napi_env env, napi_status status, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_create_int32(env, context->policyResult, &info);
    if (context->callbackRef == nullptr) {
        // promiss return
        if (context->policyResult != static_cast<int32_t>(NetPolicyResultCode::ERR_NONE)) {
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, info));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else {
        // call back
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->policyResult != static_cast<int32_t>(NetPolicyResultCode::ERR_NONE)) {
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

void NapiNetPolicy::CompleteGetUids(napi_env env, napi_status status, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    std::vector<uint32_t>& res = context->uidTogether;
    napi_create_array(env, &info);
    for (unsigned int i = 0; i < res.size(); i++) {
        napi_value num;
        napi_create_uint32(env, res[i], &num);
        napi_set_element(env, info, i, num);
    }
    if (!context->callbackRef) {
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
    } else {
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        callbackValues[CALLBACK_ARGV_INDEX_1] = info;
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiNetPolicy::CompleteGetUidPolicy(napi_env env, napi_status status, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_create_int32(env, context->policyResult, &info);
    if (!context->callbackRef) {
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
    } else {
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        callbackValues[CALLBACK_ARGV_INDEX_1] = info;
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiNetPolicy::CompleteIsUidNetAccess(napi_env env, napi_status status, void *data)
{
    NetPolicyAsyncContext* context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value info = nullptr;
    napi_get_boolean(env, context->result, &info);
    if (!context->callbackRef) {
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
    } else {
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        callbackValues[CALLBACK_ARGV_INDEX_1] = info;
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiNetPolicy::CompleteSetNetPolicys(napi_env env, napi_status status, void *data)
{
    CompleteNetPolicyResultCode(env, status, data);
}

void NapiNetPolicy::CompleteSnoozePolicy(napi_env env, napi_status status, void *data)
{
    CompleteNetPolicyResultCode(env, status, data);
}

void NapiNetPolicy::CompleteSetIdleWhitelist(napi_env env, napi_status status, void *data)
{
    CompleteNetPolicyResultCode(env, status, data);
}

void NapiNetPolicy::CompleteGetIdleWhitelist(napi_env env, napi_status status, void *data)
{
    NetPolicyAsyncContext *context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    napi_value uids = nullptr;
    if (context->resultCode == NetPolicyResultCode::ERR_NONE) {
        napi_create_array(env, &uids);
        int32_t arraySize = static_cast<int32_t>(context->uids.size());
        for (int32_t i = 0; i < arraySize; i++) {
            napi_value itemValue = nullptr;
            napi_create_int32(env, context->uids[i], &itemValue);
            napi_set_element(env, uids, i, itemValue);
        }
    }
    if (!context->callbackRef) { // promiss return
        napi_value info = nullptr;
        if (context->resultCode != NetPolicyResultCode::ERR_NONE) {
            info = NapiCommon::CreateCodeMessage(env, "fail", static_cast<int32_t>(context->resultCode));
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, info));
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, uids));
        }
    } else { // call back
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->resultCode != NetPolicyResultCode::ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_0] =
                NapiCommon::CreateCodeMessage(env, "fail", static_cast<int32_t>(context->resultCode));
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_1] = uids;
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

void NapiNetPolicy::CompleteNetPolicyResultCode(napi_env env, napi_status status, void *data)
{
    NetPolicyAsyncContext *context = static_cast<NetPolicyAsyncContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    if (!context->callbackRef) { // promiss return
        napi_value info = nullptr;
        if (context->resultCode != NetPolicyResultCode::ERR_NONE) {
            info = NapiCommon::CreateCodeMessage(env, "fail", static_cast<int32_t>(context->resultCode));
            NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, info));
        } else {
            info = NapiCommon::CreateCodeMessage(env, "successful", static_cast<int32_t>(context->resultCode));
            NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, context->deferred, info));
        }
    } else { // call back
        napi_value callbackValues[CALLBACK_ARGV_CNT] = {nullptr, nullptr};
        napi_value recv = nullptr;
        napi_value result = nullptr;
        napi_value callbackFunc = nullptr;
        napi_get_undefined(env, &recv);
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        if (context->resultCode != NetPolicyResultCode::ERR_NONE) {
            callbackValues[CALLBACK_ARGV_INDEX_0] =
                NapiCommon::CreateCodeMessage(env, "fail", static_cast<int32_t>(context->resultCode));
        } else {
            callbackValues[CALLBACK_ARGV_INDEX_1] =
                NapiCommon::CreateCodeMessage(env, "successful", static_cast<int32_t>(context->resultCode));
        }
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &result);
        napi_delete_reference(env, context->callbackRef);
    }
    napi_delete_async_work(env, context->work);
    delete context;
    context = nullptr;
}

NetPolicyQuotaPolicy NapiNetPolicy::ReadQuotaPolicy(napi_env env, napi_value value)
{
    NetPolicyQuotaPolicy data;
    data.netType_ = static_cast<int8_t>(NapiCommon::GetNapiInt32Value(env, value, "netType"));
    data.subscriberId_ = NapiCommon::GetNapiStringValue(env, value, "subscriberId");
    data.periodStartTime_ = NapiCommon::GetNapiInt32Value(env, value, "periodStartTime");
    data.periodDuration_ = NapiCommon::GetNapiStringValue(env, value, "periodDuration");
    data.warningBytes_ = NapiCommon::GetNapiInt64Value(env, value, "warningBytes");
    data.limitBytes_ = NapiCommon::GetNapiInt64Value(env, value, "limitBytes");
    data.lastLimitSnooze_ = NapiCommon::GetNapiInt32Value(env, value, "lastLimitSnooze");
    data.metered_ = static_cast<int8_t>(NapiCommon::GetNapiInt32Value(env, value, "metered"));
    data.source_ = static_cast<int8_t>(NapiCommon::GetNapiInt32Value(env, value, "source"));
    return data;
}

bool MatchGetNetPolicysInputParam(napi_env env, const napi_value parameters[], size_t parameterCount)
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

void NativeGetNetPolicys(napi_env env, void *data)
{
    auto context = static_cast<GetNetPolicyContext *>(data);
    if (context == nullptr) {
        NETMGR_LOG_E("context == nullptr");
        return;
    }
    NetPolicyResultCode result = DelayedSingleton<NetPolicyClient>::GetInstance()->GetNetPolicys(context->result);
    context->resolved = result == NetPolicyResultCode::ERR_NONE;
    context->errorCode = static_cast<int32_t>(result);
}

void GetNetPolicysCallback(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<GetNetPolicyContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok) {
        if (context->resolved) {
            napi_get_undefined(env, &callbackValue);
            napi_create_array(env, &callbackValue);
            int32_t arraySize = static_cast<int32_t>(context->result.size());
            for (int32_t i = 0; i < arraySize; i++) {
                napi_value elementObject = nullptr;
                napi_create_object(env, &elementObject);
                NetPolicyQuotaPolicy item = context->result[i];
                NapiCommon::SetPropertyInt32(env, elementObject, "netType", item.netType_);
                NapiCommon::SetPropertyString(env, elementObject, "subscriberId", item.subscriberId_);
                NapiCommon::SetPropertyInt64(env, elementObject, "periodStartTime", item.periodStartTime_);
                NapiCommon::SetPropertyString(env, elementObject, "periodDuration", item.periodDuration_);
                NapiCommon::SetPropertyInt64(env, elementObject, "warningBytes", item.warningBytes_);
                NapiCommon::SetPropertyInt64(env, elementObject, "limitBytes", item.limitBytes_);
                NapiCommon::SetPropertyInt64(env, elementObject, "lastLimitSnooze", item.lastLimitSnooze_);
                NapiCommon::SetPropertyInt32(env, elementObject, "metered", static_cast<int32_t>(item.metered_));
                NapiCommon::SetPropertyInt32(env, elementObject, "source", static_cast<int32_t>(item.source_));
                napi_set_element(env, callbackValue, i, elementObject);
            }
        } else {
            callbackValue = NapiCommon::CreateCodeMessage(env, "Failed to GetNetPolicys",
                static_cast<int32_t>(context->errorCode));
        }
    } else {
        callbackValue =
                NapiCommon::CreateErrorMessage(env, "GetNetPolicys error,napi_status = " + std::to_string(status));
    }
    NapiCommon::Handle1ValueCallback(env, context, callbackValue);
}

napi_value NapiNetPolicy::GetNetPolicys(napi_env env, napi_callback_info info)
{
    size_t paramsCount = ARGV_NUM_1;
    napi_value params[ARGV_NUM_1] = {0};
    napi_value arg = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &paramsCount, params, &arg, &data);

    NAPI_ASSERT(env, MatchGetNetPolicysInputParam(env, params, paramsCount), "GetNetPolicys input param type mismatch");

    auto context = std::make_unique<GetNetPolicyContext>().release();
    if (paramsCount == ARGV_NUM_1) {
        napi_create_reference(env, params[0], DEFAULT_REF_COUNT, &context->callbackRef);
    }
    napi_value result =
            NapiCommon::HandleAsyncWork(env, context, "GetNetPolicys", NativeGetNetPolicys, GetNetPolicysCallback);
    return result;
}

napi_value NapiNetPolicy::DeclareNapiNetPolicyData(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("NET_POLICY_NONE",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetUidPolicy::NET_POLICY_NONE))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_POLICY_ALLOW_METERED_BACKGROUND",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_POLICY_TEMPORARY_ALLOW_METERED",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetUidPolicy::NET_POLICY_TEMPORARY_ALLOW_METERED))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_POLICY_REJECT_METERED_BACKGROUND",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_POLICY_ALLOW_ALL",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetUidPolicy::NET_POLICY_ALLOW_ALL))),
        DECLARE_NAPI_STATIC_PROPERTY("NET_POLICY_REJECT_ALL",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetUidPolicy::NET_POLICY_REJECT_ALL))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiNetPolicy::DeclareNapiNetPolicyResultData(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("ERR_NONE",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetPolicyResultCode::ERR_NONE))),
        DECLARE_NAPI_STATIC_PROPERTY("ERR_INTERNAL_ERROR",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetPolicyResultCode::ERR_INTERNAL_ERROR))),
        DECLARE_NAPI_STATIC_PROPERTY("ERR_INVALID_UID",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetPolicyResultCode::ERR_INVALID_UID))),
        DECLARE_NAPI_STATIC_PROPERTY("ERR_INVALID_POLICY",
            NapiCommon::NapiValueByInt32(env, static_cast<int32_t>(NetPolicyResultCode::ERR_INVALID_POLICY))),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

bool MatchResetFactoryParameters(napi_env env, napi_value argv[], size_t argc)
{
    switch (argc) {
        case ARGV_INDEX_1: {
            return NapiCommon::MatchParameters(env, argv, {napi_function});
        }
        case ARGV_INDEX_2: {
            return NapiCommon::MatchParameters(env, argv, {napi_string, napi_function});
        }
        default: {
            return false;
        }
    }
}

void NapiNetPolicy::ExecResetFactory(napi_env env, void *data)
{
    auto context = static_cast<ResetFactoryContext *>(data);
    context->resultCode = DelayedSingleton<NetPolicyClient>::GetInstance()->ResetFactory(context->subscriberId);
    context->resolved = true;
}

void NapiNetPolicy::CompleteResetFactory(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_I("CompleteResetFactory start");
    auto context = static_cast<ResetFactoryContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok)
    {
        if (context->resolved)
        {
            napi_create_int32(env, static_cast<int32_t>(context->resultCode), &callbackValue);
        }
        else
        {
            callbackValue = NapiCommon::CreateErrorMessage(env, "reset factory error");
        }
    }
    else
    {
        callbackValue = NapiCommon::CreateErrorMessage(
            env, "reset factory error cause napi_status = " + std::to_string(status));
    }
    NapiCommon::Handle2ValueCallback(env, context, callbackValue);
}

napi_value NapiNetPolicy::ResetFactory(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr, nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, MatchResetFactoryParameters(env, argv, argc), "type mismatch");
    auto context = std::make_unique<ResetFactoryContext>().release();
    NETMGR_LOG_I("napi_policy ResetFactory start��");
    context->subscriberId = NapiCommon::GetNapiStringValue(env, argv[0], "subscriberId");
    NETMGR_LOG_I("napi_policy ResetFactory context->subscriberId = %{public}s", context->subscriberId.c_str());
    if (argc == 2)
    {
        napi_create_reference(env, argv[1], CALLBACK_REF_CNT, &context->callbackRef);
    }
    napi_value result =
        NapiCommon::HandleAsyncWork(env, context, "ResetFactory", ExecResetFactory, CompleteResetFactory);
    return result;
}
bool MatchSetBackgroundPolicyParameters(napi_env env, napi_value argv[], size_t argc)
{
    switch (argc)
    {
    case ARGV_INDEX_1:
    {
        return NapiCommon::MatchParameters(env, argv, {napi_function});
    }
    case ARGV_INDEX_2:
    {
        return NapiCommon::MatchParameters(env, argv, {napi_boolean, napi_function});
    }
    default:
    {
        return false;
    }
    }
}

void NapiNetPolicy::ExecSetBackgroundPolicy(napi_env env, void *data)
{
    auto context = static_cast<SetBackgroundPolicyContext *>(data);
    context->resultCode =
    DelayedSingleton<NetPolicyClient>::GetInstance()->SetBackgroundPolicy(context->backgroundPolicy);
    context->resolved = true;
}

void NapiNetPolicy::CompleteSetBackgroundPolicy(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_I("CompleteSetBackgroundPolicy start");
    auto context = static_cast<SetBackgroundPolicyContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok)
    {
        if (context->resolved)
        {
            napi_create_int32(env, static_cast<int32_t>(context->resultCode), &callbackValue);
        }
        else
        {
            callbackValue = NapiCommon::CreateErrorMessage(env, "set background error");
        }
    }
    else
    {
        callbackValue = NapiCommon::CreateErrorMessage(
            env, "set background error cause napi_status = " + std::to_string(status));
    }
    NapiCommon::Handle2ValueCallback(env, context, callbackValue);
}

napi_value NapiNetPolicy::SetBackgroundPolicy(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr, nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, MatchSetBackgroundPolicyParameters(env, argv, argc), "type mismatch");

    auto context = std::make_unique<SetBackgroundPolicyContext>().release();
    napi_value enableValue = NapiCommon::GetNamedProperty(env, argv[0], "backgroundPolicy");
    if (enableValue != nullptr)
    {
        napi_get_value_bool(env, enableValue, &context->backgroundPolicy);
    }
    if (argc == 2)
    {
        napi_create_reference(env, argv[1], CALLBACK_REF_CNT, &context->callbackRef);
    }
    napi_value result =
        NapiCommon::HandleAsyncWork(env, context, "SetBackgroundPolicy", ExecSetBackgroundPolicy,
        CompleteSetBackgroundPolicy);
    return result;
}

bool MatchGetBackgroundPolicyParameters(napi_env env, napi_value argv[], size_t argc)
{
    switch (argc)
    {
    case ARGV_INDEX_1:
    {
        return NapiCommon::MatchParameters(env, argv, {napi_function});
    }
    default:
    {
        return false;
    }
    }
}

void NapiNetPolicy::ExecGetBackgroundPolicy(napi_env env, void *data)
{
    auto context = static_cast<SetBackgroundPolicyContext *>(data);
    context->backgroundPolicy = DelayedSingleton<NetPolicyClient>::GetInstance()->GetBackgroundPolicy();
    context->resolved = true;
}

void NapiNetPolicy::CompleteGetBackgroundPolicy(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_I("CompleteSetBackgroundPolicy start");
    auto context = static_cast<SetBackgroundPolicyContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok)
    {
        if (context->resolved)
        {
            napi_get_boolean(env, context->backgroundPolicy, &callbackValue);
        }
        else
        {
            callbackValue = NapiCommon::CreateErrorMessage(env, "set background error");
        }
    }
    else
    {
        callbackValue = NapiCommon::CreateErrorMessage(
            env, "set background error cause napi_status = " + std::to_string(status));
    }
    NapiCommon::Handle2ValueCallback(env, context, callbackValue);
}

napi_value NapiNetPolicy::GetBackgroundPolicy(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, MatchGetBackgroundPolicyParameters(env, argv, argc), "type mismatch");
    auto context = std::make_unique<GetBackgroundPolicyContext>().release();
    if (argc == 1)
    {
        napi_create_reference(env, argv[0], CALLBACK_REF_CNT, &context->callbackRef);
    }
    napi_value result =
        NapiCommon::HandleAsyncWork(env, context, "GetBackgroundPolicy", ExecGetBackgroundPolicy,
        CompleteGetBackgroundPolicy);
    return result;
}

bool MatchGetBackgroundPolicyByUidParameters(napi_env env, napi_value argv[], size_t argc)
{
    switch (argc)
    {
    case ARGV_INDEX_1:
    {
        return NapiCommon::MatchParameters(env, argv, {napi_function});
    }
    case ARGV_INDEX_2:
    {
        return NapiCommon::MatchParameters(env, argv, {napi_number, napi_function});
    }
    default:
    {
        return false;
    }
    }
}

void NapiNetPolicy::ExecGetBackgroundPolicyByUid(napi_env env, void *data)
{
    auto context = static_cast<GetBackgroundPolicyByUidContext *>(data);
    context->backgroundPolicy =
    DelayedSingleton<NetPolicyClient>::GetInstance()->GetBackgroundPolicyByUid(context->uid); context->resolved
    = true;
}

void NapiNetPolicy::CompleteGetBackgroundPolicyByUid(napi_env env, napi_status status, void *data)
{
    NETMGR_LOG_I("CompleteGetBackgroundPolicyByUid start");
    auto context = static_cast<GetBackgroundPolicyByUidContext *>(data);
    napi_value callbackValue = nullptr;
    if (status == napi_ok)
    {
        if (context->resolved)
        {
            napi_get_boolean(env, context->backgroundPolicy, &callbackValue);
        }
        else
        {
            callbackValue = NapiCommon::CreateErrorMessage(env, "get background error");
        }
    }
    else
    {
        callbackValue = NapiCommon::CreateErrorMessage(
            env, "get background error cause napi_status = " + std::to_string(status));
    }
    NapiCommon::Handle2ValueCallback(env, context, callbackValue);
}

napi_value NapiNetPolicy::GetBackgroundPolicyByUid(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr, nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, MatchGetBackgroundPolicyByUidParameters(env, argv, argc), "type mismatch");
    auto context = std::make_unique<GetBackgroundPolicyByUidContext>().release();
    napi_value uidValue = nullptr;
    napi_get_named_property(env, argv[0], "uid", &uidValue);
    napi_get_value_int32(env, uidValue, &context->uid);
    if (argc == 2)
    {
        napi_create_reference(env, argv[1], CALLBACK_REF_CNT, &context->callbackRef);
    }
    napi_value result =
        NapiCommon::HandleAsyncWork(env, context, "GetBackgroundPolicyByUid", ExecGetBackgroundPolicyByUid,
        CompleteGetBackgroundPolicyByUid);
    return result;
}

void NapiNetPolicy::ExecGetCurrentBackgroundPolicy(napi_env env, void *data)
{
    auto context = static_cast<GetBackgroundPolicyContext *>(data);
    context->backgroundPolicy = DelayedSingleton<NetPolicyClient>::GetInstance()->GetCurrentBackgroundPolicy();
    context->resolved = true;
}

napi_value NapiNetPolicy::GetCurrentBackgroundPolicy(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, MatchGetBackgroundPolicyParameters(env, argv, argc), "type mismatch");
    auto context = std::make_unique<GetBackgroundPolicyContext>().release();
    if (argc == 1)
    {
        napi_create_reference(env, argv[0], CALLBACK_REF_CNT, &context->callbackRef);
    }
    napi_value result =
        NapiCommon::HandleAsyncWork(env, context, "GetCurrentBackgroundPolicy", ExecGetCurrentBackgroundPolicy,
        CompleteGetBackgroundPolicy);
    return result;
}

napi_value NapiNetPolicy::DeclareNapiNetPolicyInterface(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("setUidPolicy", SetUidPolicy),
        DECLARE_NAPI_FUNCTION("getUidPolicy", GetUidPolicy),
        DECLARE_NAPI_FUNCTION("getUids", GetUids),
        DECLARE_NAPI_FUNCTION("isUidNetAccess", IsUidNetAccess),
        DECLARE_NAPI_FUNCTION("setNetPolicys", SetNetPolicys),
        DECLARE_NAPI_FUNCTION("getNetPolicys", GetNetPolicys),
        DECLARE_NAPI_FUNCTION("snoozePolicy", SnoozePolicy),
        DECLARE_NAPI_FUNCTION("setIdleWhitelist", SetIdleWhitelist),
        DECLARE_NAPI_FUNCTION("getIdleWhitelist", GetIdleWhitelist),
		DECLARE_NAPI_FUNCTION("resetFactory", ResetFactory),
        DECLARE_NAPI_FUNCTION("setBackgroundPolicy", SetBackgroundPolicy),
        DECLARE_NAPI_FUNCTION("getBackgroundPolicy", GetBackgroundPolicy),
        DECLARE_NAPI_FUNCTION("getBackgroundPolicyByUid", GetBackgroundPolicyByUid),
        DECLARE_NAPI_FUNCTION("getCurrentBackgroundPolicy", GetCurrentBackgroundPolicy),
		
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiNetPolicy::SetUidPolicy(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_3;
    napi_value argv[] = {nullptr, nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetPolicyAsyncContext* context = std::make_unique<NetPolicyAsyncContext>().release();
    // Parse Js argv;
    NAPI_CALL(env, napi_get_value_uint32(env, argv[ARGV_INDEX_0], &context->uid));
    NAPI_CALL(env, napi_get_value_uint32(env, argv[ARGV_INDEX_1], &context->policy));
    NETMGR_LOG_I("JS agvc count = [%{public}d], argv[ARGV_INDEX_0] = [%{public}d], argv[ARGV_INDEX_1] = [%{public}d]",
        static_cast<int>(argc), context->uid, static_cast<int>(context->policy));
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
        NETMGR_LOG_E("SetUidPolicy exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "SetUidPolicy", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecSetUidPolicy,
        CompleteSetUidPolicy,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetPolicy::GetUidPolicy(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetPolicyAsyncContext* context = std::make_unique<NetPolicyAsyncContext>().release();
    // Parse Js argv;
    NAPI_CALL(env, napi_get_value_uint32(env, argv[ARGV_INDEX_0], &context->uid));
    NETMGR_LOG_E("JS agvc count = [%{public}d], argv[0] = [%{public}d]",
        static_cast<int32_t>(argc), context->uid);
    // Get and verify parameter[js]
    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) {
        if (!context->callbackRef) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("GetUidPolicy exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "getUids", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetUidPolicy,
        CompleteGetUidPolicy,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetPolicy::GetUids(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr} ;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetPolicyAsyncContext* context = std::make_unique<NetPolicyAsyncContext>().release();
    // Parse Js argv;
    NAPI_CALL(env, napi_get_value_uint32(env, argv[ARGV_INDEX_0], &context->policy));
    NETMGR_LOG_E("JS agvc count = [%{public}d], argv[0] = [%{public}d]",
        static_cast<int32_t>(argc), context->policy);
    // Get and verify parameter[js]
    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) {
        if (!context->callbackRef) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("GetUids exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "getUids", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetUids,
        CompleteGetUids,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetPolicy::IsUidNetAccess(napi_env env, napi_callback_info info)
{
    napi_valuetype valueType = napi_undefined;
    size_t argc = ARGV_NUM_3;
    napi_value argv[] = {nullptr, nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetPolicyAsyncContext* context = std::make_unique<NetPolicyAsyncContext>().release();
    NAPI_CALL(env, napi_get_value_uint32(env, argv[ARGV_INDEX_0], &context->uid));
    napi_typeof(env, argv[ARGV_INDEX_1], &valueType);
    if (napi_valuetype::napi_boolean == valueType) {
        NAPI_CALL(env, napi_get_value_bool(env, argv[ARGV_INDEX_1], &context->metered));
    }
    if (napi_valuetype::napi_string == valueType) {
        char buf[BUFFER_BYTE] = {0};
        size_t typeLen = 0;
        napi_get_value_string_utf8(env, argv[ARGV_INDEX_1], buf, sizeof(buf), &typeLen);
        context->interfaceName = buf;
        NETMGR_LOG_E("interfaceName = [%{public}s].\n", context->interfaceName.c_str());
    }
    NETMGR_LOG_I("JS agvc count = [%{public}d], argv[0] = [%{public}d], argv[1] = [%{public}d]",
        static_cast<int32_t>(argc), context->uid, context->metered);
    napi_value result = nullptr;
    if (argc == ARGV_NUM_2) {
        if (!context->callbackRef) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result));
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_3) {
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_2], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("IsUidNetAccess exception");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "IsUidNetAccess", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecIsUidNetAccess,
        CompleteIsUidNetAccess,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetPolicy::SetNetPolicys(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_D("SetNetPolicys");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetPolicyAsyncContext* context = std::make_unique<NetPolicyAsyncContext>().release();

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[0], &isArray));
    if (!isArray) {
        NETMGR_LOG_D("An array parameter is required.");
        return nullptr;
    }
    uint32_t arrayLength = 0;
    napi_get_array_length(env, argv[0], &arrayLength);
    napi_value elementValue = nullptr;
    for (uint32_t i = 0; i < arrayLength; i++) {
        napi_get_element(env, argv[0], i, &elementValue);
        NetPolicyQuotaPolicy data = ReadQuotaPolicy(env, elementValue);
        context->quotaPolicys.push_back(data);
    }

    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) { // promise call
        if (!context->callbackRef) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        // exception
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "setNetPolicys", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecSetNetPolicys,
        CompleteSetNetPolicys,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetPolicy::SnoozePolicy(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_D("SnoozePolicy");
    size_t argc = ARGV_NUM_2;
    napi_value argv[] = {nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetPolicyAsyncContext* context = std::make_unique<NetPolicyAsyncContext>().release();

    napi_valuetype type;
    NAPI_CALL(env, napi_typeof(env, argv[ARGV_INDEX_0], &type));
    if (type == napi_object) {
        context->quotaPolicy = ReadQuotaPolicy(env, argv[ARGV_INDEX_0]);
    } else {
        NETMGR_LOG_E("Unexpected parameters.");
    }

    napi_value result = nullptr;
    if (argc == ARGV_NUM_1) { // promise call
        if (!context->callbackRef) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_2) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("Unexpected parameters.");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "snoozePolicy", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecSnoozePolicy,
        CompleteSnoozePolicy,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetPolicy::SetIdleWhitelist(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_D("setIdleWhitelist");
    size_t argc = ARGV_NUM_3;
    napi_value argv[] = {nullptr, nullptr, nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetPolicyAsyncContext* context = std::make_unique<NetPolicyAsyncContext>().release();
    NAPI_CALL(env, napi_get_value_uint32(env, argv[ARGV_INDEX_0], &context->uid));
    NAPI_CALL(env, napi_get_value_bool(env, argv[ARGV_INDEX_1], &context->isWhiteList));

    napi_value result = nullptr;
    if (argc == ARGV_NUM_2) { // promise call
        if (!context->callbackRef) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_3) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_2], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("Unexpected parameters.");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "setIdleWhitelist", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecSetIdleWhitelist,
        CompleteSetIdleWhitelist,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetPolicy::GetIdleWhitelist(napi_env env, napi_callback_info info)
{
    NETMGR_LOG_D("getIdleWhitelist");
    size_t argc = ARGV_NUM_1;
    napi_value argv[] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NetPolicyAsyncContext* context = std::make_unique<NetPolicyAsyncContext>().release();

    napi_value result = nullptr;
    if (argc == ARGV_NUM_0) { // promise call
        if (!context->callbackRef) {
            NAPI_CALL(env, napi_create_promise(env, &context->deferred, &result)); // promise call, other callref call
        } else {
            NAPI_CALL(env, napi_get_undefined(env, &result));
        }
    } else if (argc == ARGV_NUM_1) { // callback
        NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_0], CALLBACK_REF_CNT, &context->callbackRef));
    } else {
        NETMGR_LOG_E("Unexpected parameters.");
    }
    // creat async work
    napi_value resource = nullptr;
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &resource));
    NAPI_CALL(env, napi_create_string_utf8(env, "getIdleWhitelist", NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_async_work(env, resource, resourceName,
        ExecGetIdleWhitelist,
        CompleteGetIdleWhitelist,
        (void *)context,
        &context->work));
    NAPI_CALL(env, napi_queue_async_work(env, context->work));
    return result;
}

napi_value NapiNetPolicy::RegisterNetPolicyInterface(napi_env env, napi_value exports)
{
    NETMGR_LOG_I("RegisterNetPolicyInterface");
    DeclareNapiNetPolicyInterface(env, exports);
    DeclareNapiNetPolicyData(env, exports);
    DeclareNapiNetPolicyResultData(env, exports);
    return nullptr;
}

static napi_module _netPolicyModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = NapiNetPolicy::RegisterNetPolicyInterface,
    .nm_modname = "netmanager.netpolicy",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterNetPolicyModule(void)
{
    napi_module_register(&_netPolicyModule);
}
} // namespace NetManagerStandard
} // namespace OHOS
