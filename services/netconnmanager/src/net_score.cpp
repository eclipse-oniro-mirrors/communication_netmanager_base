/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "net_score.h"

namespace OHOS {
namespace NetManagerStandard {
bool NetScore::GetServiceScore(sptr<NetSupplier> &supplier)
{
    if (supplier == nullptr) {
        NETMGR_LOG_E("the input supplier is nullptr");
        return false;
    }

    NetBearType bearerType = supplier->GetNetSupplierType();
    int32_t netScore = 0;
    NetTypeScore::iterator iter = netTypeScore_.find(bearerType);
    if (iter == netTypeScore_.end()) {
        NETMGR_LOG_E("Can not find net bearer type[%{public}d] for this net service", bearerType);
        return false;
    }
    NETMGR_LOG_D("Net type[%{public}d],default score[%{public}d]",
        static_cast<int32_t>(iter->first), static_cast<int32_t>(iter->second));
    netScore = static_cast<int32_t>(iter->second);
    supplier->SetNetScore(netScore);
    if (!(supplier->IsNetValidated())) {
        netScore -= NET_VALID_SCORE;
    }
    if (supplier->IsNetQualityPoor()) {
        netScore -= DIFF_SCORE_BETWEEN_GOOD_POOR;
        NETMGR_LOG_I("supplier net quality poor, supplierId[%{public}d], bearerType[%{public}d], score[%{public}d]",
            supplier->GetSupplierId(), bearerType, netScore);
    }
    if (supplier->IsNetQualityGood()) {
        netScore += DIFF_SCORE_BETWEEN_GOOD_POOR;
        NETMGR_LOG_I("supplier net quality good, supplierId[%{public}d], bearerType[%{public}d], score[%{public}d]",
            supplier->GetSupplierId(), bearerType, netScore);
    }
    supplier->ResetNetQuality();
    supplier->SetRealScore(netScore);
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
