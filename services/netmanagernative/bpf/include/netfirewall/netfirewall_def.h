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

#ifndef NET_FIREWALL_DEF_H
#define NET_FIREWALL_DEF_H

#include "netfirewall_types.h"
#include "netfirewall_map_def.h"
#include "netfirewall_ct_def.h"

/* The per-dimension OTHER_*_KEY sentinel constants were removed:
 * wildcard fallback entries now live in the direction's wildcard map keyed by
 * dimension role, whose key space is structurally disjoint from rule values.
 * Do NOT reintroduce extreme-value sentinels in dimension maps. */

#endif // NET_FIREWALL_DEF_H