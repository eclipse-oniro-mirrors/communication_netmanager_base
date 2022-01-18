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
#include "napi_common.h"
#include "net_all_capabilities.h"
#include "net_conn_client.h"
#include "net_handle.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t HOST_MAX_BYTES = 256;
static napi_ref *g_constructor = nullptr;

struct NetConnAsyncContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    int32_t result;
    char eventContent[EVENT_CONTENT_MAX_BYTE] = {0};
    size_t eventContentRealBytes = 0;
    int32_t eventNetId = 0;
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
    uint32_t bearerType = 0;
    NetAllCapabilities netAllCap;
    std::list<sptr<NetHandle>> netList;
    NetLinkInfo netLinkInfo;
    int32_t uid = 0;
    bool flag = false;
    bool useDnsResolver = false;
    sptr<NetHandle> addon = nullptr;
};

class NapiNetConn {
public:
    NapiNetConn();
    ~NapiNetConn() = default;
    static napi_value RegisterNetConnInterface(napi_env env, napi_value exports);
    static napi_value DeclareNetConnInterface(napi_env env, napi_value exports);
    static napi_value DeclareNetworkTypeData(napi_env env, napi_value exports);
    static napi_value DeclareNetCapabilityData(napi_env env, napi_value exports);
    static napi_value DeclareNetConnConstructor(napi_env env, napi_value exports);
    static napi_value DeclareNetConnNew(napi_env env, napi_callback_info info);
    static void DeclareNetConnDestructor(napi_env env, void *nativeObject, void *hint);
    static napi_value CreateNetHandle(napi_env env, sptr<NetHandle> &net);

    static void ExecAddNetStatusCallback(napi_env env, void *data);
    static void ExecRemoveNetStatusCallback(napi_env env, void *data);
    static void ExecNetDetection(napi_env env, void *data);
    static void ExecBindSocket(napi_env env, void *data);
    static void ExecGetAddressesByName(napi_env env, void *data);
    static void ExecGetAddressByName(napi_env env, void *data);
    static void ExecGetDefaultNet(napi_env env, void *data);
    static void ExecGetAllNets(napi_env env, void *data);
    static void ExecGetConnectionProperties(napi_env env, void *data);
    static void ExecGetNetCapabilities(napi_env env, void *data);
    static void ExecHasDefaultNet(napi_env env, void *data);
    static void NativeRestoreFactoryData(napi_env env, void *data);
    static void CompleteAddNetStatusCallback(napi_env env, napi_status status, void *data);
    static void CompleteRemoveNetStatusCallback(napi_env env, napi_status status, void *data);
    static void CompleteNetDetection(napi_env env, napi_status status, void *data);
    static void CompleteBindSocket(napi_env env, napi_status status, void *data);
    static void CompleteGetAddressesByName(napi_env env, napi_status status, void *data);
    static void CompleteGetAddressByName(napi_env env, napi_status status, void *data);
    static void CompleteGetDefaultNet(napi_env env, napi_status status, void *data);
    static void CompleteGetAllNets(napi_env env, napi_status status, void *data);
    static void CompleteGetConnectionProperties(napi_env env, napi_status status, void *data);
    static void CompleteGetNetCapabilities(napi_env env, napi_status status, void *data);
    static void CompleteHasDefaultNet(napi_env env, napi_status status, void *data);
    static void RestoreFactoryDataCallback(napi_env env, napi_status status, void *data);

    // Declare napi interfaces for JS;
    static napi_value NetDetection(napi_env env, napi_callback_info info);
    static napi_value BindSocket(napi_env env, napi_callback_info info);
    static napi_value GetAddressesByName(napi_env env, napi_callback_info info);
    static napi_value GetAddressByName(napi_env env, napi_callback_info info);
    static napi_value GetDefaultNet(napi_env env, napi_callback_info info);
    static napi_value GetAllNets(napi_env env, napi_callback_info info);
    static napi_value GetConnectionProperties(napi_env env, napi_callback_info info);
    static napi_value GetNetCapabilities(napi_env env, napi_callback_info info);
    static napi_value HasDefaultNet(napi_env env, napi_callback_info info);
    static napi_value EnableAirplaneMode(napi_env env, napi_callback_info info);
    static napi_value DisableAirplaneMode(napi_env env, napi_callback_info info);
    static napi_value RestoreFactoryData(napi_env env, napi_callback_info info);
    static napi_value AddNetStatusCallback(napi_env env, napi_callback_info info);
    static napi_value RemoveNetStatusCallback(napi_env env, napi_callback_info info);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NAPI_NET_CONN_H
