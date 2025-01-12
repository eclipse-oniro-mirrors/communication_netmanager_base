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

#include <chrono>

#include "net_stats_utils.h"
#include "net_mgr_log_wrapper.h"
#include "cellular_data_client.h"
#include "core_service_client.h"

namespace OHOS {
namespace NetManagerStandard {

const int32_t TM_YEAR_START = 1900;

int32_t NetStatsUtils::GetStartTimestamp(int32_t startdate)
{
    // 获取当前日期和时间
    auto now = std::chrono::system_clock::now();
    time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    tm* now_tm = std::localtime(&now_time_t);
 
    // 获取当前年份和月份
    int current_year = now_tm->tm_year + TM_YEAR_START;
    int current_month = now_tm->tm_mon + 1; // tm_mon是0-11，所以需要加1
    int current_day = now_tm->tm_mday;
 
    // 计算上个月的年份和月份
    int previous_month = current_month == 1 ? 12 : current_month - 1;
    int previous_year = current_month == 1 ? current_year - 1 : current_year;

    // 构建 tm 结构表示上个月的10号
    tm last_month_xth_tm = {};
    last_month_xth_tm.tm_year = previous_year - TM_YEAR_START;
    last_month_xth_tm.tm_mon = previous_month - 1;
    last_month_xth_tm.tm_mday = startdate;

    int daysInCurrentMonth = NetStatsUtils::GetDaysInMonth(previous_year, previous_month);
    // 如果上个月没有起始日期这一天，就从本月第一天凌晨开始算
    if (daysInCurrentMonth < startdate) {
        last_month_xth_tm.tm_year = current_year - TM_YEAR_START;
        last_month_xth_tm.tm_mon = current_month -1 ;
        last_month_xth_tm.tm_mday = 1;
    }
    // 如果当前天大于起始日期，则获取本月的时间
    if (startdate <= current_day) {
        last_month_xth_tm.tm_year = current_year - TM_YEAR_START;
        last_month_xth_tm.tm_mon = current_month - 1;
        last_month_xth_tm.tm_mday = startdate;
    }

    NETMGR_LOG_I("last year: %{public}d, month: %{public}d, day: %{public}d",
        last_month_xth_tm.tm_year + TM_YEAR_START, last_month_xth_tm.tm_mon + 1, last_month_xth_tm.tm_mday);
 
    // 转换为 time_t
    time_t last_month_xth_time_t = mktime(&last_month_xth_tm);
    auto last_month_xth = std::chrono::system_clock::from_time_t(last_month_xth_time_t);

    int32_t timestamp = static_cast<int32_t>(std::chrono::system_clock::to_time_t(last_month_xth));
    NETMGR_LOG_I("timestamp: %{public}d", timestamp);
 
    return timestamp;
}

int32_t NetStatsUtils::GetTodayStartTimestamp()
{
    auto now = std::chrono::system_clock::now();
    time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    tm* now_tm = std::localtime(&now_time_t);

    tm last_month_xth_tm = {};
    last_month_xth_tm.tm_year = now_tm->tm_year;
    last_month_xth_tm.tm_mon = now_tm->tm_mon;
    last_month_xth_tm.tm_mday = now_tm->tm_mday;

    // 转换为 time_t
    time_t last_month_xth_time_t = mktime(&last_month_xth_tm);
 
    // 转换为系统时钟时间
    auto last_month_xth = std::chrono::system_clock::from_time_t(last_month_xth_time_t);
 
    // 转换为时间戳（通常用于网络传输的秒级时间戳）
    auto timestamp = std::chrono::system_clock::to_time_t(last_month_xth);

    return timestamp;
}

int32_t NetStatsUtils::GetNowTimestamp()
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::system_clock::to_time_t(now);
}

bool NetStatsUtils::IsLeapYear(int32_t year)
{
    // 判断是否是闰年
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0); // 4: 100/400计算闰年
}
 
int32_t NetStatsUtils::GetDaysInMonth(int32_t year, int32_t month)
{
    // 每个月的天数，默认是28天（适用于2月，即使不是闰年）
    static const int32_t daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
 
    if (month == 2 && NetStatsUtils::IsLeapYear(year)) { // 如果是2月并且是闰年
        return 29;  // 则天数为29
    }
 
    // 返回对应月份的天数
    return daysInMonth[month - 1];
}

bool NetStatsUtils::IsMobileDataEnabled()
{
    bool dataEnabled = false;
    int32_t errorCode =
        DelayedRefSingleton<Telephony::CellularDataClient>::GetInstance().IsCellularDataEnabled(dataEnabled);
    NETMGR_LOG_I("errorCode: %{public}d, isEnabled: %{public}d", errorCode, dataEnabled);
    return dataEnabled;
}

int32_t NetStatsUtils::IsDaulCardEnabled()
{
    int32_t actualSimNum = 0;
    int32_t simNum = Telephony::CoreServiceClient::GetInstance().GetMaxSimCount();
    for (int32_t i = 0; i < simNum; ++i) {
        bool hasSimCard;
        Telephony::CoreServiceClient::GetInstance().HasSimCard(i, hasSimCard);
        if (hasSimCard) {
            actualSimNum++;
        }
    }
    NETMGR_LOG_I("actualSimNum == %{public}d.", actualSimNum);
    return actualSimNum;
}
}
}