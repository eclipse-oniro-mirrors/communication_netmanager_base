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

#ifndef NET_TETHER_CONSTANTS_H
#define NET_TETHER_CONSTANTS_H

namespace OHOS {
namespace NetManagerStandard {
enum TetheringType {
    TETHERING_INVALID = 0,
    TETHERING_WIFI,
    TETHERING_USB,
    TETHERING_BLUETOOTH,
};
enum TetherResultCode {
    TETHERING_NO_ERR = 0,
    TETHERING_TYPE_ERR,
    TETHERING_GET_SA_ERR,
    TETHERING_UNKNOWN_IFACE_ERROR,
    TETHERING_UNAVAIL_IFACE_ERROR,
    TETHERING_INTERNAL_ERROR,
    TETHERING_TETHER_NOT_OPEN,
    TETHERING_TETHER_ALREADY_OPEN,
    TETHERING_PARAM_ERR,
    TETHERING_REMOTE_NULLPTR_ERR,
    TETHERING_IPC_ERR,
};
enum TetherFailedCode {
    TETHERING_ERR_IFACE_SET = 0,
    TETHERING_ERR_IFACE_TETHER,
    TETHERING_ERR_OPENAP_FAIL,
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_TETHER_CONSTANTS_H
