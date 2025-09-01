/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include <atomic>
#include <charconv>
#include <fstream>
#include <functional>
#include <memory>
#include <sys/time.h>
#include <utility>
#include <regex>
#include <condition_variable>

#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "netmanager_base_common_utils.h"
#include "network.h"
#include "system_ability_definition.h"
#include "want.h"

#include "broadcast_manager.h"
#include "event_report.h"
#include "net_activate.h"
#include "net_conn_service.h"
#include "net_conn_callback_proxy_wrapper.h"
#include "net_conn_types.h"
#include "net_policy_client.h"
#include "net_datashare_utils.h"
#include "net_http_proxy_tracker.h"
#include "net_manager_center.h"
#include "net_manager_constants.h"
#include "net_mgr_log_wrapper.h"
#include "net_supplier.h"
#include "netmanager_base_permission.h"
#include "netsys_controller.h"
#include "ipc_skeleton.h"
#include "parameter.h"
#include "parameters.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr uint32_t MAX_ALLOW_UID_NUM = 2000;
constexpr uint32_t PROXY_INIT_DELAY_TIME = 2000;
constexpr uint32_t INVALID_SUPPLIER_ID = 0;
constexpr char SIGNAL_LEVEL_3 = 3;
// hisysevent error messgae
constexpr const char *ERROR_MSG_NULL_SUPPLIER_INFO = "Net supplier info is nullptr";
constexpr const char *ERROR_MSG_NULL_NET_LINK_INFO = "Net link info is nullptr";
constexpr const char *ERROR_MSG_NULL_NET_SPECIFIER = "The parameter of netSpecifier or callback is null";
constexpr const char *ERROR_MSG_CAN_NOT_FIND_SUPPLIER = "Can not find supplier by id:";
constexpr const char *ERROR_MSG_UPDATE_NETLINK_INFO_FAILED = "Update net link info failed";
constexpr const char *ERROR_MSG_UPDATE_ERROR_UID = "Update net link info by error uid";
constexpr const char *NET_CONN_MANAGER_WORK_THREAD = "NET_CONN_MANAGER_WORK_THREAD";
constexpr const char *URL_CFG_FILE = "/system/etc/netdetectionurl.conf";
constexpr const char *HTTP_URL_HEADER = "HttpProbeUrl:";
constexpr const char NEW_LINE_STR = '\n';
const uint32_t SYS_PARAMETER_SIZE = 256;
constexpr const char *CFG_NETWORK_PRE_AIRPLANE_MODE_WAIT_TIMES = "persist.network.pre_airplane_mode_wait_times";
constexpr const char *NO_DELAY_TIME_CONFIG = "100";
constexpr const char *SETTINGS_DATASHARE_URI =
        "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
constexpr const char *SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
constexpr uint32_t INPUT_VALUE_LENGTH = 10;
constexpr uint32_t MAX_DELAY_TIME = 200;
constexpr uint16_t DEFAULT_MTU = 1500;
constexpr int32_t SUCCESS_CODE = 204;
constexpr int32_t RETRY_TIMES = 3;
constexpr long AUTH_TIME_OUT = 5L;
constexpr uint32_t MAX_NET_EXT_ATTRIBUTE = 10240;
constexpr const char *BOOTEVENT_NETMANAGER_SERVICE_READY = "bootevent.netmanager.ready";
constexpr const char *BOOTEVENT_NETSYSNATIVE_SERVICE_READY = "bootevent.netsysnative.ready";
constexpr const char *PERSIST_EDM_MMS_DISABLE = "persist.edm.mms_disable";
constexpr const char *PERSIST_EDM_AIRPLANE_MODE_DISABLE = "persist.edm.airplane_mode_disable";
constexpr const char *PERSIST_WIFI_DELAY_ELEVATOR_ENABLE = "persist.booster.enable_wifi_delay_elevator";
constexpr const char *PERSIST_WIFI_DELAY_WEAK_SIGNAL_ENABLE = "persist.booster.enable_wifi_delay_weak_signal";
} // namespace

const bool REGISTER_LOCAL_RESULT =
    SystemAbility::MakeAndRegisterAbility(NetConnService::GetInstance().get());

NetConnService::NetConnService()
    : SystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, true), registerToService_(false), state_(STATE_STOPPED)
{
}

NetConnService::~NetConnService()
{
    RemoveALLClientDeathRecipient();
}

void NetConnService::OnStart()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    NETMGR_LOG_D("OnStart begin");
    if (state_ == STATE_RUNNING) {
        NETMGR_LOG_D("the state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_LOG_E("init failed");
        return;
    }
    state_ = STATE_RUNNING;
    gettimeofday(&tv, nullptr);
    if (!system::GetBoolParameter(BOOTEVENT_NETMANAGER_SERVICE_READY, false)) {
        system::SetParameter(BOOTEVENT_NETMANAGER_SERVICE_READY, "true");
        NETMGR_LOG_I("set netmanager service start true");
    }
    system::SetParameter(PERSIST_WIFI_DELAY_ELEVATOR_ENABLE, "false");
    system::SetParameter(PERSIST_WIFI_DELAY_WEAK_SIGNAL_ENABLE, "false");
    NETMGR_LOG_D("OnStart end");
}

void NetConnService::CreateDefaultRequest()
{
    if (!defaultNetActivate_) {
        defaultNetSpecifier_ = (std::make_unique<NetSpecifier>()).release();
        defaultNetSpecifier_->SetCapabilities({NET_CAPABILITY_INTERNET, NET_CAPABILITY_NOT_VPN});
        std::weak_ptr<INetActivateCallback> timeoutCb;
        defaultNetActivate_ = std::make_shared<NetActivate>(defaultNetSpecifier_, nullptr, timeoutCb, 0,
                                                            netConnEventHandler_, 0, REQUEST);
        defaultNetActivate_->StartTimeOutNetAvailable();
        defaultNetActivate_->SetRequestId(DEFAULT_REQUEST_ID);
        {
            std::unique_lock<std::shared_mutex> lock(netActivatesMutex_);
            netActivates_[DEFAULT_REQUEST_ID] = defaultNetActivate_;
        }
        NETMGR_LOG_D("defaultnetcap size = [%{public}zu]", defaultNetSpecifier_->netCapabilities_.netCaps_.size());
    }
}

void NetConnService::OnStop()
{
    NETMGR_LOG_D("OnStop begin");
    if (netConnEventRunner_) {
        netConnEventRunner_->Stop();
        netConnEventRunner_.reset();
    }
    if (netConnEventHandler_) {
        netConnEventHandler_.reset();
    }
    state_ = STATE_STOPPED;
    registerToService_ = false;
    NETMGR_LOG_D("OnStop end");
}

bool NetConnService::Init()
{
    if (!REGISTER_LOCAL_RESULT) {
        NETMGR_LOG_E("Register to local sa manager failed");
        registerToService_ = false;
        return false;
    }

    AddSystemAbilityListener(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);

    netConnEventRunner_ = AppExecFwk::EventRunner::Create(NET_CONN_MANAGER_WORK_THREAD);
    if (netConnEventRunner_ == nullptr) {
        NETMGR_LOG_E("Create event runner failed.");
        return false;
    }
    netConnEventHandler_ = std::make_shared<NetConnEventHandler>(netConnEventRunner_);
    CreateDefaultRequest();
    serviceIface_ = std::make_unique<NetConnServiceIface>().release();
    NetManagerCenter::GetInstance().RegisterConnService(serviceIface_);

    interfaceStateCallback_ = new (std::nothrow) NetInterfaceStateCallback();
    if (interfaceStateCallback_) {
        NetsysController::GetInstance().RegisterCallback(interfaceStateCallback_);
    }
    dnsResultCallback_ = std::make_unique<NetDnsResultCallback>().release();
    int32_t regDnsResult = NetsysController::GetInstance().RegisterDnsResultCallback(dnsResultCallback_, 0);
    NETMGR_LOG_I("Register Dns Result callback result: [%{public}d]", regDnsResult);

    netFactoryResetCallback_ = std::make_unique<NetFactoryResetCallback>().release();
    if (netFactoryResetCallback_ == nullptr) {
        NETMGR_LOG_E("netFactoryResetCallback_ is nullptr");
    }
    AddSystemAbilityListener(ACCESS_TOKEN_MANAGER_SERVICE_ID);
    AddSystemAbilityListener(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);

    netConnEventHandler_->PostAsyncTask([&]() { CheckProxyStatus(); }, PROXY_INIT_DELAY_TIME);
#ifdef ENABLE_SET_APP_FROZENED
    if (netConnEventHandler_) {
        int64_t delayTime = 3000;
        appStateAwareCallback_.OnForegroundAppChanged = [] (const uint32_t uid) {
            std::shared_ptr<NetConnService> netConnService = NetConnService::GetInstance();
            if (netConnService) {
                netConnService ->SetAppIsFrozened(uid, false);
            }
        };
        netConnEventHandler_->PostAsyncTask([this]() {
            AppStateAwareManager::GetInstance().RegisterAppStateAwareCallback(appStateAwareCallback_);
        },
            delayTime);
    }
#endif
    NETMGR_LOG_I("Init end");
    return true;
}

bool NetConnService::CheckIfSettingsDataReady()
{
    if (isDataShareReady_.load()) {
        return true;
    }
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        NETMGR_LOG_E("GetSystemAbilityManager failed.");
        return false;
    }
    sptr<IRemoteObject> dataShareSa = saManager->GetSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
    if (dataShareSa == nullptr) {
        NETMGR_LOG_E("Get dataShare SA Failed.");
        return false;
    }
    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        NETMGR_LOG_E("NetDataShareHelperUtils GetSystemAbility Service Failed.");
        return false;
    }
    std::pair<int, std::shared_ptr<DataShare::DataShareHelper>> ret =
            DataShare::DataShareHelper::Create(remoteObj, SETTINGS_DATASHARE_URI, SETTINGS_DATA_EXT_URI);
    NETMGR_LOG_I("create data_share helper, ret=%{public}d", ret.first);
    if (ret.first == DataShare::E_OK) {
        NETMGR_LOG_I("create data_share helper success");
        auto helper = ret.second;
        if (helper != nullptr) {
            bool releaseRet = helper->Release();
            NETMGR_LOG_I("release data_share helper, releaseRet=%{public}d", releaseRet);
        }
        isDataShareReady_ = true;
        return true;
    } else if (ret.first == DataShare::E_DATA_SHARE_NOT_READY) {
        NETMGR_LOG_E("create data_share helper failed");
        isDataShareReady_ = false;
        return false;
    }
    NETMGR_LOG_E("data_share unknown.");
    return true;
}

int32_t NetConnService::SystemReady()
{
    if (state_ == STATE_RUNNING) {
        NETMGR_LOG_D("System ready.");
        return NETMANAGER_SUCCESS;
    } else {
        return NETMANAGER_ERROR;
    }
}

// Do not post into event handler, because this interface should have good performance
int32_t NetConnService::SetInternetPermission(uint32_t uid, uint8_t allow)
{
    return NetsysController::GetInstance().SetInternetPermission(uid, allow);
}

int32_t NetConnService::RegisterNetSupplier(NetBearType bearerType, const std::string &ident,
                                            const std::set<NetCap> &netCaps, uint32_t &supplierId)
{
    std::set<NetCap> tmp = netCaps;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    NETMGR_LOG_D("RegisterNetSupplier in netcaps size = %{public}zu.", tmp.size());
    if (bearerType != BEARER_VPN) {
        tmp.insert(NET_CAPABILITY_NOT_VPN);
    }
    NETMGR_LOG_D("RegisterNetSupplier out netcaps size = %{public}zu.", tmp.size());
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, bearerType, &ident, tmp, &supplierId, callingUid, &result]() {
            result = this->RegisterNetSupplierAsync(bearerType, ident, tmp, supplierId, callingUid);
        });
    }
    return result;
}

int32_t NetConnService::RegisterNetSupplierCallback(uint32_t supplierId, const sptr<INetSupplierCallback> &callback)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, supplierId, &callback, &result]() {
            result = this->RegisterNetSupplierCallbackAsync(supplierId, callback);
        });
    }
    return result;
}

int32_t NetConnService::RegisterNetConnCallback(const sptr<INetConnCallback> callback)
{
    NETMGR_LOG_D("RegisterNetConnCallback service in.");
    return RegisterNetConnCallback(defaultNetSpecifier_, callback, 0);
}

int32_t NetConnService::RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
                                                const sptr<INetConnCallback> callback, const uint32_t &timeoutMS)
{
    uint32_t callingUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());

    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, &netSpecifier, &callback, timeoutMS, callingUid, &result]() {
            result = this->RegisterNetConnCallbackAsync(netSpecifier, callback, timeoutMS, callingUid);
        });
    }
    return result;
}

int32_t NetConnService::RequestNetConnection(
    const sptr<NetSpecifier> netSpecifier, const sptr<INetConnCallback> callback, const uint32_t timeoutMS)
{
    uint32_t callingUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());

    int32_t result = NETMANAGER_ERROR;
    if (netSpecifier == nullptr) {
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    std::set<NetCap> &netCaps = netSpecifier->netCapabilities_.netCaps_;
    if (netCaps.find(NetCap::NET_CAPABILITY_INTERNAL_DEFAULT) != netCaps.end()) {
        if (!NetManagerPermission::CheckPermission(Permission::CONNECTIVITY_INTERNAL)) {
                NETMGR_LOG_I("Permission deny: Request with INTERNAL_DEFAULT But not has CONNECTIVITY_INTERNAL");
                return NETMANAGER_ERR_PERMISSION_DENIED;
        }
    } else {
        if (!NetManagerPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
                NETMGR_LOG_I("Permission deny: request need GET_NETWORK_INFO permission");
                return NETMANAGER_ERR_PERMISSION_DENIED;
        }
    }
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, netSpecifier, callback, timeoutMS, &callingUid, &result]() {
            result = this->RequestNetConnectionAsync(netSpecifier, callback, timeoutMS, callingUid);
        });
    }
    return result;
}

int32_t NetConnService::RegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback)
{
    NETMGR_LOG_D("Enter RegisterNetDetectionCallback");
    return RegUnRegNetDetectionCallback(netId, callback, true);
}

int32_t NetConnService::UnregisterNetSupplier(uint32_t supplierId)
{
    int32_t result = NETMANAGER_ERROR;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, supplierId, &callingUid, &result]() {
            result = this->UnregisterNetSupplierAsync(supplierId, false, callingUid);
        });
    }
    return result;
}

int32_t NetConnService::UnregisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    uint32_t callingUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, &callback, callingUid, &result]() {
            result = this->UnregisterNetConnCallbackAsync(callback, callingUid);
        });
    }
    return result;
}

int32_t NetConnService::UnRegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback)
{
    NETMGR_LOG_D("Enter UnRegisterNetDetectionCallback");
    return RegUnRegNetDetectionCallback(netId, callback, false);
}

int32_t NetConnService::RegUnRegNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback,
                                                     bool isReg)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, netId, &callback, isReg, &result]() {
            result = this->RegUnRegNetDetectionCallbackAsync(netId, callback, isReg);
        });
    }
    return result;
}

int32_t NetConnService::UpdateNetCaps(const std::set<NetCap> &netCaps, const uint32_t supplierId)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, &netCaps, supplierId, &result]() {
            result = this->UpdateNetCapsAsync(netCaps, supplierId);
        });
    }
    return result;
}

int32_t NetConnService::UpdateNetStateForTest(const sptr<NetSpecifier> &netSpecifier, int32_t netState)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, &netSpecifier, netState, &result]() {
            result = this->UpdateNetStateForTestAsync(netSpecifier, netState);
        });
    }
    return result;
}

int32_t NetConnService::UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo)
{
    int32_t result = NETMANAGER_ERROR;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, supplierId, &netSupplierInfo, callingUid, &result]() {
            result = this->UpdateNetSupplierInfoAsync(supplierId, netSupplierInfo, callingUid);
        });
    }
    return result;
}

int32_t NetConnService::UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo)
{
    int32_t result = NETMANAGER_ERROR;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    httpProxyThreadCv_.notify_all();
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, supplierId, &netLinkInfo, callingUid, &result]() {
            result = this->UpdateNetLinkInfoAsync(supplierId, netLinkInfo, callingUid);
        });
    }
    return result;
}

int32_t NetConnService::NetDetection(int32_t netId)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    NETMGR_LOG_I("NetDetection, call uid [%{public}d]", callingUid);
    httpProxyThreadCv_.notify_all();
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, netId, &result]() { result = this->NetDetectionAsync(netId); });
    }
    return result;
}

int32_t NetConnService::RestrictBackgroundChanged(bool restrictBackground)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, restrictBackground, &result]() {
            result = this->RestrictBackgroundChangedAsync(restrictBackground);
        });
    }
    return result;
}

int32_t NetConnService::RegisterNetSupplierAsync(NetBearType bearerType, const std::string &ident,
                                                 const std::set<NetCap> &netCaps, uint32_t &supplierId,
                                                 int32_t callingUid)
{
    NETMGR_LOG_I("RegisterNetSupplier service in, bearerType[%{public}u], ident[%{public}s]",
                 static_cast<uint32_t>(bearerType), ident.c_str());
    // If there is no supplier in the list, create a supplier
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        NETMGR_LOG_E("netType parameter invalid");
        return NET_CONN_ERR_NET_TYPE_NOT_FOUND;
    }
    sptr<NetSupplier> supplier = GetNetSupplierFromList(bearerType, ident, netCaps);
    if (supplier != nullptr) {
        NETMGR_LOG_E("Supplier[%{public}d %{public}s] already exists.", supplier->GetSupplierId(), ident.c_str());
        supplierId = supplier->GetSupplierId();
        return NETMANAGER_SUCCESS;
    }
    // If there is no supplier in the list, create a supplier
    supplier = (std::make_unique<NetSupplier>(bearerType, ident, netCaps)).release();
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier is nullptr");
        return NET_CONN_ERR_NO_SUPPLIER;
    }
    supplierId = supplier->GetSupplierId();
    // create network
    bool isContainInternal = netCaps.find(NetCap::NET_CAPABILITY_INTERNAL_DEFAULT) != netCaps.end();
    int32_t netId = isContainInternal ? GenerateInternalNetId() : GenerateNetId();
    NETMGR_LOG_D("GenerateNetId is: [%{public}d], bearerType: %{public}d, supplierId: %{public}d",
        netId, bearerType, supplierId);
    if (netId == INVALID_NET_ID) {
        NETMGR_LOG_E("GenerateNetId fail");
        return NET_CONN_ERR_INVALID_NETWORK;
    }
    std::shared_ptr<Network> network = std::make_shared<Network>(
        netId, supplierId,
        std::bind(&NetConnService::HandleDetectionResult, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2),
        bearerType, netConnEventHandler_);
    network->SetScreenState(isScreenOn_);
    network->SetNetCaps(netCaps);
    supplier->SetNetwork(network);
    supplier->SetUid(callingUid);
    // save supplier
    std::unique_lock<std::recursive_mutex> locker(netManagerMutex_);
    netSuppliers_[supplierId] = supplier;
    networks_[netId] = network;
    locker.unlock();
    struct EventInfo eventInfo = {.netId = netId, .bearerType = bearerType, .ident = ident, .supplierId = supplierId};
    EventReport::SendSupplierBehaviorEvent(eventInfo);
    NETMGR_LOG_I("RegisterNetSupplier service out, supplier[%{public}d %{public}s] netId[%{public}d]", supplierId,
                 ident.c_str(), netId);
    return NETMANAGER_SUCCESS;
}

void NetConnService::OnNetSupplierRemoteDied(const wptr<IRemoteObject> &remoteObject)
{
    sptr<IRemoteObject> diedRemoted = remoteObject.promote();
    if (diedRemoted == nullptr || netConnEventHandler_ == nullptr) {
        NETMGR_LOG_E("diedRemoted or netConnEventHandler_ is null");
        return;
    }
    uint32_t callingUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());
    uint32_t tmpSupplierId = INVALID_SUPPLIER_ID;
    NETMGR_LOG_I("OnNetSupplierRemoteDied, callingUid=%{public}u", callingUid);
    sptr<INetSupplierCallback> callback = iface_cast<INetSupplierCallback>(diedRemoted);

    netConnEventHandler_->PostSyncTask([this, &tmpSupplierId, callingUid, &callback]() {
        for (const auto &supplier : netSuppliers_) {
            if (supplier.second == nullptr || supplier.second->GetSupplierCallback() == nullptr) {
                continue;
            }
            if (supplier.second->GetSupplierCallback()->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr()) {
                tmpSupplierId = supplier.second->GetSupplierId();
                break;
            }
        }
        if (tmpSupplierId != INVALID_SUPPLIER_ID) {
            NETMGR_LOG_I("OnNetSupplierRemoteDied UnregisterNetSupplier SupplierId %{public}u", tmpSupplierId);
            UnregisterNetSupplierAsync(tmpSupplierId, true, callingUid);
        }
    });
}

void NetConnService::RemoveNetSupplierDeathRecipient(const sptr<INetSupplierCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("RemoveNetSupplierDeathRecipient is null");
        return;
    }
    callback->AsObject()->RemoveDeathRecipient(netSuplierDeathRecipient_);
}

void NetConnService::AddNetSupplierDeathRecipient(const sptr<INetSupplierCallback> &callback)
{
    if (netSuplierDeathRecipient_ == nullptr) {
        netSuplierDeathRecipient_ = new (std::nothrow) NetSupplierCallbackDeathRecipient(*this);
    }
    if (netSuplierDeathRecipient_ == nullptr) {
        NETMGR_LOG_E("netSuplierDeathRecipient_ is null");
        return;
    }
    if (!callback->AsObject()->AddDeathRecipient(netSuplierDeathRecipient_)) {
        NETMGR_LOG_E("AddNetSupplierDeathRecipient failed");
        return;
    }
}

int32_t NetConnService::RegisterNetSupplierCallbackAsync(uint32_t supplierId,
                                                         const sptr<INetSupplierCallback> &callback)
{
    NETMGR_LOG_I("RegisterNetSupplierCallback service in, supplierId[%{public}d]", supplierId);
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    auto supplier = FindNetSupplier(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier doesn't exist.");
        return NET_CONN_ERR_NO_SUPPLIER;
    }
    supplier->RegisterSupplierCallback(callback);
    SendAllRequestToNetwork(supplier);
    AddNetSupplierDeathRecipient(callback);
    NETMGR_LOG_I("RegisterNetSupplierCallback service out");
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::RegisterNetConnCallbackAsync(const sptr<NetSpecifier> &netSpecifier,
                                                     const sptr<INetConnCallback> &callback, const uint32_t &timeoutMS,
                                                     const uint32_t callingUid)
{
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        struct EventInfo eventInfo = {.errorType = static_cast<int32_t>(FAULT_INVALID_PARAMETER),
                                      .errorMsg = ERROR_MSG_NULL_NET_SPECIFIER};
        EventReport::SendRequestFaultEvent(eventInfo);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    uint32_t reqId = 0;
    if (FindSameCallback(callback, reqId)) {
        NETMGR_LOG_E("FindSameCallback callUid:%{public}u reqId:%{public}u", callingUid,
                     reqId);
        return NET_CONN_ERR_SAME_CALLBACK;
    }
    auto registerType = (netSpecifier != nullptr && ((netSpecifier->netCapabilities_.netCaps_.count(
        NetManagerStandard::NET_CAPABILITY_INTERNAL_DEFAULT) > 0) ||
        (netSpecifier->netCapabilities_.bearerTypes_.count(NetManagerStandard::BEARER_CELLULAR) > 0))) ?
        REQUEST : REGISTER;
    NETMGR_LOG_I("Register net connect callback async, callUid[%{public}u], reqId[%{public}u], regType[%{public}u]",
                 callingUid, reqId, registerType);
    if ((netSpecifier->netCapabilities_.netCaps_.count(
        NetManagerStandard::NET_CAPABILITY_MMS) > 0) && system::GetBoolParameter(PERSIST_EDM_MMS_DISABLE, false)) {
        NETMGR_LOG_E("NetConnService::RegisterNetConnCallbackAsync mms policy is disable");
        return NET_CONN_ERR_POLICY_DISABLED;
    }
    int32_t ret = IncreaseNetConnCallbackCntForUid(callingUid, registerType);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    AddClientDeathRecipient(callback);
    return ActivateNetwork(netSpecifier, callback, timeoutMS, REGISTER, callingUid);
}

int32_t NetConnService::RequestNetConnectionAsync(const sptr<NetSpecifier> &netSpecifier,
                                                  const sptr<INetConnCallback> &callback, const uint32_t &timeoutMS,
                                                  const uint32_t callingUid)
{
    NETMGR_LOG_I("Request net connect callback async, call uid [%{public}u]", callingUid);
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        struct EventInfo eventInfo = {.errorType = static_cast<int32_t>(FAULT_INVALID_PARAMETER),
                                      .errorMsg = ERROR_MSG_NULL_NET_SPECIFIER};
        EventReport::SendRequestFaultEvent(eventInfo);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    uint32_t reqId = 0;
    if (FindSameCallback(callback, reqId)) {
        NETMGR_LOG_E("RequestNetConnection found same callback");
        return NET_CONN_ERR_SAME_CALLBACK;
    }
    int32_t ret = IncreaseNetConnCallbackCntForUid(callingUid, REQUEST);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }
    AddClientDeathRecipient(callback);
    return ActivateNetwork(netSpecifier, callback, timeoutMS, REQUEST, callingUid);
}

