/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef NET_BUNDLE_IMPL_H
#define NET_BUNDLE_IMPL_H

#include "net_bundle.h"

namespace OHOS {
namespace NetManagerStandard {
class NetBundleImpl : public INetBundle {
public:
    int32_t GetJsonFromBundle(std::string &jsonProfile) override;
    bool IsAtomicService(std::string &bundleName) override;
};
extern "C" __attribute__((visibility("default"))) INetBundle *GetNetBundle();
extern "C" __attribute__((visibility("default"))) bool IsAtomicService(std::string &bundleName);
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_BUNDLE_IMPL_H