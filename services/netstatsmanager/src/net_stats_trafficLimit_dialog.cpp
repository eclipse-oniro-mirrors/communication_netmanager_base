/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <fstream>
#include <memory>
#include <set>
#include <string_ex.h>
#include <string>

#include <ability_manager_client.h>
#include <message_parcel.h>

#include "net_stats_rdb.h"
#include "net_stats_trafficLimit_dialog.h"
#include "net_stats_service.h"
#include "net_stats_utils.h"
#include "cellular_data_client.h"
#include "core_service_client.h"
#include "cJSON.h"
#include "net_mgr_log_wrapper.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t INVALID_USERID = -1;
constexpr int32_t MESSAGE_PARCEL_KEY_SIZE = 6;

TrafficLimitDialog::TrafficLimitDialog() {}

TrafficLimitDialog::~TrafficLimitDialog()
{
    if (isDialogOpen_) {
        (void)UnShowTrafficLimitDialog();
    }
}

bool TrafficLimitDialog::PopUpTrafficLimitDialog()
{
    isDialogOpen_ = true;
    return ShowTrafficLimitDialog();
}

bool TrafficLimitDialog::DismissTrafficLimitDialog()
{
    if (!isDialogOpen_) {
        return false;
    }
    NETMGR_LOG_I("Dismiss TrafficLimit Dialog");
    return UnShowTrafficLimitDialog();
}

void TrafficLimitDialog::TrafficLimitAbilityConn::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    NETMGR_LOG_I("TrafficLimitDialog::OnAbilityConnectDone");
    int32_t curSimId = DelayedSingleton<NetStatsService>::GetInstance()->GetCurActiviteSimId();
    int32_t slotId = Telephony::CoreServiceClient::GetInstance().GetSlotId(curSimId);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    std::string parameters =
        "{\"ability.want.params.uiExtensionType\":\"sysDialog/common\",\"sysDialogZOrder\":1, \"slotId\":";
    std::string paramStr = parameters + std::to_string(slotId) + "}";

    data.WriteInt32(MESSAGE_PARCEL_KEY_SIZE);
    data.WriteString16(u"bundleName");
    data.WriteString16(u"com.xxxxxx.hmos.communicationsetting");
    data.WriteString16(u"abilityName");
    data.WriteString16(u"DisableMobileDataDialogAbility");

    data.WriteString16(u"parameters");
    data.WriteString16(Str8ToStr16(paramStr));

    NETMGR_LOG_I("OnAbilityConnectDone : curSimId = %{public}d", curSimId);
    NETMGR_LOG_I("OnAbilityConnectDone : tmpParameters = %{public}s", paramStr.c_str());

    const uint32_t cmdCode = 1;
    int32_t ret = remoteObject->SendRequest(cmdCode, data, reply, option);
    if (ret != ERR_OK) {
        NETMGR_LOG_I("TrafficLimit Dialog failed: ret=%{public}u", ret);
        return;
    }
    remoteObject_ = remoteObject;
    return;
}

void TrafficLimitDialog::TrafficLimitAbilityConn::OnAbilityDisconnectDone(
    const AppExecFwk::ElementName& element, int resultCode)
{
    NETMGR_LOG_E("TrafficLimitAbilityConn::OnAbilityDisconnectDone");
    remoteObject_ = nullptr;
    return;
}

void TrafficLimitDialog::TrafficLimitAbilityConn::CloseDialog()
{
    if (remoteObject_ == nullptr) {
        NETMGR_LOG_I("CloseDialog: disconnected");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    const uint32_t cmdCode = 3;  // 3：表示关闭弹窗
    int32_t ret = remoteObject_->SendRequest(cmdCode, data, reply, option);
    int32_t replyCode = -1;
    bool success = false;
    if (ret == ERR_OK) {
        success = reply.ReadInt32(replyCode);
    }
    NETMGR_LOG_I("CloseDialog: ret=%{public}d, %{public}d, %{public}d", ret, success, replyCode);
}

bool TrafficLimitDialog::ShowTrafficLimitDialog()
{
    NETMGR_LOG_I("Show TrafficLimit Dialog");
    std::lock_guard<std::mutex> guard(opMutex_);
    if (trafficlimitAbilityConn_ == nullptr) {
        trafficlimitAbilityConn_ = new (std::nothrow) TrafficLimitAbilityConn();
    }
    if (trafficlimitAbilityConn_ == nullptr) {
        NETMGR_LOG_I("TrafficLimitAbilityConn create failed");
        return false;
    }
    auto abilityManager = OHOS::AAFwk::AbilityManagerClient::GetInstance();
    if (abilityManager == nullptr) {
        NETMGR_LOG_I("AbilityManagerClient is nullptr");
        return false;
    }
  
    int32_t curSimId = DelayedSingleton<NetStatsService>::GetInstance()->GetCurActiviteSimId();
    auto settingsObserverMap_ = DelayedSingleton<NetStatsService>::GetInstance()->GetSettingsObserverMap();
    if (settingsObserverMap_.find(curSimId) == settingsObserverMap_.end()) {
        return false;
    }

    NETMGR_LOG_I("Set CellularData false");
    DelayedRefSingleton<Telephony::CellularDataClient>::GetInstance().EnableCellularData(false);

    Want want;
    NETMGR_LOG_I("SetElementName");
    want.SetElementName("com.ohos.sceneboard", "com.ohos.sceneboard.systemdialog");
    NETMGR_LOG_I("ConnectAbility start");
    auto ret = abilityManager->ConnectAbility(want, trafficlimitAbilityConn_, INVALID_USERID);
    if (ret != ERR_OK) {
        NETMGR_LOG_I("ConnectServiceExtensionAbility systemui failed");
        trafficlimitAbilityConn_ = nullptr;
        return false;
    }

    return true;
}

bool TrafficLimitDialog::UnShowTrafficLimitDialog()
{
    std::lock_guard<std::mutex> guard(opMutex_);
    if (trafficlimitAbilityConn_ == nullptr) {
        return true;
    }

    auto abmc = OHOS::AAFwk::AbilityManagerClient::GetInstance();
    if (abmc == nullptr) {
        NETMGR_LOG_I("GetInstance failed");
        return false;
    }
    NETMGR_LOG_I("Unshow TrafficLimit Dialog");
    trafficlimitAbilityConn_->CloseDialog();

    auto ret = abmc->DisconnectAbility(trafficlimitAbilityConn_);
    if (ret != 0) {
        NETMGR_LOG_I("DisconnectAbility failed %{public}d", ret);
        return false;
    }
    NETMGR_LOG_I("Unshow TrafficLimit Dialog success");
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
