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

#include "net_tether_callback_test.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetTetherCallbackTest::NetTetherCallbackTest() {}

NetTetherCallbackTest::~NetTetherCallbackTest() {}

int32_t NetTetherCallbackTest::TetherSuccess(int32_t tetherType, const std::string &ifName)
{
    NETMGR_LOG_I("NetTetherCallbackTest::TetherSuccess(), tetherType:[%{public}d], ifName:[%{public}s]",
        tetherType, ifName.c_str());
    return 0;
}

int32_t NetTetherCallbackTest::TetherFailed(int32_t tetherType, const std::string& ifName, int32_t failCode)
{
    NETMGR_LOG_I("NetTetherCallbackTest::TetherFailed()");
    NETMGR_LOG_I("tetherType:[%{public}d], ifName:[%{public}s],failCode:[%{public}d]",
        tetherType, ifName.c_str(), failCode);
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS
