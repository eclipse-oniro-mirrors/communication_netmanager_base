/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "net_mgr_log_wrapper.h"
#include "net_conn_async.h"

namespace OHOS {
namespace NetManagerStandard {
NetConnAsync::NetConnAsync() {}
    
NetConnAsync::~NetConnAsync() {}

Scheduler& NetConnAsync::GetScheduler()
{
    return async_;
}

void NetConnAsync::CallbackOnNetAvailableChanged(uint32_t supplierId, bool available)
{
    async_.Post(std::bind(&NetConnAsync::OnNetAvailableChanged, this, supplierId, available));
}

void NetConnAsync::CallbackOnNetCapabilitiesChanged(uint32_t supplierId, const NetAllCapabilities &allCaps)
{
    async_.Post(std::bind(&NetConnAsync::OnNetCapabilitiesChanged, this, supplierId, allCaps));
}

void NetConnAsync::CallbackOnNetLinkInfoChanged(uint32_t supplierId, const NetLinkInfo &linkInfo)
{
    async_.Post(std::bind(&NetConnAsync::OnNetLinkInfoChanged, this, supplierId, linkInfo));
}

void NetConnAsync::CallbackOnNetDetectionResultChanged(
    uint32_t netId, NetDetectionResultCode detectionResult, const std::string &urlRedirect)
{
    async_.Post(std::bind(&NetConnAsync::OnNetDetectionResultChanged, this, netId, detectionResult, urlRedirect));
}

void NetConnAsync::CallbackOnNetScoreChanged(uint32_t supplierId, uint32_t score)
{
    async_.Post(std::bind(&NetConnAsync::OnNetScoreChanged, this, supplierId, score));
}
}
}
