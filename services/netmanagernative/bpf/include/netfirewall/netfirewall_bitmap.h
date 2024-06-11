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

static __always_inline void bitmap_clr(bitmap_ptr bitmap)
{
    for (int i = 0; i < BITMAP_LEN; i++) {
        bitmap[i] = 0;
    }
}

static __always_inline int bitmap_get(bitmap_ptr bitmap, uint32_t n)
{
    uint32_t len = BITMAP_BITS;
    if (n < len) {
        int i = n >> 5;   // n/32
        int j = n & 0x1f; // n%32

        return bitmap[i] & (1 << j);
    }
    return -1;
}

static __always_inline int bitmap_count(bitmap_ptr bitmap)
{
    int bits = BITMAP_BITS;
    int cnt = 0;
    for (int i = 0; i < bits; i++) {
        int flag = bitmap_get(bitmap, i);
        if (flag) {
            cnt++;
        }
    }
    return cnt;
}

static __always_inline void bitmap_and(bitmap_ptr bitmap, bitmap_ptr val)
{
    for (int i = 0; i < BITMAP_LEN; i++) {
        bitmap[i] &= val[i];
    }
}

static __always_inline void bitmap_or(bitmap_ptr bitmap, bitmap_ptr val)
{
    for (int i = 0; i < BITMAP_LEN; i++) {
        bitmap[i] |= val[i];
    }
}

#endif // NET_FIREWALL_BITMAP_H