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

#include "net_access_policy_config.h"

#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <vector>

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetAccessPolicyConfigUtils NetAccessPolicyConfigUtils::instance_;
namespace {
const char *PATH = "/system/variant/phone/base/etc/netmanager/net_access_policy_config.json";
const char *ARRAY_NAME = "configs";
const char *ITEM_BUNDLE_NAME = "bundleName";
const char *ITEM_DISABLE_WLAN_SWITCH = "disableWlanSwitch";
const char *ITEM_DISABLE_CELLULAR_SWITCH = "disableCellularSwitch";

bool CheckFilePath(const std::string &fileName, std::string &realPath)
{
    char tmpPath[PATH_MAX] = {0};
    if (!realpath(fileName.c_str(), tmpPath)) {
        NETMGR_LOG_E("file name is illegal");
        return false;
    }
    if (strcmp(tmpPath, PATH) != 0) {
        NETMGR_LOG_E("file path is illegal");
        return false;
    }
    realPath = tmpPath;
    return true;
}
} // namespace
NetAccessPolicyConfigUtils &NetAccessPolicyConfigUtils::GetInstance()
{
    return instance_;
}
std::vector<NetAccessPolicyConfig> NetAccessPolicyConfigUtils::GetNetAccessPolicyConfig()
{
    if (!isInit_) {
        Init();
    }
    return netAccessPolicyConfigs_;
}

void NetAccessPolicyConfigUtils::Init()
{
    std::lock_guard<ffrt::mutex> lock(lock_);
    if (isInit_) {
        return;
    }
    netAccessPolicyConfigs_.clear();
    ParseNetAccessPolicyConfigs();
    isInit_ = true;
}
void NetAccessPolicyConfigUtils::ParseNetAccessPolicyConfigs()
{
    std::string content;
    if (!ReadFile(content, PATH)) {
        NETMGR_LOG_E("read json file failed.");
        return;
    }
    if (content.empty()) {
        NETMGR_LOG_E("read content is empty.");
        return;
    }
    cJSON *root = cJSON_Parse(content.c_str());
    if (root == nullptr) {
        NETMGR_LOG_E("json root is nullptr.");
        return;
    }
    cJSON *configs = cJSON_GetObjectItem(root, ARRAY_NAME);
    if (configs == nullptr || !cJSON_IsArray(configs) || cJSON_GetArraySize(configs) == 0) {
        cJSON_Delete(root);
        configs = nullptr;
        root = nullptr;
        return;
    }

    cJSON *item = nullptr;
    for (int i = 0; i < cJSON_GetArraySize(configs); i++) {
        item = cJSON_GetArrayItem(configs, i);
        if (item == nullptr) {
            NETMGR_LOG_E("config item is nullptr.");
            continue;
        }
        NetAccessPolicyConfig tmp;
        tmp.bundleName = ParseString(cJSON_GetObjectItem(item, ITEM_BUNDLE_NAME));
        tmp.disableWlanSwitch = ParseBoolean(cJSON_GetObjectItem(item, ITEM_DISABLE_WLAN_SWITCH));
        tmp.disableCellularSwitch = ParseBoolean(cJSON_GetObjectItem(item, ITEM_DISABLE_CELLULAR_SWITCH));
        netAccessPolicyConfigs_.push_back(tmp);
    }

    cJSON_Delete(root);
    configs = nullptr;
    root = nullptr;
}

bool NetAccessPolicyConfigUtils::ReadFile(std::string &content, const std::string &fileName)
{
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
    content = buffer.str();
    file.close();
    return true;
}

std::string NetAccessPolicyConfigUtils::ParseString(cJSON *value)
{
    if (cJSON_IsString(value) && value->valuestring != nullptr) {
        return value->valuestring;
    }
    return "";
}

bool NetAccessPolicyConfigUtils::ParseBoolean(cJSON *value)
{
    if (cJSON_IsBool(value)) {
        return cJSON_IsTrue(value);
    }
    return false;
}

int32_t NetAccessPolicyConfigUtils::ParseInt32(cJSON *value)
{
    if (cJSON_IsNumber(value)) {
        return value->valueint;
    }
    return false;
}
} // namespace NetManagerStandard
} // namespace OHOS