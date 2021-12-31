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

#include <vector>

#include <gtest/gtest.h>

#include "net_mgr_log_wrapper.h"
#include "net_tether_constants.h"
#include "net_tether_client.h"
#include "net_tether_callback_test.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class NetTetherClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetTetherClientTest::SetUpTestCase() {}

void NetTetherClientTest::TearDownTestCase() {}

void NetTetherClientTest::SetUp() {}

void NetTetherClientTest::TearDown() {}

/**
 * @tc.name: TetherByTypeAP
 * @tc.desc: Test NetTetherClient TetherByType.
 * @tc.type: FUNC
 */
HWTEST_F(NetTetherClientTest, TetherByTypeAP, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetTetherClient>::GetInstance()->TetherByType(TETHERING_WIFI);
    ASSERT_TRUE(ret == TETHERING_NO_ERR);
}

/**
 * @tc.name: TetherByTypeUSB
 * @tc.desc: Test NetTetherClient TetherByType.
 * @tc.type: FUNC
 */
HWTEST_F(NetTetherClientTest, TetherByTypeUSB, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetTetherClient>::GetInstance()->TetherByType(TETHERING_USB);
    ASSERT_TRUE(ret == TETHERING_NO_ERR);
}

/**
 * @tc.name: TetherByTypeBLUETOOTH
 * @tc.desc: Test NetTetherClient TetherByType.
 * @tc.type: FUNC
 */
HWTEST_F(NetTetherClientTest, TetherByTypeBLUETOOTH, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetTetherClient>::GetInstance()->TetherByType(TETHERING_BLUETOOTH);
    ASSERT_TRUE(ret == TETHERING_NO_ERR);
}

/**
 * @tc.name: UntetherByTypeAP
 * @tc.desc: Test NetTetherClient UntetherByType.
 * @tc.type: FUNC
 */
HWTEST_F(NetTetherClientTest, UntetherByTypeAP, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetTetherClient>::GetInstance()->UntetherByType(TETHERING_WIFI);
    ASSERT_TRUE(ret == TETHERING_NO_ERR);
}

/**
 * @tc.name: UntetherByTypeUSB
 * @tc.desc: Test NetTetherClient UntetherByType.
 * @tc.type: FUNC
 */
HWTEST_F(NetTetherClientTest, UntetherByTypeUSB, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetTetherClient>::GetInstance()->UntetherByType(TETHERING_USB);
    ASSERT_TRUE(ret == TETHERING_NO_ERR);
}

/**
 * @tc.name: UntetherByTypeBLUETOOTH
 * @tc.desc: Test NetTetherClient UntetherByType.
 * @tc.type: FUNC
 */
HWTEST_F(NetTetherClientTest, UntetherByTypeBLUETOOTH, TestSize.Level1)
{
    int32_t ret = DelayedSingleton<NetTetherClient>::GetInstance()->UntetherByType(TETHERING_BLUETOOTH);
    ASSERT_TRUE(ret == TETHERING_NO_ERR);
}

/**
 * @tc.name: TetherByIfaceFail
 * @tc.desc: Test NetTetherClient TetherByIface.
 * @tc.type: FUNC
 */
HWTEST_F(NetTetherClientTest, TetherByIfaceFail, TestSize.Level1)
{
    std::string ifaceName = "wlan1";
    int32_t ret = DelayedSingleton<NetTetherClient>::GetInstance()->TetherByIface(ifaceName);
    ASSERT_TRUE(ret == TETHERING_UNKNOWN_IFACE_ERROR);
}

/**
 * @tc.name: UntetherByIfaceFail
 * @tc.desc: Test NetTetherClient UntetherByIface.
 * @tc.type: FUNC
 */
HWTEST_F(NetTetherClientTest, UntetherByIfaceFail, TestSize.Level1)
{
    std::string ifaceName = "wlan1";
    int32_t ret = DelayedSingleton<NetTetherClient>::GetInstance()->UntetherByIface(ifaceName);
    ASSERT_TRUE(ret == TETHERING_UNKNOWN_IFACE_ERROR);
}

/**
 * @tc.name: RegisterTetheringEventCallbackSussess
 * @tc.desc: Test NetTetherClient RegisterTetheringEventCallback.
 * @tc.type: FUNC
 */
HWTEST_F(NetTetherClientTest, RegisterTetheringEventCallbackSussess, TestSize.Level1)
{
    sptr<NetTetherCallbackTest> callback = (std::make_unique<NetTetherCallbackTest>()).release();
    TetherResultCode ret=static_cast<TetherResultCode>(DelayedSingleton<NetTetherClient>::GetInstance()->RegisterTetheringEventCallback(callback));
    ASSERT_TRUE(ret == TETHERING_NO_ERR);
}
} // namespace NetManagerStandard
} // namespace OHOS
