/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef NET_CONN_TYPES_H
#define NET_CONN_TYPES_H

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t MAX_IPV4_ADDRESS_LEN = 32;

enum ResultCode {
    ERR_NONE                                                        = 0,
    ERR_SERVICE_REQUEST_SUCCESS                                     = (-1),
    ERR_SERVICE_REQUEST_CONNECT_FAIL                                = (-2),
    ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL                           = (-3),
    ERR_SERVICE_CONNECTING                                          = (-4),
    ERR_SERVICE_CONNECTED                                           = (-5),
    ERR_SERVICE_DISCONNECTED_FAIL                                   = (-6),
    ERR_SERVICE_DISCONNECTING                                       = (-7),
    ERR_SERVICE_DISCONNECTED_SUCCESS                                = (-8),
    ERR_SERVICE_NULL_PTR                                            = (-9),
    ERR_NO_SUPPLIER                                                 = (-10),
    ERR_NO_NETWORK                                                  = (-11),
    ERR_INVALID_PARAMS                                              = (-12),
    ERR_INVALID_NETORK_TYPE                                         = (-13),
    ERR_SERVICE_UPDATE_NET_LINK_INFO_SUCCES                         = (-14),
    ERR_NET_MONITOR_OPT_SUCCESS                                     = (-15),
    ERR_NET_MONITOR_OPT_FAILED                                      = (-16),
    ERR_NET_SUPPLIER_NO_API                                         = (-17),
    ERR_NET_TYPE_NOT_FOUND                                          = (-18),
    ERR_NO_ANY_NET_TYPE                                             = (-19),
    ERR_NO_REGISTERED                                               = (-20),
    ERR_NET_HOST_EMPTY = (-21),
    ERR_SERVICE_NO_REQUEST = (-22),
    ERR_NO_ADDRESS = (-23),
    ERR_NET_FIND_BESTNETWORK_FOR_REQUEST = (-24),
    ERR_NET_NOT_FIND_BESTNETWORK_FOR_REQUEST = (-25),
    ERR_NET_NOT_FIND_REQUEST_ID = (-26),
    ERR_NET_NOT_FIND_NETID = (-27),
    ERR_VPN = (-28),
    ERR_NET_DEFAULTNET_NOT_EXIST = (-29),
    ERR_NET_NO_RESTRICT_BACKGROUND = (-30)
};

enum NetMonitorResponseCode {
    OK = 200,
    CREATED = 201,
    NO_CONTENT = 204,
    URL_REDIRECT_MAX = 399,
    BAD_REQUEST = 400,
    CLIENT_ERROR_MAX = 499,
};

enum NetDetectionStatus {
    INVALID_DETECTION_STATE,
    VERIFICATION_STATE,
    EVALUATING_STATE,
    CAPTIVE_PORTAL_STATE,
};
using NetDetectionStateHandler = std::function<void(NetDetectionStatus netDetectionState,
    const std::string &urlRedirect)>;
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_TYPES_H
