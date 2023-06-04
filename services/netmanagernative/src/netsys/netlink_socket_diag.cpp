/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "netlink_socket_diag.h"

#include <cstring>
#include <unistd.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "net_manager_constants.h"
#include "netnative_log_wrapper.h"
#include "fwmark.h"
#include "securec.h"

namespace OHOS {
namespace nmd {
using namespace NetManagerStandard;

namespace {
constexpr uint32_t KERNEL_BUFFER_SIZE = 8192U;
constexpr uint8_t ADDR_POSITION = 3U;

bool InLookBack(uint32_t hostLong)
{
    return ((hostLong & 0xff000000) == 0x7f000000);
}

int32_t CheckError(int fd)
{
    struct {
        nlmsghdr h;
        nlmsgerr err;
    } ack;
    ssize_t bytesread = recv(fd, &ack, sizeof(ack), MSG_DONTWAIT | MSG_PEEK);
    if (bytesread == -1) {
        return (errno == EAGAIN) ? NETMANAGER_SUCCESS : -errno;
    }
    if (bytesread == static_cast<ssize_t>(sizeof(ack)) && ack.h.nlmsg_type == NLMSG_ERROR) {
        recv(fd, &ack, sizeof(ack), 0);
        return ack.err.error;
    }
    return NETMANAGER_SUCCESS;
}

bool IsLoopbackSocket(const inet_diag_msg *msg)
{
    switch (msg->idiag_family) {
        case AF_INET:
            return InLookBack(htonl(msg->id.idiag_src[0])) || InLookBack(htonl(msg->id.idiag_dst[0])) ||
                   msg->id.idiag_src[0] == msg->id.idiag_dst[0];

        case AF_INET6: {
            const struct in6_addr *src = (const struct in6_addr *)&msg->id.idiag_src;
            const struct in6_addr *dst = (const struct in6_addr *)&msg->id.idiag_dst;
            return (IN6_IS_ADDR_V4MAPPED(src) && InLookBack(src->s6_addr32[ADDR_POSITION])) ||
                   (IN6_IS_ADDR_V4MAPPED(dst) && InLookBack(dst->s6_addr32[ADDR_POSITION])) ||
                   IN6_IS_ADDR_LOOPBACK(src) || IN6_IS_ADDR_LOOPBACK(dst) || !memcmp(src, dst, sizeof(*src));
        }
        default:
            return false;
    }
}
} // namespace

NetLinkSocketDiag::~NetLinkSocketDiag()
{
    CloseSocks();
}

bool NetLinkSocketDiag::Connect()
{
    if (sock_ != -1 && writeSock_ != -1) {
        return false;
    }

    sock_ = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_INET_DIAG);
    writeSock_ = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_INET_DIAG);
    if (sock_ == -1 || writeSock_ == -1) {
        CloseSocks();
        return false;
    }

    sockaddr_nl nl = {.nl_family = AF_NETLINK};
    if ((connect(sock_, reinterpret_cast<sockaddr *>(&nl), sizeof(nl)) == -1) ||
        (connect(writeSock_, reinterpret_cast<sockaddr *>(&nl), sizeof(nl)) == -1)) {
        CloseSocks();
        return false;
    }
    return true;
}

int32_t NetLinkSocketDiag::SendDumpRequest(uint8_t proto, uint8_t family, uint32_t states, iovec *iov, int iovcnt)
{
    Request request = {
        .nlh_ =
            {
                .nlmsg_type = SOCK_DIAG_BY_FAMILY,
                .nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP,
            },
        .req_ =
            {
                .sdiag_family = family,
                .sdiag_protocol = proto,
                .idiag_states = states,
            },
    };

    size_t len = 0;
    iov[0].iov_base = &request;
    iov[0].iov_len = sizeof(request);
    for (int32_t i = 0; i < iovcnt; i++) {
        len += iov[i].iov_len;
    }
    request.nlh_.nlmsg_len = len;

    if (writev(sock_, iov, iovcnt) != static_cast<ssize_t>(len)) {
        NETNATIVE_LOGE("Write dump request failed errno:%{public}d, strerror:%{public}s", errno, strerror(errno));
        return NETMANAGER_ERR_INTERNAL;
    }

    return CheckError(sock_);
}

int32_t NetLinkSocketDiag::ReadDiagMsg(uint8_t proto, const NetLinkSocketDiag::DestroyFilter &shouldDestroy)
{
    NetlinkDumpCallback callback = [this, proto, shouldDestroy](nlmsghdr *nlh) {
        const inet_diag_msg *msg = reinterpret_cast<inet_diag_msg *>(NLMSG_DATA(nlh));
        if (shouldDestroy(proto, msg)) {
            DestroySocket(proto, msg);
        }
    };
    return ProcessNetlinkDump(sock_, callback);
}

