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

#ifndef NET_ACCESS_POLICY_DIALOG_IMPL_H
#define NET_ACCESS_POLICY_DIALOG_IMPL_H

#include "net_access_policy_dialog.h"

#include <iostream>
#include <mutex>


#include "ability_connect_callback_interface.h"
#include "ability_connect_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
class NetAccessPolicyDialogImpl : public INetAccessPolicyDialog {
public:
    NetAccessPolicyDialogImpl();
    ~NetAccessPolicyDialogImpl();
    bool ConnectSystemUi(uint32_t uid) override;
    static std::string GetBundleName()
    {
        return bundleName_;
    }

    static std::string GetAbilityName()
    {
        return abilityName_;
    }

    static std::string GetUiExtensionType()
    {
        return uiExtensionType_;
    }
private:
    class DialogAbilityConnection : public OHOS::AAFwk::AbilityConnectionStub {
    public:
        void OnAbilityConnectDone(const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject,
                                  int resultCode) override;
        void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
        void SetUid(uint32_t uid);
        bool ReshowDialog();

    private:
        int32_t SendDialogRequest(const sptr<IRemoteObject> &remoteObject);

        std::mutex mutex_;
        sptr<IRemoteObject> remoteObject_ = nullptr;
        bool isDialogShow_ = false;
        uint32_t uid_ = 0;
    };

    sptr<DialogAbilityConnection> dialogConnectionCallback_{nullptr};
    static std::string bundleName_;
    static std::string abilityName_;
    static std::string uiExtensionType_;
};

extern "C" INetAccessPolicyDialog *GetNetAccessPolicyDialog();
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NET_ACCESS_POLICY_DIALOG_IMPL_H
