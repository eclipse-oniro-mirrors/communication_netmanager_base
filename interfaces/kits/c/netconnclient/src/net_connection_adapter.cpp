/*
* Copyright (c) 2023 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
 */

#include <map>

#include "net_connection_adapter.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "securec.h"

namespace OHOS::NetManagerStandard {
int32_t Conv2NetHandleList(const std::list<sptr<NetHandle>> &netHandleObjList, OH_NetConn_NetHandleList *netHandleList)
{
    int32_t i = 0;
    for (const auto& netHandleObj : netHandleObjList) {
        if (i > OH_NETCONN_MAX_NET_SIZE - 1) {
            NETMGR_LOG_E("netHandleList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        netHandleList->netHandles[i++].netId = (*netHandleObj).GetNetId();
    }
    netHandleList->netHandleListSize = netHandleObjList.size();
    return NETMANAGER_SUCCESS;
}
} // namespace OHOS::NetManagerStandard