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
#include <json/json.h>
#include <string>

#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"
#include "net_policy_inner_define.h"
#include "netmanager_base_common_utils.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string MONTH_DEFAULT = "M1";

NetPolicyFile::NetPolicyFile() = default;

NetPolicyFile::~NetPolicyFile() = default;

bool NetPolicyFile::FileExists(const std::string &fileName)
{
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == ERR_NONE);
}

bool NetPolicyFile::CreateFile(const std::string &fileName)
{
    if (fileName.empty() || FileExists(fileName)) {
        NETMGR_LOG_E("fileName empty or file not exists.");
        return false;
    }

    int32_t fd = open(fileName.c_str(), O_CREAT | O_WRONLY, CHOWN_RWX_USR_GRP);
    if (fd < 0) {
        NETMGR_LOG_E("open file error.");
        return false;
    }
    close(fd);

    return true;
}

const std::vector<UidPolicy> &NetPolicyFile::GetNetPolicies()
{
    return netPolicy_.uidPolicys;
}

void NetPolicyFile::ParseUidPolicy(const Json::Value &root, NetPolicy &netPolicy)
{
    const Json::Value arrayUidPolicy = root[CONFIG_UID_POLICY];
    uint32_t size = arrayUidPolicy.size();
    UidPolicy uidPolicy;
    for (uint32_t i = 0; i < size; i++) {
        uidPolicy.uid = arrayUidPolicy[i][CONFIG_UID].asString();
        uidPolicy.policy = arrayUidPolicy[i][CONFIG_POLICY].asString();
        netPolicy.uidPolicys.push_back(uidPolicy);
    }
}

void NetPolicyFile::ParseBackgroundPolicy(const Json::Value &root, NetPolicy &netPolicy)
{
    const Json::Value mapBackgroundPolicy = root[CONFIG_BACKGROUND_POLICY];
    netPolicy.backgroundPolicyStatus_ = mapBackgroundPolicy[CONFIG_BACKGROUND_POLICY_STATUS].asString();
}

void NetPolicyFile::ParseQuotaPolicy(const Json::Value &root, NetPolicy &netPolicy)
{
    const Json::Value arrayQuotaPolicy = root[CONFIG_QUOTA_POLICY];
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
        quotaPolicy.source = arrayQuotaPolicy[i][CONFIG_QUOTA_POLICY_SOURCE].asString();
        netPolicy.netQuotaPolicys.push_back(quotaPolicy);
    }
}

void NetPolicyFile::ParseCellularPolicy(const Json::Value &root, NetPolicy &netPolicy)
{
    const Json::Value arrayCellularPolicy = root[CONFIG_CELLULAR_POLICY];
    uint32_t size = arrayCellularPolicy.size();
    NetPolicyCellular cellularPolicy;
    for (uint32_t i = 0; i < size; i++) {
        cellularPolicy.iccid = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_SUBSCRIBERID].asString();
        cellularPolicy.periodStartTime = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_PERIODSTARTTIME].asString();
        cellularPolicy.periodDuration = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_PERIODDURATION].asString();
        cellularPolicy.title = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_TITLE].asString();
        cellularPolicy.summary = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_SUMMARY].asString();
        cellularPolicy.limitBytes = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_LIMITBYTES].asString();
        cellularPolicy.limitAction = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_LIMITACTION].asString();
        cellularPolicy.usedBytes = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_USEDBYTES].asString();
        cellularPolicy.usedTimeDuration = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_USEDTIMEDURATION].asString();
        cellularPolicy.possessor = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_POSSESSOR].asString();
    }
}

bool NetPolicyFile::Json2Obj(const std::string &content, NetPolicy &netPolicy)
{
    if (content.empty()) {
        return false;
    }

    Json::Value root;
    Json::CharReaderBuilder buidler;
    std::unique_ptr<Json::CharReader> reader(buidler.newCharReader());
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
        // parse cellular policy from file
        ParseCellularPolicy(root, netPolicy);
    }

    return true;
}

