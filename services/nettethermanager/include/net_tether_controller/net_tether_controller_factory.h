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

#ifndef NET_TETHER_CONTROLLER_FACTORY_H
#define NET_TETHER_CONTROLLER_FACTORY_H

#include <unordered_map>
#include <string>
#include <singleton.h>

#include "net_tether_controller.h"

namespace OHOS {
namespace NetManagerStandard {
struct NetTetherControllerConfig {
    uint32_t netAbilityId_;
    std::unordered_map<std::string, uint32_t> callCode_;

    NetTetherControllerConfig() : netAbilityId_(0)
    {}
};

class NetTetherControllerFactory {
    DECLARE_DELAYED_SINGLETON(NetTetherControllerFactory)
public:
    sptr<NetTetherController> MakeNetTetherController(uint32_t netType);
    void RemoveNetTetherController(uint32_t netType);

private:
    bool InitNetTetherControllerConfig(void);
    sptr<NetTetherController> GetNetTetherControllerFromMap(uint32_t netType);

private:
    std::unordered_map<uint32_t, sptr<NetTetherController>> netTetherControllers_;
    std::unordered_map<uint32_t, NetTetherControllerConfig> netTetherControllerConfs_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_TETHER_CONTROLLER_FACTORY_H
