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
#include "net_policy_file.h"

#include <fcntl.h>
#include <string>

#include <json/json.h>

#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"
#include "net_policy_define.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string MONTH_DEFAULT = "M1";

bool NetPolicyFile::FileExists(const std::string& fileName)
{
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == static_cast<int32_t>(NetPolicyResultCode::ERR_NONE));
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

void NetPolicyFile::ParseUidPolicy(const Json::Value &root, NetPolicy &netPolicy)
{
    const Json::Value arrayUidPolicy = root[CONFIG_UID_POLICY];
    uint32_t size = arrayUidPolicy.size();
    UidPolicy uidPolicy;
    for (uint32_t i = 0; i < size; i++) {
        uidPolicy.uid= arrayUidPolicy[i][CONFIG_UID].asString();
        uidPolicy.policy = arrayUidPolicy[i][CONFIG_POLICY].asString();
        netPolicy.uidPolicys.push_back(uidPolicy);
    }
}

void NetPolicyFile::ParseBackgroundPolicy(const Json::Value &root, NetPolicy& netPolicy)
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
        quotaPolicy.simId = arrayQuotaPolicy[i][CONFIG_QUOTA_POLICY_SUBSCRIBERID].asString();
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
        cellularPolicy.simId = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_SUBSCRIBERID].asString();
        cellularPolicy.periodStartTime = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_PERIODSTARTTIME].asString();
        cellularPolicy.periodDuration = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_PERIODDURATION].asString();
        cellularPolicy.title = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_TITLE].asString();
        cellularPolicy.summary = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_SUMMARY].asString();
        cellularPolicy.limitBytes = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_LIMITBYTES].asString();
        cellularPolicy.limitAction = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_LIMITACTION].asString();
        cellularPolicy.usedBytes = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_USEDBYTES].asString();
        cellularPolicy.usedTimeDuration = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_USEDTIMEDURATION].asString();
        cellularPolicy.possessor = arrayCellularPolicy[i][CONFIG_CELLULAR_POLICY_POSSESSOR].asString();
        netPolicy.netCellularPolicys.push_back(cellularPolicy);
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
        quotaPolicy[CONFIG_QUOTA_POLICY_SUBSCRIBERID] = netPolicy_.netQuotaPolicys[i].simId;
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

void NetPolicyFile::AppendCellularPolicy(Json::Value &root)
{
    uint32_t size = netPolicy_.netCellularPolicys.size();
    for (uint32_t i = 0; i < size; i++) {
        Json::Value cellularPolicy;
        cellularPolicy[CONFIG_CELLULAR_POLICY_SUBSCRIBERID] = netPolicy_.netCellularPolicys[i].simId;
        cellularPolicy[CONFIG_CELLULAR_POLICY_PERIODSTARTTIME] =
            netPolicy_.netCellularPolicys[i].periodStartTime;
        cellularPolicy[CONFIG_CELLULAR_POLICY_PERIODDURATION] = netPolicy_.netCellularPolicys[i].periodDuration;
        cellularPolicy[CONFIG_CELLULAR_POLICY_TITLE] = netPolicy_.netCellularPolicys[i].title;
        cellularPolicy[CONFIG_CELLULAR_POLICY_SUMMARY] = netPolicy_.netCellularPolicys[i].summary;
        cellularPolicy[CONFIG_CELLULAR_POLICY_LIMITBYTES] = netPolicy_.netCellularPolicys[i].limitBytes;
        cellularPolicy[CONFIG_CELLULAR_POLICY_LIMITACTION] = netPolicy_.netCellularPolicys[i].limitAction;
        cellularPolicy[CONFIG_CELLULAR_POLICY_USEDBYTES] = netPolicy_.netCellularPolicys[i].usedBytes;
        cellularPolicy[CONFIG_CELLULAR_POLICY_USEDTIMEDURATION] =
            netPolicy_.netCellularPolicys[i].usedTimeDuration;
        cellularPolicy[CONFIG_CELLULAR_POLICY_POSSESSOR] = netPolicy_.netCellularPolicys[i].possessor;
        root[CONFIG_CELLULAR_POLICY].append(cellularPolicy);
    }
}

