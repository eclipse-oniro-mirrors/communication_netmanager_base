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
#include "net_pac_proxy_adapter.h"
#include "net_conn_client.h"
#include "netmanager_base_log.h"

namespace OHOS::NetManagerStandard {

NetPacFileProxyStubAdapter::NetPacFileProxyStubAdapter(OH_NetConn_PacFileUrlChange *callback)
    : NetPacFileUrlCallbackStub()
{
    this->callback_.onNetPacFileUrlChange = callback->onNetPacFileUrlChange;
}

int32_t NetPacFileProxyStubAdapter::PacFileUrlChange(const std::string &pacFileUrl)
{
    if (this->callback_.onNetPacFileUrlChange) {
        this->callback_.onNetPacFileUrlChange(pacFileUrl.c_str());
    }
    return 0;
}

NetPacFilePorxyCallbackManager &NetPacFilePorxyCallbackManager::GetInstance()
{
    static NetPacFilePorxyCallbackManager instance;
    return instance;
}

int32_t NetPacFilePorxyCallbackManager::RegisterPacFileUrlCallback(void *specifier,
                                                                   OH_NetConn_PacFileUrlChange *netConnCallback,
                                                                   const uint32_t &timeout, uint32_t *callbackId)
{
    sptr<NetPacFileProxyStubAdapter> callback = sptr<NetPacFileProxyStubAdapter>::MakeSptr(netConnCallback);
    int32_t ret = NetConnClient::GetInstance().RegisterPacFileProxyCallback(callback);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("RegisterNetConnCallback failed");
        return ret;
    }
    std::lock_guard<std::mutex> lock(this->callbackMapMutex_);
    *callbackId = this->index_++;
    this->callbackMap_[*callbackId] = callback;
    return NETMANAGER_SUCCESS;
}

int32_t NetPacFilePorxyCallbackManager::UnregisterPacFileUrlCallback(uint32_t callbackId)
{
    std::lock_guard<std::mutex> lock(this->callbackMapMutex_);
    auto it = this->callbackMap_.find(callbackId);
    if (it != this->callbackMap_.end()) {
        int32_t ret = NetConnClient::GetInstance().UnregisterPacFileProxyCallback(it->second);
        this->callbackMap_.erase(it);
        return ret;
    } else {
        return NET_CONN_ERR_CALLBACK_NOT_FOUND;
    }
}
} // namespace OHOS::NetManagerStandard
