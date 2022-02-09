/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

import {AsyncCallback, Callback} from "./basic";
import connection from "./@ohos.net.connection";

/**
 * Provides interfaces to manage network policy rules.
 *
 * @since 8
 * @sysCap SystemCapability.Communication.NetManager
 * @devices phone, tablet, tv, wearable, car
 */
declare namespace policy {
  type NetBearType = connection.NetBearType;

  /**
   * Set the background policy.
   *
   * @param allow Indicates whether the background appications are allowed to access network.
   * @permission ohos.permission.SET_NETWORK_POLICY
   * @systemapi Hide this for inner system use.
   */
  function setBackgroundPolicy(allow: boolean, callback: AsyncCallback<void>): void ;
  function setBackgroundPolicy(allow: boolean): Promise<void>;

  /**
   * Query the background policy.
   *
   * @param callback Returns the background policy.
   *      For details, see {@link BackgroundPolicy#BACKGROUND_POLICY_DISABLE}.
   * @permission ohos.permission.GET_NETWORK_POLICY
   * @systemapi Hide this for inner system use.
   */
  function getBackgroundPolicy(callback: AsyncCallback<BackgroundPolicy>): void;
  function getBackgroundPolicy(): Promise<BackgroundPolicy>;

  /**
   * Set policy for the specified UID.
   *
   * @param uid the specified UID of application.
   * @param policy the policy of the current UID of application.
   *      For details, see {@link NetUidPolicy}.
   * @permission ohos.permission.SET_NETWORK_POLICY
   * @systemapi Hide this for inner system use.
   */
  function setPolicyByUid(uid: number, policy: NetUidPolicy, callback: AsyncCallback<void>): void;
  function setPolicyByUid(uid: number, policy: NetUidPolicy): Promise<void>;

  /**
   * Query the policy of the specified UID.
   *
   * @param uid the specified UID of application.
   * @param callback Returns the policy of the current UID of application.
   *      For details, see {@link NetUidPolicy}.
   * @permission ohos.permission.GET_NETWORK_POLICY
   * @systemapi Hide this for inner system use.
   */
  function getPolicyByUid(uid: number, callback: AsyncCallback<NetUidPolicy>): void;
  function getPolicyByUid(uid: number): Promise<NetUidPolicy>;

  /**
   * Query the application UIDs of the specified policy.
   *
   * @param policy the policy of the current UID of application.
   *      For details, see {@link NetUidPolicy}.
   * @param callback Returns the UIDs of the specified policy.
   * @permission ohos.permission.GET_NETWORK_POLICY
   * @systemapi Hide this for inner system use.
   */
  function getUidsByPolicy(policy: NetUidPolicy, callback: AsyncCallback<Array<number>>): void;
  function getUidsByPolicy(policy: NetUidPolicy): Promise<Array<number>>;

  /**
   * Register and unregister network policy listener.
   *
   * @permission ohos.permission.CONNECTIVITY_INTERNAL
   * @systemapi Hide this for inner system use.
   */
  function on(type: 'netUidPolicyChange', callback: Callback<{ uid: number, policy: NetUidPolicy }>): void;

  /**
   * @systemapi Hide this for inner system use.
   */
  function off(type: 'netUidPolicyChange', callback?: Callback<{ uid: number, policy: NetUidPolicy }>): void;

  /**
   * Get network policies.
   *
   * @return See {@link NetPolicyQuotaPolicy}.
   * @permission ohos.permission.GET_NETWORK_POLICY
   * @systemapi Hide this for inner system use.
   */
  function getNetQuotaPolicies(callback: AsyncCallback<Array<NetPolicyQuotaPolicy>>): void;
  function getNetQuotaPolicies(): Promise<Array<NetPolicyQuotaPolicy>>;

  /**
   * Set network policies.
   *
   * @param quotaPolicies Indicates {@link NetPolicyQuotaPolicy}.
   * @permission ohos.permission.SET_NETWORK_POLICY
   * @systemapi Hide this for inner system use.
   */
  function setNetQuotaPolicies(quotaPolicies: Array<NetPolicyQuotaPolicy>, callback: AsyncCallback<void>): void;
  function setNetQuotaPolicies(quotaPolicies: Array<NetPolicyQuotaPolicy>): Promise<void>;

  /**
   * Temporarily deactivate the specified network management policy.
   *
   * @param simId Indicates the specified sim that is valid when netType is cellular.
   * @param netType Indicates the {@link NetBearType}.
   * @permission ohos.permission.SET_NETWORK_POLICY
   * @systemapi Hide this for inner system use.
   */
  function setSnoozePolicy(simId: number, netType: NetBearType, callback: AsyncCallback<void>): void;
  function setSnoozePolicy(simId: number, netType: NetBearType): Promise<void>;

  /**
   * Reset the specified network management policy.
   *
   * @param simId Indicates the specified sim that is valid when netType is cellular.
   * @permission ohos.permission.SET_NETWORK_POLICY
   * @systemapi Hide this for inner system use.
   */
  function setFactoryPolicy(simId: number, callback: AsyncCallback<void>): void;
  function setFactoryPolicy(simId: number): Promise<void>;

  export enum BackgroundPolicy {
    /**
     * Indicates that applications can use metered networks.
     */
    BACKGROUND_POLICY_DISABLE = 1,

    /**
     * Indicates that only applications in the allowlist can use metered networks.
     */
    BACKGROUND_POLICY_ALLOWLISTED = 2,

    /**
     * Indicates that applications cannot use metered networks.
     */
    BACKGROUND_POLICY_ENABLED = 3
  }

  /**
   * @systemapi Hide this for inner system use.
   */
  export interface NetPolicyQuotaPolicy {
    /* netType value range in NetBearType */
    netType: NetBearType;
    /* The ID of the target card, valid when netType is BEARER_CELLULAR. */
    simId: number;
    /*  Time rubbing, for example:1636598990 */
    periodStartTime: number;
    /* Unit: The cycle starts on one day of month, for example: M1 Indicates the 1st of each month. */
    periodDuration: string;
    /* Alarm threshold */
    warningBytes: number;
    /* Limit threshold */
    limitBytes: number;
    /* Time rubbing, for example:1636598990, -1 Indicates the policy need not snooze */
    lastLimitSnooze?: number;
    /* @see{MeteringMode} */
    metered?: MeteringMode;
  }

  /**
   * @systemapi Hide this for inner system use.
   */
  export enum NetUidPolicy {
    NET_POLICY_NONE = 0,
    NET_POLICY_ALLOW_METERED_BACKGROUND = 1 << 0,
    NET_POLICY_TEMPORARY_ALLOW_METERED = 1 << 1,
    NET_POLICY_REJECT_METERED_BACKGROUND = 1 << 2,
    NET_POLICY_ALLOW_METERED = 1 << 3,
    NET_POLICY_REJECT_METERED = 1 << 4,
    NET_POLICY_ALLOW_ALL = 1 << 5,
    NET_POLICY_REJECT_ALL = 1 << 6
  }

  /**
   * @systemapi Hide this for inner system use.
   */
  export enum MeteringMode {
    /* non metering */
    UN_METERED = 0,
    /* metering */
    METERED = 1
  }
}

export default policy;