void NetPolicyFile::AppendUidPolicy(Json::Value &root)
{
    uint32_t size = netPolicy_.uidPolicys.size();
    for (uint32_t i = 0; i < size; i++) {
        Json::Value uidPolicy;
        uidPolicy[CONFIG_UID] = netPolicy_.uidPolicys[i].uid;
        uidPolicy[CONFIG_POLICY] = netPolicy_.uidPolicys[i].policy;
        std::string policy = std::to_string(static_cast<uint32_t>(NetUidPolicy::NET_POLICY_TEMPORARY_ALLOW_METERED));
        /* Temporary permission, no need to write files */
        if (netPolicy_.uidPolicys[i].policy == policy) {
            continue;
        }
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
    // cellular policy
    AppendCellularPolicy(root);
    std::ostringstream out;
    streamWriter->write(root, &out);
    file << out.str().c_str();
    file.close();

    return true;
}

bool NetPolicyFile::WriteFile(NetUidPolicyOpType netUidPolicyOpType, uint32_t uid, NetUidPolicy policy)
{
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
    } else {
        UidPolicy uidPolicy;
        uidPolicy.uid = std::to_string(uid);
        uidPolicy.policy = std::to_string(static_cast<uint32_t>(policy));
        netPolicy_.uidPolicys.push_back(uidPolicy);
    }

    if (!WriteFile(POLICY_FILE_NAME)) {
        NETMGR_LOG_E("WriteFile failed");
        return false;
    }

    return true;
}

bool NetPolicyFile::UpdateQuotaPolicyExist(const NetPolicyQuotaPolicy &quotaPolicy)
{
    if (netPolicy_.netQuotaPolicys.empty()) {
        NETMGR_LOG_E("UpdateQuotaPolicyExist netPolicyQuotaPolicys is empty");
        return false;
    }

    for (uint32_t i = 0; i < netPolicy_.netQuotaPolicys.size(); ++i) {
        if (quotaPolicy.simId_ == netPolicy_.netQuotaPolicys[i].simId &&
            netPolicy_.netQuotaPolicys[i].netType == std::to_string(quotaPolicy.netType_)) {
            netPolicy_.netQuotaPolicys[i].lastLimitSnooze = std::to_string(quotaPolicy.lastLimitSnooze_);
            netPolicy_.netQuotaPolicys[i].limitBytes = std::to_string(quotaPolicy.limitBytes_);
            netPolicy_.netQuotaPolicys[i].metered = std::to_string(quotaPolicy.metered_);
            netPolicy_.netQuotaPolicys[i].netType = std::to_string(quotaPolicy.netType_);
            netPolicy_.netQuotaPolicys[i].periodDuration = quotaPolicy.periodDuration_;
            netPolicy_.netQuotaPolicys[i].periodStartTime = std::to_string(quotaPolicy.periodStartTime_);
            netPolicy_.netQuotaPolicys[i].source = std::to_string(quotaPolicy.source_);
            netPolicy_.netQuotaPolicys[i].simId = quotaPolicy.simId_;
            netPolicy_.netQuotaPolicys[i].warningBytes = std::to_string(quotaPolicy.warningBytes_);
            return true;
        }
    }

    return false;
}

