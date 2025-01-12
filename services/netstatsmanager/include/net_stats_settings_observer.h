/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef SWITCH_OBSERVER_H
#define SWITCH_OBSERVER_H

#include <vector>
#include <map>
#include <memory>
#include "singleton.h"
#include "data_ability_observer_stub.h"

namespace OHOS {
namespace NetManagerStandard {

struct TrafficSettingsInfo {
    int32_t beginDate = 0;
    int8_t unLimitedDataEnable = 0;
    int8_t monthlyLimitdNotifyType = -1;
    uint64_t monthlyLimit = UINT64_MAX;  // B
    uint16_t monthlyMark = UINT16_MAX;
    uint16_t dailyMark = UINT16_MAX;
    bool isCanNotifyMonthlyLimit = false;
    bool isCanNotifyMonthlyMark = false;
    bool isCanNotifyDailyMark = false;
    int32_t lastMonAlertTime = 0;
    int32_t lastMonNotifyTime = 0;
    int32_t lastDayNotifyTime = 0;
};

// 无限流量
class UnlimitTrafficEnableObserver : public AAFwk::DataAbilityObserverStub {
public:
    UnlimitTrafficEnableObserver(int32_t simId);
    ~UnlimitTrafficEnableObserver() = default;
    void OnChange() override;
private:
    int32_t simId_;
};

// 套餐限额
class TrafficMonthlyValueObserver : public AAFwk::DataAbilityObserverStub {
public:
    TrafficMonthlyValueObserver(int32_t simId);
    ~TrafficMonthlyValueObserver() = default;
    void OnChange() override;
private:
    int32_t simId_;
};

// 每月起始日期
class TrafficMonthlyBeginDateObserver : public AAFwk::DataAbilityObserverStub {
public:
    TrafficMonthlyBeginDateObserver(int32_t simId);
    ~TrafficMonthlyBeginDateObserver() = default;
    void OnChange() override;
private:
    int32_t simId_;
};

// 月提醒类型——弹窗/断网
class TrafficMonthlyNotifyTypeObserver : public AAFwk::DataAbilityObserverStub {
public:
    TrafficMonthlyNotifyTypeObserver(int32_t simId);
    ~TrafficMonthlyNotifyTypeObserver() = default;
    void OnChange() override;
private:
    int32_t simId_;
};

// 月超额
class TrafficMonthlyMarkObserver : public AAFwk::DataAbilityObserverStub {
public:
    TrafficMonthlyMarkObserver(int32_t simId);
    ~TrafficMonthlyMarkObserver() = default;
    void OnChange() override;
private:
    int32_t simId_;
};

// 日超额
class TrafficDailyMarkObserver : public AAFwk::DataAbilityObserverStub {
public:
    TrafficDailyMarkObserver(int32_t simId);
    ~TrafficDailyMarkObserver() = default;
    void OnChange() override;
private:
    int32_t simId_;
};

class CellularDataObserver : public AAFwk::DataAbilityObserverStub {
public:
    CellularDataObserver() = default;
    ~CellularDataObserver() = default;
    void OnChange() override;
};

class TrafficDataObserver {
public:
    TrafficDataObserver(int32_t simId_);
    ~TrafficDataObserver() = default;
    void RegisterTrafficDataSettingObserver();
    void UnRegisterTrafficDataSettingObserver();
    void ReadTrafficDataSettings(std::shared_ptr<TrafficSettingsInfo> info);
    void ReadTrafficDataSettingsPart2(std::shared_ptr<TrafficSettingsInfo> info);

public:
    int32_t simId_ { -1 };
    sptr<UnlimitTrafficEnableObserver> mUnlimitTrafficEnableObserver_ { nullptr} ;
    sptr<TrafficMonthlyValueObserver> mTrafficMonthlyValueObserver_ { nullptr} ;
    sptr<TrafficMonthlyBeginDateObserver> mTrafficMonthlyBeginDateObserver_ { nullptr} ;
    sptr<TrafficMonthlyNotifyTypeObserver> mTrafficMonthlyNotifyTypeObserver_ { nullptr} ;
    sptr<TrafficMonthlyMarkObserver> mTrafficMonthlyMarkObserver_ { nullptr} ;
    sptr<TrafficDailyMarkObserver> mTrafficDailyMarkObserver_ { nullptr} ;
    sptr<CellularDataObserver> mCellularDataObserver_ { nullptr} ;
};
}
}
#endif