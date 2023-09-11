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
#ifndef I_NET_DIAG_CALLBACK_H
#define I_NET_DIAG_CALLBACK_H

#include <string>

#include "iremote_broker.h"
#include "netsys_net_diag_data.h"

namespace OHOS {
namespace NetsysNative {
class INetDiagCallback : public IRemoteBroker {
public:
    virtual ~INetDiagCallback() = default;

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetsysNative.INetDiagCallback");

public:
    virtual int32_t OnNotifyPingResult(const NetDiagPingResult &pingResult) = 0;
};
} // namespace NetsysNative
} // namespace OHOS
#endif // I_NOTIFY_CALLBACK_H
