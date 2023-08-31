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
 * @brief 声明了网络连接管理所需要的各种宏、枚举、数据结构等
 *
 * @since 10
 * @version 1.0
 */

/**
 * @file native_net_conn_type.h
 * @brief 提供NetConn中需要的宏、枚举、数据结构等
 * @since 10
 * @version 1.0
 *
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NET_SIZE 32
#define MAX_BEAR_TYPE_SIZE 32
#define MAX_CAP_SIZE 32
#define MAX_ADDR_SIZE 32
#define MAX_ROUTE_SIZE 64
#define MAX_EXCLUSION_SIZE 256
#define MAX_STR_LEN 256

/**
 * @brief 网络能力类型
 *
 * @since 10
 * @version 1.0
 */

typedef enum OH_NetConn_NetCap {
    /* MMS */
    OH_NETCONN_NET_CAPABILITY_MMS = 0,
    /* 不计流量 */
    OH_NETCONN_NET_CAPABILITY_NOT_METERED = 11,
    /* INTERNET */
    OH_NETCONN_NET_CAPABILITY_INTERNET = 12,
    /* NOT VPN */
    OH_NETCONN_NET_CAPABILITY_NOT_VPN = 15,
    /* 校验 */
    OH_NETCONN_NET_CAPABILITY_VALIDATED = 16,
    /* 强制主页 */
    OH_NETCONN_NET_CAPABILITY_CAPTIVE_PORTAL = 17,
    /* 缺省 */
    OH_NETCONN_NET_CAPABILITY_INTERNAL_DEFAULT
} OH_NetConn_NetCap;

/**
 * @brief 网络载体类型
 *
 * @since 10
 * @version 1.0
 */

typedef enum OH_NetConn_NetBearType {
    /* 蜂窝数据 */
    OH_NETCONN_BEARER_CELLULAR = 0,
    /* WIFI */
    OH_NETCONN_BEARER_WIFI = 1,
    /* 蓝牙 */
    OH_NETCONN_BEARER_BLUETOOTH = 2,
    /* 以太网 */
    OH_NETCONN_BEARER_ETHERNET = 3,
    /* VPN */
    OH_NETCONN_BEARER_VPN = 4,
    /* WIFI_AWARE */
    OH_NETCONN_BEARER_WIFI_AWARE = 5,
    /* 缺省 */
    OH_NETCONN_BEARER_DEFAULT
} OH_NetConn_NetBearType;

/**
 * @brief 路由返回类型
 *
 * @since 10
 * @version 1.0
 */

typedef enum OH_NetConn_RtnType {
    /* UNICAST */
    OH_NETCONN_RTN_UNICAST = 1,
    /* UNREACHABLE */
    OH_NETCONN_RTN_UNREACHABLE = 7,
    /* THROW */
    OH_NETCONN_RTN_THROW = 9
} OH_NetConn_RtnType;

/**
 * @brief IP的类型
 *
 * @since 10
 * @version 1.0
 */

typedef enum {
    /* 未知 */
    OH_NETCONN_UNKNOWN = 0x00,
    /* IPV4 */
    OH_NETCONN_IPV4 = 0x01,
    /* IPV6 */
    OH_NETCONN_IPV6 = 0x02,
} OH_NetConn_IpType;

/**
 * @brief 存放netId的结构体
 *
 * @since 10
 * @version 1.0
 */

typedef struct OH_NetConn_NetHandle {
    /* 网络ID标识 */
    int32_t netId;
} OH_NetConn_NetHandle;

/**
 * @brief 存放netHandle的列表的结构体
 *
 * @since 10
 * @version 1.0
 */

typedef struct OH_NetConn_NetHandleList {
    /* netHandle列表 */
    OH_NetConn_NetHandle netHandleList[MAX_NET_SIZE];
    /* netHandleList实际有效空间大小 */
    int32_t netHandleListSize;
} OH_NetConn_NetHandleList;

/**
 * @brief 存放网络能力集的结构体
 *
 * @since 10
 * @version 1.0
 */

typedef struct OH_NetConn_NetAllCapabilities {
    /* 上行带宽 */
    uint32_t linkUpBandwidthKbps;
    /* 下行带宽 */
    uint32_t linkDownBandwidthKbps;
    /* 能力列表 */
    OH_NetConn_NetCap netCaps[MAX_CAP_SIZE];
    /* netCaps实际有效空间大小 */
    int32_t netCapsSize;
    /* 载体类型 */
    OH_NetConn_NetBearType bearerTypes[MAX_BEAR_TYPE_SIZE];
    /* bearerTypes实际有效空间大小 */
    int32_t bearerTypesSize;
} OH_NetConn_NetAllCapabilities;

/**
 * @brief 网络地址的结构体
 *
 * @since 10
 * @version 1.0
 */

typedef struct OH_NetConn_INetAddr {
    /* IP类型 */
    OH_NetConn_IpType type;
    /* 家族 */
    uint8_t family;
    /* 前缀长度 */
    uint8_t prefixlen;
    /* 端口 */
    uint8_t port;
    /* 地址 */
    char address[MAX_STR_LEN];
    /* 主机名 */
    char hostName[MAX_STR_LEN];
} OH_NetConn_INetAddr;

/**
 * @brief 路由信息的结构体
 *
 * @since 10
 * @version 1.0
 */

typedef struct OH_NetConn_Route {
    /* 网络接口 */
    char iface[MAX_STR_LEN];
    /* 目的地址 */
    OH_NetConn_INetAddr destination;
    /* 网关 */
    OH_NetConn_INetAddr gateway;
    /* 返回类型 */
    OH_NetConn_RtnType rtnType;
    /* 最大传输单元 */
    int32_t mtu;
    /* 是否是主机 1是0否 */
    int32_t isHost;
    /* 是否有网关 1是0否*/
    int32_t hasGateway;
    /* 是否是默认路由 1是0否*/
    int32_t isDefaultRoute;
} OH_NetConn_Route;

/**
 * @brief 网络代理信息的结构体
 *
 * @since 10
 * @version 1.0
 */

typedef struct OH_NetConn_HttpProxy {
    /* 主机名 */
    char host[MAX_STR_LEN];
    /* 绕过的地址 */
    char exclusionList[MAX_EXCLUSION_SIZE][MAX_STR_LEN];
    /* exclusionList实际有效空间大小 */
    int32_t exclusionListSize;
    /* 端口 */
    uint16_t port;
} OH_NetConn_HttpProxy;

/**
 * @brief 网络链路信息的结构体
 *
 * @since 10
 * @version 1.0
 */

typedef struct OH_NetConn_NetLinkInfo {
    /* 接口名 */
    char ifaceName[MAX_STR_LEN];
    /* 主机 */
    char domain[MAX_STR_LEN];
    /* TCP缓冲区大小 */
    char tcpBufferSizes[MAX_STR_LEN];
    /* 最大传输单元 */
    uint16_t mtu;
    /* 网络地址列表 */
    OH_NetConn_INetAddr netAddrList[MAX_ADDR_SIZE];
    /* netAddrList实际有效空间大小 */
    int32_t netAddrListSize;
    /* DNS表 */
    OH_NetConn_INetAddr dnsList[MAX_ADDR_SIZE];
    /* dnsList实际有效空间大小 */
    int32_t dnsListSize;
    /* 路由表 */
    OH_NetConn_Route routeList[MAX_ROUTE_SIZE];
    /* routeList实际有效空间大小 */
    int32_t routeListSize;
    /* 代理 */
    OH_NetConn_HttpProxy httpProxy;
} OH_NetConn_NetLinkInfo;

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* NATIVE_NET_CONN_TYPE_H */