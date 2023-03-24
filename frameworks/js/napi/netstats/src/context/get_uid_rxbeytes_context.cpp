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

#include "get_uid_rxbeytes_context.h"

#include "constant.h"
#include "napi_constant.h"
#include "napi_utils.h"

namespace OHOS {
namespace NetManagerStandard {
GetUidRxBytesContext::GetUidRxBytesContext(napi_env env, EventManager *manager) : BaseContext(env, manager) {}

void GetUidRxBytesContext::ParseParams(napi_value *params, size_t paramsCount)
{
    if (!CheckParamsType(params, paramsCount)) {
        return;
    }

    uid_ = static_cast<int32_t>(NapiUtils::GetUint32FromValue(GetEnv(), params[ARG_INDEX_0]));

    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        SetParseOK(SetCallback(params[ARG_INDEX_1]) == napi_ok);
        return;
    }
    SetParseOK(true);
}

bool GetUidRxBytesContext::CheckParamsType(napi_value *params, size_t paramsCount)
{
    if (paramsCount == PARAM_JUST_OPTIONS) {
        return true;
    }
    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        return NapiUtils::GetValueType(GetEnv(), params[ARG_INDEX_1]) == napi_function;
    }
    return false;
}

void GetUidRxBytesContext::SetBytes64(int64_t bytes64)
{
    bytes64_ = bytes64;
}

int64_t GetUidRxBytesContext::GetBytes64() const
{
    return bytes64_;
}

void GetUidRxBytesContext::SetUid(int32_t uid)
{
    uid_ = uid;
}

int32_t GetUidRxBytesContext::GetUid() const
{
    return uid_;
}
} // namespace NetManagerStandard
} // namespace OHOS