bool NetPolicyFile::WriteFile(const std::vector<NetPolicyQuotaPolicy> &quotaPolicies)
{
    uint32_t vSize = static_cast<uint32_t>(quotaPolicies.size());
    NetPolicyQuota quotaPolicy;
    for (uint32_t i = 0; i < vSize; i++) {
        if (UpdateQuotaPolicyExist(quotaPolicies[i])) {
            continue;
        }
        quotaPolicy.lastLimitSnooze = std::to_string(quotaPolicies[i].lastLimitSnooze_);
        quotaPolicy.limitBytes = std::to_string(quotaPolicies[i].limitBytes_);
        quotaPolicy.metered = std::to_string(quotaPolicies[i].metered_);
        quotaPolicy.netType = std::to_string(quotaPolicies[i].netType_);
        quotaPolicy.periodDuration = quotaPolicies[i].periodDuration_;
        quotaPolicy.periodStartTime = std::to_string(quotaPolicies[i].periodStartTime_);
        quotaPolicy.source = std::to_string(quotaPolicies[i].source_);
        quotaPolicy.simId = quotaPolicies[i].simId_;
        quotaPolicy.warningBytes = std::to_string(quotaPolicies[i].warningBytes_);
        netPolicy_.netQuotaPolicys.push_back(quotaPolicy);
    }

    if (!WriteFile(POLICY_FILE_NAME)) {
        NETMGR_LOG_E("WriteFile failed");
        return false;
    }

    return true;
}

bool NetPolicyFile::UpdateCellularPolicyExist(const NetPolicyCellularPolicy &cellularPolicy)
{
    if (netPolicy_.netCellularPolicys.empty()) {
        NETMGR_LOG_E("UpdateCellularPolicyExist netCellularPolicys is empty");
        return false;
    }

    for (uint32_t i = 0; i < netPolicy_.netCellularPolicys.size(); ++i) {
        if (cellularPolicy.simId_ == netPolicy_.netCellularPolicys[i].simId) {
            netPolicy_.netCellularPolicys[i].simId = cellularPolicy.simId_;
            netPolicy_.netCellularPolicys[i].periodStartTime = std::to_string(cellularPolicy.periodStartTime_);
            netPolicy_.netCellularPolicys[i].periodDuration = cellularPolicy.periodDuration_;
            netPolicy_.netCellularPolicys[i].title = cellularPolicy.title_;
            netPolicy_.netCellularPolicys[i].summary = cellularPolicy.summary_;
            netPolicy_.netCellularPolicys[i].limitBytes = std::to_string(cellularPolicy.limitBytes_);
            netPolicy_.netCellularPolicys[i].limitAction = std::to_string(cellularPolicy.limitAction_);
            netPolicy_.netCellularPolicys[i].usedBytes = std::to_string(cellularPolicy.usedBytes_);
            netPolicy_.netCellularPolicys[i].usedTimeDuration = std::to_string(cellularPolicy.usedTimeDuration_);
            netPolicy_.netCellularPolicys[i].possessor = cellularPolicy.possessor_;
            return true;
        }
    }

    return false;
}

bool NetPolicyFile::WriteFile(const std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    uint32_t vSize = static_cast<uint32_t>(cellularPolicies.size());
    NetPolicyCellular cellularPolicy;
    for (uint32_t i = 0; i < vSize; i++) {
        if (UpdateCellularPolicyExist(cellularPolicies[i])) {
            continue;
        }
        cellularPolicy.simId = cellularPolicies[i].simId_;
        cellularPolicy.periodStartTime = std::to_string(cellularPolicies[i].periodStartTime_);
        cellularPolicy.periodDuration = cellularPolicies[i].periodDuration_;
        cellularPolicy.title = cellularPolicies[i].title_;
        cellularPolicy.summary = cellularPolicies[i].summary_;
        cellularPolicy.limitBytes = std::to_string(cellularPolicies[i].limitBytes_);
        cellularPolicy.limitAction = std::to_string(cellularPolicies[i].limitAction_);
        cellularPolicy.usedBytes = std::to_string(cellularPolicies[i].usedBytes_);
        cellularPolicy.usedTimeDuration = std::to_string(cellularPolicies[i].usedTimeDuration_);
        cellularPolicy.possessor = cellularPolicies[i].possessor_;
        netPolicy_.netCellularPolicys.push_back(cellularPolicy);
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
        if (static_cast<uint32_t>(stol(netPolicy_.uidPolicys[i].uid)) == uid) {
            return true;
        }
    }

    return false;
}

