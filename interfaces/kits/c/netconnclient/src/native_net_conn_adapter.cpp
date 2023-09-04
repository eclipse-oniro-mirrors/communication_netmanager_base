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

#include "native_net_conn_adapter.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "securec.h"
#include <map>

namespace OHOS::NetManagerStandard {

using BearTypeMap = std::map<NetBearType, OH_NetConn_NetBearType>;
using IpTypeMap = std::map<uint8_t, OH_NetConn_IpType>;
using NetCapMap = std::map<NetCap, OH_NetConn_NetCap>;
using RtnTypeMap = std::map<int32_t, OH_NetConn_RtnType>;

static BearTypeMap bearTypeMap = {{BEARER_CELLULAR, OH_NETCONN_BEARER_CELLULAR},
                                  {BEARER_WIFI, OH_NETCONN_BEARER_WIFI},
                                  {BEARER_BLUETOOTH, OH_NETCONN_BEARER_BLUETOOTH},
                                  {BEARER_ETHERNET, OH_NETCONN_BEARER_ETHERNET},
                                  {BEARER_VPN, OH_NETCONN_BEARER_VPN},
                                  {BEARER_WIFI_AWARE, OH_NETCONN_BEARER_WIFI_AWARE},
                                  {BEARER_DEFAULT, OH_NETCONN_BEARER_DEFAULT}};

static IpTypeMap ipTypeMap = {{INetAddr::IPV4, OH_NETCONN_IPV4},
                              {INetAddr::IPV6, OH_NETCONN_IPV6},
                              {INetAddr::UNKNOWN, OH_NETCONN_UNKNOWN}};

static NetCapMap netCapMap = {{NET_CAPABILITY_MMS, OH_NETCONN_NET_CAPABILITY_MMS},
                              {NET_CAPABILITY_NOT_METERED, OH_NETCONN_NET_CAPABILITY_NOT_METERED},
                              {NET_CAPABILITY_INTERNET, OH_NETCONN_NET_CAPABILITY_INTERNET},
                              {NET_CAPABILITY_NOT_VPN, OH_NETCONN_NET_CAPABILITY_NOT_VPN},
                              {NET_CAPABILITY_VALIDATED, OH_NETCONN_NET_CAPABILITY_VALIDATED},
                              {NET_CAPABILITY_CAPTIVE_PORTAL, OH_NETCONN_NET_CAPABILITY_CAPTIVE_PORTAL},
                              {NET_CAPABILITY_INTERNAL_DEFAULT, OH_NETCONN_NET_CAPABILITY_INTERNAL_DEFAULT}};

static RtnTypeMap rtnTypeMap = {{RTN_UNICAST, OH_NETCONN_RTN_UNICAST},
                                {RTN_UNREACHABLE, OH_NETCONN_RTN_UNREACHABLE},
                                {RTN_THROW, OH_NETCONN_RTN_THROW}};

static int32_t Conv2Ch(std::string s, char *ch)
{
    if (s.length() > OH_NETCONN_MAX_STR_LEN - 1) {
        NETMGR_LOG_E("string out of memory");
        return NETMANAGER_ERR_INTERNAL;
    }
    if (strcpy_s(ch, s.length() + 1, s.c_str()) != 0) {
        NETMGR_LOG_E("string copy failed");
        return NETMANAGER_ERR_INTERNAL;
    }
    return NETMANAGER_SUCCESS;
}

static int32_t Conv2INetAddr(INetAddr &netAddrObj, OH_NetConn_INetAddr *netAddr)
{
    netAddr->family = netAddrObj.family_;
    netAddr->prefixlen = netAddrObj.prefixlen_;
    netAddr->port = netAddrObj.port_;

    IpTypeMap::iterator iter = ipTypeMap.find(netAddrObj.type_);
    if (iter == ipTypeMap.end()) {
        NETMGR_LOG_E("unknown ipTypeMap key");
        return NETMANAGER_ERR_INTERNAL;
    }
    netAddr->type = iter->second;

    int32_t ret;
    ret = Conv2Ch(netAddrObj.address_, netAddr->address);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    ret = Conv2Ch(netAddrObj.hostName_, netAddr->hostName);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    return NETMANAGER_SUCCESS;
}

static int32_t Conv2Route(Route &routeObj, OH_NetConn_Route *route)
{
    route->mtu = routeObj.mtu_;
    route->isHost = routeObj.isHost_;
    route->hasGateway = routeObj.hasGateway_;
    route->isDefaultRoute = routeObj.isDefaultRoute_;

    RtnTypeMap::iterator iter = rtnTypeMap.find(routeObj.rtnType_);
    if (iter == rtnTypeMap.end()) {
        NETMGR_LOG_E("unknown rtnTypeMap key");
        return NETMANAGER_ERR_INTERNAL;
    }
    route->rtnType = iter->second;

    int32_t ret;
    ret = Conv2Ch(routeObj.iface_, route->iface);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    ret = Conv2INetAddr(routeObj.destination_, &(route->destination));
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    ret = Conv2INetAddr(routeObj.gateway_, &(route->gateway));
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    return NETMANAGER_SUCCESS;
}

int32_t Conv2NetHandle(NetHandle &netHandleObj, OH_NetConn_NetHandle *netHandle)
{
    netHandle->netId = netHandleObj.GetNetId();
    return NETMANAGER_SUCCESS;
}

int32_t Conv2NetHandleObj(OH_NetConn_NetHandle *netHandle, NetHandle &netHandleObj)
{
    netHandleObj.SetNetId(netHandle->netId);
    return NETMANAGER_SUCCESS;
}

int32_t Conv2NetHandleList(std::list<sptr<NetHandle>> &netHandleObjList, OH_NetConn_NetHandleList *netHandleList)
{
    int32_t i = 0;
    for (auto netHandleObj : netHandleObjList) {
        if (i > OH_NETCONN_MAX_NET_SIZE - 1) {
            NETMGR_LOG_E("netHandleList out of memory")
            return NETMANAGER_ERR_INTERNAL;
        }
        netHandleList->netHandleList[i++].netId = (*netHandleObj).GetNetId();
    }
    netHandleList->netHandleListSize = netHandleObjList.size();
    return NETMANAGER_SUCCESS;
}

int32_t Conv2HttpProxy(HttpProxy &httpProxyObj, OH_NetConn_HttpProxy *httpProxy)
{
    int32_t ret;
    ret = Conv2Ch(httpProxyObj.GetHost(), httpProxy->host);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    httpProxy->port = httpProxyObj.GetPort();

    int32_t i = 0;
    for (auto exclusion : httpProxyObj.GetExclusionList())
        if (i > OH_NETCONN_MAX_EXCLUSION_SIZE - 1) {
            NETMGR_LOG_E("exclusionList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        ret = Conv2Ch(exclusion, httpProxy->exclusionList[i++]));
        if (ret != NETMANAGER_SUCCESS) {
            return ret;
        }
}
httpProxy->exclusionListSize = httpProxyObj.GetExclusionList().size();

return NETMANAGER_SUCCESS;
}

int32_t Conv2NetLinkInfo(NetLinkInfo &infoObj, OH_NetConn_NetLinkInfo *info)
{
    int32_t ret;
    ret = Conv2Ch(infoObj.ifaceName_, info->ifaceName);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    ret = Conv2Ch(infoObj.domain_, info->domain);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    ret = Conv2Ch(infoObj.tcpBufferSizes_, info->tcpBufferSizes);
    if ((ret != NETMANAGER_SUCCESS) {
        return ret;
    }

    int32_t i = 0;
    for (auto netAddr : infoObj.netAddrList_) {
        if (i > OH_NETCONN_MAX_ADDR_SIZE - 1) {
            NETMGR_LOG_E("netAddrList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        ret = Conv2INetAddr(netAddr, &(info->netAddrList[i++]));
        if (ret != NETMANAGER_SUCCESS) {
            return ret;
        }
    }
    info->netAddrListSize = infoObj.netAddrList_.size();

    i = 0;
    for (auto dns : infoObj.dnsList_) {
        if (i > OH_NETCONN_MAX_ADDR_SIZE - 1) {
            NETMGR_LOG_E("dnsList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        ret = Conv2INetAddr(dns, &(info->dnsList[i++]));
        if (ret != NETMANAGER_SUCCESS) {
            return ret;
        }
    }
    info->dnsListSize = infoObj.dnsList_.size();

    i = 0;
    for (auto route : infoObj.routeList_) {
        if (i > OH_NETCONN_MAX_ROUTE_SIZE - 1) {
            NETMGR_LOG_E("routeList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        ret = Conv2Route(route, &(info->routeList[i++]); if (ret != NETMANAGER_SUCCESS))
        {
            return ret;
        }
    }
    info->routeListSize = infoObj.routeList_.size();
    ret = Conv2HttpProxy(infoObj.httpProxy_, &(info->httpProxy))
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }

    return NETMANAGER_SUCCESS;
}

int32_t Conv2NetAllCapabilities(NetAllCapabilities &netAllCapsObj, OH_NetConn_NetAllCapabilities *netAllCaps)
{
    netAllCaps->linkUpBandwidthKbps = netAllCapsObj.linkUpBandwidthKbps_;
    netAllCaps->linkDownBandwidthKbps = netAllCapsObj.linkDownBandwidthKbps_;

    int32_t i = 0;
    for (auto netCap : netAllCapsObj.netCaps_) {
        if (i > OH_NETCONN_MAX_CAP_SIZE - 1) {
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
    for (auto bearType : netAllCapsObj.bearerTypes) {
        if (i > OH_NETCONN_MAX_BEAR_TYPE_SIZE - 1) {
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
    netAllCaps->bearerTypesSize = netAllCapsObj.bearerTypes_.size();

    return NETMANAGER_SUCCESS;
}

} // namespace OHOS::NetManagerStandard
