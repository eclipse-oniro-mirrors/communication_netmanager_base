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

#include "socket_client.h"

#include <cerrno>
#include <cstdio>
#include <sys/socket.h>
#include <unistd.h>

#include "netlink_define.h"
#include "netnative_log_wrapper.h"

namespace OHOS {
namespace nmd {
SocketClient::SocketClient(int32_t socket, bool owned)
{
    Init(socket, owned, false);
}

SocketClient::SocketClient(int32_t socket, bool owned, bool useCmdNum)
{
    Init(socket, owned, useCmdNum);
}

void SocketClient::Init(int32_t socket, bool owned, bool useCmdNum)
{
    NETNATIVE_LOGI("SocketClient: Init SocketClient");
    socket_ = socket;
    socketOwned_ = owned;
    useCmdNum_ = useCmdNum;
    pid_ = -1;
    uid_ = -1;
    gid_ = -1;
    cmdNum_ = 0;

    ucred creds;
    socklen_t size = sizeof(creds);

    int32_t err = getsockopt(socket, SOL_SOCKET, SO_PEERCRED, &creds, &size);
    if (err == 0) {
        pid_ = creds.pid;
        uid_ = creds.uid;
        gid_ = creds.gid;
    } else {
        NETNATIVE_LOGE("SocketClient: getsockopt failed, errno: %d", errno);
    }
}

SocketClient::~SocketClient()
{
    if (socketOwned_) {
        close(socket_);
    }
}
} // namespace nmd
} // namespace OHOS
