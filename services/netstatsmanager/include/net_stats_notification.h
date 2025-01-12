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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NET_STATS_NOTIFICATION_H
#define NET_STATS_NOTIFICATION_H

#include <mutex>
#include <string>

#include "image_source.h"
#include "pixel_map.h"
#include "notification_helper.h"
#include "number_format.h"

static const int32_t COMM_NETSYS_NATIVE_SYS_ABILITY_ID = 1158;

const std::string NETMGR_ENHANCED_SYS_ABILITY_NAME = "netmanager";
const std::string NET_ACC_HEADER_NOTIFICATION = "ohos.event.notification.netmgr.NETWORK_ACCELERATION";
const std::string NEVER_REMIND_NOTIFICATION = "ohos.event.notification.netmgr.NEVER_REMIND";
const std::string START_ABILITY_NOTIFICATION = "ohos.event.notification.netmgr.START_ABILITY";
const std::string SWITCH_BACK_NOTIFICATION = "ohos.event.notification.netmgr.SWITCH_BACK";
const std::string SWITCH_BACK_FEATURE_TYPE = "swtich_back_feature_type";

typedef void (*NetMgrStatsLimitNtfCallback)(int notificationId);

enum NetMgrStatsLimitNotificationId : int {
    NETMGR_STATS_LIMIT_DAY = 115800,
    NETMGR_STATS_LIMIT_MONTH = 115801,
    NETMGR_STATS_ALERT_MONTH = 115802,
};
namespace OHOS {
namespace NetManagerStandard {
class NetMgrNetStatsLimitNotification {

public:
    static NetMgrNetStatsLimitNotification &GetInstance(void);

    void PublishNetStatsLimitNotification(int notificationId, bool isDaulCard);
    void RegNotificationCallback(NetMgrStatsLimitNtfCallback callback);

private:
    NetMgrNetStatsLimitNotification();
    ~NetMgrNetStatsLimitNotification();
    NetMgrNetStatsLimitNotification(const NetMgrNetStatsLimitNotification&) = delete;
    NetMgrNetStatsLimitNotification &operator=(const NetMgrNetStatsLimitNotification&) = delete;

    void GetPixelMap();
    void UpdateResourceMap();
    void ParseJSONFile(const std::string& filePath, std::map<std::string, std::string>& container);
    std::unique_ptr<OHOS::Global::I18n::NumberFormat> GetNumberFormatter();

    std::string GetDayNotificationText();
    std::string GetMonthNotificationText();
    std::string GetMonthAlertText();
    std::string GetNotificationTile(std::string &notificationType);
    void SetMonthWarningActionButton(Notification::NotificationRequest &request);
    void SetDayWarningActionButton(Notification::NotificationRequest &request);
    void SetNotificationWant(int notificationId, Notification::NotificationRequest &request);
    bool SetTitleAndText(int notificationId,
        std::shared_ptr<Notification::NotificationNormalContent> content, bool isDaulCard);
    std::string GetTrafficNum(uint64_t traffic);

    std::shared_ptr<Media::PixelMap> netmgrStatsLimitIconPixelMap_  {};
    std::map<std::string, std::string> g_resourceMap;
    std::map<std::string, std::string> g_languageMap;
    std::string g_localeBaseName;
    std::mutex mutex_ {};
    const int NETMGR_TRAFFIC_NTF_CONTROL_FLAG = 0;

    // std::unique_ptr<NetMgrEnhancedEventSubscriberManager> netMgrEnhancedEventSubscriberManager = nullptr;
};
}  // namespace NetManagerStandard
}  // namespace OHOS
#endif  // NETMGR_LIMIT_NOTIFICATION_H
