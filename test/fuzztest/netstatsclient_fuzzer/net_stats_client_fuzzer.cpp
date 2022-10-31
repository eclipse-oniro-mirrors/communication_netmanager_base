/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <thread>
#include <ctime>
#include <securec.h>

#include "data_flow_statistics.h"
#include "net_mgr_log_wrapper.h"
#include "net_stats_constants.h"
#include "net_stats_client.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t STR_LEN = 10;
}

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_baseFuzzData == nullptr || objectSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, g_baseFuzzData + g_baseFuzzPos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += objectSize;
    return object;
}

std::string GetStringFromData(int strlen)
{
    char cstr[strlen];
    cstr[strlen - 1] = '\0';
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = GetData<char>();
    }
    std::string str(cstr);
    return str;
}

class INetStatsCallbackTest : public INetStatsCallback {
public:
    INetStatsCallbackTest() : INetStatsCallback() {}
    virtual ~INetStatsCallbackTest() {}
};

void RegisterNetStatsCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    sptr<INetStatsCallbackTest> callback = sptr<INetStatsCallbackTest>();
    DelayedSingleton<NetStatsClient>::GetInstance()->RegisterNetStatsCallback(callback);
}

void UnregisterNetStatsCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    sptr<INetStatsCallbackTest> callback = sptr<INetStatsCallbackTest>();
    DelayedSingleton<NetStatsClient>::GetInstance()->UnregisterNetStatsCallback(callback);
}

void GetIfaceRxBytesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    std::string interfaceName = GetStringFromData(STR_LEN);
    DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceRxBytes(interfaceName);
}

void GetIfaceTxBytesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    std::string interfaceName = GetStringFromData(STR_LEN);
    int64_t ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceTxBytes(interfaceName);
    ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetCellularRxBytes();
    ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetCellularTxBytes();
    ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetAllRxBytes();
    ret = DelayedSingleton<NetStatsClient>::GetInstance()->GetAllTxBytes();
}

void GetUidRxBytesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    int32_t uid = GetData<int32_t>();
    DelayedSingleton<NetStatsClient>::GetInstance()->GetUidRxBytes(uid);
}

void GetUidTxBytesFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    int32_t uid = GetData<int32_t>();
    DelayedSingleton<NetStatsClient>::GetInstance()->GetUidTxBytes(uid);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::RegisterNetStatsCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterNetStatsCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::GetIfaceRxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetIfaceTxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetUidRxBytesFuzzTest(data, size);
    OHOS::NetManagerStandard::GetUidTxBytesFuzzTest(data, size);
    return 0;
}