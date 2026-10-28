/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef COMMON_NETSYS_CONTROLLER_MOCK_GUARD_H
#define COMMON_NETSYS_CONTROLLER_MOCK_GUARD_H

#include <initializer_list>
#include <string>

#include "net_manager_constants.h"
#include "netsys_controller.h"
#include "netsys_controller_service_impl.h"

namespace OHOS {
namespace NetManagerStandard {

/*
 * Mock api names for NetsysControllerTestStub below. These controller apis have no
 * in-process mock branch in the production NetsysControllerServiceImpl (unlike the
 * legacy apis registered by MockNetsysNativeClient::RegisterMockApi), so the stub
 * overrides them for unit tests instead.
 */
constexpr const char *MOCK_SETNETWORKACCESSPOLICY_API = "SetNetworkAccessPolicy";
constexpr const char *MOCK_DELETENETWORKACCESSPOLICY_API = "DeleteNetworkAccessPolicy";
constexpr const char *MOCK_NOTIFYNETBEARERTYPECHANGE_API = "NotifyNetBearerTypeChange";
constexpr const char *MOCK_SETBROKERUIDACCESSPOLICYMAP_API = "SetBrokerUidAccessPolicyMap";
constexpr const char *MOCK_DELBROKERUIDACCESSPOLICYMAP_API = "DelBrokerUidAccessPolicyMap";
constexpr const char *MOCK_CLEARSIMSTATSBPFMAP_API = "ClearSimStatsBpfMap";
constexpr const char *MOCK_SETNETSTATETRAFFICMAP_API = "SetNetStateTrafficMap";
constexpr const char *MOCK_GETNETSTATETRAFFICMAP_API = "GetNetStateTrafficMap";
constexpr const char *MOCK_CLEARINCREASETRAFFICMAP_API = "ClearIncreaseTrafficMap";
constexpr const char *MOCK_UPDATEIFINDEXMAP_API = "UpdateIfIndexMap";
constexpr const char *MOCK_GETALLSIMSTATSINFO_API = "GetAllSimStatsInfo";
constexpr const char *MOCK_GETTOTALSTATS_API = "GetTotalStats";
constexpr const char *MOCK_GETUIDSTATS_API = "GetUidStats";
constexpr const char *MOCK_GETALLSTATSINFO_API = "GetAllStatsInfo";
constexpr const char *MOCK_DELETESTATSINFO_API = "DeleteStatsInfo";
constexpr const char *MOCK_DELETESIMSTATSINFO_API = "DeleteSimStatsInfo";
constexpr const char *MOCK_BANDWIDTHENABLEDATASAVER_API = "BandwidthEnableDataSaver";
constexpr const char *MOCK_BANDWIDTHSETIFACEQUOTA_API = "BandwidthSetIfaceQuota";
constexpr const char *MOCK_BANDWIDTHREMOVEIFACEQUOTA_API = "BandwidthRemoveIfaceQuota";
constexpr const char *MOCK_BANDWIDTHADDDENIEDLIST_API = "BandwidthAddDeniedList";
constexpr const char *MOCK_BANDWIDTHREMOVEDENIEDLIST_API = "BandwidthRemoveDeniedList";
constexpr const char *MOCK_BANDWIDTHADDALLOWEDLIST_API = "BandwidthAddAllowedList";
constexpr const char *MOCK_BANDWIDTHREMOVEALLOWEDLIST_API = "BandwidthRemoveAllowedList";
constexpr const char *MOCK_FIREWALLSETUIDSALLOWEDLISTCHAIN_API = "FirewallSetUidsAllowedListChain";
constexpr const char *MOCK_FIREWALLSETUIDSDENIEDLISTCHAIN_API = "FirewallSetUidsDeniedListChain";
constexpr const char *MOCK_FIREWALLENABLECHAIN_API = "FirewallEnableChain";
constexpr const char *MOCK_FIREWALLSETUIDRULE_API = "FirewallSetUidRule";
constexpr const char *MOCK_SETNICTRAFFICALLOWED_API = "SetNicTrafficAllowed";
constexpr const char *MOCK_ADDSTATICIPV6ADDR_API = "AddStaticIpv6Addr";
constexpr const char *MOCK_DELSTATICIPV6ADDR_API = "DelStaticIpv6Addr";

/**
 * @brief Test-only stub of the production NetsysControllerServiceImpl. It short-circuits
 *        the apis listed above to NETMANAGER_SUCCESS when the api name is present in
 *        mockNetsysClient_.mockApi_ -- the SAME set that drives the in-process mock
 *        branches of the production implementation -- so tests keep managing every mock
 *        (legacy and stub) through the single mockApi_ set they already handle. When the
 *        name is absent the call is forwarded to the real implementation unchanged.
 * @note  The overrides read the private member mockNetsysClient_ of the base class, so
 *        this header MUST be included while `private` is (re)defined as `public`
 *        (see the #define private public blocks in the including test files), and
 *        netsys_controller_service_impl.h must not have been included earlier without
 *        that define.
 */
class NetsysControllerTestStub : public NetsysControllerServiceImpl {
public:
    int32_t SetNetworkAccessPolicy(uint32_t uid, NetworkAccessPolicy policy, bool reconfirmFlag) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_SETNETWORKACCESSPOLICY_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::SetNetworkAccessPolicy(uid, policy, reconfirmFlag);
    }

