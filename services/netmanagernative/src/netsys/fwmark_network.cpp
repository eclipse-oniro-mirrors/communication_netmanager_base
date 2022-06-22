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

#include "fwmark_network.h"

#include <cerrno>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

#include "fwmark.h"
#include "fwmark_command.h"
#include "netnative_log_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace nmd {
static constexpr const uint16_t NETID_UNSET = 0;
static constexpr const int32_t NO_ERROR_CODE = 0;
static constexpr const int32_t ERROR_CODE_RECVMSG_FAILED = -1;
static constexpr const int32_t ERROR_CODE_SOCKETFD_INVALID = -2;
static constexpr const int32_t ERROR_CODE_WRITE_FAILED = -3;
static constexpr const int32_t ERROR_CODE_GETSOCKOPT_FAILED = -4;
static constexpr const int32_t ERROR_CODE_SETSOCKOPT_FAILED = -5;
static constexpr const int32_t ERROR_CODE_SET_MARK = -6;
static constexpr const int32_t MAX_CONCURRENT_CONNECTION_REQUESTS = 10;

void CloseSocket(int32_t *socket, int32_t ret, int32_t errorCode)
{
    switch (errorCode) {
        case ERROR_CODE_RECVMSG_FAILED:
            NETNATIVE_LOGE(
                "recvmsg failed, clientSockfd:%{public}d, ret:%{public}d, errno: %{public}d", *socket, ret, errno);
            break;
        case ERROR_CODE_SOCKETFD_INVALID:
            NETNATIVE_LOGE("socketFd invalid:%{public}d, ret:%{public}d, errno: %{public}d", *socket, ret, errno);
            break;
        case ERROR_CODE_WRITE_FAILED:
            NETNATIVE_LOGE(
                "wirte failed, clientSockfd:%{public}d, ret:%{public}d, errno: %{public}d", *socket, ret, errno);
            break;
        case ERROR_CODE_GETSOCKOPT_FAILED:
            NETNATIVE_LOGE(
                "getsockopt failed, socketFd:%{public}d, ret:%{public}d, errno: %{public}d", *socket, ret, errno);
            break;
        case ERROR_CODE_SETSOCKOPT_FAILED:
            NETNATIVE_LOGE(
                "setsockopt failed socketFd:%{public}d, ret:%{public}d, errno: %{public}d", *socket, ret, errno);
            break;
        case ERROR_CODE_SET_MARK:
            NETNATIVE_LOGE(
                "SetMark failed, clientSockfd:%{public}d, ret:%{public}d, errno: %{public}d", *socket, ret, errno);
            break;
        default:
            NETNATIVE_LOGI("NO_ERROR_CODE CloseSocket socket:%{public}d, ret:%{public}d", *socket, ret);
            break;
    }
    close(*socket);
    *socket = -1;
}

int32_t SetMark(int32_t *socketFd, FwmarkCommand *command)
{
    Fwmark fwmark;
    socklen_t fwmarkLen = sizeof(fwmark.intValue);
    int32_t ret = getsockopt(*socketFd, SOL_SOCKET, SO_MARK, &fwmark.intValue, &fwmarkLen);
    if (ret != 0) {
        CloseSocket(socketFd, ret, ERROR_CODE_GETSOCKOPT_FAILED);
        return ret;
    }
    fwmark.netId = command->netId;
    NETNATIVE_LOGI("FwmarkNetwork: SetMark netId: %{public}d, socketFd:%{public}d", command->netId, *socketFd);
    if (command->netId == NETID_UNSET) {
        fwmark.explicitlySelected = false;
        fwmark.protectedFromVpn = false;
        fwmark.permission = PERMISSION_NONE;
    } else {
        fwmark.explicitlySelected = true;
    }
    ret = setsockopt(*socketFd, SOL_SOCKET, SO_MARK, &fwmark.intValue, sizeof(fwmark.intValue));
    if (ret != 0) {
        CloseSocket(socketFd, ret, ERROR_CODE_SETSOCKOPT_FAILED);
        return ret;
    }
    CloseSocket(socketFd, ret, NO_ERROR_CODE);
    return ret;
}

