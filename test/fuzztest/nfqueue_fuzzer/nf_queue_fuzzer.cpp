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

#include "message_parcel.h"
#include "message_option.h"
#include "netsys_ipc_interface_code.h"
#include "netsys_native_service_stub.h"

#ifdef FEATURE_NET_FIREWALL_ENABLE
#include "mock_netsys_native_service_stub.h"
#include "netfirewall_parcel.h"
#endif

#include "nf_queue_fuzzer.h"

namespace OHOS {
namespace NetsysNative {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos = 0;

template <class T> T NfQueueGetData()
{
    T object{};
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

void CheckParamValid(MessageParcel &dataParcel, const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    dataParcel.WriteInterfaceToken(NetsysNativeServiceStub::GetDescriptor());
}

__attribute__((no_sanitize("cfi"))) int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    MessageParcel reply;
    MessageOption option;
#ifdef FEATURE_NET_FIREWALL_ENABLE
    std::shared_ptr<NetsysNativeServiceStub> stub = std::make_shared<TestNetsysNativeServiceStub>();
    return stub->OnRemoteRequest(code, data, reply, option);
#else
    (void)code;
    (void)data;
    return 0;
#endif
}

void FuzzNfqQueueUnmarshalling(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    MessageParcel parcel;
    parcel.WriteBuffer(data, size);
    (void)NetManagerStandard::NfqQueue::Unmarshalling(parcel);
}

void FuzzNfqCtxUnmarshalling(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    MessageParcel parcel;
    parcel.WriteBuffer(data, size);
    (void)NetManagerStandard::NfqCtx::Unmarshalling(parcel);
}

void NfqOpenFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamValid(dataParcel, data, size);
    (void)OnRemoteRequest(static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NFQUEUE_OPEN), dataParcel);
}

void NfqCloseFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamValid(dataParcel, data, size);

    sptr<NetManagerStandard::NfqCtx> ctx = new (std::nothrow) NetManagerStandard::NfqCtx();
    if (ctx == nullptr) {
        return;
    }
    ctx->seq = NfQueueGetData<uint32_t>();
    ctx->Marshalling(dataParcel);

    (void)OnRemoteRequest(static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NFQUEUE_CLOSE), dataParcel);
}

void NfqBindPfFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamValid(dataParcel, data, size);

    sptr<NetManagerStandard::NfqCtx> ctx = new (std::nothrow) NetManagerStandard::NfqCtx();
    if (ctx == nullptr) {
        return;
    }
    ctx->seq = NfQueueGetData<uint32_t>();
    ctx->Marshalling(dataParcel);
    dataParcel.WriteUint16(NfQueueGetData<uint16_t>());

    (void)OnRemoteRequest(static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NFQUEUE_BIND_PF), dataParcel);
}

void NfqUnbindPfFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamValid(dataParcel, data, size);

    sptr<NetManagerStandard::NfqCtx> ctx = new (std::nothrow) NetManagerStandard::NfqCtx();
    if (ctx == nullptr) {
        return;
    }
    ctx->seq = NfQueueGetData<uint32_t>();
    ctx->Marshalling(dataParcel);
    dataParcel.WriteUint16(NfQueueGetData<uint16_t>());

    (void)OnRemoteRequest(static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NFQUEUE_UNBIND_PF), dataParcel);
}

void NfqQueueCreateFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamValid(dataParcel, data, size);

    sptr<NetManagerStandard::NfqCtx> ctx = new (std::nothrow) NetManagerStandard::NfqCtx();
    if (ctx == nullptr) {
        return;
    }
    ctx->seq = NfQueueGetData<uint32_t>();
    ctx->Marshalling(dataParcel);
    dataParcel.WriteUint16(NfQueueGetData<uint16_t>());

    (void)OnRemoteRequest(static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NFQUEUE_QUEUE_CREATE), dataParcel);
}

void NfqQueueDestroyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamValid(dataParcel, data, size);

    sptr<NetManagerStandard::NfqCtx> ctx = new (std::nothrow) NetManagerStandard::NfqCtx();
    sptr<NetManagerStandard::NfqQueue> q = new (std::nothrow) NetManagerStandard::NfqQueue();
    if (ctx == nullptr || q == nullptr) {
        return;
    }
    ctx->seq = NfQueueGetData<uint32_t>();
    q->queueNum = NfQueueGetData<uint16_t>();
    ctx->Marshalling(dataParcel);
    q->Marshalling(dataParcel);

    (void)OnRemoteRequest(static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NFQUEUE_QUEUE_DESTROY), dataParcel);
}

void NfqQueueSetModeFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamValid(dataParcel, data, size);

    sptr<NetManagerStandard::NfqCtx> ctx = new (std::nothrow) NetManagerStandard::NfqCtx();
    sptr<NetManagerStandard::NfqQueue> q = new (std::nothrow) NetManagerStandard::NfqQueue();
    if (ctx == nullptr || q == nullptr) {
        return;
    }
    ctx->seq = NfQueueGetData<uint32_t>();
    q->queueNum = NfQueueGetData<uint16_t>();
    ctx->Marshalling(dataParcel);
    q->Marshalling(dataParcel);
    dataParcel.WriteUint8(NfQueueGetData<uint8_t>());
    dataParcel.WriteUint32(NfQueueGetData<uint32_t>());

    (void)OnRemoteRequest(static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NFQUEUE_QUEUE_SET_MODE), dataParcel);
}

void NfqQueueSetMaxLenFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamValid(dataParcel, data, size);

    sptr<NetManagerStandard::NfqCtx> ctx = new (std::nothrow) NetManagerStandard::NfqCtx();
    sptr<NetManagerStandard::NfqQueue> q = new (std::nothrow) NetManagerStandard::NfqQueue();
    if (ctx == nullptr || q == nullptr) {
        return;
    }
    ctx->seq = NfQueueGetData<uint32_t>();
    q->queueNum = NfQueueGetData<uint16_t>();
    ctx->Marshalling(dataParcel);
    q->Marshalling(dataParcel);
    dataParcel.WriteUint32(NfQueueGetData<uint32_t>());

    (void)OnRemoteRequest(static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NFQUEUE_QUEUE_SET_MAX_LEN), dataParcel);
}

void NfqQueueSetFlagFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamValid(dataParcel, data, size);

    sptr<NetManagerStandard::NfqCtx> ctx = new (std::nothrow) NetManagerStandard::NfqCtx();
    sptr<NetManagerStandard::NfqQueue> q = new (std::nothrow) NetManagerStandard::NfqQueue();
    if (ctx == nullptr || q == nullptr) {
        return;
    }
    ctx->seq = NfQueueGetData<uint32_t>();
    q->queueNum = NfQueueGetData<uint16_t>();
    ctx->Marshalling(dataParcel);
    q->Marshalling(dataParcel);
    dataParcel.WriteUint32(NfQueueGetData<uint32_t>());
    dataParcel.WriteUint32(NfQueueGetData<uint32_t>());

    (void)OnRemoteRequest(static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NFQUEUE_QUEUE_SET_FLAG), dataParcel);
}

void NfqPktVerdictMarkFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel dataParcel;
    CheckParamValid(dataParcel, data, size);

    sptr<NetManagerStandard::NfqCtx> ctx = new (std::nothrow) NetManagerStandard::NfqCtx();
    sptr<NetManagerStandard::NfqQueue> q = new (std::nothrow) NetManagerStandard::NfqQueue();
    if (ctx == nullptr || q == nullptr) {
        return;
    }
    ctx->seq = NfQueueGetData<uint32_t>();
    q->queueNum = NfQueueGetData<uint16_t>();
    ctx->Marshalling(dataParcel);
    q->Marshalling(dataParcel);
    dataParcel.WriteUint32(NfQueueGetData<uint32_t>());
    dataParcel.WriteInt32(NfQueueGetData<int32_t>());
    dataParcel.WriteUint32(NfQueueGetData<uint32_t>());

    (void)OnRemoteRequest(static_cast<uint32_t>(NetsysInterfaceCode::NETSYS_NFQUEUE_PKT_VERDICT_MARK), dataParcel);
}
} // namespace
} // namespace NetsysNative

namespace NetManagerStandard {
void DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    OHOS::NetsysNative::FuzzNfqQueueUnmarshalling(data, size);
    OHOS::NetsysNative::FuzzNfqCtxUnmarshalling(data, size);
    OHOS::NetsysNative::NfqOpenFuzzTest(data, size);
    OHOS::NetsysNative::NfqCloseFuzzTest(data, size);
    OHOS::NetsysNative::NfqBindPfFuzzTest(data, size);
    OHOS::NetsysNative::NfqUnbindPfFuzzTest(data, size);
    OHOS::NetsysNative::NfqQueueCreateFuzzTest(data, size);
    OHOS::NetsysNative::NfqQueueDestroyFuzzTest(data, size);
    OHOS::NetsysNative::NfqQueueSetModeFuzzTest(data, size);
    OHOS::NetsysNative::NfqQueueSetMaxLenFuzzTest(data, size);
    OHOS::NetsysNative::NfqQueueSetFlagFuzzTest(data, size);
    OHOS::NetsysNative::NfqPktVerdictMarkFuzzTest(data, size);
}
} // namespace NetManagerStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::NetManagerStandard::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
