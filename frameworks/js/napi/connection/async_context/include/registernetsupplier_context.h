/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef NETMANAGER_BASE_NAPI_REGISTER_NET_SUPPLIER_CONTEXT_H
#define NETMANAGER_BASE_NAPI_REGISTER_NET_SUPPLIER_CONTEXT_H

#include <cstddef>
#include <napi/native_api.h>

#include "base_context.h"
#include "event_manager.h"
#include "net_all_capabilities.h"

namespace OHOS {
namespace NetManagerStandard {
class RegisterNetSupplierContext : public BaseContext {
public:
    RegisterNetSupplierContext() = delete;
    RegisterNetSupplierContext(napi_env env, EventManager *manager);

    void ParseParams(napi_value *params, size_t paramsCount);

public:
    NetBearType bearerType_ { BEARER_DEFAULT };
    std::string ident_;
    std::set<NetCap> netCaps_;
    uint32_t netSupplierId_ { 0 };
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_BASE_NAPI_REGISTER_NET_SUPPLIER_CONTEXT_H
