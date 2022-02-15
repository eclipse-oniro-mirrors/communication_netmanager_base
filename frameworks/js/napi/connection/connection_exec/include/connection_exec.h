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

#ifndef COMMUNICATIONNETMANAGERBASE_CONNECTION_EXEC_H
#define COMMUNICATIONNETMANAGERBASE_CONNECTION_EXEC_H

#include "getaddressbyname_context.h"
#include "getdefaultnet_context.h"
#include "napi/native_api.h"
#include "noncopyable.h"

namespace OHOS::NetManagerBase {
class ConnectionExec final {
public:
    ACE_DISALLOW_COPY_AND_MOVE(ConnectionExec);

    ConnectionExec() = delete;

    ~ConnectionExec() = delete;

    static bool ExecGetDefaultNet(GetDefaultNetContext *context);

    static napi_value GetDefaultNetCallback(GetDefaultNetContext *context);

    class NetHandleExec final {
    public:
        ACE_DISALLOW_COPY_AND_MOVE(NetHandleExec);

        NetHandleExec() = delete;

        ~NetHandleExec() = delete;

        static bool ExecGetAddressByName(GetAddressByNameContext *context);

        static napi_value GetAddressByNameCallback(GetAddressByNameContext *context);

        static bool ExecGetAddressesByName(GetAddressByNameContext *context);

        static napi_value GetAddressesByNameCallback(GetAddressByNameContext *context);

    private:
        static napi_value MakeNetAddressJsValue(napi_env env, const NetAddress &address);
    };
};

} // namespace OHOS::NetManagerBase

#endif /* COMMUNICATIONNETMANAGERBASE_CONNECTION_EXEC_H */
