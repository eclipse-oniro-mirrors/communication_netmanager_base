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

#include <securec.h>
#include "netnative_log_wrapper.h"
#include "netsys_addr_info_parcel.h"

namespace OHOS {
namespace NetsysNative {
using namespace  std;
NetsysAddrInfoParcel::NetsysAddrInfoParcel(const struct addrinfo* addr, const uint16_t netId,
    const char *Node, const char *Service): Head(nullptr)
{
    NETNATIVE_LOGE("Construct   begin");
    ai_family=addr->ai_family;
    ai_socktype=addr->ai_socktype;
    ai_flags=addr->ai_flags;
    ai_protocol=addr->ai_protocol;
    ai_addrlen=addr->ai_addrlen;
    netid=netId;
    NETNATIVE_LOGE("Construct   begin");
    if (Node != NULL) {
        node=std::string(Node);
    } else {
        node="";
    }
    NETNATIVE_LOGE("Construct   over");
    if (Service != NULL) {
        service=std::string(Service);
    } else {
        service="";
    }
    NETNATIVE_LOGE("Construct   over");
}

bool NetsysAddrInfoParcel::Marshalling(Parcel &parcel) const
{
    NETNATIVE_LOGE("Marshalling  begin");
    if (!parcel.WriteInt16(ai_family)) {
        return false;
    }
    if (!parcel.WriteInt16(ai_socktype)) {
        return false;
    }
    if (!parcel.WriteInt16(ai_flags)) {
        return false;
    }
    if (!parcel.WriteInt16(ai_protocol)) {
        return false;
    }
    if (!parcel.WriteInt16(netid)) {
        return false;
    }
    if (!parcel.WriteString(node)) {
        return false;
    }
    if (!parcel.WriteString(service)) {
        return false;
    }
    NETNATIVE_LOGE("Marshalling  over");
    return true;
}

sptr<NetsysAddrInfoParcel> NetsysAddrInfoParcel::Unmarshalling(Parcel &parcel)
{
    Parcel *pParcel = &parcel;
    MessageParcel  *parcelMsg = static_cast<MessageParcel*>(pParcel);
    sptr<NetsysAddrInfoParcel> ptr = new (std::nothrow) NetsysAddrInfoParcel();
    if (ptr == nullptr) {
        return nullptr;
    }
    struct addrinfo *Head, *pointer, *cur;
    ptr->ret = parcelMsg->ReadInt32();
    ptr->addrSize=parcelMsg->ReadInt32();
    NETNATIVE_LOGE("Log return ret = %{public}d, size %{public}d", ptr->ret, ptr->addrSize);
    Head=(addrinfo *)malloc(sizeof(addrinfo));
    if (Head == nullptr) {
        return  nullptr;
    }
    Head->ai_next = NULL;
    pointer = Head;
    struct addrinfo addrints;
    int  k = 0;
    int size = ptr->addrSize;
    while (size--) {
        cur = (addrinfo *)malloc(sizeof(addrinfo));
        if (cur == nullptr)
            break;
        bzero(&addrints, sizeof(addrinfo));
        addrints.ai_flags = parcelMsg->ReadInt16();
        addrints.ai_family = parcelMsg->ReadInt16();
        addrints.ai_socktype = parcelMsg->ReadInt16();
        addrints.ai_protocol = parcelMsg->ReadInt16();
        addrints.ai_addrlen = static_cast<socklen_t>(parcelMsg->ReadUint32());
        int canSize = parcelMsg->ReadInt16();
        addrints.ai_canonname = NULL;
        const uint8_t *buffer1 = canSize > 0?parcelMsg->ReadBuffer(canSize):nullptr;
        if (buffer1 != nullptr) {
            int copyRet = memcpy_s(addrints.ai_canonname, canSize, buffer1, canSize);
            if (copyRet != 0) {
                NETNATIVE_LOGE("copyRet = %{public}d", copyRet);
            }
        }
        addrints.ai_addr = (struct  sockaddr *) parcelMsg->ReadRawData(sizeof(struct sockaddr));
        addrints.ai_next = NULL;
        memcpy_s(cur, sizeof(addrinfo), &addrints, sizeof(addrinfo));
        if (k == 0) {
            memcpy_s(Head, sizeof(addrinfo), &addrints, sizeof(addrinfo));
            Head->ai_next = NULL;
        } else {
            pointer->ai_next=cur;
            pointer=cur;
        }
        k++;
    }
    ptr->Head=Head;
    return  ptr;
}
} // namespace NetManagerStandard
} // namespace OHOS