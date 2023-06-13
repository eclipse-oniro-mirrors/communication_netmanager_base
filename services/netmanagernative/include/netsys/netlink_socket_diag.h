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

#include <linux/netlink.h>
#include <linux/sock_diag.h>
#include <linux/inet_diag.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <unistd.h>

namespace OHOS {
namespace nmd {
class NetLinkSocketDiag final {
public:
    NetLinkSocketDiag() = default;
    ~NetLinkSocketDiag();

    /**
     * Destroy all 'active' TCP sockets that no longer exist.
     *
     * @param ipAddr Network IP address
     * @param excludeLoopback “true” to exclude loopback.
     */
    void DestroyLiveSockets(const char *ipAddr, bool excludeLoopback);

private:
    static bool InLookBack(uint32_t a);

    bool CreateNetlinkSocket();
    void CloseNetlinkSocket();
    int32_t ExecuteDestroySocket(uint8_t proto, const inet_diag_msg *msg);
    int32_t GetErrorFromKernel(int32_t fd);
    bool IsLoopbackSocket(const inet_diag_msg *msg);
    bool IsMatchNetwork(const inet_diag_msg *msg, const std::string &ipAddr);
    int32_t ProcessSockDiagDumpResponse(uint8_t proto, const std::string &ipAddr, bool excludeLoopback);
    int32_t SendSockDiagDumpRequest(uint8_t proto, uint8_t family, uint32_t states);
    void SockDiagDumpCallback(uint8_t proto, const inet_diag_msg *msg, const std::string &ipAddr, bool excludeLoopback);

private:
    struct SockDiagRequest {
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
    struct Ack {
        nlmsghdr hdr_;
        nlmsgerr err_;
    };

    int32_t dumpSock_ = -1;
    int32_t destroySock_ = -1;
    int32_t socketsDestroyed_ = 0;
};
} // namespace nmd
} // namespace OHOS
#endif // INCLUDE_NETLINK_SOCK_DIAG_H