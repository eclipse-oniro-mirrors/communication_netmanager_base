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

#include <arpa/inet.h>
#include <cstring>
#include <net/if.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <unistd.h>

#include "fwmark.h"
#include "net_manager_constants.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace nmd {
using namespace NetManagerStandard;

namespace {
constexpr uint32_t KERNEL_BUFFER_SIZE = 8192U;
constexpr uint8_t ADDR_POSITION = 3U;
constexpr int32_t DOMAIN_IP_ADDR_MAX_LEN = 128;
constexpr uint32_t LOCKBACK_MASK = 0xff000000;
constexpr uint32_t LOCKBACK_DEFINE = 0x7f000000;
} // namespace

NetLinkSocketDiag::~NetLinkSocketDiag()
{
    CloseNetlinkSocket();
}

bool NetLinkSocketDiag::InLookBack(uint32_t a)
{
    return (a & LOCKBACK_MASK) == LOCKBACK_DEFINE;
}

bool NetLinkSocketDiag::CreateNetlinkSocket()
{
    dumpSock_ = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_INET_DIAG);
    if (dumpSock_ < 0) {
        NETNATIVE_LOGE("Create netlink socket for dump failed, error[%{public}d]: %{public}s", errno, strerror(errno));
        return false;
    }

    destroySock_ = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_INET_DIAG);
    if (destroySock_ < 0) {
        NETNATIVE_LOGE("Create netlink socket for destroy failed, error[%{public}d]: %{public}s", errno,
                       strerror(errno));
        close(dumpSock_);
        return false;
    }

    sockaddr_nl nl = {.nl_family = AF_NETLINK};
    if ((connect(dumpSock_, reinterpret_cast<sockaddr *>(&nl), sizeof(nl)) < 0) ||
        (connect(destroySock_, reinterpret_cast<sockaddr *>(&nl), sizeof(nl)) < 0)) {
        NETNATIVE_LOGE("Connect to netlink socket failed, error[%{public}d]: %{public}s", errno, strerror(errno));
        CloseNetlinkSocket();
        return false;
    }
    return true;
}

void NetLinkSocketDiag::CloseNetlinkSocket()
{
    close(dumpSock_);
    close(destroySock_);
    dumpSock_ = -1;
    destroySock_ = -1;
}

int32_t NetLinkSocketDiag::ExecuteDestroySocket(uint8_t proto, const inet_diag_msg *msg)
{
    if (msg == nullptr) {
        NETNATIVE_LOGE("inet_diag_msg is nullptr");
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }

    SockDiagRequest request;
    request.nlh_.nlmsg_type = SOCK_DESTROY;
    request.nlh_.nlmsg_flags = NLM_F_REQUEST;
    request.nlh_.nlmsg_len = sizeof(request);

    request.req_ = {.sdiag_family = msg->idiag_family,
                    .sdiag_protocol = proto,
                    .idiag_states = static_cast<uint32_t>(1 << msg->idiag_state),
                    .id = msg->id};
    ssize_t writeLen = write(destroySock_, &request, sizeof(request));
    if (writeLen < static_cast<ssize_t>(sizeof(request))) {
        NETNATIVE_LOGE("Write destroy request to socket failed errno[%{public}d]: strerror:%{public}s", errno,
                       strerror(errno));
        return NETMANAGER_ERR_INTERNAL;
    }

    int32_t ret = GetErrorFromKernel(destroySock_);
    if (ret == NETMANAGER_SUCCESS) {
        socketsDestroyed_++;
    }
    return ret;
}

int32_t NetLinkSocketDiag::GetErrorFromKernel(int32_t fd)
{
    Ack ack;
    ssize_t bytesread = recv(fd, &ack, sizeof(ack), MSG_DONTWAIT | MSG_PEEK);
    if (bytesread < 0) {
        NETNATIVE_LOGE("Get error info from kernel failed errno[%{public}d]: strerror:%{public}s", errno,
                       strerror(errno));
        return (errno == EAGAIN) ? NETMANAGER_SUCCESS : -errno;
    }
    if (bytesread == static_cast<ssize_t>(sizeof(ack)) && ack.hdr_.nlmsg_type == NLMSG_ERROR) {
        recv(fd, &ack, sizeof(ack), 0);
        NETNATIVE_LOGE("Receive NLMSG_ERROR:[%{public}d] from kernel", ack.err_.error);
        return NETMANAGER_ERR_INTERNAL;
    }
    return NETMANAGER_SUCCESS;
}

bool NetLinkSocketDiag::IsLoopbackSocket(const inet_diag_msg *msg)
{
    if (msg->idiag_family == AF_INET) {
        return InLookBack(htonl(msg->id.idiag_src[0])) || InLookBack(htonl(msg->id.idiag_dst[0]));
    }

    if (msg->idiag_family == AF_INET6) {
        const struct in6_addr *src = (const struct in6_addr *)&msg->id.idiag_src;
        const struct in6_addr *dst = (const struct in6_addr *)&msg->id.idiag_dst;
        return (IN6_IS_ADDR_V4MAPPED(src) && InLookBack(src->s6_addr32[ADDR_POSITION])) ||
               (IN6_IS_ADDR_V4MAPPED(dst) && InLookBack(dst->s6_addr32[ADDR_POSITION])) || IN6_IS_ADDR_LOOPBACK(src) ||
               IN6_IS_ADDR_LOOPBACK(dst);
    }
    return false;
}

