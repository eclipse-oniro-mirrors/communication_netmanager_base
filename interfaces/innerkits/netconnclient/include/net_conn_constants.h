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

#ifndef NET_CONN_CONSTANTS_H
#define NET_CONN_CONSTANTS_H

namespace OHOS {
namespace NetManagerStandard {
enum NetConnResultCode {
    NET_CONN_SUCCESS                                = 0,
    NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED           = (-1),
    NET_CONN_ERR_INPUT_NULL_PTR                     = (-2),
    NET_CONN_ERR_INVALID_SUPPLIER_ID                = (-3),
    NET_CONN_ERR_INVALID_PARAMETER                  = (-4),
    NET_CONN_ERR_NET_TYPE_NOT_FOUND                 = (-5),
    NET_CONN_ERR_NO_ANY_NET_TYPE                    = (-6),
    NET_CONN_ERR_NO_REGISTERED                      = (-7),
    NET_CONN_ERR_NETID_NOT_FOUND                    = (-8),
    NET_CONN_ERR_PERMISSION_CHECK_FAILED            = (-9),
    NET_CONN_ERR_SAME_CALLBACK                      = (-10),
    NET_CONN_ERR_CALLBACK_NOT_FOUND                 = (-11),
    NET_CONN_ERR_REQ_ID_NOT_FOUND                   = (-12),
    NET_CONN_ERR_NO_DEFAULT_NET                     = (-13),
    NET_CONN_ERR_INTERNAL_ERROR                     = (-1000)
};

enum NetDetectionResultCode {
    NET_DETECTION_FAIL = 0,
    NET_DETECTION_SUCCESS,
    NET_DETECTION_CAPTIVE_PORTAL,
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_CONSTANTS_H