/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#include <cinttypes>
#include "net_stats_settings_observer.h"
#include "net_datashare_utils.h"
#include "net_mgr_log_wrapper.h"
#include "net_manager_constants.h"
#include "net_stats_service.h"
#include "net_stats_client.h"
#include "net_stats_utils.h"
namespace OHOS {
namespace NetManagerStandard {

static constexpr const char *CELLULAR_DATA_SETTING_DATA_ENABLE_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=cellular_data_enable";

static constexpr const char *SETTING_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=";

const std::string UNLIMITED_TRAFFIC_ENABLE  = "unlimited_traffic_enable";
const std::string MONTHLY_LIMITED_TRAFFIC = "monthly_limited_traffic";
const std::string MONTHLY_BEGIN_DATE = "monthly_start_date";
const std::string MONTHLY_NOTIFY_TYPE = "monthly_notify_type";
const std::string OVER_MONTHLY_MARK = "over_monthly_mark";
const std::string OVER_DAILY_MARK = "over_daily_mark";
const std::string TAG_NAME = "net_stats_";

TrafficDataObserver::TrafficDataObserver(int32_t simId)
{
    NETMGR_LOG_I("TrafficDataObserver start. simId: %{public}d", simId);
    simId_ = simId;
    mUnlimitTrafficEnableObserver_ = std::make_unique<UnlimitTrafficEnableObserver>(simId).release();
    mTrafficMonthlyValueObserver_ = std::make_unique<TrafficMonthlyValueObserver>(simId).release();
    mTrafficMonthlyBeginDateObserver_ = std::make_unique<TrafficMonthlyBeginDateObserver>(simId).release();
    mTrafficMonthlyNotifyTypeObserver_ = std::make_unique<TrafficMonthlyNotifyTypeObserver>(simId).release();
    mTrafficMonthlyMarkObserver_ = std::make_unique<TrafficMonthlyMarkObserver>(simId).release();
    mTrafficDailyMarkObserver_ = std::make_unique<TrafficDailyMarkObserver>(simId).release();
    mCellularDataObserver_ = std::make_unique<CellularDataObserver>().release();
}

void TrafficDataObserver::RegisterTrafficDataSettingObserver()
{
    NETMGR_LOG_E("RegisterTrafficDataSettingObserver start.");
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri uri1(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    if (dataShareHelperUtils->RegisterSettingsObserver(uri1, mCellularDataObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("register mCellularDataObserver_ failed.");
    }
    
    Uri uri2(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + UNLIMITED_TRAFFIC_ENABLE);
    if (dataShareHelperUtils->RegisterSettingsObserver(uri2, mUnlimitTrafficEnableObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("register mUnlimitTrafficEnableObserver_ failed.");
    }

    Uri uri3(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_LIMITED_TRAFFIC);
    if (dataShareHelperUtils->RegisterSettingsObserver(uri3, mTrafficMonthlyValueObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("register mTrafficMonthlyValueObserver_ failed.");
    }

    Uri uri4(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_BEGIN_DATE);
    if (dataShareHelperUtils->RegisterSettingsObserver(uri4, mTrafficMonthlyBeginDateObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("register mTrafficMonthlyValueObserver_ failed.");
    }

    Uri uri5(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_NOTIFY_TYPE);
    if (dataShareHelperUtils->RegisterSettingsObserver(uri5, mTrafficMonthlyNotifyTypeObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("register mTrafficMonthlyNotifyTypeObserver_ failed.");
    }

    Uri uri6(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + OVER_MONTHLY_MARK);
    if (dataShareHelperUtils->RegisterSettingsObserver(uri6, mTrafficMonthlyMarkObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("register mTrafficMonthlyMarkObserver_ failed.");
    }

    Uri uri7(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + OVER_DAILY_MARK);
    if (dataShareHelperUtils->RegisterSettingsObserver(uri7, mTrafficDailyMarkObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("register mTrafficDailyMarkObserver_ failed.");
    }
    NETMGR_LOG_E("RegisterTrafficDataSettingObserver end.");
}

void TrafficDataObserver::UnRegisterTrafficDataSettingObserver()
{
    NETMGR_LOG_E("UnRegisterTrafficDataSettingObserver start.");
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri uri1(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    if (dataShareHelperUtils->UnRegisterSettingsObserver(uri1, mCellularDataObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("unregister mCellularDataObserver_ failed.");
    }
    Uri uri2(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + UNLIMITED_TRAFFIC_ENABLE);
    if (dataShareHelperUtils->UnRegisterSettingsObserver(uri2, mUnlimitTrafficEnableObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("unregister mUnlimitTrafficEnableObserver_ failed.");
    }
    Uri uri3(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_LIMITED_TRAFFIC);
    if (dataShareHelperUtils->UnRegisterSettingsObserver(uri3, mTrafficMonthlyValueObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("unregister mTrafficMonthlyValueObserver_ failed.");
    }
    Uri uri4(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_BEGIN_DATE);
    if (dataShareHelperUtils->UnRegisterSettingsObserver(uri4, mTrafficMonthlyBeginDateObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("unregister mTrafficMonthlyValueObserver_ failed.");
    }
    Uri uri5(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_NOTIFY_TYPE);
    if (dataShareHelperUtils->UnRegisterSettingsObserver(uri5, mTrafficMonthlyNotifyTypeObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("unregister mTrafficMonthlyNotifyTypeObserver_ failed.");
    }
    Uri uri6(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + OVER_MONTHLY_MARK);
    if (dataShareHelperUtils->UnRegisterSettingsObserver(uri6, mTrafficMonthlyMarkObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("unregister mTrafficMonthlyMarkObserver_ failed.");
    }
    Uri uri7(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + OVER_DAILY_MARK);
    if (dataShareHelperUtils->UnRegisterSettingsObserver(uri7, mTrafficDailyMarkObserver_) != NETSYS_SUCCESS) {
        NETMGR_LOG_E("unregister mTrafficDailyMarkObserver_ failed.");
    }
    NETMGR_LOG_E("UnRegisterTrafficDataSettingObserver end.");
}

void TrafficDataObserver::ReadTrafficDataSettings(std::shared_ptr<TrafficSettingsInfo> info)
{
    NETMGR_LOG_E("ReadTrafficDataSettings start.");
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    // 读取流量设置
    Uri unLimitUri(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + UNLIMITED_TRAFFIC_ENABLE);
    std::string value = "";
    dataShareHelperUtils->Query(unLimitUri, TAG_NAME + std::to_string(simId_) + "_" + UNLIMITED_TRAFFIC_ENABLE, value);
    info->unLimitedDataEnable = 0;
    int32_t enable = 0;
    if (!value.empty() && NetStatsUtils::ConvertToInt32(value, enable)) {
        info->unLimitedDataEnable = enable;
    }

    Uri mLimitUri(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_LIMITED_TRAFFIC);
    value = "";
    dataShareHelperUtils->Query(mLimitUri, TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_LIMITED_TRAFFIC, value);
    info->monthlyLimit = UINT64_MAX;
    uint64_t trafficInt = 0;
    if (!value.empty() && NetStatsUtils::ConvertToUint64(value, trafficInt)) {
        info->monthlyLimit = trafficInt;
    }

    Uri beginTimeUri(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_BEGIN_DATE);
    value = "";
    dataShareHelperUtils->Query(beginTimeUri, TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_BEGIN_DATE, value);
    info->beginDate = 1;
    int32_t dateInt = 0;
    if (!value.empty() && NetStatsUtils::ConvertToInt32(value, dateInt)) {
        info->beginDate = dateInt;
    }

    ReadTrafficDataSettingsPart2(info);
    NETMGR_LOG_I("ReadTrafficDataSettings beginDate:%{public}d, unLimitedDataEnable:%{public}d,\
monthlyLimitdNotifyType:%{public}d, monthlyLimit:%{public}" PRIu64 ", monthlyMark:%{public}u, dailyMark:%{public}u",
        info->beginDate, info->unLimitedDataEnable, info->monthlyLimitdNotifyType, info->monthlyLimit,
        info->monthlyMark, info->dailyMark);
}

void TrafficDataObserver::ReadTrafficDataSettingsPart2(std::shared_ptr<TrafficSettingsInfo> info)
{
    NETMGR_LOG_E("ReadTrafficDataSettings part2 start.");
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    Uri typeUri(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_NOTIFY_TYPE);
    std::string value = "";
    dataShareHelperUtils->Query(typeUri, TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_NOTIFY_TYPE, value);
    info->monthlyLimitdNotifyType = 1; // 1:默认断网
    int32_t type = 0;
    if (!value.empty() && NetStatsUtils::ConvertToInt32(value, type)) {
        info->monthlyLimitdNotifyType = type;
    }

    Uri mMarkuri(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + OVER_MONTHLY_MARK);
    value = "";
    dataShareHelperUtils->Query(mMarkuri, TAG_NAME + std::to_string(simId_) + "_" + OVER_MONTHLY_MARK, value);
    info->monthlyMark = 80;  // 月限额比例默认80%
    uint64_t mMark = 0;
    if (!value.empty() && NetStatsUtils::ConvertToUint64(value, mMark)) {
        info->monthlyMark = mMark;
    }

    Uri dMarkuri(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + OVER_DAILY_MARK);
    value = "";
    dataShareHelperUtils->Query(dMarkuri, TAG_NAME + std::to_string(simId_) + "_" + OVER_DAILY_MARK, value);
    info->dailyMark = 10;  // 日限额比例默认10%
    uint64_t dMark = 0;
    if (!value.empty() && NetStatsUtils::ConvertToUint64(value, dMark)) {
        info->dailyMark = dMark;
    }
}

// 无限流量开关
UnlimitTrafficEnableObserver::UnlimitTrafficEnableObserver(int32_t simId) : simId_(simId) {}

void UnlimitTrafficEnableObserver::OnChange()
{
    Uri uri(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + UNLIMITED_TRAFFIC_ENABLE);

    std::string value = "";
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    dataShareHelperUtils->Query(uri, TAG_NAME + std::to_string(simId_) + "_" + UNLIMITED_TRAFFIC_ENABLE, value);
    int32_t enable = 0;
    if (!value.empty()) {
        NetStatsUtils::ConvertToInt32(value, enable);
    }
    NETMGR_LOG_E("UnlimitTrafficEnableObserver OnChanged. dataString: %{public}s, TrafficInt: %{public}d",
        value.c_str(), enable);
    DelayedSingleton<NetStatsService>::GetInstance()->UpdataSettingsdata(simId_, NET_STATS_NO_LIMIT_ENABLE, enable);
}

// 套餐限额选项
TrafficMonthlyValueObserver::TrafficMonthlyValueObserver(int32_t simId) : simId_(simId) {}

void TrafficMonthlyValueObserver::OnChange()
{
    Uri uri(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_LIMITED_TRAFFIC);
    std::string value = "";
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    dataShareHelperUtils->Query(uri, TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_LIMITED_TRAFFIC, value);
    uint64_t trafficInt = 0;
    if (!value.empty()) {
        NetStatsUtils::ConvertToUint64(value, trafficInt);
    }
    NETMGR_LOG_E("TrafficMonthlyValueObserver OnChanged. dataString: %{public}s, TrafficInt: %{public}lu",
        value.c_str(), trafficInt);
    DelayedSingleton<NetStatsService>::GetInstance()->UpdataSettingsdata(simId_, NET_STATS_MONTHLY_LIMIT, trafficInt);
}

// 每月起始日期
TrafficMonthlyBeginDateObserver::TrafficMonthlyBeginDateObserver(int32_t simId) : simId_(simId) {}

void TrafficMonthlyBeginDateObserver::OnChange()
{
    Uri uri(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_BEGIN_DATE);
    std::string value = "";
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    dataShareHelperUtils->Query(uri, TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_BEGIN_DATE, value);
    int32_t dateInt = 0;
    if (!value.empty()) {
        NetStatsUtils::ConvertToInt32(value, dateInt);
    }
    NETMGR_LOG_E("TrafficMonthlyBeginDateObserver OnChanged. dataString: %{public}s, dateInt: %{public}d",
        value.c_str(), dateInt);
    DelayedSingleton<NetStatsService>::GetInstance()->UpdataSettingsdata(simId_, NET_STATS_BEGIN_DATE, dateInt);
}

// 月超限提醒类型
TrafficMonthlyNotifyTypeObserver::TrafficMonthlyNotifyTypeObserver(int32_t simId) : simId_(simId) {}

void TrafficMonthlyNotifyTypeObserver::OnChange()
{
    Uri uri(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_NOTIFY_TYPE);
    std::string value = "";
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    dataShareHelperUtils->Query(uri,  TAG_NAME + std::to_string(simId_) + "_" + MONTHLY_NOTIFY_TYPE, value);
    int32_t typeInt = 0;
    if (!value.empty()) {
        NetStatsUtils::ConvertToInt32(value, typeInt);
    }
    NETMGR_LOG_E("TrafficMonthlyNotifyTypeObserver OnChanged. typeString: %{public}s, typeInt: %{public}d",
        value.c_str(), typeInt);
    DelayedSingleton<NetStatsService>::GetInstance()->UpdataSettingsdata(simId_, NET_STATS_NOTIFY_TYPE, typeInt);
}

// 月超额提醒比例
TrafficMonthlyMarkObserver::TrafficMonthlyMarkObserver(int32_t simId) : simId_(simId) {}

void TrafficMonthlyMarkObserver::OnChange()
{
    Uri uri(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + OVER_MONTHLY_MARK);
    std::string value = "";
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    dataShareHelperUtils->Query(uri,  TAG_NAME + std::to_string(simId_) + "_" + OVER_MONTHLY_MARK, value);
    int32_t percentInt = 0;
    if (!value.empty()) {
        NetStatsUtils::ConvertToInt32(value, percentInt);
    }
    NETMGR_LOG_E("TrafficMonthlyMarkObserver OnChanged. percentString: %{public}s, percentInt: %{public}d",
        value.c_str(), percentInt);
    DelayedSingleton<NetStatsService>::GetInstance()->UpdataSettingsdata(simId_, NET_STATS_MONTHLY_MARK, percentInt);
}

// 日超额提醒比例
TrafficDailyMarkObserver::TrafficDailyMarkObserver(int32_t simId) : simId_(simId) {}

void TrafficDailyMarkObserver::OnChange()
{
    Uri uri(SETTING_URI + TAG_NAME + std::to_string(simId_) + "_" + OVER_DAILY_MARK);
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    std::string value = "";
    dataShareHelperUtils->Query(uri,  TAG_NAME + std::to_string(simId_) + "_" + OVER_DAILY_MARK, value);
    int32_t percentInt  = 0;
    if (!value.empty()) {
        NetStatsUtils::ConvertToInt32(value, percentInt);
    }
    NETMGR_LOG_E("TrafficDailyMarkObserver OnChanged. percentString: %{public}s, percentInt: %{public}d",
        value.c_str(), percentInt);
    DelayedSingleton<NetStatsService>::GetInstance()->UpdataSettingsdata(simId_, NET_STATS_DAILY_MARK, percentInt);
}

void CellularDataObserver::OnChange()
{
    auto dataShareHelperUtils = std::make_unique<NetDataShareHelperUtils>();
    std::string value = "";
    Uri uri(CELLULAR_DATA_SETTING_DATA_ENABLE_URI);
    dataShareHelperUtils->Query(uri,  "cellular_data_enable", value);
    NETMGR_LOG_I("CellularDataObserver OnChanged. enable: %{public}s", value.c_str());
}

} // namespace NetManagerStandard
} // namespace OHOS
