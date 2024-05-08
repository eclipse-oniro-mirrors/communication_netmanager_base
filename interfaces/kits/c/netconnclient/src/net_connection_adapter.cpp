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

using BearTypeMap = std::map<NetBearType, NetConn_NetBearerType>;
using NetCapMap = std::map<NetCap, NetConn_NetCap>;

static BearTypeMap bearTypeMap = {{BEARER_CELLULAR, NETCONN_BEARER_CELLULAR},
                                  {BEARER_WIFI, NETCONN_BEARER_WIFI},
                                  {BEARER_ETHERNET, NETCONN_BEARER_ETHERNET}};

static NetCapMap netCapMap = {{NET_CAPABILITY_MMS,         NETCONN_NET_CAPABILITY_MMS},
                              {NET_CAPABILITY_SUPL,         NETCONN_NET_CAPABILITY_SUPL},
                              {NET_CAPABILITY_DUN,         NETCONN_NET_CAPABILITY_DUN},
                              {NET_CAPABILITY_IA,         NETCONN_NET_CAPABILITY_IA},
                              {NET_CAPABILITY_XCAP,         NETCONN_NET_CAPABILITY_XCAP},
                              {NET_CAPABILITY_NOT_METERED, NETCONN_NET_CAPABILITY_NOT_METERED},
                              {NET_CAPABILITY_INTERNET,    NETCONN_NET_CAPABILITY_INTERNET},
                              {NET_CAPABILITY_NOT_VPN,     NETCONN_NET_CAPABILITY_NOT_VPN},
                              {NET_CAPABILITY_VALIDATED,   NETCONN_NET_CAPABILITY_VALIDATED},
                              {NET_CAPABILITY_PORTAL,   NETCONN_NET_CAPABILITY_PORTAL}};

static int32_t Conv2Ch(const std::string s, char *ch)
{
    if (s.length() > NETCONN_MAX_STR_LEN - 1) {
        NETMGR_LOG_E("string out of memory");
        return NETMANAGER_ERR_INTERNAL;
    }
    if (strcpy_s(ch, s.length() + 1, s.c_str()) != 0) {
        NETMGR_LOG_E("string copy failed");
        return NETMANAGER_ERR_INTERNAL;
    }
    return NETMANAGER_SUCCESS;
}

static int32_t Conv2INetAddr(const INetAddr &netAddrObj, NetConn_NetAddr *netAddr)
{
    netAddr->family = netAddrObj.family_;
    netAddr->prefixlen = netAddrObj.prefixlen_;
    netAddr->port = netAddrObj.port_;

    int32_t ret = Conv2Ch(netAddrObj.address_, netAddr->address);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    return NETMANAGER_SUCCESS;
}