    int32_t DeleteNetworkAccessPolicy(uint32_t uid) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_DELETENETWORKACCESSPOLICY_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::DeleteNetworkAccessPolicy(uid);
    }

    int32_t NotifyNetBearerTypeChange(std::set<NetBearType> bearerTypes) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_NOTIFYNETBEARERTYPECHANGE_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::NotifyNetBearerTypeChange(bearerTypes);
    }

    int32_t SetBrokerUidAccessPolicyMap(const std::unordered_map<uint32_t, uint32_t> &uidMaps) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_SETBROKERUIDACCESSPOLICYMAP_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::SetBrokerUidAccessPolicyMap(uidMaps);
    }

    int32_t DelBrokerUidAccessPolicyMap(uint32_t uid) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_DELBROKERUIDACCESSPOLICYMAP_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::DelBrokerUidAccessPolicyMap(uid);
    }

    int32_t ClearSimStatsBpfMap() override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_CLEARSIMSTATSBPFMAP_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::ClearSimStatsBpfMap();
    }

    int32_t SetNetStateTrafficMap(uint8_t flag, uint64_t availableTraffic) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_SETNETSTATETRAFFICMAP_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::SetNetStateTrafficMap(flag, availableTraffic);
    }

    int32_t GetNetStateTrafficMap(uint8_t flag, uint64_t &availableTraffic) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_GETNETSTATETRAFFICMAP_API)) {
            availableTraffic = 0;
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::GetNetStateTrafficMap(flag, availableTraffic);
    }

    int32_t ClearIncreaseTrafficMap() override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_CLEARINCREASETRAFFICMAP_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::ClearIncreaseTrafficMap();
    }

    int32_t UpdateIfIndexMap(int8_t key, uint64_t index) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_UPDATEIFINDEXMAP_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::UpdateIfIndexMap(key, index);
    }

    int32_t GetAllSimStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_GETALLSIMSTATSINFO_API)) {
            stats.clear();
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::GetAllSimStatsInfo(stats);
    }

    int32_t GetTotalStats(uint64_t &stats, uint32_t type) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_GETTOTALSTATS_API)) {
            stats = 0;
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::GetTotalStats(stats, type);
    }

    int32_t GetUidStats(uint64_t &stats, uint32_t type, uint32_t uid) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_GETUIDSTATS_API)) {
            stats = 0;
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::GetUidStats(stats, type, uid);
    }

    int32_t GetAllStatsInfo(std::vector<OHOS::NetManagerStandard::NetStatsInfo> &stats) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_GETALLSTATSINFO_API)) {
            stats.clear();
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::GetAllStatsInfo(stats);
    }

    int32_t DeleteStatsInfo(uint32_t uid) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_DELETESTATSINFO_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::DeleteStatsInfo(uid);
    }

    int32_t DeleteSimStatsInfo(uint32_t uid) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_DELETESIMSTATSINFO_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::DeleteSimStatsInfo(uid);
    }

    int32_t BandwidthEnableDataSaver(bool enable) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_BANDWIDTHENABLEDATASAVER_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::BandwidthEnableDataSaver(enable);
    }

    int32_t BandwidthSetIfaceQuota(const std::string &ifName, int64_t bytes) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_BANDWIDTHSETIFACEQUOTA_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::BandwidthSetIfaceQuota(ifName, bytes);
    }

    int32_t BandwidthRemoveIfaceQuota(const std::string &ifName) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_BANDWIDTHREMOVEIFACEQUOTA_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::BandwidthRemoveIfaceQuota(ifName);
    }

    int32_t BandwidthAddDeniedList(uint32_t uid) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_BANDWIDTHADDDENIEDLIST_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::BandwidthAddDeniedList(uid);
    }

    int32_t BandwidthRemoveDeniedList(uint32_t uid) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_BANDWIDTHREMOVEDENIEDLIST_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::BandwidthRemoveDeniedList(uid);
    }

    int32_t BandwidthAddAllowedList(uint32_t uid) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_BANDWIDTHADDALLOWEDLIST_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::BandwidthAddAllowedList(uid);
    }

    int32_t BandwidthRemoveAllowedList(uint32_t uid) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_BANDWIDTHREMOVEALLOWEDLIST_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::BandwidthRemoveAllowedList(uid);
    }

    int32_t FirewallSetUidsAllowedListChain(uint32_t chain, const std::vector<uint32_t> &uids) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_FIREWALLSETUIDSALLOWEDLISTCHAIN_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::FirewallSetUidsAllowedListChain(chain, uids);
    }

    int32_t FirewallSetUidsDeniedListChain(uint32_t chain, const std::vector<uint32_t> &uids) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_FIREWALLSETUIDSDENIEDLISTCHAIN_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::FirewallSetUidsDeniedListChain(chain, uids);
    }

    int32_t FirewallEnableChain(uint32_t chain, bool enable) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_FIREWALLENABLECHAIN_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::FirewallEnableChain(chain, enable);
    }

    int32_t FirewallSetUidRule(uint32_t chain, const std::vector<uint32_t> &uids, uint32_t firewallRule) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_FIREWALLSETUIDRULE_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::FirewallSetUidRule(chain, uids, firewallRule);
    }

    int32_t SetNicTrafficAllowed(const std::vector<std::string> &ifaceNames, bool status) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_SETNICTRAFFICALLOWED_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::SetNicTrafficAllowed(ifaceNames, status);
    }

    int32_t AddStaticIpv6Addr(const std::string &ipv6Addr, const std::string &macAddr,
                              const std::string &ifName) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_ADDSTATICIPV6ADDR_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::AddStaticIpv6Addr(ipv6Addr, macAddr, ifName);
    }

    int32_t DelStaticIpv6Addr(const std::string &ipv6Addr, const std::string &macAddr,
                              const std::string &ifName) override
    {
        if (mockNetsysClient_.CheckMockApi(MOCK_DELSTATICIPV6ADDR_API)) {
            return NETMANAGER_SUCCESS;
        }
        return NetsysControllerServiceImpl::DelStaticIpv6Addr(ipv6Addr, macAddr, ifName);
    }
};

