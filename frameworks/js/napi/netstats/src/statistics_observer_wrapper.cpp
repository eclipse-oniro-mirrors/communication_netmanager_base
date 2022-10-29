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

#include "statistics_observer_wrapper.h"

#include "constant.h"
#include "module_template.h"
#include "netmanager_base_log.h"
#include "net_stats_client.h"
#include "net_stats_constants.h"

namespace OHOS {
namespace NetManagerStandard {
StatisticsObserverWrapper::StatisticsObserverWrapper()
    : observer_(new StatisticsCallbackObserver()), manager_(new EventManager()), registed_(false)
{
}

StatisticsObserverWrapper::~StatisticsObserverWrapper()
{
    delete manager_;
}

napi_value StatisticsObserverWrapper::On(napi_env env, napi_callback_info info,
                                         const std::initializer_list<std::string> &events, bool asyncCallback)
{
    size_t paramsCount = MAX_PARAM_NUM;
    napi_value params[MAX_PARAM_NUM] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramsCount, params, nullptr, nullptr));
    if (paramsCount != PARAM_OPTIONS_AND_CALLBACK || NapiUtils::GetValueType(env, params[0]) != napi_string ||
        NapiUtils::GetValueType(env, params[1]) != napi_function) {
        NETMANAGER_BASE_LOGE("on off once interface para: [string, function]");
        return NapiUtils::GetUndefined(env);
    }

    const auto &event = NapiUtils::GetStringFromValueUtf8(env, params[0]);
    if (std::find(events.begin(), events.end(), event) == events.end()) {
        return NapiUtils::GetUndefined(env);
    }
    if (Register()) {
        manager_->AddListener(env, event, params[1], false, asyncCallback);
    } else {
        NETMANAGER_BASE_LOGE("unregister callback or manager is nullptr");
    }
    return NapiUtils::GetUndefined(env);
}

bool StatisticsObserverWrapper::Register()
{
    if (!registed_) {
        int32_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->RegisterNetStatsCallback(observer_);
        NETMANAGER_BASE_LOGI("ret = [%{public}d]", ret);
        registed_ = (ret == static_cast<int32_t>(NetStatsResultCode::ERR_NONE));
    }
    return registed_;
}

napi_value StatisticsObserverWrapper::Off(napi_env env, napi_callback_info info,
                                          const std::initializer_list<std::string> &events, bool asyncCallback)
{
    napi_value thisVal = nullptr;
    size_t paramsCount = MAX_PARAM_NUM;
    napi_value params[MAX_PARAM_NUM] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramsCount, params, &thisVal, nullptr));

    if ((paramsCount != 1 && paramsCount != PARAM_OPTIONS_AND_CALLBACK) ||
        NapiUtils::GetValueType(env, params[0]) != napi_string) {
        NETMANAGER_BASE_LOGE("on off once interface para: [string, function?]");
        return NapiUtils::GetUndefined(env);
    }

    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK && NapiUtils::GetValueType(env, params[1]) != napi_function) {
        NETMANAGER_BASE_LOGE("on off once interface para: [string, function]");
        return NapiUtils::GetUndefined(env);
    }

    std::string event = NapiUtils::GetStringFromValueUtf8(env, params[0]);
    if (std::find(events.begin(), events.end(), event) == events.end()) {
        return NapiUtils::GetUndefined(env);
    }

    if (paramsCount == PARAM_OPTIONS_AND_CALLBACK) {
        manager_->DeleteListener(event, params[1]);
    } else {
        manager_->DeleteListener(event);
    }

    if (manager_->IsListenerListEmpty()) {
        registed_ = false;
        auto ret = DelayedSingleton<NetStatsClient>::GetInstance()->UnregisterNetStatsCallback(observer_);
        if (ret != 0) {
            NETMANAGER_BASE_LOGE("unregister ret = %{public}d", ret);
        }
    }
    return NapiUtils::GetUndefined(env);
}
} // namespace NetManagerStandard
} // namespace OHOS
