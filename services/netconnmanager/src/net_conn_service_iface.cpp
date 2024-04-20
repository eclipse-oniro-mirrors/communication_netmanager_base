/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "net_conn_service_iface.h"
#include "net_conn_service.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t NetConnServiceIface::GetIfaceNames(NetBearType bearerType, std::list<std::string> &ifaceNames)
{
    return NetConnService::GetInstance()->GetIfaceNames(bearerType, ifaceNames);
}

int32_t NetConnServiceIface::GetIfaceNameByType(NetBearType bearerType, const std::string &ident,
                                                std::string &ifaceName)
{
    return NetConnService::GetInstance()->GetIfaceNameByType(bearerType, ident, ifaceName);
}

int32_t NetConnServiceIface::RegisterNetSupplier(NetBearType bearerType, const std::string &ident,
                                                 const std::set<NetCap> &netCaps, uint32_t &supplierId)
{
    return NetConnService::GetInstance()->RegisterNetSupplier(bearerType, ident, netCaps, supplierId);
}

int32_t NetConnServiceIface::UnregisterNetSupplier(uint32_t supplierId)
{
    return NetConnService::GetInstance()->UnregisterNetSupplier(supplierId);
}

int32_t NetConnServiceIface::UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo)
{
    return NetConnService::GetInstance()->UpdateNetLinkInfo(supplierId, netLinkInfo);
}

int32_t NetConnServiceIface::UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo)
{
    return NetConnService::GetInstance()->UpdateNetSupplierInfo(supplierId, netSupplierInfo);
}

int32_t NetConnServiceIface::RestrictBackgroundChanged(bool isRestrictBackground)
{
    return NetConnService::GetInstance()->RestrictBackgroundChanged(isRestrictBackground);
}

int32_t NetConnServiceIface::RegisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    return NetConnService::GetInstance()->RegisterNetConnCallback(callback);
}

int32_t NetConnServiceIface::RegisterNetFactoryResetCallback(const sptr<INetFactoryResetCallback> &callback)
{
    return NetConnService::GetInstance()->RegisterNetFactoryResetCallback(callback);
}

bool NetConnServiceIface::IsAddrInOtherNetwork(const std::string &ifaceName, int32_t netId, const INetAddr &netAddr)
{
    return NetConnService::GetInstance()->IsAddrInOtherNetwork(ifaceName, netId, netAddr);
}
bool NetConnServiceIface::IsIfaceNameInUse(const std::string &ifaceName, int32_t netId)
{
    return NetConnService::GetInstance()->IsIfaceNameInUse(ifaceName, netId);
}
} // namespace NetManagerStandard
} // namespace OHOS