int32_t NetConnService::UnregisterNetSupplierAsync(uint32_t supplierId, bool ignoreUid, int32_t callingUid)
{
    NETMGR_LOG_I("UnregisterNetSupplier service in, supplierId[%{public}d]", supplierId);
    // Remove supplier from the list based on supplierId
    auto supplier = FindNetSupplier(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier doesn't exist.");
        return NET_CONN_ERR_NO_SUPPLIER;
    }
    if (!ignoreUid && CheckAndCompareUid(supplier, callingUid) != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("UnregisterNetSupplierAsync uid[%{public}d] is not equal to callingUid[%{public}d].",
                     supplier->GetUid(), callingUid);
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    NETMGR_LOG_I("Unregister supplier[%{public}d, %{public}d, %{public}s], defaultNetSupplier[%{public}d], %{public}s]",
                 supplier->GetSupplierId(), supplier->GetUid(), supplier->GetNetSupplierIdent().c_str(),
                 defaultNetSupplier_ ? defaultNetSupplier_->GetSupplierId() : 0,
                 defaultNetSupplier_ ? defaultNetSupplier_->GetNetSupplierIdent().c_str() : "null");

    struct EventInfo eventInfo = {.bearerType = supplier->GetNetSupplierType(),
                                  .ident = supplier->GetNetSupplierIdent(),
                                  .supplierId = supplier->GetSupplierId()};
    EventReport::SendSupplierBehaviorEvent(eventInfo);

    int32_t netId = supplier->GetNetId();
    NET_NETWORK_MAP::iterator iterNetwork = networks_.find(netId);
    if (iterNetwork != networks_.end()) {
        NETMGR_LOG_I("the iterNetwork already exists.");
        std::unique_lock<std::recursive_mutex> locker(netManagerMutex_);
        networks_.erase(iterNetwork);
        locker.unlock();
    }
    if (defaultNetSupplier_ == supplier) {
        NETMGR_LOG_I("Set default net supplier to nullptr.");
        sptr<NetSupplier> newSupplier = nullptr;
        MakeDefaultNetWork(defaultNetSupplier_, newSupplier);
    }
    NetSupplierInfo info;
    supplier->UpdateNetSupplierInfo(info);
    RemoveNetSupplierDeathRecipient(supplier->GetSupplierCallback());
    std::unique_lock<std::recursive_mutex> locker(netManagerMutex_);
    netSuppliers_.erase(supplierId);
    locker.unlock();
    FindBestNetworkForAllRequest();
    NETMGR_LOG_I("UnregisterNetSupplier supplierId[%{public}d] out", supplierId);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::CheckAndCompareUid(sptr<NetSupplier> &supplier, int32_t callingUid)
{
    int32_t uid = supplier->GetUid();
    if (uid != callingUid) {
        struct EventInfo eventInfo = {
            .errorType = static_cast<int32_t>(NETMANAGER_ERR_INVALID_PARAMETER),
            .errorMsg = std::string(ERROR_MSG_UPDATE_ERROR_UID) +
                        std::to_string(callingUid)
        };
        EventReport::SendSupplierFaultEvent(eventInfo);
    }
    return NETMANAGER_SUCCESS;
}

#ifdef FEATURE_SUPPORT_POWERMANAGER
void NetConnService::StopAllNetDetection()
{
    netConnEventHandler_->PostSyncTask([this]() {
        for (const auto& pNetSupplier : netSuppliers_) {
            if (pNetSupplier.second == nullptr) {
                continue;
            }
            std::shared_ptr<Network> pNetwork = pNetSupplier.second->GetNetwork();
            if (pNetwork == nullptr) {
                NETMGR_LOG_E("pNetwork is null, id:%{public}d", pNetSupplier.first);
                continue;
            }
            pNetwork->StopNetDetection();
            pNetwork->UpdateForbidDetectionFlag(true);
        }
    });
}

void NetConnService::StartAllNetDetection()
{
    netConnEventHandler_->PostSyncTask([this]() {
        for (const auto& pNetSupplier : netSuppliers_) {
            if (pNetSupplier.second == nullptr) {
                continue;
            }
            std::shared_ptr<Network> pNetwork = pNetSupplier.second->GetNetwork();
            if (pNetwork == nullptr) {
                NETMGR_LOG_E("pNetwork is null, id:%{public}d", pNetSupplier.first);
                continue;
            }
            pNetwork->UpdateForbidDetectionFlag(false);
        }
        if ((defaultNetSupplier_ == nullptr)) {
            NETMGR_LOG_W("defaultNetSupplier_ is  null");
            return;
        }
        std::shared_ptr<Network> pDefaultNetwork = defaultNetSupplier_->GetNetwork();
        if (pDefaultNetwork == nullptr) {
            NETMGR_LOG_E("pDefaultNetwork is null");
            return;
        }
        httpProxyThreadCv_.notify_all();
        pDefaultNetwork->StartNetDetection(false);
    });
}

void NetConnService::HandlePowerMgrEvent(int code)
{
    if (code == STATE_ENTER_FORCESLEEP || code == STATE_ENTER_SLEEP_NOT_FORCE) {
        NETMGR_LOG_I("on receive enter sleep, code %{public}d.", code);
        if (netConnEventHandler_) {
            netConnEventHandler_->PostSyncTask([this]() {
                this->StopAllNetDetection();
            });
        }
        isInSleep_.store(true);
    } else if (code == STATE_EXIT_FORCESLEEP || code == STATE_EXIT_SLEEP_NOT_FORCE) {
        NETMGR_LOG_I("on receive exit sleep, code %{public}d.", code);
        if (netConnEventHandler_) {
            netConnEventHandler_->PostSyncTask([this]() {
                this->StartAllNetDetection();
            });
        }
        isInSleep_.store(false);
    }
}
#endif

void NetConnService::HandleScreenEvent(bool isScreenOn)
{
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    isScreenOn_ = isScreenOn;
    for (const auto& pNetSupplier : netSuppliers_) {
        if (pNetSupplier.second == nullptr) {
            continue;
        }
        std::shared_ptr<Network> pNetwork = pNetSupplier.second->GetNetwork();
        if (pNetwork == nullptr) {
            NETMGR_LOG_E("pNetwork is null, id:%{public}d", pNetSupplier.first);
            continue;
        }
        int delayTime = 0;
        if (netConnEventHandler_) {
            netConnEventHandler_->PostAsyncTask([pNetwork, isScreenOn]() { pNetwork->SetScreenState(isScreenOn); },
                                                delayTime);
        }
        if (!isScreenOn || pNetSupplier.second->GetNetSupplierType() != BEARER_WIFI ||
            !pNetSupplier.second->HasNetCap(NET_CAPABILITY_PORTAL)) {
            continue;
        }
        if (netConnEventHandler_) {
            netConnEventHandler_->PostAsyncTask([pNetwork]() { pNetwork->StartNetDetection(true); }, delayTime);
        }
    }
}

int32_t NetConnService::UnregisterNetConnCallbackAsync(const sptr<INetConnCallback> &callback,
                                                       const uint32_t callingUid)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("callback is null, callUid[%{public}u]", callingUid);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    RegisterType registerType = INVALIDTYPE;
    uint32_t reqId = 0;
    uint32_t uid = 0;
    if (!FindSameCallback(callback, reqId, registerType, uid) || registerType == INVALIDTYPE) {
        NETMGR_LOG_D("NotFindSameCallback callUid:%{public}u reqId:%{public}u, uid:%{public}d",
                     callingUid, reqId, uid);
        return NET_CONN_ERR_CALLBACK_NOT_FOUND;
    }
    NETMGR_LOG_I("start, callUid:%{public}u, reqId:%{public}u, uid:%{public}d", callingUid, reqId, uid);
    DecreaseNetConnCallbackCntForUid(uid, registerType);
    DecreaseNetActivatesForUid(uid, callback);
    DecreaseNetActivates(uid, callback, reqId);

    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::IncreaseNetConnCallbackCntForUid(const uint32_t callingUid, const RegisterType registerType)
{
    std::lock_guard guard(netUidRequestMutex_);
    auto &netUidRequest = registerType == REGISTER ?
        netUidRequest_ : internalDefaultUidRequest_;
    auto requestNetwork = netUidRequest.find(callingUid);
    if (requestNetwork == netUidRequest.end()) {
        netUidRequest.insert(std::make_pair(callingUid, 1));
    } else {
        if (requestNetwork->second >= MAX_ALLOW_UID_NUM) {
            NETMGR_LOG_E("return falied for UID [%{public}d] has registered over [%{public}d] callback",
                         callingUid, MAX_ALLOW_UID_NUM);
            return NET_CONN_ERR_NET_OVER_MAX_REQUEST_NUM;
        } else {
            requestNetwork->second++;
        }
    }
    return NETMANAGER_SUCCESS;
}

void NetConnService::DecreaseNetConnCallbackCntForUid(const uint32_t callingUid, const RegisterType registerType)
{
    std::lock_guard guard(netUidRequestMutex_);
    auto &netUidRequest = registerType == REGISTER ?
        netUidRequest_ : internalDefaultUidRequest_;
    auto requestNetwork = netUidRequest.find(callingUid);
    if (requestNetwork == netUidRequest.end()) {
        NETMGR_LOG_E("Could not find the request calling uid");
    } else {
        if (requestNetwork->second >= 1) {
            requestNetwork->second--;
        }
        if (requestNetwork->second == 0) {
            netUidRequest.erase(requestNetwork);
        }
    }
}

void NetConnService::DecreaseNetActivatesForUid(const uint32_t callingUid, const sptr<INetConnCallback> &callback)
{
    std::lock_guard guard(uidActivateMutex_);
    auto it = netUidActivates_.find(callingUid);
    if (it != netUidActivates_.end()) {
        std::vector<std::shared_ptr<NetActivate>> &activates = it->second;
        for (auto iter = activates.begin(); iter != activates.end();) {
            if ((*iter)->GetNetCallback() == callback) {
                iter = activates.erase(iter);
                break;
            } else {
                ++iter;
            }
        }
        if (activates.empty()) {
            netUidActivates_.erase(it);
        }
    }
}

void NetConnService::DecreaseNetActivates(const uint32_t callingUid, const sptr<INetConnCallback> &callback,
                                          uint32_t reqId)
{
    std::lock_guard<std::shared_mutex> lock(netActivatesMutex_);
    for (auto iterActive = netActivates_.begin(); iterActive != netActivates_.end();) {
        if (!iterActive->second) {
            ++iterActive;
            continue;
        }
        sptr<INetConnCallback> saveCallback = iterActive->second->GetNetCallback();
        if (saveCallback == nullptr) {
            ++iterActive;
            continue;
        }
        if (callback->AsObject().GetRefPtr() != saveCallback->AsObject().GetRefPtr()) {
            ++iterActive;
            continue;
        }
        reqId = iterActive->first;
        auto netActivate = iterActive->second;
        NetRequest netRequest(netActivate->GetUid(), reqId);
        if (netActivate) {
            sptr<NetSupplier> supplier = netActivate->GetServiceSupply();
            if (supplier) {
                supplier->CancelRequest(netRequest);
            }
        }
        NET_SUPPLIER_MAP::iterator iterSupplier;
        for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
            if (iterSupplier->second != nullptr) {
                iterSupplier->second->CancelRequest(netRequest);
            }
        }
        iterActive = netActivates_.erase(iterActive);
        RemoveClientDeathRecipient(callback);
    }
    NETMGR_LOG_I("end, callUid:%{public}u, reqId:%{public}u", callingUid, reqId);
}

int32_t NetConnService::RegUnRegNetDetectionCallbackAsync(int32_t netId, const sptr<INetDetectionCallback> &callback,
                                                          bool isReg)
{
    NETMGR_LOG_I("Enter Async");
    if (callback == nullptr) {
        NETMGR_LOG_E("The parameter of callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    auto iterNetwork = networks_.find(netId);
    if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr)) {
        NETMGR_LOG_E("Could not find the corresponding network.");
        return NET_CONN_ERR_NETID_NOT_FOUND;
    }
    if (isReg) {
        iterNetwork->second->RegisterNetDetectionCallback(callback);
        return NETMANAGER_SUCCESS;
    }
    return iterNetwork->second->UnRegisterNetDetectionCallback(callback);
}

int32_t NetConnService::UpdateNetCapsAsync(const std::set<NetCap> &netCaps, const uint32_t supplierId)
{
    NETMGR_LOG_I("Update net caps async.");
    auto supplier = FindNetSupplier(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier is not exists.");
        return NET_CONN_ERR_NO_SUPPLIER;
    }

    std::unique_lock<std::recursive_mutex> locker(netManagerMutex_);
    auto network = supplier->GetNetwork();
    if (network == nullptr) {
        NETMGR_LOG_E("network is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    network->SetNetCaps(netCaps);
    supplier->SetNetwork(network);
    CallbackForSupplier(supplier, CALL_TYPE_UPDATE_CAP);
    FindBestNetworkForAllRequest();
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::UpdateNetStateForTestAsync(const sptr<NetSpecifier> &netSpecifier, int32_t netState)
{
    NETMGR_LOG_D("Test NetConnService::UpdateNetStateForTest(), begin");
    if (netSpecifier == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return NETMANAGER_SUCCESS;
}

void NetConnService::UpdateNetSupplierInfoAsyncInvalid(uint32_t supplierId)
{
    struct EventInfo eventInfo = {.updateSupplierId = supplierId};
    NETMGR_LOG_E("UpdateNetSupplierInfoAsync netSupplierInfo is nullptr");
    eventInfo.errorType = static_cast<int32_t>(FAULT_UPDATE_SUPPLIERINFO_INV_PARAM);
    eventInfo.errorMsg = ERROR_MSG_NULL_SUPPLIER_INFO;
    EventReport::SendSupplierFaultEvent(eventInfo);
}

int32_t NetConnService::UpdateNetSupplierInfoAsync(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo,
                                                   int32_t callingUid)
{
    NETMGR_LOG_I("UpdateNetSupplierInfo service in. supplierId[%{public}d]", supplierId);
    struct EventInfo eventInfo = {.updateSupplierId = supplierId};
    if (netSupplierInfo == nullptr) {
        UpdateNetSupplierInfoAsyncInvalid(supplierId);
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    eventInfo.supplierInfo = netSupplierInfo->ToString("\"");

    auto supplier = FindNetSupplier(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("Can not find supplier for supplierId[%{public}d]", supplierId);
        eventInfo.errorType = static_cast<int32_t>(FAULT_UPDATE_SUPPLIERINFO_INV_PARAM);
        eventInfo.errorMsg = std::string(ERROR_MSG_CAN_NOT_FIND_SUPPLIER).append(std::to_string(supplierId));
        EventReport::SendSupplierFaultEvent(eventInfo);
        return NET_CONN_ERR_NO_SUPPLIER;
    }

    if (CheckAndCompareUid(supplier, callingUid) != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("UpdateNetSupplierInfoAsync uid[%{public}d] is not equal to callingUid[%{public}d].",
                     supplier->GetUid(), callingUid);
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    eventInfo.bearerType = supplier->GetNetSupplierType();
    eventInfo.netId = supplier->GetNetId();
    EventReport::SendSupplierBehaviorEvent(eventInfo);
    NETMGR_LOG_I("Update supplier[%{public}d, %{public}d, %{public}s], supplierInfo:[ %{public}s ]", supplierId,
                 supplier->GetUid(), supplier->GetNetSupplierIdent().c_str(), netSupplierInfo->ToString(" ").c_str());
    supplier->UpdateNetSupplierInfo(*netSupplierInfo);
    if (!netSupplierInfo->isAvailable_) {
        CallbackForSupplier(supplier, CALL_TYPE_LOST);
        if (supplierId == delaySupplierId_) {
            RemoveDelayNetwork();
        }
        std::unique_lock<std::recursive_mutex> locker(netManagerMutex_);
        supplier->ResetNetSupplier();
        locker.unlock();
        ProcessHttpProxyCancel(supplier);
    } else {
        CallbackForSupplier(supplier, CALL_TYPE_UPDATE_CAP);
    }
    // Init score again here in case of net supplier type changed.
    std::unique_lock<std::recursive_mutex> initLocker(netManagerMutex_);
    if (netSupplierInfo->score_ == 0) {
        supplier->InitNetScore();
    }
    initLocker.unlock();
    FindBestNetworkForAllRequest();
    NETMGR_LOG_I("UpdateNetSupplierInfo service out.");
    return NETMANAGER_SUCCESS;
}

void NetConnService::RemoveDelayNetwork()
{
    if (netConnEventHandler_) {
        netConnEventHandler_->RemoveTask("HandleFindBestNetworkForDelay");
    }
    isDelayHandleFindBestNetwork_ = false;
    delaySupplierId_ = 0;
}

void NetConnService::HandleFindBestNetworkForDelay()
{
    isDelayHandleFindBestNetwork_ = false;
    auto supplier = FindNetSupplier(delaySupplierId_);
    if (supplier == nullptr) {
        delaySupplierId_ = 0;
        return;
    }
    if (supplier->IsNetValidated()) {
        NETMGR_LOG_I("HandleFindBestNetworkForDelay.");
        HandleDetectionResult(delaySupplierId_, VERIFICATION_STATE);
    }
    delaySupplierId_ = 0;
}

void NetConnService::ProcessHttpProxyCancel(const sptr<NetSupplier> &supplier)
{
    HttpProxy oldHttpProxy;
    supplier->GetHttpProxy(oldHttpProxy);
    if (!oldHttpProxy.GetHost().empty()) {
        HttpProxy emptyProxy;
        SendHttpProxyChangeBroadcast(emptyProxy);
    }
}

int32_t NetConnService::UpdateNetLinkInfoAsync(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo,
                                               int32_t callingUid)
{
    NETMGR_LOG_I("UpdateNetLinkInfo service in. supplierId[%{public}d]", supplierId);
    struct EventInfo eventInfo = {.updateNetlinkId = supplierId};

    if (netLinkInfo == nullptr) {
        NETMGR_LOG_E("netLinkInfo is nullptr");
        eventInfo.errorType = static_cast<int32_t>(FAULT_UPDATE_NETLINK_INFO_INV_PARAM);
        eventInfo.errorMsg = ERROR_MSG_NULL_NET_LINK_INFO;
        EventReport::SendSupplierFaultEvent(eventInfo);
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    eventInfo.netlinkInfo = netLinkInfo->ToString(" ");

    auto supplier = FindNetSupplier(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier is nullptr");
        eventInfo.errorType = static_cast<int32_t>(FAULT_UPDATE_NETLINK_INFO_INV_PARAM);
        eventInfo.errorMsg = std::string(ERROR_MSG_CAN_NOT_FIND_SUPPLIER).append(std::to_string(supplierId));
        EventReport::SendSupplierFaultEvent(eventInfo);
        return NET_CONN_ERR_NO_SUPPLIER;
    }

    if (CheckAndCompareUid(supplier, callingUid) != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("UpdateNetLinkInfoAsync uid[%{public}d] is not equal to callingUid[%{public}d].",
                     supplier->GetUid(), callingUid);
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    eventInfo.bearerType = supplier->GetNetSupplierType();
    eventInfo.netId = supplier->GetNetId();
    EventReport::SendSupplierBehaviorEvent(eventInfo);
    HttpProxy oldHttpProxy;
    supplier->GetHttpProxy(oldHttpProxy);
    // According to supplier id, get network from the list
    std::unique_lock<std::recursive_mutex> locker(netManagerMutex_);
    if (supplier->UpdateNetLinkInfo(*netLinkInfo) != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("UpdateNetLinkInfo fail");
        eventInfo.errorType = static_cast<int32_t>(FAULT_UPDATE_NETLINK_INFO_FAILED);
        eventInfo.errorMsg = ERROR_MSG_UPDATE_NETLINK_INFO_FAILED;
        EventReport::SendSupplierFaultEvent(eventInfo);
        return NET_CONN_ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    locker.unlock();
    CallbackForSupplier(supplier, CALL_TYPE_UPDATE_LINK);
    HandlePreFindBestNetworkForDelay(supplierId, supplier);
    if (!isDelayHandleFindBestNetwork_) {
        FindBestNetworkForAllRequest();
    }
    if (oldHttpProxy != netLinkInfo->httpProxy_) {
        SendHttpProxyChangeBroadcast(netLinkInfo->httpProxy_);
    }
    NETMGR_LOG_I("UpdateNetLinkInfo service out.");
    return NETMANAGER_SUCCESS;
}

void NetConnService::HandlePreFindBestNetworkForDelay(uint32_t supplierId, const sptr<NetSupplier> &supplier)
{
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier is nullptr");
        return;
    }
    if (isDelayHandleFindBestNetwork_) {
        return;
    }
    bool isNeedDelay = (system::GetBoolParameter(PERSIST_WIFI_DELAY_ELEVATOR_ENABLE, false) ||
        system::GetBoolParameter(PERSIST_WIFI_DELAY_WEAK_SIGNAL_ENABLE, false));
    if (supplier->GetNetSupplierType() == BEARER_WIFI && !supplier->IsNetValidated() &&
        defaultNetSupplier_ != nullptr && defaultNetSupplier_->GetNetSupplierType() == BEARER_CELLULAR &&
        isNeedDelay) {
        int64_t delayTime = 2000;
        if (netConnEventHandler_) {
            NETMGR_LOG_I("HandlePreFindBestNetworkForDelay action");
            isDelayHandleFindBestNetwork_ = true;
            delaySupplierId_ = supplierId;
            netConnEventHandler_->PostAsyncTask([this]() { HandleFindBestNetworkForDelay(); },
                "HandleFindBestNetworkForDelay", delayTime);
        }
    }
}

#ifdef SUPPORT_SYSVPN
bool NetConnService::IsCallingUserSupplier(uint32_t supplierId)
{
    NETMGR_LOG_D("IsCallingUserSupplier, supplierId:%{public}d", supplierId);
    sptr<NetSupplier> supplier = FindNetSupplier(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("IsCallingUserSupplier FindNetSupplier error");
        return false;
    }

    int32_t supplierUid = supplier->GetSupplierUid();
    if (supplierUid == ROOT_USER_ID) {
        NETMGR_LOG_D("supplierUid is ROOT_USER_ID");
        return true;
    }

    int32_t callingUserId = INVALID_USER_ID;
    int32_t supplierUserId = INVALID_USER_ID;

    if (AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(realCallingUid_, callingUserId) != ERR_OK) {
        NETMGR_LOG_D("GetOsAccountLocalIdFromUid fail, realCallingUid_: %{public}d", realCallingUid_);
        return false;
    }

    if (callingUserId == ROOT_USER_ID) {
        NETMGR_LOG_D("IsCallingUserSupplier callingUserId is ROOT_USER_ID");
        return true;
    }

    if (AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(supplierUid, supplierUserId) != ERR_OK) {
        NETMGR_LOG_D("GetOsAccountLocalIdFromUid fail, supplierUid: %{public}d", supplierUid);
        return false;
    }

    NETMGR_LOG_D("callingUserId: %{public}d  supplierUserId: %{public}d", callingUserId, supplierUserId);
    if (callingUserId == supplierUserId) {
        NETMGR_LOG_D("IsCallingUserSupplier is true, userId: %{public}d", supplierUserId);
        return true;
    }
    return false;
}
#endif // SUPPORT_SYSVPN

int32_t NetConnService::NetDetectionAsync(int32_t netId)
{
    NETMGR_LOG_I("Enter NetDetection, netId=[%{public}d]", netId);
    auto iterNetwork = networks_.find(netId);
    if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr) || !iterNetwork->second->IsConnected()) {
        NETMGR_LOG_E("Could not find the corresponding network or network is not connected.");
        return NET_CONN_ERR_NETID_NOT_FOUND;
    }
    iterNetwork->second->StartNetDetection(false);
    NETMGR_LOG_D("End NetDetection");
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetDetectionForDnsHealthSync(int32_t netId, bool dnsHealthSuccess)
{
    NETMGR_LOG_D("Enter NetDetectionForDnsHealthSync");
    auto iterNetwork = networks_.find(netId);
    if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr)) {
        NETMGR_LOG_E("Could not find the corresponding network");
        return NET_CONN_ERR_NETID_NOT_FOUND;
    }
    iterNetwork->second->NetDetectionForDnsHealth(dnsHealthSuccess);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::RestrictBackgroundChangedAsync(bool restrictBackground)
{
    NETMGR_LOG_I("Restrict background changed, background = %{public}d", restrictBackground);
    for (auto it = netSuppliers_.begin(); it != netSuppliers_.end(); ++it) {
        if (it->second == nullptr) {
            continue;
        }

        if (it->second->GetRestrictBackground() == restrictBackground) {
            NETMGR_LOG_D("it->second->GetRestrictBackground() == restrictBackground");
            return NET_CONN_ERR_NET_NO_RESTRICT_BACKGROUND;
        }

        if (it->second->GetNetSupplierType() == BEARER_VPN) {
            CallbackForSupplier(it->second, CALL_TYPE_BLOCK_STATUS);
        }
        it->second->SetRestrictBackground(restrictBackground);
    }
    NETMGR_LOG_I("End RestrictBackgroundChangedAsync");
    return NETMANAGER_SUCCESS;
}

void NetConnService::SendHttpProxyChangeBroadcast(const HttpProxy &httpProxy)
{
    BroadcastInfo info;
    info.action = EventFwk::CommonEventSupport::COMMON_EVENT_HTTP_PROXY_CHANGE;
    info.data = "Global HttpProxy Changed";
    info.ordered = false;
    std::map<std::string, std::string> param = {{"HttpProxy", httpProxy.ToString()}};
    int32_t userId = GetValidUserIdFromProxy(httpProxy);
    if (userId == INVALID_USER_ID) {
        return;
    }
    param.emplace("UserId", std::to_string(userId));
    BroadcastManager::GetInstance().SendBroadcast(info, param);
}

int32_t NetConnService::ActivateNetwork(const sptr<NetSpecifier> &netSpecifier, const sptr<INetConnCallback> &callback,
                                        const uint32_t &timeoutMS, const int32_t registerType,
                                        const uint32_t callingUid)
{
    NETMGR_LOG_D("ActivateNetwork Enter");
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("The parameter of netSpecifier or callback is null");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    std::shared_ptr<NetActivate> request = CreateNetActivateRequest(netSpecifier, callback,
        timeoutMS, registerType, callingUid);

    request->StartTimeOutNetAvailable();
    uint32_t reqId = request->GetRequestId();
    NETMGR_LOG_I("New request [id:%{public}u]", reqId);
    NetRequest netrequest(request->GetUid(), reqId);
    {
        std::lock_guard<std::shared_mutex> guard(netActivatesMutex_);
        netActivates_[reqId] = request;
    }
    {
        std::lock_guard guard(uidActivateMutex_);
        netUidActivates_[callingUid].push_back(request);
    }
    sptr<NetSupplier> bestNet = nullptr;
    int bestScore = static_cast<int>(FindBestNetworkForRequest(bestNet, request));
    if (bestScore != 0 && bestNet != nullptr) {
        NETMGR_LOG_I(
            "Match to optimal supplier:[%{public}d %{public}s] netId[%{public}d] score[%{public}d] "
            "reqId[%{public}u]",
            bestNet->GetSupplierId(), bestNet->GetNetSupplierIdent().c_str(), bestNet->GetNetId(), bestScore, reqId);
        bestNet->SelectAsBestNetwork(netrequest);
        request->SetServiceSupply(bestNet);
        CallbackForAvailable(bestNet, callback);
        if ((bestNet->GetNetSupplierType() == BEARER_CELLULAR) || (bestNet->GetNetSupplierType() == BEARER_WIFI)) {
            std::unique_lock<std::recursive_mutex> locker(netManagerMutex_);
            struct EventInfo eventInfo = {.capabilities = bestNet->GetNetCapabilities().ToString(" "),
                                          .supplierIdent = bestNet->GetNetSupplierIdent()};
            locker.unlock();
            EventReport::SendRequestBehaviorEvent(eventInfo);
        }
        return NETMANAGER_SUCCESS;
    }
    if (timeoutMS == 0) {
        callback->NetUnavailable();
    }

    NETMGR_LOG_D("Not matched to the optimal network, send request to all networks.");
    SendRequestToAllNetwork(request);
    return NETMANAGER_SUCCESS;
}

std::shared_ptr<NetActivate> NetConnService::CreateNetActivateRequest(const sptr<NetSpecifier> &netSpecifier,
                                                                      const sptr<INetConnCallback> &callback,
                                                                      const uint32_t &timeoutMS,
                                                                      const int32_t registerType,
                                                                      const uint32_t callingUid)
{
    std::weak_ptr<INetActivateCallback> timeoutCb = shared_from_this();
    std::shared_ptr<NetActivate> request = nullptr;
#ifdef ENABLE_SET_APP_FROZENED
    sptr<NetConnCallbackProxyWrapper> callbakWrapper = new (std::nothrow) NetConnCallbackProxyWrapper(callback);
    if (callbakWrapper == nullptr) {
        NETMGR_LOG_E("NetConnCallbackProxyWrapper ptr is null");
        request = std::make_shared<NetActivate>(
            netSpecifier, callback, timeoutCb, timeoutMS, netConnEventHandler_, callingUid, registerType);
    } else {
        request = std::make_shared<NetActivate>(
        netSpecifier, callbakWrapper, timeoutCb, timeoutMS, netConnEventHandler_, callingUid, registerType);
        callbakWrapper->SetNetActivate(request);
    }
#else
    request = std::make_shared<NetActivate>(
        netSpecifier, callback, timeoutCb, timeoutMS, netConnEventHandler_, callingUid, registerType);
#endif
    return request;
}

void NetConnService::OnNetActivateTimeOut(uint32_t reqId)
{
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([reqId, this]() {
            NETMGR_LOG_I("DeactivateNetwork Enter, reqId is [%{public}d]", reqId);
            std::shared_lock<std::shared_mutex> lock(netActivatesMutex_);
            auto iterActivate = netActivates_.find(reqId);
            if (iterActivate == netActivates_.end()) {
                NETMGR_LOG_E("not found the reqId: [%{public}d]", reqId);
                return;
            }
            NetRequest netrequest;
            netrequest.requestId = reqId;
            if (iterActivate->second != nullptr) {
                sptr<NetSupplier> pNetService = iterActivate->second->GetServiceSupply();
                netrequest.uid = iterActivate->second->GetUid();
                if (pNetService) {
                    pNetService->CancelRequest(netrequest);
                }
            }
            lock.unlock();

            NET_SUPPLIER_MAP::iterator iterSupplier;
            for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
                if (iterSupplier->second == nullptr) {
                    continue;
                }
                iterSupplier->second->CancelRequest(netrequest);
            }
        });
    }
}

sptr<NetSupplier> NetConnService::FindNetSupplier(uint32_t supplierId)
{
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    auto iterSupplier = netSuppliers_.find(supplierId);
    if (iterSupplier != netSuppliers_.end()) {
        return iterSupplier->second;
    }
    return nullptr;
}

bool NetConnService::FindSameCallback(const sptr<INetConnCallback> &callback,
                                      uint32_t &reqId, RegisterType &registerType, uint32_t &uid)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("callback is null");
        return false;
    }
    NET_ACTIVATE_MAP::iterator iterActive;
    NET_ACTIVATE_MAP netActivatesBck;
    {
        std::shared_lock<std::shared_mutex> lock(netActivatesMutex_);
        netActivatesBck = netActivates_;
    }
    for (iterActive = netActivatesBck.begin(); iterActive != netActivatesBck.end(); ++iterActive) {
        if (!iterActive->second) {
            continue;
        }
        sptr<INetConnCallback> saveCallback = iterActive->second->GetNetCallback();
        if (saveCallback == nullptr) {
            continue;
        }
        if (callback->AsObject().GetRefPtr() == saveCallback->AsObject().GetRefPtr()) {
            reqId = iterActive->first;
            if (iterActive->second) {
                auto specifier = iterActive->second->GetNetSpecifier();
                registerType = (specifier != nullptr && ((specifier->netCapabilities_.netCaps_.count(
                    NetManagerStandard::NET_CAPABILITY_INTERNAL_DEFAULT) > 0) ||
                    (specifier->netCapabilities_.bearerTypes_.count(NetManagerStandard::BEARER_CELLULAR) > 0))) ?
                    REQUEST : REGISTER;
                uid = iterActive->second->GetUid();
            }
            return true;
        }
    }
    return false;
}

bool NetConnService::FindSameCallback(const sptr<INetConnCallback> &callback, uint32_t &reqId)
{
    RegisterType registerType = INVALIDTYPE;
    uint32_t uid = 0;
    return FindSameCallback(callback, reqId, registerType, uid);
}

void NetConnService::FindBestNetworkForAllRequest()
{
    NET_ACTIVATE_MAP netActivatesBck;
    {
        std::shared_lock<std::shared_mutex> lock(netActivatesMutex_);
        netActivatesBck = netActivates_;
    }
    NETMGR_LOG_I("FindBestNetworkForAllRequest Enter. netActivates_ size: [%{public}zu]", netActivatesBck.size());
    NET_ACTIVATE_MAP::iterator iterActive;
    sptr<NetSupplier> bestSupplier = nullptr;
    for (iterActive = netActivatesBck.begin(); iterActive != netActivatesBck.end(); ++iterActive) {
        if (!iterActive->second) {
            continue;
        }
        int score = static_cast<int>(FindBestNetworkForRequest(bestSupplier, iterActive->second));
        NETMGR_LOG_D("Find best supplier[%{public}d, %{public}s]for request[%{public}d]",
                     bestSupplier ? bestSupplier->GetSupplierId() : 0,
                     bestSupplier ? bestSupplier->GetNetSupplierIdent().c_str() : "null",
                     iterActive->second->GetRequestId());
        if (iterActive->second == defaultNetActivate_) {
            MakeDefaultNetWork(defaultNetSupplier_, bestSupplier);
        }
        sptr<NetSupplier> oldSupplier = iterActive->second->GetServiceSupply();
        sptr<INetConnCallback> callback = iterActive->second->GetNetCallback();
        if (!bestSupplier) {
            // not found the bestNetwork
            NotFindBestSupplier(iterActive->first, iterActive->second, oldSupplier, callback);
            continue;
        }
        SendBestScoreAllNetwork(iterActive->first, score, bestSupplier->GetSupplierId(), iterActive->second->GetUid());
        if (bestSupplier == oldSupplier) {
            NETMGR_LOG_D("bestSupplier is equal with oldSupplier.");
            continue;
        }
        if (oldSupplier) {
            oldSupplier->RemoveBestRequest(iterActive->first);
        }
        iterActive->second->SetServiceSupply(bestSupplier);
        CallbackForAvailable(bestSupplier, callback);
        NetRequest netRequest(iterActive->second->GetUid(), iterActive->first);
        bestSupplier->SelectAsBestNetwork(netRequest);
    }
    NETMGR_LOG_I("FindBestNetworkForAllRequest end");
}

uint32_t NetConnService::FindBestNetworkForRequest(sptr<NetSupplier> &supplier,
                                                   std::shared_ptr<NetActivate> &netActivateNetwork)
{
    int bestScore = 0;
    supplier = nullptr;
    if (netActivateNetwork == nullptr) {
        NETMGR_LOG_E("netActivateNetwork is null");
        return bestScore;
    }
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    NET_SUPPLIER_MAP::iterator iter;
    for (iter = netSuppliers_.begin(); iter != netSuppliers_.end(); ++iter) {
        if (iter->second == nullptr) {
            continue;
        }
        NETMGR_LOG_D("supplier info, supplier[%{public}d, %{public}s], realScore[%{public}d], isConnected[%{public}d]",
                     iter->second->GetSupplierId(), iter->second->GetNetSupplierIdent().c_str(),
                     iter->second->GetRealScore(), iter->second->IsConnected());
        if ((!iter->second->IsConnected()) || (!netActivateNetwork->MatchRequestAndNetwork(iter->second))) {
            NETMGR_LOG_D("Supplier[%{public}d] is not connected or not match request.", iter->second->GetSupplierId());
            continue;
        }
        int score = iter->second->GetRealScore();
        if (score > bestScore) {
            bestScore = score;
            supplier = iter->second;
        }
    }
    NETMGR_LOG_D(
        "bestScore[%{public}d], bestSupplier[%{public}d, %{public}s], "
        "request[%{public}d] is [%{public}s],",
        bestScore, supplier ? supplier->GetSupplierId() : 0,
        supplier ? supplier->GetNetSupplierIdent().c_str() : "null", netActivateNetwork->GetRequestId(),
        netActivateNetwork->GetNetSpecifier() ? netActivateNetwork->GetNetSpecifier()->ToString(" ").c_str() : "null");
    return bestScore;
}

void NetConnService::RequestAllNetworkExceptDefault()
{
    if ((defaultNetSupplier_ == nullptr) || (defaultNetSupplier_->IsNetValidated())
        || (defaultNetSupplier_->IsNetAcceptUnavalidate())) {
        NETMGR_LOG_E("defaultNetSupplier_ is  null or IsNetValidated or AcceptUnavalidate");
        return;
    }
    NETMGR_LOG_I("Default supplier[%{public}d, %{public}s] is not valid,request to activate another network",
                 defaultNetSupplier_->GetSupplierId(), defaultNetSupplier_->GetNetSupplierIdent().c_str());
    if (defaultNetActivate_ == nullptr) {
        NETMGR_LOG_E("Default net request is null");
        return;
    }
    // Request activation of all networks except the default network
    NetRequest netrequest(
        defaultNetActivate_->GetUid(), defaultNetActivate_->GetRequestId(), defaultNetActivate_->GetRegisterType());
    for (const auto &netSupplier : netSuppliers_) {
        if (netSupplier.second == nullptr || netSupplier.second == defaultNetSupplier_) {
            NETMGR_LOG_E("netSupplier is null or is defaultNetSupplier_");
            continue;
        }
        if (netSupplier.second->GetNetScore() >= defaultNetSupplier_->GetNetScore()) {
            continue;
        }
        if (netSupplier.second->HasNetCap(NetCap::NET_CAPABILITY_INTERNAL_DEFAULT)) {
            NETMGR_LOG_I("Supplier[%{public}d] is internal, skip.", netSupplier.second->GetSupplierId());
            continue;
        }
        if (!defaultNetActivate_->MatchRequestAndNetwork(netSupplier.second, true)) {
            continue;
        }
        if (!netSupplier.second->RequestToConnect(netrequest)) {
            NETMGR_LOG_E("Request network for supplier[%{public}d, %{public}s] failed",
                         netSupplier.second->GetSupplierId(), netSupplier.second->GetNetSupplierIdent().c_str());
        }
    }
}

int32_t NetConnService::GenerateNetId()
{
    for (int32_t i = MIN_NET_ID; i <= MAX_NET_ID; ++i) {
        netIdLastValue_++;
        if (netIdLastValue_ > MAX_NET_ID) {
            netIdLastValue_ = MIN_NET_ID;
        }
        if (networks_.find(netIdLastValue_) == networks_.end()) {
            return netIdLastValue_;
        }
    }
    return INVALID_NET_ID;
}

int32_t NetConnService::GenerateInternalNetId()
{
    for (int32_t i = MIN_INTERNAL_NET_ID; i <= MAX_INTERNAL_NET_ID; ++i) {
        int32_t value = internalNetIdLastValue_++;
        if (value > MAX_INTERNAL_NET_ID) {
            internalNetIdLastValue_ = MIN_INTERNAL_NET_ID;
            value = MIN_INTERNAL_NET_ID;
        }
        if (networks_.find(value) == networks_.end()) {
            return value;
        }
    }
    return INVALID_NET_ID;
}

void NetConnService::NotFindBestSupplier(uint32_t reqId, const std::shared_ptr<NetActivate> &active,
                                         const sptr<NetSupplier> &supplier, const sptr<INetConnCallback> &callback)
{
    NETMGR_LOG_I("Could not find best supplier for request:[%{public}d]", reqId);
    if (supplier != nullptr) {
        supplier->RemoveBestRequest(reqId);
        if (callback != nullptr) {
            sptr<NetHandle> netHandle = supplier->GetNetHandle();
            callback->NetLost(netHandle);
        }
    }
    if (active != nullptr) {
        active->SetServiceSupply(nullptr);
        SendRequestToAllNetwork(active);
    }
}

void NetConnService::SendAllRequestToNetwork(sptr<NetSupplier> supplier)
{
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier is null");
        return;
    }
    NETMGR_LOG_I("Send all request to supplier[%{public}d, %{public}s]", supplier->GetSupplierId(),
                 supplier->GetNetSupplierIdent().c_str());
    NET_ACTIVATE_MAP::iterator iter;
    std::shared_lock<std::shared_mutex> lock(netActivatesMutex_);
    for (iter = netActivates_.begin(); iter != netActivates_.end(); ++iter) {
        if (iter->second == nullptr) {
            continue;
        }
        if (!iter->second->MatchRequestAndNetwork(supplier, true)) {
            continue;
        }
        NetRequest netrequest(iter->second->GetUid(), iter->first, iter->second->GetRegisterType());
        netrequest.bearTypes = iter->second->GetBearType();
        bool result = supplier->RequestToConnect(netrequest);
        if (!result) {
            NETMGR_LOG_E("Request network for supplier[%{public}d, %{public}s] failed", supplier->GetSupplierId(),
                         supplier->GetNetSupplierIdent().c_str());
        }
    }
}

void NetConnService::SendRequestToAllNetwork(std::shared_ptr<NetActivate> request)
{
    if (request == nullptr) {
        NETMGR_LOG_E("request is null");
        return;
    }

    NetRequest netrequest(request->GetUid(),
            request->GetRequestId(),
            request->GetRegisterType(),
            request->GetNetSpecifier()->ident_,
            request->GetBearType());
    NETMGR_LOG_I("Send request[%{public}d] to all supplier", netrequest.requestId);
    NET_SUPPLIER_MAP::iterator iter;
    std::unique_lock<std::recursive_mutex> locker(netManagerMutex_);
    for (iter = netSuppliers_.begin(); iter != netSuppliers_.end(); ++iter) {
        if (iter->second == nullptr) {
            continue;
        }
        if (!request->MatchRequestAndNetwork(iter->second, true)) {
            continue;
        }

        bool result = iter->second->RequestToConnect(netrequest);
        if (!result) {
            NETMGR_LOG_E("Request network for supplier[%{public}d, %{public}s] failed", iter->second->GetSupplierId(),
                         iter->second->GetNetSupplierIdent().c_str());
        }
    }
}

void NetConnService::SendBestScoreAllNetwork(uint32_t reqId, int32_t bestScore, uint32_t supplierId, uint32_t uid)
{
    NETMGR_LOG_D("Send best supplier[%{public}d]-score[%{public}d] to all supplier", supplierId, bestScore);
    NET_SUPPLIER_MAP::iterator iter;
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    for (iter = netSuppliers_.begin(); iter != netSuppliers_.end(); ++iter) {
        if (iter->second == nullptr) {
            continue;
        }
        if (iter->second->HasNetCap(NetCap::NET_CAPABILITY_INTERNAL_DEFAULT)) {
            continue;
        }
        NetRequest netrequest;
        netrequest.uid = uid;
        netrequest.requestId = reqId;
        iter->second->ReceiveBestScore(bestScore, supplierId, netrequest);
    }
}

void NetConnService::CallbackForSupplier(sptr<NetSupplier> &supplier, CallbackType type)
{
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier is nullptr");
        return;
    }
    std::set<uint32_t> &bestReqList = supplier->GetBestRequestList();
    NETMGR_LOG_I("Callback type: %{public}d for supplier[%{public}d, %{public}s], best request size: %{public}zd",
                 static_cast<int32_t>(type), supplier->GetSupplierId(), supplier->GetNetSupplierIdent().c_str(),
                 bestReqList.size());
    NET_ACTIVATE_MAP netActivatesBck;
    {
        std::shared_lock<std::shared_mutex> lock(netActivatesMutex_);
        netActivatesBck = netActivates_;
    }
    for (auto it : bestReqList) {
        auto reqIt = netActivatesBck.find(it);
        if (reqIt == netActivatesBck.end() || reqIt->second == nullptr || reqIt->second->GetNetCallback() == nullptr) {
            continue;
        }
        sptr<INetConnCallback> callback = reqIt->second->GetNetCallback();
        sptr<NetHandle> netHandle = supplier->GetNetHandle();
        HandleCallback(supplier, netHandle, callback, type);
    }
}

void NetConnService::HandleCallback(sptr<NetSupplier> &supplier, sptr<NetHandle> &netHandle,
    sptr<INetConnCallback> callback, CallbackType type)
{
    switch (type) {
        case CALL_TYPE_LOST: {
            callback->NetLost(netHandle);
            break;
        }
        case CALL_TYPE_UPDATE_CAP: {
            sptr<NetAllCapabilities> pNetAllCap = std::make_unique<NetAllCapabilities>().release();
            std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
            *pNetAllCap = supplier->GetNetCapabilities();
            callback->NetCapabilitiesChange(netHandle, pNetAllCap);
            break;
        }
        case CALL_TYPE_UPDATE_LINK: {
            sptr<NetLinkInfo> pInfo = std::make_unique<NetLinkInfo>().release();
            auto network = supplier->GetNetwork();
            if (network != nullptr && pInfo != nullptr) {
                *pInfo = network->GetNetLinkInfo();
            }
            callback->NetConnectionPropertiesChange(netHandle, pInfo);
            break;
        }
        case CALL_TYPE_BLOCK_STATUS: {
            callback->NetBlockStatusChange(netHandle, NetManagerCenter::GetInstance().IsUidNetAccess(
                supplier->GetSupplierUid(), supplier->HasNetCap(NET_CAPABILITY_NOT_METERED)));
            break;
        }
        case CALL_TYPE_UNAVAILABLE: {
            callback->NetUnavailable();
            break;
        }
        default:
            break;
    }
}

void NetConnService::CallbackForAvailable(sptr<NetSupplier> &supplier, const sptr<INetConnCallback> &callback)
{
    if (supplier == nullptr || callback == nullptr) {
        NETMGR_LOG_E("Input parameter is null.");
        return;
    }
    NETMGR_LOG_D("CallbackForAvailable supplier[%{public}d, %{public}s]", supplier->GetSupplierId(),
                 supplier->GetNetSupplierIdent().c_str());
    sptr<NetHandle> netHandle = supplier->GetNetHandle();
    callback->NetAvailable(netHandle);
    sptr<NetAllCapabilities> pNetAllCap = std::make_unique<NetAllCapabilities>().release();
    std::unique_lock<std::recursive_mutex> locker(netManagerMutex_);
    *pNetAllCap = supplier->GetNetCapabilities();
    locker.unlock();
    callback->NetCapabilitiesChange(netHandle, pNetAllCap);
    sptr<NetLinkInfo> pInfo = std::make_unique<NetLinkInfo>().release();
    auto network = supplier->GetNetwork();
    if (network != nullptr && pInfo != nullptr) {
        *pInfo = network->GetNetLinkInfo();
    }
    callback->NetConnectionPropertiesChange(netHandle, pInfo);
    NetsysController::GetInstance().NotifyNetBearerTypeChange(pNetAllCap->bearerTypes_);
}

void NetConnService::MakeDefaultNetWork(sptr<NetSupplier> &oldSupplier, sptr<NetSupplier> &newSupplier)
{
    NETMGR_LOG_I(
        "oldSupplier[%{public}d, %{public}s], newSupplier[%{public}d, %{public}s], old equals "
        "new is [%{public}d]", oldSupplier ? oldSupplier->GetSupplierId() : 0,
        oldSupplier ? oldSupplier->GetNetSupplierIdent().c_str() : "null",
        newSupplier ? newSupplier->GetSupplierId() : 0,
        newSupplier ? newSupplier->GetNetSupplierIdent().c_str() : "null", oldSupplier == newSupplier);
    if (oldSupplier == newSupplier) {
        NETMGR_LOG_D("old supplier equal to new supplier.");
        return;
    }
    if (oldSupplier != nullptr) {
        oldSupplier->ClearDefault();
    }
    if (newSupplier != nullptr) {
        newSupplier->SetDefault();
    }
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    oldSupplier = newSupplier;
}

void NetConnService::HandleDetectionResult(uint32_t supplierId, NetDetectionStatus netState)
{
    NETMGR_LOG_I("Enter HandleDetectionResult, ifValid[%{public}d]", netState);
    auto supplier = FindNetSupplier(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier doesn't exist.");
        return;
    }
    std::unique_lock<std::recursive_mutex> locker(netManagerMutex_);
    supplier->SetNetValid(netState);
    supplier->SetDetectionDone();
    locker.unlock();
    CallbackForSupplier(supplier, CALL_TYPE_UPDATE_CAP);
    bool ifValid = netState == VERIFICATION_STATE;
    if (defaultNetSupplier_ && defaultNetSupplier_->GetNetSupplierType() != BEARER_CELLULAR) {
        RemoveDelayNetwork();
    }
    if (delaySupplierId_ == supplierId &&
        isDelayHandleFindBestNetwork_ && supplier->GetNetSupplierType() == BEARER_WIFI && ifValid) {
        NETMGR_LOG_I("Enter HandleDetectionResult delay");
    } else {
        FindBestNetworkForAllRequest();
    }
    if (!ifValid && defaultNetSupplier_ && defaultNetSupplier_->GetSupplierId() == supplierId) {
        RequestAllNetworkExceptDefault();
    }
    NETMGR_LOG_I("Enter HandleDetectionResult end");
}

std::list<sptr<NetSupplier>> NetConnService::GetNetSupplierFromList(NetBearType bearerType, const std::string &ident)
{
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    std::list<sptr<NetSupplier>> ret;
    for (const auto &netSupplier : netSuppliers_) {
        if (netSupplier.second == nullptr) {
            continue;
        }
        if ((bearerType != netSupplier.second->GetNetSupplierType())) {
            continue;
        }
        if (!ident.empty() && netSupplier.second->GetNetSupplierIdent() != ident) {
            continue;
        }
        ret.push_back(netSupplier.second);
    }
    return ret;
}

sptr<NetSupplier> NetConnService::GetNetSupplierFromList(NetBearType bearerType, const std::string &ident,
                                                         const std::set<NetCap> &netCaps)
{
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    for (const auto &netSupplier : netSuppliers_) {
        if (netSupplier.second == nullptr) {
            continue;
        }
        if ((bearerType == netSupplier.second->GetNetSupplierType()) &&
            (ident == netSupplier.second->GetNetSupplierIdent()) && netSupplier.second->CompareNetCaps(netCaps)) {
            return netSupplier.second;
        }
    }
    return nullptr;
}

int32_t NetConnService::GetDefaultNet(int32_t &netId)
{
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    if (!defaultNetSupplier_) {
        NETMGR_LOG_D("not found the netId");
        return NETMANAGER_SUCCESS;
    }

    netId = defaultNetSupplier_->GetNetId();
    NETMGR_LOG_D("GetDefaultNet found the netId: [%{public}d]", netId);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetAddressesByName(const std::string &host, int32_t netId, std::vector<INetAddr> &addrList)
{
    return NetManagerCenter::GetInstance().GetAddressesByName(host, static_cast<uint16_t>(netId), addrList);
}

int32_t NetConnService::GetAddressByName(const std::string &host, int32_t netId, INetAddr &addr)
{
    std::vector<INetAddr> addrList;
    int ret = GetAddressesByName(host, netId, addrList);
    if (ret == NETMANAGER_SUCCESS) {
        if (!addrList.empty()) {
            addr = addrList[0];
            return ret;
        }
        return NET_CONN_ERR_NO_ADDRESS;
    }
    return ret;
}

int32_t NetConnService::GetSpecificNet(NetBearType bearerType, std::list<int32_t> &netIdList)
{
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        NETMGR_LOG_E("netType parameter invalid");
        return NET_CONN_ERR_NET_TYPE_NOT_FOUND;
    }

    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        if (iterSupplier->second == nullptr) {
            continue;
        }
        auto supplierType = iterSupplier->second->GetNetSupplierType();
        if (bearerType == supplierType) {
            netIdList.push_back(iterSupplier->second->GetNetId());
        }
    }
    NETMGR_LOG_D("netSuppliers_ size[%{public}zd] networks_ size[%{public}zd]", netSuppliers_.size(), networks_.size());
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetSpecificNetByIdent(NetBearType bearerType, const std::string &ident,
                                              std::list<int32_t> &netIdList)
{
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        NETMGR_LOG_E("netType parameter invalid");
        return NET_CONN_ERR_NET_TYPE_NOT_FOUND;
    }

    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        if (iterSupplier->second == nullptr) {
            continue;
        }
        auto supplierType = iterSupplier->second->GetNetSupplierType();
        std::string supplierIdent = iterSupplier->second->GetNetSupplierIdent();
        if (bearerType == supplierType && ident == supplierIdent) {
            netIdList.push_back(iterSupplier->second->GetNetId());
        }
    }
    NETMGR_LOG_D("netSuppliers_ size[%{public}zd] networks_ size[%{public}zd]", netSuppliers_.size(), networks_.size());
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetAllNetsAsync(std::list<int32_t> &netIdList)
{
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    auto currentUid = IPCSkeleton::GetCallingUid();
    for (const auto &network : networks_) {
        if (network.second != nullptr && network.second->IsConnected()) {
            auto netId = network.second->GetNetId();
            sptr<NetSupplier> curSupplier = FindNetSupplier(network.second->GetSupplierId());
            // inner virtual interface and uid is not trusted, skip
            if (curSupplier != nullptr &&
                curSupplier->HasNetCap(NetCap::NET_CAPABILITY_INTERNAL_DEFAULT) &&
                !IsInRequestNetUids(currentUid)) {
                NETMGR_LOG_D("Network [%{public}d] is internal, uid [%{public}d] skips.", netId, currentUid);
                continue;
            }
#ifdef SUPPORT_SYSVPN
            if (!IsCallingUserSupplier(curSupplier->GetSupplierId())) {
                continue;
            }
#endif // SUPPORT_SYSVPN
            netIdList.push_back(netId);
        }
    }
    NETMGR_LOG_D("netSuppliers_ size[%{public}zd] netIdList size[%{public}zd]", netSuppliers_.size(), netIdList.size());
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetAllNets(std::list<int32_t> &netIdList)
{
#ifdef SUPPORT_SYSVPN
    realCallingUid_ = IPCSkeleton::GetCallingUid();
#endif // SUPPORT_SYSVPN
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, &netIdList, &result]() {
            result = this->GetAllNetsAsync(netIdList);
        });
    }
    return result;
}

