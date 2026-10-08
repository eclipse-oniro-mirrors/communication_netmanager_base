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

#include "net_access_policy_dialog_impl.h"

#include <fstream>
#include <memory>
#include <mutex>
#include <set>
#include <string_ex.h>
#include <string>

#include "extension_manager_client.h"
#include <message_parcel.h>
#include "net_mgr_log_wrapper.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace NetManagerStandard {
namespace {
static constexpr int32_t INVALID_USERID = -1;
constexpr int32_t SIGNAL_NUM = 3;
} // namespace

NetAccessPolicyDialogImpl::NetAccessPolicyDialogImpl() : dialogConnectionCallback_(new DialogAbilityConnection()) {}

NetAccessPolicyDialogImpl::~NetAccessPolicyDialogImpl()
{
    dialogConnectionCallback_ = nullptr;
}

bool NetAccessPolicyDialogImpl::ConnectSystemUi(uint32_t uid)
{
    NETMGR_LOG_I("ConnectSystemUi");

    dialogConnectionCallback_->SetUid(uid);
    if (dialogConnectionCallback_->ReshowDialog()) {
        NETMGR_LOG_I("network access policy dialog has been show");
        return true;
    }

    Want want;
    want.SetElementName("com.ohos.sceneboard", "com.ohos.sceneboard.systemdialog");
    ErrCode result = ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(
        want, dialogConnectionCallback_->AsObject(), INVALID_USERID);
    if (result != ERR_OK) {
        NETMGR_LOG_E("ConnectAbility err");
        return false;
    }

    return true;
}

void NetAccessPolicyDialogImpl::DialogAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode)
{
    NETMGR_LOG_I("OnAbilityConnectDone");
    std::lock_guard<std::mutex> lock(mutex_);
    if (remoteObject == nullptr) {
        NETMGR_LOG_E("remote object is nullptr");
        return;
    }

    if (remoteObject_ == nullptr) {
        remoteObject_ = remoteObject;
    }

    if (SendDialogRequest(remoteObject) != ERR_OK) {
        NETMGR_LOG_E("Show power dialog is failed");
        return;
    }
    isDialogShow_ = true;
    NETMGR_LOG_I("Show network access policy dialog is success");
}

void NetAccessPolicyDialogImpl::DialogAbilityConnection::OnAbilityDisconnectDone(
    const AppExecFwk::ElementName& element, int resultCode)
{
    NETMGR_LOG_I("OnAbilityDisconnectDone");
    std::lock_guard<std::mutex> lock(mutex_);
    remoteObject_ = nullptr;
    isDialogShow_ = false;
}

void NetAccessPolicyDialogImpl::DialogAbilityConnection::SetUid(uint32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    uid_ = uid;
}

bool NetAccessPolicyDialogImpl::DialogAbilityConnection::ReshowDialog()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!isDialogShow_ || remoteObject_ == nullptr) {
        return false;
    }
    int32_t ret = SendDialogRequest(remoteObject_);
    if (ret != ERR_OK) {
        NETMGR_LOG_E("Reshow dialog is failed:%{public}d", ret);
        return false;
    }
    return true;
}

int32_t NetAccessPolicyDialogImpl::DialogAbilityConnection::SendDialogRequest(const sptr<IRemoteObject> &remoteObject)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    std::string parameters =
        "{\"ability.want.params.uiExtensionType\":\"sysDialog/common\",\"sysDialogZOrder\":1, \"appUid\":";
    std::string tmpParameters = parameters + std::to_string(uid_) + "}";
    data.WriteInt32(SIGNAL_NUM);
    data.WriteString16(u"bundleName");
    data.WriteString16(u"com.huawei.hmos.communicationsetting");
    data.WriteString16(u"abilityName");
    data.WriteString16(u"UIExtensionProvider");
    data.WriteString16(u"parameters");
    data.WriteString16(Str8ToStr16(tmpParameters));

    return remoteObject->SendRequest(IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
}

INetAccessPolicyDialog *GetNetAccessPolicyDialog()
{
    static NetAccessPolicyDialogImpl impl;
    return &impl;
}
} // namespace NetManagerStandard
} // namespace OHOS
