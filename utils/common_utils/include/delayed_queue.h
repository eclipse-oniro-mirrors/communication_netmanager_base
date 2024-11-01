/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef COMMUNICATION_NETMANAGER_BASE_DELAYED_QUEUE_H
#define COMMUNICATION_NETMANAGER_BASE_DELAYED_QUEUE_H

#include <array>
#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <thread>

#ifndef CROSS_PLATFORM
#include "netnative_log_wrapper.h"
#endif

namespace OHOS::NetManagerStandard {
template <typename T, size_t ARRAY_SIZE, size_t DELAYED_COUNT> class DelayedQueue {
public:
    DelayedQueue() : index_(0), needRun_(true)
    {
        pthread_ = std::thread([this]() {
#ifndef CROSS_PLATFORM
            size_t allCounter = 0;
#endif
            while (needRun_) {
                {
                    std::lock_guard<std::mutex> guard(mutex_);
#ifndef CROSS_PLATFORM
                    size_t counter = 0;
                    for (auto &temp : elems_) {
                        counter += temp.size();
                    }
                    if (allCounter != counter) {
                        NETNATIVE_LOGI("dns:%{public}zu", counter);
                        allCounter = counter;
                    }
#endif
		    for (const auto &elem : elems_[index_]) {
                        if (elem) {
                            elem->Execute();
                        }
                        indexMap_.erase(elem);
                    }
                    elems_[index_].clear();
                }
                if (!needRun_) {
                    break;
                }
                std::unique_lock<std::mutex> needRunLock(needRunMutex_);
                needRunCondition_.wait_for(needRunLock, std::chrono::seconds(1), [this] { return !needRun_; });
                std::lock_guard<std::mutex> guard(mutex_);
                index_ = (index_ + 1) % (ARRAY_SIZE + DELAYED_COUNT);
            }
        });
    }

    ~DelayedQueue()
    {
        // set needRun_ = false, and notify the thread to wake
        needRun_ = false;
        needRunCondition_.notify_all();
        if (pthread_.joinable()) {
            pthread_.join();
        }
    }

    void Put(const std::shared_ptr<T> &elem)
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (indexMap_.find(elem) != indexMap_.end()) {
            int oldIndex = indexMap_[elem];
            if (oldIndex >= 0 && oldIndex < static_cast<int>(elems_.size()) &&
                (elems_[oldIndex].find(elem) != elems_[oldIndex].end())) {
                elems_[oldIndex].erase(elem);
            }
        }
        int index = (index_ + DELAYED_COUNT) % (ARRAY_SIZE + DELAYED_COUNT);
        elems_[index].insert(elem);
        indexMap_[elem] = index;
    }

private:
    std::thread pthread_;
    int index_;
    std::mutex mutex_;
    std::atomic_bool needRun_;
    std::condition_variable needRunCondition_;
    std::mutex needRunMutex_;
    std::array<std::set<std::shared_ptr<T>, std::owner_less<std::shared_ptr<T>>>, ARRAY_SIZE + DELAYED_COUNT> elems_;
    std::map<std::shared_ptr<T>, int, std::owner_less<std::shared_ptr<T>>> indexMap_;
};
} // namespace OHOS::NetManagerStandard

#endif // COMMUNICATION_NETMANAGER_BASE_DELAYED_QUEUE_H
