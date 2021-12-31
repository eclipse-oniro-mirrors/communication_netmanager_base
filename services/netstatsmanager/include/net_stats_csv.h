/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef NET_STATS_CSV_H
#define NET_STATS_CSV_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "net_stats_info.h"

namespace OHOS {
namespace NetManagerStandard {
// iface.csv column definition
enum class IfaceCsvColumn {
    IFACE_NAME,
    TYPE
};

// uid.csv column definition
enum class UidCsvColumn {
    UID,
    UID_NAME
};

// iface_stats.csv column definition
enum class IfaceStatsCsvColumn {
    IFACE_NAME,
    TIME,
    RXBYTES,
    TXBYTES
} ;

// uid_stats.csv column definition
enum class UidStatCsvColumn {
    UID,
    IFACE_NAME,
    TIME,
    RXBYTES,
    TXBYTES
} ;

class CSVRow {
public:
    std::string const& operator[](uint32_t index) const
    {
        return data_[index];
    }

    void ReadNextRow(std::istream& str);

private:
    std::vector<std::string> data_;
};

class NetStatsCsv : public virtual RefBase {
public:
    NetStatsCsv();
    ~NetStatsCsv();

    bool ExistsIface(const std::string& iface);
    bool ExistsUid(const uint32_t uid);
    bool CorrectedIfacesStats(const std::string &iface, uint32_t start, uint32_t end, const NetStatsInfo &stats);
    bool UpdateIfaceCsvInfo();
    bool UpdateUidCsvInfo();
    bool UpdateIfaceStats();
    bool UpdateUidStats();
    bool UpdateIfaceStatsCsv(const std::string &iface);
    bool UpdateUidStatsCsv(uint32_t uid, const std::string &iface);
    NetStatsResultCode GetIfaceBytes(const std::string &iface, uint32_t start, uint32_t end,
        NetStatsInfo &statsInfo);
    NetStatsResultCode GetUidBytes(const std::string &iface, uint32_t uid, uint32_t start,
        uint32_t end, NetStatsInfo &statsInfo);
    NetStatsResultCode ResetFactory();

private:
    std::string GetCurrentTime();
    uint32_t GetIfaceCalculateTime(const std::string &iface, uint32_t time);
    uint32_t GetUidCalculateTime(uint32_t uid, const std::string &iface, uint32_t time);
    void GetCalculateStatsInfo(const CSVRow &row, uint32_t calStartTime, uint32_t calEndTime,
        uint32_t timeCloumn, std::vector<NetStatsInfo> &vecRow);
    void GetSumStats(const std::vector<NetStatsInfo> &vecRow, NetStatsInfo &sumStats);
    void GetPeriodStats(const NetStatsInfo &startStats, const NetStatsInfo &endStats, NetStatsInfo &totalStats);
    bool RenameIfacesStatsCsv(const std::string &fromFileName);

private:
    std::mutex mutex_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_STATS_CSV_H
