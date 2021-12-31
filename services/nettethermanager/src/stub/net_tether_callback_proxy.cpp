/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "net_tether_callback_proxy.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetTetherCallbackProxy::NetTetherCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetTetherCallback>(impl) {}

NetTetherCallbackProxy::~NetTetherCallbackProxy() {}

int32_t NetTetherCallbackProxy::TetherSuccess(int32_t tetherType, const std::string &ifName)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(tetherType)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString(ifName)) {
        return ERR_FLATTEN_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(NET_TETHER_SUCCESS, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t NetTetherCallbackProxy::TetherFailed(int32_t tetherType, const std::string &ifName, int32_t failCode)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(tetherType)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString(ifName)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(failCode)) {
        return ERR_FLATTEN_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(NET_TETHER_FAILED, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

bool NetTetherCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetTetherCallbackProxy::GetDescriptor())) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return false;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
