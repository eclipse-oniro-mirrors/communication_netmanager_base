/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef SOCKET_CLIENT_H__
#define SOCKET_CLIENT_H__

#include <pthread.h>
#include <sys/types.h>
#include <sys/uio.h>

namespace OHOS {
namespace nmd {
class SocketClient {
public:
    SocketClient(int32_t sock, bool owned);
    SocketClient(int32_t sock, bool owned, bool useCmdNum);
    virtual ~SocketClient();

    inline int32_t GetSocket() const
    {
        return socket_;
    }
    inline pid_t GetPid() const
    {
        return pid_;
    }
    inline uid_t GetUid() const
    {
        return uid_;
    }
    inline gid_t GetGid() const
    {
        return gid_;
    }

    inline int32_t GetCmdNum() const
    {
        return cmdNum_;
    }

private:
    int32_t socket_ = -1;
    bool socketOwned_ = false;
    pid_t pid_ = -1;
    uid_t uid_ = -1;
    gid_t gid_ = -1;
    int32_t cmdNum_ = -1;
    bool useCmdNum_ = false;

    void Init(int32_t socket, bool owned, bool useCmdNum);
};
} // namespace nmd
} // namespace OHOS
#endif // SOCKET_CLIENT_H__
