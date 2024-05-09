/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "bpf_ring_buffer.h"
#include "net_policy_client.h"
#include "libbpf.h"
#include "ffrt.h"
#include "ffrt_inner.h"

namespace OHOS::NetManagerStandard {
namespace {
    const int RING_BUFFER_POLL_TIME_OUT_MS = -1;
}
bool NetsysBpfRingBuffer::existThread_ = true;

uint64_t NetsysBpfRingBuffer::BpfMapPathNameToU64(const std::string &pathName)
{
    return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(pathName.c_str()));
}

int32_t NetsysBpfRingBuffer::BpfSyscall(int32_t cmd, const bpf_attr &attr)
{
    return static_cast<int32_t>(syscall(__NR_bpf, cmd, &attr, sizeof(attr)));
}

int NetsysBpfRingBuffer::GetRingbufFd(const std::string &path, uint32_t fileFlags)
{
    bpf_attr bpfAttr{};
    memset_s(&bpfAttr, sizeof(bpfAttr), 0, sizeof(bpfAttr));
    bpfAttr.pathname = BpfMapPathNameToU64(path);
    bpfAttr.file_flags = fileFlags;
    return BpfSyscall(BPF_OBJ_GET, bpfAttr);
}

int NetsysBpfRingBuffer::HandleNetworkPolicyEventCallback(void *ctx, void *data, size_t data_sz)
{
    NETNATIVE_LOG_D("HandleNetworkPolicyEventCallback enter");
    int32_t *e = (int32_t*)data;

    if (NetPolicyClient::GetInstance().NotifyNetAccessPolicyDiag(*e) != NETMANAGER_SUCCESS) {
        NETNATIVE_LOGE("Notify to diag fail");
        return RING_BUFFER_ERR_INTERNAL;
    }

    return RING_BUFFER_ERR_NONE;
}

void NetsysBpfRingBuffer::ListenRingBufferThread(void)
{
    auto ringbufFd = GetRingbufFd(RING_BUFFER_MAP_PATH, 0);
    if (ringbufFd > 0) {
        struct ring_buffer *rb = NULL;
        int err = 0;
        /* Set up ring buffer polling */
        rb = ring_buffer__new(ringbufFd, HandleNetworkPolicyEventCallback, NULL, NULL);
        if (!rb) {
            err = -1;
            NETNATIVE_LOGE("Bpf ring buffer new fail");
            return;
        }

        /* Process events */
        while (existThread_) {
            if (ffrt::this_task::get_id() != 0) {
                ffrt::sync_io(ringbufFd);
            }
            err = ring_buffer__poll(rb, RING_BUFFER_POLL_TIME_OUT_MS);
            if (err < 0) {
                NETNATIVE_LOGE("Bpf ring buffer poll fail");
                break;
            }
        }

        ring_buffer__free(rb);
    }

    NETNATIVE_LOGE("Could not get bpf ring buffer map");
    return;
}

void NetsysBpfRingBuffer::ListenNetworkAccessPolicyEvent(void)
{
    ffrt::submit(ListenRingBufferThread, {}, {}, ffrt::task_attr().name("ListenRingBufferThread"));
}

void NetsysBpfRingBuffer::ExistRingBufferPoll(void)
{
    existThread_ = false;
}
}