bool NetConnService::IsInRequestNetUids(int32_t uid)
{
    return internalDefaultUidRequest_.count(uid) > 0;
}

int32_t NetConnService::GetSpecificUidNet(int32_t uid, int32_t &netId)
{
    NETMGR_LOG_D("Enter GetSpecificUidNet, uid is [%{public}d].", uid);
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    netId = INVALID_NET_ID;
    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        if ((iterSupplier->second != nullptr) && (uid == iterSupplier->second->GetSupplierUid()) &&
            (iterSupplier->second->GetNetSupplierType() == BEARER_VPN)) {
            netId = iterSupplier->second->GetNetId();
            return NETMANAGER_SUCCESS;
        }
    }
    if (defaultNetSupplier_ != nullptr) {
        netId = defaultNetSupplier_->GetNetId();
    }
    NETMGR_LOG_D("GetDefaultNet found the netId: [%{public}d]", netId);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetConnectionProperties(int32_t netId, NetLinkInfo &info)
{
#ifdef SUPPORT_SYSVPN
    realCallingUid_ = IPCSkeleton::GetCallingUid();
#endif // SUPPORT_SYSVPN
    if (netConnEventHandler_ == nullptr) {
        NETMGR_LOG_E("netConnEventHandler_ is nullptr.");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    int32_t result = NETMANAGER_SUCCESS;
    netConnEventHandler_->PostSyncTask([netId, &info, &result, this]() {
        auto iterNetwork = networks_.find(netId);
        if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr)) {
            result = NET_CONN_ERR_INVALID_NETWORK;
            return;
        }
#ifdef SUPPORT_SYSVPN
        if (!IsCallingUserSupplier(iterNetwork->second->GetSupplierId())) {
            result = NET_CONN_ERR_INVALID_NETWORK;
            return;
        }
#endif // SUPPORT_SYSVPN
        info = iterNetwork->second->GetNetLinkInfo();
        if (info.mtu_ == 0) {
            info.mtu_ = DEFAULT_MTU;
        }
    });
    return result;
}

