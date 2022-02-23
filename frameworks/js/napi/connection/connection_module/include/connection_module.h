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

#ifndef COMMUNICATIONNETMANAGERBASE_CONNECTION_MODULE_H
#define COMMUNICATIONNETMANAGERBASE_CONNECTION_MODULE_H

#include "napi/native_api.h"

namespace OHOS::NetManagerStandard {
class ConnectionModule final {
public:
    static constexpr const char *FUNCTION_GET_DEFAULT_NET = "getDefaultNet";
    static constexpr const char *FUNCTION_CREATE_NET_CONNECTION = "createNetConnection";
    static constexpr const char *INTERFACE_NET_CONNECTION = "NetConnection";

    static napi_value InitConnectionModule(napi_env env, napi_value exports);

    class NetHandleInterface final {
    public:
        static constexpr const char *PROPERTY_NET_ID = "netId";
        static constexpr const char *FUNCTION_GET_ADDRESSES_BY_NAME = "getAddressesByName";
        static constexpr const char *FUNCTION_GET_ADDRESS_BY_NAME = "getAddressByName";

        static napi_value GetAddressesByName(napi_env env, napi_callback_info info);
        static napi_value GetAddressByName(napi_env env, napi_callback_info info);
    };

    class NetConnectionInterface final {
    public:
        static constexpr const char *FUNCTION_ON = "on";
        static constexpr const char *FUNCTION_REGISTER = "register";
        static constexpr const char *FUNCTION_UNREGISTER = "unregister";

        static napi_value On(napi_env env, napi_callback_info info);
        static napi_value Register(napi_env env, napi_callback_info info);
        static napi_value Unregister(napi_env env, napi_callback_info info);
    };

private:
    static napi_value GetDefaultNet(napi_env env, napi_callback_info info);
    static napi_value CreateNetConnection(napi_env env, napi_callback_info info);
};
} // namespace OHOS::NetManagerStandard

#endif /* COMMUNICATIONNETMANAGERBASE_CONNECTION_MODULE_H */
