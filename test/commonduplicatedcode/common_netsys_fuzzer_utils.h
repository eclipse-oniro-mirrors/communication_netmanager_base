/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_NETSYS_FUZZER_UTILS_H
#define COMMON_NETSYS_FUZZER_UTILS_H

#include <array>
#include <securec.h>
#include <string>

#include "message_parcel.h"
#include "mock_netsys_native_service_stub.h"
#include "netsys_native_service_stub.h"

namespace OHOS {
namespace NetsysNative {
namespace NetsysFuzzer {
constexpr size_t STR_LEN = 10;
constexpr size_t MAX_VECTOR_LEN = 8;
constexpr uint32_t CONVERT_NUMBER_TO_BOOL = 2;

const uint8_t *g_fuzzData = nullptr;
size_t g_fuzzSize = 0;
size_t g_fuzzPos = 0;

inline void SetFuzzData(const uint8_t *data, size_t size)
{
    g_fuzzData = data;
    g_fuzzSize = size;
    g_fuzzPos = 0;
}

template <typename T> T GetData()
{
    T value {};
    if (g_fuzzData == nullptr || g_fuzzPos >= g_fuzzSize || sizeof(T) > g_fuzzSize - g_fuzzPos) {
        return value;
    }
    if (memcpy_s(&value, sizeof(T), g_fuzzData + g_fuzzPos, sizeof(T)) != EOK) {
        return T {};
    }
    g_fuzzPos += sizeof(T);
    return value;
}

inline std::string GetStringFromData()
{
    std::array<char, STR_LEN> cstr {};
    for (size_t i = 0; i + 1 < STR_LEN; i++) {
        cstr[i] = GetData<char>();
    }
    return std::string(cstr.data());
}

inline bool WriteInterfaceToken(MessageParcel &dataParcel)
{
    return dataParcel.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor());
}

// 进程内直调 stub（不发起真实 IPC）；服务方法由 TestNetsysNativeServiceStub mock，
// 避免内核面真实操作（netlink/BPF/iptables）。
__attribute__((no_sanitize("cfi"))) inline void DispatchRequest(uint32_t code, MessageParcel &dataParcel)
{
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<NetsysNativeServiceStub> stub = std::make_shared<TestNetsysNativeServiceStub>();
    (void)stub->OnRemoteRequest(code, dataParcel, reply, option);
}

inline bool IsValidFuzzData(const uint8_t *data, size_t size, MessageParcel &dataParcel)
{
    if (data == nullptr || size == 0) {
        return false;
    }
    SetFuzzData(data, size);
    return WriteInterfaceToken(dataParcel);
}
} // namespace NetsysFuzzer
} // namespace NetsysNative
} // namespace OHOS

#endif // COMMON_NETSYS_FUZZER_UTILS_H
