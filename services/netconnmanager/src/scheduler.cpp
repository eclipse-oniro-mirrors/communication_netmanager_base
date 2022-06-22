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

#include "scheduler.h"
#include <future>
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
static constexpr int32_t LOOP_INTERVAL_MS = 100;

Scheduler::Task::Task(TaskFunction func) : func_(func) {}

Scheduler::Task::~Task() {}

void Scheduler::Task::Process()
{
    std::unique_lock<std::mutex> locker(mtx_);
    if (func_) {
        func_();
    }
    cond_.notify_all();
}

void Scheduler::Task::Wait()
{
    std::unique_lock<std::mutex> locker(mtx_);
    if (func_) {
        cond_.wait(locker);
    }
}

bool Scheduler::Task::WaitFor(uint64_t timeoutMs)
{
    std::unique_lock<std::mutex> locker(mtx_);
    if (func_) {
        return std::cv_status::timeout != cond_.wait_for(locker, std::chrono::milliseconds(timeoutMs));
    }
    return false;
}

void Scheduler::Task::Cancel()
{
    std::unique_lock<std::mutex> locker(delayMtx_);
    if (!canceled_) {
        canceled_ = true;
        delayCond_.notify_one();
    }
}

bool Scheduler::Task::Delay(uint64_t delayMs)
{
    std::unique_lock<std::mutex> locker(delayMtx_);
    if (!canceled_) {
        std::cv_status s = delayCond_.wait_for(locker, std::chrono::milliseconds(delayMs));
        if (s == std::cv_status::timeout) {
            return true;
        }
    }
    return false;
}

Scheduler::Scheduler() {}

Scheduler::~Scheduler()
{
    Stop();
}

void Scheduler::Post(std::shared_ptr<Task> task)
{
    std::unique_lock<std::mutex> locker(mtx_);
    tasks_.push_back(task);
    cond_.notify_one();
}

std::shared_ptr<Scheduler::Task> Scheduler::Post(TaskFunction taskFunc)
{
    auto task = std::make_shared<Task>(taskFunc);
    Post(task);
    return task;
}

std::shared_ptr<Scheduler::Task> Scheduler::DelayPost(TaskFunction taskFunc, uint64_t delayMs)
{
    auto task = std::make_shared<Task>(taskFunc);
    std::async(std::launch::async, [&](std::shared_ptr<Task> task, uint64_t delayMs) {
        if (task->Delay(delayMs)) {
            Post(task);
        }
    }, task, delayMs);

    return task;
}

void Scheduler::Run()
{
    runThreadId_ = std::this_thread::get_id();
    running_ = true;
    while (running_) {
        std::unique_lock<std::mutex> locker(mtx_);
        if (tasks_.empty()) {
            cond_.wait_for(locker, std::chrono::milliseconds(LOOP_INTERVAL_MS)); // to avoid stuck here,
        } else {
            auto task = tasks_.front();
            tasks_.pop_front();
            locker.unlock();
            task->Process();
        }
    }
}

void Scheduler::Stop()
{
    std::unique_lock<std::mutex> locker(mtx_);
    tasks_.clear();
    running_ = false;
    cond_.notify_one();
}

bool Scheduler::InRunThread() const
{
    return runThreadId_ == std::this_thread::get_id();
}
} // namespace NetManagerStandard
} // namespace OHOS