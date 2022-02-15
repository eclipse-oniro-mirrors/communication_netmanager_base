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

#include "connection_module.h"

#include "connection_async_work.h"
#include "getaddressbyname_context.h"
#include "getdefaultnet_context.h"
#include "netmanager_base_module_template.h"

static constexpr const char *CONNECTION_MODULE_NAME = "net.connection";

namespace OHOS::NetManagerBase {
napi_value ConnectionModule::InitConnectionModule(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_DEFAULT_NET, GetDefaultNet),
    };
    NapiUtils::DefineProperties(env, exports, properties);

    return exports;
}

napi_value ConnectionModule::GetDefaultNet(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetDefaultNetContext>(env, info, FUNCTION_GET_DEFAULT_NET, nullptr,
                                                           ConnectionAsyncWork::ExecGetDefaultNet,
                                                           ConnectionAsyncWork::GetDefaultNetCallback);
}

napi_value ConnectionModule::NetHandle::GetAddressesByName(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAddressByNameContext>(
        env, info, FUNCTION_GET_ADDRESSES_BY_NAME, nullptr,
        ConnectionAsyncWork::NetHandleAsyncWork::ExecGetAddressesByName,
        ConnectionAsyncWork::NetHandleAsyncWork::GetAddressesByNameCallback);
}

napi_value ConnectionModule::NetHandle::GetAddressByName(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAddressByNameContext>(
        env, info, FUNCTION_GET_ADDRESSES_BY_NAME, nullptr,
        ConnectionAsyncWork::NetHandleAsyncWork::ExecGetAddressByName,
        ConnectionAsyncWork::NetHandleAsyncWork::GetAddressByNameCallback);
}

static napi_module g_connectionModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ConnectionModule::InitConnectionModule,
    .nm_modname = CONNECTION_MODULE_NAME,
    .nm_priv = nullptr,
    .reserved = {nullptr},
};

extern "C" __attribute__((constructor)) void RegisterConnectionModule(void)
{
    napi_module_register(&g_connectionModule);
}
} // namespace OHOS::NetManagerBase