int32_t NetLinkSocketDiag::ProcessNetlinkDump(int32_t sock, const NetlinkDumpCallback &callback)
{
    char buf[KERNEL_BUFFER_SIZE];

    ssize_t readBytes = read(sock, buf, sizeof(buf));
    if (readBytes < 0) {
        NETNATIVE_LOGE("Failed to read socket, errno:%{public}d, strerror:%{public}s", errno, strerror(errno));
        return NETMANAGER_ERR_INTERNAL;
    }
    while (readBytes > 0) {
        uint32_t len = readBytes;
        for (nlmsghdr *nlh = reinterpret_cast<nlmsghdr *>(buf); NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
            if (nlh->nlmsg_type == NLMSG_ERROR) {
                nlmsgerr *err = reinterpret_cast<nlmsgerr *>(NLMSG_DATA(nlh));
                NETNATIVE_LOGE("Error netlink msg, errno:%{public}d, strerror:%{public}s", -err->error,
                               strerror(-err->error));
                return err->error;
            } else if (nlh->nlmsg_type == NLMSG_DONE) {
                return NETMANAGER_SUCCESS;
            } else {
                callback(nlh);
            }
        }
        readBytes = read(sock, buf, sizeof(buf));
        if (readBytes < 0) {
            return -errno;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetLinkSocketDiag::DestroySocket(uint8_t proto, const inet_diag_msg *msg)
{
    if (msg == nullptr) {
        NETNATIVE_LOGE("inet_diag_msg is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    Request destroyRequest = {
        .nlh_ =
            {
                .nlmsg_type = SOCK_DESTROY,
                .nlmsg_flags = NLM_F_REQUEST,
            },
        .req_ =
            {
                .sdiag_family = msg->idiag_family,
                .sdiag_protocol = proto,
                .idiag_states = static_cast<uint32_t>(1 << msg->idiag_state),
                .id = msg->id,
            },
    };
    destroyRequest.nlh_.nlmsg_len = sizeof(destroyRequest);

    if (write(writeSock_, &destroyRequest, sizeof(destroyRequest)) < static_cast<ssize_t>(sizeof(destroyRequest))) {
        NETNATIVE_LOGE("Write socket request failed errno:%{public}d, strerror:%{public}s", errno, strerror(errno));
        return NETMANAGER_ERR_INTERNAL;
    }
    int32_t ret = CheckError(writeSock_);
    if (ret == NETMANAGER_SUCCESS) {
        socketsDestroyed_++;
    }
    return ret;
}

int32_t NetLinkSocketDiag::DestroyLiveSockets(const DestroyFilter &destroyFilter, iovec *iov, int iovcnt)
{
    const int32_t proto = IPPROTO_TCP;
    const uint32_t states = (1 << TCP_ESTABLISHED) | (1 << TCP_SYN_SENT) | (1 << TCP_SYN_RECV);

    for (const int family : {AF_INET, AF_INET6}) {
        if (int32_t ret = SendDumpRequest(proto, family, states, iov, iovcnt)) {
            NETNATIVE_LOGE("Failed to dump %{public}s sockets", family == AF_INET ? "IPv4" : "IPv6");
            return ret;
        }
        if (int32_t ret = ReadDiagMsg(proto, destroyFilter)) {
            NETNATIVE_LOGE("Failed to destroy %{public}s sockets", family == AF_INET ? "IPv4" : "IPv6");
            return ret;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetLinkSocketDiag::DestroySocketsLackingNetwork(uint16_t netId, bool excludeLoopback)
{
    NETNATIVE_LOG_D("DestroySocketsLackingNetwork in, netid: %{public}d", netId);
    if (!Connect()) {
        NETNATIVE_LOGE("Error closing sockets for netId change");
        return NETMANAGER_ERR_INTERNAL;
    }

    uint8_t matchLen = sizeof(MarkMatch);
    uint8_t byteCodeLen = sizeof(ByteCode);
    uint8_t jmpLen = sizeof(inet_diag_bc_op);
    uint8_t rejectOffSet = sizeof(inet_diag_bc_op);

    Fwmark netIdMark;
    netIdMark.netId = netId;
    Fwmark netIdMask;
    netIdMask.netId = 0xffff;

    Fwmark controlMark;
    controlMark.explicitlySelected = true;
    controlMark.permission = PERMISSION_NETWORK;

    ByteCode byteCode = {
        // If netId matches, continue, otherwise, leave socket alone.
        {{INET_DIAG_BC_MARK_COND, matchLen, byteCodeLen + rejectOffSet}, netIdMark.intValue, netIdMask.intValue},
        // If the permission bits match, jump to the section below that rejects the socket.
        {{INET_DIAG_BC_MARK_COND, matchLen, matchLen + jmpLen}, controlMark.intValue, controlMark.intValue},
        // Unconditionally rejects the packet by jumping to the reject target.
        {INET_DIAG_BC_JMP, jmpLen, jmpLen + rejectOffSet},
    };

    nlattr nla = {
        .nla_len = sizeof(struct nlattr) + byteCodeLen,
        .nla_type = INET_DIAG_REQ_BYTECODE,
    };

    iovec iov[] = {
        {nullptr, 0},
        {&nla, sizeof(nla)},
        {&byteCode, byteCodeLen},
    };

    int ret = DestroyLiveSockets(
        [&](uint8_t, const inet_diag_msg *msg) {
            return msg != nullptr && !(excludeLoopback && IsLoopbackSocket(msg));
        },
        iov, (sizeof(iov) / sizeof(*iov)));
    if (ret) {
        NETNATIVE_LOGE("Failed to destroy live sockets");
        return ret;
    }

    NETNATIVE_LOG_D("Destroyed %{public}d sockets for netId %{publib}d", socketsDestroyed_, netId);
    return NETMANAGER_SUCCESS;
}
} // namespace nmd
} // namespace OHOS