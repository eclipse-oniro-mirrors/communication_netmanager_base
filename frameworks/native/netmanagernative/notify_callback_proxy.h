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
#ifndef  NOTIFY_CALLBACK_PROXY_H__
#define  NOTIFY_CALLBACK_PROXY_H__
#include "iremote_proxy.h"

#include "i_notify_callback.h"

namespace OHOS {
namespace NetdNative {
class NotifyCallbackProxy : public IRemoteProxy<INotifyCallback> {
public:
    explicit NotifyCallbackProxy(const sptr<IRemoteObject> &impl);
    virtual ~NotifyCallbackProxy();

public:
    int32_t OnInterfaceAddressUpdated(const std::string &, const std::string &, int, int) override;
    int32_t OnInterfaceAddressRemoved(const std::string &, const std::string &, int, int) override;
    int32_t OnInterfaceAdded(const std::string &) override;
    int32_t OnInterfaceRemoved(const std::string &) override;
    int32_t OnInterfaceChanged(const std::string &, bool) override;
    int32_t OnInterfaceLinkStateChanged(const std::string &, bool) override;
    int32_t OnRouteChanged(bool, const std::string &, const std::string &, const std::string &) override;
    int32_t OnDhcpSuccess(sptr<DhcpResultParcel> &dhcpResult) override;

private:
    static inline BrokerDelegator<NotifyCallbackProxy> delegator_;
};
} // namespace NetdNative
} // namespace OHOS
#endif
