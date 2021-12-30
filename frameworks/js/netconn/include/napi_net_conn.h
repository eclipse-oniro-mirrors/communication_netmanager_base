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

#ifndef NAPI_NET_CONN_H
#define NAPI_NET_CONN_H

#include <string>
#include <vector>
#include <list>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace NetManagerStandard {
// JS register event
constexpr int32_t EVENT_NET_CONN_CHANGE = 11;
const std::string NET_CONN_CHANGE = "netconnChange";
constexpr int32_t EVENT_CONTENT_MAX_BYTE = 64;
constexpr int32_t HOST_MAX_BYTES = 256;

struct NetConnAsyncContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    int32_t result;
    char eventContent[EVENT_CONTENT_MAX_BYTE] = {0};
    size_t eventContentRealBytes = 0;
    // NetSpecifier
    std::string identifier;
    int32_t netType = 0;
    int32_t netCapabilities = 0;
    int32_t netId = 0;
    uint32_t reqId = 0;
    int32_t socketId = 0;
    char host[HOST_MAX_BYTES] = {0};
    size_t hostRealBytes = 0;
    std::vector<std::string> addr;
    std::string hostAddress;
    uint32_t type = 0;
    uint64_t cap = 0;
    std::list<int32_t> netIdList;
    napi_value netLinkInfo = nullptr;
    int32_t uid = 0;
};

class NapiNetConn {
public:
    NapiNetConn();
    ~NapiNetConn() = default;
    static napi_value RegisterNetConnInterface(napi_env env, napi_value exports);
    static napi_value DeclareNetConnInterface(napi_env env, napi_value exports);
    static napi_value DeclareNetworkTypeData(napi_env env, napi_value exports);
    static napi_value DeclareNetCapabilityData(napi_env env, napi_value exports);

    static void ExecAddNetStatusCallback(napi_env env, void *data);
    static void ExecOn(napi_env env, void *data);
    static void ExecOff(napi_env env, void *data);
    static void ExecDeactivateNetwork(napi_env env, void *data);
    static void ExecBindSocket(napi_env env, void *data);
    static void ExecGetAddressesByName(napi_env env, void *data);
    static void ExecGetAddressByName(napi_env env, void *data);
    static void ExecGetDefaultNet(napi_env env, void *data);
    static void ExecGetSpecificNet(napi_env env, void *data);
    static void ExecGetAllNets(napi_env env, void *data);
    static void ExecGetConnectionProperties(napi_env env, void *data);
    static void ExecGetNetCapabilities(napi_env env, void *data);
    static void ExecGetSpecificUidNet(napi_env env, void *data);
    static void CompleteAddNetStatusCallback(napi_env env, napi_status status, void *data);
    static void CompleteOn(napi_env env, napi_status status, void *data);
    static void CompleteOff(napi_env env, napi_status status, void *data);
    static void CompleteDeactivateNetwork(napi_env env, napi_status status, void *data);
    static void CompleteBindSocket(napi_env env, napi_status status, void *data);
    static void CompleteGetAddressesByName(napi_env env, napi_status status, void *data);
    static void CompleteGetAddressByName(napi_env env, napi_status status, void *data);
    static void CompleteGetDefaultNet(napi_env env, napi_status status, void *data);
    static void CompleteGetSpecificNet(napi_env env, napi_status status, void *data);
    static void CompleteGetAllNets(napi_env env, napi_status status, void *data);
    static void CompleteGetConnectionProperties(napi_env env, napi_status status, void *data);
    static void CompleteGetNetCapabilities(napi_env env, napi_status status, void *data);
    static void CompleteGetSpecificUidNet(napi_env env, napi_status status, void *data);
    // Declare napi interfaces for JS
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value DeactivateNetwork(napi_env env, napi_callback_info info);
    static napi_value BindSocket(napi_env env, napi_callback_info info);
    static napi_value GetAddressesByName(napi_env env, napi_callback_info info);
    static napi_value GetAddressByName(napi_env env, napi_callback_info info);
    static napi_value GetDefaultNet(napi_env env, napi_callback_info info);
    static napi_value GetSpecificNet(napi_env env, napi_callback_info info);
    static napi_value GetAllNets(napi_env env, napi_callback_info info);
    static napi_value GetConnectionProperties(napi_env env, napi_callback_info info);
    static napi_value GetNetCapabilities(napi_env env, napi_callback_info info);
    static napi_value GetSpecificUidNet(napi_env env, napi_callback_info info);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NAPI_NET_CONN_H
