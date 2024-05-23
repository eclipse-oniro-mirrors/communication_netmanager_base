/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "net_access_policy_dialog.h"

#include <atomic>
#include <fstream>
#include <memory>
#include <set>
#include <string_ex.h>
#include <string>

#include <ability_manager_client.h>
#include <message_parcel.h>
#include "netnative_log_wrapper.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace NetManagerStandard {
namespace {
static constexpr int32_t INVALID_USERID = -1;
constexpr int32_t SIGNAL_NUM = 3;
sptr<IRemoteObject> g_remoteObject = nullptr;
uint32_t g_uid = 0;
} // namespace

NetAccessPolicyDialog::NetAccessPolicyDialog() : dialogConnectionCallback_(new DialogAbilityConnection()) {}

NetAccessPolicyDialog::~NetAccessPolicyDialog()
{
    dialogConnectionCallback_ = nullptr;
}

bool NetAccessPolicyDialog::ConnectSystemUi(uint32_t uid)
{
    NETNATIVE_LOGI("OnAbilityConnectDone");
    auto abilityManager = AbilityManagerClient::GetInstance();
    if (abilityManager == nullptr) {
        NETNATIVE_LOGE("Get abilityManager err");
        return false;
    }

    Want want;
    want.SetElementName("com.ohos.sceneboard", "com.ohos.sceneboard.systemdialog");
    ErrCode result = abilityManager->ConnectAbility(want, dialogConnectionCallback_, INVALID_USERID);
    if (result != ERR_OK) {
        NETNATIVE_LOGE("ConnectAbility err");
        return false;
    }

    g_uid = uid;
    return true;
}

void NetAccessPolicyDialog::DialogAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode)
{
    NETNATIVE_LOGI("OnAbilityConnectDone");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    std::string parameters =
        "{\"ability.want.parabilityManager.uiExtensionType\":\"sysDialog/common\",\"sysDialogZOrder\":2, \"appUid\":";
    std::string tmpParameters = parameters + std::to_string(g_uid) + "}";
    NETNATIVE_LOGI("OnAbilityConnectDone: uid: %{public}d", g_uid);
    data.WriteInt32(SIGNAL_NUM);
    data.WriteString16(u"bundleName");
    data.WriteString16(u"com.example.myapplication");
    data.WriteString16(u"abilityName");
    data.WriteString16(u"UIExtensionProvider");

    data.WriteString16(u"parameters");
    data.WriteString16(Str8ToStr16(tmpParameters));
 
    remoteObject->SendRequest(IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
}

void NetAccessPolicyDialog::DialogAbilityConnection::OnAbilityDisconnectDone(
    const AppExecFwk::ElementName& element, int resultCode)
{
    NETNATIVE_LOGI("OnAbilityDisconnectDone");
    std::lock_guard lock(mutex_);
    g_remoteObject = nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS
