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

#include "connection_async_work.h"

#include "connection_exec.h"
#include "netmanager_base_base_async_work.h"

namespace OHOS::NetManagerStandard {
void ConnectionAsyncWork::ExecGetDefaultNet(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetDefaultNetContext, ConnectionExec::ExecGetDefaultNet>(env, data);
}

void ConnectionAsyncWork::GetDefaultNetCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetDefaultNetContext, ConnectionExec::GetDefaultNetCallback>(env, status, data);
}

void ConnectionAsyncWork::NetHandleAsyncWork::ExecGetAddressesByName(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetAddressByNameContext, ConnectionExec::NetHandleExec::ExecGetAddressesByName>(env,
                                                                                                                 data);
}

void ConnectionAsyncWork::NetHandleAsyncWork::GetAddressesByNameCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetAddressByNameContext,
                                     ConnectionExec::NetHandleExec::GetAddressesByNameCallback>(env, status, data);
}

void ConnectionAsyncWork::NetHandleAsyncWork::ExecGetAddressByName(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<GetAddressByNameContext, ConnectionExec::NetHandleExec::ExecGetAddressByName>(env,
                                                                                                               data);
}

void ConnectionAsyncWork::NetHandleAsyncWork::GetAddressByNameCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<GetAddressByNameContext, ConnectionExec::NetHandleExec::GetAddressByNameCallback>(
        env, status, data);
}

void ConnectionAsyncWork::NetConnectionAsyncWork::ExecRegister(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<RegisterContext, ConnectionExec::NetConnectionExec::ExecRegister>(env, data);
}

void ConnectionAsyncWork::NetConnectionAsyncWork::RegisterCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<RegisterContext, ConnectionExec::NetConnectionExec::RegisterCallback>(env, status,
                                                                                                           data);
}

void ConnectionAsyncWork::NetConnectionAsyncWork::ExecUnregister(napi_env env, void *data)
{
    BaseAsyncWork::ExecAsyncWork<UnregisterContext, ConnectionExec::NetConnectionExec::ExecUnregister>(env, data);
}

void ConnectionAsyncWork::NetConnectionAsyncWork::UnregisterCallback(napi_env env, napi_status status, void *data)
{
    BaseAsyncWork::AsyncWorkCallback<UnregisterContext, ConnectionExec::NetConnectionExec::UnregisterCallback>(
        env, status, data);
}
} // namespace OHOS::NetManagerStandard