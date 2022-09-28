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
#include "net_stats_csv.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_base_fuzz_data = nullptr;
size_t g_base_fuzz_size = 0;
size_t g_base_fuzz_pos;
constexpr size_t STR_LEN = 10;
}

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_base_fuzz_data == nullptr || objectSize > g_base_fuzz_size - g_base_fuzz_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, g_base_fuzz_data + g_base_fuzz_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_base_fuzz_pos += objectSize;
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

void GetIfaceStatsDetailFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_base_fuzz_data = data;
    g_base_fuzz_size = size;
    g_base_fuzz_pos = 0;

    std::string iface = GetStringFromData(STR_LEN);
    uint32_t start = GetData<uint32_t>();
    uint32_t end = GetData<uint32_t>();
    NetStatsInfo statsInfo;
    DelayedSingleton<NetStatsClient>::GetInstance()->GetIfaceStatsDetail(iface, start, end, statsInfo);
}

void GetUidStatsDetailFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_base_fuzz_data = data;
    g_base_fuzz_size = size;
    g_base_fuzz_pos = 0;

    std::string iface = GetStringFromData(STR_LEN);
    uint32_t start = GetData<uint32_t>();
    uint32_t uid = GetData<uint32_t>();
    uint32_t end = GetData<uint32_t>();
    NetStatsInfo statsInfo;
    DelayedSingleton<NetStatsClient>::GetInstance()->GetUidStatsDetail(iface, uid, start, end, statsInfo);
}

void UpdateIfacesStatsFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    g_base_fuzz_data = data;
    g_base_fuzz_size = size;
    g_base_fuzz_pos = 0;

    std::string iface = GetStringFromData(STR_LEN);
    uint32_t start = GetData<uint32_t>();
    uint32_t end = GetData<uint32_t>();
    NetStatsInfo stats;
    DelayedSingleton<NetStatsClient>::GetInstance()->UpdateIfacesStats(iface, start, end, stats);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::GetIfaceStatsDetailFuzzTest(data, size);
    OHOS::NetManagerStandard::GetUidStatsDetailFuzzTest(data, size);
    OHOS::NetManagerStandard::UpdateIfacesStatsFuzzTest(data, size);
    OHOS::NetManagerStandard::RegisterNetStatsCallbackFuzzTest(data, size);
    OHOS::NetManagerStandard::UnregisterNetStatsCallbackFuzzTest(data, size);
    return 0;
}