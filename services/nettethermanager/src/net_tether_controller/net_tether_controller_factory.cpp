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

#include "net_tether_controller_factory.h"
#include "net_mgr_log_wrapper.h"
#include "net_tether_define.h"

namespace OHOS {
namespace NetManagerStandard {
static std::vector<std::string> SplitString(const std::string &str, char c)
{
    std::vector<std::string> ret;
    std::string::size_type beg = 0;
    std::string::size_type end = 0;
    while ((end = str.find(c, beg)) != std::string::npos) {
        ret.push_back(str.substr(beg, end - beg));
        beg = end + 1;
    }
    if (beg < str.length()) {
        ret.push_back(str.substr(beg));
    }
    return ret;
}

NetTetherControllerFactory::NetTetherControllerFactory()
{
    InitNetTetherControllerConfig();
}

NetTetherControllerFactory::~NetTetherControllerFactory() {}

bool NetTetherControllerFactory::InitNetTetherControllerConfig(void)
{
    uint32_t size = sizeof(NETTETHERWORKTYPE_CONF_STR) / sizeof(NETTETHERWORKTYPE_CONF_STR[0]);
    for (uint32_t i = 0; i < size; ++i) {
        const std::string &tmp = NETTETHERWORKTYPE_CONF_STR[i];
        if (tmp.empty()) {
            continue;
        }
        std::vector<std::string> ret = SplitString(tmp, ',');
        NetTetherControllerConfig conf;
        size_t pos = 0;
        if (ret.size() > pos) {
            conf.netAbilityId_ = atoi(ret[pos].c_str());
        }
        ++pos;
        if (ret.size() > pos && ret[pos] != "-1") {
            conf.callCode_.insert(std::make_pair("OpenTether", atoi(ret[pos].c_str())));
        }
        ++pos;
        if (ret.size() > pos && ret[pos] != "-1") {
            conf.callCode_.insert(std::make_pair("CloseTether", atoi(ret[pos].c_str())));
        }
        netTetherControllerConfs_.insert(std::make_pair(conf.netAbilityId_, conf));
    }
    return true;
}

sptr<NetTetherController> NetTetherControllerFactory::MakeNetTetherController(uint32_t netType)
{
    NETMGR_LOG_D("make controller netType[%{public}u]", netType);
    sptr<NetTetherController> netTetherController = GetNetTetherControllerFromMap(netType);
    if (netTetherController != nullptr) {
        return netTetherController;
    }
    NETMGR_LOG_D("factory need create netTetherController");
    auto iter = netTetherControllerConfs_.find(netType);
    if (iter == netTetherControllerConfs_.end()) {
        NETMGR_LOG_E("Not find netType[%{public}u] config", netType);
        return nullptr;
    }

    netTetherController = (std::make_unique<NetTetherController>(iter->second.netAbilityId_)).release();
    if (!netTetherController->Init(iter->second.callCode_)) {
        NETMGR_LOG_E("factory create nettethercontroller failed, may be the network's sa not started!");
        return nullptr;
    }
    netTetherControllers_.insert(std::make_pair(netType, netTetherController));
    return netTetherController;
}

sptr<NetTetherController> NetTetherControllerFactory::GetNetTetherControllerFromMap(uint32_t netType)
{
    auto it = netTetherControllers_.find(netType);
    if (it != netTetherControllers_.end()) {
        return it->second;
    }
    NETMGR_LOG_D("NetTetherController* is not found, return null");
    return nullptr;
}

void NetTetherControllerFactory::RemoveNetTetherController(uint32_t netType)
{
    auto it = netTetherControllers_.find(netType);
    if (it != netTetherControllers_.end()) {
        netTetherControllers_.erase(it);
    }
    return;
}
} // namespace NetManagerStandard
} // namespace OHOS