/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef POLICY_IPC_INTERFACE_CODE_H
#define POLICY_IPC_INTERFACE_CODE_H

/* SAID: 1152 */
namespace OHOS {
namespace NetManagerStandard {
enum class PolicyInterfaceCode {
    CMD_NPS_START = 0,
    CMD_NPS_SET_POLICY_BY_UID,
    CMD_NPS_GET_POLICY_BY_UID,
    CMD_NPS_GET_UIDS_BY_POLICY,
    CMD_NPS_IS_NET_ALLOWED_BY_METERED,
    CMD_NPS_IS_NET_ALLOWED_BY_IFACE,
    CMD_NPS_REGISTER_NET_POLICY_CALLBACK,
    CMD_NPS_UNREGISTER_NET_POLICY_CALLBACK,
    CMD_NPS_SET_NET_QUOTA_POLICIES,
    CMD_NPS_GET_NET_QUOTA_POLICIES,
    CMD_NPS_UPDATE_REMIND_POLICY,
    CMD_NPS_SET_IDLE_TRUSTLIST,
    CMD_NPS_GET_IDLE_TRUSTLIST,
    CMD_NPS_SET_DEVICE_IDLE_POLICY,
    CMD_NPS_RESET_POLICIES,
    CMD_NPS_SET_BACKGROUND_POLICY,
    CMD_NPS_GET_BACKGROUND_POLICY,
    CMD_NPS_GET_BACKGROUND_POLICY_BY_UID,
    CMD_NPS_SET_POWER_SAVE_TRUSTLIST,
    CMD_NPS_GET_POWER_SAVE_TRUSTLIST,
    CMD_NPS_SET_POWER_SAVE_POLICY,
    CMD_NPS_END = 100,
};
}
} // namespace OHOS
#endif // POLICY_IPC_INTERFACE_CODE_H