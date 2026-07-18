/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

import { BusinessError } from '@ohos.base';
import type { AsyncCallback, Callback } from '@ohos.base';
import type connection from '@ohos.net.connection';
import Context from 'application.Context';

export default namespace policy {
  loadLibrary("policy_ani");

  export type NetBearType = connection.NetBearType;

  export native function setBackgroundAllowedSync(isAllowed: boolean): void;

  export function setBackgroundAllowed(isAllowed: boolean, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): void => {
      return setBackgroundAllowedSync(isAllowed);
    })
    p1.then(() => {
      callback(new BusinessError(), undefined);
    }, (err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  export function setBackgroundAllowed(isAllowed: boolean): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return setBackgroundAllowedSync(isAllowed);
      }).then(() => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function isBackgroundAllowedSync(): boolean;

  export function isBackgroundAllowed(callback: AsyncCallback<boolean>): void {
    let p1 = taskpool.execute((): boolean => {
      return isBackgroundAllowedSync();
    })
    p1.then((content: Any) => {
      callback(new BusinessError(), content as boolean);
    }, (err: Error): void => {
      callback(err as BusinessError, false);
    });
  }

  export function isBackgroundAllowed(): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
      taskpool.execute((): boolean => {
        return isBackgroundAllowedSync();
      }).then((content: Any) => {
        resolve(content as boolean);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getBackgroundPolicyByUidSync(uid: int): NetBackgroundPolicy;

  export function getBackgroundPolicyByUid(uid: int, callback: AsyncCallback<NetBackgroundPolicy>): void {
    let p1 = taskpool.execute((): NetBackgroundPolicy => {
      return getBackgroundPolicyByUidSync(uid);
    })
    p1.then((content: Any) => {
      callback(new BusinessError(), content as NetBackgroundPolicy);
    }, (err: Error): void => {
      callback(err as BusinessError, NetBackgroundPolicy.NET_BACKGROUND_POLICY_NONE);
    });
  }

  export function getBackgroundPolicyByUid(uid: int): Promise<NetBackgroundPolicy> {
    return new Promise<NetBackgroundPolicy>((resolve, reject) => {
      taskpool.execute((): NetBackgroundPolicy => {
        return getBackgroundPolicyByUidSync(uid);
      }).then((content: Any) => {
        resolve(content as NetBackgroundPolicy);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function setPolicyByUidSync(uid: int, policy: NetUidPolicy): void;

  export function setPolicyByUid(uid: int, policy: NetUidPolicy, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): void => {
      return setPolicyByUidSync(uid, policy);
    })
    p1.then(() => {
      callback(new BusinessError(), undefined);
    }, (err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  export function setPolicyByUid(uid: int, policy: NetUidPolicy): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return setPolicyByUidSync(uid, policy);
      }).then(() => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getPolicyByUidSync(uid: int): NetUidPolicy;

  export function getPolicyByUid(uid: int, callback: AsyncCallback<NetUidPolicy>): void {
    let p1 = taskpool.execute((): NetUidPolicy => {
      return getPolicyByUidSync(uid);
    })
    p1.then((content: Any) => {
      callback(new BusinessError(), content as NetUidPolicy);
    }, (err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  export function getPolicyByUid(uid: int): Promise<NetUidPolicy> {
    return new Promise<NetUidPolicy>((resolve, reject) => {
      taskpool.execute((): NetUidPolicy => {
        return getPolicyByUidSync(uid);
      }).then((content: Any) => {
        resolve(content as NetUidPolicy);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getUidsByPolicySync(policy: NetUidPolicy): Array<int>;

  export function getUidsByPolicy(policy: NetUidPolicy, callback: AsyncCallback<Array<int>>): void {
    let p1 = taskpool.execute((): Array<int> => {
      return getUidsByPolicySync(policy);
    })
    p1.then((content: Any) => {
      callback(new BusinessError(), content as Array<int>);
    }, (err: Error): void => {
      callback(err as BusinessError, new Array<int>());
    });
  }

  export function getUidsByPolicy(policy: NetUidPolicy): Promise<Array<int>> {
    return new Promise<Array<int>>((resolve, reject) => {
      taskpool.execute((): Array<int> => {
        return getUidsByPolicySync(policy);
      }).then((content: Any) => {
        resolve(content as Array<int>);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function isUidNetAllowedSync(uid: int, isMetered: boolean): boolean;

  export function isUidNetAllowed(uid: int, isMetered: boolean, callback: AsyncCallback<boolean>): void {
    let p1 = taskpool.execute((): boolean => {
      return isUidNetAllowedSync(uid, isMetered);
    })
    p1.then((content: Any) => {
      callback(new BusinessError(), content as boolean);
    }, (err: Error): void => {
      callback(err as BusinessError, false);
    });
  }

  export function isUidNetAllowed(uid: int, isMetered: boolean): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
      taskpool.execute((): boolean => {
        return isUidNetAllowedSync(uid, isMetered);
      }).then((content: Any) => {
        resolve(content as boolean);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function isUidNetAllowedByIfaceSync(uid: int, iface: string): boolean;

  export function isUidNetAllowed(uid: int, iface: string, callback: AsyncCallback<boolean>): void {
    let p1 = taskpool.execute((): boolean => {
      return isUidNetAllowedByIfaceSync(uid, iface);
    })
    p1.then((content: Any) => {
      callback(new BusinessError(), content as boolean);
    }, (err: Error): void => {
      callback(err as BusinessError, false);
    });
  }

  export function isUidNetAllowed(uid: int, iface: string): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
      taskpool.execute((): boolean => {
        return isUidNetAllowedByIfaceSync(uid, iface);
      }).then((content: Any) => {
        resolve(content as boolean);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function setNetQuotaPoliciesSync(quotaPolicies: Array<NetQuotaPolicy>): void;

  export function setNetQuotaPolicies(quotaPolicies: Array<NetQuotaPolicy>, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): void => {
      return setNetQuotaPoliciesSync(quotaPolicies);
    })
    p1.then(() => {
      callback(new BusinessError(), undefined);
    }, (err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  export function setNetQuotaPolicies(quotaPolicies: Array<NetQuotaPolicy>): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return setNetQuotaPoliciesSync(quotaPolicies);
      }).then(() => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function restoreAllPoliciesSync(iccid: string): void;

  export function restoreAllPolicies(iccid: string): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return restoreAllPoliciesSync(iccid);
      }).then(() => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getNetQuotaPoliciesSync(): Array<NetQuotaPolicy>;

  export function getNetQuotaPolicies(callback: AsyncCallback<Array<NetQuotaPolicy>>): void {
    let p1 = taskpool.execute((): Array<NetQuotaPolicy> => {
      return getNetQuotaPoliciesSync();
    })
    p1.then((content: Any) => {
      callback(new BusinessError(), content as Array<NetQuotaPolicy>);
    }, (err: Error): void => {
      callback(err as BusinessError, new Array<NetQuotaPolicy>());
    });
  }

  export function getNetQuotaPolicies(): Promise<Array<NetQuotaPolicy>> {
    return new Promise<Array<NetQuotaPolicy>>((resolve, reject) => {
      taskpool.execute((): Array<NetQuotaPolicy> => {
        return getNetQuotaPoliciesSync();
      }).then((content: Any) => {
        resolve(content as Array<NetQuotaPolicy>);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function updateRemindPolicySync(netType: NetBearType, simId: string, remindType: RemindType): void;

  export function updateRemindPolicy(netType: NetBearType, simId: string, remindType: RemindType, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): void => {
      return updateRemindPolicySync(netType, simId, remindType);
    })
    p1.then(() => {
      callback(new BusinessError(), undefined);
    }, (err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  export function updateRemindPolicy(netType: NetBearType, simId: string, remindType: RemindType): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return updateRemindPolicySync(netType, simId, remindType);
      }).then(() => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function setDeviceIdleTrustlistSync(uids: Array<int>, isAllowed: boolean): void;

  export function setDeviceIdleTrustlist(uids: Array<int>, isAllowed: boolean, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): void => {
      return setDeviceIdleTrustlistSync(uids, isAllowed);
    })
    p1.then(() => {
      callback(new BusinessError(), undefined);
    }, (err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  export function setDeviceIdleTrustlist(uids: Array<int>, isAllowed: boolean): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return setDeviceIdleTrustlistSync(uids, isAllowed);
      }).then(() => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getDeviceIdleTrustlistSync(): Array<int>;

  export function getDeviceIdleTrustlist(callback: AsyncCallback<Array<int>>): void {
    let p1 = taskpool.execute((): Array<int> => {
      return getDeviceIdleTrustlistSync();
    })
    p1.then((content: Any) => {
      callback(new BusinessError(), content as Array<int>);
    }, (err: Error): void => {
      callback(err as BusinessError, new Array<int>());
    });
  }

  export function getDeviceIdleTrustlist(): Promise<Array<int>> {
    return new Promise<Array<int>>((resolve, reject) => {
      taskpool.execute((): Array<int> => {
        return getDeviceIdleTrustlistSync();
      }).then((content: Any) => {
        resolve(content as Array<int>);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function setPowerSaveTrustlistSync(uids: Array<int>, isAllowed: boolean): void;

  export function setPowerSaveTrustlist(uids: Array<int>, isAllowed: boolean, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): void => {
      return setPowerSaveTrustlistSync(uids, isAllowed);
    })
    p1.then(() => {
      callback(new BusinessError(), undefined);
    }, (err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  export function setPowerSaveTrustlist(uids: Array<int>, isAllowed: boolean): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return setPowerSaveTrustlistSync(uids, isAllowed);
      }).then(() => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getPowerSaveTrustlistSync(): Array<int>;

  export function getPowerSaveTrustlist(callback: AsyncCallback<Array<int>>): void {
    let p1 = taskpool.execute((): Array<int> => {
      return getPowerSaveTrustlistSync();
    })
    p1.then((content: Any) => {
      callback(new BusinessError(), content as Array<int>);
    }, (err: Error): void => {
      callback(err as BusinessError, new Array<int>());
    });
  }

  export function getPowerSaveTrustlist(): Promise<Array<int>> {
    return new Promise<Array<int>>((resolve, reject) => {
      taskpool.execute((): Array<int> => {
        return getPowerSaveTrustlistSync();
      }).then((content: Any) => {
        resolve(content as Array<int>);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function resetPoliciesSync(simId: string): void;

  export function resetPolicies(simId: string, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): void => {
      return resetPoliciesSync(simId);
    })
    p1.then(() => {
      callback(new BusinessError(), undefined);
    }, (err: Error): void => {
      callback(err as BusinessError, undefined);
    });
  }

  export function resetPolicies(simId: string): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return resetPoliciesSync(simId);
      }).then(() => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function setNetworkAccessPolicySync(uid: int, policy: NetworkAccessPolicy, isReconfirmed?: boolean): void;

  export function setNetworkAccessPolicy(uid: int, policy: NetworkAccessPolicy, isReconfirmed?: boolean): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return setNetworkAccessPolicySync(uid, policy, isReconfirmed);
      }).then(() => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getNetworkAccessPolicyByUidSync(uid: int): NetworkAccessPolicy;

  export function getNetworkAccessPolicy(uid: int): Promise<NetworkAccessPolicy> {
    return new Promise<NetworkAccessPolicy>((resolve, reject) => {
      taskpool.execute((): NetworkAccessPolicy => {
        return getNetworkAccessPolicyByUidSync(uid);
      }).then((content: Any) => {
        resolve(content as NetworkAccessPolicy);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getNetworkAccessPolicySync(): UidNetworkAccessPolicy;

  export function getNetworkAccessPolicy(): Promise<UidNetworkAccessPolicy> {
    return new Promise<UidNetworkAccessPolicy>((resolve, reject) => {
      taskpool.execute((): UidNetworkAccessPolicy => {
        return getNetworkAccessPolicySync();
      }).then((content: Any) => {
        resolve(content as UidNetworkAccessPolicy);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getNetAccessPolicySync(): NetAccessPolicy;

  export function getNetAccessPolicy(): Promise<NetAccessPolicy> {
    return new Promise<NetAccessPolicy>((resolve, reject) => {
      taskpool.execute((): NetAccessPolicy => {
        return getNetAccessPolicySync();
      }).then((content: Any) => {
        resolve(content as NetAccessPolicy);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function onNetUidPolicyChangeSync(callback: Callback<NetUidPolicyInfo>): void;
  export native function offNetUidPolicyChangeSync(callback?: Callback<NetUidPolicyInfo>): void;

  export function onNetUidPolicyChange(callback: Callback<NetUidPolicyInfo>): void {
    onNetUidPolicyChangeSync(callback);
  }

  export function offNetUidPolicyChange(callback?: Callback<NetUidPolicyInfo>): void {
    if (callback === undefined) {
      offNetUidPolicyChangeSync();
    } else {
      offNetUidPolicyChangeSync(callback);
    }
  }

  export native function onNetUidRuleChangeSync(callback: Callback<NetUidRuleInfo>): void;
  export native function offNetUidRuleChangeSync(callback?: Callback<NetUidRuleInfo>): void;

  export function onNetUidRuleChange(callback: Callback<NetUidRuleInfo>): void {
    onNetUidRuleChangeSync(callback);
  }

  export function offNetUidRuleChange(callback?: Callback<NetUidRuleInfo>): void {
    if (callback === undefined) {
      offNetUidRuleChangeSync();
    } else {
      offNetUidRuleChangeSync(callback);
    }
  }

  export native function onNetMeteredIfacesChangeSync(callback: Callback<Array<string>>): void;
  export native function offNetMeteredIfacesChangeSync(callback?: Callback<Array<string>>): void;

  export function onNetMeteredIfacesChange(callback: Callback<Array<string>>): void {
    onNetMeteredIfacesChangeSync(callback);
  }

  export function offNetMeteredIfacesChange(callback?: Callback<Array<string>>): void {
    if (callback === undefined) {
      offNetMeteredIfacesChangeSync();
    } else {
      offNetMeteredIfacesChangeSync(callback);
    }
  }

  export native function onNetQuotaPolicyChangeSync(callback: Callback<Array<NetQuotaPolicy>>): void;
  export native function offNetQuotaPolicyChangeSync(callback?: Callback<Array<NetQuotaPolicy>>): void;

  export function onNetQuotaPolicyChange(callback: Callback<Array<NetQuotaPolicy>>): void {
    onNetQuotaPolicyChangeSync(callback);
  }

  export function offNetQuotaPolicyChange(callback?: Callback<Array<NetQuotaPolicy>>): void {
    if (callback === undefined) {
      offNetQuotaPolicyChangeSync();
    } else {
      offNetQuotaPolicyChangeSync(callback);
    }
  }

  export native function onNetBackgroundPolicyChangeSync(callback: Callback<boolean>): void;
  export native function offNetBackgroundPolicyChangeSync(callback?: Callback<boolean>): void;

  export function onNetBackgroundPolicyChange(callback: Callback<boolean>): void {
    onNetBackgroundPolicyChangeSync(callback);
  }

  export function offNetBackgroundPolicyChange(callback?: Callback<boolean>): void {
    if (callback === undefined) {
      offNetBackgroundPolicyChangeSync();
    } else {
      offNetBackgroundPolicyChangeSync(callback);
    }
  }

  export native function showAppNetPolicySettingsSync(context: Context): void;

  export function showAppNetPolicySettings(context: Context): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return showAppNetPolicySettingsSync(context);
      }).then(() => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export enum NetBackgroundPolicy {
    NET_BACKGROUND_POLICY_NONE = 0,
    NET_BACKGROUND_POLICY_ENABLE = 1,
    NET_BACKGROUND_POLICY_DISABLE = 2,
    NET_BACKGROUND_POLICY_TRUSTLIST = 3
  }

  export interface NetQuotaPolicy {
    networkMatchRule: NetworkMatchRule;
    quotaPolicy: QuotaPolicy;
  }

  export class NetQuotaPolicyInner implements NetQuotaPolicy {
    networkMatchRule: NetworkMatchRule;
    quotaPolicy: QuotaPolicy;
  }

  export interface QuotaPolicy {
    periodDuration: string;
    warningBytes: long;
    limitBytes: long;
    metered: boolean;
    limitAction: LimitAction;
    lastWarningRemind?: long;
    lastLimitRemind?: long;
  }

  export class QuotaPolicyInner implements QuotaPolicy {
    periodDuration: string;
    warningBytes: long;
    limitBytes: long;
    metered: boolean;
    limitAction: LimitAction;
    lastWarningRemind?: long;
    lastLimitRemind?: long;
  }

  export interface NetworkMatchRule {
    netType: NetBearType;
    identity: string;
    simId: string;
  }

  export class NetworkMatchRuleInner implements NetworkMatchRule {
    netType: NetBearType;
    identity: string;
    simId: string;
  }

  export interface NetUidRuleInfo {
    uid: int;
    rule: NetUidRule;
  }

  export class NetUidRuleInfoInner implements NetUidRuleInfo {
    uid: int;
    rule: NetUidRule;
  }

  export interface NetUidPolicyInfo {
    uid: int;
    policy: NetUidPolicy;
  }

  export class NetUidPolicyInfoInner implements NetUidPolicyInfo {
    uid: int;
    policy: NetUidPolicy;
  }

  export enum LimitAction {
    LIMIT_ACTION_NONE = -1,
    LIMIT_ACTION_ACCESS_DISABLED = 0,
    LIMIT_ACTION_ALERT_ONLY = 1
  }

  export enum NetUidRule {
    NET_RULE_NONE = 0,
    NET_RULE_ALLOW_METERED_FOREGROUND = 1 << 0,
    NET_RULE_ALLOW_METERED = 1 << 1,
    NET_RULE_REJECT_METERED = 1 << 2,
    NET_RULE_ALLOW_ALL = 1 << 5,
    NET_RULE_REJECT_ALL = 1 << 6
  }

  export enum RemindType {
    REMIND_TYPE_WARNING = 1,
    REMIND_TYPE_LIMIT = 2
  }

  export enum NetUidPolicy {
    NET_POLICY_NONE = 0,
    NET_POLICY_ALLOW_METERED_BACKGROUND = 1 << 0,
    NET_POLICY_REJECT_METERED_BACKGROUND = 1 << 1
  }

  export interface NetworkAccessPolicy {
    allowWiFi?: boolean;
    allowCellular?: boolean;
    alwaysAllowWiFi?: boolean;
    alwaysAllowCellular?: boolean;
  }

  export class NetworkAccessPolicyInner implements NetworkAccessPolicy {
    allowWiFi?: boolean;
    allowCellular?: boolean;
    alwaysAllowWiFi?: boolean;
    alwaysAllowCellular?: boolean;
  }

  export interface NetAccessPolicy {
    allowWiFi: boolean;
    allowCellular: boolean;
  }

  export class NetAccessPolicyInner implements NetAccessPolicy {
    allowWiFi: boolean;
    allowCellular: boolean;
  }

  export type UidNetworkAccessPolicy = Record<string, NetworkAccessPolicy>;
}
