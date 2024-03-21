/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "net_policy_file.h"

#include <fcntl.h>
#include <string>

#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"
#include "net_policy_file_event_handler.h"
#include "net_policy_inner_define.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
bool CheckFilePath(const std::string &fileName, std::string &realPath)
{
    char tmpPath[PATH_MAX] = {0};
    if (!realpath(fileName.c_str(), tmpPath)) {
        NETMGR_LOG_E("file name is illegal");
        return false;
    }
    if (strcmp(tmpPath, POLICY_FILE_NAME) != 0) {
        NETMGR_LOG_E("file path is illegal");
        return false;
    }
    realPath = tmpPath;
    return true;
}
} // namespace
constexpr const char *NET_POLICY_WORK_THREAD = "NET_POLICY_FILE_WORK_THREAD";

NetPolicyFile::NetPolicyFile()
{
    InitPolicy();
}

NetPolicyFile::~NetPolicyFile() = default;

bool NetPolicyFile::ReadFile(const std::string &fileName)
{
    NETMGR_LOG_D("read [%{public}s] from disk.", fileName.c_str());
    struct stat st;
    if (stat(fileName.c_str(), &st) != 0) {
        NETMGR_LOG_E("stat file fail");
        return false;
    }

    std::string realPath;
    if (!CheckFilePath(fileName, realPath)) {
        NETMGR_LOG_E("file does not exist");
        return false;
    }

    std::fstream file(realPath.c_str(), std::fstream::in);
    if (!file.is_open()) {
        NETMGR_LOG_E("file open fail");
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContent = buffer.str();
    file.close();
    return Json2Obj(fileContent, netPolicy_);
}

bool NetPolicyFile::ReadFile()
{
    return ReadFile(POLICY_FILE_NAME) || ReadFile(POLICY_FILE_BAK_NAME);
}

bool NetPolicyFile::WriteFile()
{
    auto data = std::make_shared<PolicyFileEvent>();
    Obj2Json(netPolicy_, data->json);
    auto event = AppExecFwk::InnerEvent::Get(NetPolicyFileEventHandler::MSG_POLICY_FILE_WRITE, data);
    auto handler = GetHandler();
    if (!handler) {
        NETMGR_LOG_E("NetPolicyFileEventHandler not existed");
        return false;
    }
    handler->SendWriteEvent(event);
    return true;
}

const std::vector<UidPolicy> &NetPolicyFile::ReadUidPolicies()
{
    return netPolicy_.uidPolicies;
}

void NetPolicyFile::ParseUidPolicy(const Json::Value &root, const cJSON* const object, NetPolicy &netPolicy)
{
    cJSON *netUidPolicy = cJSON_GetObjectItem(object, CONFIG_UID_POLICY);
    if (netUidPolicy != nullptr) {
        uint32_t sized = cJSON_GetArraySize(netUidPolicy);
        UidPolicy uidPolicyTest;
        for (uint32_t j = 0; j < sized; j++) {
            cJSON *uidPolicyItem = cJSON_GetArrayItem(netUidPolicy, j);
            if (uidPolicyItem == nullptr || !cJSON_IsObject(uidPolicyItem)) {
                NETMGR_LOG_E("Json2Obj uidPolicyItem is null or not object");
                continue;
            }
            cJSON *uid = cJSON_GetObjectItem(uidPolicyItem, CONFIG_UID);
            if (uid == nullptr || !cJSON_IsString(uid)) {
                NETMGR_LOG_E("Json2Obj uid is null or not string");
                continue;
            }
            cJSON *policy = cJSON_GetObjectItem(uidPolicyItem, CONFIG_POLICY);
            if (policy == nullptr || !cJSON_IsString(policy)) {
                NETMGR_LOG_E("Json2Obj policy is null or not string");
                continue;
            }
            uidPolicyTest.uid = cJSON_GetStringValue(uid);
            NETMGR_LOG_E("Json2Obj uid: %{public}s", uidPolicyTest.uid.c_str());
            uidPolicyTest.policy = cJSON_GetStringValue(policy);
            NETMGR_LOG_E("Json2Obj policy: %{public}s", uidPolicyTest.policy.c_str());
        }
    }

    const Json::Value &arrayUidPolicy = root[CONFIG_UID_POLICY];
    uint32_t size = arrayUidPolicy.size();
    UidPolicy uidPolicy;
    for (uint32_t i = 0; i < size; i++) {
        uidPolicy.uid = arrayUidPolicy[i][CONFIG_UID].asString();
        NETMGR_LOG_E("ParseUidPolicy uid: %{public}s", uidPolicy.uid.c_str());
        uidPolicy.policy = arrayUidPolicy[i][CONFIG_POLICY].asString();
        NETMGR_LOG_E("ParseUidPolicy policy: %{public}s", uidPolicy.policy.c_str());
        netPolicy.uidPolicies.push_back(uidPolicy);
    }
}

void NetPolicyFile::ParseBackgroundPolicy(const Json::Value &root, const cJSON* const object, NetPolicy &netPolicy)
{
    cJSON *netBackgroundPolicy = cJSON_GetObjectItem(object, CONFIG_BACKGROUND_POLICY);
    if (netBackgroundPolicy != nullptr) {
        cJSON *status = cJSON_GetObjectItem(netBackgroundPolicy, CONFIG_BACKGROUND_POLICY_STATUS);
        netPolicy.backgroundPolicyStatus = cJSON_GetStringValue(status);
        NETMGR_LOG_E("Json2Obj backgroundPolicyStatus: %{public}s", netPolicy.backgroundPolicyStatus.c_str());
    }

    const Json::Value &mapBackgroundPolicy = root[CONFIG_BACKGROUND_POLICY];
    netPolicy.backgroundPolicyStatus = mapBackgroundPolicy[CONFIG_BACKGROUND_POLICY_STATUS].asString();
    NETMGR_LOG_E("ParseBackgroundPolicy backgroundPolicyStatus: %{public}s", netPolicy.backgroundPolicyStatus.c_str());
}

void NetPolicyFile::ParseQuotaPolicy(const Json::Value &root, const cJSON* const object, NetPolicy &netPolicy)
{
    cJSON *netQuotaPolicy = cJSON_GetObjectItem(object, CONFIG_QUOTA_POLICY);
    if (netQuotaPolicy != nullptr) {
        uint32_t sized = cJSON_GetArraySize(netQuotaPolicy);
        NETMGR_LOG_E("Json2Obj netQuotaPolicy sized: %{public}u", sized);
        for (uint32_t j = 0; j < sized; j++) {
            cJSON *quotaPolicyItem = cJSON_GetArrayItem(netQuotaPolicy, j);
            cJSON *netType = cJSON_GetObjectItem(quotaPolicyItem, CONFIG_QUOTA_POLICY_NETTYPE);
            cJSON *simId = cJSON_GetObjectItem(quotaPolicyItem, CONFIG_QUOTA_POLICY_SUBSCRIBERID);
            cJSON *periodStartTime = cJSON_GetObjectItem(quotaPolicyItem, CONFIG_QUOTA_POLICY_PERIODSTARTTIME);
            cJSON *periodDuration = cJSON_GetObjectItem(quotaPolicyItem, CONFIG_QUOTA_POLICY_PERIODDURATION);
            cJSON *warningBytes = cJSON_GetObjectItem(quotaPolicyItem, CONFIG_QUOTA_POLICY_WARNINGBYTES);
            cJSON *limitBytes = cJSON_GetObjectItem(quotaPolicyItem, CONFIG_QUOTA_POLICY_LIMITBYTES);
            cJSON *lastLimitSnooze = cJSON_GetObjectItem(quotaPolicyItem, CONFIG_QUOTA_POLICY_LASTLIMITSNOOZE);
            cJSON *metered = cJSON_GetObjectItem(quotaPolicyItem, CONFIG_QUOTA_POLICY_METERED);
            cJSON *ident = cJSON_GetObjectItem(quotaPolicyItem, CONFIG_QUOTA_POLICY_IDENT);
            NETMGR_LOG_E("Json2Obj netType: %{public}s", netType->valuestring);
            NETMGR_LOG_E("Json2Obj policy: %{public}s", simId->valuestring);
            NETMGR_LOG_E("Json2Obj periodStartTime: %{public}s", periodStartTime->valuestring);
            NETMGR_LOG_E("Json2Obj periodDuration: %{public}s", periodDuration->valuestring);
            NETMGR_LOG_E("Json2Obj warningBytes: %{public}s", warningBytes->valuestring);
            NETMGR_LOG_E("Json2Obj limitBytes: %{public}s", limitBytes->valuestring);
            NETMGR_LOG_E("Json2Obj lastLimitSnooze: %{public}s", lastLimitSnooze->valuestring);
            NETMGR_LOG_E("Json2Obj metered: %{public}s", metered->valuestring);
            NETMGR_LOG_E("Json2Obj ident: %{public}s", ident->valuestring);
        }
    }

    const Json::Value &arrayQuotaPolicy = root[CONFIG_QUOTA_POLICY];
    uint32_t size = arrayQuotaPolicy.size();
    NetPolicyQuota quotaPolicy;
    for (uint32_t i = 0; i < size; i++) {
        quotaPolicy.netType = arrayQuotaPolicy[i][CONFIG_QUOTA_POLICY_NETTYPE].asString();
        quotaPolicy.simId = arrayQuotaPolicy[i][CONFIG_QUOTA_POLICY_SUBSCRIBERID].asString();
        quotaPolicy.periodStartTime = arrayQuotaPolicy[i][CONFIG_QUOTA_POLICY_PERIODSTARTTIME].asString();
        quotaPolicy.periodDuration = arrayQuotaPolicy[i][CONFIG_QUOTA_POLICY_PERIODDURATION].asString();
        quotaPolicy.warningBytes = arrayQuotaPolicy[i][CONFIG_QUOTA_POLICY_WARNINGBYTES].asString();
        quotaPolicy.limitBytes = arrayQuotaPolicy[i][CONFIG_QUOTA_POLICY_LIMITBYTES].asString();
        quotaPolicy.lastLimitSnooze = arrayQuotaPolicy[i][CONFIG_QUOTA_POLICY_LASTLIMITSNOOZE].asString();
        quotaPolicy.metered = arrayQuotaPolicy[i][CONFIG_QUOTA_POLICY_METERED].asString();
        quotaPolicy.ident = arrayQuotaPolicy[i][CONFIG_QUOTA_POLICY_IDENT].asString();
        netPolicy.netQuotaPolicies.push_back(quotaPolicy);
    }
}

void NetPolicyFile::ParseFirewallRule(const Json::Value &root, const cJSON* const object, NetPolicy &netPolicy)
{
    cJSON *netFirewallRules = cJSON_GetObjectItem(object, CONFIG_FIREWALL_RULE);
    if (netFirewallRules != nullptr) {
        uint32_t size = cJSON_GetArraySize(netFirewallRules);
        for (uint32_t k = 0; k < size; k++) {
            cJSON *firewallRulesItem = cJSON_GetArrayItem(netFirewallRules, k);
            std::string firewallRulesItemStr = firewallRulesItem->string;
            uint32_t chainType = CommonUtils::StrToUint(firewallRulesItemStr);
            cJSON *netDeniedList = cJSON_GetObjectItem(firewallRulesItem, CONFIG_FIREWALL_RULE_DENIEDLIST);
            cJSON *netAllowedList = cJSON_GetObjectItem(firewallRulesItem, CONFIG_FIREWALL_RULE_ALLOWEDLIST);
            NETMGR_LOG_E("Json2Obj netDeniedList[%{public}u]: %{public}s", chainType, cJSON_Print(netDeniedList));
            NETMGR_LOG_E("Json2Obj netAllowedList[%{public}u]: %{public}s", chainType, cJSON_Print(netAllowedList));
            uint32_t itemSize = cJSON_GetArraySize(netDeniedList);
            for (uint32_t j = 0; j < itemSize; j++) {
                cJSON *netDeniedListItem = cJSON_GetArrayItem(netDeniedList, j);
                std::string netDeniedListItemStr = cJSON_GetStringValue(netDeniedListItem);
                uint32_t deniedListNumber = CommonUtils::StrToUint(netDeniedListItemStr);
                NETMGR_LOG_E("Json2Obj netFirewallRules.deniedList: %{public}d", deniedListNumber);
            }
            itemSize = cJSON_GetArraySize(netAllowedList);
            for (uint32_t j = 0; j < itemSize; j++) {
                cJSON *netAllowedListItem = cJSON_GetArrayItem(netAllowedList, j);
                std::string netAllowedListItemStr = cJSON_GetStringValue(netAllowedListItem);
                uint32_t allowedListNumber = CommonUtils::StrToUint(netAllowedListItemStr);
                NETMGR_LOG_E("Json2Obj netFirewallRules.allowedList: %{public}d", allowedListNumber);
            }
        }
    }

    Json::StyledWriter styledWriter;
    std::string jsonStr;
    const Json::Value &mapFirewallList = root[CONFIG_FIREWALL_RULE];
    for (auto iter = mapFirewallList.begin(); iter != mapFirewallList.end(); iter++) {
        uint32_t chainType = CommonUtils::StrToUint(iter.key().asString());
        const Json::Value &deniedList = (*iter)[CONFIG_FIREWALL_RULE_DENIEDLIST];
        const Json::Value &allowedList = (*iter)[CONFIG_FIREWALL_RULE_ALLOWEDLIST];
        jsonStr = styledWriter.write(deniedList);
        NETMGR_LOG_E("ParseFirewallRule.deniedList : %{public}s", jsonStr.c_str());
        jsonStr = styledWriter.write(allowedList);
        NETMGR_LOG_E("ParseFirewallRule.allowedList : %{public}s", jsonStr.c_str());
        for (uint32_t i = 0; i < deniedList.size(); i++) {
            netPolicy_.netFirewallRules[chainType].deniedList.insert(CommonUtils::StrToUint(deniedList[i].asString()));
            NETMGR_LOG_E("deniedList[%{public}u]= %{public}s", chainType, deniedList[i].asString().c_str());
        }
        for (uint32_t i = 0; i < allowedList.size(); i++) {
            netPolicy_.netFirewallRules[chainType].allowedList.insert(
                CommonUtils::StrToUint(allowedList[i].asString()));
            NETMGR_LOG_E("allowedList[%{public}u]= %{public}s", chainType, allowedList[i].asString().c_str());
        }
    }
}

bool NetPolicyFile::Json2Obj(const std::string &content, NetPolicy &netPolicy)
{
    if (content.empty()) {
        return false;
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    JSONCPP_STRING errs;

    cJSON *json = cJSON_Parse(content.c_str());
    if (json == nullptr) {
        return false;
    }
    NETMGR_LOG_E("Json2Obj : %{public}s", cJSON_Print(json));

    bool isSuccess = reader->parse(content.c_str(), content.c_str() + content.length(), &root, &errs);
    if (isSuccess && errs.size() == 0) {
        cJSON *hosVersion = cJSON_GetObjectItem(json, CONFIG_HOS_VERSION);
        if (hosVersion == nullptr) {
            netPolicy.hosVersion = HOS_VERSION;
        } else {
            netPolicy.hosVersion = cJSON_GetStringValue(hosVersion);
            NETMGR_LOG_E("Json2Obj hosVersion: %{public}s", netPolicy.hosVersion.c_str());
        }

        netPolicy.hosVersion = root[CONFIG_HOS_VERSION].asString();
        if (netPolicy.hosVersion.empty()) {
            netPolicy.hosVersion = HOS_VERSION;
        }

        Json::StyledWriter styledWriter;
        std::string jsonStr = styledWriter.write(root);
        NETMGR_LOG_E("WriteCacheToJsonValue : %{public}s", jsonStr.c_str());

        // parse uid policy from file
        ParseUidPolicy(root, json, netPolicy);

        // parse background policy from file
        ParseBackgroundPolicy(root, json, netPolicy);

        // parse quota policy from file
        ParseQuotaPolicy(root, json, netPolicy);

        // parse firewall rule from file
        ParseFirewallRule(root, json, netPolicy);
        cJSON_Delete(json);
        return true;
    }
    cJSON_Delete(json);
    return false;
}

bool NetPolicyFile::Obj2Json(const NetPolicy &netPolicy, std::string &content)
{
    cJSON *object = cJSON_CreateObject();

    Json::Value root;
    Json::StreamWriterBuilder builder;
    auto streamWriter = std::unique_ptr<Json::StreamWriter>(builder.newStreamWriter());
    if (netPolicy_.hosVersion.empty()) {
        netPolicy_.hosVersion = HOS_VERSION;
    }
    cJSON_AddItemToObject(object, CONFIG_HOS_VERSION, cJSON_CreateString(netPolicy_.hosVersion.c_str()));
    root[CONFIG_HOS_VERSION] = Json::Value(netPolicy_.hosVersion);
    AddUidPolicy(root, object);
    AddBackgroundPolicy(root, object);
    AddQuotaPolicy(root, object);
    AddFirewallRule(root, object);
    NETMGR_LOG_E("Obj2Json : %{public}s", cJSON_Print(object));
    std::ostringstream out;
    streamWriter->write(root, &out);
    content = out.str();
    NETMGR_LOG_E("content : %{public}s", content.c_str());
    cJSON_Delete(object);
    return true;
}

void NetPolicyFile::AddQuotaPolicy(Json::Value &root, cJSON *object)
{
    cJSON *quotaPo = cJSON_CreateObject();

    uint32_t size = netPolicy_.netQuotaPolicies.size();
    for (uint32_t i = 0; i < size; i++) {
        cJSON *quotaPoItem = cJSON_CreateObject();
        cJSON_AddItemToObject(quotaPoItem, CONFIG_QUOTA_POLICY_NETTYPE,
                              cJSON_CreateString(netPolicy_.netQuotaPolicies[i].netType.c_str()));
        cJSON_AddItemToObject(quotaPoItem, CONFIG_QUOTA_POLICY_SUBSCRIBERID,
                              cJSON_CreateString(netPolicy_.netQuotaPolicies[i].simId.c_str()));
        cJSON_AddItemToObject(quotaPoItem, CONFIG_QUOTA_POLICY_PERIODSTARTTIME,
                              cJSON_CreateString(netPolicy_.netQuotaPolicies[i].periodStartTime.c_str()));
        cJSON_AddItemToObject(quotaPoItem, CONFIG_QUOTA_POLICY_PERIODDURATION,
                              cJSON_CreateString(netPolicy_.netQuotaPolicies[i].periodDuration.c_str()));
        cJSON_AddItemToObject(quotaPoItem, CONFIG_QUOTA_POLICY_WARNINGBYTES,
                              cJSON_CreateString(netPolicy_.netQuotaPolicies[i].warningBytes.c_str()));
        cJSON_AddItemToObject(quotaPoItem, CONFIG_QUOTA_POLICY_LIMITBYTES,
                              cJSON_CreateString(netPolicy_.netQuotaPolicies[i].limitBytes.c_str()));
        cJSON_AddItemToObject(quotaPoItem, CONFIG_QUOTA_POLICY_LASTLIMITSNOOZE,
                              cJSON_CreateString(netPolicy_.netQuotaPolicies[i].lastLimitSnooze.c_str()));
        cJSON_AddItemToObject(quotaPoItem, CONFIG_QUOTA_POLICY_METERED,
                              cJSON_CreateString(netPolicy_.netQuotaPolicies[i].metered.c_str()));
        cJSON_AddItemToObject(quotaPoItem, CONFIG_QUOTA_POLICY_IDENT,
                              cJSON_CreateString(netPolicy_.netQuotaPolicies[i].ident.c_str()));
        cJSON_AddItemToArray(quotaPo, quotaPoItem);

        Json::Value quotaPolicy;
        quotaPolicy[CONFIG_QUOTA_POLICY_NETTYPE] = netPolicy_.netQuotaPolicies[i].netType;
        quotaPolicy[CONFIG_QUOTA_POLICY_SUBSCRIBERID] = netPolicy_.netQuotaPolicies[i].simId;
        quotaPolicy[CONFIG_QUOTA_POLICY_PERIODSTARTTIME] = netPolicy_.netQuotaPolicies[i].periodStartTime;
        quotaPolicy[CONFIG_QUOTA_POLICY_PERIODDURATION] = netPolicy_.netQuotaPolicies[i].periodDuration;
        quotaPolicy[CONFIG_QUOTA_POLICY_WARNINGBYTES] = netPolicy_.netQuotaPolicies[i].warningBytes;
        quotaPolicy[CONFIG_QUOTA_POLICY_LIMITBYTES] = netPolicy_.netQuotaPolicies[i].limitBytes;
        quotaPolicy[CONFIG_QUOTA_POLICY_LASTLIMITSNOOZE] = netPolicy_.netQuotaPolicies[i].lastLimitSnooze;
        quotaPolicy[CONFIG_QUOTA_POLICY_METERED] = netPolicy_.netQuotaPolicies[i].metered;
        quotaPolicy[CONFIG_QUOTA_POLICY_IDENT] = netPolicy_.netQuotaPolicies[i].ident;
        root[CONFIG_QUOTA_POLICY].append(quotaPolicy);
    }

    cJSON_AddItemToObject(object, CONFIG_QUOTA_POLICY, quotaPo);
}

void NetPolicyFile::AddUidPolicy(Json::Value &root, cJSON *object)
{
    cJSON *uidPo = cJSON_CreateObject();

    uint32_t size = netPolicy_.uidPolicies.size();
    for (uint32_t i = 0; i < size; i++) {
        cJSON *uidPoItem = cJSON_CreateObject();
        cJSON_AddItemToObject(uidPoItem, CONFIG_UID, cJSON_CreateString(netPolicy_.uidPolicies[i].uid.c_str()));
        cJSON_AddItemToObject(uidPoItem, CONFIG_POLICY, cJSON_CreateString(netPolicy_.uidPolicies[i].policy.c_str()));
        cJSON_AddItemToArray(uidPo, uidPoItem);

        Json::Value uidPolicy;
        uidPolicy[CONFIG_UID] = netPolicy_.uidPolicies[i].uid;
        uidPolicy[CONFIG_POLICY] = netPolicy_.uidPolicies[i].policy;
        root[CONFIG_UID_POLICY].append(uidPolicy);
    }

    cJSON_AddItemToObject(object, CONFIG_UID_POLICY, uidPo);
}

void NetPolicyFile::AddBackgroundPolicy(Json::Value &root, cJSON *object)
{
    cJSON *backgroundPo = cJSON_CreateObject();

    Json::Value backgroundPolicy;
    if (netPolicy_.backgroundPolicyStatus.empty()) {
        netPolicy_.backgroundPolicyStatus = BACKGROUND_POLICY_ALLOW;
    }
    cJSON_AddItemToObject(backgroundPo, CONFIG_BACKGROUND_POLICY_STATUS,
                          cJSON_CreateString(netPolicy_.backgroundPolicyStatus.c_str()));
    cJSON_AddItemToObject(object, CONFIG_BACKGROUND_POLICY, backgroundPo);
    backgroundPolicy[CONFIG_BACKGROUND_POLICY_STATUS] = netPolicy_.backgroundPolicyStatus;
    root[CONFIG_BACKGROUND_POLICY] = backgroundPolicy;
}

void NetPolicyFile::AddFirewallRule(Json::Value &root, cJSON *object)
{
    cJSON *firewallRuleObj = cJSON_CreateObject();
    Json::Value mapFirewallList(Json::objectValue);
    for (auto &&[k, v] : netPolicy_.netFirewallRules) {
        NETMGR_LOG_D("read k[%{public}d].", k);
        cJSON *deniedListArr = cJSON_CreateArray();
        cJSON *allowedListArr = cJSON_CreateArray();
        cJSON *firewallRuleItem = cJSON_CreateObject();
        for (auto &it : v.deniedList) {
            cJSON_AddItemToArray(deniedListArr, cJSON_CreateString(std::to_string(it).c_str()));
        }
        for (auto &it : v.allowedList) {
            cJSON_AddItemToArray(allowedListArr, cJSON_CreateString(std::to_string(it).c_str()));
        }
        cJSON_AddItemToObject(firewallRuleItem, CONFIG_FIREWALL_RULE_DENIEDLIST, deniedListArr);
        cJSON_AddItemToObject(firewallRuleItem, CONFIG_FIREWALL_RULE_ALLOWEDLIST, allowedListArr);
        cJSON_AddItemToObject(firewallRuleObj, std::to_string(k).c_str(), firewallRuleItem);

        Json::Value deniedList(Json::arrayValue);
        Json::Value allowedList(Json::arrayValue);
        std::for_each(v.deniedList.begin(), v.deniedList.end(),
                      [&deniedList](const auto &it) { deniedList.append(std::to_string(it)); });
        std::for_each(v.allowedList.begin(), v.allowedList.end(),
                      [&allowedList](const auto &it) { allowedList.append(std::to_string(it)); });
        mapFirewallList[std::to_string(k)][CONFIG_FIREWALL_RULE_DENIEDLIST] = deniedList;
        mapFirewallList[std::to_string(k)][CONFIG_FIREWALL_RULE_ALLOWEDLIST] = allowedList;
    }
    cJSON_AddItemToObject(object, CONFIG_FIREWALL_RULE, firewallRuleObj);
    root[CONFIG_FIREWALL_RULE] = mapFirewallList;
}

uint32_t NetPolicyFile::ArbitrationWritePolicyToFile(uint32_t uid, uint32_t policy)
{
    uint32_t size = netPolicy_.uidPolicies.size();
    bool haveUidAndPolicy = false;
    uint32_t oldPolicy;
    for (uint32_t i = 0; i < size; i++) {
        auto uidTemp = CommonUtils::StrToUint(netPolicy_.uidPolicies[i].uid.c_str());
        if (uid == uidTemp) {
            haveUidAndPolicy = true;
            oldPolicy = uidTemp;
        }
    }

    if (haveUidAndPolicy) {
        if (oldPolicy != policy && policy == NET_POLICY_NONE) {
            return NET_POLICY_UID_OP_TYPE_DELETE;
        }

        if (oldPolicy != policy && policy != NET_POLICY_NONE) {
            return NET_POLICY_UID_OP_TYPE_UPDATE;
        }

        return NET_POLICY_UID_OP_TYPE_DO_NOTHING;
    }

    if (policy == NET_POLICY_NONE) {
        return NET_POLICY_UID_OP_TYPE_DO_NOTHING;
    }
    return NET_POLICY_UID_OP_TYPE_ADD;
}

void NetPolicyFile::WritePolicyByUid(uint32_t uid, uint32_t policy)
{
    uint32_t netUidPolicyOpType = ArbitrationWritePolicyToFile(uid, policy);
    WritePolicyByUid(netUidPolicyOpType, uid, policy);
}

void NetPolicyFile::WritePolicyByUid(uint32_t netUidPolicyOpType, uint32_t uid, uint32_t policy)
{
    NETMGR_LOG_D("Write File start, model:[%{public}u]", netUidPolicyOpType);
    if (netUidPolicyOpType == NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_UPDATE) {
        for (auto &uidPolicy : netPolicy_.uidPolicies) {
            if (uidPolicy.uid == std::to_string(uid)) {
                uidPolicy.policy = std::to_string(policy);
                break;
            }
        }
    } else if (netUidPolicyOpType == NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_DELETE) {
        for (auto iter = netPolicy_.uidPolicies.begin(); iter != netPolicy_.uidPolicies.end(); ++iter) {
            if (iter->uid == std::to_string(uid)) {
                netPolicy_.uidPolicies.erase(iter);
                break;
            }
        }
    } else if (netUidPolicyOpType == NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_ADD) {
        UidPolicy uidPolicy;
        uidPolicy.uid = std::to_string(uid);
        uidPolicy.policy = std::to_string(static_cast<uint32_t>(policy));
        netPolicy_.uidPolicies.push_back(uidPolicy);
    } else {
        NETMGR_LOG_D("Need to do nothing!");
    }

    WriteFile();
}

bool NetPolicyFile::UpdateQuotaPolicyExist(const NetQuotaPolicy &quotaPolicy)
{
    if (netPolicy_.netQuotaPolicies.empty()) {
        NETMGR_LOG_E("UpdateQuotaPolicyExist netQuotaPolicies is empty");
        return false;
    }

    for (uint32_t i = 0; i < netPolicy_.netQuotaPolicies.size(); ++i) {
        if (quotaPolicy.networkmatchrule.simId == netPolicy_.netQuotaPolicies[i].simId &&
            netPolicy_.netQuotaPolicies[i].netType == std::to_string(quotaPolicy.networkmatchrule.netType)) {
            netPolicy_.netQuotaPolicies[i].lastLimitSnooze = std::to_string(quotaPolicy.quotapolicy.lastLimitRemind);
            netPolicy_.netQuotaPolicies[i].limitBytes = std::to_string(quotaPolicy.quotapolicy.limitBytes);
            netPolicy_.netQuotaPolicies[i].metered = std::to_string(quotaPolicy.quotapolicy.metered);
            netPolicy_.netQuotaPolicies[i].netType = std::to_string(quotaPolicy.networkmatchrule.netType);
            netPolicy_.netQuotaPolicies[i].periodDuration = quotaPolicy.quotapolicy.periodDuration;
            netPolicy_.netQuotaPolicies[i].periodStartTime = std::to_string(quotaPolicy.quotapolicy.periodStartTime);
            netPolicy_.netQuotaPolicies[i].ident = quotaPolicy.networkmatchrule.ident;
            netPolicy_.netQuotaPolicies[i].simId = quotaPolicy.networkmatchrule.simId;
            netPolicy_.netQuotaPolicies[i].warningBytes = std::to_string(quotaPolicy.quotapolicy.warningBytes);
            return true;
        }
    }

    return false;
}

bool NetPolicyFile::WriteQuotaPolicies(const std::vector<NetQuotaPolicy> &quotaPolicies)
{
    netPolicy_.netQuotaPolicies.clear();
    uint32_t vSize = quotaPolicies.size();
    NetPolicyQuota quotaPolicy;
    for (uint32_t i = 0; i < vSize; i++) {
        if (UpdateQuotaPolicyExist(quotaPolicies[i])) {
            NETMGR_LOG_E("quotaPolicies:periodDuration[%{public}s], don't write this quotaPolicies!",
                         quotaPolicies[i].quotapolicy.periodDuration.c_str());
            continue;
        }
        quotaPolicy.lastLimitSnooze = std::to_string(quotaPolicies[i].quotapolicy.lastLimitRemind);
        quotaPolicy.limitBytes = std::to_string(quotaPolicies[i].quotapolicy.limitBytes);
        quotaPolicy.metered = std::to_string(quotaPolicies[i].quotapolicy.metered);
        quotaPolicy.ident = quotaPolicies[i].networkmatchrule.ident;
        quotaPolicy.netType = std::to_string(quotaPolicies[i].networkmatchrule.netType);
        quotaPolicy.periodDuration = quotaPolicies[i].quotapolicy.periodDuration;
        quotaPolicy.periodStartTime = std::to_string(quotaPolicies[i].quotapolicy.periodStartTime);
        quotaPolicy.simId = quotaPolicies[i].networkmatchrule.simId;
        quotaPolicy.warningBytes = std::to_string(quotaPolicies[i].quotapolicy.warningBytes);
        netPolicy_.netQuotaPolicies.push_back(quotaPolicy);
    }

    return WriteFile();
}

void NetPolicyFile::ReadQuotaPolicies(std::vector<NetQuotaPolicy> &quotaPolicies)
{
    NetQuotaPolicy quotaPolicyTmp;
    for (const auto &quotaPolicy : netPolicy_.netQuotaPolicies) {
        ToQuotaPolicy(quotaPolicy, quotaPolicyTmp);
        quotaPolicies.push_back(quotaPolicyTmp);
    }
}

int32_t NetPolicyFile::ReadFirewallRules(uint32_t chainType, std::set<uint32_t> &allowedList,
                                         std::set<uint32_t> &deniedList)
{
    auto &&w = netPolicy_.netFirewallRules[chainType].allowedList;
    auto &&b = netPolicy_.netFirewallRules[chainType].deniedList;
    allowedList.insert(w.begin(), w.end());
    deniedList.insert(b.begin(), b.end());
    return NETMANAGER_SUCCESS;
}

void NetPolicyFile::WriteFirewallRules(uint32_t chainType, const std::set<uint32_t> &allowedList,
                                       const std::set<uint32_t> &deniedList)
{
    netPolicy_.netFirewallRules[chainType].allowedList.clear();
    netPolicy_.netFirewallRules[chainType].deniedList.clear();
    netPolicy_.netFirewallRules[chainType].allowedList.insert(allowedList.begin(), allowedList.end());
    netPolicy_.netFirewallRules[chainType].deniedList.insert(deniedList.begin(), deniedList.end());
    WriteFile();
}

int32_t NetPolicyFile::ResetPolicies()
{
    netPolicy_.uidPolicies.clear();
    netPolicy_.backgroundPolicyStatus = BACKGROUND_POLICY_ALLOW;
    netPolicy_.netQuotaPolicies.clear();
    netPolicy_.netFirewallRules.clear();
    WriteFile();

    return NETMANAGER_SUCCESS;
}

void NetPolicyFile::WriteBackgroundPolicy(bool backgroundPolicy)
{
    if (backgroundPolicy) {
        netPolicy_.backgroundPolicyStatus = BACKGROUND_POLICY_ALLOW;
    } else {
        netPolicy_.backgroundPolicyStatus = BACKGROUND_POLICY_REJECT;
    }

    WriteFile();
}

bool NetPolicyFile::ReadBackgroundPolicy()
{
    return netPolicy_.backgroundPolicyStatus == BACKGROUND_POLICY_ALLOW;
}

std::shared_ptr<NetPolicyFileEventHandler> NetPolicyFile::GetHandler()
{
    static auto handler = [this]() -> std::shared_ptr<NetPolicyFileEventHandler> {
        auto runner = AppExecFwk::EventRunner::Create(NET_POLICY_WORK_THREAD);
        if (!runner) {
            NETMGR_LOG_E("Create net policy file work event runner.");
            return nullptr;
        }
        return std::make_shared<NetPolicyFileEventHandler>(runner);
    }();
    return handler;
}

bool NetPolicyFile::InitPolicy()
{
    ResetPolicies();
    return ReadFile();
}

void NetPolicyFile::RemoveInexistentUid(uint32_t uid)
{
    WritePolicyByUid(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_DELETE, uid, 0);
}
} // namespace NetManagerStandard
} // namespace OHOS
