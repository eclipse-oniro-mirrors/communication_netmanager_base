/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_mgr_log_wrapper.h"
#include "net_stats_traffic_plan_service.h"
#include "net_manager_constants.h"
#include "mock_core_service_manager.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing;
using namespace testing::ext;

class NetStatsTrafficPlanServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

class MockCoreServiceClient {
public:
    MOCK_METHOD(int32_t, GetSlotId, (int32_t), (const));
};

void NetStatsTrafficPlanServiceTest::SetUpTestCase()
{
    NETMGR_LOG_D("NetStatsTrafficPlanServiceTest SetUpTestCase");
}

void NetStatsTrafficPlanServiceTest::TearDownTestCase()
{
    NETMGR_LOG_D("NetStatsTrafficPlanServiceTest TearDownTestCase");
}

void NetStatsTrafficPlanServiceTest::SetUp()
{
    NETMGR_LOG_D("NetStatsTrafficPlanServiceTest SetUp");
}

void NetStatsTrafficPlanServiceTest::TearDown()
{
    NETMGR_LOG_D("NetStatsTrafficPlanServiceTest TearDown");
}
#ifdef SUPPORT_TRAFFIC_STATISTIC
/**
 * @tc.number: NetStatsTrafficPlanService_InitTrafficPlanInfo_Normal
 * @tc.name: InitTrafficPlanInfo normal path
 * @tc.desc: Test normal initialization of traffic plan info from database
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, InitTrafficPlanInfo_Normal, TestSize.Level1)
{
    // Arrange - Stub database query to return success
    int32_t simId = 1;
    std::u16string iccid = {u"1234564654651"};
    // MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSimIccId(_, _)).
        WillRepeatedly(DoAll(SetArgReferee<1>(iccid), Return(0)));
    NetStatsTrafficPlanService trafficPlanService;
    // Act
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Assert - Traffic plan info should be initialized
    EXPECT_TRUE(trafficPlanService.IsSimIdExistInMap(simId));
}

/**
 * @tc.number: NetStatsTrafficPlanService_InitTrafficPlanInfo_SimIdExists
 * @tc.name: InitTrafficPlanInfo with existing simId
 * @tc.desc: Test that initialization skips when simId already exists in map
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, InitTrafficPlanInfo_SimIdExists, TestSize.Level1)
{
    // Arrange - Initialize once to create entry
    int32_t simId = 1;
    // MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));

    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act - Initialize again with same simId
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Assert - Should not cause any error, just return early
    EXPECT_TRUE(trafficPlanService.IsSimIdExistInMap(simId));
}

/**
 * @tc.number: NetStatsTrafficPlanService_DeleteTrafficPlanInfo_Normal
 * @tc.name: DeleteTrafficPlanInfo normal path
 * @tc.desc: Test normal deletion of traffic plan info
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, DeleteTrafficPlanInfo_Normal, TestSize.Level1)
{
    // Arrange - Add traffic plan info
    int32_t simId = 1;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));

    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act - Delete traffic plan info
    int32_t slotId = 0;
    trafficPlanService.DeleteTrafficPlanInfo(slotId);

    // Assert - Traffic plan info should be removed
    // Note: Need to verify deletion logic
    EXPECT_FALSE(trafficPlanService.IsSimIdExistInMap(simId));
}

/**
 * @tc.number: NetStatsTrafficPlanService_DeleteTrafficPlanInfo_NotFound
 * @tc.name: DeleteTrafficPlanInfo with non-existent slot
 * @tc.desc: Test deletion when slotId is not found in map
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, DeleteTrafficPlanInfo_NotFound, TestSize.Level1)
{
    // Arrange - Non-existent slotId
    int32_t slotId = 999;

    // Act - Delete with non-existent slotId
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.DeleteTrafficPlanInfo(slotId);

    EXPECT_TRUE(trafficPlanService.trafficPlanInfoMap_.find(slotId) == trafficPlanService.trafficPlanInfoMap_.end());
}

/**
 * @tc.number: NetStatsTrafficPlanService_DeleteTrafficPlanInfo_EmptyMap
 * @tc.name: DeleteTrafficPlanInfo with empty map
 * @tc.desc: Test deletion when traffic plan info map is empty
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, DeleteTrafficPlanInfo_EmptyMap, TestSize.Level1)
{
    // Arrange - Empty map (no initialization)

    // Act - Delete from empty map
    int32_t slotId = 0;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.DeleteTrafficPlanInfo(slotId);

    EXPECT_TRUE(trafficPlanService.trafficPlanInfoMap_.find(slotId) == trafficPlanService.trafficPlanInfoMap_.end());
}

/**
 * @tc.number: NetStatsTrafficPlanService_SetTrafficPlanInfo_Normal
 * @tc.name: SetTrafficPlanInfo normal path
 * @tc.desc: Test normal setting of traffic plan info
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, SetTrafficPlanInfo_Normal, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    TrafficPlanParam param = TrafficPlanParam::UNLIMIT_TRAFFIC_SWITCH;
    int64_t value = 1;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));

    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    int32_t ret = trafficPlanService.SetTrafficPlanInfo(simId, param, value);

    // Assert
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.number: NetStatsTrafficPlanService_SetTrafficPlanInfo_InvalidSimId
 * @tc.name: SetTrafficPlanInfo with invalid simId
 * @tc.desc: Test error handling when simId is invalid
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, SetTrafficPlanInfo_InvalidSimId, TestSize.Level1)
{
    // Arrange
    int32_t simId = -1;
    TrafficPlanParam param = TrafficPlanParam::DISPLAY_TRAFFIC_SWITCH;
    int64_t value = 1;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(-1));

    // Act
    NetStatsTrafficPlanService trafficPlanService;
    int32_t ret = trafficPlanService.SetTrafficPlanInfo(simId, param, value);

    // Assert
    EXPECT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
}

/**
 * @tc.number: NetStatsTrafficPlanService_SetTrafficPlanInfo_InvalidParam
 * @tc.name: SetTrafficPlanInfo with invalid param
 * @tc.desc: Test error handling when param value is invalid
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, SetTrafficPlanInfo_InvalidParam, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    TrafficPlanParam param = static_cast<TrafficPlanParam>(99);
    int64_t value = 1;

    // Act
    NetStatsTrafficPlanService trafficPlanService;
    int32_t ret = trafficPlanService.SetTrafficPlanInfo(simId, param, value);

    // Assert
    EXPECT_EQ(ret, TRAFFIC_PLAN_ERR_INVALID_PARAM);
}

/**
 * @tc.number: NetStatsTrafficPlanService_SetTrafficPlanInfo_InvalidValue
 * @tc.name: SetTrafficPlanInfo with invalid value
 * @tc.desc: Test error handling when value is invalid for the param type
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, SetTrafficPlanInfo_InvalidValue, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    TrafficPlanParam param = TrafficPlanParam::DISPLAY_TRAFFIC_SWITCH;
    int64_t value = 2; // Invalid: should be 0 or 1

    // Act
    NetStatsTrafficPlanService trafficPlanService;
    int32_t ret = trafficPlanService.SetTrafficPlanInfo(simId, param, value);

    // Assert
    EXPECT_EQ(ret, TRAFFIC_PLAN_ERR_INVALID_PARAM);
}

/**
 * @tc.number: NetStatsTrafficPlanService_SetTrafficPlanInfo_DisplaySwitch
 * @tc.name: SetTrafficPlanInfo display switch param
 * @tc.desc: Test setting display traffic switch parameter
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, SetTrafficPlanInfo_DisplaySwitch, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    TrafficPlanParam param = TrafficPlanParam::DISPLAY_TRAFFIC_SWITCH;
    int64_t value = 1;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    int32_t ret = trafficPlanService.SetTrafficPlanInfo(simId, param, value);

    // Assert
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.number: NetStatsTrafficPlanService_SetTrafficPlanInfo_TrafficLimit
 * @tc.name: SetTrafficPlanInfo traffic limit param
 * @tc.desc: Test setting traffic limit parameter
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, SetTrafficPlanInfo_TrafficLimit, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    TrafficPlanParam param = TrafficPlanParam::TRAFFIC_LIMIT;
    int64_t value = 1024 * 1024 * 1024; // 1GB
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    int32_t ret = trafficPlanService.SetTrafficPlanInfo(simId, param, value);

    // Assert
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.number: NetStatsTrafficPlanService_GetTrafficPlanInfo_CacheHit
 * @tc.name: GetTrafficPlanInfo cache hit
 * @tc.desc: Test getting traffic plan info from cache
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, GetTrafficPlanInfo_CacheHit, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    TrafficPlanParam param = TrafficPlanParam::DISPLAY_TRAFFIC_SWITCH;
    int64_t value = 1;
    int64_t result = 0;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));

    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);
    trafficPlanService.SetTrafficPlanInfo(simId, param, value);

    // Act
    int32_t ret = trafficPlanService.GetTrafficPlanInfo(simId, param, result);

    // Assert
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_EQ(result, value);
}

/**
 * @tc.number: NetStatsTrafficPlanService_GetTrafficPlanInfo_InvalidSimId
 * @tc.name: GetTrafficPlanInfo with invalid simId
 * @tc.desc: Test error handling when simId is invalid
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, GetTrafficPlanInfo_InvalidSimId, TestSize.Level1)
{
    // Arrange
    int32_t simId = -1;
    TrafficPlanParam param = TrafficPlanParam::DISPLAY_TRAFFIC_SWITCH;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(-1));
    int64_t result = 0;

    // Act
    NetStatsTrafficPlanService trafficPlanService;
    int32_t ret = trafficPlanService.GetTrafficPlanInfo(simId, param, result);

    // Assert
    EXPECT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ValidateTrafficPlanParam_DisplaySwitch_Valid
 * @tc.name: ValidateTrafficPlanParam with valid display switch
 * @tc.desc: Test validation of display traffic switch with valid value
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ValidateTrafficPlanParam_DisplaySwitch_Valid, TestSize.Level1)
{
    // Arrange
    TrafficPlanParam param = TrafficPlanParam::DISPLAY_TRAFFIC_SWITCH;
    int64_t value = 1;

    // Act - This is a private method, need to access via public interface or test mode
    // For now, we'll test indirectly through SetTrafficPlanInfo
    int32_t simId = 1;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    int32_t ret = trafficPlanService.SetTrafficPlanInfo(simId, param, value);

    // Assert
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ValidateTrafficPlanParam_DisplaySwitch_Invalid
 * @tc.name: ValidateTrafficPlanParam with invalid display switch
 * @tc.desc: Test validation of display traffic switch with invalid value
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ValidateTrafficPlanParam_DisplaySwitch_Invalid, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    TrafficPlanParam param = TrafficPlanParam::DISPLAY_TRAFFIC_SWITCH;
    int64_t value = 2; // Invalid: should be 0 or 1
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    int32_t ret = trafficPlanService.SetTrafficPlanInfo(simId, param, value);

    // Assert
    EXPECT_EQ(ret, TRAFFIC_PLAN_ERR_INVALID_PARAM);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ValidateTrafficPlanParam_TrafficLimit_Valid
 * @tc.name: ValidateTrafficPlanParam with valid traffic limit
 * @tc.desc: Test validation of traffic limit with valid value
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ValidateTrafficPlanParam_TrafficLimit_Valid, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    TrafficPlanParam param = TrafficPlanParam::TRAFFIC_LIMIT;
    int64_t value = 1024 * 1024 * 1024; // 1GB
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));

    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    int32_t ret = trafficPlanService.SetTrafficPlanInfo(simId, param, value);

    // Assert
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetStatsTrafficPlanServiceTest, ValidateTrafficPlanParam_TrafficLimit_valid2, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    TrafficPlanParam param = TrafficPlanParam::TRAFFIC_LIMIT;
    int64_t value = -1;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));

    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    int32_t ret = trafficPlanService.SetTrafficPlanInfo(simId, param, value);

    // Assert
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ValidateTrafficPlanParam_TrafficLimit_Invalid
 * @tc.name: ValidateTrafficPlanParam with invalid traffic limit
 * @tc.desc: Test validation of traffic limit with invalid value
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ValidateTrafficPlanParam_TrafficLimit_Invalid, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    TrafficPlanParam param = TrafficPlanParam::TRAFFIC_LIMIT;
    int64_t value = -2; // Invalid: should be -1 or >= 0
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));

    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    int32_t ret = trafficPlanService.SetTrafficPlanInfo(simId, param, value);

    // Assert
    EXPECT_EQ(ret, TRAFFIC_PLAN_ERR_INVALID_PARAM);
}

/**
 * @tc.number: NetStatsTrafficPlanService_GetFieldValueByParam_AllTypes
 * @tc.name: GetFieldValueByParam all param types
 * @tc.desc: Test getting field values for all parameter types
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, GetFieldValueByParam_AllTypes, TestSize.Level1)
{
    // Arrange - Test via GetTrafficPlanInfo
    int32_t simId = 1;
    int64_t result = 0;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));

    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act & Assert - Test each parameter type
    TrafficPlanParam params[] = {TrafficPlanParam::DISPLAY_TRAFFIC_SWITCH, TrafficPlanParam::UNLIMIT_TRAFFIC_SWITCH,
                                 TrafficPlanParam::TRAFFIC_LIMIT,          TrafficPlanParam::START_DATE,
                                 TrafficPlanParam::OVER_LIMIT_BEHAVIOR,    TrafficPlanParam::MONTHLY_LIMIT_PERCENTAGE,
                                 TrafficPlanParam::DAILY_LIMIT_PERCENTAGE};

    for (const auto &param : params) {
        result = 0;
        int32_t ret = trafficPlanService.GetTrafficPlanInfo(simId, param, result);
        EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    }
}

/**
 * @tc.number: NetStatsTrafficPlanService_OnBackup_Normal
 * @tc.name: OnBackup normal path
 * @tc.desc: Test normal backup operation
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, OnBackup_Normal, TestSize.Level1)
{
    // Arrange
    MessageParcel data;
    MessageParcel reply;

    // Act
    NetStatsTrafficPlanService trafficPlanService;
    int32_t ret = trafficPlanService.OnBackup(data, reply);

    // Assert
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: NetStatsTrafficPlanService_OnRestore_Normal
 * @tc.name: OnRestore normal path
 * @tc.desc: Test normal restore operation
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, OnRestore_Normal, TestSize.Level1)
{
    // Arrange
    MessageParcel data;
    MessageParcel reply;

    // Act
    NetStatsTrafficPlanService trafficPlanService;
    int32_t ret = trafficPlanService.OnRestore(data, reply);

    // Assert
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ResetNotifyState_Normal
 * @tc.name: ResetNotifyState normal path
 * @tc.desc: Test normal reset of notification state
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ResetNotifyState_Normal, TestSize.Level1)
{
    // Arrange - Initialize traffic plan info
    int32_t simId = 1;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    trafficPlanService.ResetNotifyState(simId);

    EXPECT_TRUE(trafficPlanService.trafficPlanInfoMap_.find(simId) != trafficPlanService.trafficPlanInfoMap_.end());
}

/**
 * @tc.number: NetStatsTrafficPlanService_ResetNotifyState_SimIdNotFound
 * @tc.name: ResetNotifyState with non-existent simId
 * @tc.desc: Test reset when simId is not found in map
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ResetNotifyState_SimIdNotFound, TestSize.Level1)
{
    // Arrange
    int32_t simId = 999;

    // Act
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.ResetNotifyState(simId);

    EXPECT_FALSE(trafficPlanService.trafficPlanInfoMap_.find(simId) != trafficPlanService.trafficPlanInfoMap_.end());
}

/**
 * @tc.number: NetStatsTrafficPlanService_UpdateTrafficLimitDate_Normal
 * @tc.name: UpdateTrafficLimitDate normal path
 * @tc.desc: Test normal update of traffic limit date
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, UpdateTrafficLimitDate_Normal, TestSize.Level1)
{
    // Arrange - Initialize traffic plan info
    int32_t simId = 1;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    trafficPlanService.UpdateTrafficLimitDate(simId);

    EXPECT_TRUE(trafficPlanService.trafficPlanInfoMap_.find(simId) != trafficPlanService.trafficPlanInfoMap_.end());
}

/**
 * @tc.number: NetStatsTrafficPlanService_IsSimIdExistInMap_True
 * @tc.name: IsSimIdExistInMap when simId exists
 * @tc.desc: Test checking existence when simId is in map
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, IsSimIdExistInMap_True, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    bool exists = trafficPlanService.IsSimIdExistInMap(simId);

    // Assert
    EXPECT_TRUE(exists);
}

/**
 * @tc.number: NetStatsTrafficPlanService_IsSimIdExistInMap_False
 * @tc.name: IsSimIdExistInMap when simId not found
 * @tc.desc: Test checking existence when simId is not in map
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, IsSimIdExistInMap_False, TestSize.Level1)
{
    // Arrange
    int32_t simId = 999;

    // Act
    NetStatsTrafficPlanService trafficPlanService;
    bool exists = trafficPlanService.IsSimIdExistInMap(simId);

    // Assert
    EXPECT_FALSE(exists);
}

/**
 * @tc.number: NetStatsTrafficPlanService_GetTrafficPlanInfoSnapshot_Found
 * @tc.name: GetTrafficPlanInfoSnapshot when found
 * @tc.desc: Test getting traffic plan info snapshot when simId exists
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, GetTrafficPlanInfoSnapshot_Found, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    TrafficPlanInfo info;
    bool ret = trafficPlanService.GetTrafficPlanInfoSnapshot(simId, info);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(info.trafficLimit, trafficPlanService.trafficPlanInfoMap_[simId]->trafficLimit);
}

/**
 * @tc.number: NetStatsTrafficPlanService_GetTrafficPlanInfoSnapshot_NotFound
 * @tc.name: GetTrafficPlanInfoSnapshot when not found
 * @tc.desc: Test getting traffic plan info snapshot when simId doesn't exist
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, GetTrafficPlanInfoSnapshot_NotFound, TestSize.Level1)
{
    // Arrange
    int32_t simId = 999;

    // Act
    NetStatsTrafficPlanService trafficPlanService;
    TrafficPlanInfo info;
    bool ret = trafficPlanService.GetTrafficPlanInfoSnapshot(simId, info);

    // Assert
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: NetStatsTrafficPlanService_GetTrafficPlanInfoSnapshot_Nullptr
 * @tc.name: GetTrafficPlanInfoSnapshot with null entry
 * @tc.desc: Test getting traffic plan info snapshot when map entry is null
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, GetTrafficPlanInfoSnapshot_Nullptr, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[simId] = nullptr;

    // Act
    TrafficPlanInfo info;
    bool ret = trafficPlanService.GetTrafficPlanInfoSnapshot(simId, info);

    // Assert
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ResetNotifyState_NotFound
 * @tc.name: ResetNotifyState when simId not found
 * @tc.desc: Test ResetNotifyState does not crash and inserts nothing when simId is absent
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ResetNotifyState_NotFound, TestSize.Level1)
{
    // Arrange
    int32_t simId = 999;
    NetStatsTrafficPlanService trafficPlanService;

    // Act
    trafficPlanService.ResetNotifyState(simId);

    // Assert
    EXPECT_TRUE(trafficPlanService.trafficPlanInfoMap_.empty());
}

/**
 * @tc.number: NetStatsTrafficPlanService_ResetNotifyState_Nullptr
 * @tc.name: ResetNotifyState with null entry
 * @tc.desc: Test ResetNotifyState returns early without crash when map entry is null
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ResetNotifyState_Nullptr, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[simId] = nullptr;

    // Act
    trafficPlanService.ResetNotifyState(simId);

    // Assert
    EXPECT_NE(trafficPlanService.trafficPlanInfoMap_.find(simId), trafficPlanService.trafficPlanInfoMap_.end());
}

/**
 * @tc.number: NetStatsTrafficPlanService_ResetNotifyState_SetFlags
 * @tc.name: ResetNotifyState enables all notify flags
 * @tc.desc: Test ResetNotifyState sets the three notify flags to true
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ResetNotifyState_SetFlags, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[simId] = std::make_shared<TrafficPlanInfo>();
    trafficPlanService.trafficPlanInfoMap_[simId]->isCanNotifyMonthlyLimit = false;
    trafficPlanService.trafficPlanInfoMap_[simId]->isCanNotifyMonthlyMark = false;
    trafficPlanService.trafficPlanInfoMap_[simId]->isCanNotifyDailyMark = false;

    // Act
    trafficPlanService.ResetNotifyState(simId);

    // Assert
    TrafficPlanInfo info;
    EXPECT_TRUE(trafficPlanService.GetTrafficPlanInfoSnapshot(simId, info));
    EXPECT_TRUE(info.isCanNotifyMonthlyLimit);
    EXPECT_TRUE(info.isCanNotifyMonthlyMark);
    EXPECT_TRUE(info.isCanNotifyDailyMark);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ConsumeMonAlertState_NotFound
 * @tc.name: ConsumeMonAlertState when simId not found
 * @tc.desc: Test ConsumeMonAlertState returns false when simId is absent
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ConsumeMonAlertState_NotFound, TestSize.Level1)
{
    // Arrange
    int32_t simId = 999;
    NetStatsTrafficPlanService trafficPlanService;

    // Act
    bool ret = trafficPlanService.ConsumeMonAlertState(simId);

    // Assert
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ConsumeMonAlertState_Nullptr
 * @tc.name: ConsumeMonAlertState with null entry
 * @tc.desc: Test ConsumeMonAlertState returns false when map entry is null
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ConsumeMonAlertState_Nullptr, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[simId] = nullptr;

    // Act
    bool ret = trafficPlanService.ConsumeMonAlertState(simId);

    // Assert
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ConsumeMonAlertState_CanNotify
 * @tc.name: ConsumeMonAlertState when monthly limit notify flag is set
 * @tc.desc: Test ConsumeMonAlertState returns true and clears all notify flags
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ConsumeMonAlertState_CanNotify, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[simId] = std::make_shared<TrafficPlanInfo>();
    trafficPlanService.trafficPlanInfoMap_[simId]->isCanNotifyMonthlyLimit = true;
    trafficPlanService.trafficPlanInfoMap_[simId]->isCanNotifyMonthlyMark = true;
    trafficPlanService.trafficPlanInfoMap_[simId]->isCanNotifyDailyMark = true;

    // Act
    bool ret = trafficPlanService.ConsumeMonAlertState(simId);

    // Assert
    EXPECT_TRUE(ret);
    TrafficPlanInfo info;
    EXPECT_TRUE(trafficPlanService.GetTrafficPlanInfoSnapshot(simId, info));
    EXPECT_FALSE(info.isCanNotifyMonthlyLimit);
    EXPECT_FALSE(info.isCanNotifyMonthlyMark);
    EXPECT_FALSE(info.isCanNotifyDailyMark);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ConsumeMonAlertState_TimeWindow
 * @tc.name: ConsumeMonAlertState time window branches
 * @tc.desc: Test ConsumeMonAlertState result depends on lastMonAlertTime vs current month start
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ConsumeMonAlertState_TimeWindow, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[simId] = std::make_shared<TrafficPlanInfo>();
    trafficPlanService.trafficPlanInfoMap_[simId]->lastMonAlertTime = 0;

    // Act
    bool retEarly = trafficPlanService.ConsumeMonAlertState(simId);

    trafficPlanService.trafficPlanInfoMap_[simId]->lastMonAlertTime =
        static_cast<int32_t>(NetStatsUtils::GetNowTimestamp());
    bool retLate = trafficPlanService.ConsumeMonAlertState(simId);

    // Assert
    EXPECT_TRUE(retEarly);
    EXPECT_FALSE(retLate);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ConsumeMonNotifyState_NotFound
 * @tc.name: ConsumeMonNotifyState when simId not found
 * @tc.desc: Test ConsumeMonNotifyState returns false when simId is absent or entry is null
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ConsumeMonNotifyState_NotFound, TestSize.Level1)
{
    // Arrange
    int32_t simId = 999;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[1] = nullptr;

    // Act
    bool retAbsent = trafficPlanService.ConsumeMonNotifyState(simId);
    bool retNull = trafficPlanService.ConsumeMonNotifyState(1);

    // Assert
    EXPECT_FALSE(retAbsent);
    EXPECT_FALSE(retNull);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ConsumeMonNotifyState_CanNotify
 * @tc.name: ConsumeMonNotifyState when monthly mark notify flag is set
 * @tc.desc: Test ConsumeMonNotifyState returns true and clears the monthly mark flag
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ConsumeMonNotifyState_CanNotify, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[simId] = std::make_shared<TrafficPlanInfo>();
    trafficPlanService.trafficPlanInfoMap_[simId]->isCanNotifyMonthlyMark = true;

    // Act
    bool ret = trafficPlanService.ConsumeMonNotifyState(simId);

    // Assert
    EXPECT_TRUE(ret);
    TrafficPlanInfo info;
    EXPECT_TRUE(trafficPlanService.GetTrafficPlanInfoSnapshot(simId, info));
    EXPECT_FALSE(info.isCanNotifyMonthlyMark);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ConsumeMonNotifyState_TimeWindow
 * @tc.name: ConsumeMonNotifyState time window branches
 * @tc.desc: Test ConsumeMonNotifyState result depends on lastMonNotifyTime vs current month start
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ConsumeMonNotifyState_TimeWindow, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[simId] = std::make_shared<TrafficPlanInfo>();
    trafficPlanService.trafficPlanInfoMap_[simId]->lastMonNotifyTime = 0;

    // Act
    bool retEarly = trafficPlanService.ConsumeMonNotifyState(simId);

    trafficPlanService.trafficPlanInfoMap_[simId]->lastMonNotifyTime =
        static_cast<int32_t>(NetStatsUtils::GetNowTimestamp());
    bool retLate = trafficPlanService.ConsumeMonNotifyState(simId);

    // Assert
    EXPECT_TRUE(retEarly);
    EXPECT_FALSE(retLate);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ConsumeDayNotifyState_NotFound
 * @tc.name: ConsumeDayNotifyState when simId not found
 * @tc.desc: Test ConsumeDayNotifyState returns false when simId is absent or entry is null
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ConsumeDayNotifyState_NotFound, TestSize.Level1)
{
    // Arrange
    int32_t simId = 999;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[1] = nullptr;

    // Act
    bool retAbsent = trafficPlanService.ConsumeDayNotifyState(simId);
    bool retNull = trafficPlanService.ConsumeDayNotifyState(1);

    // Assert
    EXPECT_FALSE(retAbsent);
    EXPECT_FALSE(retNull);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ConsumeDayNotifyState_CanNotify
 * @tc.name: ConsumeDayNotifyState when daily mark notify flag is set
 * @tc.desc: Test ConsumeDayNotifyState returns true and clears the daily mark flag
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ConsumeDayNotifyState_CanNotify, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[simId] = std::make_shared<TrafficPlanInfo>();
    trafficPlanService.trafficPlanInfoMap_[simId]->isCanNotifyDailyMark = true;

    // Act
    bool ret = trafficPlanService.ConsumeDayNotifyState(simId);

    // Assert
    EXPECT_TRUE(ret);
    TrafficPlanInfo info;
    EXPECT_TRUE(trafficPlanService.GetTrafficPlanInfoSnapshot(simId, info));
    EXPECT_FALSE(info.isCanNotifyDailyMark);
}

/**
 * @tc.number: NetStatsTrafficPlanService_ConsumeDayNotifyState_TimeWindow
 * @tc.name: ConsumeDayNotifyState time window branches
 * @tc.desc: Test ConsumeDayNotifyState result depends on lastDayNotifyTime vs today start
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, ConsumeDayNotifyState_TimeWindow, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[simId] = std::make_shared<TrafficPlanInfo>();
    trafficPlanService.trafficPlanInfoMap_[simId]->lastDayNotifyTime = 0;

    // Act
    bool retEarly = trafficPlanService.ConsumeDayNotifyState(simId);

    trafficPlanService.trafficPlanInfoMap_[simId]->lastDayNotifyTime =
        static_cast<int32_t>(NetStatsUtils::GetNowTimestamp());
    bool retLate = trafficPlanService.ConsumeDayNotifyState(simId);

    // Assert
    EXPECT_TRUE(retEarly);
    EXPECT_FALSE(retLate);
}

/**
 * @tc.number: NetStatsTrafficPlanService_UpdateNotifyTime_NotFound
 * @tc.name: UpdateMonAlertTime/UpdateMonNotifyTime/UpdateDayNotifyTime when simId not found
 * @tc.desc: Test the three Update*Time methods do not crash when simId is absent or entry is null
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, UpdateNotifyTime_NotFound, TestSize.Level1)
{
    // Arrange
    int32_t simId = 999;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[1] = nullptr;

    // Act
    trafficPlanService.UpdateMonAlertTime(simId);
    trafficPlanService.UpdateMonNotifyTime(simId);
    trafficPlanService.UpdateDayNotifyTime(simId);
    trafficPlanService.UpdateMonAlertTime(1);
    trafficPlanService.UpdateMonNotifyTime(1);
    trafficPlanService.UpdateDayNotifyTime(1);

    // Assert
    EXPECT_TRUE(trafficPlanService.trafficPlanInfoMap_.find(1) != trafficPlanService.trafficPlanInfoMap_.end());
}

/**
 * @tc.number: NetStatsTrafficPlanService_UpdateNotifyTime_Normal
 * @tc.name: UpdateMonAlertTime/UpdateMonNotifyTime/UpdateDayNotifyTime normal path
 * @tc.desc: Test the three Update*Time methods record the current timestamp
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, UpdateNotifyTime_Normal, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[simId] = std::make_shared<TrafficPlanInfo>();

    // Act
    trafficPlanService.UpdateMonAlertTime(simId);
    trafficPlanService.UpdateMonNotifyTime(simId);
    trafficPlanService.UpdateDayNotifyTime(simId);

    // Assert
    TrafficPlanInfo info;
    EXPECT_TRUE(trafficPlanService.GetTrafficPlanInfoSnapshot(simId, info));
    EXPECT_GT(info.lastMonAlertTime, 0);
    EXPECT_GT(info.lastMonNotifyTime, 0);
    EXPECT_GT(info.lastDayNotifyTime, 0);
}

/**
 * @tc.number: NetStatsTrafficPlanService_IsOverLimitDisconnect_Branches
 * @tc.name: IsOverLimitDisconnect branch coverage
 * @tc.desc: Test IsOverLimitDisconnect with absent simId, null entry and both behavior values
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, IsOverLimitDisconnect_Branches, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[2] = nullptr;

    // Act
    bool retAbsent = trafficPlanService.IsOverLimitDisconnect(999);
    bool retNull = trafficPlanService.IsOverLimitDisconnect(2);

    trafficPlanService.trafficPlanInfoMap_[simId] = std::make_shared<TrafficPlanInfo>();
    trafficPlanService.trafficPlanInfoMap_[simId]->overLimitBehavior = 1;
    bool retDisconnect = trafficPlanService.IsOverLimitDisconnect(simId);

    trafficPlanService.trafficPlanInfoMap_[simId]->overLimitBehavior = 0;
    bool retPopup = trafficPlanService.IsOverLimitDisconnect(simId);

    // Assert
    EXPECT_FALSE(retAbsent);
    EXPECT_FALSE(retNull);
    EXPECT_TRUE(retDisconnect);
    EXPECT_FALSE(retPopup);
}

/**
 * @tc.number: NetStatsTrafficPlanService_IsUnlimitTrafficSwitchOn_Branches
 * @tc.name: IsUnlimitTrafficSwitchOn branch coverage
 * @tc.desc: Test IsUnlimitTrafficSwitchOn with absent simId, null entry and both switch values
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, IsUnlimitTrafficSwitchOn_Branches, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[2] = nullptr;

    // Act
    bool retAbsent = trafficPlanService.IsUnlimitTrafficSwitchOn(999);
    bool retNull = trafficPlanService.IsUnlimitTrafficSwitchOn(2);

    trafficPlanService.trafficPlanInfoMap_[simId] = std::make_shared<TrafficPlanInfo>();
    trafficPlanService.trafficPlanInfoMap_[simId]->unlimitTrafficSwitch = 1;
    bool retOn = trafficPlanService.IsUnlimitTrafficSwitchOn(simId);

    trafficPlanService.trafficPlanInfoMap_[simId]->unlimitTrafficSwitch = 0;
    bool retOff = trafficPlanService.IsUnlimitTrafficSwitchOn(simId);

    // Assert
    EXPECT_FALSE(retAbsent);
    EXPECT_FALSE(retNull);
    EXPECT_TRUE(retOn);
    EXPECT_FALSE(retOff);
}

/**
 * @tc.number: NetStatsTrafficPlanService_UpdateTrafficLimit_NotFound
 * @tc.name: UpdateTrafficLimit when simId not found
 * @tc.desc: Test UpdateTrafficLimit does not crash and inserts nothing when simId is absent or null
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, UpdateTrafficLimit_NotFound, TestSize.Level1)
{
    // Arrange
    uint64_t trafficLimit = 1024;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[2] = nullptr;

    // Act
    trafficPlanService.UpdateTrafficLimit(999, trafficLimit);
    trafficPlanService.UpdateTrafficLimit(2, trafficLimit);

    // Assert
    EXPECT_EQ(trafficPlanService.trafficPlanInfoMap_.size(), 1);
    EXPECT_EQ(trafficPlanService.trafficPlanInfoMap_[2], nullptr);
}

/**
 * @tc.number: NetStatsTrafficPlanService_UpdateTrafficLimit_Normal
 * @tc.name: UpdateTrafficLimit normal path
 * @tc.desc: Test UpdateTrafficLimit updates the traffic limit of the existing entry
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, UpdateTrafficLimit_Normal, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    uint64_t trafficLimit = 2048;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[simId] = std::make_shared<TrafficPlanInfo>();
    trafficPlanService.trafficPlanInfoMap_[simId]->trafficLimit = 1024;

    // Act
    trafficPlanService.UpdateTrafficLimit(simId, trafficLimit);

    // Assert
    uint64_t monthlyLimit = 0;
    EXPECT_TRUE(trafficPlanService.GetMonthlyLimitBySimId(simId, monthlyLimit));
    EXPECT_EQ(monthlyLimit, trafficLimit);
}

/**
 * @tc.number: NetStatsTrafficPlanService_GetMonthlyDailyMarkBySimId_Nullptr
 * @tc.name: GetMonthlyMarkBySimId/GetDailyMarkBySimId with null entry
 * @tc.desc: Test both mark getters return false when map entry is null
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, GetMonthlyDailyMarkBySimId_Nullptr, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    uint16_t monthlyMark = 0;
    uint16_t dailyMark = 0;
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.trafficPlanInfoMap_[simId] = nullptr;

    // Act
    bool retMonthly = trafficPlanService.GetMonthlyMarkBySimId(simId, monthlyMark);
    bool retDaily = trafficPlanService.GetDailyMarkBySimId(simId, dailyMark);

    // Assert
    EXPECT_FALSE(retMonthly);
    EXPECT_FALSE(retDaily);
}

/**
 * @tc.number: NetStatsTrafficPlanService_GetMonthlyLimitBySimId_Found
 * @tc.name: GetMonthlyLimitBySimId when found
 * @tc.desc: Test getting monthly limit when simId exists
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, GetMonthlyLimitBySimId_Found, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    uint64_t monthlyLimit = 0;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    bool ret = trafficPlanService.GetMonthlyLimitBySimId(simId, monthlyLimit);

    // Assert
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: NetStatsTrafficPlanService_GetMonthlyLimitBySimId_NotFound
 * @tc.name: GetMonthlyLimitBySimId when not found
 * @tc.desc: Test getting monthly limit when simId doesn't exist
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, GetMonthlyLimitBySimId_NotFound, TestSize.Level1)
{
    // Arrange
    int32_t simId = 999;
    uint64_t monthlyLimit = 0;

    // Act
    NetStatsTrafficPlanService trafficPlanService;
    bool ret = trafficPlanService.GetMonthlyLimitBySimId(simId, monthlyLimit);

    // Assert
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: NetStatsTrafficPlanService_GetMonthlyMarkBySimId_Found
 * @tc.name: GetMonthlyMarkBySimId when found
 * @tc.desc: Test getting monthly percentage mark when simId exists
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, GetMonthlyMarkBySimId_Found, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    uint16_t monthlyMark = 0;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    bool ret = trafficPlanService.GetMonthlyMarkBySimId(simId, monthlyMark);

    // Assert
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: NetStatsTrafficPlanService_GetMonthlyMarkBySimId_NotFound
 * @tc.name: GetMonthlyMarkBySimId when not found
 * @tc.desc: Test getting monthly percentage mark when simId doesn't exist
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, GetMonthlyMarkBySimId_NotFound, TestSize.Level1)
{
    // Arrange
    int32_t simId = 999;
    uint16_t monthlyMark = 0;

    // Act
    NetStatsTrafficPlanService trafficPlanService;
    bool ret = trafficPlanService.GetMonthlyMarkBySimId(simId, monthlyMark);

    // Assert
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: NetStatsTrafficPlanService_GetdailyMarkBySimId_Found
 * @tc.name: GetDailyMarkBySimId when found
 * @tc.desc: Test getting daily percentage mark when simId exists
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, GetdailyMarkBySimId_Found, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    uint16_t dailyMark = 0;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    bool ret = trafficPlanService.GetDailyMarkBySimId(simId, dailyMark);

    // Assert
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: NetStatsTrafficPlanService_GetdailyMarkBySimId_NotFound
 * @tc.name: GetDailyMarkBySimId when not found
 * @tc.desc: Test getting daily percentage mark when simId doesn't exist
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, GetdailyMarkBySimId_NotFound, TestSize.Level1)
{
    // Arrange
    int32_t simId = 999;
    uint16_t dailyMark = 0;

    // Act
    NetStatsTrafficPlanService trafficPlanService;
    bool ret = trafficPlanService.GetDailyMarkBySimId(simId, dailyMark);

    // Assert
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: NetStatsTrafficPlanService_UpdateNetStatsToMapFromDB_Normal
 * @tc.name: UpdateNetStatsToMapFromDB normal path
 * @tc.desc: Test normal update of net stats from database
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, UpdateNetStatsToMapFromDB_Normal, TestSize.Level1)
{
    // Arrange
    int32_t simId = 1;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    trafficPlanService.UpdateNetStatsToMapFromDB(simId);

    EXPECT_TRUE(trafficPlanService.trafficPlanInfoMap_.find(simId) != trafficPlanService.trafficPlanInfoMap_.end());
}

/**
 * @tc.number: NetStatsTrafficPlanService_UpdateNetStatsToMapFromDB_EmptyResult
 * @tc.name: UpdateNetStatsToMapFromDB with empty DB result
 * @tc.desc: Test update when database returns empty result
 */
HWTEST_F(NetStatsTrafficPlanServiceTest, UpdateNetStatsToMapFromDB_EmptyResult, TestSize.Level1)
{
    // Arrange - Initialize without DB data
    int32_t simId = 2;
    MockCoreServiceClient mockClient;
    EXPECT_CALL(MockCoreServiceManager::GetInstance(), GetSlotId(simId)).WillRepeatedly(Return(0));
    NetStatsTrafficPlanService trafficPlanService;
    trafficPlanService.InitTrafficPlanInfo(simId);

    // Act
    trafficPlanService.UpdateNetStatsToMapFromDB(simId);

    // Assert - Should handle empty result gracefully
    EXPECT_TRUE(trafficPlanService.trafficPlanInfoMap_.find(simId) != trafficPlanService.trafficPlanInfoMap_.end());
}
#endif
} // namespace NetManagerStandard
} // namespace OHOS
