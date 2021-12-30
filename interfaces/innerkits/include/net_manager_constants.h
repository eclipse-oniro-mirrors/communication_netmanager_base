/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef NETMANAGER_CONSTANTS_H
#define NETMANAGER_CONSTANTS_H

#include <errors.h>

namespace OHOS {
namespace NetManagerStandard {
constexpr int NETMANAGER_ERROR = -1;
constexpr int NETMANAGER_SUCCESS = 0;

enum {
    NETMANAGER_COMMON = 0x00,
    NETMANAGER_DNS_RESOLVER_MANAGER = 0x01,
    NETMANAGER_ETHERNET_MANAGER = 0x02,
    NETMANAGER_NET_CONN_MANAGER = 0x03,
    NETMANAGER_NET_POLICY_MANAGER = 0x04,
};

// Error code for common
constexpr ErrCode COMMON_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMUNICATION, NETMANAGER_COMMON);

enum {
    NETMANAGER_ERR_FAIL = COMMON_ERR_OFFSET,
    NETMANAGER_ERR_MEMCPY_FAIL,
    NETMANAGER_ERR_MEMSET_FAIL,
    NETMANAGER_ERR_STRCPY_FAIL,
    NETMANAGER_ERR_STRING_EMPTY,
    NETMANAGER_ERR_LOCAL_PTR_NULL,
    NETMANAGER_ERR_PERMISSION_ERR,
    NETMANAGER_ERR_DESCRIPTOR_MISMATCH,
    NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL,
    NETMANAGER_ERR_WRITE_DATA_FAIL,
    NETMANAGER_ERR_WRITE_REPLY_FAIL,
    NETMANAGER_ERR_READ_DATA_FAIL,
    NETMANAGER_ERR_READ_REPLY_FAIL,
    NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL,
    NETMANAGER_ERR_ADD_DEATH_RECIPIENT_FAIL,
    NETMANAGER_ERR_REGISTER_CALLBACK_FAIL,
    NETMANAGER_ERR_UNINIT,
};

// Error code for netmanager dns resolver
constexpr ErrCode DNS_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMUNICATION, NETMANAGER_DNS_RESOLVER_MANAGER);
// Error code for netmanager ethernet
constexpr ErrCode ETHERNET_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMUNICATION, NETMANAGER_ETHERNET_MANAGER);
// Error code for netmanager conn manager
constexpr ErrCode CONN_MANAGER_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMUNICATION, NETMANAGER_NET_CONN_MANAGER);
// Error code for netmanager policy manager
constexpr ErrCode POLICY_MANAGER_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMUNICATION, NETMANAGER_NET_POLICY_MANAGER);
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETMANAGER_CONSTANTS_H