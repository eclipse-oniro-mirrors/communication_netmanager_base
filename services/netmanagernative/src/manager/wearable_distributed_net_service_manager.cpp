/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <cstring>
#include <cstdio>
#include <sstream>
#include <string>
#include <iostream>
#include "wearable_distributed_net_service_manager.h"
#include "net_manager_constants.h"
#include "iptables_wrapper.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"
#include "errorcode_convertor.h"

namespace OHOS {
namespace nmd {
using namespace NetManagerStandard;
constexpr int32_t MAX_CMD_LENGTH = 256;

const char* g_tcpIptables[] = {
    "-w -t nat -N DISTRIBUTED_NET_TCP",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 0.0.0.0/8 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 10.0.0.0/8 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 100.64.0.0/10 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 127.0.0.0/8 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 169.254.0.0/16 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 172.16.0.0/12 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 192.0.0.0/29 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 192.0.2.0/24 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 192.168.0.0/16 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 198.18.0.0/15 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 198.51.100.0/24 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 203.0.113.0/24 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 224.0.0.0/4 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 240.0.0.0/4 -j RETURN",
    "-w -o lo -t nat -A DISTRIBUTED_NET_TCP -d 255.255.255.255/32 -j RETURN"
};

const char g_outputAddTcp[] = 
    "-w -o lo -t nat -A OUTPUT -p tcp -j DISTRIBUTED_NET_TCP";

const char* g_udpIptables[] = {
    "-w -t mangle -N DISTRIBUTED_NET_UDP",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 0.0.0.0/8 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 10.0.0.0/8 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 100.64.0.0/10 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 127.0.0.0/8 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 169.254.0.0/16 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 172.16.0.0/12 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 192.0.0.0/29 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 192.0.2.0/24 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 192.168.0.0/16 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 198.18.0.0/15 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 198.51.100.0/24 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 203.0.113.0/24 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 224.0.0.0/4 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 240.0.0.0/4 -j RETURN",
    "-w -i lo -t mangle -A DISTRIBUTED_NET_UDP -d 255.255.255.255/32 -j RETURN"
};

const char g_preroutingAddUdp[] =
    "-w -i lo -t mangle -A PREROUTING -p udp -j DISTRIBUTED_NET_UDP";

const char* g_iptablesDeleteCmds[] = {
    "-w -i lo -t mangle -D PREROUTING -p udp -j DISTRIBUTED_NET_UDP",
    "-w -t mangle -F DISTRIBUTED_NET_UDP",
    "-w -t mangle -X DISTRIBUTED_NET_UDP",
    "-w -o lo -t nat -D OUTPUT -p tcp -j DISTRIBUTED_NET_TCP",
    "-w -t nat -F DISTRIBUTED_NET_TCP",
    "-w -t nat -X DISTRIBUTED_NET_TCP"
};

int32_t WearbleDistributedNet::GetTcpPort()
{
    return tcpPort_;
}

int32_t WearbleDistributedNet::ExecuteIptablesCommands(const char** commands)
{
    for (int i = 0; commands[i] != nullptr; ++i) {
        std::string response = IptablesWrapper::GetInstance()->RunCommandForRes(OHOS::nmd::IpType::IPTYPE_IPV4, commands[i]);
        if (!response.empty()) {
            return NETMANAGER_ERROR;
        }
    }
    return NETMANAGER_SUCCESS;
}

int32_t WearbleDistributedNet::EnableWearbleDistributedNetForward(const int32_t tcpPortId, const int32_t udpPortId)
{
    NETNATIVE_LOGI("WearbleDistributedNet tcpPortId = %{public}d udpPortId = %{public}d", tcpPortId, udpPortId);
    int32_t ret = AddTcpIpRules();  
    if (ret != NETMANAGER_SUCCESS) {  
        return ret;  
    }  
    ret = AddUdpIpRules();  
    if (ret != NETMANAGER_SUCCESS) {  
        return ret;  
    }

    return NETMANAGER_SUCCESS;  
}

std::string WearbleDistributedNet::GenerateRule(const char *inputRules, const int32_t portId)
{
    NETNATIVE_LOGI("WearbleDistributedNet GenerateRule portId = %{public}d", portId);
    char res[MAX_CMD_LENGTH] = {0};
    if (snprintf(res, MAX_CMD_LENGTH, inputRules, portId) >= MAX_CMD_LENGTH) {
        return "";
    }
  
    NETNATIVE_LOGI("WearbleDistributedNet GenerateRule Out rule:%{public}s", res);
    return std::string(res);
}

int32_t WearbleDistributedNet::ApplyRule(const RULES_TYPE type, const int32_t portId)
{
 
    std::string resultRules = GenerateRule(GetRuleTemplate(type), portId);  
    if (resultRules.empty()) {  
        return NETMANAGER_ERR_INVALID_PARAMETER;  
    }  
  
    std::string response = IptablesWrapper::GetInstance()->RunCommandForRes(OHOS::nmd::IpType::IPTYPE_IPV4, resultRules);  
    return response.empty() ? NETMANAGER_SUCCESS : NETMANAGER_ERROR; 
}

const char* WearbleDistributedNet::GetRuleTemplate(const RULES_TYPE type)  
{  
    switch (type) {  
        case TCP_ADD_RULE:  
            return TCP_ADD16;  
        case UDP_ADD_RULE:  
            return UDP_ADD16;  
        case INPUT_ADD_RULE:  
            return INPUT_ADD;  
        case INPUT_DEL_RULE:  
            return INPUT_DEL;  
        default:  
            return nullptr;  
    }  
}

int32_t WearbleDistributedNet::EstablishTcpIpRulesForNetworkDistribution()
{
    if (ExecuteIptablesCommands(g_tcpIptables) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }
    if (ApplyRule(TCP_ADD_RULE, GetTcpPort()) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }
    std::string response = IptablesWrapper::GetInstance()->RunCommandForRes(OHOS::nmd::IpType::IPTYPE_IPV4, g_outputAddTcp);
    return response.empty() ? NETMANAGER_SUCCESS : NETMANAGER_ERROR;
}

int32_t WearbleDistributedNet::EstablishUdpIpRulesForNetworkDistribution()
{
    if (ExecuteIptablesCommands(g_udpIptables) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }
    if (ApplyRule(UDP_ADD_RULE, 0) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }
    std::string response = IptablesWrapper::GetInstance()->RunCommandForRes(OHOS::nmd::IpType::IPTYPE_IPV4, g_preroutingAddUdp);
    if (!response.empty()) {
        return NETMANAGER_ERROR;
    }
    return NETMANAGER_SUCCESS;
}

int32_t WearbleDistributedNet::DisableWearbleDistributedNetForward()
{
    if (ExecuteIptablesCommands(g_iptablesDeleteCmds) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }
    if (ApplyRule(INPUT_DEL_RULE, GetTcpPort()) != NETMANAGER_SUCCESS) {
        return NETMANAGER_ERROR;
    }
    return NETMANAGER_SUCCESS;
}
} // namespace nmd
} // namespace OHOS
