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
#include "net_policy_callback_proxy.h"

#include "net_policy_cellular_policy.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyCallbackProxy::NetPolicyCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetPolicyCallback>(impl)
{}

NetPolicyCallbackProxy::~NetPolicyCallbackProxy() {}

int32_t NetPolicyCallbackProxy::NetUidPolicyChanged(uint32_t uid, NetUidPolicy policy)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteUint32(uid)) {
        return ERR_NULL_OBJECT;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(policy))) {
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(NET_POLICY_UIDPOLICY_CHANGED, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t NetPolicyCallbackProxy::NetBackgroundPolicyChanged(bool isBackgroundPolicyAllow)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteBool(isBackgroundPolicyAllow)) {
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(NET_POLICY_BACKGROUNDPOLICY_CHANGED, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t NetPolicyCallbackProxy::NetCellularPolicyChanged(const std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    if (cellularPolicies.empty()) {
        NETMGR_LOG_E("NetCellularPolicyChanged proxy cellularPolicies empty");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!NetPolicyCellularPolicy::Marshalling(data, cellularPolicies)) {
        NETMGR_LOG_E("Marshalling failed.");
        return ERR_FLATTEN_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(NET_POLICY_CELLULARPOLICY_CHANGED, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t NetPolicyCallbackProxy::NetStrategySwitch(const std::string &simId, bool enable)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteString(simId)) {
        return ERR_NULL_OBJECT;
    }

    if (!data.WriteBool(enable)) {
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOG_E("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(NET_POLICY_STRATEGYSWITCH_CHANGED, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOG_E("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

bool NetPolicyCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetPolicyCallbackProxy::GetDescriptor())) {
        NETMGR_LOG_E("WriteInterfaceToken failed");
        return false;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