void SendMessage(int32_t *serverSockfd)
{
    int32_t clientSockfd;
    struct sockaddr_un clientAddr;
    socklen_t len = sizeof(clientAddr);
    while (true) {
        clientSockfd = accept(*serverSockfd, reinterpret_cast<struct sockaddr *>(&clientAddr), &len);
        NETNATIVE_LOGI("FwmarkNetwork: clientSockfd: %{public}d", clientSockfd);
        FwmarkCommand command;
        iovec iov;
        iov.iov_base = &command;
        iov.iov_len = sizeof(command);
        int32_t socketFd = -1;
        msghdr message;
        (void)memset_s(&message, sizeof(message), 0, sizeof(message));
        message.msg_iov = &iov;
        message.msg_iovlen = 1;
        union {
            cmsghdr cmh;
            char cmsg[CMSG_SPACE(sizeof(socketFd))];
        } cmsgu;
        (void)memset_s(cmsgu.cmsg, sizeof(cmsgu.cmsg), 0, sizeof(cmsgu.cmsg));
        message.msg_control = cmsgu.cmsg;
        message.msg_controllen = sizeof(cmsgu.cmsg);
        int32_t ret = recvmsg(clientSockfd, &message, 0);
        if (ret < 0) {
            CloseSocket(&clientSockfd, ret, ERROR_CODE_RECVMSG_FAILED);
            continue;
        }
        cmsghdr *const cmsgh = CMSG_FIRSTHDR(&message);
        if (cmsgh && cmsgh->cmsg_level == SOL_SOCKET && cmsgh->cmsg_type == SCM_RIGHTS &&
            cmsgh->cmsg_len == CMSG_LEN(sizeof(socketFd))) {
            (void)memcpy_s(&socketFd, sizeof(socketFd), CMSG_DATA(cmsgh), sizeof(socketFd));
        }
        if (socketFd < 0) {
            CloseSocket(&clientSockfd, ret, ERROR_CODE_SOCKETFD_INVALID);
            continue;
        }
        if ((ret = SetMark(&socketFd, &command)) != 0) {
            CloseSocket(&clientSockfd, ret, ERROR_CODE_SET_MARK);
            continue;
        }
        if ((ret = write(clientSockfd, &ret, sizeof(ret))) < 0) {
            CloseSocket(&clientSockfd, ret, ERROR_CODE_WRITE_FAILED);
            continue;
        }
        CloseSocket(&clientSockfd, ret, NO_ERROR_CODE);
    }
}

void StartListener()
{
    unlink(FWMARK_SERVER_PATH.sun_path);
    int32_t serverSockfd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    struct sockaddr_un serverAddr = {0};
    serverAddr.sun_family = AF_UNIX;
    (void)strcpy_s(serverAddr.sun_path, sizeof(serverAddr.sun_path), FWMARK_SERVER_PATH.sun_path);
    NETNATIVE_LOGI("FwmarkNetwork: address.sun_path: %{public}s", serverAddr.sun_path);

    int32_t result = bind(serverSockfd, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr));
    if (result < 0) {
        NETNATIVE_LOGE("FwmarkNetwork: bind failed result %{public}d, errno: %{public}d", result, errno);
        close(serverSockfd);
        serverSockfd = -1;
    }

    result = listen(serverSockfd, MAX_CONCURRENT_CONNECTION_REQUESTS);
    if (result < 0) {
        NETNATIVE_LOGE("FwmarkNetwork: listen failed result %{public}d, errno: %{public}d", result, errno);
        close(serverSockfd);
        serverSockfd = -1;
    }
    SendMessage(&serverSockfd);
    close(serverSockfd);
    serverSockfd = -1;
}

FwmarkNetwork::FwmarkNetwork()
{
    ListenerClient();
}

FwmarkNetwork::~FwmarkNetwork() {}

void FwmarkNetwork::ListenerClient()
{
    std::thread startListener(StartListener);
    startListener.detach();
    NETNATIVE_LOGI("FwmarkNetwork: StartListener");
}
} // namespace nmd
} // namespace OHOS