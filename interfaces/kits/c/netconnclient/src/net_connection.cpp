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

#include "net_connection.h"
#include "net_conn_client.h"
#include "net_connection_type.h"
#include "net_connection_adapter.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"

using namespace OHOS::NetManagerStandard;

constexpr int32_t VALID_NETID_START = 100;

static int32_t ErrorCodeTrans(int status)
{
    int32_t ret;
    switch (status) {
        case EAI_BADFLAGS:
            if (errno == EPERM || errno == EACCES) {
                ret = NETMANAGER_ERR_PERMISSION_DENIED;
            } else {
                ret = NETMANAGER_ERR_PARAMETER_ERROR;
            }
            break;
        case EAI_SERVICE:
            ret = NETMANAGER_ERR_OPERATION_FAILED;
            break;
        default:
            ret = NETMANAGER_ERR_INTERNAL;
            break;
    }
    return ret;
}

int32_t OH_NetConn_GetAddrInfo(char *host, char *serv, struct addrinfo *hint, struct addrinfo **res, int32_t netId)
{
    int32_t ret = NETMANAGER_SUCCESS;
    int status = 0;
    struct queryparam qp_param;
    if (host == nullptr || res == nullptr) {
        NETMGR_LOG_E("OH_NetConn_GetAddrInfo received invalid parameters");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    if (strlen(host) == 0) {
        NETMGR_LOG_E("OH_NetConn_GetAddrInfo received invalid host");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    if (netId > 0 && netId < VALID_NETID_START) {
        NETMGR_LOG_E("OH_NetConn_GetAddrInfo received invalid netId");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    if (memset_s(&qp_param, sizeof(struct queryparam), 0, sizeof(struct queryparam)) != EOK) {
        NETMGR_LOG_E("OH_NetConn_GetAddrInfo memset_s failed!");
        return NETMANAGER_ERR_MEMSET_FAIL;
    }
    qp_param.qp_netid = netId;
    qp_param.qp_type = 0;

    status = getaddrinfo_ext(host, serv, hint, res, &qp_param);
    if (status < 0) {
        NETMGR_LOG_E("OH_NetConn_GetAddrInfo fail status:%{public}d", status);
        ret = ErrorCodeTrans(status);
    }

    return ret;
}

int32_t OH_NetConn_FreeDnsResult(struct addrinfo *res)
{
    if (res == nullptr) {
        NETMGR_LOG_E("OH_NetConn_FreeDnsResult received invalid parameters");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    freeaddrinfo(res);

    return NETMANAGER_SUCCESS;
}

int32_t OH_NetConn_GetAllNets(NetConn_NetHandleList *netHandleList)
{
    if (netHandleList == nullptr) {
        NETMGR_LOG_E("OH_NetConn_GetAllNets received invalid parameters");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    std::list<OHOS::sptr<NetHandle>> netHandleObjList;
    int32_t ret = NetConnClient::GetInstance().GetAllNets(netHandleObjList);
    int32_t retConv = Conv2NetHandleList(netHandleObjList, netHandleList);
    if (retConv != NETMANAGER_SUCCESS) {
        return retConv;
    }
    return ret;
}

int32_t OH_NetConn_HasDefaultNet(int32_t *hasDefaultNet)
{
    if (hasDefaultNet == nullptr) {
        NETMGR_LOG_E("OH_NetConn_HasDefaultNet received invalid parameters");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    bool flagBool = false;
    int32_t ret = NetConnClient::GetInstance().HasDefaultNet(flagBool);
    *hasDefaultNet = flagBool;
    return ret;
}

int32_t OH_NetConn_GetDefaultNet(NetConn_NetHandle *netHandle)
{
    if (netHandle == nullptr) {
        NETMGR_LOG_E("OH_NetConn_GetDefaultNet received invalid parameters");
        return NETMANAGER_ERR_PARAMETER_ERROR;
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
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    bool flagBool = false;
    int32_t ret = NetConnClient::GetInstance().IsDefaultNetMetered(flagBool);
    *isMetered = flagBool;
    return ret;
}

int32_t OH_NetConn_GetConnectionProperties(NetConn_NetHandle *netHandle, NetConn_ConnectionProperties *prop)
{
    if (netHandle == nullptr || prop == nullptr) {
        NETMGR_LOG_E("OH_NetConn_GetConnectionProperties received invalid parameters");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    NetHandle netHandleObj = NetHandle();
    int32_t retConv = Conv2NetHandleObj(netHandle, netHandleObj);
    if (retConv != NETMANAGER_SUCCESS) {
        return retConv;
    }
    NetLinkInfo infoObj = NetLinkInfo();
    int32_t ret = NetConnClient::GetInstance().GetConnectionProperties(netHandleObj, infoObj);
    retConv = Conv2NetLinkInfo(infoObj, prop);
    if (retConv != NETMANAGER_SUCCESS) {
        return retConv;
    }
    return ret;
}

int32_t OH_NetConn_GetNetCapabilities(NetConn_NetHandle *netHandle,
                                      NetConn_NetCapabilities *netAllCapabilities)
{
    if (netHandle == nullptr || netAllCapabilities == nullptr) {
        NETMGR_LOG_E("OH_NetConn_GetNetCapabilities received invalid parameters");
        return NETMANAGER_ERR_PARAMETER_ERROR;
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

int32_t OH_NetConn_GetDefaultHttpProxy(NetConn_HttpProxy *httpProxy)
{
    if (httpProxy == nullptr) {
        NETMGR_LOG_E("OH_NetConn_GetDefaultHttpProxy received invalid parameters");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    HttpProxy httpProxyObj = HttpProxy();
    int32_t ret = NetConnClient::GetInstance().GetDefaultHttpProxy(httpProxyObj);
    int32_t retConv = Conv2HttpProxy(httpProxyObj, httpProxy);
    if (retConv != NETMANAGER_SUCCESS) {
        return retConv;
    }
    return ret;
}

int32_t OHOS_NetConn_RegisterDnsResolver(OH_NetConn_CustomDnsResolver resolver)
{
    if (resolver == nullptr) {
        NETMGR_LOG_E("OHOS_NetConn_RegisterDnsResolver received invalid parameters");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    int32_t ret = setdnsresolvehook(resolver);
    if (ret < 0) {
        ret = NETMANAGER_ERR_PARAMETER_ERROR;
    }
    return ret;
}

int32_t OHOS_NetConn_UnregisterDnsResolver()
{
    int32_t ret = removednsresolvehook();
    return ret;
}

int32_t OH_NetConn_BindSocket(int32_t socketFd, NetConn_NetHandle *netHandle)
{
    if (netHandle == nullptr) {
        NETMGR_LOG_E("OH_NetConn_BindSocket netHandle is NULL");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    if (socketFd < 0) {
        NETMGR_LOG_E("OH_NetConn_BindSocket socketFd is invalid");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    if (netHandle->netId < VALID_NETID_START) {
        NETMGR_LOG_E("OH_NetConn_BindSocket netId is invalid");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    int32_t ret = NetConnClient::GetInstance().BindSocket(socketFd, netHandle->netId);
    return ret;
}
