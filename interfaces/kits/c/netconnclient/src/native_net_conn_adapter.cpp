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
#include<map>

namespace OHOS::NetManagerStandard {

using BearTypeMap = std::map<NetBearType, OH_NetConn_NetBearType>;
using IpTypeMap = std::map<uint8_t, OH_NetConn_IpType>;
using NetCapMap = std::map<NetCap, OH_NetConn_NetCap>;
using RtnTypeMap = std::map<int32_t, OH_NetConn_RtnType>;

static BearTypeMap bearTypeMap = {
    {BEARER_CELLULAR, OH_NETCONN_BEARER_CELLULAR},
    {BEARER_WIFI, OH_NETCONN_BEARER_WIFI},
    {BEARER_BLUETOOTH, OH_NETCONN_BEARER_BLUETOOTH},
    {BEARER_ETHERNET, OH_NETCONN_BEARER_ETHERNET},
    {BEARER_VPN, OH_NETCONN_BEARER_VPN},
    {BEARER_WIFI_AWARE, OH_NETCONN_BEARER_WIFI_AWARE},
    {BEARER_DEFAULT, OH_NETCONN_BEARER_DEFAULT}
};

static IpTypeMap ipTypeMap = {
    {INetAddr::IPV4, OH_NETCONN_IPV4},
    {INetAddr::IPV6, OH_NETCONN_IPV6},
    {INetAddr::UNKNOWN, OH_NETCONN_UNKNOWN}
};

static NetCapMap netCapMap = {
    {NET_CAPABILITY_MMS, OH_NETCONN_NET_CAPABILITY_MMS},
    {NET_CAPABILITY_NOT_METERED, OH_NETCONN_NET_CAPABILITY_NOT_METERED},
    {NET_CAPABILITY_INTERNET, OH_NETCONN_NET_CAPABILITY_INTERNET},
    {NET_CAPABILITY_NOT_VPN, OH_NETCONN_NET_CAPABILITY_NOT_VPN},
    {NET_CAPABILITY_VALIDATED, OH_NETCONN_NET_CAPABILITY_VALIDATED},
    {NET_CAPABILITY_CAPTIVE_PORTAL, OH_NETCONN_NET_CAPABILITY_CAPTIVE_PORTAL},
    {NET_CAPABILITY_INTERNAL_DEFAULT, OH_NETCONN_NET_CAPABILITY_INTERNAL_DEFAULT}
};

static RtnTypeMap rtnTypeMap= {
    {RTN_UNICAST, OH_NETCONN_RTN_UNICAST},
    {RTN_UNREACHABLE, OH_NETCONN_RTN_UNREACHABLE},
    {RTN_THROW, OH_NETCONN_RTN_THROW}
};

static int32_t Conv2Ch(std::string s, char* ch)
{
    if (s.length() > MAX_STR_LEN - 1) {
        NETMGR_LOG_E("string out of memory");
        return NETMANAGER_ERR_INTERNAL;
    }
    strcpy(ch, s.c_str());
    return NETMANAGER_SUCCESS;
}

static int32_t Conv2INetAddr(INetAddr &netAddrObj, OH_NetConn_INetAddr *netAddr)
{
    netAddr->family = netAddrObj.family_;
    netAddr->prefixlen = netAddrObj.prefixlen_;
    netAddr->port = netAddrObj.port_;

    IpTypeMap::iterator iter = ipTypeMap.find(netAddrObj.type_);
    if(iter == ipTypeMap.end()){
        NETMGR_LOG_E("unknown ipTypeMap key");
        return NETMANAGER_ERR_INTERNAL;
    }
    netAddr->type = iter->second;

    int32_t ret;
    if ((ret = Conv2Ch(netAddrObj.address_, netAddr->address)) != NETMANAGER_SUCCESS) {
        return ret;
    }
    if ((ret = Conv2Ch(netAddrObj.hostName_, netAddr->hostName)) != NETMANAGER_SUCCESS) {
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
    if(iter == rtnTypeMap.end()){
        NETMGR_LOG_E("unknown rtnTypeMap key");
        return NETMANAGER_ERR_INTERNAL;
    }
    route->rtnType = iter->second;

    int32_t ret;
    if ((ret = Conv2Ch(routeObj.iface_, route->iface)) != NETMANAGER_SUCCESS) {
        return ret;
    }
    if ((ret = Conv2INetAddr(routeObj.destination_, &(route->destination))) != NETMANAGER_SUCCESS) {
        return ret;
    }
    if ((ret = Conv2INetAddr(routeObj.gateway_, &(route->gateway))) != NETMANAGER_SUCCESS) {
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
    std::list<sptr<NetHandle>>::iterator iter;
    int32_t i = 0;
    for (iter = netHandleObjList.begin(); iter != netHandleObjList.end(); ++iter) {
        if (i > MAX_NET_SIZE - 1) {
            NETMGR_LOG_E("netHandleList out of memory")
            return NETMANAGER_ERR_INTERNAL;
        }
        netHandleList->netHandleList[i++].netId = (**iter).GetNetId();
    }
    netHandleList->netHandleListSize = netHandleObjList.size();
    return NETMANAGER_SUCCESS;
}

int32_t Conv2HttpProxy(HttpProxy &httpProxyObj, OH_NetConn_HttpProxy *httpProxy)
{
    int32_t ret;
    if ((ret = Conv2Ch(httpProxyObj.GetHost(), httpProxy->host)) != NETMANAGER_SUCCESS) {
        return ret;
    }
    httpProxy->port = httpProxyObj.GetPort();

    std::list<std::string>::iterator iter;
    int32_t i = 0;
    for (iter = httpProxyObj.GetExclusionList().begin(); iter != httpProxyObj.GetExclusionList().end(); ++iter) {
        if (i > MAX_EXCLUSION_SIZE - 1) {
            NETMGR_LOG_E("exclusionList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        if ((ret = Conv2Ch(*iter, httpProxy->exclusionList[i++])) != NETMANAGER_SUCCESS) {
            return ret;
        }
    }
    httpProxy->exclusionListSize = httpProxyObj.GetExclusionList().size();

    return NETMANAGER_SUCCESS;
}

int32_t Conv2NetLinkInfo(NetLinkInfo &infoObj, OH_NetConn_NetLinkInfo *info)
{
    int32_t ret;
    if ((ret = Conv2Ch(infoObj.ifaceName_, info->ifaceName)) != NETMANAGER_SUCCESS) {
        return ret;
    }
    if ((ret = Conv2Ch(infoObj.domain_, info->domain)) != NETMANAGER_SUCCESS) {
        return ret;
    }
    if ((ret = Conv2Ch(infoObj.tcpBufferSizes_, info->tcpBufferSizes)) != NETMANAGER_SUCCESS) {
        return ret;
    }

    int32_t i = 0;
    for (std::list<INetAddr>::iterator iter = infoObj.netAddrList_.begin(); iter != infoObj.netAddrList_.end();
         ++iter) {
        if (i > MAX_ADDR_SIZE - 1) {
            NETMGR_LOG_E("netAddrList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        if ((ret = Conv2INetAddr(*iter, &(info->netAddrList[i++]))) != NETMANAGER_SUCCESS) {
            return ret;
        }
    }
    info->netAddrListSize = infoObj.netAddrList_.size();

    i = 0;
    for (std::list<INetAddr>::iterator iter = infoObj.dnsList_.begin(); iter != infoObj.dnsList_.end(); ++iter) {
        if (i > MAX_ADDR_SIZE - 1) {
            NETMGR_LOG_E("dnsList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        if ((ret = Conv2INetAddr(*iter, &(info->dnsList[i++]))) != NETMANAGER_SUCCESS) {
            return ret;
        }
    }
    info->dnsListSize = infoObj.dnsList_.size();

    i = 0;
    for (std::list<Route>::iterator iter = infoObj.routeList_.begin(); iter != infoObj.routeList_.end(); ++iter) {
        if (i > MAX_ROUTE_SIZE - 1) {
            NETMGR_LOG_E("routeList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        if ((ret = Conv2Route(*iter, &(info->routeList[i++])) != NETMANAGER_SUCCESS)) {
            return ret;
        }
    }
    info->routeListSize = infoObj.routeList_.size();

    if ((ret = Conv2HttpProxy(infoObj.httpProxy_, &(info->httpProxy))) != NETMANAGER_SUCCESS) {
        return ret;
    }

    return NETMANAGER_SUCCESS;
}

int32_t Conv2NetAllCapabilities(NetAllCapabilities &netAllCapsObj, OH_NetConn_NetAllCapabilities *netAllCaps)
{
    netAllCaps->linkUpBandwidthKbps = netAllCapsObj.linkUpBandwidthKbps_;
    netAllCaps->linkDownBandwidthKbps = netAllCapsObj.linkDownBandwidthKbps_;

    int32_t i = 0;
    for (std::set<NetCap>::iterator iter = netAllCapsObj.netCaps_.begin(); iter != netAllCapsObj.netCaps_.end();
         ++iter) {
        if (i > MAX_CAP_SIZE - 1) {
            NETMGR_LOG_E("netCapsList out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }

        NetCapMap::iterator iterMap = netCapMap.find(*iter);
        if(iterMap == netCapMap.end()){
            NETMGR_LOG_E("unknown netCapMap key");
            return NETMANAGER_ERR_INTERNAL;
        }
        netAllCaps->netCaps[i++] = iterMap->second;
    }
    netAllCaps->netCapsSize = netAllCapsObj.netCaps_.size();

    i = 0;
    for (std::set<NetBearType>::iterator iter = netAllCapsObj.bearerTypes_.begin();
         iter != netAllCapsObj.bearerTypes_.end(); ++iter) {
        if (i > MAX_BEAR_TYPE_SIZE - 1) {
            NETMGR_LOG_E("bearerTypes out of memory");
            return NETMANAGER_ERR_INTERNAL;
        }
        
        BearTypeMap::iterator iterMap = bearTypeMap.find(*iter);
        if(iterMap == bearTypeMap.end()){
            NETMGR_LOG_E("unknown bearTypeMap key");
            return NETMANAGER_ERR_INTERNAL;
        }
        netAllCaps->bearerTypes[i++] = iterMap->second;
    }
    netAllCaps->bearerTypesSize = netAllCapsObj.bearerTypes_.size();

    return NETMANAGER_SUCCESS;
}

} // namespace OHOS::NetManagerStandard
