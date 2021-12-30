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

#ifndef NAPI_NET_POLICY_H
#define NAPI_NET_POLICY_H

#include <string>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "net_policy_constants.h"
#include "net_policy_quota_policy.h"
#include "base_context.h"

namespace OHOS {
namespace NetManagerStandard {
// net policy async context
struct NetPolicyAsyncContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    // uid
    uint32_t uid = 0;
    // policy
    uint32_t policy = 0;
    // result of policy napi
    int32_t policyResult = 0;
    // vector of uid
    std::vector<uint32_t> uidTogether;
    // result of bool type
    bool result = false;
    // metered
    bool metered = false;
    // interface name
    std::string interfaceName;
    // quotaPolicys
    std::vector<NetPolicyQuotaPolicy> quotaPolicys;
    // quotaPolicy
    NetPolicyQuotaPolicy quotaPolicy;
    // result code of NetPolicyResultCode
    NetPolicyResultCode resultCode = NetPolicyResultCode::ERR_NONE;
    // argument of SetIdleWhitelist function
    bool isWhiteList = false;
    std::vector<uint32_t> uids;
};

struct GetNetPolicyContext : BaseContext{
    std::vector<NetPolicyQuotaPolicy> result;
};

struct ResetFactoryContext : BaseContext{
    std::string subscriberId;
    NetPolicyResultCode resultCode;
};

struct SetBackgroundPolicyContext : BaseContext{
    bool backgroundPolicy;
    NetPolicyResultCode resultCode;
};

struct GetBackgroundPolicyContext : BaseContext{
    bool backgroundPolicy;
};

struct GetBackgroundPolicyByUidContext : BaseContext{
    int32_t uid;
    bool backgroundPolicy;
};

class NapiNetPolicy {
public:
    NapiNetPolicy();
    ~NapiNetPolicy() = default;
    static napi_value RegisterNetPolicyInterface(napi_env env, napi_value exports);
    static napi_value DeclareNapiNetPolicyInterface(napi_env env, napi_value exports);
    static napi_value DeclareNapiNetPolicyData(napi_env env, napi_value exports);
    static napi_value DeclareNapiNetPolicyResultData(napi_env env, napi_value exports);

    static void ExecSetUidPolicy(napi_env env, void *data);
    static void ExecGetUids(napi_env env, void *data);
    static void ExecGetUidPolicy(napi_env env, void *data);
    static void ExecIsUidNetAccess(napi_env env, void *data);
    static void ExecSetNetPolicys(napi_env env, void *data);
    static void ExecSnoozePolicy(napi_env env, void *data);
    static void ExecSetIdleWhitelist(napi_env env, void *data);
    static void ExecGetIdleWhitelist(napi_env env, void *data);
    static void ExecResetFactory(napi_env env, void *data);
    static void ExecSetBackgroundPolicy(napi_env env, void *data);
    static void ExecGetBackgroundPolicy(napi_env env, void *data);
    static void ExecGetBackgroundPolicyByUid(napi_env env, void *data);
    static void ExecGetCurrentBackgroundPolicy(napi_env env, void *data);
    static void CompleteSetUidPolicy(napi_env env, napi_status status, void *data);
    static void CompleteGetUids(napi_env env, napi_status status, void *data);
    static void CompleteGetUidPolicy(napi_env env, napi_status status, void *data);
    static void CompleteIsUidNetAccess(napi_env env, napi_status status, void *data);
    static void CompleteSetNetPolicys(napi_env env, napi_status status, void *data);
    static void CompleteSnoozePolicy(napi_env env, napi_status status, void *data);
    static void CompleteSetIdleWhitelist(napi_env env, napi_status status, void *data);
    static void CompleteGetIdleWhitelist(napi_env env, napi_status status, void *data);
    static void CompleteNetPolicyResultCode(napi_env env, napi_status status, void *data);
    static void CompleteResetFactory(napi_env env, napi_status status, void *data);
    static void CompleteSetBackgroundPolicy(napi_env env, napi_status status, void *data);
    static void CompleteGetBackgroundPolicy(napi_env env, napi_status status, void *data);
    static void CompleteGetBackgroundPolicyByUid(napi_env env, napi_status status, void *data);

    // Declare napi interfaces for JS
    static NetPolicyQuotaPolicy ReadQuotaPolicy(napi_env env, napi_value value);
    static napi_value SetUidPolicy(napi_env env, napi_callback_info info);
    static napi_value GetUidPolicy(napi_env env, napi_callback_info info);
    static napi_value GetUids(napi_env env, napi_callback_info info);
    static napi_value IsUidNetAccess(napi_env env, napi_callback_info info);
    static napi_value SetNetPolicys(napi_env env, napi_callback_info info);
    static napi_value GetNetPolicys(napi_env env, napi_callback_info info);
    static napi_value SnoozePolicy(napi_env env, napi_callback_info info);
    static napi_value SetIdleWhitelist(napi_env env, napi_callback_info info);
    static napi_value GetIdleWhitelist(napi_env env, napi_callback_info info);
    static napi_value ResetFactory(napi_env env, napi_callback_info info);
    static napi_value SetBackgroundPolicy(napi_env env, napi_callback_info info);
    static napi_value GetBackgroundPolicy(napi_env env, napi_callback_info info);
    static napi_value GetBackgroundPolicyByUid(napi_env env, napi_callback_info info);
    static napi_value GetCurrentBackgroundPolicy(napi_env env, napi_callback_info info);

};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NAPI_NET_POLICY_H
