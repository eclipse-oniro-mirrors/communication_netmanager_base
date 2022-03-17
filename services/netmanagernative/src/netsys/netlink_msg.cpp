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

#include "netlink_msg.h"
#include "securec.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace nmd {
NetlinkMsg::NetlinkMsg(uint16_t flags, size_t maxBufLen, int pid)
{
    this->maxBufLen = maxBufLen;
    this->netlinkMessage = reinterpret_cast<struct nlmsghdr *>(malloc(NLMSG_SPACE(maxBufLen)));
    errno_t result = memset_s(this->netlinkMessage, NLMSG_SPACE(maxBufLen), 0, NLMSG_SPACE(maxBufLen));
    if (result != 0) {
        NETNATIVE_LOGE("[NetlinkMessage]: memset result %{public}d", result);
    }
    this->netlinkMessage->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
    this->netlinkMessage->nlmsg_pid = static_cast<uint32_t>(pid);
    this->netlinkMessage->nlmsg_seq = 1;
}

NetlinkMsg::~NetlinkMsg()
{
    delete this->netlinkMessage;
}

void NetlinkMsg::AddRoute(unsigned short action, struct rtmsg msg)
{
    this->netlinkMessage->nlmsg_type = action;
    int32_t result = memcpy_s(NLMSG_DATA(this->netlinkMessage), sizeof(struct rtmsg), &msg, sizeof(struct rtmsg));
    if (result != 0) {
        NETNATIVE_LOGE("[AddRoute]: string copy failed result %{public}d", result);
    }
    this->netlinkMessage->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
}

void NetlinkMsg::AddRule(unsigned short action, struct fib_rule_hdr msg)
{
    this->netlinkMessage->nlmsg_type = action;
    int32_t result = memcpy_s(NLMSG_DATA(this->netlinkMessage), sizeof(struct fib_rule_hdr),
        &msg, sizeof(struct fib_rule_hdr));
    if (result != 0) {
        NETNATIVE_LOGE("[AddRule]: string copy failed result %{public}d", result);
    }
    this->netlinkMessage->nlmsg_len = NLMSG_LENGTH(sizeof(struct fib_rule_hdr));
}

void NetlinkMsg::AddAddress(unsigned short action, struct ifaddrmsg msg)
{
    this->netlinkMessage->nlmsg_type = action;
    int32_t result = memcpy_s(NLMSG_DATA(this->netlinkMessage), sizeof(struct ifaddrmsg),
        &msg, sizeof(struct ifaddrmsg));
    if (result != 0) {
        NETNATIVE_LOGE("[AddAddress]: string copy failed result %{public}d", result);
    }
    this->netlinkMessage->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
}

int NetlinkMsg::AddAttr(unsigned int type, void *data, size_t alen)
{
    if (alen == 0) {
        NETNATIVE_LOGE("[NetlinkMessage]: length  data can not be 0");
        return -1;
    }

    if (data == nullptr) {
        NETNATIVE_LOGE("[NetlinkMessage]: attr data can not be null");
        return -1;
    }

    int len = RTA_LENGTH(alen);
    if (NLMSG_ALIGN(this->netlinkMessage->nlmsg_len) + RTA_ALIGN(len) > this->maxBufLen) {
        NETNATIVE_LOGE("[NetlinkMessage]: attr length than max len: %{public}d", (int32_t)this->maxBufLen);
        return -1;
    }

    struct rtattr *rta =
        (struct rtattr *)(((char *)this->netlinkMessage) + NLMSG_ALIGN(this->netlinkMessage->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = static_cast<uint16_t>(len);

    if (data != nullptr) {
        int32_t result = memcpy_s(RTA_DATA(rta), alen, data, alen);
        if (result != 0) {
            NETNATIVE_LOGE("[get_addr_info]: string copy failed result %{public}d", result);
            return -1;
        }
    }

    this->netlinkMessage->nlmsg_len = NLMSG_ALIGN(this->netlinkMessage->nlmsg_len) + RTA_ALIGN(len);
    return 1;
}

int NetlinkMsg::AddAttr16(unsigned int type, uint16_t data)
{
    return this->AddAttr(type, &data, sizeof(uint16_t));
}

int NetlinkMsg::AddAttr32(unsigned int type, uint32_t data)
{
    return this->AddAttr(type, &data, sizeof(uint32_t));
}

nlmsghdr *NetlinkMsg::GetNetLinkMessage()
{
    return this->netlinkMessage;
}
} // namespace nmd
} // namespace OHOS
