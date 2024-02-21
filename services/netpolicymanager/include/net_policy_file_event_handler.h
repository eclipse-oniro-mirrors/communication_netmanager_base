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

#ifndef NET_POLICY_FILE_EVENT_HANDLER_H
#define NET_POLICY_FILE_EVENT_HANDLER_H

#include <iostream>
#include <dirent.h>

#include "event_handler.h"
#include "event_runner.h"
#include "singleton.h"

struct PolicyFileEvent {
    std::string json;
};

namespace OHOS {
namespace NetManagerStandard {
class NetPolicyFileEventHandler : public AppExecFwk::EventHandler {
public:
    static constexpr uint32_t MSG_POLICY_FILE_WRITE = 2;
    static constexpr uint32_t MSG_POLICY_FILE_DELETE = 3;
    static constexpr uint32_t MSG_POLICY_FILE_COMMIT = 4;

    explicit NetPolicyFileEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
    virtual ~NetPolicyFileEventHandler() = default;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    void SendWriteEvent(AppExecFwk::InnerEvent::Pointer &event);

private:
    bool Write();
    bool DeleteBak();

    std::atomic<int64_t> timeStamp_ = 0;
    std::atomic<bool> commitWait_ = false;
    std::string fileContent_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_EVENT_HANDLER_H