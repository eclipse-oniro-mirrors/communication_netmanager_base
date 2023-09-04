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
 * @brief Provides C interfaces to manage and use data networks.
 *
 * @since 10
 * @version 1.0
 */

/**
 * @file native_net_conn_api.h
 *
 * @brief Defines C interfaces to manage and use data networks.
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
 * @brief Checks whether the default data network is activated.
 *
 * @param hasDefaultNet Whether the default data network is activated.
 * @return 0 - Success.
 * @return 201 - Permission denied.
 * @return 401 - Parameter error.
 * @return 2100003 - System internal error.
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_HasDefaultNet(int32_t *hasDefaultNet);

/**
 * @brief Obtains the data network that is activated by default.
 *
 * @param netHandle Stores the data network id.
 * @return 0 - Success.
 * @return 201 - Permission denied.
 * @return 401 - Parameter error.
 * @return 2100003 - System internal error.
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetDefaultNet(OH_NetConn_NetHandle *netHandle);

/**
 * @brief Checks whether data traffic usage on the current network is metered.
 *
 * @param isMetered Whether the current network is metered.
 * @return 0 - Success.
 * @return 201 - Permission denied.
 * @return 401 - Parameter error.
 * @return 2100003 - System internal error.
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_IsDefaultNetMetered(int32_t *isMetered);

/**
 * @brief Obtains the list of data networks that are activated.
 *
 * @param netHandleList Stores the list of data networks.
 * @return 0 - Success.
 * @return 201 - Permission denied.
 * @return 401 - Parameter error.
 * @return 2100003 - System internal error.
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetAllNets(OH_NetConn_NetHandleList *netHandleList);

/**
 * @brief Queries the connection properties of a network.
 *
 * @param netHandle Stores the data network id.
 * @param info Stores the connection properties
 * @return 0 - Success.
 * @return 201 - Permission denied.
 * @return 401 - Parameter error.
 * @return 2100003 - System internal error.
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetConnectionProperties(OH_NetConn_NetHandle *netHandle, OH_NetConn_NetLinkInfo *info);

/**
 * @brief Obtains capabilities of a network.
 *
 * @param netHandle Stores the data network id.
 * @param netAllCapacities Stores capabilities.
 * @return 0 - Success.
 * @return 201 - Permission denied.
 * @return 401 - Parameter error.
 * @return 2100003 - System internal error.
 * @permission ohos.permission.GET_NETWORK_INFO
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetNetCapabilities(OH_NetConn_NetHandle *netHandle, OH_NetConn_NetAllCapabilities *netAllCapacities);

/**
 * @brief Obtains the default proxy settings.
 *
 * @param httpProxy Stores proxy settings.
 * @return 0 - Success.
 * @return 401 - Parameter error.
 * @return 2100003 - System internal error.
 * @since 10
 * @version 1.0
 */
int32_t OH_NetConn_GetDefaultHttpProxy(OH_NetConn_HttpProxy *httpProxy);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* NATIVE_NET_CONN_API_H */