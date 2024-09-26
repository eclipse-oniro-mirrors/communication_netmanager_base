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
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include "errorcode_convertor.h"
#include "iptables_wrapper.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"
#include "net_manager_constants.h"
#include "wearable_distributed_net_manager.h"

namespace OHOS {
namespace nmd {
using namespace NetManagerStandard;
const int32_t MAX_CMD_LENGTH = 256;
const int32_t MAX_PORT_ID = 65535;

const std::string CONFIG_KEY_NETFORWARD_COMPONENT_FLAG = "config_wearable_distributed_net_forward";
const std::string TCP_IPTABLES = "tcpiptables";
const std::string TCP_OUTPUT = "tcpoutput";
const std::string UDP_IPTABLES = "udpiptables";
const std::string UDP_OUTPUT = "udpoutput";
const std::string IPTABLES_DELETE_CMDS = "iptablesdeletecmds";

const std::vector<std::string> &WearableDistributedNet::GetTcpIptables()
{
    return tcpIptables_;
}

const std::string &WearableDistributedNet::GetOutputAddTcp()
{
    return tcpOutput_;
}

const std::vector<std::string> &WearableDistributedNet::GetUdpIptables()
{
    return udpIptables_;
}

const std::string &WearableDistributedNet::GetUdpoutput()
{
    return udpOutput_;
}

const std::vector<std::string> &WearableDistributedNet::GetIptablesDeleteCmds()
{
    return iptablesDeleteCmds_;
}

bool WearableDistributedNet::ReadSystemIptablesConfiguration()
{
    const auto &jsonStr = ReadJsonFile(NETWORK_CONFIG_PATH);
    if (jsonStr.length() == 0) {
        NETNATIVE_LOGE("ReadConfigData config file is return empty");
        return false;
    }
    cJSON *json = cJSON_Parse(jsonStr.c_str());
    if (json == nullptr) {
        NETNATIVE_LOGE("Json parse failed");
        return false;
    }
    cJSON *jsonIptables = cJSON_GetObjectItem(json, CONFIG_KEY_NETFORWARD_COMPONENT_FLAG.c_str());
    if (jsonIptables == nullptr) {
        NETNATIVE_LOGE("ReadConfigData not find");
        cJSON_Delete(json);
        return false;
    }
    bool result = ReadIptablesInterfaces(*jsonIptables);
    if (result == false) {
        NETNATIVE_LOGE("Failed to read iptables interfaces");
        cJSON_Delete(json);
        return false;
    }
    cJSON_Delete(json);
    return true;
}

std::string WearableDistributedNet::ReadJsonFile(const std::string &filePath)
{
    std::ifstream infile;
    std::string lineConfigInfo;
    std::string allConfigInfo;
    infile.open(filePath);
    if (!infile.is_open()) {
        NETNATIVE_LOGE("ReadJsonFile filePath failed");
        return allConfigInfo;
    }
    while (getline(infile, lineConfigInfo)) {
        allConfigInfo.append(lineConfigInfo);
    }
    infile.close();
    return allConfigInfo;
}

bool WearableDistributedNet::ParseTcpIptables(const cJSON &json)
{
    cJSON *tcpIptablesObj = cJSON_GetObjectItem(&json, TCP_IPTABLES.c_str());
    for (int32_t i = 0; i < cJSON_GetArraySize(tcpIptablesObj); i++) {
        cJSON *tcpIptablesItem = cJSON_GetArrayItem(tcpIptablesObj, i);
        if (tcpIptablesItem == nullptr) {
            NETNATIVE_LOGE("Invalid item in TCP iptables array");
            return false;
        }
        const auto tcpIptablesValue = tcpIptablesItem->valuestring;
        tcpIptables_.push_back(std::string(tcpIptablesValue));
    }
    return true;
}

bool WearableDistributedNet::ParseTcpOutputRule(const cJSON &json)
{
    cJSON *tcpOutputJsonItem = cJSON_GetObjectItem(&json, TCP_OUTPUT.c_str());
    if (tcpOutputJsonItem == nullptr) {
        NETNATIVE_LOGE("Failed to find tcpOutputJsonItem information");
        return false;
    }
    tcpOutput_ = cJSON_GetStringValue(tcpOutputJsonItem);
    return true;
}

bool WearableDistributedNet::ParseUdpIptables(const cJSON &json)
{
    cJSON *udpIptablesObj = cJSON_GetObjectItem(&json, UDP_IPTABLES.c_str());
    for (int32_t i = 0; i < cJSON_GetArraySize(udpIptablesObj); i++) {
        cJSON *udpIptablesItem = cJSON_GetArrayItem(udpIptablesObj, i);
        if (udpIptablesItem == nullptr) {
            NETNATIVE_LOGE("Invalid item in UDP iptables array");
            return false;
        }
        const auto udpIptablesValue = udpIptablesItem->valuestring;
        udpIptables_.push_back(std::string(udpIptablesValue));
    }
    return true;
}

bool WearableDistributedNet::ParseUdpOutputRule(const cJSON &json)
{
    cJSON *udpOutputItem = cJSON_GetObjectItem(&json, UDP_OUTPUT.c_str());
    if (udpOutputItem == nullptr) {
        NETNATIVE_LOGE("Failed to find udpOutputItem information");
        return false;
    }
    udpOutput_ = cJSON_GetStringValue(udpOutputItem);
    return true;
}

bool WearableDistributedNet::ParseIptablesDeleteCmds(const cJSON &json)
{
    cJSON *iptablesDeleteCmdsObj = cJSON_GetObjectItem(&json, IPTABLES_DELETE_CMDS.c_str());
    for (int32_t i = 0; i < cJSON_GetArraySize(iptablesDeleteCmdsObj); i++) {
        cJSON *iptablesDeleteCmdsItem = cJSON_GetArrayItem(iptablesDeleteCmdsObj, i);
        if (iptablesDeleteCmdsItem == nullptr) {
            NETNATIVE_LOGE("Invalid item in iptables delete commands array");
            return false;
        }
        const auto iptablesDeleteCmdsValue = iptablesDeleteCmdsItem->valuestring;
        iptablesDeleteCmds_.push_back(std::string(iptablesDeleteCmdsValue));
    }
    return true;
}

bool WearableDistributedNet::ReadIptablesInterfaces(const cJSON &json)
{
    auto logErrorAndFail = [&](const char *functionName) {
        NETNATIVE_LOGE("%{public}s failed", functionName);
        return false;
    };
    if (!ParseTcpIptables(json)) {
        return logErrorAndFail("ParseTcpIptables");
    }
    if (!ParseTcpOutputRule(json)) {
        return logErrorAndFail("ParseTcpOutputRule");
    }
    if (!ParseUdpIptables(json)) {
        return logErrorAndFail("ParseUdpIptables");
    }
    if (!ParseUdpOutputRule(json)) {
        return logErrorAndFail("ParseUdpOutputRule");
    }
    if (!ParseIptablesDeleteCmds(json)) {
        return logErrorAndFail("ParseIptablesDeleteCmds");
    }
    return true;
}

void WearableDistributedNet::SetTcpPort(const int32_t tcpPortId)
{
    tcpPort_ = tcpPortId;
}

int32_t WearableDistributedNet::GetTcpPort()
{
    return tcpPort_;
}

int32_t WearableDistributedNet::ExecuteIptablesCommands(const std::vector<std::string> &commands)
{
    if (commands.empty()) {
        NETNATIVE_LOGE("Invalid commands array");
        return NETMANAGER_ERROR;
    }
    for (const auto &command : commands) {
        if (command.length() > MAX_CMD_LENGTH) {
            NETNATIVE_LOGE("Invalid command found at index");
            return NETMANAGER_ERROR;
        }
        std::string response =
            IptablesWrapper::GetInstance()->RunCommandForRes(OHOS::nmd::IpType::IPTYPE_IPV4, command);
        return response.empty() ? NETMANAGER_ERROR : NETMANAGER_SUCCESS;
    }
    return NETMANAGER_SUCCESS;
}

int32_t WearableDistributedNet::EnableWearableDistributedNetForward(const int32_t tcpPortId, const int32_t udpPortId)
{
    if (!ReadSystemIptablesConfiguration()) {
        NETNATIVE_LOGE("Failed to read system iptables configuration");
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    if (tcpPortId <= 0 || tcpPortId > MAX_PORT_ID) {
        NETNATIVE_LOGE("Invalid TCP port ID");
        return NETMANAGER_WEARABLE_DISTRIBUTED_NET_ERR_INVALID_TCP_PORT_ID;
    }
    if (udpPortId <= 0 || udpPortId > MAX_PORT_ID) {
        NETNATIVE_LOGE("Invalid UDP port ID");
        return NETMANAGER_WEARABLE_DISTRIBUTED_NET_ERR_INVALID_UDP_PORT_ID;
    }
    int32_t ret = EstablishTcpIpRules();
    if (ret != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("Failed to establish TCP IP rules for network distribution");
        return ret;
    }
    ret = EstablishUdpIpRules(udpPortId);
    if (ret != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("Failed to establish UDP IP rules for network distribution");
        return ret;
    }

    return NETMANAGER_SUCCESS;
}

std::string WearableDistributedNet::GenerateRule(const std::string &inputRules, const int32_t portId)
{
    if (inputRules.empty()) {
        NETNATIVE_LOGE("Input rules are null");
        return "";
    }
    if (inputRules.length() > MAX_CMD_LENGTH) {
        NETNATIVE_LOGE("Input rules are invalid");
        return "";
    }
    char res[MAX_CMD_LENGTH] = {0};
    if (sprintf_s(res, MAX_CMD_LENGTH, inputRules.c_str(), portId) == -1) {
        return "";
    }
    return std::string(res);
}

int32_t WearableDistributedNet::ApplyRule(const RULES_TYPE type, const int32_t portId)
{
    std::string resultRules;
    switch (type) {
        case TCP_ADD_RULE:
            resultRules = GenerateRule(TCP_ADD16, portId);
            break;
        case UDP_ADD_RULE:
            resultRules = GenerateRule(UDP_ADD16, portId);
            break;
        case INPUT_ADD_RULE:
            resultRules = GenerateRule(INPUT_ADD, portId);
            break;
        case INPUT_DEL_RULE:
            resultRules = GenerateRule(INPUT_DEL, portId);
            break;
        default:
            NETNATIVE_LOGE("Invalid rule type");
            return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    if (resultRules.empty()) {
        NETNATIVE_LOGE("Failed to generate rule");
        return NETMANAGER_ERR_INVALID_PARAMETER;
    }
    std::string response =
        IptablesWrapper::GetInstance()->RunCommandForRes(OHOS::nmd::IpType::IPTYPE_IPV4, resultRules);
    return response.empty() ? NETMANAGER_ERROR : NETMANAGER_SUCCESS;
}

int32_t WearableDistributedNet::EstablishTcpIpRules()
{
    NETNATIVE_LOGI("Establishing TCP IP rules for network distribution");
    if (ExecuteIptablesCommands(GetTcpIptables()) != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("Failed to execute TCP iptables commands");
        return NETMANAGER_ERROR;
    }
    if (ApplyRule(TCP_ADD_RULE, GetTcpPort()) != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("Failed to apply TCP add rule");
        return NETMANAGER_ERROR;
    }
    std::string response =
        IptablesWrapper::GetInstance()->RunCommandForRes(OHOS::nmd::IpType::IPTYPE_IPV4, GetOutputAddTcp());
    return response.empty() ? NETMANAGER_ERROR : NETMANAGER_SUCCESS;
}

int32_t WearableDistributedNet::EstablishUdpIpRules(const int32_t udpPortId)
{
    NETNATIVE_LOGI("Establishing UDP IP rules for network distribution");
    if (ExecuteIptablesCommands(GetUdpIptables()) != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("Failed to execute UDP iptables commands");
        return NETMANAGER_ERROR;
    }
    if (ApplyRule(UDP_ADD_RULE, udpPortId) != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("Failed to apply UDP add rule");
        return NETMANAGER_ERROR;
    }
    std::string response =
        IptablesWrapper::GetInstance()->RunCommandForRes(OHOS::nmd::IpType::IPTYPE_IPV4, GetUdpoutput());
    return response.empty() ? NETMANAGER_ERROR : NETMANAGER_SUCCESS;
}

int32_t WearableDistributedNet::DisableWearableDistributedNetForward()
{
    NETNATIVE_LOGI("Disabling wearable distributed net forward");
    if (ExecuteIptablesCommands(GetIptablesDeleteCmds()) != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("Failed to execute iptables delete commands");
        return NETMANAGER_ERROR;
    }
    if (ApplyRule(INPUT_DEL_RULE, GetTcpPort()) != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("Failed to apply input delete rule");
        return NETMANAGER_ERROR;
    }
    return NETMANAGER_SUCCESS;
}
} // namespace nmd
} // namespace OHOS