bool NetPolicyFile::ReadFile(const std::string &fileName, std::string &fileContent)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (fileName.empty()) {
        NETMGR_LOG_E("fileName empty.");
        return false;
    }

    if (!FileExists(fileName)) {
        NETMGR_LOG_E("[%{public}s] not exist.", fileName.c_str());
        return false;
    }

    std::fstream file(fileName.c_str(), std::fstream::in);
    if (file.is_open() == false) {
        NETMGR_LOG_E("fstream failed.");
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    fileContent = buffer.str();
    file.close();

    return true;
}

void NetPolicyFile::AppendQuotaPolicy(Json::Value &root)
{
    uint32_t size = netPolicy_.netQuotaPolicys.size();
    for (uint32_t i = 0; i < size; i++) {
        Json::Value quotaPolicy;
        quotaPolicy[CONFIG_QUOTA_POLICY_NETTYPE] = netPolicy_.netQuotaPolicys[i].netType;
        quotaPolicy[CONFIG_QUOTA_POLICY_SUBSCRIBERID] = netPolicy_.netQuotaPolicys[i].iccid;
        quotaPolicy[CONFIG_QUOTA_POLICY_PERIODSTARTTIME] = netPolicy_.netQuotaPolicys[i].periodStartTime;
        quotaPolicy[CONFIG_QUOTA_POLICY_PERIODDURATION] = netPolicy_.netQuotaPolicys[i].periodDuration;
        quotaPolicy[CONFIG_QUOTA_POLICY_WARNINGBYTES] = netPolicy_.netQuotaPolicys[i].warningBytes;
        quotaPolicy[CONFIG_QUOTA_POLICY_LIMITBYTES] = netPolicy_.netQuotaPolicys[i].limitBytes;
        quotaPolicy[CONFIG_QUOTA_POLICY_LASTLIMITSNOOZE] = netPolicy_.netQuotaPolicys[i].lastLimitSnooze;
        quotaPolicy[CONFIG_QUOTA_POLICY_METERED] = netPolicy_.netQuotaPolicys[i].metered;
        quotaPolicy[CONFIG_QUOTA_POLICY_SOURCE] = netPolicy_.netQuotaPolicys[i].source;
        root[CONFIG_QUOTA_POLICY].append(quotaPolicy);
    }
}

void NetPolicyFile::AppendUidPolicy(Json::Value &root)
{
    uint32_t size = netPolicy_.uidPolicys.size();
    for (uint32_t i = 0; i < size; i++) {
        Json::Value uidPolicy;
        uidPolicy[CONFIG_UID] = netPolicy_.uidPolicys[i].uid;
        uidPolicy[CONFIG_POLICY] = netPolicy_.uidPolicys[i].policy;
        root[CONFIG_UID_POLICY].append(uidPolicy);
    }
}

void NetPolicyFile::AppendBackgroundPolicy(Json::Value &root)
{
    Json::Value backgroundPolicy;
    if (netPolicy_.backgroundPolicyStatus_.empty()) {
        netPolicy_.backgroundPolicyStatus_ = BACKGROUND_POLICY_ALLOW;
    }
    backgroundPolicy[CONFIG_BACKGROUND_POLICY_STATUS] = netPolicy_.backgroundPolicyStatus_;
    root[CONFIG_BACKGROUND_POLICY] = backgroundPolicy;
}

