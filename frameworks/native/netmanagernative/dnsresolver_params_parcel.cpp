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

#include "netnative_log_wrapper.h"

#include "dnsresolver_params_parcel.h"

namespace OHOS {
namespace NetsysNative {
DnsResolverParamsParcel::DnsResolverParamsParcel() : netId_(0), baseTimeoutMsec_(0), retryCount_(0) {}

bool DnsResolverParamsParcel::Marshalling(Parcel &parcel) const
{
    parcel.WriteUint16(netId_);
    parcel.WriteUint16(baseTimeoutMsec_);
    parcel.WriteUint8(retryCount_);

    uint16_t netId_t;
    uint16_t baseTimeoutMsec_t;
    uint8_t retryCount_t;
    if (!parcel.ReadUint16(netId_t)) {
        return false;
    }
    if (!parcel.ReadUint16(baseTimeoutMsec_t)) {
        return false;
    }
    if (!parcel.ReadUint8(retryCount_t)) {
        return false;
    }

    NETNATIVE_LOGI("DnsResolverParamsParcel::Marshalling %{public}d and %{public}d", retryCount_, retryCount_t);
    return true;
}

sptr<DnsResolverParamsParcel> DnsResolverParamsParcel::Unmarshalling(Parcel &parcel)
{
    sptr<DnsResolverParamsParcel> ptr = new (std::nothrow) DnsResolverParamsParcel();
    if (ptr == nullptr) {
        NETNATIVE_LOGE("The parameter of ptr is nullptr");
        return nullptr;
    }

    if (!parcel.ReadUint16(ptr->netId_)) {
        return nullptr;
    }
    if (!parcel.ReadUint16(ptr->baseTimeoutMsec_)) {
        return nullptr;
    }
    if (!parcel.ReadUint8(ptr->retryCount_)) {
        return nullptr;
    }
    NETNATIVE_LOGI("DnsResolverParamsParcel::Unmarshalling %{public}d", ptr->retryCount_);
    return ptr;
}
} // namespace NetsysNative
} // namespace OHOS
