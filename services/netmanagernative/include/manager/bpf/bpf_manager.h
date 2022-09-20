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

#ifndef BPF_MANAGER_H
#define BPF_MANAGER_H

#include "netlink_manager.h"

namespace OHOS {
namespace nmd {
class BpfManager {
    class IfacelistNotifyCallback : public NetsysNative::INotifyCallback {
    public:
        IfacelistNotifyCallback() = default;
        sptr<IRemoteObject> AsObject() override;
        int32_t OnInterfaceAddressUpdated(const std::string &addr, const std::string &ifName, int flags,
                                          int scope) override;
        int32_t OnInterfaceAddressRemoved(const std::string &addr, const std::string &ifName, int flags,
                                          int scope) override;
        int32_t OnInterfaceAdded(const std::string &ifName) override;
        int32_t OnInterfaceRemoved(const std::string &ifName) override;
        int32_t OnInterfaceChanged(const std::string &ifName, bool up) override;
        int32_t OnInterfaceLinkStateChanged(const std::string &ifName, bool up) override;
        int32_t OnRouteChanged(bool updated, const std::string &route, const std::string &gateway,
                               const std::string &ifName) override;
        int32_t OnDhcpSuccess(sptr<NetsysNative::DhcpResultParcel> &dhcpResult) override;
        int32_t OnBandwidthReachedLimit(const std::string &limitName, const std::string &iface) override;
    };

public:
    /**
     * Construct a new Bpf Manager object.
     *
     */
    BpfManager() = default;

    /**
     * Initialize Bpf Manager.
     *
     * @return Returns true on success, false on failure.
     */
    bool Init() const;

private:
    /**
     * Modify the permission of maps.
     *
     * @return Returns true on success, false on failure.
     */
    bool ModifyMapPermission() const;

    /**
     * Create iptables Chain
     *
     * @return Returns true on success, false on failure.
     */
    bool CreateIptablesChain() const;
};
} // namespace nmd
} // namespace OHOS
#endif // BPF_MANAGER_H
