/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef NET_CONN_SCHEDULER_H
#define NET_CONN_SCHEDULER_H

#include <functional>
#include <list>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>

namespace OHOS {
namespace NetManagerStandard {
class Scheduler {
public:
    typedef std::function<void(void)> TaskFunction;

    class Task {
        friend class Scheduler;

    public:
        /**
         * Construct a async task
         *
         * @param func Task process function
         */
        Task(TaskFunction func);

        /**
         * Destroy the task
         *
         */
        virtual ~Task();

        /**
         * Wait for task processed or may be canceled by call Cancel()
         *
         */
        void Wait();

        /**
         * Wait for task processed for timeoutMs milliseconds
         *
         * @param timeoutMs Milliseconds to wait
         * @return Return true if not timeout, otherwise return false
         */
        bool WaitFor(uint64_t timeoutMs);

        /**
         * Cancel Wait() or WaitFor()
         *
         */
        void Cancel();

        /**
         * Ignore this task, task will not be processed, and deley will be canceled
         *
         */
        void Ignore();

    private:
        bool Delay(uint64_t delayMs);

        void Process();

    private:
        TaskFunction func_;
        std::mutex mtx_;
        std::condition_variable cond_;
        bool canceled_ {false};
        bool processed_ {false};
        bool ignored_ {false};
        std::mutex delayMtx_;
        std::condition_variable delayCond_;
        std::future<void> delayFuture_;
    };

    /**
     * Construct a new Scheduler
     *
     */
    Scheduler();

    /**
     * Destroy the Scheduler
     *
     */
    virtual ~Scheduler();

    /**
     * Insert task to scheduler task list, task will be processed async
     *
     * @param task task to async processed
     */
    void Post(std::shared_ptr<Task> task);

    /**
     * Create a task with taskFunc and post it
     *
     * @param taskFunc task function
     * @return std::shared_ptr<Task> Created task
     */
    std::shared_ptr<Task> Post(TaskFunction taskFunc);

    /**
     * Create a task with taskFunc and delay post it after delayMs milliseconds
     *
     * @param taskFunc task function
     * @return std::shared_ptr<Task> Created task
     * @return std::shared_ptr<Task>
     */
    std::shared_ptr<Task> DelayPost(TaskFunction taskFunc, uint64_t delayMs);

    /**
     * Start async process loop, block call
     *
     */
    void Run();

    /**
     * Stop async process loop
     *
     */
    void Stop();

    /**
     * Determine if current thread is same with the Scheduler's run thread
     *
     * @return bool Current thread is same with the Scheduler's run thread
     */
    bool InRunThread() const;

private:
    std::list<std::shared_ptr<Task>> tasks_;
    bool running_{false};
    std::mutex mtx_;
    std::condition_variable_any cond_;
    std::thread::id runThreadId_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_SCHEDULER_H