bool NetLinkSocketDiag::IsMatchNetwork(const inet_diag_msg *msg, const std::string &ipAddr)
{
    if (msg->idiag_family == AF_INET) {
        if (CommonUtils::GetAddrFamily(ipAddr) != AF_INET) {
            return false;
        }

        in_addr_t addr = inet_addr(ipAddr.c_str());
        if (addr == msg->id.idiag_src[0] || addr == msg->id.idiag_dst[0]) {
            return true;
        }
    }

    if (msg->idiag_family == AF_INET6) {
        if (CommonUtils::GetAddrFamily(ipAddr) != AF_INET6) {
            return false;
        }

        char src[DOMAIN_IP_ADDR_MAX_LEN] = {0};
        char dst[DOMAIN_IP_ADDR_MAX_LEN] = {0};
        inet_ntop(AF_INET6, msg->id.idiag_src, src, sizeof(src));
        inet_ntop(AF_INET6, msg->id.idiag_dst, dst, sizeof(dst));
        if (src == ipAddr || dst == ipAddr) {
            return true;
        }
    }
    return false;
}

int32_t NetLinkSocketDiag::ProcessSockDiagDumpResponse(uint8_t proto, const std::string &ipAddr, bool excludeLoopback)
{
    char buf[KERNEL_BUFFER_SIZE] = {0};
    ssize_t readBytes = read(dumpSock_, buf, sizeof(buf));
    if (readBytes < 0) {
        NETNATIVE_LOGE("Failed to read socket, errno:%{public}d, strerror:%{public}s", errno, strerror(errno));
        return NETMANAGER_ERR_INTERNAL;
    }
    while (readBytes > 0) {
        uint32_t len = static_cast<uint32_t>(readBytes);
        for (nlmsghdr *nlh = reinterpret_cast<nlmsghdr *>(buf); NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
            if (nlh->nlmsg_type == NLMSG_ERROR) {
                nlmsgerr *err = reinterpret_cast<nlmsgerr *>(NLMSG_DATA(nlh));
                NETNATIVE_LOGE("Error netlink msg, errno:%{public}d, strerror:%{public}s", -err->error,
                               strerror(-err->error));
                return err->error;
            } else if (nlh->nlmsg_type == NLMSG_DONE) {
                return NETMANAGER_SUCCESS;
            } else {
                const auto *msg = reinterpret_cast<inet_diag_msg *>(NLMSG_DATA(nlh));
                SockDiagDumpCallback(proto, msg, ipAddr, excludeLoopback);
            }
        }
        readBytes = read(dumpSock_, buf, sizeof(buf));
        if (readBytes < 0) {
            return -errno;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t NetLinkSocketDiag::SendSockDiagDumpRequest(uint8_t proto, uint8_t family, uint32_t states)
{
    SockDiagRequest request;
    size_t len = sizeof(request);
    iovec iov;
    iov.iov_base = &request;
    iov.iov_len = len;
    request.nlh_.nlmsg_type = SOCK_DIAG_BY_FAMILY;
    request.nlh_.nlmsg_flags = (NLM_F_REQUEST | NLM_F_DUMP);
    request.nlh_.nlmsg_len = len;

    request.req_ = {.sdiag_family = family, .sdiag_protocol = proto, .idiag_states = states};

    ssize_t writeLen = writev(dumpSock_, &iov, (sizeof(iov) / sizeof(iovec)));
    if (writeLen != static_cast<ssize_t>(len)) {
        NETNATIVE_LOGE("Write dump request failed errno:%{public}d, strerror:%{public}s", errno, strerror(errno));
        return NETMANAGER_ERR_INTERNAL;
    }

    return GetErrorFromKernel(dumpSock_);
}

void NetLinkSocketDiag::SockDiagDumpCallback(uint8_t proto, const inet_diag_msg *msg, const std::string &ipAddr,
                                             bool excludeLoopback)
{
    if (msg == nullptr) {
        NETNATIVE_LOGE("msg is nullptr");
        return;
    }

    if (excludeLoopback && IsLoopbackSocket(msg)) {
        NETNATIVE_LOGE("Loop back socket, no need to close.");
        return;
    }

    if (!IsMatchNetwork(msg, ipAddr)) {
        NETNATIVE_LOGE("Socket is not associated with the network");
        return;
    }

    ExecuteDestroySocket(proto, msg);
}

void NetLinkSocketDiag::DestroyLiveSockets(const char *ipAddr, bool excludeLoopback)
{
    NETNATIVE_LOG_D("DestroySocketsLackingNetwork in");
    if (ipAddr == nullptr) {
        NETNATIVE_LOGE("Ip address is nullptr.");
        return;
    }

    if (!CreateNetlinkSocket()) {
        NETNATIVE_LOGE("Create netlink diag socket failed.");
        return;
    }

    const int32_t proto = IPPROTO_TCP;
    const uint32_t states = (1 << TCP_ESTABLISHED) | (1 << TCP_SYN_SENT) | (1 << TCP_SYN_RECV);

    for (const int family : {AF_INET, AF_INET6}) {
        int32_t ret = SendSockDiagDumpRequest(proto, family, states);
        if (ret != NETMANAGER_SUCCESS) {
            NETNATIVE_LOGE("Failed to dump %{public}s sockets", family == AF_INET ? "IPv4" : "IPv6");
            break;
        }
        ret = ProcessSockDiagDumpResponse(proto, ipAddr, excludeLoopback);
        if (ret != NETMANAGER_SUCCESS) {
            NETNATIVE_LOGE("Failed to destroy %{public}s sockets", family == AF_INET ? "IPv4" : "IPv6");
            break;
        }
    }

    NETNATIVE_LOG_D("Destroyed %{public}d sockets", socketsDestroyed_);
}
} // namespace nmd
} // namespace OHOS