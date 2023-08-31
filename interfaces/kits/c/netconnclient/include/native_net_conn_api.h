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
 * @brief 向应用提供网络连接管理功能，包括获取各种数据网络连接的链路信息和能力集信息等
 *
 * @since 10
 * @version 1.0
 */

/**
 * @file native_net_conn_api.h
 *
 * @brief 定义获取和使用网络信息的相关接口
 *
 * @since 10
 * @version 1.0
 */

#include "native_net_conn_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 查询是否有默认网络
 *
 * @param hasDefaultNet 用来存放是否有默认网络的结果 1是0否
 * @return 查询成功则返回0, 否则失败
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_HasDefaultNet(int32_t *hasDefaultNet);

/**
 * @brief 获取一个含有默认网络的netId的NetHandle
 *
 * @param netHandle 用来存放网络的ID
 * @return 获取成功则返回0, 否则失败
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetDefaultNet(OH_NetConn_NetHandle *netHandle);

/**
 * @brief 查询默认网络是否记录流量
 *
 * @param isMetered 用来存放是否记流量的结果 1是0否
 * @return 查询成功则返回0, 否则失败
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_IsDefaultNetMetered(int32_t *isMetered);

/**
 * @brief 获取所处于连接状态的网络的MetHandle列表
 *
 * @param netHandleList 用来存放netHandle的列表
 * @return 获取成功则返回0, 否则失败
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetAllNets(OH_NetConn_NetHandleList *netHandleList);

/**
 * @brief 查询网络的链路信息
 *
 * @param netHandle 存放要查询的网络ID
 * @param info 存放查询到的链路信息
 * @return 查询成功则返回0, 否则失败
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetConnectionProperties(OH_NetConn_NetHandle *netHandle, OH_NetConn_NetLinkInfo *info);

/**
 * @brief 查询网络的能力集信息
 *
 * @param netHandle 存放要查询的网络ID
 * @param netAllCapacities 存放查询到的能力集信息
 * @return 成功则返回0, 否则失败
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetNetCapabilities(OH_NetConn_NetHandle *netHandle, OH_NetConn_NetAllCapabilities *netAllCapacities);

/**
 * @brief 获取当前网络代理信息
 *
 * @param httpProxy 存放代理的信息
 * @return 成功则返回0, 否则失败
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetDefaultHttpProxy(OH_NetConn_HttpProxy *httpProxy);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* NATIVE_NET_CONN_API_H */