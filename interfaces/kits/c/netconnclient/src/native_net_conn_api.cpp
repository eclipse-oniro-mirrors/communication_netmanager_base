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

#include "native_net_conn_api.h"
#include "native_net_conn_adapter.h"
#include "net_conn_client.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"

using namespace OHOS::NetManagerStandard;

int32_t OH_NetConn_HasDefaultNet(int32_t *hasDefaultNet)
{
    if (hasDefaultNet == nullptr) {
        NETMGR_LOG_E("OH_NetConn_HasDefaultNet received invalid parameters");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }

    bool flagBool = false;
    int32_t ret = NetConnClient::GetInstance().HasDefaultNet(flagBool);
    *hasDefaultNet = flagBool;
    return ret;
}

int32_t OH_NetConn_GetDefaultNet(OH_NetConn_NetHandle *netHandle)
{
    if (netHandle == nullptr) {
        NETMGR_LOG_E("OH_NetConn_GetDefaultNet received invalid parameters");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }

    NetHandle netHandleObj = NetHandle();
    int32_t ret = NetConnClient::GetInstance().GetDefaultNet(netHandleObj);
    int32_t retConv = Conv2NetHandle(netHandleObj, netHandle);
    if (retConv != NETMANAGER_SUCCESS) {
        return retConv;
    }
    return ret;
}

int32_t OH_NetConn_IsDefaultNetMetered(int32_t *isMetered)
{
    if (isMetered == nullptr) {
        NETMGR_LOG_E("OH_NetConn_IsDefaultNetMetered received invalid parameters");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }

    bool flagBool = false;
    int32_t ret = NetConnClient::GetInstance().IsDefaultNetMetered(flagBool);
    *isMetered = flagBool;
    return ret;
}

int32_t OH_NetConn_GetAllNets(OH_NetConn_NetHandleList *netHandleList)
{
    if (netHandleList == nullptr) {
        NETMGR_LOG_E("OH_NetConn_GetAllNets received invalid parameters");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }

    std::list<OHOS::sptr<NetHandle>> netHandleObjList;
    int32_t ret = NetConnClient::GetInstance().GetAllNets(netHandleObjList);
    int32_t retConv = Conv2NetHandleList(netHandleObjList, netHandleList);
    if (retConv != NETMANAGER_SUCCESS) {
        return retConv;
    }
    return ret;
}

int32_t OH_NetConn_GetConnectionProperties(OH_NetConn_NetHandle *netHandle, OH_NetConn_NetLinkInfo *info)
{
    if (netHandle == nullptr || info == nullptr) {
        NETMGR_LOG_E("OH_NetConn_GetConnectionProperties received invalid parameters");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }

    NetHandle netHandleObj = NetHandle();
    int32_t retConv = Conv2NetHandleObj(netHandle, netHandleObj);
    if (retConv != NETMANAGER_SUCCESS) {
        return retConv;
    }
    NetLinkInfo infoObj = NetLinkInfo();
    int32_t ret = NetConnClient::GetInstance().GetConnectionProperties(netHandleObj, infoObj);
    retConv = Conv2NetLinkInfo(infoObj, info);
    if (retConv != NETMANAGER_SUCCESS) {
        return retConv;
    }
    return ret;
}

int32_t OH_NetConn_GetNetCapabilities(OH_NetConn_NetHandle *netHandle,
                                      OH_NetConn_NetAllCapabilities *netAllCapabilities)
{
    if (netHandle == nullptr || netAllCapabilities == nullptr) {
        NETMGR_LOG_E("OH_NetConn_GetNetCapabilities received invalid parameters");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }

    NetHandle netHandleObj = NetHandle();
    int32_t retConv = Conv2NetHandleObj(netHandle, netHandleObj);
    if (retConv != NETMANAGER_SUCCESS) {
        return retConv;
    }
    NetAllCapabilities netAllCapsObj = NetAllCapabilities();
    int32_t ret = NetConnClient::GetInstance().GetNetCapabilities(netHandleObj, netAllCapsObj);
    retConv = Conv2NetAllCapabilities(netAllCapsObj, netAllCapabilities);
    if (retConv != NETMANAGER_SUCCESS) {
        return retConv;
    }
    return ret;
}

int32_t OH_NetConn_GetDefaultHttpProxy(OH_NetConn_HttpProxy *httpProxy)
{
    if (httpProxy == nullptr) {
        NETMGR_LOG_E("OH_NetConn_GetDefaultHttpProxy received invalid parameters");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }

    HttpProxy httpProxyObj = HttpProxy();
    int32_t ret = NetConnClient::GetInstance().GetDefaultHttpProxy(httpProxyObj);
    int32_t retConv = Conv2HttpProxy(httpProxyObj, httpProxy);
    if (retConv != NETMANAGER_SUCCESS) {
        return retConv;
    }
    return ret;
}