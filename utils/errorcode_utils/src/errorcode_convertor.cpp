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

namespace OHOS {
namespace NetManagerStandard {
ErrorCodeConvertor::ErrorCodeConvertor()
{
    errorMap_[NETMANAGER_SUCCESS] = "successful";
    errorMap_[NETMANAGER_ERR_MEMCPY_FAIL] = "memcpy fail";
    errorMap_[NETMANAGER_ERR_MEMSET_FAIL] = "memset fail";
    errorMap_[NETMANAGER_ERR_STRCPY_FAIL] = "strcpy fail";
    errorMap_[NETMANAGER_ERR_STRING_EMPTY] = "string is null";
    errorMap_[NETMANAGER_ERR_LOCAL_PTR_NULL] = "pointer is null";
    errorMap_[NETMANAGER_ERR_DESCRIPTOR_MISMATCH] = "ipc stub mismatch descriptor";
    errorMap_[NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL] = "ipc proxy write descriptor token fail";
    errorMap_[NETMANAGER_ERR_WRITE_DATA_FAIL] = "ipc proxy write data fail";
    errorMap_[NETMANAGER_ERR_WRITE_REPLY_FAIL] = "ipc proxy write reply fail";
    errorMap_[NETMANAGER_ERR_READ_DATA_FAIL] = "ipc stub read data fail";
    errorMap_[NETMANAGER_ERR_READ_REPLY_FAIL] = "ipc stub read reply fail";
    errorMap_[NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL] = "ipc connect stub fail";
    errorMap_[NETMANAGER_ERR_PERMISSION_DENIED] = "permission denied";
    errorMap_[NETMANAGER_ERR_PARAMETER_ERROR] = "parameter error";
    errorMap_[NETMANAGER_ERR_ADD_DEATH_RECIPIENT_FAIL] = "ipc add death recipient fail";
    errorMap_[NETMANAGER_ERR_REGISTER_CALLBACK_FAIL] = "register callback fail";
    errorMap_[NETMANAGER_ERR_UNINIT] = "uninit error";
    errorMap_[NETMANAGER_ERR_CAPABILITY_NOT_SUPPORTED] = "capability not supported";
}

std::string ErrorCodeConvertor::ConvertErrorCode(int32_t errorCode)
{
    return "";
}

NetBaseErrorCodeConvertor::NetBaseErrorCodeConvertor()
{
    errorMap_[NET_CONN_ERR_INVALID_SUPPLIER_ID] = "invalid supplier id";
}
std::map<int32_t, const char *> g_errStringMap = {

    /* Net mDNS Manager */
    {NET_MDNS_ERR_UNKNOWN, "Unknown error"},
    {NET_MDNS_ERR_CALLBACK_NOT_FOUND, "Callback not found"},
    {NET_MDNS_ERR_CALLBACK_DUPLICATED, "Callback duplicated"},
    {NET_MDNS_ERR_PAYLOAD_PARSER_FAIL, "Payload parser failed"},
    {NET_MDNS_ERR_EMPTY_PAYLOAD, "Empty payload"},
    {NET_MDNS_ERR_TIMEOUT, "Request timeout"},
    {NET_MDNS_ERR_ILLEGAL_ARGUMENT, "Illegal argument"},
    {NET_MDNS_ERR_SERVICE_INSTANCE_DUPLICATE, "Service instance duplicated"},
    {NET_MDNS_ERR_SERVICE_INSTANCE_NOT_FOUND, "Service instance not found"},
    {NET_MDNS_ERR_SEND, "Send packet failed"}
};

std::string NetBaseErrorCodeConvertor::ConvertErrorCode(int32_t errorCode)
{
    if (errorMap_.find(errorCode) == errorMap_.end()) {
        return "";
    }
    return errorMap_.at(errorCode);
}

} // namespace NetManagerStandard
} // namespace OHOS
