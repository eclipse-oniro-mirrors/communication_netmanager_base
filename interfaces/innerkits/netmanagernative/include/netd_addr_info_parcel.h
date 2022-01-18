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

#ifndef NETD_ADDR_INFO_H
#define NETD_ADDR_INFO_H

#include  <netdb.h>
#include <string>
#include <vector>
#include "message_parcel.h"
#include "parcel.h"

namespace OHOS {
namespace NetdNative {
struct  NetdAddrInfoParcel  final  : public Parcelable {
public:
    NetdAddrInfoParcel() {};
    NetdAddrInfoParcel(const  struct addrinfo* addr, const  uint16_t netId, const char *Node, const char *Service);
    ~NetdAddrInfoParcel() {};
    struct addrinfo  *Head;
    int32_t   ai_family;
    int32_t   ai_socktype;
    int32_t   ai_flags;
    int32_t   ai_protocol;
    int32_t   ai_addrlen;
    int32_t   netid;
    std::string   node;
    std::string   service;
    int32_t   ret;
    int32_t   addrSize;
    
    virtual bool Marshalling(Parcel &parcel) const override;
    static sptr<NetdAddrInfoParcel> Unmarshalling(Parcel &parcel);
};
} // namespace NetdNative
} // namespace OHOS
#endif