/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef NET_CONN_NETWORK_H
#define NET_CONN_NETWORK_H

#include "inet_addr.h"
#include "route.h"
#include "net_conn_async.h"
#include "socket_factory.h"

namespace OHOS {
namespace NetManagerStandard {
class Network : public SocketFactory, public virtual RefBase {
public:
    /**
     * Construct a new Network
     *
     */
    Network();

    /**
     * Destroy the Network
     *
     */
    virtual ~Network();

    /**
     * Get the network id
     *
     * @return uint32_t network id
     */
    uint32_t GetId() const;

    /**
     * Create a Physical network
     *
     * @return int32_t Return 0 if success create, failed fo others
     */
    int32_t CreatePhy();

    /**
     * Destroy created physical network
     *
     * @return int32_t Return 0 if success destroyed, failed fo others
     */
    int32_t DestroyPhy();

    /**
     * Set the Interface name of the network
     *
     * @param ifaceName Interface name to set
     * @return int32_t Return 0 if success set, failed fo others
     */
    int32_t SetIfaceName(const std::string &ifaceName);

    /**
     * Set the domain of the network
     *
     * @param domain Domain to set
     * @return int32_t Return 0 if success set, failed fo others
     */
    int32_t SetDomain(const std::string &domain);

    /**
     * Set the current interface net addrs
     *
     * @param netAddrList Net addrs to set
     * @return int32_t Return 0 if success set, failed fo others
     */
    int32_t SetNetAddrList(const std::list<INetAddr> &netAddrList);

    /**
     * Set the current interface route list
     *
     * @param routeList Route list to set
     * @return int32_t Return 0 if success set, failed fo others
     */
    int32_t SetRouteList(const std::list<Route> &routeList);

    /**
     * Set the current interface dns list
     *
     * @param dnsAddrList dns server addrs list
     * @return int32_t Return 0 if success set, failed fo others
     */
    int32_t SetDnsList(const std::list<INetAddr> &dnsAddrList);

    /**
     * Set the current interface MTU
     *
     * @param mtu
     * @return int32_t Return 0 if success set, failed fo others
     */
    int32_t SetMtu(const uint16_t &mtu);

    /**
     * Set the current interface tcp buffer sizes
     *
     * @param tcpBufferSizes
     * @return int32_t Return 0 if success set, failed fo others
     */
    int32_t SetTcpBufferSizes(const std::string &tcpBufferSizes);

    /**
     * Set the current interface as default network
     *
     * @return int32_t Return 0 if success set, failed fo others
     */
    int32_t SetDefault();

    /**
     * Override from SocketFactory
     */
    int32_t CreateSocket(int32_t domain, int32_t type, int32_t protocol) override;

    /**
     * Override from SocketFactory
     */
    void DestroySocket(int32_t sockFd) override;

private:
    void ClearData();

private:
    uint32_t id_{0};
    bool phyCreated_{false};
    std::string ifaceName_;
    std::string domain_;
    std::list<INetAddr> netAddrList_;
    std::list<INetAddr> dnsList_;
    std::list<Route> routeList_;
    uint16_t mtu_{0};
    std::string tcpBufferSizes_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_NETWORK_H
