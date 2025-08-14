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
#include "net_pac_file_url_callback_stub.h"
#include "net_manager_constants.h"
#include "netmanager_base_log.h"

namespace OHOS {
namespace NetManagerStandard {
NetPacFileUrlCallbackStub::NetPacFileUrlCallbackStub() : IRemoteStub<INetPacFileUrlCallback>()
{
    memberFuncMap_[static_cast<uint32_t>(PacFileUrlInterfaceCode::PAC_FILE_URL_CHANGE)] =
        &NetPacFileUrlCallbackStub::PacFileUrlChange;
}

int32_t NetPacFileUrlCallbackStub::PacFileUrlChange(MessageParcel &data, MessageParcel &reply)
{
    if (!data.ContainFileDescriptors()) {
        NETMGR_LOG_D("data error");
    }
    std::string url;
    if (!data.ReadString(url)) {
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    PacFileUrlChange(url);
    return NETMANAGER_SUCCESS;
}

int32_t NetPacFileUrlCallbackStub::PacFileUrlChange(const std::string &pacFileUrl)
{
    return NETMANAGER_SUCCESS;
};

int32_t NetPacFileUrlCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                                   MessageOption &option)
{
    NETMGR_LOG_D("Stub call start, code:[%{public}d]", code);
    std::u16string myDescripter = NetPacFileUrlCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETMGR_LOG_E("Descriptor checked failed");
        return NETMANAGER_ERR_DESCRIPTOR_MISMATCH;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }

    NETMGR_LOG_D("Stub default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
} // namespace NetManagerStandard
} // namespace OHOS