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

#include "net_dns_result_callback_stub.h"
#include "netnative_log_wrapper.h"
#include "netsys_ipc_interface_code.h"

namespace OHOS {
namespace NetsysNative {
NetDnsResultCallbackStub::NetDnsResultCallbackStub()
{
    memberFuncMap_[static_cast<uint32_t>(NetDnsResultInterfaceCode::ON_DNS_RESULT_REPORT)] =
        &NetDnsResultCallbackStub::CmdDnsResultReport;
}

int32_t NetDnsResultCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    NETNATIVE_LOG_D("Stub call start, code:[%{public}d]", code);
    std::u16string myDescripter = NetDnsResultCallbackStub::GetDescriptor();
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

int32_t NetDnsResultCallbackStub::CmdDnsResultReport(MessageParcel &data, MessageParcel &reply)
{
    std::list<NetDnsResultReport> dnsResultReport;

    uint32_t size = data.ReadUint32();
    
    for (uint32_t i = 0; i < size; ++i) {
        NetDnsResultReport report;
        if (!NetDnsResultReport::Unmarshalling(data, report)) {
            return -1;
        }
        dnsResultReport.push_back(report);
    }

    int32_t result = OnDnsResultReport(size, dnsResultReport);
    if (!reply.WriteInt32(result)) {
        NETNATIVE_LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t NetDnsResultCallbackStub::OnDnsResultReport(uint32_t size, std::list<NetDnsResultReport> dnsResultReport)
{
    return 0;
}
} // namespace NetsysNative
} // namespace OHOS
