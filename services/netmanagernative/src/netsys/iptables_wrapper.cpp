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

#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace nmd {
using namespace NetManagerStandard;
namespace {
constexpr int32_t IPTABLES_THREAD_SLEEP_DURATION_MS = 50;
constexpr int32_t IPTABLES_WAIT_FOR_TIME_MS = 1000;
constexpr const char *IPATBLES_CMD_PATH = "/system/bin/iptables";
void ExecuteCommand(const std::string &command)
{
    if (CommonUtils::ForkExec(command) == NETMANAGER_ERROR) {
        NETNATIVE_LOGE("run exec faild, command=%{public}s", command.c_str());
    }
}

std::string ExecuteCommandForRes(const std::string &command)
{
    std::string result;
    if (CommonUtils::ForkExec(command, &result) == NETMANAGER_ERROR) {
        NETNATIVE_LOGE("run exec faild, command=%{public}s", command.c_str());
    }
    return result;
}
} // namespace

IptablesWrapper::IptablesWrapper()
{
    isRunningFlag_ = true;
    if (access(IPATBLES_CMD_PATH, F_OK) == 0) {
        isIptablesSystemAccess_ = true;
    } else {
        isIptablesSystemAccess_ = false;
    }

    iptablesWrapperThread_ = std::thread(IptablesWrapper::ThreadStart, this);
}

IptablesWrapper::~IptablesWrapper()
{
    isRunningFlag_ = false;
    iptablesWrapperThread_.detach();
}

void IptablesWrapper::ThreadStart(IptablesWrapper *wrapper)
{
    wrapper->RunSystemFunc();
}

void IptablesWrapper::RunSystemFunc()
{
    NETNATIVE_LOGI("IptablesWrapper::RunSystemFunc");
    while (isRunningFlag_) {
        if (commandsQueue_.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(IPTABLES_THREAD_SLEEP_DURATION_MS));
        } else {
            std::unique_lock<std::mutex> lock(iptablesMutex_);
            std::string cmd = commandsQueue_.front();
            commandsQueue_.pop();
            lock.unlock();
            if (cmd.empty()) {
                NETNATIVE_LOGE("pop cmd empty");
                continue;
            }

            if (isIptablesSystemAccess_ == false) {
                NETNATIVE_LOG_D("%{public}s", cmd.c_str());
                continue;
            }
            if (forRes_) {
                result_ = ExecuteCommandForRes(cmd);
                forRes_ = false;
                conditionVarLock_.notify_one();
            } else {
                ExecuteCommand(cmd);
            }
        }
    }
}

int32_t IptablesWrapper::RunCommand(const IpType &ipType, const std::string &command)
{
    NETNATIVE_LOG_D("IptablesWrapper::RunCommand, ipType:%{public}d, command:%{public}s", ipType, command.c_str());
    std::string cmd = std::string(IPATBLES_CMD_PATH) + " " + command;
    std::unique_lock<std::mutex> lock(iptablesMutex_);
    commandsQueue_.push(cmd);

    return NetManagerStandard::NETMANAGER_SUCCESS;
}

std::string IptablesWrapper::RunCommandForRes(const IpType &ipType, const std::string &command)
{
    NETNATIVE_LOGI("IptablesWrapper::RunCommandForRes, ipType:%{public}d, command:%{public}s", ipType,
                   command.c_str());
    forRes_ = true;
    std::string cmd = std::string(IPATBLES_CMD_PATH) + " " + command;
    std::unique_lock<std::mutex> lock(iptablesMutex_);
    commandsQueue_.push(cmd);
    conditionVarLock_.wait_for(lock, std::chrono::milliseconds(IPTABLES_WAIT_FOR_TIME_MS));
    if (forRes_) {
        NETNATIVE_LOGE("IptablesWrapper::RunCommandForRes is timeout.");
        return "";
    }
    return result_;
}
} // namespace nmd
} // namespace OHOS
