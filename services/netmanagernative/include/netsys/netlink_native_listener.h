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

#ifndef SOCKET_LISTENER_H
#define SOCKET_LISTENER_H

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <poll.h>

#include "netlink_message_decoder.h"
#include "socket_client.h"

namespace OHOS {
namespace nmd {
class NetlinkNativeListener {
public:
    NetlinkNativeListener() = delete;
    NetlinkNativeListener(int32_t socketFd, bool listen, int32_t format);

    virtual ~NetlinkNativeListener();
    int32_t OpenMonitor();
    int32_t OpenMonitor(int32_t backlog);
    int32_t CloseMonitor();

    bool RemoveSocket(SocketClient *client)
    {
        return RemoveSocket(client, true);
    }

protected:
    virtual bool IsValidData(const SocketClient *cli);
    virtual void OnEvent(std::shared_ptr<NetlinkMessageDecoder> message) = 0;

private:
    bool listen_ = false;
    std::string socketName_;
    int32_t socket_ = -1;
    std::unordered_map<int32_t, std::unique_ptr<SocketClient>> socketClients_;
    std::mutex clientsLock_;
    int32_t ctrlPipe_[2] = {0};
    std::thread thread_;
    bool useCmdNum_ = false;

    char buffer_[NetlinkDefine::BUFFER_SIZE] __attribute__((aligned(4))) = {0};
    int32_t format_ = NetlinkDefine::NETLINK_FORMAT_ASCII;

    static constexpr int32_t CTRLPIPE_SHUTDOWN = 0;
    static constexpr int32_t CTRLPIPE_WAKEUP = 1;
    static constexpr int32_t BACK_LOG = 4;
    static void ThreadStart(NetlinkNativeListener *listener);

    bool RemoveSocket(SocketClient *client, bool wakeup);
    void RunListener();
    void Init(const std::string &socketName, int32_t socketFd, bool listen, bool useCmdNum, int32_t format);
    ssize_t ReceiveUEvent(int32_t socket, void *buffer, size_t length, bool require_group, uid_t *uid);
    void ProcessMessage(const std::vector<pollfd> &fds);
};
} // namespace nmd
} // namespace OHOS
#endif // !SOCKET_LISTENER_H
