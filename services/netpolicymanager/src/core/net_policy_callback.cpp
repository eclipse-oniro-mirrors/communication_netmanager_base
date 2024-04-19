/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "net_policy_callback.h"

#include "net_mgr_log_wrapper.h"
#include "net_policy_constants.h"
#include "net_policy_inner_define.h"

namespace OHOS {
namespace NetManagerStandard {

NetPolicyCallback::NetPolicyCallback()
{
    netPolicyCallbackFfrtQueue_ = std::make_shared<ffrt::queue>("NetPolicyCallback");
}

NetPolicyCallback::~NetPolicyCallback() {}

int32_t NetPolicyCallback::RegisterNetPolicyCallbackAsync(const sptr<INetPolicyCallback> &callback)
{
    if (callback == nullptr || callback->AsObject() == nullptr || callback->AsObject().GetRefPtr() == nullptr) {
        NETMGR_LOG_E("The parameter callback is null");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    if (!netPolicyCallbackFfrtQueue_) {
        NETMGR_LOG_E("FFRT Init Fail");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    int32_t ret = NETMANAGER_SUCCESS;
    ffrt::task_handle RegisterNetPolicyCallbackAsyncTask =
        netPolicyCallbackFfrtQueue_->submit_h([this, callback, &ret]() {
            ret = this->RegisterNetPolicyCallback(callback);
        });
    netPolicyCallbackFfrtQueue_->wait(RegisterNetPolicyCallbackAsyncTask);

    return ret;
}

int32_t NetPolicyCallback::RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    uint32_t callbackCounts = callbacks_.size();
    NETMGR_LOG_I("callback counts [%{public}u]", callbackCounts);
    if (callbackCounts >= LIMIT_CALLBACK_NUM) {
        NETMGR_LOG_E("callback counts cannot more than [%{public}u]", LIMIT_CALLBACK_NUM);
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }

    for (uint32_t i = 0; i < callbackCounts; i++) {
        if (callback->AsObject().GetRefPtr() == callbacks_[i]->AsObject().GetRefPtr()) {
            NETMGR_LOG_W("netPolicyCallback_ had this callback");
            return NETMANAGER_ERR_PARAMETER_ERROR;
        }
    }

    callbacks_.emplace_back(callback);
    NETMGR_LOG_I("End RegisterNetPolicyCallback");
    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyCallback::UnregisterNetPolicyCallbackAsync(const sptr<INetPolicyCallback> &callback)
{
    if (callback == nullptr || callback->AsObject() == nullptr || callback->AsObject().GetRefPtr() == nullptr) {
        NETMGR_LOG_E("The parameter of callback is null");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    if (!netPolicyCallbackFfrtQueue_) {
        NETMGR_LOG_E("FFRT Init Fail");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    int32_t ret = NETMANAGER_SUCCESS;
    ffrt::task_handle UnregisterNetPolicyCallbackAsyncTask =
        netPolicyCallbackFfrtQueue_->submit_h([this, callback, &ret]() {
            ret = this->UnregisterNetPolicyCallback(callback);
        });
    netPolicyCallbackFfrtQueue_->wait(UnregisterNetPolicyCallbackAsyncTask);

    return ret;
}

int32_t NetPolicyCallback::UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    NETMGR_LOG_I("Enter UnregisterNetPolicyCallback");
    auto it = std::remove_if(
        callbacks_.begin(), callbacks_.end(), [callback](const sptr<INetPolicyCallback> &tempCallback) -> bool {
            if (tempCallback == nullptr || tempCallback->AsObject() == nullptr ||
                tempCallback->AsObject().GetRefPtr() == nullptr) {
                return true;
            }
            return callback->AsObject().GetRefPtr() == tempCallback->AsObject().GetRefPtr();
        });
    callbacks_.erase(it, callbacks_.end());
    NETMGR_LOG_I("End UnregisterNetPolicyCallback");
    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyCallback::NotifyNetUidPolicyChangeAsync(uint32_t uid, uint32_t policy)
{
    NETMGR_LOG_D("NotifyNetUidPolicyChange uid[%{public}u] policy[%{public}u]", uid, policy);
    if (!netPolicyCallbackFfrtQueue_) {
        NETMGR_LOG_E("FFRT Init Fail");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    int32_t ret = NETMANAGER_SUCCESS;
    ffrt::task_handle NotifyNetUidPolicyChangeAsyncTask =
        netPolicyCallbackFfrtQueue_->submit_h([this, uid, policy, &ret]() {
            ret = this->NotifyNetUidPolicyChange(uid, policy);
        });
    netPolicyCallbackFfrtQueue_->wait(NotifyNetUidPolicyChangeAsyncTask);

    return ret;
}

int32_t NetPolicyCallback::NotifyNetUidPolicyChange(uint32_t uid, uint32_t policy)
{
    NETMGR_LOG_I("NotifyNetUidPolicyChange uid= %{public}d policy= %{public}d", uid, policy);
    for (const auto &callback : callbacks_) {
        if (callback != nullptr && callback->AsObject() != nullptr && callback->AsObject().GetRefPtr() != nullptr) {
            callback->NetUidPolicyChange(uid, policy);
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyCallback::NotifyNetUidRuleChangeAsync(uint32_t uid, uint32_t rule)
{
    NETMGR_LOG_D("NotifyNetUidRuleChange uid[%{public}u] rule[%{public}u]", uid, rule);
    if (!netPolicyCallbackFfrtQueue_) {
        NETMGR_LOG_E("FFRT Init Fail");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    int32_t ret = NETMANAGER_SUCCESS;
    ffrt::task_handle NotifyNetUidRuleChangeAsyncTask =
        netPolicyCallbackFfrtQueue_->submit_h([this, uid, rule, &ret]() {
            ret = this->NotifyNetUidRuleChange(uid, rule);
        });
    netPolicyCallbackFfrtQueue_->wait(NotifyNetUidRuleChangeAsyncTask);
    return ret;
}

int32_t NetPolicyCallback::NotifyNetUidRuleChange(uint32_t uid, uint32_t rule)
{
    NETMGR_LOG_I("NotifyNetUidRuleChange uid= %{public}d policy= %{public}d", uid, rule);
    for (const auto &callback : callbacks_) {
        if (callback != nullptr && callback->AsObject() != nullptr && callback->AsObject().GetRefPtr() != nullptr) {
            callback->NetUidRuleChange(uid, rule);
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyCallback::NotifyNetBackgroundPolicyChangeAsync(bool isAllowed)
{
    NETMGR_LOG_D("NotifyNetBackgroundPolicyChange  isAllowed[%{public}d]", isAllowed);
    if (!netPolicyCallbackFfrtQueue_) {
        NETMGR_LOG_E("FFRT Init Fail");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    int32_t ret = NETMANAGER_SUCCESS;
    ffrt::task_handle NotifyNetBackgroundPolicyChangeAsyncTask =
        netPolicyCallbackFfrtQueue_->submit_h([this, isAllowed, &ret]() {
            ret = this->NotifyNetBackgroundPolicyChange(isAllowed);
        });
    netPolicyCallbackFfrtQueue_->wait(NotifyNetBackgroundPolicyChangeAsyncTask);
    return ret;
}

int32_t NetPolicyCallback::NotifyNetBackgroundPolicyChange(bool isAllowed)
{
    for (const auto &callback : callbacks_) {
        if (callback != nullptr && callback->AsObject() != nullptr && callback->AsObject().GetRefPtr() != nullptr) {
            callback->NetBackgroundPolicyChange(isAllowed);
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyCallback::NotifyNetQuotaPolicyChangeAsync(const std::vector<NetQuotaPolicy> &quotaPolicies)
{
    if (quotaPolicies.empty()) {
        NETMGR_LOG_E("NotifyNetQuotaPolicyChange quotaPolicies empty");
        return POLICY_ERR_QUOTA_POLICY_NOT_EXIST;
    }
    NETMGR_LOG_D("NotifyNetQuotaPolicyChange quotaPolicies.size[%{public}zu]", quotaPolicies.size());
    if (!netPolicyCallbackFfrtQueue_) {
        NETMGR_LOG_E("FFRT Init Fail");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    int32_t ret = NETMANAGER_SUCCESS;
    ffrt::task_handle NotifyNetQuotaPolicyChangeTask =
        netPolicyCallbackFfrtQueue_->submit_h([this, &quotaPolicies, &ret]() {
            ret = this->NotifyNetQuotaPolicyChange(quotaPolicies);
        });
    netPolicyCallbackFfrtQueue_->wait(NotifyNetQuotaPolicyChangeTask);
    return ret;
}

int32_t NetPolicyCallback::NotifyNetQuotaPolicyChange(const std::vector<NetQuotaPolicy> &quotaPolicies)
{
    for (const auto &callback : callbacks_) {
        if (callback != nullptr && callback->AsObject() != nullptr && callback->AsObject().GetRefPtr() != nullptr) {
            callback->NetQuotaPolicyChange(quotaPolicies);
        }
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetPolicyCallback::NotifyNetMeteredIfacesChangeAsync(std::vector<std::string> &ifaces)
{
    NETMGR_LOG_D("NotifyNetMeteredIfacesChange iface size[%{public}zu]", ifaces.size());
    if (!netPolicyCallbackFfrtQueue_) {
        NETMGR_LOG_E("FFRT Init Fail");
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    int32_t ret = NETMANAGER_SUCCESS;
    ffrt::task_handle NotifyNetMeteredIfacesChangeTask =
        netPolicyCallbackFfrtQueue_->submit_h([this, &ifaces, &ret]() {
            ret = this->NotifyNetMeteredIfacesChange(ifaces);
        });
    netPolicyCallbackFfrtQueue_->wait(NotifyNetMeteredIfacesChangeTask);
    return ret;
}

int32_t NetPolicyCallback::NotifyNetMeteredIfacesChange(std::vector<std::string> &ifaces)
{
    NETMGR_LOG_D("NotifyNetMeteredIfacesChange iface size[%{public}zu]", ifaces.size());
    for (const auto &callback : callbacks_) {
        if (callback != nullptr && callback->AsObject() != nullptr && callback->AsObject().GetRefPtr() != nullptr) {
            callback->NetMeteredIfacesChange(ifaces);
        }
    }

    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
