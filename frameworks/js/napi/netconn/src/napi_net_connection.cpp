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

#include "napi_net_connection.h"
#include <memory>
#include "event_listener_context.h"
#include "napi_common.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
struct ObserverContext : public BaseContext {
    NapiNetConnection *thisObj;
    int32_t eventId;
};

void OnExecute(napi_env env, void *data)
{
    if (data == nullptr) {
        NETMGR_LOG_E("onExecute data is nullptr");
        return;
    }
    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
    EventListener listener;
    listener.eventId = asyncContext->eventId;
    listener.env = env;
    listener.callbackRef = asyncContext->callbackRef;
    int32_t result = EventListenerContext::GetInstance().AddListense(asyncContext->thisObj, listener);
    asyncContext->resolved = !result;
}

void OnComplete(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        NETMGR_LOG_E("onComplete data is nullptr");
        return;
    }
    std::unique_ptr<ObserverContext> asyncContext(static_cast<ObserverContext *>(data));
    if (!asyncContext->resolved) {
        NETMGR_LOG_E("onComplete error by add observer failed");
    }
    napi_delete_async_work(env, asyncContext->work);
}

void RegisterExecute(napi_env env, void *data)
{
    if (data == nullptr) {
        NETMGR_LOG_E("registerExecute data is nullptr");
        return;
    }
    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
    asyncContext->errorCode = EventListenerContext::GetInstance().Register(asyncContext->thisObj);
    asyncContext->resolved = !asyncContext->errorCode;
}

void RegisterComplete(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        NETMGR_LOG_E("registerComplete data is nullptr");
        return;
    }
    std::unique_ptr<ObserverContext> asyncContext(static_cast<ObserverContext *>(data));
    napi_value callbackValue = nullptr;
    if (!asyncContext->resolved) {
        callbackValue = NapiCommon::CreateErrorMessage(
            env, "registerComplete error by add observer failed", asyncContext->errorCode);
    } else {
        callbackValue = NapiCommon::CreateUndefined(env);
    }
    NapiCommon::Handle2ValueCallback(env, asyncContext.release(), callbackValue);
}

void UnregisterExecute(napi_env env, void *data)
{
    if (data == nullptr) {
        NETMGR_LOG_E("UnregisterExecute data is nullptr");
        return;
    }
    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
    asyncContext->errorCode = EventListenerContext::GetInstance().Unregister(asyncContext->thisObj);
    asyncContext->resolved = !asyncContext->errorCode;
}

void UnregisterComplete(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        NETMGR_LOG_E("UnregisterComplete data is nullptr");
        return;
    }
    std::unique_ptr<ObserverContext> asyncContext(static_cast<ObserverContext *>(data));
    napi_value callbackValue = nullptr;
    if (!asyncContext->resolved) {
        callbackValue = NapiCommon::CreateErrorMessage(env, "unregister failed", asyncContext->errorCode);
    } else {
        callbackValue = NapiCommon::CreateUndefined(env);
    }
    NapiCommon::Handle2ValueCallback(env, asyncContext.release(), callbackValue);
}
} // namespace

napi_value NapiNetConnection::On(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[] = {nullptr, nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    if (argc != std::size(argv)) {
        NETMGR_LOG_E("Invalid number of arguments");
        return nullptr;
    }

    NapiNetConnection *objectInfo = nullptr;
    if (NapiCommon::MatchValueType(env, thisVar, napi_object)) {
        napi_unwrap(env, thisVar, (void **)&objectInfo);
    }
    if (objectInfo == nullptr) {
        NETMGR_LOG_E("this object parsing failed!");
        return nullptr;
    }

    std::unique_ptr<ObserverContext> asyncContext = std::make_unique<ObserverContext>();
    asyncContext->thisObj = objectInfo;
    if (!NapiCommon::MatchValueType(env, argv[ARGV_INDEX_0], napi_string)) {
        NETMGR_LOG_E("the first parameter type is invalid！");
        return nullptr;
    }
    std::string type = NapiCommon::GetStringFromValue(env, argv[ARGV_INDEX_0]);
    if (!NapiCommon::IsValidEvent(type, asyncContext->eventId)) {
        NETMGR_LOG_E("Invalid listen type");
        return nullptr;
    }
    if (!NapiCommon::MatchValueType(env, argv[ARGV_INDEX_1], napi_function)) {
        NETMGR_LOG_E("the second parameter type is invalid！");
    }
    NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_1], 1, &asyncContext->callbackRef));
    return NapiCommon::HandleAsyncWork(env, asyncContext.release(), "On", OnExecute, OnComplete);
}

napi_value NapiNetConnection::Register(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    if (argc != std::size(argv)) {
        NETMGR_LOG_E("Invalid number of arguments");
        return nullptr;
    }

    NapiNetConnection *objectInfo = nullptr;
    if (NapiCommon::MatchValueType(env, thisVar, napi_object)) {
        napi_unwrap(env, thisVar, (void **)&objectInfo);
    }
    if (objectInfo == nullptr) {
        NETMGR_LOG_E("this object parsing failed!");
        return nullptr;
    }

    std::unique_ptr<ObserverContext> asyncContext = std::make_unique<ObserverContext>();
    asyncContext->thisObj = objectInfo;
    if (!NapiCommon::MatchValueType(env, argv[ARGV_INDEX_0], napi_function)) {
        NETMGR_LOG_E("Invalid type of argument");
    }
    NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_0], 1, &asyncContext->callbackRef));
    return NapiCommon::HandleAsyncWork(env, asyncContext.release(), "Register", RegisterExecute, RegisterComplete);
}

napi_value NapiNetConnection::Unregister(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    if (argc != std::size(argv)) {
        NETMGR_LOG_E("Invalid number of arguments");
        return nullptr;
    }

    NapiNetConnection *objectInfo = nullptr;
    if (NapiCommon::MatchValueType(env, thisVar, napi_object)) {
        napi_unwrap(env, thisVar, (void **)&objectInfo);
    }
    if (objectInfo == nullptr) {
        NETMGR_LOG_E("this object parsing failed!");
        return nullptr;
    }

    std::unique_ptr<ObserverContext> asyncContext = std::make_unique<ObserverContext>();
    asyncContext->thisObj = objectInfo;
    if (!NapiCommon::MatchValueType(env, argv[ARGV_INDEX_0], napi_function)) {
        NETMGR_LOG_E("Invalid type of argument");
    }
    NAPI_CALL(env, napi_create_reference(env, argv[ARGV_INDEX_0], 1, &asyncContext->callbackRef));
    return NapiCommon::HandleAsyncWork(
        env, asyncContext.release(), "Unregister", UnregisterExecute, UnregisterComplete);
}
} // namespace NetManagerStandard
} // namespace OHOS