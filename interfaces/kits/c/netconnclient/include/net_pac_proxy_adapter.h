/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef NATIVE_NET_PAC_PROXY_ADAPTER_H
#define NATIVE_NET_PAC_PROXY_ADAPTER_H

#include <map>
#include <mutex>

#include "http_proxy.h"
#include "net_all_capabilities.h"
#include "net_conn_callback_stub.h"
#include "net_connection_type.h"
#include "net_link_info.h"
#include "refbase.h"
#include "net_pac_file_url_callback_stub.h"
namespace OHOS::NetManagerStandard {

class NetPacFileProxyStubAdapter : public NetPacFileUrlCallbackStub {
public:

    NetPacFileProxyStubAdapter(OH_NetConn_PacFileUrlChange *callback);

    ~NetPacFileProxyStubAdapter() = default;

    int32_t PacFileUrlChange(const std::string &pacFileUrl) override;

    OH_NetConn_PacFileUrlChange callback_;
};

class NetPacFilePorxyCallbackManager {
public:
    static NetPacFilePorxyCallbackManager &GetInstance();

    int32_t RegisterPacFileUrlCallback(void *specifier, OH_NetConn_PacFileUrlChange *netConnCallback,
                                     const uint32_t &timeout, uint32_t *callbackId);

    int32_t UnregisterPacFileUrlCallback(uint32_t callbackId);

 private:

    NetPacFilePorxyCallbackManager() = default;

    std::mutex callbackMapMutex_;

    std::map<uint32_t, sptr<INetPacFileUrlCallback>> callbackMap_;

    uint32_t index_{0};
};

} // namespace OHOS::NetManagerStandard
#endif /* NATIVE_NET_PAC_PROXY_ADAPTER_H */