NetUidPolicy NetPolicyFile::GetPolicyByUid(uint32_t uid)
{
    for (auto &uidPolicy : netPolicy_.uidPolicys) {
        if (uidPolicy.uid == std::to_string(uid)) {
            return static_cast<NetUidPolicy>(std::stol(uidPolicy.policy));
        }
    }

    return NetUidPolicy::NET_POLICY_NONE;
}

bool NetPolicyFile::GetUidsByPolicy(NetUidPolicy policy, std::vector<uint32_t> &uids)
{
    for (auto &uidPolicy : netPolicy_.uidPolicys) {
        if (uidPolicy.policy == std::to_string(static_cast<uint32_t>(policy))) {
            uint32_t uid = static_cast<uint32_t>(std::stol(uidPolicy.uid));
            uids.push_back(uid);
        }
    }

    return true;
}

NetPolicyResultCode NetPolicyFile::GetNetQuotaPolicies(std::vector<NetPolicyQuotaPolicy> &quotaPolicies)
{
    NetPolicyQuotaPolicy quotaPolicyTmp;
    for (auto &quotaPolicy : netPolicy_.netQuotaPolicys) {
        quotaPolicyTmp.lastLimitSnooze_ = static_cast<int64_t>(std::stoll(quotaPolicy.lastLimitSnooze));
        quotaPolicyTmp.limitBytes_ = static_cast<int64_t>(std::stoll(quotaPolicy.limitBytes));
        quotaPolicyTmp.metered_ = static_cast<int8_t>(std::stol(quotaPolicy.metered));
        quotaPolicyTmp.netType_ = static_cast<int8_t>(std::stol(quotaPolicy.netType));
        quotaPolicyTmp.periodDuration_ = quotaPolicy.periodDuration;
        quotaPolicyTmp.periodStartTime_ = static_cast<int64_t>(std::stoll(quotaPolicy.periodStartTime));
        quotaPolicyTmp.source_ = static_cast<int8_t>(std::stol(quotaPolicy.source));
        quotaPolicyTmp.simId_ = quotaPolicy.simId;
        quotaPolicyTmp.warningBytes_ = static_cast<int64_t>(std::stoll(quotaPolicy.warningBytes));
        quotaPolicies.push_back(quotaPolicyTmp);
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyFile::GetNetQuotaPolicy(int8_t netType, const std::string &simId,
    NetPolicyQuotaPolicy &quotaPolicy)
{
    for (auto &quotaPolicyTemp : netPolicy_.netQuotaPolicys) {
        if (netType == static_cast<int8_t>(std::stol(quotaPolicyTemp.netType)) &&
            simId == quotaPolicyTemp.simId) {
            quotaPolicy.lastLimitSnooze_ = static_cast<int64_t>(std::stoll(quotaPolicyTemp.lastLimitSnooze));
            quotaPolicy.limitBytes_ = static_cast<int64_t>(std::stoll(quotaPolicyTemp.limitBytes));
            quotaPolicy.metered_ = static_cast<int8_t>(std::stol(quotaPolicyTemp.metered));
            quotaPolicy.netType_ = static_cast<int8_t>(std::stol(quotaPolicyTemp.netType));
            quotaPolicy.periodDuration_ = quotaPolicyTemp.periodDuration;
            quotaPolicy.periodStartTime_ = static_cast<int64_t>(std::stoll(quotaPolicyTemp.periodStartTime));
            quotaPolicy.source_ = static_cast<int8_t>(std::stol(quotaPolicyTemp.source));
            quotaPolicy.simId_ = quotaPolicyTemp.simId;
            quotaPolicy.warningBytes_ = static_cast<int64_t>(std::stoll(quotaPolicyTemp.warningBytes));
            return NetPolicyResultCode::ERR_NONE;
        }
    }

    return NetPolicyResultCode::ERR_QUOTA_POLICY_NOT_EXIST;
}

bool NetPolicyFile::IsInterfaceMetered(const std::string &ifaceName)
{
    for (auto &quotaPolicy : netPolicy_.netQuotaPolicys) {
        int8_t netType = static_cast<int8_t>(std::stol(quotaPolicy.netType));
        NetBearType bearerType = static_cast<NetBearType>(netType);
        std::string policyIfaceName;
        int32_t ret = NetManagerCenter::GetInstance().GetIfaceNameByType(bearerType,
            IDENT_PREFIX, policyIfaceName);
        if (ret != 0 || policyIfaceName.empty()) {
            continue;
        }
        if (ifaceName.compare(policyIfaceName) == 0) {
            return static_cast<bool>(std::stol(quotaPolicy.metered));
        }
    }
    return false;
}

NetPolicyResultCode NetPolicyFile::GetCellularPolicies(std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    NetPolicyCellularPolicy cellularPolicyTmp;
    for (auto &cellularPolicy : netPolicy_.netCellularPolicys) {
        cellularPolicyTmp.simId_ = cellularPolicy.simId;
        cellularPolicyTmp.periodStartTime_ = static_cast<int64_t>(std::stoll(cellularPolicy.periodStartTime));
        cellularPolicyTmp.periodDuration_ = cellularPolicy.periodDuration;
        cellularPolicyTmp.title_ = cellularPolicy.title;
        cellularPolicyTmp.summary_ = cellularPolicy.summary;
        cellularPolicyTmp.limitBytes_ = static_cast<int64_t>(std::stoll(cellularPolicy.limitBytes));
        cellularPolicyTmp.limitAction_ = static_cast<int32_t>(std::stol(cellularPolicy.limitAction));
        cellularPolicyTmp.usedBytes_ = static_cast<int64_t>(std::stoll(cellularPolicy.usedBytes));
        cellularPolicyTmp.usedTimeDuration_ = static_cast<int64_t>(std::stoll(cellularPolicy.usedTimeDuration));
        cellularPolicyTmp.possessor_ = cellularPolicy.possessor;
        cellularPolicies.push_back(cellularPolicyTmp);
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyFile::SetFactoryPolicy(const std::string &simId)
{
    netPolicy_.uidPolicys.clear();
    netPolicy_.backgroundPolicyStatus_ = BACKGROUND_POLICY_ALLOW;

    if (simId.empty()) {
        netPolicy_.netQuotaPolicys.clear();
        netPolicy_.netCellularPolicys.clear();
    } else {
        for (auto iter = netPolicy_.netQuotaPolicys.begin(); iter != netPolicy_.netQuotaPolicys.end(); ++iter) {
            if (simId == iter->simId) {
                netPolicy_.netQuotaPolicys.erase(iter);
                break;
            }
        }

        for (auto iter = netPolicy_.netCellularPolicys.begin(); iter != netPolicy_.netCellularPolicys.end(); ++iter) {
            if (simId == iter->simId) {
                netPolicy_.netCellularPolicys.erase(iter);
                break;
            }
        }
    }

    if (!WriteFile(POLICY_FILE_NAME)) {
        NETMGR_LOG_E("WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyFile::SetBackgroundPolicy(bool backgroundPolicy)
{
    if (backgroundPolicy) {
        netPolicy_.backgroundPolicyStatus_ = BACKGROUND_POLICY_ALLOW;
    } else {
        netPolicy_.backgroundPolicyStatus_ = BACKGROUND_POLICY_REJECT;
    }

    if (!WriteFile(POLICY_FILE_NAME)) {
        NETMGR_LOG_E("WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

bool NetPolicyFile::GetBackgroundPolicy()
{
    if (netPolicy_.backgroundPolicyStatus_.compare(BACKGROUND_POLICY_ALLOW) == 0) {
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
            NETMGR_LOG_D("CreateFile [%{public}s] failed", POLICY_FILE_NAME.c_str());
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
