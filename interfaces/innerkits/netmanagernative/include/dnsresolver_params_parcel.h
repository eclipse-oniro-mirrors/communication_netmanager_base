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
#ifndef  DSNRESOLVER_PARAMS_PARCEL_H__
#define  DSNRESOLVER_PARAMS_PARCEL_H__

#include "parcel.h"
#include "dnsresolv.h"

namespace OHOS {
namespace NetdNative {
using namespace nmd;
struct DnsresolverParamsParcel : public Parcelable {
    DnsresolverParamsParcel();
    DnsresolverParamsParcel(const dnsresolver_params &resolvParams);
    ~DnsresolverParamsParcel() {}

    uint16_t netId_;
    uint16_t baseTimeoutMsec_;
    uint8_t retryCount_;

    bool Marshalling(Parcel &parcel) const override;
    static sptr<DnsresolverParamsParcel> Unmarshalling(Parcel &parcel);
};
} // namespace NetdNative
} // namespace OHOS
#endif