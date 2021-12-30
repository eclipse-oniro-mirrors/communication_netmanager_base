/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef INCLUDE_THREAD_POOL_H__
#define INCLUDE_THREAD_POOL_H__

#include <memory>
#include <vector>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "blocking_queue.h"
#include "job.h"

namespace OHOS {
namespace nmd {
class thread_pool {
public:
    thread_pool(unsigned int threadNums, unsigned int queueSize);

    void execute(nmd::job *job);

    ~thread_pool();

private:
    unsigned int threadNums_;
    unsigned int queueSize_;

    bool running_ = false;

    std::vector<std::thread *> workers_;
    nmd::blocking_queue<nmd::job *> *workQueue_;

    std::mutex mutex_;
    std::condition_variable cond_;

    void threadLoop();

    nmd::job *takeJobFromQueue();
};
} // namespace nmd
} // namespace OHOS
#endif // !INCLUDE_THREAD_POOL_H__