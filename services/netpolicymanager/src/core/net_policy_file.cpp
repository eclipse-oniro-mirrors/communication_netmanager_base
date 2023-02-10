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

#include "net_policy_file.h"

#include <fcntl.h>
#include <string>

#include <json/json.h>

#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"
#include "net_policy_file_event_handler.h"
#include "net_policy_inner_define.h"
#include "netmanager_base_common_utils.h"

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
    std::lock_guard<std::mutex> lock(mutex_);
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
    if (file.is_open() == false) {
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

void NetPolicyFile::ParseUidPolicy(const Json::Value &root, NetPolicy &netPolicy)
{
    const Json::Value &arrayUidPolicy = root[CONFIG_UID_POLICY];
    uint32_t size = arrayUidPolicy.size();
    UidPolicy uidPolicy;
    for (uint32_t i = 0; i < size; i++) {
        uidPolicy.uid = arrayUidPolicy[i][CONFIG_UID].asString();
        uidPolicy.policy = arrayUidPolicy[i][CONFIG_POLICY].asString();
        netPolicy.uidPolicies.push_back(uidPolicy);
    }
}

void NetPolicyFile::ParseBackgroundPolicy(const Json::Value &root, NetPolicy &netPolicy)
{
    const Json::Value &mapBackgroundPolicy = root[CONFIG_BACKGROUND_POLICY];
    netPolicy.backgroundPolicyStatus = mapBackgroundPolicy[CONFIG_BACKGROUND_POLICY_STATUS].asString();
}

void NetPolicyFile::ParseQuotaPolicy(const Json::Value &root, NetPolicy &netPolicy)
{
    const Json::Value &arrayQuotaPolicy = root[CONFIG_QUOTA_POLICY];
    uint32_t size = arrayQuotaPolicy.size();
    NetPolicyQuota quotaPolicy;
    for (uint32_t i = 0; i < size; i++) {
        quotaPolicy.netType = arrayQuotaPolicy[i][CONFIG_QUOTA_POLICY_NETTYPE].asString();
        quotaPolicy.iccid = arrayQuotaPolicy[i][CONFIG_QUOTA_POLICY_SUBSCRIBERID].asString();
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

void NetPolicyFile::ParseFirewallRule(const Json::Value &root, NetPolicy &netPolicy)
{
    const Json::Value &mapFirewallList = root[CONFIG_FIREWALL_RULE];
    for (auto iter = mapFirewallList.begin(); iter != mapFirewallList.end(); iter++) {
        uint32_t chainType = CommonUtils::StrToUint(iter.key().asString());
        const Json::Value &deniedList = (*iter)[CONFIG_FIREWALL_RULE_DENIEDLIST];
        const Json::Value &allowedList = (*iter)[CONFIG_FIREWALL_RULE_ALLOWEDLIST];
        for (uint32_t i = 0; i < deniedList.size(); i++) {
            netPolicy_.netFirewallRules[chainType].deniedList.insert(CommonUtils::StrToUint(deniedList[i].asString()));
        }
        for (uint32_t i = 0; i < allowedList.size(); i++) {
            netPolicy_.netFirewallRules[chainType].allowedList.insert(
                CommonUtils::StrToUint(allowedList[i].asString()));
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

    bool isSuccess = reader->parse(content.c_str(), content.c_str() + content.length(), &root, &errs);
    if (isSuccess && errs.size() == 0) {
        netPolicy.hosVersion = root[CONFIG_HOS_VERSION].asString();
        if (netPolicy.hosVersion.empty()) {
            netPolicy.hosVersion = HOS_VERSION;
        }

        // parse uid policy from file
        ParseUidPolicy(root, netPolicy);

        // parse background policy from file
        ParseBackgroundPolicy(root, netPolicy);

        // parse quota policy from file
        ParseQuotaPolicy(root, netPolicy);

        // parse firewall rule from file
        ParseFirewallRule(root, netPolicy);

        return true;
    }

    return false;
}

bool NetPolicyFile::Obj2Json(const NetPolicy &netPolicy, std::string &content)
{
    Json::Value root;
    Json::StreamWriterBuilder builder;
    auto streamWriter = std::unique_ptr<Json::StreamWriter>(builder.newStreamWriter());

    if (netPolicy_.hosVersion.empty()) {
        netPolicy_.hosVersion = HOS_VERSION;
    }

    root[CONFIG_HOS_VERSION] = Json::Value(netPolicy_.hosVersion);

    // uid policy
    AppendUidPolicy(root);

    // background policy
    AppendBackgroundPolicy(root);

    // quota policy
    AppendQuotaPolicy(root);

    // firewall rule
    AppendFirewallRule(root);

    std::ostringstream out;
    streamWriter->write(root, &out);
    content = out.str();
    return true;
}

void NetPolicyFile::AppendQuotaPolicy(Json::Value &root)
{
    uint32_t size = netPolicy_.netQuotaPolicies.size();
    for (uint32_t i = 0; i < size; i++) {
        Json::Value quotaPolicy;
        quotaPolicy[CONFIG_QUOTA_POLICY_NETTYPE] = netPolicy_.netQuotaPolicies[i].netType;
        quotaPolicy[CONFIG_QUOTA_POLICY_SUBSCRIBERID] = netPolicy_.netQuotaPolicies[i].iccid;
        quotaPolicy[CONFIG_QUOTA_POLICY_PERIODSTARTTIME] = netPolicy_.netQuotaPolicies[i].periodStartTime;
        quotaPolicy[CONFIG_QUOTA_POLICY_PERIODDURATION] = netPolicy_.netQuotaPolicies[i].periodDuration;
        quotaPolicy[CONFIG_QUOTA_POLICY_WARNINGBYTES] = netPolicy_.netQuotaPolicies[i].warningBytes;
        quotaPolicy[CONFIG_QUOTA_POLICY_LIMITBYTES] = netPolicy_.netQuotaPolicies[i].limitBytes;
        quotaPolicy[CONFIG_QUOTA_POLICY_LASTLIMITSNOOZE] = netPolicy_.netQuotaPolicies[i].lastLimitSnooze;
        quotaPolicy[CONFIG_QUOTA_POLICY_METERED] = netPolicy_.netQuotaPolicies[i].metered;
        quotaPolicy[CONFIG_QUOTA_POLICY_IDENT] = netPolicy_.netQuotaPolicies[i].ident;
        root[CONFIG_QUOTA_POLICY].append(quotaPolicy);
    }
}

void NetPolicyFile::AppendUidPolicy(Json::Value &root)
{
    uint32_t size = netPolicy_.uidPolicies.size();
    for (uint32_t i = 0; i < size; i++) {
        Json::Value uidPolicy;
        uidPolicy[CONFIG_UID] = netPolicy_.uidPolicies[i].uid;
        uidPolicy[CONFIG_POLICY] = netPolicy_.uidPolicies[i].policy;
        root[CONFIG_UID_POLICY].append(uidPolicy);
    }
}

void NetPolicyFile::AppendBackgroundPolicy(Json::Value &root)
{
    Json::Value backgroundPolicy;
    if (netPolicy_.backgroundPolicyStatus.empty()) {
        netPolicy_.backgroundPolicyStatus = BACKGROUND_POLICY_ALLOW;
    }
    backgroundPolicy[CONFIG_BACKGROUND_POLICY_STATUS] = netPolicy_.backgroundPolicyStatus;
    root[CONFIG_BACKGROUND_POLICY] = backgroundPolicy;
}

void NetPolicyFile::AppendFirewallRule(Json::Value &root)
{
    Json::Value mapFirewallList(Json::objectValue);
    for (auto &&[k, v] : netPolicy_.netFirewallRules) {
        NETMGR_LOG_D("read k[%{public}d].", k);
        Json::Value deniedList(Json::arrayValue);
        Json::Value allowedList(Json::arrayValue);
        std::for_each(v.deniedList.begin(), v.deniedList.end(),
                      [&deniedList](const auto &it) { deniedList.append(std::to_string(it)); });
        std::for_each(v.allowedList.begin(), v.allowedList.end(),
                      [&allowedList](const auto &it) { allowedList.append(std::to_string(it)); });
        mapFirewallList[std::to_string(k)][CONFIG_FIREWALL_RULE_DENIEDLIST] = deniedList;
        mapFirewallList[std::to_string(k)][CONFIG_FIREWALL_RULE_ALLOWEDLIST] = allowedList;
    }
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

void NetPolicyFile::WriteUidPolicy(uint32_t uid, uint32_t policy)
{
    uint32_t netUidPolicyOpType = ArbitrationWritePolicyToFile(uid, policy);
    WriteUidPolicy(netUidPolicyOpType, uid, policy);
}

void NetPolicyFile::WriteUidPolicy(uint32_t netUidPolicyOpType, uint32_t uid, uint32_t policy)
{
    NETMGR_LOG_D("Write File start, model:[%{public}u]", netUidPolicyOpType);

    for (const auto &i : netPolicy_.uidPolicies) {
        uint32_t uid = CommonUtils::StrToUint(i.uid.c_str());
        uint32_t policy = CommonUtils::StrToUint(i.policy.c_str());
        NETMGR_LOG_D("Struct:uid[%{public}u],policy[%{public}u]", uid, policy);
    }

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
        NETMGR_LOG_I("Need to do nothing!");
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
        if (quotaPolicy.iccid == netPolicy_.netQuotaPolicies[i].iccid &&
            netPolicy_.netQuotaPolicies[i].netType == std::to_string(quotaPolicy.netType)) {
            netPolicy_.netQuotaPolicies[i].lastLimitSnooze = std::to_string(quotaPolicy.lastLimitRemind);
            netPolicy_.netQuotaPolicies[i].limitBytes = std::to_string(quotaPolicy.limitBytes);
            netPolicy_.netQuotaPolicies[i].metered = std::to_string(quotaPolicy.metered);
            netPolicy_.netQuotaPolicies[i].netType = std::to_string(quotaPolicy.netType);
            netPolicy_.netQuotaPolicies[i].periodDuration = quotaPolicy.periodDuration;
            netPolicy_.netQuotaPolicies[i].periodStartTime = std::to_string(quotaPolicy.periodStartTime);
            netPolicy_.netQuotaPolicies[i].ident = quotaPolicy.ident;
            netPolicy_.netQuotaPolicies[i].iccid = quotaPolicy.iccid;
            netPolicy_.netQuotaPolicies[i].warningBytes = std::to_string(quotaPolicy.warningBytes);
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
                         quotaPolicies[i].periodDuration.c_str());
            continue;
        }
        quotaPolicy.lastLimitSnooze = std::to_string(quotaPolicies[i].lastLimitRemind);
        quotaPolicy.limitBytes = std::to_string(quotaPolicies[i].limitBytes);
        quotaPolicy.metered = std::to_string(quotaPolicies[i].metered);
        quotaPolicy.ident = quotaPolicies[i].ident;
        quotaPolicy.netType = std::to_string(quotaPolicies[i].netType);
        quotaPolicy.periodDuration = quotaPolicies[i].periodDuration;
        quotaPolicy.periodStartTime = std::to_string(quotaPolicies[i].periodStartTime);
        quotaPolicy.iccid = quotaPolicies[i].iccid;
        quotaPolicy.warningBytes = std::to_string(quotaPolicies[i].warningBytes);
        netPolicy_.netQuotaPolicies.push_back(quotaPolicy);
    }

    return WriteFile();
}

int32_t NetPolicyFile::ReadQuotaPolicies(std::vector<NetQuotaPolicy> &quotaPolicies)
{
    NetQuotaPolicy quotaPolicyTmp;
    for (auto &quotaPolicy : netPolicy_.netQuotaPolicies) {
        quotaPolicyTmp.lastLimitRemind = CommonUtils::StrToLong(quotaPolicy.lastLimitSnooze);
        quotaPolicyTmp.limitBytes = CommonUtils::StrToLong(quotaPolicy.limitBytes);
        quotaPolicyTmp.metered = CommonUtils::StrToBool(quotaPolicy.metered);
        quotaPolicyTmp.netType = CommonUtils::StrToInt(quotaPolicy.netType);
        quotaPolicyTmp.periodDuration = quotaPolicy.periodDuration;
        quotaPolicyTmp.periodStartTime = CommonUtils::StrToLong(quotaPolicy.periodStartTime);
        quotaPolicyTmp.ident = quotaPolicy.ident;
        quotaPolicyTmp.iccid = quotaPolicy.iccid;
        quotaPolicyTmp.warningBytes = CommonUtils::StrToLong(quotaPolicy.warningBytes);
        quotaPolicies.push_back(quotaPolicyTmp);
    }

    return NETMANAGER_SUCCESS;
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
    WriteUidPolicy(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_DELETE, uid, 0);
}
} // namespace NetManagerStandard
} // namespace OHOS
