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

#include "net_tether_recv_broadcast.h"
#include "net_mgr_log_wrapper.h"
#include "net_tether_define.h"

using namespace OHOS::EventFwk;

namespace OHOS {
namespace NetManagerStandard {
NetTetherRecvBroadcast *NetTetherRecvBroadcast::instance_ = nullptr;

NetTetherRecvBroadcast *NetTetherRecvBroadcast::GetInstance()
{
    if (instance_ == nullptr) {
        instance_ = new NetTetherRecvBroadcast();
    }
    return instance_;
}

void NetTetherRecvBroadcast::ReleaseInstance()
{
    if (instance_ != nullptr) {
        delete instance_;
        instance_ = nullptr;
    }
}

NetTetherRecvBroadcast::NetTetherRecvBroadcast() : apStateChangeCb_(nullptr), usbStateChangeCb_(nullptr) {}

NetTetherRecvBroadcast::~NetTetherRecvBroadcast() {}

bool NetTetherRecvBroadcast::AddApStateChangeSubscribe(std::function<void(int32_t)> cb)
{
    if (apStateChangeCb_ == nullptr) {
        if (SubscribeBroadcastEvent(AP_EVENT)) {
            apStateChangeCb_ = cb;
            return true;
        } else {
            return false;
        }
    }
    NETMGR_LOG_D("AddApStateChangeSubscribe repeatly.");
    return false;
}

void NetTetherRecvBroadcast::RemoveApStateChangeSubscribe()
{
    if (apStateChangeCb_ == nullptr) {
        NETMGR_LOG_D("apStateChangeCb_ is nullptr.");
        return;
    }
    apStateChangeCb_ = nullptr;
    UnsubscribeServiceEvent(AP_EVENT);
    return;
}

bool NetTetherRecvBroadcast::AddUsbStateChangeSubscribe(std::function<void(bool)> cb)
{
    if (usbStateChangeCb_ == nullptr) {
        NETMGR_LOG_D("Subscribe usb broadcast event.");
        usbStateChangeCb_ = cb;
        return true;
    }
    NETMGR_LOG_D("AddUsbStateChangeSubscribe repeatly.");
    return false;
}

void NetTetherRecvBroadcast::RemoveUsbStateChangeSubscribe()
{
    if (usbStateChangeCb_ == nullptr) {
        NETMGR_LOG_D("usbStateChangeCb_ is nullptr.");
        return;
    }
    usbStateChangeCb_ = nullptr;
    NETMGR_LOG_D("Unsubscribe usb service event.");
}

bool NetTetherRecvBroadcast::SubscribeBroadcastEvent(const std::string &event)
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(event);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<NetTetherEventSubscriber> subscriber = std::make_shared<NetTetherEventSubscriber>(subscriberInfo);
    NETMGR_LOG_I("Subscribe event: [%{public}s]", event.c_str());
    bool subscribeResult = CommonEventManager::SubscribeCommonEvent(subscriber);
    if (subscribeResult) {
        mapEventSubscriber_[event] = subscriber;
    } else {
        NETMGR_LOG_E("Subscribe service event fail: [%{public}s]", event.c_str());
    }
    return subscribeResult;
}

bool NetTetherRecvBroadcast::UnsubscribeServiceEvent(const std::string &event)
{
    std::map<std::string, std::shared_ptr<NetTetherEventSubscriber>>::iterator iter = mapEventSubscriber_.find(event);
    if (iter == mapEventSubscriber_.end()) {
        return false;
    }

    bool unsubscribeResult = CommonEventManager::UnSubscribeCommonEvent(iter->second);
    if (!unsubscribeResult) {
        NETMGR_LOG_E("Unsubscribe event fail: [%{public}s]", event.c_str());
    }
    return unsubscribeResult;
}

std::function<void(int32_t)> NetTetherRecvBroadcast::GetApEventCb()
{
    return apStateChangeCb_;
}

std::function<void(bool)> NetTetherRecvBroadcast::GetUsbEventCb()
{
    return usbStateChangeCb_;
}

NetTetherRecvBroadcast::NetTetherEventSubscriber::NetTetherEventSubscriber(
    const OHOS::EventFwk::CommonEventSubscribeInfo &subscribeInfo) : CommonEventSubscriber(subscribeInfo) {}

NetTetherRecvBroadcast::NetTetherEventSubscriber::~NetTetherEventSubscriber() {}

void NetTetherRecvBroadcast::NetTetherEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    std::string event = data.GetWant().GetAction();
    NETMGR_LOG_I("Received event: [%{public}s]", event.c_str());

    if (event == AP_EVENT && NetTetherRecvBroadcast::GetInstance()->GetApEventCb() != nullptr) {
        std::function<void(int32_t)> cb = NetTetherRecvBroadcast::GetInstance()->GetApEventCb();
        int32_t code = data.GetCode();
        cb(code);
    }
}
} // namespace NetManagerStandard
} // namespace OHOS