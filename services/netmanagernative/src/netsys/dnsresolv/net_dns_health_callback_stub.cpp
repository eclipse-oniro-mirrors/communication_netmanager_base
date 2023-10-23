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
#include "net_dns_health_callback_stub.h"
#include "netnative_log_wrapper.h"
#include "netsys_ipc_interface_code.h"
#include "net_conn_constants.h"

namespace OHOS {
namespace NetsysNative {
NetDnsHealthCallbackStub::NetDnsHealthCallbackStub()
{
    memberFuncMap_[static_cast<uint32_t>(NetDnsHealthInterfaceCode::ON_DNS_HEALTH_REPORT)] =
        &NetDnsHealthCallbackStub::CmdDnsHealthReport;
}

int32_t NetDnsHealthCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    NETNATIVE_LOGI("Stub call start, code:[%{public}d]", code);
    std::u16string myDescripter = NetDnsHealthCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETNATIVE_LOGE("Descriptor checked failed");
        return ERR_FLATTEN_OBJECT;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }

    NETNATIVE_LOGI("Stub default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
int32_t NetDnsHealthCallbackStub::CmdDnsHealthReport(MessageParcel &data, MessageParcel &reply)
{
    NetDnsHealthReport dnsHealthReport;
    if (!NetDnsHealthReport::Unmarshalling(data, dnsHealthReport)) {
        return NetManagerStandard::NETMANAGER_ERR_READ_DATA_FAIL;
    }

    int32_t result = OnDnsHealthReport(dnsHealthReport);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NetDnsHealthCallbackStub::OnDnsHealthReport(const NetDnsHealthReport &dnsHealthReport)
{
    return 0;
}
} // namespace NetsysNative
} // namespace OHOS