int32_t NetConnService::GetNetCapabilities(int32_t netId, NetAllCapabilities &netAllCap)
{
#ifdef SUPPORT_SYSVPN
    realCallingUid_ = IPCSkeleton::GetCallingUid();
#endif // SUPPORT_SYSVPN
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    NET_SUPPLIER_MAP::iterator iterSupplier;
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        if ((iterSupplier->second != nullptr) && (netId == iterSupplier->second->GetNetId())) {
#ifdef SUPPORT_SYSVPN
            if (!IsCallingUserSupplier(iterSupplier->second->GetSupplierId())) {
                return NET_CONN_ERR_INVALID_NETWORK;
            }
#endif // SUPPORT_SYSVPN
            netAllCap = iterSupplier->second->GetNetCapabilities();
            return NETMANAGER_SUCCESS;
        }
    }
    return NET_CONN_ERR_INVALID_NETWORK;
}

int32_t NetConnService::GetIfaceNames(NetBearType bearerType, std::list<std::string> &ifaceNames)
{
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        return NET_CONN_ERR_NET_TYPE_NOT_FOUND;
    }

    if (netConnEventHandler_ == nullptr) {
        NETMGR_LOG_E("netConnEventHandler_ is nullptr.");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netConnEventHandler_->PostSyncTask([bearerType, &ifaceNames, this]() {
        auto suppliers = GetNetSupplierFromList(bearerType);
        for (auto supplier : suppliers) {
            if (supplier == nullptr) {
                continue;
            }
            std::shared_ptr<Network> network = supplier->GetNetwork();
            if (network == nullptr) {
                continue;
            }
            std::string ifaceName = network->GetIfaceName();
            if (!ifaceName.empty()) {
                ifaceNames.push_back(ifaceName);
            }
        }
    });
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetIfaceNameByType(NetBearType bearerType, const std::string &ident, std::string &ifaceName)
{
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        NETMGR_LOG_E("netType parameter invalid");
        return NET_CONN_ERR_NET_TYPE_NOT_FOUND;
    }
    if (netConnEventHandler_ == nullptr) {
        NETMGR_LOG_E("netConnEventHandler_ is nullptr.");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    int32_t result = NETMANAGER_SUCCESS;
    netConnEventHandler_->PostSyncTask([bearerType, &ifaceName, &ident, &result, this]() {
        auto suppliers = GetNetSupplierFromList(bearerType, ident);
        if (suppliers.empty()) {
            NETMGR_LOG_D("supplier is nullptr.");
            result = NET_CONN_ERR_NO_SUPPLIER;
            return;
        }
        auto supplier = suppliers.front();
        if (supplier == nullptr) {
            NETMGR_LOG_E("supplier is nullptr");
            result = NETMANAGER_ERR_LOCAL_PTR_NULL;
            return;
        }
        std::shared_ptr<Network> network = supplier->GetNetwork();
        if (network == nullptr) {
            NETMGR_LOG_E("network is nullptr");
            result = NET_CONN_ERR_INVALID_NETWORK;
            return;
        }
        ifaceName = network->GetIfaceName();
    });
    return result;
}

int32_t NetConnService::GetIfaceNameIdentMaps(NetBearType bearerType,
                                              SafeMap<std::string, std::string> &ifaceNameIdentMaps)
{
    if (bearerType < BEARER_CELLULAR || bearerType >= BEARER_DEFAULT) {
        return NET_CONN_ERR_NET_TYPE_NOT_FOUND;
    }

    if (netConnEventHandler_ == nullptr) {
        NETMGR_LOG_E("netConnEventHandler_ is nullptr.");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netConnEventHandler_->PostSyncTask([bearerType, &ifaceNameIdentMaps, this]() {
        NETMGR_LOG_I("Enter GetIfaceNameIdentMaps, netBearType=%{public}d", bearerType);
        ifaceNameIdentMaps.Clear();
        auto suppliers = GetNetSupplierFromList(bearerType);
        for (auto supplier: suppliers) {
            if (supplier == nullptr || !supplier->HasNetCap(NET_CAPABILITY_INTERNET)) {
                continue;
            }
            std::shared_ptr <Network> network = supplier->GetNetwork();
            if (network == nullptr || !network->IsConnected()) {
                continue;
            }
            std::string ifaceName = network->GetIfaceName();
            if (ifaceName.empty()) {
                continue;
            }
            std::string ident = network->GetIdent();
            ifaceNameIdentMaps.EnsureInsert(std::move(ifaceName), std::move(ident));
        }
    });
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetGlobalHttpProxy(HttpProxy &httpProxy)
{
    if (httpProxy.GetUserId() == ROOT_USER_ID || httpProxy.GetUserId() == INVALID_USER_ID) {
        LoadGlobalHttpProxy(ACTIVE, httpProxy);
    }
    if (httpProxy.GetUserId() > 0) {
        // if the valid userId is given. so load http proxy from specified user.
        LoadGlobalHttpProxy(SPECIFY, httpProxy);
    } else {
        // executed in the caller process, so load http proxy from local user which the process belongs.
        LoadGlobalHttpProxy(LOCAL, httpProxy);
    }
    if (httpProxy.GetHost().empty()) {
        httpProxy.SetPort(0);
        NETMGR_LOG_E("The http proxy host is empty");
        return NETMANAGER_SUCCESS;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetDefaultHttpProxy(int32_t bindNetId, HttpProxy &httpProxy)
{
    NETMGR_LOG_D("GetDefaultHttpProxy userId[%{public}d]", httpProxy.GetUserId());
    auto startTime = std::chrono::steady_clock::now();
    if (httpProxy.GetUserId() == ROOT_USER_ID || httpProxy.GetUserId() == INVALID_USER_ID) {
        LoadGlobalHttpProxy(ACTIVE, httpProxy);
    } else if (httpProxy.GetUserId() > 0) {
        // if the valid userId is given. so load http proxy from specified user.
        LoadGlobalHttpProxy(SPECIFY, httpProxy);
    } else {
        // executed in the caller process, so load http proxy from local user which the process belongs.
        LoadGlobalHttpProxy(LOCAL, httpProxy);
    }
    if (!httpProxy.GetHost().empty()) {
        NETMGR_LOG_D("Return global http proxy as default.");
        return NETMANAGER_SUCCESS;
    }

    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    auto iter = networks_.find(bindNetId);
    if ((iter != networks_.end()) && (iter->second != nullptr)) {
        httpProxy = iter->second->GetHttpProxy();
        NETMGR_LOG_I("Return bound network's http proxy as default.");
        return NETMANAGER_SUCCESS;
    }

    if (defaultNetSupplier_ != nullptr) {
        defaultNetSupplier_->GetHttpProxy(httpProxy);
        auto endTime = std::chrono::steady_clock::now();
        auto durationNs = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
        NETMGR_LOG_D("Use default http proxy, cost=%{public}lld",  durationNs.count());
        return NETMANAGER_SUCCESS;
    }
    auto endTime = std::chrono::steady_clock::now();
    auto durationNs = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
    NETMGR_LOG_I("No default http proxy, durationNs=%{public}lld", durationNs.count());
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetNetIdByIdentifier(const std::string &ident, std::list<int32_t> &netIdList)
{
    if (ident.empty()) {
        NETMGR_LOG_E("The identifier in service is null");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    for (auto iterSupplier : netSuppliers_) {
        if (iterSupplier.second == nullptr) {
            continue;
        }
        if (iterSupplier.second->GetNetSupplierIdent() == ident) {
            int32_t netId = iterSupplier.second->GetNetId();
            netIdList.push_back(netId);
        }
    }
    return NETMANAGER_SUCCESS;
}

void NetConnService::GetDumpMessage(std::string &message)
{
    message.append("Net connect Info:\n");
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    if (defaultNetSupplier_) {
        message.append("\tSupplierId: " + std::to_string(defaultNetSupplier_->GetSupplierId()) + "\n");
        std::shared_ptr<Network> network = defaultNetSupplier_->GetNetwork();
        if (network) {
            message.append("\tNetId: " + std::to_string(network->GetNetId()) + "\n");
        } else {
            message.append("\tNetId: " + std::to_string(INVALID_NET_ID) + "\n");
        }
        message.append("\tConnStat: " + std::to_string(defaultNetSupplier_->IsConnected()) + "\n");
        message.append("\tIsAvailable: " + std::to_string(defaultNetSupplier_->IsNetValidated()) + "\n");
        message.append("\tIsRoaming: " + std::to_string(defaultNetSupplier_->GetRoaming()) + "\n");
        message.append("\tStrength: " + std::to_string(defaultNetSupplier_->GetStrength()) + "\n");
        message.append("\tFrequency: " + std::to_string(defaultNetSupplier_->GetFrequency()) + "\n");
        message.append("\tLinkUpBandwidthKbps: " +
                       std::to_string(defaultNetSupplier_->GetNetCapabilities().linkUpBandwidthKbps_) + "\n");
        message.append("\tLinkDownBandwidthKbps: " +
                       std::to_string(defaultNetSupplier_->GetNetCapabilities().linkDownBandwidthKbps_) + "\n");
        message.append("\tUid: " + std::to_string(defaultNetSupplier_->GetSupplierUid()) + "\n");
    } else {
        message.append("\tdefaultNetSupplier_ is nullptr\n");
        message.append("\tSupplierId: \n");
        message.append("\tNetId: 0\n");
        message.append("\tConnStat: 0\n");
        message.append("\tIsAvailable: \n");
        message.append("\tIsRoaming: 0\n");
        message.append("\tStrength: 0\n");
        message.append("\tFrequency: 0\n");
        message.append("\tLinkUpBandwidthKbps: 0\n");
        message.append("\tLinkDownBandwidthKbps: 0\n");
        message.append("\tUid: 0\n");
    }
    if (dnsResultCallback_ != nullptr) {
        dnsResultCallback_->GetDumpMessageForDnsResult(message);
    }
}

int32_t NetConnService::HasDefaultNet(bool &flag)
{
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    if (!defaultNetSupplier_) {
        flag = false;
        return NETMANAGER_SUCCESS;
    }
    flag = true;
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::IsDefaultNetMetered(bool &isMetered)
{
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    if (defaultNetSupplier_) {
        isMetered = !defaultNetSupplier_->HasNetCap(NET_CAPABILITY_NOT_METERED);
    } else {
        isMetered = true;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::BindSocket(int32_t socketFd, int32_t netId)
{
    NETMGR_LOG_D("Enter BindSocket.");
    return NetsysController::GetInstance().BindSocket(socketFd, netId);
}

int32_t NetConnService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    NETMGR_LOG_D("Start Dump, fd: %{public}d", fd);
    std::string result;
    GetDumpMessage(result);
    int32_t ret = dprintf(fd, "%s\n", result.c_str());
    return (ret < 0) ? static_cast<int32_t>(NET_CONN_ERR_CREATE_DUMP_FAILED) : static_cast<int32_t>(NETMANAGER_SUCCESS);
}

static bool ConvertStrToLong(const std::string &str, int64_t &value)
{
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    return ec == std::errc{} && ptr == str.data() + str.size();
}

bool NetConnService::IsValidDecValue(const std::string &inputValue)
{
    if (inputValue.length() > INPUT_VALUE_LENGTH) {
        NETMGR_LOG_E("The value entered is out of range, value:%{public}s", inputValue.c_str());
        return false;
    }
    bool isValueNumber = regex_match(inputValue, std::regex("(-[\\d+]+)|(\\d+)"));
    if (isValueNumber) {
        int64_t numberValue = INT64_MAX;
        bool isSuccess = ConvertStrToLong(inputValue, numberValue);
        if (isSuccess && (numberValue >= INT32_MIN) && (numberValue <= INT32_MAX)) {
            return true;
        }
    }
    NETMGR_LOG_I("InputValue is not a decimal number");
    return false;
}

int32_t NetConnService::GetDelayNotifyTime()
{
    char param[SYS_PARAMETER_SIZE] = { 0 };
    int32_t delayTime = 0;
    int32_t code = GetParameter(CFG_NETWORK_PRE_AIRPLANE_MODE_WAIT_TIMES, NO_DELAY_TIME_CONFIG,
                                param, SYS_PARAMETER_SIZE);
    std::string time = param;
    if (code <= 0 || !IsValidDecValue(time)) {
        try {
            delayTime = std::stoi(NO_DELAY_TIME_CONFIG);
        } catch (const std::invalid_argument& e) {
            NETMGR_LOG_E("invalid_argument");
            return delayTime;
        } catch (const std::out_of_range& e) {
            NETMGR_LOG_E("out_of_range");
            return delayTime;
        }
    } else {
        try {
            auto tmp = std::stoi(time);
            delayTime = tmp > static_cast<int32_t>(MAX_DELAY_TIME) ? std::stoi(NO_DELAY_TIME_CONFIG) : tmp;
        } catch (const std::invalid_argument& e) {
            NETMGR_LOG_E("invalid_argument");
            return delayTime;
        } catch (const std::out_of_range& e) {
            NETMGR_LOG_E("out_of_range");
            return delayTime;
        }
    }
    NETMGR_LOG_D("delay time is %{public}d", delayTime);
    return delayTime;
}

int32_t NetConnService::RegisterPreAirplaneCallback(const sptr<IPreAirplaneCallback> callback)
{
    int32_t callingUid = static_cast<int32_t>(IPCSkeleton::GetCallingUid());
    NETMGR_LOG_D("RegisterPreAirplaneCallback, calllinguid [%{public}d]", callingUid);
    std::lock_guard guard(preAirplaneCbsMutex_);
    preAirplaneCallbacks_[callingUid] = callback;
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::UnregisterPreAirplaneCallback(const sptr<IPreAirplaneCallback> callback)
{
    int32_t callingUid = static_cast<int32_t>(IPCSkeleton::GetCallingUid());
    NETMGR_LOG_D("UnregisterPreAirplaneCallback, calllinguid [%{public}d]", callingUid);
    std::lock_guard guard(preAirplaneCbsMutex_);
    preAirplaneCallbacks_.erase(callingUid);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::SetAirplaneMode(bool state)
{
    NETMGR_LOG_I("Enter SetAirplaneMode, AirplaneMode is %{public}d", state);
    if (state && system::GetBoolParameter(PERSIST_EDM_AIRPLANE_MODE_DISABLE, false)) {
        NETMGR_LOG_E("SetAirplaneMode policy is disallowed");
        return NET_CONN_ERR_POLICY_DISABLED;
    }
    if (state) {
        std::lock_guard guard(preAirplaneCbsMutex_);
        for (const auto& mem : preAirplaneCallbacks_) {
            if (mem.second != nullptr) {
                int32_t ret = mem.second->PreAirplaneStart();
                NETMGR_LOG_D("PreAirplaneStart result %{public}d", ret);
            }
        }
    }
    if (netConnEventHandler_ == nullptr) {
        NETMGR_LOG_E("netConnEventHandler_ is nullptr.");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netConnEventHandler_->RemoveAsyncTask("delay airplane mode");
    auto delayTime = GetDelayNotifyTime();

    bool ret = netConnEventHandler_->PostAsyncTask(
        [state]() {
            NETMGR_LOG_I("Enter delay");
            auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
            std::string airplaneMode = std::to_string(state);
            Uri uri(AIRPLANE_MODE_URI);
            int32_t ret = dataShareHelperUtils->Update(uri, KEY_AIRPLANE_MODE, airplaneMode);
            if (ret != NETMANAGER_SUCCESS) {
                NETMGR_LOG_E("Update airplane mode:%{public}d to datashare failed.", state);
                return;
            }
            BroadcastInfo info;
            info.action = EventFwk::CommonEventSupport::COMMON_EVENT_AIRPLANE_MODE_CHANGED;
            info.data = "Net Manager Airplane Mode Changed";
            info.code = static_cast<int32_t>(state);
            info.ordered = false;
            std::map<std::string, int32_t> param;
            BroadcastManager::GetInstance().SendBroadcast(info, param);
        },
        "delay airplane mode", delayTime);
    NETMGR_LOG_I("SetAirplaneMode out [%{public}d]", ret);

    return NETMANAGER_SUCCESS;
}

void NetConnService::ActiveHttpProxy()
{
    NETMGR_LOG_D("ActiveHttpProxy thread start");
    uint32_t retryTimes = RETRY_TIMES;
    while (httpProxyThreadNeedRun_.load()) {
        NETMGR_LOG_D("Keep global http-proxy active every 2 minutes");
        CURL *curl = nullptr;
        HttpProxy tempProxy;
        {
            auto userInfoHelp = NetProxyUserinfo::GetInstance();
            // executed in the SA process, so load http proxy from current active user.
            LoadGlobalHttpProxy(ACTIVE, tempProxy);
            userInfoHelp.GetHttpProxyHostPass(tempProxy);
        }
        if (!tempProxy.host_.empty() && !tempProxy.username_.empty()) {
            curl = curl_easy_init();
            SetCurlOptions(curl, tempProxy);
        }
        if (curl) {
            long response_code;
            auto ret = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            NETMGR_LOG_D("SetGlobalHttpProxy ActiveHttpProxy ret: %{public}d, code: %{public}d", static_cast<int>(ret),
                         static_cast<int32_t>(response_code));
            if (response_code != SUCCESS_CODE && retryTimes == 0 && !isInSleep_.load()) {
                retryTimes = RETRY_TIMES;
            }
            curl_easy_cleanup(curl);
        }
        if (httpProxyThreadNeedRun_.load()) {
            if (retryTimes == 0) {
                std::unique_lock lock(httpProxyThreadMutex_);
                auto notifyRet = httpProxyThreadCv_.wait_for(lock, std::chrono::seconds(isInSleep_.load() ?
                    HTTP_PROXY_ACTIVE_PERIOD_IN_SLEEP_S : HTTP_PROXY_ACTIVE_PERIOD_S));
                retryTimes = (notifyRet == std::cv_status::timeout) ? 0 : RETRY_TIMES;
            } else {
                retryTimes--;
            }
        } else {
            NETMGR_LOG_W("ActiveHttpProxy has been clear.");
        }
    }
}

void NetConnService::SetCurlOptions(CURL *curl, HttpProxy tempProxy)
{
    std::string httpUrl;
    GetHttpUrlFromConfig(httpUrl);
    if (httpUrl.empty()) {
        NETMGR_LOG_E("ActiveHttpProxy thread get url failed!");
        return;
    }
    auto proxyType = (tempProxy.host_.find("https://") != std::string::npos) ? CURLPROXY_HTTPS : CURLPROXY_HTTP;
    curl_easy_setopt(curl, CURLOPT_URL, httpUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, AUTH_TIME_OUT);
    curl_easy_setopt(curl, CURLOPT_PROXY, tempProxy.host_.c_str());
    curl_easy_setopt(curl, CURLOPT_PROXYPORT, tempProxy.port_);
    curl_easy_setopt(curl, CURLOPT_PROXYTYPE, proxyType);
    curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, tempProxy.username_.c_str());
    if (!tempProxy.password_.empty()) {
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_NTLM);
        curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_NTLM);
        curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, tempProxy.password_.c_str());
    } else {
        curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_BASIC);
    }
}

void NetConnService::GetHttpUrlFromConfig(std::string &httpUrl)
{
    if (!std::filesystem::exists(URL_CFG_FILE)) {
        NETMGR_LOG_E("File not exist (%{public}s)", URL_CFG_FILE);
        return;
    }

    std::ifstream file(URL_CFG_FILE);
    if (!file.is_open()) {
        NETMGR_LOG_E("Open file failed (%{public}s)", strerror(errno));
        return;
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string content = oss.str();
    auto pos = content.find(HTTP_URL_HEADER);
    if (pos != std::string::npos) {
        pos += strlen(HTTP_URL_HEADER);
        httpUrl = content.substr(pos, content.find(NEW_LINE_STR, pos) - pos);
    }
    NETMGR_LOG_D("Get net detection http url:[%{public}s]", httpUrl.c_str());
}

int32_t NetConnService::SetGlobalHttpProxy(const HttpProxy &httpProxy)
{
    NETMGR_LOG_I("Enter SetGlobalHttpProxy. httpproxy=%{public}zu, proxyUserid=%{public}d",
        httpProxy.GetHost().length(), httpProxy.GetUserId());
    HttpProxy oldHttpProxy;
    oldHttpProxy.SetUserId(httpProxy.GetUserId());
    GetGlobalHttpProxy(oldHttpProxy);
    if (oldHttpProxy != httpProxy) {
        HttpProxy newHttpProxy = httpProxy;
        int32_t userId = GetValidUserIdFromProxy(httpProxy);
        if (userId == INVALID_USER_ID) {
            return NETMANAGER_ERR_INTERNAL;
        }
        NETMGR_LOG_I("GlobalHttpProxy userId is %{public}d", userId);
        NetHttpProxyTracker httpProxyTracker;
        if (IsPrimaryUserId(userId)) {
            if (!httpProxyTracker.WriteToSettingsData(newHttpProxy)) {
                NETMGR_LOG_E("GlobalHttpProxy write settingDate fail.");
                return NETMANAGER_ERR_INTERNAL;
            }
        }
        if (!httpProxyTracker.WriteToSettingsDataUser(newHttpProxy, userId)) {
            NETMGR_LOG_E("GlobalHttpProxy write settingDateUser fail. userId=%{public}d", userId);
            return NETMANAGER_ERR_INTERNAL;
        }
        globalHttpProxyCache_.EnsureInsert(userId, newHttpProxy);
        SendHttpProxyChangeBroadcast(newHttpProxy);
        UpdateGlobalHttpProxy(newHttpProxy);
    }
    if (!httpProxy.GetHost().empty()) {
        httpProxyThreadCv_.notify_all();
    }
    if (!httpProxyThreadNeedRun_ && !httpProxy.GetUsername().empty()) {
        NETMGR_LOG_I("ActiveHttpProxy  user.len[%{public}zu], pwd.len[%{public}zu]", httpProxy.username_.length(),
                     httpProxy.password_.length());
        CreateActiveHttpProxyThread();
    } else if (httpProxyThreadNeedRun_ && httpProxy.GetHost().empty()) {
        httpProxyThreadNeedRun_ = false;
    }
    NETMGR_LOG_I("End SetGlobalHttpProxy.");
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetPacUrl(std::string &pacUrl)
{
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri uri(PAC_URL_URI);
    int32_t ret = dataShareHelperUtils->Query(uri, KEY_PAC_URL, pacUrl);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Query pac url failed.");
        return NETMANAGER_ERR_INTERNAL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::QueryTraceRoute(const std::string &destination, int32_t maxJumpNumber, int32_t packetsType,
    std::string &traceRouteInfo)
{
    return OHOS::NetManagerStandard::QueryTraceRouteProbeResult(destination, maxJumpNumber,
        packetsType, traceRouteInfo);
}

int32_t NetConnService::SetPacUrl(const std::string &pacUrl)
{
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri uri(PAC_URL_URI);
    int32_t ret = dataShareHelperUtils->Update(uri, KEY_PAC_URL, pacUrl);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Update pacUrl to datashare failed.");
        return NETMANAGER_ERR_INTERNAL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetPacFileUrl(std::string &pacUrl)
{
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri uri(PAC_URL_URI);
    int32_t ret = dataShareHelperUtils->Query(uri, KEY_PAC_FILE_URL, pacUrl);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_W("Query pac url failed.");
        pacUrl.clear();
    }
    return NETMANAGER_SUCCESS;
}

void NetConnService::CheckProxyStatus()
{
    OHOS::NetManagerStandard::ProxyModeType mode;
    if (GetProxyMode(mode) != NETMANAGER_SUCCESS) {
        return;
    }
    SetProxyMode(mode);
}

#ifdef NETMANAGER_ENABLE_PAC_PROXY
int NetConnService::StopPacLocalProxyServer()
{
    std::lock_guard<std::mutex> guard{netPacProxyServerMutex_};
    if (netPACProxyServer_ && netPACProxyServer_->IsRunning()) {
        netPACProxyServer_->Stop();
    }
    return 0;
}

int NetConnService::StartPacLocalProxyServer()
{
    std::lock_guard<std::mutex> guard{netPacProxyServerMutex_};
    if (netPACProxyServer_ && netPACProxyServer_->IsRunning()) {
        netPACProxyServer_->Stop();
    }
    int portStart = 1024;
    int portEnd = 65535;
    int port = ProxyServer::FindAvailablePort(portStart, portEnd);
    netPACProxyServer_ = std::make_shared<ProxyServer>(port, 0);
    netPACProxyServer_->SetFindPacProxyFunction([&](auto url, auto host) {
        std::string proxy;
        GetNetPacManager()->FindProxyForURL(url, host, proxy);
        return proxy;
    });
    bool ret = netPACProxyServer_->Start();
    if (ret) {
        HttpProxy globalProxy;
        globalProxy.SetHost("127.0.0.1");
        globalProxy.SetPort(port);
        int gpret = SetGlobalHttpProxy(globalProxy);
        if (gpret != NETMANAGER_SUCCESS) {
            netPACProxyServer_->Stop();
            NETMGR_LOG_E("SetGlobalHttpProxy failed.");
            return NETMANAGER_ERROR;
        }
        SendHttpProxyChangeBroadcast(globalProxy);
    } else {
        NETMGR_LOG_E("Start Local ProxyServer failed.");
    }
    return ret;
}

std::shared_ptr<NetPACManager> NetConnService::GetNetPacManager()
{
    std::lock_guard<std::mutex> guard{netPacManagerMutex_};
    if (!netPACManager_) {
        netPACManager_ = std::make_shared<OHOS::NetManagerStandard::NetPACManager>();
        std::string pacUrl;
        int32_t ret = GetPacFileUrl(pacUrl);
        if (ret == NETMANAGER_SUCCESS) {
            netPACManager_->InitPACScriptWithURL(pacUrl);
        }
    }
    return netPACManager_;
}

uint32_t NetConnService::SetProxyOff()
{
    StopPacLocalProxyServer();
    HttpProxy globalProxy;
    globalProxy.SetHost("");
    globalProxy.SetPort(0);
    int gpret = SetGlobalHttpProxy(globalProxy);
    if (gpret != NETMANAGER_SUCCESS)
        NETMGR_LOG_E("SetGlobalHttpProxy failed.");
    SendHttpProxyChangeBroadcast(globalProxy);
    return NETMANAGER_SUCCESS;
}

uint32_t NetConnService::SetProxyAuto()
{
    std::string pacUrl;
    if (GetPacFileUrl(pacUrl) == NETMANAGER_SUCCESS) {
        if (!GetNetPacManager()->InitPACScriptWithURL(pacUrl)) {
            NETMGR_LOG_E("InitPACScriptWithURL failed.");
            StopPacLocalProxyServer();
            return NETMANAGER_ERR_OPERATION_FAILED;
        }
        if (!StartPacLocalProxyServer()) {
            NETMGR_LOG_E("StartPacLocalProxyServer failed.");
            return NETMANAGER_ERR_OPERATION_FAILED;
        }
        return NETMANAGER_SUCCESS;
    }
    return NETMANAGER_ERR_OPERATION_FAILED;
}
#endif

int32_t NetConnService::SetProxyMode(const OHOS::NetManagerStandard::ProxyModeType mode)
{
    Uri hostUri(GLOBAL_PROXY_HOST_URI);
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    int ret = dataShareHelperUtils->Update(hostUri, KEY_PROXY_MODE, std::to_string(mode));
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Update proxy mode fail %d", mode);
        return NETMANAGER_ERR_OPERATION_FAILED;
    }
#ifdef NETMANAGER_ENABLE_PAC_PROXY
    switch (mode) {
        case PROXY_MODE_OFF: {
            return SetProxyOff();
        }
        case PROXY_MODE_AUTO:
            return SetProxyAuto();
    }
#endif
    NETMGR_LOG_E("invalide proxy mode %{public}d", mode);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetProxyMode(OHOS::NetManagerStandard::ProxyModeType &mode)
{
    Uri hostUri(GLOBAL_PROXY_HOST_URI);
    std::string modeStr;
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    int ret = dataShareHelperUtils->Query(hostUri, KEY_PROXY_MODE, modeStr);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_W("get proxy mode fail");
        mode = PROXY_MODE_OFF;
        return NETMANAGER_SUCCESS;
    }
    int temp = CommonUtils::StrToInt(modeStr);
    switch (temp) {
        case PROXY_MODE_OFF:
            mode = PROXY_MODE_OFF;
            break;
        case PROXY_MODE_AUTO:
            mode = PROXY_MODE_AUTO;
            break;
        default:
            NETMGR_LOG_E("invalide proxy mode %{public}d", temp);
            return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::SetPacFileUrl(const std::string &pacUrl)
{
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri uri(PAC_URL_URI);
    int32_t ret = dataShareHelperUtils->Update(uri, KEY_PAC_FILE_URL, pacUrl);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Update pacUrl to datashare failed.");
        return NETMANAGER_ERR_OPERATION_FAILED;
    }
#ifdef NETMANAGER_ENABLE_PAC_PROXY
    if (pacUrl.empty()) {
        return SetProxyOff();
    } else {
        return SetProxyAuto();
    }
#endif
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::FindProxyForURL(const std::string &url, const std::string &host, std::string &proxy)
{
#ifdef NETMANAGER_ENABLE_PAC_PROXY
    return GetNetPacManager()->FindProxyForURL(url, host, proxy);
#else
     proxy.clear();
     return NETMANAGER_SUCCESS;
#endif
}

void NetConnService::CreateActiveHttpProxyThread()
{
    httpProxyThreadNeedRun_ = true;
    std::thread t([sp = shared_from_this()]() { sp->ActiveHttpProxy(); });
    std::string threadName = "ActiveHttpProxy";
    pthread_setname_np(t.native_handle(), threadName.c_str());
    t.detach();
}

int32_t NetConnService::GetLocalUserId(int32_t &userId)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    int ret = AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
    if (ret != 0) {
        NETMGR_LOG_E("GetOsAccountLocalIdFromUid failed. uid is %{public}d, ret is %{public}d", uid, ret);
        return NETMANAGER_ERR_INTERNAL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetActiveUserId(int32_t &userId)
{
    std::vector<int> activeIds;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds);
    if (ret != 0) {
        NETMGR_LOG_E("QueryActiveOsAccountIds failed. ret is %{public}d", ret);
        return NETMANAGER_ERR_INTERNAL;
    }
    if (activeIds.empty()) {
        NETMGR_LOG_E("QueryActiveOsAccountIds is empty");
        return NETMANAGER_ERR_INTERNAL;
    }
    userId = activeIds[0];
    return NETMANAGER_SUCCESS;
}

bool NetConnService::IsValidUserId(int32_t userId)
{
    if (userId < 0) {
        return false;
    }
    bool isValid = false;
    auto ret = AccountSA::OsAccountManager::IsOsAccountExists(userId, isValid);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("IsOsAccountExists is failed. ret[%{public}d], userId[%{public}d]", ret, userId);
        return false;
    }
    return isValid;
}

int32_t NetConnService::GetValidUserIdFromProxy(const HttpProxy &httpProxy)
{
    int32_t userId;
    if (httpProxy.GetUserId() == ROOT_USER_ID || httpProxy.GetUserId() == INVALID_USER_ID) {
        int32_t ret = GetActiveUserId(userId);
        if (ret != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("GetValidUserIdFromProxy failed to get active userId.");
            return INVALID_USER_ID;
        }
    } else {
        userId = httpProxy.GetUserId();
    }
    return userId;
}

int32_t NetConnService::SetAppNet(int32_t netId)
{
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::RegisterNetInterfaceCallback(const sptr<INetInterfaceStateCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("callback is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    NETMGR_LOG_I("Enter RegisterNetInterfaceCallback.");
    if (interfaceStateCallback_ == nullptr) {
        NETMGR_LOG_E("interfaceStateCallback_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return interfaceStateCallback_->RegisterInterfaceCallback(callback);
}

int32_t NetConnService::UnregisterNetInterfaceCallback(const sptr<INetInterfaceStateCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("callback is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    NETMGR_LOG_I("Enter UnregisterNetInterfaceCallback.");
    if (interfaceStateCallback_ == nullptr) {
        NETMGR_LOG_E("interfaceStateCallback_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return interfaceStateCallback_->UnregisterInterfaceCallback(callback);
}

int32_t NetConnService::GetNetInterfaceConfiguration(const std::string &iface, NetInterfaceConfiguration &config)
{
    using namespace OHOS::nmd;
    InterfaceConfigurationParcel configParcel;
    configParcel.ifName = iface;
    if (NetsysController::GetInstance().GetInterfaceConfig(configParcel) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERR_INTERNAL;
    }
    config.ifName_ = configParcel.ifName;
    config.hwAddr_ = configParcel.hwAddr;
    config.ipv4Addr_ = configParcel.ipv4Addr;
    config.prefixLength_ = configParcel.prefixLength;
    config.flags_.assign(configParcel.flags.begin(), configParcel.flags.end());
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::SetNetInterfaceIpAddress(const std::string &iface, const std::string &ipAddress)
{
    return NetsysController::GetInstance().InterfaceSetIpAddress(iface, ipAddress);
}

int32_t NetConnService::SetInterfaceUp(const std::string &iface)
{
    return NetsysController::GetInstance().SetInterfaceUp(iface);
}

int32_t NetConnService::SetInterfaceDown(const std::string &iface)
{
    return NetsysController::GetInstance().SetInterfaceDown(iface);
}

int32_t NetConnService::NetDetectionForDnsHealth(int32_t netId, bool dnsHealthSuccess)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([netId, dnsHealthSuccess, &result, this]() {
            result = this->NetDetectionForDnsHealthSync(netId, dnsHealthSuccess);
        });
    }
    return result;
}

// Query the global http proxy of a specified user type.
// The user type can be ACTIVE or LOCAL.
// The ACTIVE is the user in active state on the foreground.
// The LOCAL is the user to which the application process belongs.
void NetConnService::LoadGlobalHttpProxy(UserIdType userIdType, HttpProxy &httpProxy)
{
    int32_t userId = -1;
    int32_t ret = NETMANAGER_SUCCESS;
    if (userIdType == ACTIVE) {
        ret = GetActiveUserId(userId);
    } else if (userIdType == LOCAL) {
        ret = GetLocalUserId(userId);
        if (userId == ROOT_USER_ID) {
            ret = GetActiveUserId(userId);
        }
    } else if (userIdType == SPECIFY) {
        userId = httpProxy.GetUserId();
    } else {
        NETMGR_LOG_E("LoadGlobalHttpProxy invalid userIdType.");
        return;
    }
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("LoadGlobalHttpProxy get userId fail.");
        return;
    }
    if (!IsValidUserId(userId)) {
        NETMGR_LOG_E("LoadGlobalHttpProxy userId is not exist. userId[%{public}d]", httpProxy.GetUserId());
        return;
    }
    if (globalHttpProxyCache_.Find(userId, httpProxy)) {
        NETMGR_LOG_D("Global http proxy has been loaded from the SettingsData database. userId=%{public}d", userId);
        return;
    }
    if (!isDataShareReady_.load() && !CheckIfSettingsDataReady()) {
        NETMGR_LOG_E("data share is not ready.");
        return;
    }
    NetHttpProxyTracker httpProxyTracker;
    HttpProxy tmpHttpProxy;
    if (IsPrimaryUserId(userId)) {
        httpProxyTracker.ReadFromSettingsData(tmpHttpProxy);
    } else {
        httpProxyTracker.ReadFromSettingsDataUser(tmpHttpProxy, userId);
    }
    tmpHttpProxy.SetUserId(userId);
 
    httpProxy = tmpHttpProxy;
    globalHttpProxyCache_.EnsureInsert(userId, tmpHttpProxy);
}

void NetConnService::UpdateGlobalHttpProxy(const HttpProxy &httpProxy)
{
    if (netConnEventHandler_ == nullptr) {
        NETMGR_LOG_E("netConnEventHandler_ is nullptr.");
        return;
    }
    NETMGR_LOG_I("UpdateGlobalHttpProxy start");
    netConnEventHandler_->PostAsyncTask([this, httpProxy]() {
        for (const auto &supplier : netSuppliers_) {
            if (supplier.second == nullptr) {
                continue;
            }
            supplier.second->UpdateGlobalHttpProxy(httpProxy);
        }
        NETMGR_LOG_I("UpdateGlobalHttpProxy end");
    });
}

int32_t NetConnService::NetInterfaceStateCallback::OnInterfaceAddressUpdated(const std::string &addr,
                                                                             const std::string &ifName, int flags,
                                                                             int scope)
{
    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &callback : ifaceStateCallbacks_) {
        if (callback == nullptr) {
            NETMGR_LOG_E("callback is null");
            continue;
        }
        callback->OnInterfaceAddressUpdated(addr, ifName, flags, scope);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnInterfaceAddressRemoved(const std::string &addr,
                                                                             const std::string &ifName, int flags,
                                                                             int scope)
{
    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &callback : ifaceStateCallbacks_) {
        if (callback == nullptr) {
            NETMGR_LOG_E("callback is null");
            continue;
        }
        callback->OnInterfaceAddressRemoved(addr, ifName, flags, scope);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnInterfaceAdded(const std::string &iface)
{
    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &callback : ifaceStateCallbacks_) {
        if (callback == nullptr) {
            NETMGR_LOG_E("callback is null");
            continue;
        }
        callback->OnInterfaceAdded(iface);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnInterfaceRemoved(const std::string &iface)
{
    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &callback : ifaceStateCallbacks_) {
        if (callback == nullptr) {
            NETMGR_LOG_E("callback is null");
            continue;
        }
        callback->OnInterfaceRemoved(iface);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnInterfaceChanged(const std::string &iface, bool up)
{
    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &callback : ifaceStateCallbacks_) {
        if (callback == nullptr) {
            NETMGR_LOG_E("callback is null");
            continue;
        }
        callback->OnInterfaceChanged(iface, up);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnInterfaceLinkStateChanged(const std::string &iface, bool up)
{
    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &callback : ifaceStateCallbacks_) {
        if (callback == nullptr) {
            NETMGR_LOG_E("callback is null");
            continue;
        }
        callback->OnInterfaceLinkStateChanged(iface, up);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnRouteChanged(bool updated, const std::string &route,
                                                                  const std::string &gateway, const std::string &ifName)
{
    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &callback : ifaceStateCallbacks_) {
        if (callback == nullptr) {
            NETMGR_LOG_E("callback is null");
            continue;
        }
        callback->OnRouteChanged(updated, route, gateway, ifName);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnDhcpSuccess(NetsysControllerCallback::DhcpResult &dhcpResult)
{
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::OnBandwidthReachedLimit(const std::string &limitName,
                                                                           const std::string &iface)
{
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::RegisterInterfaceCallback(
    const sptr<INetInterfaceStateCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("callback is null");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    std::lock_guard<std::mutex> locker(mutex_);
    for (const auto &iter : ifaceStateCallbacks_) {
        if (!iter) {
            continue;
        }
        if (iter->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr()) {
            NETMGR_LOG_E("RegisterInterfaceCallback find same callback");
            return NET_CONN_ERR_SAME_CALLBACK;
        }
    }
    ifaceStateCallbacks_.push_back(callback);
    NETMGR_LOG_I("Register interface callback successful");

    AddIfaceDeathRecipient(callback);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::NetInterfaceStateCallback::UnregisterInterfaceCallback(
    const sptr<INetInterfaceStateCallback> &callback)
{
    NETMGR_LOG_I("UnregisterInterfaceCallback, callingPid=%{public}d, callingUid=%{public}d",
                 IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
    
    std::lock_guard<std::mutex> locker(mutex_);
    auto isSameCallback = [&callback](const sptr<INetInterfaceStateCallback> &item) {
        return item->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr();
    };
    auto iter = std::find_if(ifaceStateCallbacks_.cbegin(), ifaceStateCallbacks_.cend(), isSameCallback);
    if (iter == ifaceStateCallbacks_.cend()) {
        NETMGR_LOG_E("UnregisterInterfaceCallback callback not found.");
        return NET_CONN_ERR_CALLBACK_NOT_FOUND;
    }

    callback->AsObject()->RemoveDeathRecipient(netIfaceStateDeathRecipient_);
    ifaceStateCallbacks_.erase(iter);
    return NETMANAGER_SUCCESS;
}

void NetConnService::NetInterfaceStateCallback::OnNetIfaceStateRemoteDied(const wptr<IRemoteObject> &remoteObject)
{
    sptr<IRemoteObject> diedRemoted = remoteObject.promote();
    if (diedRemoted == nullptr) {
        NETMGR_LOG_E("diedRemoted is null");
        return;
    }
    sptr<INetInterfaceStateCallback> callback = iface_cast<INetInterfaceStateCallback>(diedRemoted);
    
    int32_t ret = UnregisterInterfaceCallback(callback);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("UnregisterInterfaceCallback failed with code %{public}d", ret);
    }
}

void NetConnService::NetInterfaceStateCallback::AddIfaceDeathRecipient(const sptr<INetInterfaceStateCallback> &callback)
{
    if (netIfaceStateDeathRecipient_ == nullptr) {
        netIfaceStateDeathRecipient_ = new (std::nothrow) NetIfaceStateCallbackDeathRecipient(*this);
    }
    if (netIfaceStateDeathRecipient_ == nullptr) {
        NETMGR_LOG_E("netIfaceStateDeathRecipient_ is null");
        return;
    }
    if (!callback->AsObject()->AddDeathRecipient(netIfaceStateDeathRecipient_)) {
        NETMGR_LOG_E("AddNetIfaceStateCallbackDeathRecipient failed");
        return;
    }
}

int32_t NetConnService::NetPolicyCallback::NetUidPolicyChange(uint32_t uid, uint32_t policy)
{
    NETMGR_LOG_D("NetUidPolicyChange Uid=%{public}d, policy=%{public}d", uid, policy);
    auto netConnService = netConnService_.lock();
    if (netConnService == nullptr) {
        NETMGR_LOG_E("netConnService_ has destory");
        return NETMANAGER_ERROR;
    }
    if (netConnService->defaultNetSupplier_ == nullptr) {
        NETMGR_LOG_E("defaultNetSupplier_ is nullptr");
        return NETMANAGER_ERROR;
    }
    if (netConnService->netConnEventHandler_) {
        netConnService->netConnEventHandler_->PostSyncTask([this, uid, policy]() { SendNetPolicyChange(uid, policy); });
        return NETMANAGER_SUCCESS;
    }
    return NETMANAGER_ERROR;
}

void NetConnService::NetPolicyCallback::SendNetPolicyChange(uint32_t uid, uint32_t policy)
{
    auto netConnService = netConnService_.lock();
    if (netConnService == nullptr) {
        NETMGR_LOG_E("netConnService_ has destory");
        return;
    }
    sptr<NetHandle> defaultNetHandle = nullptr;
    bool metered = false;
    bool newBlocked = false;
    {
        std::lock_guard<std::recursive_mutex> locker(netConnService->netManagerMutex_);
        if (netConnService->defaultNetSupplier_ == nullptr) {
            NETMGR_LOG_E("SendNetPolicyChange defaultNetSupplier_ is nullptr");
            return;
        }
        defaultNetHandle = netConnService->defaultNetSupplier_->GetNetHandle();
        metered = netConnService->defaultNetSupplier_->HasNetCap(NET_CAPABILITY_NOT_METERED);
    }
    newBlocked = NetManagerCenter::GetInstance().IsUidNetAccess(uid, metered);
    std::vector<std::shared_ptr<NetActivate>> activates;
    {
        std::lock_guard guard(netConnService->uidActivateMutex_);
        auto it = netConnService->netUidActivates_.find(uid);
        if (it != netConnService->netUidActivates_.end()) {
            activates = it->second;
        }
    }
    for (auto &activate : activates) {
        if (activate->GetNetCallback() && activate->MatchRequestAndNetwork(netConnService->defaultNetSupplier_)) {
            NETMGR_LOG_D("NetUidPolicyChange Uid=%{public}d, policy=%{public}d", uid, policy);
            activate->GetNetCallback()->NetBlockStatusChange(defaultNetHandle, newBlocked);
        }
    }
}

int32_t NetConnService::AddNetworkRoute(int32_t netId, const std::string &ifName,
                                        const std::string &destination, const std::string &nextHop)
{
    return NetsysController::GetInstance().NetworkAddRoute(netId, ifName, destination, nextHop);
}

int32_t NetConnService::RemoveNetworkRoute(int32_t netId, const std::string &ifName,
                                           const std::string &destination, const std::string &nextHop)
{
    return NetsysController::GetInstance().NetworkRemoveRoute(netId, ifName, destination, nextHop);
}

int32_t NetConnService::AddInterfaceAddress(const std::string &ifName, const std::string &ipAddr,
                                            int32_t prefixLength)
{
    return NetsysController::GetInstance().AddInterfaceAddress(ifName, ipAddr, prefixLength);
}

int32_t NetConnService::DelInterfaceAddress(const std::string &ifName, const std::string &ipAddr,
                                            int32_t prefixLength)
{
    return NetsysController::GetInstance().DelInterfaceAddress(ifName, ipAddr, prefixLength);
}

int32_t NetConnService::AddStaticArp(const std::string &ipAddr, const std::string &macAddr,
                                     const std::string &ifName)
{
    return NetsysController::GetInstance().AddStaticArp(ipAddr, macAddr, ifName);
}

int32_t NetConnService::DelStaticArp(const std::string &ipAddr, const std::string &macAddr,
                                     const std::string &ifName)
{
    return NetsysController::GetInstance().DelStaticArp(ipAddr, macAddr, ifName);
}

int32_t NetConnService::AddStaticIpv6Addr(const std::string &ipv6Addr, const std::string &macAddr,
    const std::string &ifName)
{
    return NetsysController::GetInstance().AddStaticIpv6Addr(ipv6Addr, macAddr, ifName);
}

int32_t NetConnService::DelStaticIpv6Addr(const std::string &ipv6Addr, const std::string &macAddr,
    const std::string &ifName)
{
    return NetsysController::GetInstance().DelStaticIpv6Addr(ipv6Addr, macAddr, ifName);
}

int32_t NetConnService::RegisterSlotType(uint32_t supplierId, int32_t type)
{
    int32_t result = NETMANAGER_SUCCESS;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, supplierId, type, &result]() {
            if (netSuppliers_.find(supplierId) == netSuppliers_.end()) {
                NETMGR_LOG_E("supplierId[%{public}d] is not exits", supplierId);
                result =  NETMANAGER_ERR_INVALID_PARAMETER;
            } else {
                NETMGR_LOG_I("supplierId[%{public}d] update type[%{public}d].", supplierId, type);
                sptr<NetSupplier> supplier = netSuppliers_[supplierId];
                supplier->SetSupplierType(type);
                result =  NETMANAGER_SUCCESS;
            }
        });
    }
    return result;
}

int32_t NetConnService::GetSlotType(std::string &type)
{
    int32_t result = NETMANAGER_SUCCESS;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, &type, &result]() {
            if (defaultNetSupplier_ == nullptr) {
                NETMGR_LOG_E("supplier is nullptr");
                result =  NETMANAGER_ERR_LOCAL_PTR_NULL;
            } else {
                type = defaultNetSupplier_->GetSupplierType();
                result =  NETMANAGER_SUCCESS;
            }
        });
    }
    return result;
}

int32_t NetConnService::FactoryResetNetwork()
{
    NETMGR_LOG_I("Enter FactoryResetNetwork.");

    SetAirplaneMode(false);

    if (netFactoryResetCallback_ == nullptr) {
        NETMGR_LOG_E("netFactoryResetCallback_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    netFactoryResetCallback_->NotifyNetFactoryResetAsync();

    NETMGR_LOG_I("End FactoryResetNetwork.");
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::RegisterNetFactoryResetCallback(const sptr<INetFactoryResetCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOG_E("callback is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    NETMGR_LOG_I("Enter RegisterNetFactoryResetCallback.");
    if (netFactoryResetCallback_ == nullptr) {
        NETMGR_LOG_E("netFactoryResetCallback_ is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    return netFactoryResetCallback_->RegisterNetFactoryResetCallbackAsync(callback);
}

void NetConnService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_LOG_I("OnAddSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        if (hasSARemoved_) {
            OnNetSysRestart();
            hasSARemoved_ = false;
        }
        if (!system::GetBoolParameter(BOOTEVENT_NETSYSNATIVE_SERVICE_READY, false)) {
            system::SetParameter(BOOTEVENT_NETSYSNATIVE_SERVICE_READY, "true");
            NETMGR_LOG_I("set netsysnative service start true");
        }
    } else if (systemAbilityId == ACCESS_TOKEN_MANAGER_SERVICE_ID) {
        if (!registerToService_) {
#ifndef NETMANAGER_TEST
            if (!Publish(NetConnService::GetInstance().get())) {
                NETMGR_LOG_E("Register to sa manager failed");
            }
#endif
            registerToService_ = true;
        }
    } else if (systemAbilityId == COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID) {
        policyCallback_ = sptr<NetPolicyCallback>::MakeSptr(shared_from_this());
        int32_t registerRet = NetPolicyClient::GetInstance().RegisterNetPolicyCallback(policyCallback_);
        if (registerRet != NETMANAGER_SUCCESS) {
            NETMGR_LOG_E("Register NetPolicyCallback failed, ret =%{public}d", registerRet);
        }
    } else if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        SubscribeCommonEvent();
    }
}

void NetConnService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    NETMGR_LOG_I("OnRemoveSystemAbility systemAbilityId[%{public}d]", systemAbilityId);
    if (systemAbilityId == COMM_NETSYS_NATIVE_SYS_ABILITY_ID) {
        hasSARemoved_ = true;
    }
}

void NetConnService::SubscribeCommonEvent()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent("usual.event.DATA_SHARE_READY");
#ifdef FEATURE_SUPPORT_POWERMANAGER
    matchingSkills.AddEvent("usual.event.POWER_MANAGER_STATE_CHANGED");
#endif
    matchingSkills.AddEvent("usual.event.SCREEN_ON");
    matchingSkills.AddEvent("usual.event.SCREEN_OFF");
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    if (subscriberPtr_ == nullptr) {
        subscriberPtr_ = std::make_shared<NetConnListener>(subscribeInfo,
            [this](auto && PH1) { OnReceiveEvent(std::forward<decltype(PH1)>(PH1)); });
    }
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(subscriberPtr_)) {
        NETMGR_LOG_E("system event register fail.");
    }
}

void NetConnService::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    auto const &want = data.GetWant();
    std::string action = want.GetAction();
    if (action == "usual.event.DATA_SHARE_READY") {
        NETMGR_LOG_I("on receive data_share ready.");
        isDataShareReady_ = true;
        HttpProxy httpProxy;
        // executed in the SA process, so load http proxy from current active user.
        LoadGlobalHttpProxy(ACTIVE, httpProxy);
        UpdateGlobalHttpProxy(httpProxy);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        NETMGR_LOG_I("on receive user_switched");
        HttpProxy curProxy;
        GetGlobalHttpProxy(curProxy);
        SendHttpProxyChangeBroadcast(curProxy);
    }
#ifdef FEATURE_SUPPORT_POWERMANAGER
    if (action == "usual.event.POWER_MANAGER_STATE_CHANGED") {
        int code = data.GetCode();
        HandlePowerMgrEvent(code);
    }
#endif
    if (action == "usual.event.SCREEN_ON") {
        HandleScreenEvent(true);
    } else if (action == "usual.event.SCREEN_OFF") {
        HandleScreenEvent(false);
    }
}

bool NetConnService::IsSupplierMatchRequestAndNetwork(sptr<NetSupplier> ns)
{
    if (ns == nullptr) {
        NETMGR_LOG_E("supplier is nullptr");
        return false;
    }
    NET_ACTIVATE_MAP::iterator iterActive;
    std::shared_lock<std::shared_mutex> lock(netActivatesMutex_);
    for (iterActive = netActivates_.begin(); iterActive != netActivates_.end(); ++iterActive) {
        if (!iterActive->second) {
            continue;
        }
        if (iterActive->second->MatchRequestAndNetwork(ns)) {
            return true;
        }
    }

    return false;
}

void NetConnService::RecoverNetSys()
{
    NETMGR_LOG_I("RecoverNetSys");

    NET_SUPPLIER_MAP::iterator iter;
    for (iter = netSuppliers_.begin(); iter != netSuppliers_.end(); ++iter) {
        if (iter->second == nullptr) {
            continue;
        }
        NETMGR_LOG_D("supplier info, supplier[%{public}d, %{public}s], realScore[%{public}d], isConnected[%{public}d]",
            iter->second->GetSupplierId(), iter->second->GetNetSupplierIdent().c_str(),
            iter->second->GetRealScore(), iter->second->IsConnected());

        if ((!iter->second->IsConnected()) || (!IsSupplierMatchRequestAndNetwork(iter->second))) {
            NETMGR_LOG_D("Supplier[%{public}d] is not connected or not match request.", iter->second->GetSupplierId());
            continue;
        }
        iter->second->ResumeNetworkInfo();
    }
    if (defaultNetSupplier_ != nullptr) {
        defaultNetSupplier_->ClearDefault();
        defaultNetSupplier_ = nullptr;
    }
    FindBestNetworkForAllRequest();
}

void NetConnService::OnNetSysRestart()
{
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this]() {
            NETMGR_LOG_I("OnNetSysRestart");
            this->RecoverNetSys();
        });
    }
}

int32_t NetConnService::IsPreferCellularUrl(const std::string& url, bool& preferCellular)
{
    std::string hostName = CommonUtils::GetHostnameFromURL(url);
    static std::vector<std::string> preferredRegexList = GetPreferredRegex();
    preferCellular = std::any_of(preferredRegexList.begin(), preferredRegexList.end(),
        [&hostName](const std::string &str) -> bool {
            try {
                return std::regex_match(hostName, std::regex(str));
            } catch (const std::regex_error& e) {
                NETMGR_LOG_E("regex_match exception!");
                return false;
            }
        });
    NETMGR_LOG_I("preferCellular:%{public}d", preferCellular);
    return 0;
}

bool NetConnService::IsIfaceNameInUse(const std::string &ifaceName, int32_t netId)
{
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    for (const auto &netSupplier : netSuppliers_) {
        if (netSupplier.second->GetNetwork()->GetNetId() == netId) {
            continue;
        }
        if (!netSupplier.second->IsAvailable()) {
            continue;
        }
        if (netSupplier.second->GetNetwork()->GetIfaceName() == ifaceName) {
            return true;
        }
    }
    return false;
}

std::string NetConnService::GetNetCapabilitiesAsString(const uint32_t supplierId)
{
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    const auto iterNetSuppliers = netSuppliers_.find(supplierId);
    if (iterNetSuppliers != netSuppliers_.end() && iterNetSuppliers->second != nullptr) {
        return iterNetSuppliers->second->GetNetCapabilities().ToString(" ");
    }
    return {};
}

std::vector<std::string> NetConnService::GetPreferredRegex()
{
    std::vector<std::string> preferCellularRegexList;
    const std::string preferCellularRegexPath = "/system/etc/prefer_cellular_regex_list.txt";
    std::ifstream preferCellularFile(preferCellularRegexPath);
    if (preferCellularFile.is_open()) {
        std::string line;
        while (getline(preferCellularFile, line)) {
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());
            preferCellularRegexList.push_back(line);
        }
        preferCellularFile.close();
    } else {
        NETMGR_LOG_E("open prefer cellular url file failure.");
    }
    return preferCellularRegexList;
}

void NetConnService::OnRemoteDied(const wptr<IRemoteObject> &remoteObject)
{
    sptr<IRemoteObject> diedRemoted = remoteObject.promote();
    if (diedRemoted == nullptr) {
        NETMGR_LOG_E("diedRemoted is null");
        return;
    }
    uint32_t callingUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());
    NETMGR_LOG_D("OnRemoteDied, callingUid=%{public}u", callingUid);
    sptr<INetConnCallback> callback = iface_cast<INetConnCallback>(diedRemoted);
    UnregisterNetConnCallback(callback);
}

void NetConnService::RemoveClientDeathRecipient(const sptr<INetConnCallback> &callback)
{
    std::lock_guard<std::mutex> autoLock(remoteMutex_);
    auto iter =
        std::find_if(remoteCallback_.cbegin(), remoteCallback_.cend(), [&callback](const sptr<INetConnCallback> &item) {
            return item->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr();
        });
    if (iter == remoteCallback_.cend()) {
        return;
    }
    callback->AsObject()->RemoveDeathRecipient(deathRecipient_);
    remoteCallback_.erase(iter);
}

void NetConnService::AddClientDeathRecipient(const sptr<INetConnCallback> &callback)
{
    std::lock_guard<std::mutex> autoLock(remoteMutex_);
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new (std::nothrow) ConnCallbackDeathRecipient(*this);
    }
    if (deathRecipient_ == nullptr) {
        NETMGR_LOG_E("deathRecipient is null");
        return;
    }
    if (!callback->AsObject()->AddDeathRecipient(deathRecipient_)) {
        NETMGR_LOG_E("AddClientDeathRecipient failed");
        return;
    }
    auto iter =
        std::find_if(remoteCallback_.cbegin(), remoteCallback_.cend(), [&callback](const sptr<INetConnCallback> &item) {
            return item->AsObject().GetRefPtr() == callback->AsObject().GetRefPtr();
        });
    if (iter == remoteCallback_.cend()) {
        remoteCallback_.emplace_back(callback);
    }
}

void NetConnService::RemoveALLClientDeathRecipient()
{
    std::lock_guard<std::mutex> autoLock(remoteMutex_);
    for (auto &item : remoteCallback_) {
        item->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    remoteCallback_.clear();
    appStateAwareCallback_.OnForegroundAppChanged = nullptr;
    deathRecipient_ = nullptr;
}

std::vector<sptr<NetSupplier>> NetConnService::FindSupplierWithInternetByBearerType(
    NetBearType bearerType, const std::string &ident)
{
    std::vector<sptr<NetSupplier>> result;
    NET_SUPPLIER_MAP::iterator iterSupplier;
    std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
    for (iterSupplier = netSuppliers_.begin(); iterSupplier != netSuppliers_.end(); ++iterSupplier) {
        if (iterSupplier->second == nullptr) {
            continue;
        }
        if (!iterSupplier->second->GetNetCaps().HasNetCap(NET_CAPABILITY_INTERNET)) {
            continue;
        }
        if (iterSupplier->second->GetNetSupplierIdent() != ident) {
            continue;
        }
        std::set<NetBearType>::iterator iter = iterSupplier->second->GetNetCapabilities().bearerTypes_.find(bearerType);
        if (iter != iterSupplier->second->GetNetCapabilities().bearerTypes_.end()) {
            NETMGR_LOG_I("found supplierId[%{public}d] by bearertype[%{public}d].", iterSupplier->first, bearerType);
            result.push_back(iterSupplier->second);
        }
    }
    return result;
}

int32_t NetConnService::UpdateSupplierScore(uint32_t supplierId, uint32_t detectionStatus)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, supplierId, detectionStatus, &result]() {
            result = this->UpdateSupplierScoreAsync(supplierId, detectionStatus);
        });
    }
    return result;
}

int32_t NetConnService::UpdateSupplierScoreAsync(uint32_t supplierId, uint32_t detectionStatus)
{
    NETMGR_LOG_I("UpdateSupplierScoreAsync by supplierId[%{public}d], detectionStatus[%{public}d]",
        supplierId, detectionStatus);
    std::unique_lock<std::recursive_mutex> locker(netManagerMutex_);
    NetDetectionStatus state = static_cast<NetDetectionStatus>(detectionStatus);
    auto supplier = FindNetSupplier(supplierId);
    if (supplier == nullptr) {
        locker.unlock();
        NETMGR_LOG_E("supplier doesn't exist.");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    RemoveDelayNetwork();
    supplier->SetNetValid(state);
    locker.unlock();
    // Find best network because supplier score changed.
    FindBestNetworkForAllRequest();
    // Tell other suppliers to enable if current default supplier is not better than others.
    if (defaultNetSupplier_ && defaultNetSupplier_->GetSupplierId() == supplierId) {
        RequestAllNetworkExceptDefault();
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetDefaultSupplierId(NetBearType bearerType, const std::string &ident,
    uint32_t& supplierId)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, bearerType, ident, &supplierId, &result]() {
            result = this->GetDefaultSupplierIdAsync(bearerType, ident, supplierId);
        });
    }
    return result;
}

int32_t NetConnService::GetDefaultSupplierIdAsync(NetBearType bearerType, const std::string &ident,
    uint32_t& supplierId)
{
    NETMGR_LOG_I("GetSupplierIdAsync by type[%{public}d], ident[%{public}s]",
        bearerType, ident.c_str());
    std::vector<sptr<NetSupplier>> suppliers = FindSupplierWithInternetByBearerType(bearerType, ident);
    if (suppliers.empty()) {
        NETMGR_LOG_E("not found supplierId by bearertype[%{public}d].", bearerType);
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    uint32_t tmpSupplierId = FindSupplierForConnected(suppliers);
    if (tmpSupplierId == INVALID_SUPPLIER_ID) {
        NETMGR_LOG_E("not found supplierId");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    supplierId = tmpSupplierId;
    NETMGR_LOG_I("FindSupplierForInterface supplierId by supplierId[%{public}d].", supplierId);
    return NETMANAGER_SUCCESS;
}

uint32_t NetConnService::FindSupplierForConnected(std::vector<sptr<NetSupplier>> &suppliers)
{
    uint32_t ret = INVALID_SUPPLIER_ID;
    std::vector<sptr<NetSupplier>>::iterator iter;
    for (iter = suppliers.begin(); iter != suppliers.end(); ++iter) {
        if (*iter == nullptr) {
            continue;
        }
        if ((*iter)->IsConnected()) {
            ret = (*iter)->GetSupplierId();
            break;
        }
    }
    return ret;
}

NetConnService::NetConnListener::NetConnListener(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
    EventReceiver receiver) : EventFwk::CommonEventSubscriber(subscribeInfo), eventReceiver_(receiver) {}

void NetConnService::NetConnListener::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    if (eventReceiver_ == nullptr) {
        NETMGR_LOG_E("eventReceiver is nullptr");
        return;
    }
    NETMGR_LOG_I("NetConnListener::OnReceiveEvent(), event:[%{public}s], data:[%{public}s], code:[%{public}d]",
                 eventData.GetWant().GetAction().c_str(), eventData.GetData().c_str(), eventData.GetCode());
    eventReceiver_(eventData);
}

int32_t NetConnService::EnableVnicNetwork(const sptr<NetLinkInfo> &netLinkInfo, const std::set<int32_t> &uids)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask(
            [this, &netLinkInfo, &uids, &result]() { result = this->EnableVnicNetworkAsync(netLinkInfo, uids); });
    }
    return result;
}

int32_t NetConnService::EnableVnicNetworkAsync(const sptr<NetLinkInfo> &netLinkInfo, const std::set<int32_t> &uids)
{
    NETMGR_LOG_I("enable vnic network");

    if (vnicCreated.load()) {
        NETMGR_LOG_E("Enable Vnic Network already");
        return NETWORKVPN_ERROR_VNIC_EXIST;
    }

    uint16_t mtu = netLinkInfo->mtu_;
    if (netLinkInfo->netAddrList_.empty()) {
        NETMGR_LOG_E("the netLinkInfo netAddrList is empty");
        return NET_CONN_ERR_INVALID_NETWORK;
    }

    const std::string &tunAddr = netLinkInfo->netAddrList_.front().address_;
    int32_t prefix = netLinkInfo->netAddrList_.front().prefixlen_;
    if (!CommonUtils::IsValidIPV4(tunAddr)) {
        NETMGR_LOG_E("the netLinkInfo tunAddr is not valid");
        return NET_CONN_ERR_INVALID_NETWORK;
    }

    NETMGR_LOG_I("EnableVnicNetwork tunAddr:[%{public}s], prefix:[%{public}d]", tunAddr.c_str(), prefix);
    if (NetsysController::GetInstance().CreateVnic(mtu, tunAddr, prefix, uids) != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("EnableVnicNetwork CreateVnic failed");
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    vnicCreated = true;
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::DisableVnicNetwork()
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask(
            [this, &result]() { result = this->DisableVnicNetworkAsync(); });
    }
    return result;
}

int32_t NetConnService::DisableVnicNetworkAsync()
{
    NETMGR_LOG_I("del internal virtual network");

    if (!vnicCreated.load()) {
        NETMGR_LOG_E("cannot find vnic network");
        return NET_CONN_ERR_INVALID_NETWORK;
    }

    if (NetsysController::GetInstance().DestroyVnic() != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERR_OPERATION_FAILED;
    }
    vnicCreated = false;
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::EnableDistributedClientNet(const std::string &virnicAddr, const std::string &iif)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask(
            [this, &virnicAddr, &iif, &result]() { result = this->EnableDistributedClientNetAsync(virnicAddr, iif); });
    }
    return result;
}

int32_t NetConnService::EnableDistributedClientNetAsync(const std::string &virnicAddr, const std::string &iif)
{
    if (iif.empty()) {
        NETMGR_LOG_E("iif is empty");
        return NET_CONN_ERR_INVALID_NETWORK;
    }

    if (!CommonUtils::IsValidIPV4(virnicAddr)) {
        NETMGR_LOG_E("the virnicAddr is not valid");
        return NET_CONN_ERR_INVALID_NETWORK;
    }

    if (NetsysController::GetInstance().EnableDistributedClientNet(virnicAddr, iif) != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("EnableDistributedClientNet failed");
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::EnableDistributedServerNet(const std::string &iif, const std::string &devIface,
                                                   const std::string &dstAddr)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, &iif, &devIface, &dstAddr, &result]() {
            result = this->EnableDistributedServerNetAsync(iif, devIface, dstAddr);
        });
    }
    return result;
}

int32_t NetConnService::EnableDistributedServerNetAsync(const std::string &iif, const std::string &devIface,
                                                        const std::string &dstAddr)
{
    if (iif.empty() || devIface.empty()) {
        NETMGR_LOG_E("iif || devIface is empty");
        return NET_CONN_ERR_INVALID_NETWORK;
    }

    if (!CommonUtils::IsValidIPV4(dstAddr)) {
        NETMGR_LOG_E("the dstAddr is not valid");
        return NET_CONN_ERR_INVALID_NETWORK;
    }

    if (NetsysController::GetInstance().EnableDistributedServerNet(iif, devIface, dstAddr) != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("EnableDistributedServerNet failed");
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::DisableDistributedNet(bool isServer)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask(
            [this, isServer, &result]() { result = this->DisableDistributedNetAsync(isServer); });
    }
    return result;
}

int32_t NetConnService::DisableDistributedNetAsync(bool isServer)
{
    if (NetsysController::GetInstance().DisableDistributedNet(isServer) != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("DisableDistributedNet");
        return NETMANAGER_ERR_OPERATION_FAILED;
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::CloseSocketsUid(int32_t netId, uint32_t uid)
{
    int32_t result = NETMANAGER_ERROR;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask(
            [this, netId, uid, &result]() { result = this->CloseSocketsUidAsync(netId, uid); });
    }
    return result;
}

int32_t NetConnService::CloseSocketsUidAsync(int32_t netId, uint32_t uid)
{
    auto iterNetwork = networks_.find(netId);
    if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr)) {
        NETMGR_LOG_E("Could not find the corresponding network.");
        return NET_CONN_ERR_NETID_NOT_FOUND;
    }
    iterNetwork->second->CloseSocketsUid(uid);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::SetAppIsFrozened(uint32_t uid, bool isFrozened)
{
    int32_t result = NETMANAGER_SUCCESS;
#ifdef ENABLE_SET_APP_FROZENED
    if (netConnEventHandler_ && enableAppFrozenedCallbackLimitation_.load()) {
        netConnEventHandler_->PostSyncTask(
            [this, uid, isFrozened, &result]() { result = this->SetAppIsFrozenedAsync(uid, isFrozened); });
    }
#endif
    return result;
}

int32_t NetConnService::SetAppIsFrozenedAsync(uint32_t uid, bool isFrozened)
{
    std::lock_guard guard(uidActivateMutex_);
    auto it = netUidActivates_.find(uid);
    if ((it == netUidActivates_.end())) {
        return NETMANAGER_SUCCESS;
    }
    std::vector<std::shared_ptr<NetActivate>> activates = it->second;
    NETMGR_LOG_D("SetAppIsFrozenedAsync uid[%{public}d], isFrozened=[%{public}d].", uid, isFrozened);
    for (auto iter = activates.begin(); iter != activates.end();++iter) {
        auto curNetAct = (*iter);
        if (curNetAct->IsAppFrozened() == isFrozened) {
            continue;
        }
        curNetAct->SetIsAppFrozened(isFrozened);
        if (isFrozened) {
            continue;
        }
        sptr<NetSupplier> netSupplier = curNetAct->GetServiceSupply();
        sptr<INetConnCallback> callback = curNetAct->GetNetCallback();
        CallbackType callbackType = curNetAct->GetLastCallbackType();
        if (callbackType == CALL_TYPE_UNKNOWN) {
            continue;
        }
        if (netSupplier == nullptr) {
            if (callbackType != CALL_TYPE_LOST) {
                continue;
            }
            int32_t lastNetid = curNetAct->GetLastNetid();
            if (callback) {
                sptr<NetHandle> netHandle = sptr<NetHandle>::MakeSptr();
                netHandle->SetNetId(lastNetid);
                callback->NetLost(netHandle);
            }
        } else if (callbackType == CALL_TYPE_AVAILABLE) {
            CallbackForAvailable(netSupplier, curNetAct->GetNetCallback());
        } else {
            sptr<NetHandle> netHandle = netSupplier->GetNetHandle();
            HandleCallback(netSupplier, netHandle, callback, callbackType);
        }
        curNetAct->SetLastNetid(0);
        curNetAct->SetLastCallbackType(CALL_TYPE_UNKNOWN);
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::EnableAppFrozenedCallbackLimitation(bool flag)
{
    int32_t result = NETMANAGER_SUCCESS;
    if (netConnEventHandler_) {
        netConnEventHandler_->PostSyncTask([this, flag, &result]() {
            result = this->EnableAppFrozenedCallbackLimitationAsync(flag);
    });
    }
    return result;
}

int32_t NetConnService::EnableAppFrozenedCallbackLimitationAsync(bool flag)
{
    enableAppFrozenedCallbackLimitation_ = flag;
    NETMGR_LOG_I("enableAppFrozenedCallbackLimitation_ = %{public}d", enableAppFrozenedCallbackLimitation_.load());
    return NETMANAGER_SUCCESS;
}

bool NetConnService::IsAppFrozenedCallbackLimitation()
{
    bool ret = enableAppFrozenedCallbackLimitation_.load();
    return ret;
}

int32_t NetConnService::SetReuseSupplierId(uint32_t supplierId, uint32_t reuseSupplierId, bool isReused)
{
    NETMGR_LOG_I("SetReuseSupplierId supplierId=[%{public}d], reuseSupplierId=[%{public}d], isReused=[%{public}d].",
        supplierId, reuseSupplierId, isReused);
    {
        sptr<NetSupplier> supplier = nullptr;
        NetCap reuseCap;
        std::lock_guard<std::recursive_mutex> locker(netManagerMutex_);
        for (const auto& pNetSupplier : netSuppliers_) {
            if (pNetSupplier.second == nullptr) {
                continue;
            }
            if (pNetSupplier.second->GetSupplierId() == supplierId) {
                supplier = pNetSupplier.second;
            } else if (pNetSupplier.second->GetSupplierId() == reuseSupplierId) {
                std::set<NetCap> netCaps = pNetSupplier.second->GetNetCaps().ToSet();
                reuseCap = *netCaps.begin();
            }
        }
        if (supplier != nullptr) {
            supplier->SetReuseCap(reuseCap, isReused);
        }
    }
    if (isReused) {
        FindBestNetworkForAllRequest();
    }
    return NETMANAGER_SUCCESS;
}

sptr<NetSupplier> NetConnService::GetSupplierByNetId(int32_t netId)
{
    auto iterNetwork = networks_.find(netId);
    if ((iterNetwork == networks_.end()) || (iterNetwork->second == nullptr)) {
        NETMGR_LOG_E("Could not find the corresponding network.");
        return nullptr;
    }
    uint32_t supplierId = iterNetwork->second->GetSupplierId();
    return FindNetSupplier(supplierId);
}

int32_t NetConnService::SetNetExtAttribute(int32_t netId, const std::string &netExtAttribute)
{
    sptr<NetSupplier> supplier = GetSupplierByNetId(netId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier doesn't exist.");
        return NETMANAGER_ERR_INTERNAL;
    }
    if (netExtAttribute.length() > MAX_NET_EXT_ATTRIBUTE) {
        NETMGR_LOG_E("set netExtAttribute fail: exceed length limit");
        return NETMANAGER_ERR_OPERATION_FAILED;
    }
    supplier->SetNetExtAttribute(netExtAttribute);
    return NETMANAGER_SUCCESS;
}

int32_t NetConnService::GetNetExtAttribute(int32_t netId, std::string &netExtAttribute)
{
    sptr<NetSupplier> supplier = GetSupplierByNetId(netId);
    if (supplier == nullptr) {
        NETMGR_LOG_E("supplier doesn't exist.");
        return NETMANAGER_ERR_INTERNAL;
    }
    netExtAttribute = supplier->GetNetExtAttribute();
    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
