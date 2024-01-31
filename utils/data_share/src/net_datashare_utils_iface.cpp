/*
 * Copyright (C) 2023-2024 Huawei Device Co., Ltd.
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

#include "net_datashare_utils_iface.h"
#include "net_datashare_utils.h"

namespace OHOS {
namespace NetManagerStandard {

int32_t NetDataShareHelperUtilsIface::Query(const std::string &strUri, const std::string &key, std::string &value)
{
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri uri(strUri);
    int32_t ret = dataShareHelperUtils->Query(uri, key, value);
    return ret;
}

int32_t NetDataShareHelperUtilsIface::Insert(const std::string &strUri, const std::string &key,
                                             const std::string &value)
{
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri uri(strUri);
    int32_t ret = dataShareHelperUtils->Insert(uri, key, value);
    return ret;
}

int32_t NetDataShareHelperUtilsIface::Update(const std::string &strUri, const std::string &key,
                                             const std::string &value)
{
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri uri(strUri);
    int32_t ret = dataShareHelperUtils->Update(uri, key, value);
    return ret;
}
} // namespace NetManagerStandard
} // namespace OHOS