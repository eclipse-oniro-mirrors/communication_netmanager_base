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

namespace OHOS {
namespace NetManagerStandard {
class Scheduler {
public:
    typedef std::function<void(void)> TaskFunction;

    class Task {
    friend class Scheduler;
    public:
        Task(TaskFunction func);
        
        virtual ~Task();
        
        void Wait();
        
        bool WaitFor(uint64_t timeoutMs);
        
        void Cancel();

    private:
        bool Delay(uint64_t delayMs);

        void Process();

    private:
        TaskFunction func_;
        std::mutex mtx_;
        std::condition_variable cond_;
        bool canceled_ {false};
        std::mutex delayMtx_;
        std::condition_variable delayCond_;
    };

    Scheduler();

    virtual ~Scheduler();
    
    void Post(std::shared_ptr<Task> task);

    std::shared_ptr<Task> Post(TaskFunction taskFunc);

    std::shared_ptr<Task> DelayPost(TaskFunction taskFunc, uint64_t delayMs);

    void Run();

    void Stop();

    bool InRunThread() const;
    
private:
    std::list<std::shared_ptr<Task>> tasks_;
    bool running_ {false};
    std::mutex mtx_;
    std::condition_variable_any cond_;
    std::thread::id runThreadId_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_SCHEDULER_H
