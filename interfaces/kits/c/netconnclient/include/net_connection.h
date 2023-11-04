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
 * @brief Provide C interface for the data network connection module of network management.
 *
 * @since 11
 * @version 1.0
 */

/**
 * @file native_net_conn_api.h
 *
 * @brief Provide C interface for the data network connection module of network management.
 *
 * @syscap SystemCapability.Communication.NetManager.Core
 * @since 11
 * @version 1.0
 */

#include <netdb.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get DNS result with netId.
 *
 * @param host The host name to query.
 * @param serv Service name.
 * @param hint Pointer to the addrinfo structure.
 * @param res Store DNS query results and return them in a linked list format.
 * @param netId DNS query netId, 0 is used for default netid query.
 * @return 0 - Success.
 * @return 201 - Missing permissions.
 * @return 401 - Parameter error.
 * @return 2100002 - Unable to connect to service.
 * @return 2100003 - Internal error.
 * @permission ohos.permission.INTERNET
 * @syscap SystemCapability.Communication.NetManager.Core
 * @since 11
 * @version 1.0
*/
int32_t OH_NetConn_GetAddrInfo(char *host, char *serv, struct addrinfo *hint, struct addrinfo **res, int32_t netId);

/**
 * @brief Free DNS result.
 *
 * @param res DNS query result chain header.
 * @return 0 - Success.
 * @return 201 - Missing permissions.
 * @return 401 - Parameter error.
 * @return 2100002 - Unable to connect to service.
 * @return 2100003 - Internal error.
 * @permission ohos.permission.INTERNET
 * @syscap SystemCapability.Communication.NetManager.Core
 * @since 11
 * @version 1.0
*/
int32_t OH_NetConn_FreeDnsResult(struct addrinfo *res);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* NATIVE_NET_CONN_API_H */
