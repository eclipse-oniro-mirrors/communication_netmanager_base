/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "errorcode_convertor.h"

#include "net_conn_constants.h"
#include "net_manager_constants.h"
#include "net_policy_constants.h"
#include "net_stats_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr int32_t ERROR_DIVISOR = 1000;
constexpr int32_t ERROR_RANGE = 100;
std::map<int32_t, int32_t> g_errNumMap = {
    {NETMANAGER_ERR_MEMCPY_FAIL, NETMANAGER_ERR_INTERNAL},
    {NETMANAGER_ERR_MEMSET_FAIL, NETMANAGER_ERR_INTERNAL},
    {NETMANAGER_ERR_STRCPY_FAIL, NETMANAGER_ERR_INTERNAL},
    {NETMANAGER_ERR_STRING_EMPTY, NETMANAGER_ERR_INTERNAL},
    {NETMANAGER_ERR_LOCAL_PTR_NULL, NETMANAGER_ERR_INTERNAL},
    {NETMANAGER_ERR_DESCRIPTOR_MISMATCH, NETMANAGER_ERR_IPC},
    {NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL, NETMANAGER_ERR_IPC},
    {NETMANAGER_ERR_WRITE_DATA_FAIL, NETMANAGER_ERR_IPC},
    {NETMANAGER_ERR_WRITE_REPLY_FAIL, NETMANAGER_ERR_IPC},
    {NETMANAGER_ERR_READ_DATA_FAIL, NETMANAGER_ERR_IPC},
    {NETMANAGER_ERR_READ_REPLY_FAIL, NETMANAGER_ERR_IPC},
    {NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL, NETMANAGER_ERR_IPC},
    {NETMANAGER_ERR_GET_PROXY_FAIL, NETMANAGER_ERR_IPC},
    {NETMANAGER_EXT_ERR_MEMCPY_FAIL, NETMANAGER_EXT_ERR_INTERNAL},
    {NETMANAGER_EXT_ERR_MEMSET_FAIL, NETMANAGER_EXT_ERR_INTERNAL},
    {NETMANAGER_EXT_ERR_STRCPY_FAIL, NETMANAGER_EXT_ERR_INTERNAL},
    {NETMANAGER_EXT_ERR_STRING_EMPTY, NETMANAGER_EXT_ERR_INTERNAL},
    {NETMANAGER_EXT_ERR_LOCAL_PTR_NULL, NETMANAGER_EXT_ERR_INTERNAL},
    {NETMANAGER_EXT_ERR_DESCRIPTOR_MISMATCH, NETMANAGER_EXT_ERR_IPC},
    {NETMANAGER_EXT_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL, NETMANAGER_EXT_ERR_IPC},
    {NETMANAGER_EXT_ERR_WRITE_DATA_FAIL, NETMANAGER_EXT_ERR_IPC},
    {NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL, NETMANAGER_EXT_ERR_IPC},
    {NETMANAGER_EXT_ERR_READ_DATA_FAIL, NETMANAGER_EXT_ERR_IPC},
    {NETMANAGER_EXT_ERR_READ_REPLY_FAIL, NETMANAGER_EXT_ERR_IPC},
    {NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL, NETMANAGER_EXT_ERR_IPC},
    {NETMANAGER_EXT_ERR_GET_PROXY_FAIL, NETMANAGER_EXT_ERR_IPC},
};
std::map<int32_t, const char *> g_errStringMap = {
    /* Net base common error */
    {NETMANAGER_SUCCESS, "Successful"},
    {NETMANAGER_ERR_PERMISSION_DENIED, "Permission denied"},
    {NETMANAGER_ERR_PARAMETER_ERROR, "Parameter error"},
    {NETMANAGER_ERR_CAPABILITY_NOT_SUPPORTED, "Capability not supported"},
    {NETMANAGER_ERR_INVALID_PARAMETER, "Invalid parameter value"},
    {NETMANAGER_ERR_OPERATION_FAILED, "Operation failed. Cannot connect to service"},
    {NETMANAGER_ERR_INTERNAL, "System internal error"},
    {NETMANAGER_ERR_IPC, "IPC communication error"},
    {NETMANAGER_ERR_REGISTER_CALLBACK_FAIL, "Register callback failed"},
    {NETMANAGER_ERR_UNINIT, "Service not init"},
    /* Net ext common error */
    {NETMANAGER_EXT_SUCCESS, "successful"},
    {NETMANAGER_EXT_ERR_PERMISSION_DENIED, "Permission denied"},
    {NETMANAGER_EXT_ERR_PARAMETER_ERROR, "Parameter error"},
    {NETMANAGER_EXT_ERR_CAPABILITY_NOT_SUPPORTED, "Capability not supported"},
    {NETMANAGER_EXT_ERR_INVALID_PARAMETER, "Invalid parameter value"},
    {NETMANAGER_EXT_ERR_OPERATION_FAILED, "Operation failed. Cannot connect to service"},
    {NETMANAGER_EXT_ERR_INTERNAL, "System internal error"},
    {NETMANAGER_EXT_ERR_IPC, "IPC communication error"},
    {NETMANAGER_EXT_ERR_REGISTER_CALLBACK_FAIL, "Register callback failed"},
    {NETMANAGER_EXT_ERR_UNINIT, "Service not init"},
    /* Net Policy Manager */
    {POLICY_ERR_INVALID_UID, "Invalid uid value"},
    {POLICY_ERR_INVALID_POLICY, "Invalid policy value"},
    {POLICY_ERR_INVALID_QUOTA_POLICY, "Invalid quota policy value"},
    {POLICY_ERR_QUOTA_POLICY_NOT_EXIST, "Quota policy not exist"},
    /* Net Ethernet Manager */
    {ETHERNET_ERR_INIT_FAIL, "Failed to initialize the Ethernet"},
    {ETHERNET_ERR_EMPTY_CONFIGURATION, "The Ethernet configuration file is empty"},
    {ETHERNET_ERR_DUMP, "Ethernet DUMP error"},
    {ETHERNET_ERR_DEVICE_CONFIGURATION_INVALID, "Invalid Ethernet configuration file"},
    {ETHERNET_ERR_DEVICE_INFORMATION_NOT_EXIST, "The device information does not exist"},
    {ETHERNET_ERR_DEVICE_NOT_LINK, "The Ethernet device is not connected"},
    {ETHERNET_ERR_USER_CONIFGURATION_WRITE_FAIL,
     "Description Failed to write user configuration information on the Ethernet"},
    {ETHERNET_ERR_USER_CONIFGURATION_CLEAR_FAIL, "The Ethernet failed to clear user configuration information"},
    {ETHERNET_ERR_CONVERT_CONFIGURATINO_FAIL, "The static configuration of Ethernet conversion failed"},
    /* Net Share Manager */
    {NETWORKSHARE_ERROR_UNKNOWN_TYPE, "Sharing iface type is unkonwn"},
    {NETWORKSHARE_ERROR_UNKNOWN_IFACE, "Try to share an unknown iface"},
    {NETWORKSHARE_ERROR_UNAVAIL_IFACE, "Try to share an unavailable iface"},
    {NETWORKSHARE_ERROR_WIFI_SHARING, "WiFi sharing failed"},
    {NETWORKSHARE_ERROR_BT_SHARING, "Bluetooth sharing failed"},
    {NETWORKSHARE_ERROR_SHARING_IFACE_ERROR, "Sharing iface failed"},
    {NETWORKSHARE_ERROR_ENABLE_FORWARDING_ERROR, "Network share enable forwarding error"},
    {NETWORKSHARE_ERROR_INTERNAL_ERROR, "Network share internal error"},
    {NETWORKSHARE_ERROR_IFACE_CFG_ERROR, "Get network card configuration is null"},
    {NETWORKSHARE_ERROR_DHCPSERVER_ERROR, "DHCP server failed"},
    {NETWORKSHARE_ERROR_ISSHARING_CALLBACK_ERROR, "Issharing callback is null"},
    /* Net Stats Manager */
    {STATS_DUMP_MESSAGE_FAIL, "Dump message failed"},
    {STATS_REMOVE_FILE_FAIL, "Remove file failed "},
    {STATS_ERR_INVALID_TIME_PERIOD, "Time period invalid"},
    {STATS_ERR_READ_BPF_FAIL, "Read bpf failed"},
    {STATS_ERR_INVALID_KEY, "The key of bpf is invalid"},
    {STATS_ERR_INVALID_IFACE_STATS_MAP, "The iface stats map is invalid"},
    {STATS_ERR_INVALID_STATS_VALUE, "The stats value is invalid"},
    {STATS_ERR_INVALID_STATS_TYPE, "Invalid stats type"},
    {STATS_ERR_INVALID_UID_STATS_MAP, "The uid stats map in bpf is invalid"},
    {STATS_ERR_INVALID_IFACE_NAME_MAP, "The iface map in bpf is invalid"},
    {STATS_ERR_GET_IFACE_NAME_FAILED, "Get iface name failed"},
    {STATS_ERR_CLEAR_STATS_DATA_FAIL, "Clear stats data failed"},
    {STATS_ERR_CREATE_TABLE_FAIL, "Create stats table failed"},
    {STATS_ERR_DATABASE_RECV_NO_DATA, "The data got from database is null"},
    {STATS_ERR_WRITE_DATA_FAIL, "Write data into database failed"},
    {STATS_ERR_READ_DATA_FAIL, "Read data from database failed"},
    /* Net connection Manager */
    {NET_CONN_ERR_INVALID_SUPPLIER_ID, "the supplier id is invalid"},
    {NET_CONN_ERR_NET_TYPE_NOT_FOUND, "the net type is not found"},
    {NET_CONN_ERR_NO_ANY_NET_TYPE, "no any net type"},
    {NET_CONN_ERR_NO_REGISTERED, "no registered"},
    {NET_CONN_ERR_NETID_NOT_FOUND, "the net id is not found"},
    {NET_CONN_ERR_SAME_CALLBACK, "the same callback exists"},
    {NET_CONN_ERR_CALLBACK_NOT_FOUND, "the callback is not found"},
    {NET_CONN_ERR_REQ_ID_NOT_FOUND, "the request id is not found"},
    {NET_CONN_ERR_NO_DEFAULT_NET, "the net is not defualt"},
    {NET_CONN_ERR_HTTP_PROXY_INVALID, "the http proxy is invalid"},
    {NET_CONN_ERR_NO_HTTP_PROXY, "the httpProxy in service is null"},
    {NET_CONN_ERR_INVALID_NETWORK, "the network is nullptr"},
    {NET_CONN_ERR_SERVICE_REQUEST_CONNECT_FAIL, "the service request connection failed "},
    {NET_CONN_ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL, "the service update net link infomation failed"},
    {NET_CONN_ERR_NO_SUPPLIER, "supplier doesn't exist"},
    {NET_CONN_ERR_NET_MONITOR_OPT_FAILED, "net monitor failed"},
    {NET_CONN_ERR_SERVICE_NO_REQUEST, "no service request"},
    {NET_CONN_ERR_NO_ADDRESS, "address list is null"},
    {NET_CONN_ERR_NET_NOT_FIND_BESTNETWORK_FOR_REQUEST, "the bestnetwork request was not found"},
    {NET_CONN_ERR_NET_NO_RESTRICT_BACKGROUND, "no restrict background "},
    {NET_CONN_ERR_NET_OVER_MAX_REQUEST_NUM, "the number of requests exceeded the maximum"},
    {NET_CONN_ERR_CREATE_DUMP_FAILED, "create dump failed"},
};
} // namespace
std::string NetBaseErrorCodeConvertor::ConvertErrorCode(int32_t &errorCode)
{
    if (errorCode > ERROR_DIVISOR && (errorCode % ERROR_DIVISOR) >= ERROR_RANGE) {
        if (g_errNumMap.find(errorCode) != g_errNumMap.end()) {
            errorCode = g_errNumMap.at(errorCode);
        }
    }
    if (g_errStringMap.find(errorCode) == g_errStringMap.end()) {
        return {};
    }
    return g_errStringMap.at(errorCode);
}
} // namespace NetManagerStandard
} // namespace OHOS