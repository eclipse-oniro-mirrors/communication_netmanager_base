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
#ifndef NET_CONN_ASYNC_H
#define NET_CONN_ASYNC_H

#include "scheduler.h"
#include "net_link_info.h"
#include "net_conn_constants.h"

namespace OHOS {
namespace NetManagerStandard {
class NetConnAsync {
public:
    NetConnAsync();
    
    virtual ~NetConnAsync();

    Scheduler& GetScheduler();

    void CallbackOnNetAvailableChanged(uint32_t supplierId, bool available);
    
    void CallbackOnNetCapabilitiesChanged(uint32_t supplierId, const NetAllCapabilities &allCaps);

    void CallbackOnNetLinkInfoChanged(uint32_t supplierId, const NetLinkInfo &linkInfo);

    void CallbackOnNetDetectionResultChanged(
        uint32_t netId, NetDetectionResultCode detectionResult, const std::string &urlRedirect);

    void CallbackOnNetScoreChanged(uint32_t supplierId, uint32_t score);
    
protected:
    virtual void OnNetAvailableChanged(uint32_t supplierId, bool available) = 0;
    
    virtual void OnNetCapabilitiesChanged(uint32_t supplierId, const NetAllCapabilities &allCaps) = 0;

    virtual void OnNetLinkInfoChanged(uint32_t supplierId, const NetLinkInfo &linkInfo) = 0;

    virtual void OnNetDetectionResultChanged(
        uint32_t netId, NetDetectionResultCode detectionResult, const std::string &urlRedirect) = 0;

    virtual void OnNetScoreChanged(uint32_t supplierId, uint32_t score) = 0;
private:
    Scheduler async_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_ASYNC_H
