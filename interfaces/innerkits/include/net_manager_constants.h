/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
    NETMANAGER_NET_CONN_MANAGER = 0x03,
    NETMANAGER_NET_POLICY_MANAGER = 0x04,
};

// Error code for common
constexpr ErrCode COMMON_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMUNICATION, NETMANAGER_COMMON);

enum {
    NETMANAGER_ERR_MEMCPY_FAIL = 2,
    NETMANAGER_ERR_MEMSET_FAIL = 3,
    NETMANAGER_ERR_STRCPY_FAIL = 4,
    NETMANAGER_ERR_STRING_EMPTY = 5,
    NETMANAGER_ERR_LOCAL_PTR_NULL = 6,
    NETMANAGER_ERR_DESCRIPTOR_MISMATCH = 102,
    NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL = 103,
    NETMANAGER_ERR_WRITE_DATA_FAIL = 104,
    NETMANAGER_ERR_WRITE_REPLY_FAIL = 105,
    NETMANAGER_ERR_READ_DATA_FAIL = 106,
    NETMANAGER_ERR_READ_REPLY_FAIL = 107,
    NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL = 108,
    NETMANAGER_ERR_IPC_GET_PROXY_FAIL = 109,
    NETMANAGER_ERR_PERMISSION_DENIED = 201,
    NETMANAGER_ERR_PARAMETER_ERROR = 401,
    NETMANAGER_ERR_ADD_DEATH_RECIPIENT_FAIL = 502,
    NETMANAGER_ERR_REGISTER_CALLBACK_FAIL = 503,
    NETMANAGER_ERR_UNINIT = 504,
    NETMANAGER_ERR_CAPABILITY_NOT_SUPPORTED = 801,
};

// Error code for netmanager dns resolver
constexpr ErrCode DNS_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMUNICATION, NETMANAGER_DNS_RESOLVER_MANAGER);
// Error code for netmanager conn manager
constexpr ErrCode CONN_MANAGER_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMUNICATION, NETMANAGER_NET_CONN_MANAGER);
// Error code for netmanager policy manager
constexpr ErrCode POLICY_MANAGER_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMUNICATION, NETMANAGER_NET_POLICY_MANAGER);
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETMANAGER_CONSTANTS_H
