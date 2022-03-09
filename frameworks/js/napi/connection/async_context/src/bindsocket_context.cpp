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

#include "bindsocket_context.h"

#include "constant.h"
#include "netmanager_base_log.h"
#include "netmanager_base_napi_utils.h"

namespace OHOS::NetManagerStandard {
BindSocketContext::BindSocketContext(napi_env env, EventManager *manager)
    : BaseContext(env, manager), netId(0), socketFd(0)
{
}

void BindSocketContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(params, paramsCount)) {
        return;
    }

    socketFd = NapiUtils::GetInt32Property(GetEnv(), params[0], KEY_SOCKET_FD);
    if (socketFd == 0) {
        NETMANAGER_BASE_LOGE("socket is not bind");
        return;
    }

    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        SetParseOK(SetCallback(params[1]) == napi_ok);
        return;
    }
    NETMANAGER_BASE_LOGI("socket is %{public}d", socketFd);
    SetParseOK(true);
}

bool BindSocketContext::CheckParamsType(napi_value *params, size_t paramsCount)
{
    if (paramsCount == PARAM_JUST_OPTIONS) {
        return NapiUtils::GetValueType(GetEnv(), params[0]) == napi_object;
    }

    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        return NapiUtils::GetValueType(GetEnv(), params[0]) == napi_object &&
               NapiUtils::GetValueType(GetEnv(), params[1]) == napi_function;
    }
    return false;
}
} // namespace OHOS::NetManagerStandard