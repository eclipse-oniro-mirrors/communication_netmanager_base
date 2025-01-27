/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#include "pacurl_context.h"
 
#include "napi_constant.h"
#include "napi_utils.h"
#include "netmanager_base_log.h"
 
namespace OHOS {
namespace NetManagerStandard {
SetPacUrlContext::SetPacUrlContext(napi_env env, EventManager *manager) : BaseContext(env, manager) {}
 
bool SetPacUrlContext::CheckParamsType(napi_env env, napi_value *params, size_t paramsCount)
{
    if (paramsCount == PARAM_JUST_OPTIONS) {
        return NapiUtils::GetValueType(env, params[ARG_INDEX_0]) == napi_string;
    }
    return false;
}
 
void SetPacUrlContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(GetEnv(), params, paramsCount)) {
        NETMANAGER_BASE_LOGE("check params type failed");
        SetParseOK(false);
        SetNeedThrowException(true);
        SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
        return;
    }
    pacUrl_ = NapiUtils::GetStringFromValueUtf8(GetEnv(), params[ARG_INDEX_0]);
    SetParseOK(true);
}
 
GetPacUrlContext::GetPacUrlContext(napi_env env, EventManager *manager) : BaseContext(env, manager) {}
 
void GetPacUrlContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (paramsCount != PARAM_NONE) {
        SetParseOK(false);
        return;
    }
    SetParseOK(true);
}
 
} // namespace NetManagerStandard
} // namespace OHOS