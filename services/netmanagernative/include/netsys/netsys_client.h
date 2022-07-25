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

#ifndef COMMUNICATION_NETMANAGER_BASE_NEW_DNS_NETSYS_CLIENT_H
#define COMMUNICATION_NETMANAGER_BASE_NEW_DNS_NETSYS_CLIENT_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_CONNECT_TIMEOUT 2
#define DEFAULT_POLL_TIMEOUT 1000 // 1 second
#define MAX_POLL_RETRY 3

#if NETSYS_CLIENT_DEBUG
#ifndef NETSYS_CLIENT_PRINT
#define NETSYS_CLIENT_PRINT(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#endif
#else
#define NETSYS_CLIENT_PRINT(fmt, ...)
#endif

static bool MakeNonBlock(int sock)
{
    int flags = fcntl(sock, F_GETFL, 0);
    while (flags == -1 && errno == EINTR) {
        flags = fcntl(sock, F_GETFL, 0);
    }
    if (flags == -1) {
        NETSYS_CLIENT_PRINT("make non block failed %s", strerror(errno));
        return false;
    }
    uint32_t tempFlags = (uint32_t)flags | O_NONBLOCK;
    int ret = fcntl(sock, F_SETFL, tempFlags);
    while (ret == -1 && errno == EINTR) {
        ret = fcntl(sock, F_SETFL, tempFlags);
    }
    if (ret == -1) {
        NETSYS_CLIENT_PRINT("make non block failed %s", strerror(errno));
        return false;
    }
    return true;
}

static int64_t SendWrapper(int fd, char *buf, size_t len)
{
    return send(fd, buf, len, 0);
}

static int64_t RecvWrapper(int fd, char *buf, size_t len)
{
    return recv(fd, buf, len, 0);
}

static bool ProcData(int sock, char *data, size_t size, short event, int64_t (*func)(int fd, char *buf, size_t len))
{
    char *curPos = data;
    size_t leftSize = size;
    nfds_t num = 1;
    struct pollfd fds[1] = {{0}};
    fds[0].fd = sock;
    fds[0].events = event;

    int retry = 0;
    while (leftSize > 0) {
        int ret = poll(fds, num, DEFAULT_POLL_TIMEOUT);
        if (ret == -1) {
            NETSYS_CLIENT_PRINT("poll to proc failed %s", strerror(errno));
            return false;
        }
        if (ret == 0) {
            if (retry < MAX_POLL_RETRY) {
                ++retry;
                continue;
            }
            NETSYS_CLIENT_PRINT("poll to proc timeout");
            return false;
        }

        int64_t length = func(sock, curPos, leftSize);
        if (length < 0) {
            if (errno == EAGAIN && retry < MAX_POLL_RETRY) {
                ++retry;
                continue;
            }
            NETSYS_CLIENT_PRINT("proc failed %s", strerror(errno));
            return false;
        }
        if (length == 0) {
            break;
        }
        curPos += length;
        leftSize -= length;
    }

    if (leftSize != 0) {
        NETSYS_CLIENT_PRINT("proc not complete");
        return false;
    }
    return true;
}

static bool PollSendData(int sock, const char *data, size_t size)
{
    return ProcData(sock, (char *)data, size, POLLOUT, SendWrapper);
}

static bool PollRecvData(int sock, char *data, size_t size)
{
    return ProcData(sock, data, size, POLLIN, RecvWrapper);
}

#ifdef __cplusplus
}
#endif

#endif // COMMUNICATION_NETMANAGER_BASE_NEW_DNS_NETSYS_CLIENT_H
