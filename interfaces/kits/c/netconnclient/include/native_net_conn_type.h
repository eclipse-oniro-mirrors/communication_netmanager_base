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

#ifndef NATIVE_NET_CONN_TYPE_H
#define NATIVE_NET_CONN_TYPE_H

/**
 * @addtogroup NetConn
 * @{
 *
 * @brief 为网络管理的数据网络连接模块的C接口提供数据结构
 *
 * @since 11
 * @version 1.0
 */

/**
 * @file native_net_conn_type.h
 * @brief 定义网络连接模块的C接口需要的数据结构
 *
 * @syscap SystemCapability.Communication.NetManager.Core
 * @since 11
 * @version 1.0
 *
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OH_NETCONN_MAX_NET_SIZE 32
#define OH_NETCONN_MAX_BEAR_TYPE_SIZE 32
#define OH_NETCONN_MAX_CAP_SIZE 32
#define OH_NETCONN_MAX_ADDR_SIZE 32
#define OH_NETCONN_MAX_ROUTE_SIZE 64
#define OH_NETCONN_MAX_EXCLUSION_SIZE 256
#define OH_NETCONN_MAX_STR_LEN 256

/**
 * @brief 网络能力集
 *
 * @since 11
 * @version 1.0
 */

typedef enum OH_NetConn_NetCap {
    /* MMS */
    OH_NETCONN_NET_CAPABILITY_MMS = 0,
    /* Not Metered */
    OH_NETCONN_NET_CAPABILITY_NOT_METERED = 11,
    /* Internet */
    OH_NETCONN_NET_CAPABILITY_INTERNET = 12,
    /* Not VPN */
    OH_NETCONN_NET_CAPABILITY_NOT_VPN = 15,
    /* Validated */
    OH_NETCONN_NET_CAPABILITY_VALIDATED = 16,
    /* Captive portal */
    OH_NETCONN_NET_CAPABILITY_CAPTIVE_PORTAL = 17,
    /* Internal Default */
    OH_NETCONN_NET_CAPABILITY_INTERNAL_DEFAULT
} OH_NetConn_NetCap;

/**
 * @brief 网络载体类型
 *
 * @since 11
 * @version 1.0
 */

typedef enum OH_NetConn_NetBearType {
    /* Cellular */
    OH_NETCONN_BEARER_CELLULAR = 0,
    /* WIFI */
    OH_NETCONN_BEARER_WIFI = 1,
    /* Bluetooth */
    OH_NETCONN_BEARER_BLUETOOTH = 2,
    /* Ethernet */
    OH_NETCONN_BEARER_ETHERNET = 3,
    /* VPN */
    OH_NETCONN_BEARER_VPN = 4,
    /* WIFI aware */
    OH_NETCONN_BEARER_WIFI_AWARE = 5,
    /* Default */
    OH_NETCONN_BEARER_DEFAULT
} OH_NetConn_NetBearType;

/**
 * @brief 路由返回类型
 *
 * @since 11
 * @version 1.0
 */

typedef enum OH_NetConn_RtnType {
    /* Unicast */
    OH_NETCONN_RTN_UNICAST = 1,
    /* Unreachable */
    OH_NETCONN_RTN_UNREACHABLE = 7,
    /* Throw */
    OH_NETCONN_RTN_THROW = 9
} OH_NetConn_RtnType;

/**
 * @brief IP类型.
 *
 * @since 11
 * @version 1.0
 */

typedef enum {
    /* Unknown */
    OH_NETCONN_UNKNOWN = 0x00,
    /* IPV4 */
    OH_NETCONN_IPV4 = 0x01,
    /* IPV6 */
    OH_NETCONN_IPV6 = 0x02,
} OH_NetConn_IpType;

/**
 * @brief 存放网络ID.
 *
 * @since 11
 * @version 1.0
 */

typedef struct OH_NetConn_NetHandle {
    /* Network id */
    int32_t netId;
} OH_NetConn_NetHandle;

/**
 * @brief 网络列表
 *
 * @since 11
 * @version 1.0
 */

