/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef NET_FIREWALL_BITMAP_H
#define NET_FIREWALL_BITMAP_H

#include <linux/bpf.h>

#include "netfirewall_types.h"

/* The verifier explores every loop back edge once per parent state.
 * These helpers sit behind the per-rule match chain, so a rolled word
 * loop here is re-verified BITMAP_LEN times for every distinct state
 * that reaches the chain — with the firewall enabled that alone burned
 * most of the 1M-insn complexity budget (E2BIG). Unroll the word loops
 * by 9 (BITMAP_LEN = 63 = 9 * 7, so no remainder iterations): the back
 * edges per call drop from 63 to 7, and the at most 9 concurrently
 * live words keep the BPF stack frame inside its 512-byte limit (a
 * full unroll hoists all 63 loads and overflows it). bitmap_positive
 * folds into a branch-free OR reduction first (same result: any
 * nonzero word means positive), so each unrolled body has no branches
 * at all. */
static __always_inline void bitmap_and(bitmap_ptr bitmap, bitmap_ptr val)
{
#pragma unroll 9
    for (int i = 0; i < BITMAP_LEN; i++) {
        bitmap[i] &= val[i];
    }
}

static __always_inline void bitmap_and_inv(bitmap_ptr bitmap, bitmap_ptr val)
{
#pragma unroll 9
    for (int i = 0; i < BITMAP_LEN; i++) {
        bitmap[i] &= (~val[i]);
    }
}

static __always_inline bool bitmap_positive(bitmap_ptr bitmap)
{
    __u32 any = 0;
#pragma unroll 9
    for (int i = 0; i < BITMAP_LEN; i++) {
        any |= bitmap[i];
    }
    return any != 0;
}

#endif // NET_FIREWALL_BITMAP_H