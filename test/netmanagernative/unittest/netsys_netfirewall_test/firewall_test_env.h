/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef NETMANAGER_BASE_FIREWALL_TEST_ENV_H
#define NETMANAGER_BASE_FIREWALL_TEST_ENV_H

#include <memory>

#include "bpf_netfirewall.h"

using OHOS::NetManagerStandard::NetsysBpfNetFirewall;

/*
 * Shared test-environment helpers for both firewall test TUs.
 *
 * Isolation contract (every case must be order-independent and residue-free):
 *  - Each case constructs its OWN instance via NewFirewallInstance(): the
 *    reconcile snapshots (appliedSnapshots_), intercept handler, DNS cache
 *    flush provider and callbacks are INSTANCE members and die with the
 *    case. The production singleton is only used by the fixtures
 *    (StartListener ring-buffer thread) and by the DNS prewarm cases —
 *    RunDnsPrewarmTask and the aging thread resolve through GetInstance()
 *    by production design, so those cases must run on the singleton and
 *    keep their explicit cleanup.
 *  - isBpfLoaded_ is a CLASS-STATIC (shared by every instance) and the
 *    constructor resets it to false, so the canonical per-case start state
 *    is "not loaded". A case that needs the loaded gate pins it with
 *    FirewallLoadScope, which restores exactly; a raw SetBpfLoaded toggle
 *    that is not restored poisons every later case (that was the whole
 *    WriteSrcPortBpfMap001-family failure mode).
 *  - The Write* family is NOT flag-gated: its return value depends only on
 *    map availability, which the fixture guarantees by loading the elf
 *    once per suite. Assertions are therefore unconditional — a broken
 *    environment fails loudly instead of passing vacuously.
 */

inline std::shared_ptr<NetsysBpfNetFirewall> NewFirewallInstance()
{
    return std::make_shared<NetsysBpfNetFirewall>();
}

/* RAII pin of the class-static load flag with exact save/restore. The
 * pinned(true) transition performs the real production hook work
 * (snapshot invalidation + loopback/built-in writes, all idempotent). */
class FirewallLoadScope {
public:
    explicit FirewallLoadScope(const std::shared_ptr<NetsysBpfNetFirewall> &fw, bool loaded = true)
        : fw_(fw), saved_(fw->IsBpfLoaded())
    {
        fw_->SetBpfLoaded(loaded);
    }
    ~FirewallLoadScope()
    {
        fw_->SetBpfLoaded(saved_);
    }
    FirewallLoadScope(const FirewallLoadScope &) = delete;
    FirewallLoadScope &operator=(const FirewallLoadScope &) = delete;

private:
    std::shared_ptr<NetsysBpfNetFirewall> fw_;
    bool saved_;
};

#endif // NETMANAGER_BASE_FIREWALL_TEST_ENV_H