typedef struct OH_NetConn_NetHandleList {
    /* List of netHandle */
    OH_NetConn_NetHandle netHandleList[OH_NETCONN_MAX_NET_SIZE];
    /* Actual size of netHandleList */
    int32_t netHandleListSize;
} OH_NetConn_NetHandleList;

/**
 * @brief 网络能力集
 *
 * @since 11
 * @version 1.0
 */

typedef struct OH_NetConn_NetAllCapabilities {
    /* LinkUpBandwidthKbps */
    uint32_t linkUpBandwidthKbps;
    /* LinkDownBandwidthKbps */
    uint32_t linkDownBandwidthKbps;
    /* List of capabilities */
    OH_NetConn_NetCap netCaps[OH_NETCONN_MAX_CAP_SIZE];
    /* Actual size of netCaps */
    int32_t netCapsSize;
    /* List of bearer types */
    OH_NetConn_NetBearType bearerTypes[OH_NETCONN_MAX_BEAR_TYPE_SIZE];
    /* Actual size of bearerTypes */
    int32_t bearerTypesSize;
} OH_NetConn_NetAllCapabilities;

/**
 * @brief 网络地址
 *
 * @since 11
 * @version 1.0
 */

typedef struct OH_NetConn_INetAddr {
    /* Ip type */
    OH_NetConn_IpType type;
    /* Family */
    uint8_t family;
    /* Length of prefix */
    uint8_t prefixlen;
    /* Port */
    uint8_t port;
    /* Address */
    char address[OH_NETCONN_MAX_STR_LEN];
    /* Host name */
    char hostName[OH_NETCONN_MAX_STR_LEN];
} OH_NetConn_INetAddr;

/**
 * @brief 路由配置信息
 *
 * @since 11
 * @version 1.0
 */

typedef struct OH_NetConn_Route {
    /* Interface */
    char iface[OH_NETCONN_MAX_STR_LEN];
    /* Destination address */
    OH_NetConn_INetAddr destination;
    /* Gateway address */
    OH_NetConn_INetAddr gateway;
    /* Return type */
    OH_NetConn_RtnType rtnType;
    /* MTU */
    int32_t mtu;
    /* Whether it is host */
    int32_t isHost;
    /* Whether a gateway is present */
    int32_t hasGateway;
    /* Whether is default route */
    int32_t isDefaultRoute;
} OH_NetConn_Route;

/**
 * @brief 代理配置信息
 *
 * @since 11
 * @version 1.0
 */

typedef struct OH_NetConn_HttpProxy {
    /* Host name */
    char host[OH_NETCONN_MAX_STR_LEN];
    /* List of exclusion */
    char exclusionList[OH_NETCONN_MAX_EXCLUSION_SIZE][OH_NETCONN_MAX_STR_LEN];
    /* Actual size of exclusionList */
    int32_t exclusionListSize;
    /* Port */
    uint16_t port;
} OH_NetConn_HttpProxy;

/**
 * @brief Network link information
 *
 * @since 11
 * @version 1.0
 */

typedef struct OH_NetConn_NetLinkInfo {
    /* Interface name */
    char ifaceName[OH_NETCONN_MAX_STR_LEN];
    /* Domain */
    char domain[OH_NETCONN_MAX_STR_LEN];
    /* TCP butter size */
    char tcpBufferSizes[OH_NETCONN_MAX_STR_LEN];
    /* MTU */
    uint16_t mtu;
    /* Address list */
    OH_NetConn_INetAddr netAddrList[OH_NETCONN_MAX_ADDR_SIZE];
    /* Actual size of netAddrList */
    int32_t netAddrListSize;
    /* DNS list */
    OH_NetConn_INetAddr dnsList[OH_NETCONN_MAX_ADDR_SIZE];
    /* Actual size od dnsList */
    int32_t dnsListSize;
    /* Route list */
    OH_NetConn_Route routeList[OH_NETCONN_MAX_ROUTE_SIZE];
    /* Actual size of routeList */
    int32_t routeListSize;
    /* Http proxy */
    OH_NetConn_HttpProxy httpProxy;
} OH_NetConn_NetLinkInfo;

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* NATIVE_NET_CONN_TYPE_H */
