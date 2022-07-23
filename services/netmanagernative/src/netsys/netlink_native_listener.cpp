/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "netlink_native_listener.h"

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>

#include <cerrno>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "netlink_message_decoder.h"
#include "netlink_define.h"
#include "netnative_log_wrapper.h"
#include "socket_client.h"

namespace OHOS {
namespace nmd {
using namespace NetlinkDefine;
namespace {
constexpr int32_t PRET_SIZE = 2;
}

NetlinkNativeListener::NetlinkNativeListener(int32_t socketFd, bool listen, int32_t format)
{
    Init("", socketFd, listen, false, format);
}

void NetlinkNativeListener::Init(const std::string &socketName, int32_t socketFd, bool listen, bool useCmdNum,
                                 int32_t format)
{
    listen_ = listen;
    socketName_ = socketName;
    socket_ = socketFd;
    useCmdNum_ = useCmdNum;
    format_ = format;
}

NetlinkNativeListener::~NetlinkNativeListener()
{
    if (!socketName_.empty() && socket_ > -1)
        close(socket_);

    if (ctrlPipe_[0] != -1) {
        close(ctrlPipe_[0]);
        close(ctrlPipe_[1]);
    }
    socketClients_.clear();
}

int32_t NetlinkNativeListener::OpenMonitor()
{
    return OpenMonitor(BACK_LOG);
}

int32_t NetlinkNativeListener::OpenMonitor(int32_t backlog)
{
    if (socketName_.empty() && socket_ == -1) {
        errno = EINVAL;
        NETNATIVE_LOGE("OpenMonitor: socket name or socket fd is invalid");
        return NetlinkResult::ERROR;
    } else if (!socketName_.empty()) {
        // T.B.D
        fcntl(socket_, F_SETFD, FD_CLOEXEC);
    }

    if (listen_ && listen(socket_, backlog) < 0) {
        NETNATIVE_LOGE("OpenMonitor: listen failed");
        return NetlinkResult::ERROR;
    } else if (!listen_) {
        socketClients_[socket_] = std::make_unique<SocketClient>(socket_, false, useCmdNum_);
    }
    if (pipe2(ctrlPipe_, O_CLOEXEC)) {
        NETNATIVE_LOGE("OpenMonitor: pipe2 failed");
        return NetlinkResult::ERROR;
    }

    thread_ = std::thread(NetlinkNativeListener::ThreadStart, this);

    return NetlinkResult::OK;
}

int32_t NetlinkNativeListener::CloseMonitor()
{
    char ctlp = CTRLPIPE_SHUTDOWN;
    int32_t rc;

    rc = TEMP_FAILURE_RETRY(write(ctrlPipe_[1], &ctlp, 1));
    if (rc != 1) {
        NETNATIVE_LOGE("CloseMonitor: write failed");
        return -1;
    }

    thread_.join();
    close(ctrlPipe_[0]);
    close(ctrlPipe_[1]);
    ctrlPipe_[0] = -1;
    ctrlPipe_[1] = -1;

    if (!socketName_.empty() && socket_ > -1) {
        close(socket_);
        socket_ = -1;
    }

    socketClients_.clear();
    return 0;
}

void NetlinkNativeListener::ThreadStart(NetlinkNativeListener *listener)
{
    listener->RunListener();
}

void NetlinkNativeListener::RunListener()
{
    while (true) {
        std::vector<pollfd> fds;

        std::unique_lock<std::mutex> lock(clientsLock_);
        fds.reserve(PRET_SIZE + socketClients_.size());
        pollfd pfd;
        pfd.fd = ctrlPipe_[0];
        pfd.events = POLLIN;
        fds.push_back(pfd);
        if (listen_) {
            pfd.fd = socket_;
            pfd.events = POLLIN;
            fds.push_back(pfd);
        }
        for (auto &pair : socketClients_) {
            const int32_t fd = pair.second->GetSocket();
            if (fd != pair.first) {
                NETNATIVE_LOGE("fd mismatch: %d != %d", fd, pair.first);
            }
            pfd.fd = fd;
            pfd.events = POLLIN;
            fds.push_back(pfd);
        }
        lock.unlock();
        if (TEMP_FAILURE_RETRY(poll(fds.data(), fds.size(), -1)) < 0) {
            sleep(1);
            continue;
        }

        if (static_cast<uint32_t>(fds[0].revents) & (POLLIN | POLLERR)) {
            char ctlp = CTRLPIPE_SHUTDOWN;
            TEMP_FAILURE_RETRY(read(ctrlPipe_[0], &ctlp, 1));
            if (ctlp == CTRLPIPE_SHUTDOWN) {
                break;
            }
            continue;
        }
        lock.lock();
        if (listen_ && (static_cast<uint32_t>(fds[1].revents) & (POLLIN | POLLERR))) {
            int32_t clientfd = TEMP_FAILURE_RETRY(accept4(socket_, nullptr, nullptr, SOCK_CLOEXEC));
            if (clientfd < 0) {
                sleep(1);
                continue;
            }
            socketClients_[clientfd] = std::make_unique<SocketClient>(clientfd, true, useCmdNum_);
        }
        ProcessMessage(fds);
        lock.unlock();
    }
}

void NetlinkNativeListener::ProcessMessage(const std::vector<pollfd> &fds)
{
    std::vector<SocketClient *> pending;
    const size_t size = fds.size();
    for (size_t i = listen_ ? 2 : 1; i < size; ++i) {
        const pollfd &pfd = fds[i];
        if (static_cast<uint32_t>(pfd.revents) & (POLLIN | POLLERR)) {
            auto it = socketClients_.find(pfd.fd);
            if (it == socketClients_.end()) {
                NETNATIVE_LOGE("fd vanished: %{public}d", pfd.fd);
                continue;
            }
            auto cli = it->second.get();
            pending.push_back(cli);
        }
    }
    for (auto &cl : pending) {
        if (!IsValidData(cl)) {
            RemoveSocket(cl, false);
        }
    }
    pending.clear();
}

bool NetlinkNativeListener::RemoveSocket(SocketClient *clent, bool wakeup)
{
    bool ret = false;
    if (listen_ && (clent != nullptr)) {
        std::unique_lock<std::mutex> lock(clientsLock_);
        ret = (socketClients_.erase(clent->GetSocket()) != 0);
        lock.unlock();
        if (ret && wakeup) {
            char b = CTRLPIPE_WAKEUP;
            TEMP_FAILURE_RETRY(write(ctrlPipe_[1], &b, 1));
        }
    }
    return ret;
}

bool NetlinkNativeListener::IsValidData(const SocketClient *cli)
{
    int32_t socket = cli->GetSocket();
    ssize_t count;
    uid_t uid = -1;

    bool require_group = true;
    if (format_ == NETLINK_FORMAT_BINARY_UNICAST) {
        require_group = false;
    }

    count = TEMP_FAILURE_RETRY(ReceiveUEvent(socket, buffer_, sizeof(buffer_), require_group, &uid));
    if (count < 0) {
        // It will be add a log for error here.
        return false;
    }

    std::shared_ptr<NetlinkMessageDecoder> decoder = std::make_shared<NetlinkMessageDecoder>();

    if (decoder->Decode(buffer_, count, format_)) {
        OnEvent(decoder);
    } else if (format_ != NETLINK_FORMAT_BINARY) {
        NETNATIVE_LOGE("Error decoding NetlinkMessageDecoder fomate = %{public}d", format_);
    }
    return true;
}

ssize_t NetlinkNativeListener::ReceiveUEvent(int32_t socket, void *buffer, size_t length, bool require_group,
                                             uid_t *uid)
{
    iovec iov = {buffer, length};
    sockaddr_nl addr;
    char control[CMSG_SPACE(sizeof(ucred))];
    msghdr hdr;
    hdr.msg_name = &addr;
    hdr.msg_namelen = sizeof(addr);
    hdr.msg_iov = &iov;
    hdr.msg_iovlen = 1;
    hdr.msg_control = control;
    hdr.msg_controllen = sizeof(control);
    hdr.msg_flags = 0;

    ucred *cred = nullptr;

    *uid = -1;
    ssize_t n = TEMP_FAILURE_RETRY(recvmsg(socket, &hdr, 0));
    if (n <= 0) {
        NETNATIVE_LOGI("recvmsg failed (%{public}s)", strerror(errno));
        return n;
    }

    cmsghdr *cmsg = CMSG_FIRSTHDR(&hdr);
    if (cmsg == NULL || cmsg->cmsg_type != SCM_CREDENTIALS) {
        goto out;
    }

    cred = reinterpret_cast<ucred *>(CMSG_DATA(cmsg));
    *uid = cred->uid;

    if (addr.nl_pid != 0) {
        goto out;
    }
    if (require_group && addr.nl_groups == 0) {
        goto out;
    }
    return n;

out:
    bzero(buffer, length);
    errno = EIO;
    return NetlinkResult::ERROR;
}
} // namespace nmd
} // namespace OHOS
