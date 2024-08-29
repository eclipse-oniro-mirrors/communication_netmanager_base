/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef NETSYS_VPN_WRAPPER_H
#define NETSYS_VPN_WRAPPER_H

#include <cstring>
#include "ffrt.h"
#include "i_netsys_service.h"

#define IPSEC_PIDDIR "/data/service/el1/public/vpn"

namespace OHOS {
namespace nmd {
using namespace NetsysNative;
class NetSysVpnWrapper : public std::enable_shared_from_this<NetSysVpnWrapper> {
public:
    NetSysVpnWrapper();
    ~NetSysVpnWrapper();
    static std::shared_ptr<NetSysVpnWrapper> &GetInstance()
    {
        static std::shared_ptr<NetSysVpnWrapper> instance = std::make_shared<NetSysVpnWrapper>();
        return instance;
    }

    /**
     * @param param update vpn param
     * @return NETMANAGER_SUCCESS suceess or NETMANAGER_ERROR failed
     */
    int32_t Update(SysVpnStageCode stage);

private:
    void ExecuteUpdate(SysVpnStageCode stage);

private:
    static constexpr const char *VPN_STAGE_RESTART = "restart";
    static constexpr const char *VPN_STAGE_SWANCTL_LOAD = "swanctl --load-all --file ";
    static constexpr const char *VPN_STAGE_UP_HOME = "up home";
    static constexpr const char *VPN_STAGE_DOWN_HOME = "down home";
    static constexpr const char *VPN_STAGE_STOP = "stop";
    static constexpr const char *VPN_STAGE_L2TP_LOAD = "xl2tpd -c ";
    static constexpr const char *VPN_STAGE_L2TP_CTL = "l2tpctl";
    static constexpr const char *IPSEC_L2TP_CTL = " -C " IPSEC_PIDDIR "/l2tp-control";
    static constexpr const char *SWAN_CTL_FILE = IPSEC_PIDDIR "/swanctl.conf";
    static constexpr const char *L2TP_CFG = IPSEC_PIDDIR "/xl2tpd.conf";
    bool isIpSecAccess_ = false;
    std::shared_ptr<ffrt::queue> vpnFfrtQueue_ = nullptr;
};
} // namespace nmd
} // namespace OHOS
#endif /* NETSYS_VPN_WRAPPER_H */