uint32_t NetPolicyFile::ArbitrationWritePolicyToFile(uint32_t uid, uint32_t policy)
{
    uint32_t size = netPolicy_.uidPolicys.size();
    bool haveUidAndPolicy = false;
    uint32_t oldPolicy;
    for (uint32_t i = 0; i < size; i++) {
        if (uid == CommonUtils::StrToUint(netPolicy_.uidPolicys[i].uid.c_str())) {
            haveUidAndPolicy = true;
            oldPolicy = CommonUtils::StrToUint(netPolicy_.uidPolicys[i].policy.c_str());
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

void NetPolicyFile::WriteFile(uint32_t uid, uint32_t policy)
{
    uint32_t netUidPolicyOpType = ArbitrationWritePolicyToFile(uid, policy);
    WriteFile(netUidPolicyOpType, uid, policy);
    return;
}

bool NetPolicyFile::WriteFile(const std::string &fileName)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (fileName.empty()) {
        NETMGR_LOG_D("fileName is empty.");
        return false;
    }

    Json::Value root;
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> streamWriter(builder.newStreamWriter());
    std::fstream file(fileName.c_str(), std::fstream::out);
    if (file.is_open() == false) {
        NETMGR_LOG_E("fstream failed.");
        return false;
    }

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
    std::ostringstream out;
    streamWriter->write(root, &out);
    file << out.str().c_str();
    file.close();

    return true;
}

bool NetPolicyFile::WriteFile(uint32_t netUidPolicyOpType, uint32_t uid, uint32_t policy)
{
    NETMGR_LOG_I("Write File start, model:[%{public}u]", netUidPolicyOpType);

    for (const auto &i : netPolicy_.uidPolicys) {
        uint32_t uid = CommonUtils::StrToUint(i.uid.c_str());
        uint32_t policy = CommonUtils::StrToUint(i.policy.c_str());
        NETMGR_LOG_D("Struct:uid[%{public}u],policy[%{public}u]", uid, policy);
    }

    if (netUidPolicyOpType == NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_UPDATE) {
        for (auto &uidPolicy : netPolicy_.uidPolicys) {
            if (uidPolicy.uid == std::to_string(uid)) {
                uidPolicy.policy = std::to_string(static_cast<uint32_t>(policy));
                break;
            }
        }
    } else if (netUidPolicyOpType == NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_DELETE) {
        for (auto iter = netPolicy_.uidPolicys.begin(); iter != netPolicy_.uidPolicys.end(); ++iter) {
            if (iter->uid == std::to_string(uid)) {
                netPolicy_.uidPolicys.erase(iter);
                break;
            }
        }
    } else if (netUidPolicyOpType == NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_ADD) {
        UidPolicy uidPolicy;
        uidPolicy.uid = std::to_string(uid);
        uidPolicy.policy = std::to_string(static_cast<uint32_t>(policy));
        netPolicy_.uidPolicys.push_back(uidPolicy);
    } else {
        NETMGR_LOG_I("Need to do nothing!");
    }

    if (!WriteFile(POLICY_FILE_NAME)) {
        NETMGR_LOG_E("WriteFile failed");
        return false;
    }

    return true;
}

bool NetPolicyFile::UpdateQuotaPolicyExist(const NetQuotaPolicy &quotaPolicy)
{
    if (netPolicy_.netQuotaPolicys.empty()) {
        NETMGR_LOG_E("UpdateQuotaPolicyExist NetQuotaPolicys is empty");
        return false;
    }

    for (uint32_t i = 0; i < netPolicy_.netQuotaPolicys.size(); ++i) {
        if (quotaPolicy.iccid == netPolicy_.netQuotaPolicys[i].iccid &&
            netPolicy_.netQuotaPolicys[i].netType == std::to_string(quotaPolicy.netType)) {
            netPolicy_.netQuotaPolicys[i].lastLimitSnooze = std::to_string(quotaPolicy.lastLimitRemind);
            netPolicy_.netQuotaPolicys[i].limitBytes = std::to_string(quotaPolicy.limitBytes);
            netPolicy_.netQuotaPolicys[i].metered = std::to_string(quotaPolicy.metered);
            netPolicy_.netQuotaPolicys[i].netType = std::to_string(quotaPolicy.netType);
            netPolicy_.netQuotaPolicys[i].periodDuration = quotaPolicy.periodDuration;
            netPolicy_.netQuotaPolicys[i].periodStartTime = std::to_string(quotaPolicy.periodStartTime);
            netPolicy_.netQuotaPolicys[i].source = std::to_string(quotaPolicy.source);
            netPolicy_.netQuotaPolicys[i].iccid = quotaPolicy.iccid;
            netPolicy_.netQuotaPolicys[i].warningBytes = std::to_string(quotaPolicy.warningBytes);
            return true;
        }
    }

    return false;
}

bool NetPolicyFile::WriteFile(const std::vector<NetQuotaPolicy> &quotaPolicies)
{
    netPolicy_.netQuotaPolicys.clear();
    uint32_t vSize = static_cast<uint32_t>(quotaPolicies.size());
    NetPolicyQuota quotaPolicy;
    for (uint32_t i = 0; i < vSize; i++) {
        if (UpdateQuotaPolicyExist(quotaPolicies[i])) {
            NETMGR_LOG_E("quotaPolicies:periodDuration[%{public}s, don't write this quotaPolicies!]",
                         quotaPolicies[i].periodDuration.c_str());
            continue;
        }
        quotaPolicy.lastLimitSnooze = std::to_string(quotaPolicies[i].lastLimitRemind);
        quotaPolicy.limitBytes = std::to_string(quotaPolicies[i].limitBytes);
        quotaPolicy.metered = std::to_string(quotaPolicies[i].metered);
        quotaPolicy.netType = std::to_string(quotaPolicies[i].netType);
        quotaPolicy.periodDuration = quotaPolicies[i].periodDuration;
        quotaPolicy.periodStartTime = std::to_string(quotaPolicies[i].periodStartTime);
        quotaPolicy.source = std::to_string(quotaPolicies[i].source);
        quotaPolicy.iccid = quotaPolicies[i].iccid;
        quotaPolicy.warningBytes = std::to_string(quotaPolicies[i].warningBytes);
        netPolicy_.netQuotaPolicys.push_back(quotaPolicy);
    }

    if (!WriteFile(POLICY_FILE_NAME)) {
        NETMGR_LOG_E("WriteFile failed");
        return false;
    }

    return true;
}

bool NetPolicyFile::IsUidPolicyExist(uint32_t uid)
{
    uint32_t size = netPolicy_.uidPolicys.size();
    for (uint32_t i = 0; i < size; i++) {
        if (CommonUtils::StrToUint(netPolicy_.uidPolicys[i].uid) == uid) {
            return true;
        }
    }

    return false;
}

NetUidPolicy NetPolicyFile::GetPolicyByUid(uint32_t uid)
{
    for (auto &uidPolicy : netPolicy_.uidPolicys) {
        if (uidPolicy.uid == std::to_string(uid)) {
            return static_cast<NetUidPolicy>(CommonUtils::StrToUint(uidPolicy.policy));
        }
    }

    return NetUidPolicy::NET_POLICY_NONE;
}

bool NetPolicyFile::GetUidsByPolicy(uint32_t policy, std::vector<uint32_t> &uids)
{
    for (auto &uidPolicy : netPolicy_.uidPolicys) {
        if (uidPolicy.policy == std::to_string(static_cast<uint32_t>(policy))) {
            uint32_t uid = CommonUtils::StrToUint(uidPolicy.uid);
            uids.push_back(uid);
        }
    }
    return uids.size() > 0;
}

int32_t NetPolicyFile::ReadQuotaPolicies(std::vector<NetQuotaPolicy> &quotaPolicies)
{
    NetQuotaPolicy quotaPolicyTmp;
    for (auto &quotaPolicy : netPolicy_.netQuotaPolicys) {
        quotaPolicyTmp.lastLimitRemind = CommonUtils::StrToLong(quotaPolicy.lastLimitSnooze);
        quotaPolicyTmp.limitBytes = CommonUtils::StrToLong(quotaPolicy.limitBytes);
        quotaPolicyTmp.metered = CommonUtils::StrToBool(quotaPolicy.metered);
        quotaPolicyTmp.netType = CommonUtils::StrToInt(quotaPolicy.netType);
        quotaPolicyTmp.periodDuration = quotaPolicy.periodDuration;
        quotaPolicyTmp.periodStartTime = CommonUtils::StrToLong(quotaPolicy.periodStartTime);
        quotaPolicyTmp.source = CommonUtils::StrToInt(quotaPolicy.source);
        quotaPolicyTmp.iccid = quotaPolicy.iccid;
        quotaPolicyTmp.warningBytes = CommonUtils::StrToLong(quotaPolicy.warningBytes);
        quotaPolicies.push_back(quotaPolicyTmp);
    }

    return ERR_NONE;
}

int32_t NetPolicyFile::GetNetQuotaPolicy(int32_t netType, const std::string &iccid, NetQuotaPolicy &quotaPolicy)
{
    for (auto &quotaPolicyTemp : netPolicy_.netQuotaPolicys) {
        if (netType == CommonUtils::StrToInt(quotaPolicyTemp.netType) && iccid == quotaPolicyTemp.iccid) {
            quotaPolicy.lastLimitRemind = CommonUtils::StrToLong(quotaPolicyTemp.lastLimitSnooze);
            quotaPolicy.limitBytes = CommonUtils::StrToLong(quotaPolicyTemp.limitBytes);
            quotaPolicy.metered = CommonUtils::StrToBool(quotaPolicyTemp.metered);
            quotaPolicy.netType = CommonUtils::StrToInt(quotaPolicyTemp.netType);
            quotaPolicy.periodDuration = quotaPolicyTemp.periodDuration;
            quotaPolicy.periodStartTime = CommonUtils::StrToLong(quotaPolicyTemp.periodStartTime);
            quotaPolicy.source = CommonUtils::StrToInt(quotaPolicyTemp.source);
            quotaPolicy.iccid = quotaPolicyTemp.iccid;
            quotaPolicy.warningBytes = CommonUtils::StrToLong(quotaPolicyTemp.warningBytes);
            return ERR_NONE;
        }
    }

    return ERR_QUOTA_POLICY_NOT_EXIST;
}

int32_t NetPolicyFile::ResetPolicies(const std::string &iccid)
{
    netPolicy_.uidPolicys.clear();
    netPolicy_.backgroundPolicyStatus_ = BACKGROUND_POLICY_ALLOW;

    if (iccid.empty()) {
        netPolicy_.netQuotaPolicys.clear();
    } else {
        for (auto iter = netPolicy_.netQuotaPolicys.begin(); iter != netPolicy_.netQuotaPolicys.end(); ++iter) {
            if (iccid == iter->iccid) {
                netPolicy_.netQuotaPolicys.erase(iter);
                break;
            }
        }
    }

    if (!WriteFile(POLICY_FILE_NAME)) {
        NETMGR_LOG_E("WriteFile failed");
        return ERR_INTERNAL_ERROR;
    }

    return ERR_NONE;
}

int32_t NetPolicyFile::SetBackgroundPolicy(bool backgroundPolicy)
{
    if (backgroundPolicy) {
        netPolicy_.backgroundPolicyStatus_ = BACKGROUND_POLICY_ALLOW;
    } else {
        netPolicy_.backgroundPolicyStatus_ = BACKGROUND_POLICY_REJECT;
    }

    if (!WriteFile(POLICY_FILE_NAME)) {
        NETMGR_LOG_E("WriteFile failed");
        return ERR_INTERNAL_ERROR;
    }

    return ERR_NONE;
}

bool NetPolicyFile::GetBackgroundPolicy()
{
    if (netPolicy_.backgroundPolicyStatus_ == BACKGROUND_POLICY_ALLOW) {
        return true;
    }
    return false;
}

bool NetPolicyFile::InitPolicy()
{
    NETMGR_LOG_I("InitPolicyFile.");
    std::string content;
    if (!ReadFile(POLICY_FILE_NAME, content)) {
        if (!CreateFile(POLICY_FILE_NAME)) {
            NETMGR_LOG_D("CreateFile [%{public}s] failed", POLICY_FILE_NAME);
            return false;
        }
    }

    if (!content.empty() && !Json2Obj(content, netPolicy_)) {
        NETMGR_LOG_E("Analysis fileconfig failed");
        return false;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
