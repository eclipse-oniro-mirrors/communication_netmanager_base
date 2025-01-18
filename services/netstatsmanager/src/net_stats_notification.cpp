/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHstrNum WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "net_stats_notification.h"
#include "net_stats_service.h"

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>

#include "cJSON.h"
#include "file_ex.h"
#include "locale_config.h"
#include "locale_info.h"
#include "locale_matcher.h"
#include "securec.h"
#include "want_agent_helper.h"
#include "want_agent_info.h"

#include "image_source.h"
#include "pixel_map.h"
#include "notification_normal_content.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_utils.h"
#include "core_service_client.h"

namespace OHOS {
namespace NetManagerStandard {

static const int NTF_AUTO_DELETE_TIME = 10000;

// static const int COMM_NETSYS_NATIVE_SYS_ABILITY_ID = 1158

static const int32_t SLOT_ID_00 = 0;
static const int32_t CARD_ID_01 = 1;
static const int32_t CARD_ID_02 = 2;

static const int32_t UNIT_CONVERT_1024 = 1024;
static const int32_t TWO_PRECISION = 2;

static const int32_t TWO_CHAR = 2;

// keys in json file
static constexpr const char *KEY_STRING = "string";
static constexpr const char *KEY_NAME = "name";
static constexpr const char *KEY_VALUE = "value";
static constexpr const char *KEY_NETWORK_MONTH_LIMIT_SIMGLE_TITLE = "netstats_excess_monthlimit_notofication_title";
static constexpr const char *KEY_NETWORK_MONTH_MARK_SIMGLE_TITLE = "netstats_excess_monthmark_notofication_title";
static constexpr const char *KEY_NETWORK_DAY_MARK_SIMGLE_TITLE = "netstats_excess_daymark_notofication_title";

static constexpr const char *KEY_NETWORK_MONTH_LIMIT_DUAL_TITLE = "netstats_excess_monthlimit_notofication_title_sub";
static constexpr const char *KEY_NETWORK_MONTH_MARK_DUAL_TITLE = "netstats_excess_monthmark_notofication_title_sub";
static constexpr const char *KEY_NETWORK_DAY_MARK_DUAL_TITLE = "netstats_excess_daymark_notofication_title_sub";

static constexpr const char *KEY_MONTH_LIMIT_TEXT = "netstats_month_limit_message";
static constexpr const char *KEY_MONTH_NOTIFY_TEXT = "netstats_month_notify_message";
static constexpr const char *KEY_DAILY_NOTIFY_TEXT = "netstats_daily_notify_message";

// NOTE: icon and json path must be absolute path
// all locales are listed at: global_i18n-master\global_i18n-master\frameworks\intl\etc\supported_locales.xml
static constexpr const char *NETWORK_ICON_PATH = "//system/etc/netmanager_base/resources/network_ic.png";
static constexpr const char *DEFAULT_LANGUAGE_NAME_EN = "base";
static constexpr const char *LOCALE_TO_RESOURCE_PATH =
    "//system/etc/netmanager_base/resources/locale_to_resourcePath.json";
static constexpr const char *LANGUAGE_RESOURCE_PARENT_PATH =
    "//system/etc/netmanager_base/resources/";
static constexpr const char *LANGUAGE_RESOURCE_CHILD_PATH = "/element/string.json";

static std::mutex g_callbackMutex {};
static NetMgrStatsLimitNtfCallback g_NetMgrStatsLimitNtfCallback = nullptr;

void NetMgrNetStatsLimitNotification::ParseJSONFile(
    const std::string& filePath, std::map<std::string, std::string>& container)
{
    std::string content;
    LoadStringFromFile(filePath, content);

    cJSON *json = cJSON_Parse(content.c_str());
    if (json == nullptr) {
        NETMGR_LOG_I("ParseJSONFile: json null. filepath = %{public}s", filePath.c_str());
        return;
    }

    cJSON *resJson = cJSON_GetObjectItemCaseSensitive(json, KEY_STRING);

    if (resJson == nullptr) {
        NETMGR_LOG_I("ParseJSONFile: resJson null. filepath = %{public}s", filePath.c_str());
    } else {
        container.clear();
        cJSON *resJsonEach = nullptr;
        cJSON_ArrayForEach(resJsonEach, resJson) {
            cJSON *key = cJSON_GetObjectItemCaseSensitive(resJsonEach, KEY_NAME);
            cJSON *value = cJSON_GetObjectItemCaseSensitive(resJsonEach, KEY_VALUE);
            container.insert(std::pair<std::string, std::string>(key->valuestring, value->valuestring));
        }
    }

    cJSON_Delete(json);
}

void NetMgrNetStatsLimitNotification::UpdateResourceMap()
{
    OHOS::Global::I18n::LocaleInfo locale(Global::I18n::LocaleConfig::GetSystemLocale());
    std::string curBaseName = locale.GetBaseName();
    if (localeBaseName == curBaseName) {
        return;
    }

    NETMGR_LOG_I("UpdateResourceMap: change from %{public}s to %{public}s",
        localeBaseName.c_str(), curBaseName.c_str());
    localeBaseName = curBaseName;

    std::string languagePath = DEFAULT_LANGUAGE_NAME_EN;
    if (languageMap.find(localeBaseName) != languageMap.end()) {
        languagePath = languageMap[localeBaseName];
    } else {
        for (auto& eachPair : languageMap) {
            OHOS::Global::I18n::LocaleInfo eachLocale(eachPair.first);
            if (OHOS::Global::I18n::LocaleMatcher::Match(&locale, &eachLocale)) {
                languagePath = eachPair.second;
                break;
            }
        }
    }

    std::string resourcePath = LANGUAGE_RESOURCE_PARENT_PATH + languagePath + LANGUAGE_RESOURCE_CHILD_PATH;
    NETMGR_LOG_I("UpdateResourceMap: resourcePath = %{public}s", resourcePath.c_str());
    if (!std::filesystem::exists(resourcePath)) {
        NETMGR_LOG_E("resource path not exist: %{public}s", resourcePath.c_str());
        return;
    }
    /* 从resourcePath中拿到resourceMap */
    ParseJSONFile(resourcePath, resourceMap);
}

std::string NetMgrNetStatsLimitNotification::GetDayNotificationText()
{
    NETMGR_LOG_I("start NetMgrNetStatsLimitNotification::GetDayNotificationText");
    int32_t simId = DelayedSingleton<NetStatsService>::GetInstance()->GetCurActiviteSimId();
    auto settingsObserverMap_ = DelayedSingleton<NetStatsService>::GetInstance()->GetSettingsObserverMap();
    if (settingsObserverMap_.find(simId) == settingsObserverMap_.end()) {
        NETMGR_LOG_I("settingsObserverMap_ has no simId key:: simId %{public}d", simId);
        return "";
    }

    std::string outText = resourceMap[KEY_DAILY_NOTIFY_TEXT];
    NETMGR_LOG_I("NetMgrNetStatsLimitNotification:: simId [%{public}d]", simId);
    if (outText.find("%s") == std::string::npos) {
        NETMGR_LOG_I("incorrect format [%{public}s]", outText.c_str());
        return "";
    }
    uint64_t traffic = settingsObserverMap_[simId].second->monthlyLimit;
    double dailyTraffic = traffic / 100.0 * settingsObserverMap_[simId].second->dailyMark;
    std::string num = GetTrafficNum(dailyTraffic);
    outText = outText.replace(outText.find("%s"), TWO_CHAR, num);
    NETMGR_LOG_I("start NetMgrNetStatsLimitNotification::outText [%{public}s]", outText.c_str());
    return outText;
}

std::string NetMgrNetStatsLimitNotification::GetMonthNotificationText()
{
    std::string outText;
    
    int32_t simId = DelayedSingleton<NetStatsService>::GetInstance()->GetCurActiviteSimId();

    auto settingsObserverMap_ = DelayedSingleton<NetStatsService>::GetInstance()->GetSettingsObserverMap();
    if (settingsObserverMap_.find(simId) == settingsObserverMap_.end()) {
        NETMGR_LOG_I("settingsObserverMap_ has no simId key:: simId %{public}d", simId);
        return "";
    }

    int32_t monUsedPercent = settingsObserverMap_[simId].second->monthlyMark;
    
    outText = resourceMap[KEY_MONTH_NOTIFY_TEXT];
    if (outText.find("%s") == std::string::npos) {
        NETMGR_LOG_I("incorrect format [%{public}s]", outText.c_str());
        return "";
    }
    outText = outText.replace(outText.find("%s"), TWO_CHAR, std::to_string(monUsedPercent) + "%");
    NETMGR_LOG_I("GetMonthNotificationText outText [%{public}s]", outText.c_str());
    return outText;
}

std::string NetMgrNetStatsLimitNotification::GetMonthAlertText()
{
    std::string outText;
    
    int32_t simId = DelayedSingleton<NetStatsService>::GetInstance()->GetCurActiviteSimId();

    auto settingsObserverMap_ = DelayedSingleton<NetStatsService>::GetInstance()->GetSettingsObserverMap();
    if (settingsObserverMap_.find(simId) == settingsObserverMap_.end()) {
        NETMGR_LOG_I("settingsObserverMap_ has no simId key:: simId %{public}d", simId);
        return "";
    }

    // 检验simId是否合法
    outText = resourceMap[KEY_MONTH_LIMIT_TEXT];
    if (outText.find("%s") == std::string::npos) {
        NETMGR_LOG_I("incorrect format [%{public}s]", outText.c_str());
        return "";
    }
    std::string num = GetTrafficNum(settingsObserverMap_[simId].second->monthlyLimit);
    outText = outText.replace(outText.find("%s"), TWO_CHAR, num);
    return outText;
}

std::string NetMgrNetStatsLimitNotification::GetNotificationTile(std::string &notificationType)
{
    NETMGR_LOG_I("start NetMgrNetStatsLimitNotification::GetNotificationTile");
    std::string outText;
    int32_t simId = DelayedSingleton<NetStatsService>::GetInstance()->GetCurActiviteSimId();

    outText = resourceMap[notificationType];
    
    if (outText.find("%d") == std::string::npos) {
        NETMGR_LOG_I("incorrect format %{public}s", outText.c_str());
        return "";
    }
    int32_t slotId = Telephony::CoreServiceClient::GetInstance().GetSlotId(simId);
    NETMGR_LOG_I("GetNotificationTile. simId:%{public}d, slotId:%{public}d", simId, slotId);
    outText = outText.replace(outText.find("%d"), TWO_CHAR, std::to_string(slotId + 1));
    return outText;
}

bool NetMgrNetStatsLimitNotification::SetTitleAndText(
    int notificationId,
    std::shared_ptr<Notification::NotificationNormalContent> content,
    bool isDaulCard)
{
    NETMGR_LOG_I("start NetMgrNetStatsLimitNotification::SetTitleAndText");
    std::string strText;
    if (content == nullptr) {
        NETMGR_LOG_I("content is null");
        return false;
    }

    std::string title = "";
    if (isDaulCard) {
        title = (notificationId == NETMGR_STATS_LIMIT_DAY) ? KEY_NETWORK_DAY_MARK_DUAL_TITLE : title;
        title = (notificationId == NETMGR_STATS_LIMIT_MONTH) ? KEY_NETWORK_MONTH_MARK_DUAL_TITLE : title;
        title = (notificationId == NETMGR_STATS_ALERT_MONTH) ? KEY_NETWORK_MONTH_LIMIT_DUAL_TITLE : title;
    } else {
        title = (notificationId == NETMGR_STATS_LIMIT_DAY) ? KEY_NETWORK_DAY_MARK_SIMGLE_TITLE : title;
        title = (notificationId == NETMGR_STATS_LIMIT_MONTH) ? KEY_NETWORK_MONTH_MARK_SIMGLE_TITLE : title;
        title = (notificationId == NETMGR_STATS_ALERT_MONTH) ? KEY_NETWORK_MONTH_LIMIT_SIMGLE_TITLE : title;
    }

    if (resourceMap.find(title) == resourceMap.end()) {
        NETMGR_LOG_I("cannot get title from resources");
        return false;
    }
    std::string strTitle;
    if (isDaulCard) {
        strTitle = GetNotificationTile(title);
    } else {
        strTitle = resourceMap[title];
    }

    NETMGR_LOG_I("NetMgrNetStatsLimitNotification: strTitle = %{public}s", strTitle.c_str());

    switch (notificationId) {
        case NETMGR_STATS_LIMIT_DAY:
            strText = GetDayNotificationText();
            break;
        case NETMGR_STATS_LIMIT_MONTH:
            strText = GetMonthNotificationText();
            break;
        case NETMGR_STATS_ALERT_MONTH:
            strText = GetMonthAlertText();
            break;
        default:
            NETMGR_LOG_I("unknown notification ID");
            return false;
    }
    content->SetText(strText);
    content->SetTitle(strTitle);
    NETMGR_LOG_I("end NetMgrNetStatsLimitNotification::SetTitleAndText");
    return true;
}

void NetMgrNetStatsLimitNotification::GetPixelMap()
{
    if (netmgrStatsLimitIconPixelMap_ != nullptr) {
        return;
    }

    if (!std::filesystem::exists(NETWORK_ICON_PATH)) {
        return;
    }

    uint32_t errorCode = 0;
    Media::SourceOptions opts;
    opts.formatHint = "image/png";
    auto imageSource = Media::ImageSource::CreateImageSource(NETWORK_ICON_PATH, opts, errorCode);
    if (imageSource == nullptr) {
        NETMGR_LOG_I("CreateImageSource null");
        return;
    }
    Media::DecodeOptions decodeOpts;
    std::unique_ptr<Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    netmgrStatsLimitIconPixelMap_ = std::move(pixelMap);
}

NetMgrNetStatsLimitNotification& NetMgrNetStatsLimitNotification::GetInstance()
{
    static NetMgrNetStatsLimitNotification instance;
    return instance;
}

NetMgrNetStatsLimitNotification::NetMgrNetStatsLimitNotification()
{
    std::lock_guard<std::mutex> lock(mutex_);
    NETMGR_LOG_I("start NetMgrNetStatsLimitNotification");
    ParseJSONFile(LOCALE_TO_RESOURCE_PATH, languageMap);
    UpdateResourceMap();
    GetPixelMap();
    NETMGR_LOG_I("end NetMgrNetStatsLimitNotification");
}

NetMgrNetStatsLimitNotification::~NetMgrNetStatsLimitNotification()
{
    NETMGR_LOG_I("NetMgr Notification destructor enter.");
}

void NetMgrNetStatsLimitNotification::PublishNetStatsLimitNotification(int notificationId, bool isDaulCard)
{
    std::lock_guard<std::mutex> lock(mutex_);
    NETMGR_LOG_I("PublishNetMgrNetStatsLimitNotification: id = %{public}d", notificationId);
    UpdateResourceMap();
    std::shared_ptr<Notification::NotificationNormalContent> notificationContent =
        std::make_shared<Notification::NotificationNormalContent>();
    if (notificationContent == nullptr) {
        NETMGR_LOG_I("get notification content nullptr");
        return;
    }
    if (!SetTitleAndText(notificationId, notificationContent, isDaulCard)) {
        NETMGR_LOG_I("error setting title and text");
        return;
    }

    std::shared_ptr<Notification::NotificationContent> content =
        std::make_shared<Notification::NotificationContent>(notificationContent);
    if (content == nullptr) {
        NETMGR_LOG_I("get notification content nullptr");
        return;
    }

    Notification::NotificationRequest request;
    request.SetNotificationId(static_cast<int32_t>(notificationId));
    request.SetContent(content);

    request.SetCreatorUid(COMM_NETSYS_NATIVE_SYS_ABILITY_ID);
    request.SetAutoDeletedTime(NTF_AUTO_DELETE_TIME);
    request.SetTapDismissed(true);
    request.SetSlotType(OHOS::Notification::NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    request.SetNotificationControlFlags(NETMGR_TRAFFIC_NTF_CONTROL_FLAG);

    request.SetLittleIcon(netmgrStatsLimitIconPixelMap_);

    int ret = Notification::NotificationHelper::PublishNotification(request);
    NETMGR_LOG_I("publish notification result = %{public}d", ret);

    auto settingsObserverMap_ = DelayedSingleton<NetStatsService>::GetInstance()->GetSettingsObserverMap();
}

void NetMgrNetStatsLimitNotification::RegNotificationCallback(NetMgrStatsLimitNtfCallback callback)
{
    std::lock_guard<std::mutex> lock(g_callbackMutex);
    g_NetMgrStatsLimitNtfCallback = callback;
}

std::string NetMgrNetStatsLimitNotification::GetTrafficNum(double traffic)
{
    const char* units[] = {"B", "KB", "MB", "GB", "TB"}; // 流量单位数组
    int record = 0; // 记录单位索引
 
    // 确定应该使用的单位
    while (traffic >= UNIT_CONVERT_1024 && record < 4) { // 4: 防止units数组越界
        traffic /= UNIT_CONVERT_1024;
        record++;
    }
 
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << traffic << " " << units[record];  // 2: 保留两位小数
    // 返回格式化后的字符串
    return oss.str();
}
}  // namespace NetManagerStandard
}  // namespace OHOS