/**
 * @brief RAII guard that temporarily replaces NetsysController's service implementation
 *        with a mock-enabled NetsysControllerTestStub, so tests never perform real IPC to
 *        the netsysnative service (SA 1158) and do not depend on the device kernel/BPF
 *        environment. The stub short-circuits exactly the apis whose names are passed
 *        here; every other call is forwarded to the real implementation.
 * @note  This header reads private members of NetsysController / NetsysControllerServiceImpl
 *        / NetsysControllerTestStub, so it MUST be included while `private` is (re)defined
 *        as `public` (see the #define private public blocks in the including test files),
 *        and netsys_controller.h must not have been included earlier without that define.
 */
class NetsysControllerMockGuard {
public:
    explicit NetsysControllerMockGuard(std::initializer_list<const std::string> mockApis)
    {
        originalService_ = NetsysController::GetInstance().netsysService_;
        // Use a pointer with the concrete static type here: member lookup happens on the
        // static type, and the sptr<INetsysControllerService> member below cannot see
        // NetsysControllerTestStub::mockNetsysClient_ even with `private` redefined.
        NetsysControllerTestStub *stub = new NetsysControllerTestStub();
        for (const auto &api : mockApis) {
            stub->mockNetsysClient_.mockApi_.insert(api);
        }
        mockServiceImpl_ = stub;
        NetsysController::GetInstance().netsysService_ = mockServiceImpl_;
    }

    ~NetsysControllerMockGuard()
    {
        NetsysController::GetInstance().netsysService_ = originalService_;
    }

private:
    sptr<INetsysControllerService> originalService_;
    sptr<INetsysControllerService> mockServiceImpl_;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // COMMON_NETSYS_CONTROLLER_MOCK_GUARD_H
