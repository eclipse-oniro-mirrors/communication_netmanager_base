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

#include <securec.h>

#include "set_preset_domains_fuzzer.h"
#include "notify_callback_stub.h"
#include "singleton.h"
#include "system_ability_definition.h"
#include "bpf_def.h"
#include "netsys_native_service.h"
#include "netsys_ipc_interface_code.h"
#include "netmanager_base_test_security.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_presetDomainsFuzzData = nullptr;
size_t g_presetDomainsFuzzSize = 0;
size_t g_presetDomainsFuzzPos;
constexpr size_t STR_LEN = 10;
constexpr int32_t MAX_UID_NUM = 128;
constexpr int32_t MAX_DOMAIN_NUM = 128;
constexpr int32_t BOUND_NUM = 130;
constexpr int32_t NUMBER_ONE = 1;
constexpr int32_t NUMBER_THREE = 3;
} // namespace

template <class T> T SetPresetDomainsGetData()
{
    T object{};
    size_t objectSize = sizeof(object);
    if (g_presetDomainsFuzzData == nullptr || objectSize > g_presetDomainsFuzzSize - g_presetDomainsFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, g_presetDomainsFuzzData + g_presetDomainsFuzzPos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_presetDomainsFuzzPos += objectSize;
    return object;
}

std::string SetPresetDomainsGetString(size_t strLen)
{
    std::string str(strLen - 1, '\0');
    for (size_t i = 0; i < strLen - 1; i++) {
        str[i] = SetPresetDomainsGetData<char>();
    }
    return str;
}

static bool g_isInited = false;
__attribute__((no_sanitize("cfi"))) void Init()
{
    // The stub enforces CheckNetSysInternalPermission(NETSYS_INTERNAL), so a native token
    // holding the permission is required for the unpack branches to be reachable
    static NetManagerStandard::NetManagerBaseAccessToken token;
    g_isInited = DelayedSingleton<NetsysNative::NetsysNativeService>::GetInstance()->Init();
}

__attribute__((no_sanitize("cfi"))) int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    if (!g_isInited) {
        Init();
    }

    MessageParcel reply;
    MessageOption option;

    return DelayedSingleton<NetsysNative::NetsysNativeService>::GetInstance()->OnRemoteRequest(code, data, reply,
        option);
}

bool WriteInterfaceToken(MessageParcel &data)
{
    return data.WriteInterfaceToken(NetsysNative::NetsysNativeServiceStub::GetDescriptor());
}

bool IsDataAndSizeValid(const uint8_t *data, size_t size, MessageParcel &dataParcel)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    g_presetDomainsFuzzData = data;
    g_presetDomainsFuzzSize = size;
    g_presetDomainsFuzzPos = 0;

    return WriteInterfaceToken(dataParcel);
}

void SetPresetDomainsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    // Fuzzed uid num, bounded to cover both valid range and invalid boundary values
    int32_t uidNum = static_cast<int32_t>(SetPresetDomainsGetData<uint32_t>()) % BOUND_NUM - 1;
    dataParcel.WriteInt32(uidNum);
    for (int32_t uidIndex = 0; uidIndex < uidNum && uidIndex < MAX_UID_NUM; uidIndex++) {
        dataParcel.WriteUint32(SetPresetDomainsGetData<uint32_t>());
        // Fuzzed domain num, bounded to cover both valid range and invalid boundary values
        int32_t domainNum = static_cast<int32_t>(SetPresetDomainsGetData<uint32_t>()) % BOUND_NUM - 1;
        dataParcel.WriteInt32(domainNum);
        for (int32_t index = 0; index < domainNum && index < MAX_DOMAIN_NUM; index++) {
            dataParcel.WriteString(SetPresetDomainsGetString(STR_LEN));
        }
    }

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_PRESET_DOMAINS), dataParcel);
}

void SetPresetDomainsEmptyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    // Uid num is 0 - covers CmdSetPresetDomains branch: empty domains map
    dataParcel.WriteInt32(0);

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_PRESET_DOMAINS), dataParcel);
}

void SetPresetDomainsInvalidUidNumFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    // Negative uid num - covers CmdSetPresetDomains branch: uidNum < 0
    int32_t uidNum = -static_cast<int32_t>(SetPresetDomainsGetData<uint32_t>() % MAX_UID_NUM) - 1;
    dataParcel.WriteInt32(uidNum);

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_PRESET_DOMAINS), dataParcel);
}

void SetPresetDomainsExceedUidNumFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    // Uid num exceeds MAX_DOMAIN_NUM - covers CmdSetPresetDomains branch: uidNum > MAX_DOMAIN_NUM
    dataParcel.WriteInt32(MAX_DOMAIN_NUM + 1);

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_PRESET_DOMAINS), dataParcel);
}

void SetPresetDomainsExceedDomainNumFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    // Domain num exceeds MAX_DOMAIN_NUM - covers ReadPresetDomainsFromParcel branch: domainNum > MAX_DOMAIN_NUM
    dataParcel.WriteInt32(NUMBER_ONE);
    dataParcel.WriteUint32(SetPresetDomainsGetData<uint32_t>());
    dataParcel.WriteInt32(MAX_DOMAIN_NUM + 1);

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_PRESET_DOMAINS), dataParcel);
}

void SetPresetDomainsEmptyDomainFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    // Empty domain string - covers ReadPresetDomainsFromParcel branch: domain.empty()
    dataParcel.WriteInt32(NUMBER_ONE);
    dataParcel.WriteUint32(SetPresetDomainsGetData<uint32_t>());
    dataParcel.WriteInt32(NUMBER_ONE);
    dataParcel.WriteString("");

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_PRESET_DOMAINS), dataParcel);
}

void SetPresetDomainsTruncateFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    // Only uid num is written - covers ReadPresetDomainsFromParcel branch: read uid failed
    dataParcel.WriteInt32(NUMBER_ONE);

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_PRESET_DOMAINS), dataParcel);
}

void SetPresetDomainsInvalidDomainFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    // Domain longer than DNS_DOMAIN_NAME_SIZE - covers CmdSetPresetDomains branch: skip invalid domain
    dataParcel.WriteInt32(NUMBER_ONE);
    dataParcel.WriteUint32(SetPresetDomainsGetData<uint32_t>());
    dataParcel.WriteInt32(NUMBER_THREE);
    dataParcel.WriteString(std::string(DNS_DOMAIN_NAME_SIZE + 1, 'a'));
    // Domain with characters outside letters/digits/'.'/'-' - covers the charset validation branch
    dataParcel.WriteString("exam_ple.com");
    dataParcel.WriteString("example.com");

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_PRESET_DOMAINS), dataParcel);
}

void SetPresetDomainsTruncateUidNumFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    // Nothing written after the interface token - covers ReadPresetDomainsFromParcel branch: read uid num failed
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_PRESET_DOMAINS), dataParcel);
}

void SetPresetDomainsTruncateDomainNumFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    // Uid num and uid are written, domain num is missing - covers ReadPresetDomainsFromParcel branch: read
    // domain num failed
    dataParcel.WriteInt32(NUMBER_ONE);
    dataParcel.WriteUint32(SetPresetDomainsGetData<uint32_t>());

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_PRESET_DOMAINS), dataParcel);
}

void SetPresetDomainsInvalidDomainNumFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    // Negative domain num - covers ReadPresetDomainsFromParcel branch: domainNum < 0
    dataParcel.WriteInt32(NUMBER_ONE);
    dataParcel.WriteUint32(SetPresetDomainsGetData<uint32_t>());
    dataParcel.WriteInt32(-static_cast<int32_t>(SetPresetDomainsGetData<uint32_t>() % MAX_DOMAIN_NUM) - 1);

    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_PRESET_DOMAINS), dataParcel);
}

void SetPresetDomainsPermissionDeniedFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    if (!IsDataAndSizeValid(data, size, dataParcel)) {
        return;
    }

    // Token without NETSYS_INTERNAL permission - covers CmdSetPresetDomains branch: permission denied
    NetManagerBaseNoPermissionToken noPermissionToken;
    OnRemoteRequest(static_cast<uint32_t>(NetsysNative::NetsysInterfaceCode::NETSYS_SET_PRESET_DOMAINS), dataParcel);
}
} // namespace NetManagerStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::SetPresetDomainsFuzzTest(data, size);
    OHOS::NetManagerStandard::SetPresetDomainsEmptyFuzzTest(data, size);
    OHOS::NetManagerStandard::SetPresetDomainsInvalidUidNumFuzzTest(data, size);
    OHOS::NetManagerStandard::SetPresetDomainsExceedUidNumFuzzTest(data, size);
    OHOS::NetManagerStandard::SetPresetDomainsExceedDomainNumFuzzTest(data, size);
    OHOS::NetManagerStandard::SetPresetDomainsEmptyDomainFuzzTest(data, size);
    OHOS::NetManagerStandard::SetPresetDomainsTruncateFuzzTest(data, size);
    OHOS::NetManagerStandard::SetPresetDomainsInvalidDomainFuzzTest(data, size);
    OHOS::NetManagerStandard::SetPresetDomainsTruncateUidNumFuzzTest(data, size);
    OHOS::NetManagerStandard::SetPresetDomainsTruncateDomainNumFuzzTest(data, size);
    OHOS::NetManagerStandard::SetPresetDomainsInvalidDomainNumFuzzTest(data, size);
    OHOS::NetManagerStandard::SetPresetDomainsPermissionDeniedFuzzTest(data, size);
    return 0;
}
