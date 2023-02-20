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

#include "global_http_proxy_context.h"

#include <set>
#include <string>

#include "napi_constant.h"
#include "napi_utils.h"
#include "netmanager_base_log.h"

namespace OHOS {
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
    if (paramsCount == 1 || paramsCount == 2) {
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

GlobalHttpProxyContext::GlobalHttpProxyContext(napi_env env, EventManager *manager) : BaseContext(env, manager) {}

void GlobalHttpProxyContext::ParseParams(napi_value *params, size_t paramsCount)
{
    InterfaceType type;
    if (!CheckParamsType(GetEnv(), params, paramsCount, type)) {
        NETMANAGER_BASE_LOGE("check params type failed");
        return;
    }
    if (type == GET) {
        if (paramsCount == 1) {
            SetParseOK(SetCallback(params[0]) == napi_ok);
            return;
        }
    }
    if (type == SET) {
        httpProxy_.SetHost(NapiUtils::GetStringPropertyUtf8(GetEnv(), params[0], "host"));
        httpProxy_.SetPort(static_cast<uint16_t>(NapiUtils::GetUint32Property(GetEnv(), params[0], "port")));

        std::set<std::string> list;
        napi_value parsedExclusionList = NapiUtils::GetNamedProperty(GetEnv(), params[0], "parsedExclusionList");
        uint32_t listLength = NapiUtils::GetArrayLength(GetEnv(), parsedExclusionList);
        for (uint32_t i = 0; i < listLength; ++i) {
            napi_value element = NapiUtils::GetArrayElement(GetEnv(), parsedExclusionList, i);
            list.insert(NapiUtils::GetStringFromValueUtf8(GetEnv(), element));
        }
        httpProxy_.SetExclusionList(list);

        if (paramsCount == 2) {
            SetParseOK(SetCallback(params[1]) == napi_ok);
            return;
        }
    }
    SetParseOK(true);
}
} // namespace NetManagerStandard
} // namespace OHOS
