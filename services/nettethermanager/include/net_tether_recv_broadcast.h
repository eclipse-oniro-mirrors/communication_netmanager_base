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

#ifndef NET_TETHER_RECV_BROADCAST_H
#define NET_TETHER_RECV_BROADCAST_H

#include <functional>
#include "common_event_manager.h"
#include "common_event.h"

namespace OHOS {
namespace NetManagerStandard {
class NetTetherRecvBroadcast {
public:
    class NetTetherEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
    public:
        explicit NetTetherEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscribeInfo);
        virtual ~NetTetherEventSubscriber();
        virtual void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
    };
    friend class NetTetherEventSubscriber;
    static NetTetherRecvBroadcast *GetInstance();
    static void ReleaseInstance();
    ~NetTetherRecvBroadcast();

    bool AddApStateChangeSubscribe(std::function<void(int32_t)> cb);
    void RemoveApStateChangeSubscribe();
    bool AddUsbStateChangeSubscribe(std::function<void(bool)> cb);
    void RemoveUsbStateChangeSubscribe();

private:
    NetTetherRecvBroadcast();
    bool SubscribeBroadcastEvent(const std::string &event);
    bool UnsubscribeServiceEvent(const std::string &event);
    std::function<void(int32_t)> GetApEventCb();
    std::function<void(bool)> GetUsbEventCb();

private:
    std::function<void(int32_t)> apStateChangeCb_;
    std::function<void(bool)> usbStateChangeCb_;
    std::map<std::string, std::shared_ptr<NetTetherEventSubscriber>> mapEventSubscriber_;
    static NetTetherRecvBroadcast *instance_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_TETHER_RECV_BROADCAST_H