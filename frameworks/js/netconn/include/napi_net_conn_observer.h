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

#ifndef NAPI_NETCONN_OBSERVER_H
#define NAPI_NETCONN_OBSERVER_H

#include "net_conn_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class NapiNetConnObserver : public NetConnCallbackStub {
public:
    int32_t NetConnStateChanged(const sptr<NetConnCallbackInfo> &info);
    int32_t NetAvailable(int32_t netId);
    int32_t NetCapabilitiesChange(int32_t netId, const uint64_t &netCap);
    int32_t NetConnectionPropertiesChange(int32_t netId, const sptr<NetLinkInfo> &info);
    int32_t NetLost(int32_t netId);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NAPI_NETCONN_OBSERVER_H
