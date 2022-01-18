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

#ifndef NETD_CONTROLLER_DEFINE_H
#define NETD_CONTROLLER_DEFINE_H

#include <string>
#include <functional>

namespace OHOS {
namespace NetManagerStandard {
struct NetdNotifyCallback {
    std::function<void(const std::string &iface)> NetdResponseInterfaceAdd;
    std::function<void(const std::string &iface)> NetdResponseInterfaceRemoved;
};

enum NetdContrlResultCode {
    ERR_NULLPTR = 0,
    ERR_NATIVESERVICE_NOTFIND = (-1),
    ERR_START_DHCPSERVICE_FAILED = (-2),
    ERR_STOP_DHCPSERVICE_FAILED = (-3),
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETD_CONTROLLER_DEFINE_H
