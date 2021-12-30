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

#include "net_conn_callback_info.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool NetConnCallbackInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(netState_)) {
        return false;
    }
    if (!parcel.WriteUint32(netType_)) {
        return false;
    }

    return true;
}

sptr<NetConnCallbackInfo> NetConnCallbackInfo::Unmarshalling(Parcel &parcel)
{
    sptr<NetConnCallbackInfo> ptr = (std::make_unique<NetConnCallbackInfo>()).release();
    if (ptr == nullptr) {
        NETMGR_LOG_E("The parameter of ptr is nullptr");
        return nullptr;
    }

    if (!parcel.ReadInt32(ptr->netState_)) {
        return nullptr;
    }
    if (!parcel.ReadUint32(ptr->netType_)) {
        return nullptr;
    }

    return ptr;
}

bool NetConnCallbackInfo::Marshalling(Parcel &parcel, const sptr<NetConnCallbackInfo> &object)
{
    if (object == nullptr) {
        NETMGR_LOG_E("NetConnCallbackInfo object is nullptr");
        return false;
    }

    if (!parcel.WriteInt32(object->netState_)) {
        NETMGR_LOG_E("Write netState_ failed");
        return false;
    }
    if (!parcel.WriteUint32(object->netType_)) {
        NETMGR_LOG_E("Write netType_ failed");
        return false;
    }

    return true;
}

std::string NetConnCallbackInfo::ToString(const std::string &tab) const
{
    std::string str;
    str.append("\n");
    str.append(tab);
    str.append("[NetConnCallbackInfo]");

    str.append("\n");
    str.append(tab);
    str.append("netState_ = ");
    str.append(std::to_string(netState_));

    str.append("\n");
    str.append(tab);
    str.append("netType_ = ");
    str.append(std::to_string(netType_));

    return str;
}
} // namespace NetManagerStandard
} // namespace OHOS