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

#ifndef INCLUDE_NETLINK_SOCK_DIAG_H
#define INCLUDE_NETLINK_SOCK_DIAG_H

#include <unistd.h>
#include <sys/socket.h>

#include <linux/netlink.h>
#include <linux/sock_diag.h>
#include <linux/inet_diag.h>

#include <functional>
#include <set>

namespace OHOS {
namespace nmd {
class NetLinkSocketDiag {
public:
    NetLinkSocketDiag() = default;
    ~NetLinkSocketDiag();

    /**
     * Destroy all 'active' TCP sockets that no longer exist.
     *
     * @param netId Network ID
     * @param excludeLoopback “ture” to exclude loopback.
     * @return Returns 0, destroy successfully, otherwise it will fail.
     */
    int32_t DestroySocketsLackingNetwork(uint16_t netId, bool excludeLoopback);

private:
    inline void CloseSocks()
    {
        close(sock_);
        close(writeSock_);
        sock_ = writeSock_ = -1;
    }

    using DestroyFilter = std::function<bool(uint8_t, const inet_diag_msg *)>;
    using NetlinkDumpCallback = std::function<void(nlmsghdr *)>;
    bool Connect();
    int32_t ReadDiagMsg(uint8_t proto, const DestroyFilter &callback);
    int32_t RockDestroy(uint8_t proto, const inet_diag_msg *msg);
    int32_t DestroySocket(uint8_t proto, const inet_diag_msg *msg);
    int32_t SendDumpRequest(uint8_t proto, uint8_t family, uint32_t states, iovec *iov, int iovcnt);
    int DestroySockets(uint8_t proto, int family, const char *addrstr);
    int DestroyLiveSockets(const DestroyFilter &destroy, iovec *iov, int iovcnt);
    int32_t ProcessNetlinkDump(int32_t sock, const NetlinkDumpCallback &callback);

private:
    struct Request {
        nlmsghdr nlh_;
        inet_diag_req_v2 req_;
    };
    struct MarkMatch {
        inet_diag_bc_op op_;
        uint32_t mark_;
        uint32_t mask_;
    };
    struct ByteCode {
        MarkMatch netIdMatch_;
        MarkMatch controlMatch_;
        inet_diag_bc_op controlJump_;
    };

    int32_t sock_ = -1;
    int32_t writeSock_ = -1;
    int32_t socketsDestroyed_ = 0;
};
} // namespace nmd
} // namespace OHOS
#endif // INCLUDE_NETLINK_SOCK_DIAG_H