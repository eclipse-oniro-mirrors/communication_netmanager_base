/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "iptables_wrapper.h"

#include <unistd.h>

#include "datetime_ex.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace nmd {
using namespace NetManagerStandard;
namespace {
constexpr const char *IPATBLES_CMD_PATH = "/system/bin/iptables";
constexpr const char *IP6TABLES_CMD_PATH = "/system/bin/ip6tables";
} // namespace

IptablesWrapper::IptablesWrapper()
{
    isRunningFlag_ = true;
    isIptablesSystemAccess_ = access(IPATBLES_CMD_PATH, F_OK) == 0;
    isIp6tablesSystemAccess_ = access(IP6TABLES_CMD_PATH, F_OK) == 0;

    handlerRunner_ = AppExecFwk::EventRunner::Create("IptablesWrapper");
    if (handlerRunner_ == nullptr) {
        return;
    }
    handler_ = std::make_shared<AppExecFwk::EventHandler>(handlerRunner_);
}

IptablesWrapper::~IptablesWrapper()
{
    isRunningFlag_ = false;
    if (handlerRunner_) {
        handlerRunner_->Stop();
        handlerRunner_.reset();
    }
    if (handler_) {
        handler_.reset();
        handler_ = nullptr;
    }
}

void IptablesWrapper::ExecuteCommand(const std::string &command)
{
    if (CommonUtils::ForkExec(command) == NETMANAGER_ERROR) {
        NETNATIVE_LOGE("run exec faild, command=%{public}s", command.c_str());
    }
}

void IptablesWrapper::ExecuteCommandForRes(const std::string &command)
{
    if (CommonUtils::ForkExec(command, &result_) == NETMANAGER_ERROR) {
        NETNATIVE_LOGE("run exec faild, command=%{public}s", command.c_str());
    }
}

int32_t IptablesWrapper::RunCommand(const IpType &ipType, const std::string &command)
{
    NETNATIVE_LOG_D("IptablesWrapper::RunCommand, ipType:%{public}d, command:%{public}s", ipType, command.c_str());
    if (handler_ == nullptr) {
        NETNATIVE_LOGE("RunCommand failed! handler is nullptr");
        return NETMANAGER_ERROR;
    }

    if (isIptablesSystemAccess_ && (ipType == IPTYPE_IPV4 || ipType == IPTYPE_IPV4V6)) {
        std::string cmd = std::string(IPATBLES_CMD_PATH) + " " + command;
        std::function<void()> executeCommand = std::bind(&IptablesWrapper::ExecuteCommand, shared_from_this(), cmd);
        handler_->PostTask(executeCommand);
    }

    if (isIp6tablesSystemAccess_ && (ipType == IPTYPE_IPV6 || ipType == IPTYPE_IPV4V6)) {
        std::string cmd = std::string(IP6TABLES_CMD_PATH) + " " + command;
        std::function<void()> executeCommand = std::bind(&IptablesWrapper::ExecuteCommand, shared_from_this(), cmd);
        handler_->PostTask(executeCommand);
    }

    return NetManagerStandard::NETMANAGER_SUCCESS;
}

std::string IptablesWrapper::RunCommandForRes(const IpType &ipType, const std::string &command)
{
    NETNATIVE_LOG_D("IptablesWrapper::RunCommandForRes, ipType:%{public}d, command:%{public}s", ipType,
                    command.c_str());
    if (handler_ == nullptr) {
        NETNATIVE_LOGE("RunCommandForRes failed! handler is nullptr");
        return result_;
    }

    if (ipType == IPTYPE_IPV4 || ipType == IPTYPE_IPV4V6) {
        std::string cmd = std::string(IPATBLES_CMD_PATH) + " " + command;
        std::function<void()> executeCommandForRes =
            std::bind(&IptablesWrapper::ExecuteCommandForRes, shared_from_this(), cmd);

        int64_t start = GetTickCount();
        handler_->PostSyncTask(executeCommandForRes);
        NETNATIVE_LOGI("PostSyncTask cost:%{public}lld ms", static_cast<long long>(GetTickCount() - start));
    }

    if (ipType == IPTYPE_IPV6 || ipType == IPTYPE_IPV4V6) {
        std::string cmd = std::string(IP6TABLES_CMD_PATH) + " " + command;
        std::function<void()> executeCommandForRes =
            std::bind(&IptablesWrapper::ExecuteCommandForRes, shared_from_this(), cmd);

        int64_t start = GetTickCount();
        handler_->PostSyncTask(executeCommandForRes);
        NETNATIVE_LOGI("PostSyncTask cost:%{public}lld ms", static_cast<long long>(GetTickCount() - start));
    }

    return result_;
}
} // namespace nmd
} // namespace OHOS
