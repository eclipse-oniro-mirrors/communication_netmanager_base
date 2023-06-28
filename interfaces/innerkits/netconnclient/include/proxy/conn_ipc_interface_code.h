/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CONN_IPC_INTERFACE_CODE_H
#define CONN_IPC_INTERFACE_CODE_H

/* SAID: 1151 */
namespace OHOS {
namespace NetManagerStandard {
enum class ConnInterfaceCode {
    CMD_NM_START,
    CMD_NM_REGISTER_NET_SUPPLIER,
    CMD_NM_SYSTEM_READY,
    CMD_NM_REGISTER_NET_CONN_CALLBACK,
    CMD_NM_REGISTER_NET_CONN_CALLBACK_BY_SPECIFIER,
    CMD_NM_UNREGISTER_NET_CONN_CALLBACK,
    CMD_NM_REG_NET_SUPPLIER,
    CMD_NM_UNREG_NETWORK,
    CMD_NM_SET_NET_SUPPLIER_INFO,
    CMD_NM_SET_NET_LINK_INFO,
    CMD_NM_GETDEFAULTNETWORK,
    CMD_NM_HASDEFAULTNET,
    CMD_NM_NET_DETECTION,
    CMD_NM_GET_IFACE_NAMES,
    CMD_NM_GET_IFACENAME_BY_TYPE,
    CMD_NM_GET_ADDRESSES_BY_NAME,
    CMD_NM_GET_ADDRESS_BY_NAME,
    CMD_NM_GET_SPECIFIC_NET,
    CMD_NM_GET_ALL_NETS,
    CMD_NM_GET_SPECIFIC_UID_NET,
    CMD_NM_GET_CONNECTION_PROPERTIES,
    CMD_NM_GET_NET_CAPABILITIES,
    CMD_NM_BIND_SOCKET,
    CMD_NM_REGISTER_NET_DETECTION_RET_CALLBACK,
    CMD_NM_UNREGISTER_NET_DETECTION_RET_CALLBACK,
    CMD_NM_UPDATE_NET_STATE_FOR_TEST,
    CMD_NM_REGISTER_NET_SUPPLIER_CALLBACK,
    CMD_NM_SET_AIRPLANE_MODE,
    CMD_NM_IS_DEFAULT_NET_METERED,
    CMD_NM_SET_GLOBAL_HTTP_PROXY,
    CMD_NM_GET_GLOBAL_HTTP_PROXY,
    CMD_NM_GET_NET_ID_BY_IDENTIFIER,
    CMD_NM_SET_APP_NET,
    CMD_NM_SET_INTERNET_PERMISSION,
    CMD_NM_GET_DEFAULT_HTTP_PROXY,
    CMD_NM_REGISTER_NET_INTERFACE_CALLBACK,
    CMD_NM_GET_INTERFACE_CONFIGURATION,
    CMD_NM_END,
};
}
}
#endif // CONN_IPC_INTERFACE_CODE_H