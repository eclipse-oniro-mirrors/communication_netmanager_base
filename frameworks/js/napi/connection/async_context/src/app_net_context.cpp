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

#include "app_net_context.h"

#include "napi_constant.h"
#include "napi_utils.h"
#include "netmanager_base_log.h"

namespace OHOS {
static constexpr const int PARAMS_COUNT_TWO = 2;
static constexpr const int PARAMS_COUNT_ONE = 1;

namespace NetManagerStandard {
namespace {
enum InterfaceType {
    GET,
    SET,
};

bool CheckParamsType(napi_env env, napi_value *params, size_t paramsCount, InterfaceType &type)
{
    if (paramsCount == 0) {
        type = GET;
        return true;
    }
    if (paramsCount == PARAMS_COUNT_ONE || paramsCount == PARAMS_COUNT_TWO) {
        if (NapiUtils::GetValueType(env, params[0]) == napi_object) {
            type = SET;
            return true;
        }
        if (NapiUtils::GetValueType(env, params[0]) == napi_function) {
            type = GET;
            return true;
        }
    }
    return false;
}
} // namespace

AppNetContext::AppNetContext(napi_env env, EventManager *manager) : BaseContext(env, manager) {}

void AppNetContext::ParseParams(napi_value *params, size_t paramsCount)
{
    InterfaceType type;
    if (!CheckParamsType(GetEnv(), params, paramsCount, type)) {
        NETMANAGER_BASE_LOGE("check params type failed");
        SetNeedThrowException(true);
        SetError(PARSE_ERROR_CODE, PARSE_ERROR_MSG);
        return;
    }
    if (type == GET) {
        if (paramsCount == PARAMS_COUNT_ONE) {
            SetParseOK(SetCallback(params[0]) == napi_ok);
            return;
        }
    }
    if (type == SET) {
        netHandle_.SetNetId(NapiUtils::GetInt32Property(GetEnv(), params[0], "netId"));
        if (paramsCount == PARAMS_COUNT_TWO) {
            SetParseOK(SetCallback(params[1]) == napi_ok);
            return;
        }
    }
    SetParseOK(true);
}
} // namespace NetManagerStandard
} // namespace OHOS
