/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <arpa/inet.h>
#include <asm/types.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <linux/fib_rules.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "netnative_log_wrapper.h"
#include "securec.h"
#include "netlink_socket.h"
namespace OHOS {
namespace nmd {
int32_t SendNetlinkMsgToKernel(struct nlmsghdr *msg)
{
    if (msg == nullptr) {
        NETNATIVE_LOGE("[NetlinkSocket] msg can not be null ");
        return -1;
    }
    int32_t kernelSocket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (kernelSocket == -1) {
        NETNATIVE_LOGE("[NetlinkSocket] create socket failed: %{public}d", errno);
        return -1;
    }
    struct iovec ioVector;
    ioVector.iov_base = msg;
    ioVector.iov_len = msg->nlmsg_len;

    struct msghdr msgHeader;
    (void)memset_s(&msgHeader, sizeof(msgHeader), 0, sizeof(msgHeader));

    struct sockaddr_nl kernel;
    memset_s(&kernel, sizeof(kernel), 0, sizeof(kernel));
    kernel.nl_family = AF_NETLINK;
    kernel.nl_groups = 0;

    msgHeader.msg_name = &kernel;
    msgHeader.msg_namelen = sizeof(kernel);
    msgHeader.msg_iov = &ioVector;
    msgHeader.msg_iovlen = 1;

    ssize_t msgState = sendmsg(kernelSocket, &msgHeader, 0);
    if (msgState == -1) {
        NETNATIVE_LOGE("[NetlinkSocket] msg can not be null ");
        close(kernelSocket);
        return -1;
    } else if (msgState == 0) {
        NETNATIVE_LOGE("[NetlinkSocket] 0 bytes send.");
        close(kernelSocket);
        return -1;
    }
    NETNATIVE_LOGI("msgState=== is %{public}zd", msgState);
    close(kernelSocket);
    return msgState;
}

int32_t RtNetlinkFlush(uint16_t getAction, uint16_t deleteAction, const char *what, uint32_t table)
{
    if (getAction != deleteAction + 1) {
        NETNATIVE_LOGE("Unknown flush type getAction=%{public}d deleteAction=%{public}d", getAction, deleteAction);
        return -EINVAL;
    }
    int32_t writeSock = OpenNetlinkSocket(NETLINK_ROUTE);
    if (writeSock < 0) {
        NETNATIVE_LOGE("OpenNetlinkSocket error, writrSock=%{public}d", writeSock);
        return writeSock;
    }
    // This is a callback to process the information read back from the kernel.
    NetlinkDumpCallback callback = [writeSock, deleteAction, table, what](nlmsghdr *nlh) {
        if (deleteAction == RTM_DELRULE) {
            if (GetRtmU32Attribute(nlh, FRA_PRIORITY) != LOCAL_PRIORITY) {
                return;
            }
        } else if (deleteAction == RTM_DELROUTE) {
            uint32_t currentTable = GetRtmU32Attribute(nlh, RTA_TABLE);
            if (currentTable != table) {
                return;
            }
            NETNATIVE_LOGI("current table : %{public}d will be delete", currentTable);
        }
        nlh->nlmsg_type = deleteAction;
        nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
        // delete unnecessary routes and rules through sockets
        if (write(writeSock, nlh, nlh->nlmsg_len) == -1) {
            NETNATIVE_LOGE("Error writing flush request: %{public}s", strerror(errno));
            return;
        }
        int32_t ret = RecvNetlinkAck(writeSock);
        if (ret != 0 && ret != -ENOENT) {
            NETNATIVE_LOGI("Flushing %{public}s: %{public}s", what, strerror(-ret));
        }
    };
    rtmsg rule = {
        .rtm_family = AF_INET,
    };
    iovec iov[] = {
        {nullptr, 0},
        {&rule, sizeof(rule)},
    };
    uint16_t flags = NLM_F_REQUEST | NLM_F_DUMP;
    int32_t iovlen = sizeof(iov) / sizeof(*(iov));
    // Request the kernel to send a list of all routes or rules.
    int32_t ret = SendNetlinkRequest(getAction, flags, iov, iovlen, &callback);
    close(writeSock);
    return ret;
}

int32_t OpenNetlinkSocket(int32_t protocol)
{
    int32_t sock = socket(AF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, protocol);
    if (sock == -1) {
        return -errno;
    }
    struct sockaddr kernel;
    memset_s(&kernel, sizeof(kernel), 0, sizeof(kernel));
    kernel.sa_family = AF_NETLINK;
    if (connect(sock, &kernel, sizeof(kernel)) == -1) {
        close(sock);
        return -errno;
    }
    return sock;
}

int32_t RecvNetlinkAck(int32_t sock)
{
    struct {
        nlmsghdr msg;
        nlmsgerr err;
    } response;

    int32_t ret = recv(sock, &response, sizeof(response), 0);
    if (ret == -1) {
        ret = -errno;
        NETNATIVE_LOGE("netlink recv failed (%{public}s)", strerror(-ret));
        return ret;
    }

    if (ret != sizeof(response)) {
        NETNATIVE_LOGE("bad netlink response message size (%{public}d != %{public}zu)", ret, sizeof(response));
        return -EBADMSG;
    }

    return response.err.error;
}

int32_t SendNetlinkRequest(uint16_t action, uint16_t flags, iovec *iov, int32_t iovlen,
    const NetlinkDumpCallback *callback)
{
    int32_t sock = OpenNetlinkSocket(NETLINK_ROUTE);
    if (sock < 0) {
        return sock;
    }
    nlmsghdr nlmsg = {
        .nlmsg_type = action,
        .nlmsg_flags = flags,
    };
    iov[0].iov_base = &nlmsg;
    iov[0].iov_len = sizeof(nlmsg);
    for (int32_t i = 0; i < iovlen; ++i) {
        nlmsg.nlmsg_len += iov[i].iov_len;
    }
    ssize_t writevRet = writev(sock, iov, iovlen);
    iov[0] = {nullptr, 0};
    int32_t ret = 1;
    if (writevRet == -1) {
        ret = -errno;
        NETNATIVE_LOGE("netlink socket connect/writev failed (%{public}s)", strerror(-ret));
        close(sock);
        return ret;
    };
    ret = ProcessNetlinkDump(sock, *callback);
    close(sock);
    return ret;
}

int32_t ProcessNetlinkDump(int32_t sock, const NetlinkDumpCallback &callback)
{
    char buf[KNETLINK_DUMP_BUFFER_SIZE];
    ssize_t bytesread;
    do {
        // Read the information returned by the kernel through the socket.
        bytesread = read(sock, buf, sizeof(buf));
        if (bytesread < 0) {
            return -1;
        }
        uint32_t len = bytesread;
        // Traverse and read the information returned by the kernel for item by item processing.
        for (nlmsghdr *nlh = reinterpret_cast<nlmsghdr *>(buf); NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
            switch (nlh->nlmsg_type) {
                case NLMSG_DONE:
                    return 0;
                case NLMSG_ERROR: {
                    nlmsgerr *err = reinterpret_cast<nlmsgerr *>(NLMSG_DATA(nlh));
                    NETNATIVE_LOGE("netlink read socket failed error = %{public}d", err->error);
                    return err->error;
                }
                default:
                    callback(nlh);
            }
        }
    } while (bytesread > 0);
    return 0;
}

// It is used to extract the information returned by the kernel and decide whether to delete the configuration.
uint32_t GetRtmU32Attribute(const nlmsghdr *nlh, int32_t attribute)
{
    uint32_t rta_len = RTM_PAYLOAD(nlh);
    rtmsg *msg = reinterpret_cast<rtmsg *>(NLMSG_DATA(nlh));
    rtattr *rta = reinterpret_cast<rtattr *> RTM_RTA(msg);
    for (; RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
        if (rta->rta_type == attribute) {
            return *(reinterpret_cast<uint32_t *>(RTA_DATA(rta)));
        }
    }
    return 0;
}
} // namespace nmd
} // namespace OHOS