int32_t Conv2NetHandleList(const std::list<sptr<NetHandle>> &netHandleObjList, NetConn_NetHandleList *netHandleList)
{
    int32_t i = 0;
    for (const auto& netHandleObj : netHandleObjList) {
        if (i > NETCONN_MAX_NET_SIZE - 1) {
            NETMGR_LOG_E("netHandleList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        netHandleList->netHandles[i++].netId = (*netHandleObj).GetNetId();
    }
    netHandleList->netHandleListSize = netHandleObjList.size();
    return NETMANAGER_SUCCESS;
}

int32_t Conv2NetHandle(NetHandle &netHandleObj, NetConn_NetHandle *netHandle)
{
    netHandle->netId = netHandleObj.GetNetId();
    return NETMANAGER_SUCCESS;
}

int32_t Conv2NetHandleObj(NetConn_NetHandle *netHandle, NetHandle &netHandleObj)
{
    netHandleObj.SetNetId(netHandle->netId);
    return NETMANAGER_SUCCESS;
}

int32_t Conv2HttpProxy(HttpProxy &httpProxyObj, NetConn_HttpProxy *httpProxy)
{
    int32_t ret = Conv2Ch(httpProxyObj.GetHost(), httpProxy->host);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    httpProxy->port = httpProxyObj.GetPort();

    int32_t i = 0;
    for (const auto& exclusion : httpProxyObj.GetExclusionList()) {
        if (i > NETCONN_MAX_EXCLUSION_SIZE - 1) {
            NETMGR_LOG_E("exclusionList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        ret = Conv2Ch(exclusion, httpProxy->exclusionList[i++]);
        if (ret != NETMANAGER_SUCCESS) {
            return ret;
        }
    }

    httpProxy->exclusionListSize = static_cast<int32_t>(httpProxyObj.GetExclusionList().size());

    return NETMANAGER_SUCCESS;
}


int32_t Conv2NetLinkInfo(NetLinkInfo &infoObj, NetConn_ConnectionProperties *prop)
{
    int32_t ret = Conv2Ch(infoObj.ifaceName_, prop->ifaceName);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    ret = Conv2Ch(infoObj.domain_, prop->domain);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    ret = Conv2Ch(infoObj.tcpBufferSizes_, prop->tcpBufferSizes);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }

    int32_t i = 0;
    for (const auto& netAddr : infoObj.netAddrList_) {
        if (i > NETCONN_MAX_ADDR_SIZE - 1) {
            NETMGR_LOG_E("netAddrList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        ret = Conv2INetAddr(netAddr, &(prop->netAddrList[i++]));
        if (ret != NETMANAGER_SUCCESS) {
            return ret;
        }
    }
    prop->netAddrListSize = static_cast<int32_t>(infoObj.netAddrList_.size());

    i = 0;
    for (const auto& dns : infoObj.dnsList_) {
        if (i > NETCONN_MAX_ADDR_SIZE - 1) {
            NETMGR_LOG_E("dnsList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        ret = Conv2INetAddr(dns, &(prop->dnsList[i++]));
        if (ret != NETMANAGER_SUCCESS) {
            return ret;
        }
    }
    prop->dnsListSize = static_cast<int32_t>(infoObj.dnsList_.size());

    ret = Conv2HttpProxy(infoObj.httpProxy_, &(prop->httpProxy));
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }

    return NETMANAGER_SUCCESS;
}

int32_t Conv2NetAllCapabilities(NetAllCapabilities &netAllCapsObj, NetConn_NetCapabilities *netAllCaps)
{
    netAllCaps->linkUpBandwidthKbps = netAllCapsObj.linkUpBandwidthKbps_;
    netAllCaps->linkDownBandwidthKbps = netAllCapsObj.linkDownBandwidthKbps_;

    int32_t i = 0;
    for (const auto& netCap : netAllCapsObj.netCaps_) {
        if (i > NETCONN_MAX_CAP_SIZE - 1) {
            NETMGR_LOG_E("netCapsList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }

        NetCapMap::iterator iterMap = netCapMap.find(netCap);
        if (iterMap == netCapMap.end()) {
            NETMGR_LOG_E("unknown netCapMap key");
            return NETMANAGER_ERR_INTERNAL;
        }
        netAllCaps->netCaps[i++] = iterMap->second;
    }
    netAllCaps->netCapsSize = netAllCapsObj.netCaps_.size();

    i = 0;
    for (const auto& bearType : netAllCapsObj.bearerTypes_) {
        if (i > NETCONN_MAX_BEARER_TYPE_SIZE - 1) {
            NETMGR_LOG_E("bearerTypes out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }

        BearTypeMap::iterator iterMap = bearTypeMap.find(bearType);
        if (iterMap == bearTypeMap.end()) {
            NETMGR_LOG_E("unknown bearTypeMap key");
            return NETMANAGER_ERR_INTERNAL;
        }
        netAllCaps->bearerTypes[i++] = iterMap->second;
    }
    netAllCaps->bearerTypesSize = static_cast<int32_t>(netAllCapsObj.bearerTypes_.size());

    return NETMANAGER_SUCCESS;
}

} // namespace OHOS::NetManagerStandard