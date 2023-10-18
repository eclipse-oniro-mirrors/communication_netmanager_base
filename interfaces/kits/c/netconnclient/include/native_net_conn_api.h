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

#ifndef NATIVE_NET_CONN_API_H
#define NATIVE_NET_CONN_API_H

/**
 * @addtogroup NetConn
 * @{
 *
 * @brief 为网络管理的数据网络连接模块提供C接口
 *
 * @since 10
 * @version 1.0
 */

/**
 * @file native_net_conn_api.h
 *
 * @brief 为网络管理的数据网络连接模块定义C接口
 *
 * @syscap SystemCapability.Communication.NetManager.Core
 * @since 10
 * @version 1.0
 */

#include "native_net_conn_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 查询是否有默认激活的数据网络
 *
 * @param hasDefaultNet 是否有默认网络
 * @return 0 - 成功.
 * @return 201 - 缺少权限.
 * @return 401 - 参数错误.
 * @return 2100002 - 无法连接到服务.
 * @return 2100003 - 内部错误.
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_HasDefaultNet(int32_t *hasDefaultNet);

/**
 * @brief 获取激活的默认的数据网络
 *
 * @param netHandle 存放网络ID
 * @return 0 - 成功.
 * @return 201 - 缺少权限.
 * @return 401 - 参数错误.
 * @return 2100002 - 无法连接到服务.
 * @return 2100003 - 内部错误.
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetDefaultNet(OH_NetConn_NetHandle *netHandle);

/**
 * @brief 查询默认数据网络是否记流量
 *
 * @param isMetered 是否激活
 * @return 0 - 成功.
 * @return 201 - 缺少权限.
 * @return 401 - 参数错误.
 * @return 2100002 - 无法连接到服务.
 * @return 2100003 - 内部错误.
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_IsDefaultNetMetered(int32_t *isMetered);

/**
 * @brief 查询所有激活的数据网络
 *
 * @param netHandleList 网络信息列表
 * @return 0 - 成功.
 * @return 201 - 缺少权限.
 * @return 401 - 参数错误.
 * @return 2100002 - 无法连接到服务.
 * @return 2100003 - 内部错误.
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetAllNets(OH_NetConn_NetHandleList *netHandleList);

/**
 * @brief 查询某个数据网络的链路信息
 *
 * @param netHandle 存放网络ID
 * @param info 存放链路信息
 * @return 0 - 成功.
 * @return 201 - 缺少权限.
 * @return 401 - 参数错误.
 * @return 2100002 - 无法连接到服务.
 * @return 2100003 - 内部错误.
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetConnectionProperties(OH_NetConn_NetHandle *netHandle, OH_NetConn_NetLinkInfo *info);

/**
 * @brief 查询某个网络的能力集
 *
 * @param netHandle 存放网络ID
 * @param netAllCapacities 存放能力集
 * @return 0 - 成功.
 * @return 201 - 缺少权限.
 * @return 401 - 参数错误.
 * @return 2100002 - 无法连接到服务.
 * @return 2100003 - 内部错误.
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetNetCapabilities(OH_NetConn_NetHandle *netHandle, OH_NetConn_NetAllCapabilities *netAllCapacities);

/**
 * @brief 查询默认的网络代理
 *
 * @param httpProxy 存放代理配置信息
 * @return 0 - 成功.
 * @return 401 - 参数错误.
 * @return 2100002 - 无法连接到服务.
 * @return 2100003 - 内部错误.
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetDefaultHttpProxy(OH_NetConn_HttpProxy *httpProxy);

/**
 * @brief Query default network proxy
 *
 * @param host The host name to query
 * @param serv Service name
 * @param hint Pointer to the addrinfo structure
 * @param res Store DNS query results and return them in a linked list format
 * @param netId DNS query netId, 0 is used for default netid query
 * @return 0 - Success.
 * @return 401 - Parameter error.
 * @return 2100003 - Internal error.
 * @since 11
 * @version 1.0
*/
int32_t OH_NetConn_GetAddrInfo(char *host, char *serv, struct addrinfo *hint, struct addrinfo **res, int32_t netId);

/**
 * @brief Query default network proxy
 *
 * @param res DNS query result chain header
 * @return 0 - Success.
 * @return 401 - Parameter error.
 * @return 2100002 - Unable to connect to service.
 * @return 2100003 - Internal error.
 * @since 11
 * @version 1.0
*/
int32_t OH_NetConn_FreeDnsResult(struct addrinfo *res);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* NATIVE_NET_CONN_API_H */