/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef ROUTEUTILS_H
#define ROUTEUTILS_H

#include <list>
#include <string>
#include "route.h"
#include "net_link_info.h"

namespace OHOS {
namespace NetManagerStandard {
enum routeOperateType {
    ROUTE_ADD,
    ROUTE_REMOVE
};

enum routeLocalNetId {
    LOCAL_NET_ID = 99
};

const std::string NEXTHOP_UNREACHABLE = "unreachable";
const std::string NEXTHOP_THROW = "throw";

class RouteUtils {
public:
    RouteUtils();
    ~RouteUtils();

    /**
     * @brief add route info to table by default netId
     *
     * @param iface interface name
     * @param routes the list of route
     *
     * @return 0, add route success, otherwise it will fail
     */
    static int32_t AddRoutesToLocal(const std::string &iface, const std::list<Route> &routes);

    /**
     * @brief remove route info from table by default netId
     *
     * @param routes the list of route
     *
     * @return 0, remove route success, otherwise it will fail
     */
    static int32_t RemoveRoutesFromLocal(const std::list<Route> &routes);

    /**
     * @brief add route info to table by given netId
     *
     * @param netId
     * @param route route info will add to route table
     *
     * @return 0, add route success, otherwise it will fail
     */
    static int32_t AddRoute(int32_t netId, const Route &route);

    /**
     * @brief remove route info from table by given netId
     *
     * @param netId
     * @param route route info will remove from route table
     *
     * @return 0, remove route success, otherwise it will fail
     */
    static int32_t RemoveRoute(int32_t netId, const Route &route);

    /**
     * @brief update route from oldnl to newnl
     *
     * @param netId
     * @param newnl route info will add or update to route table
     * @oldnl oldnl route info will remove from route table
     *
     * @return 1, route changed between oldnl and newnl
     */
    static int32_t UpdateRoutes(int32_t netId, const NetLinkInfo &newnl, const NetLinkInfo &oldnl);

private:
    static int32_t ModifyRoute(routeOperateType op, int32_t netId, const Route &route);
    static void ToPrefixString(const std::string &src, int32_t prefixLen, std::string &dest);
    static std::string MaskAddress(const std::string &addr, int32_t prefixLen);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif  // ROUTEUTILS_H