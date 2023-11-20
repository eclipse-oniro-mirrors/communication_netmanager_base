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
* @addtogroup NetConnection
* @{
*
* @brief Provides the data structures for the C APIs of the network connection module for network management.
*
* @since 11
* @version 1.0
*/

/**
* @file net_connection_type.h
* @brief Defines the data structures for the C APIs of the network connection module.
*
* @library libnet_connection.so
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

/**
* @brief Defines the network handle.
*
* @since 11
* @version 1.0
*/
typedef struct OH_NetConn_NetHandle {
    /** Network ID */
    int32_t netId;
} OH_NetConn_NetHandle;

/**
* @brief Defines the network handle list.
*
* @since 11
* @version 1.0
*/
typedef struct OH_NetConn_NetHandleList {
    /** Network handle list */
    OH_NetConn_NetHandle netHandles[OH_NETCONN_MAX_NET_SIZE];
    /** Actual size of the network handle list */
    int32_t netHandleListSize;
} OH_NetConn_NetHandleList;

/*
 * @brief Pointer to the custom DNS resolver.
 *
 * @param host The host name to query.
 * @param serv Service name.
 * @param hint Pointer to the addrinfo structure.
 * @param res Store DNS query results and return them in a linked list format.
 *
 * @since 11
 * @version 1.0
 */
typedef int (*OH_NetConn_CustomDnsResolver)(const char *host, const char *serv,
    const struct addrinfo *hint, struct addrinfo **res);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* NATIVE_NET_CONN_TYPE_H */