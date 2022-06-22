/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef INCLUDE_NETSYS_NETWORK_H__
#define INCLUDE_NETSYS_NETWORK_H__

#include <set>
#include <string>

namespace OHOS {
namespace nmd {
class NetsysNetwork {
public:
    NetsysNetwork();
    virtual ~NetsysNetwork();

    /**
     * @brief Get network type
     *
     * @return Returns network type, it could be either
     */
    virtual std::string GetNetworkType() const = 0;

    /**
     * @brief Add an interface to a network
     *
     * @param interafceName The name of the interface to add
     *
     * @return Returns 0, successfully add an interface to a network, otherwise it will fail
     */
    virtual int32_t AddInterface(std::string &) = 0;

    /**
     * @brief Remove an interface from a network
     *
     * @param interafceName The name of the interface
     *
     * @return Returns 0, successfully remove an interface from a network, otherwise it will fail
     */
    virtual int32_t RemoveInterface(std::string &) = 0;

    /**
     * @brief Clear all interfaces
     *
     * @return Returns 0, successfully clear all interfaces, otherwise it will fail
     */
    int32_t ClearInterfaces();

    /**
     * @brief Clear all interfaces
     *
     * @param interafceName The name of the interface
     *
     * @return Returns true exist interface, false otherwise
     */
    bool ExistInterface(std::string &interfaceName);

    /**
     * @brief Judge network type whether or not physical
     *
     * @return Returns true physical, false otherwise
     */
    virtual bool IsPhysical()
    {
        return false;
    }

    /**
     * @brief Get all interfaces
     *
     * @return Returns interfaces_
     */
    const std::set<std::string> &GetAllInterface() const
    {
        return interfaces_;
    }

    /**
     * @brief Get netId
     *
     * @return Returns netId_
     */
    uint16_t GetNetId()
    {
        return netId_;
    }

protected:
    explicit NetsysNetwork(uint16_t netId);
    uint16_t netId_;
    std::set<std::string> interfaces_;
};
} // namespace nmd
} // namespace OHOS
#endif // INCLUDE_NETSYS_NETWORK_H__