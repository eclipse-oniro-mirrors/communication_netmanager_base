/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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


import type { AsyncCallback, Callback } from '@ohos.base';
// import type connection from '@ohos.net.connection';
import { BusinessError }  from '@ohos.base';

export default namespace statistics {
  loadLibrary("statistics_ani")

  // type NetBearType = connection.NetBearType;

  export native function getIfaceRxBytesSync(nic: string): long;

  function getIfaceRxBytes(nic: string, callback: AsyncCallback<long>): void {
      let p1 = taskpool.execute((): long => {
          return getIfaceRxBytesSync(nic);
      })
      p1.then((content: NullishType) => {
          callback(new BusinessError(), content as long);
      }, (err: Error): void => {
          callback(err as BusinessError, 0);
      });
  }

  function getIfaceRxBytes(nic: string): Promise<long> {
      return new Promise<long>((resolve, reject) => {
          taskpool.execute((): long => {
          return getIfaceRxBytesSync(nic);
      }).then((content: NullishType) => {
          resolve(content as long);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
  });
  }

  export native function getIfaceTxBytesSync(nic: string): long;

  function getIfaceTxBytes(nic: string, callback: AsyncCallback<long>): void {
      let p1 = taskpool.execute((): long => {
          return getIfaceTxBytesSync(nic);
      })
      p1.then((content: NullishType) => {
          callback(new BusinessError(), content as long);
      }, (err: Error): void => {
          callback(err as BusinessError, 0);
      });
  }

  function getIfaceTxBytes(nic: string): Promise<long> {
      return new Promise<long>((resolve, reject) => {
          taskpool.execute((): long => {
          return getIfaceTxBytesSync(nic);
      }).then((content: NullishType) => {
          resolve(content as long);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
  });
  }

  export native function getCellularRxBytesSync(): long;

  function getCellularRxBytes(callback: AsyncCallback<long>): void {
      let p1 = taskpool.execute((): long => {
          return getCellularRxBytesSync();
      })
      p1.then((content: NullishType) => {
          callback(new BusinessError(), content as long);
      }, (err: Error): void => {
          callback(err as BusinessError, 0);
      });
  }

  function getCellularRxBytes(): Promise<long> {
      return new Promise<long>((resolve, reject) => {
          taskpool.execute((): long => {
          return getCellularRxBytesSync();
      }).then((content: NullishType) => {
          resolve(content as long);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
  });
  }

  export native function getCellularTxBytesSync(): long;

  function getCellularTxBytes(callback: AsyncCallback<long>): void {
      let p1 = taskpool.execute((): long => {
          return getCellularTxBytesSync();
      })
      p1.then((content: NullishType) => {
          callback(new BusinessError(), content as long);
      }, (err: Error): void => {
          callback(err as BusinessError, 0);
      });
  }

  function getCellularTxBytes(): Promise<long> {
      return new Promise<long>((resolve, reject) => {
          taskpool.execute((): long => {
          return getCellularTxBytesSync();
      }).then((content: NullishType) => {
          resolve(content as long);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
  });
  }

  export native function getAllRxBytesSync(): long;

  function getAllRxBytes(callback: AsyncCallback<long>): void {
      let p1 = taskpool.execute((): long => {
          return getAllRxBytesSync();
      })
      p1.then((content: NullishType) => {
          callback(new BusinessError(), content as long);
      }, (err: Error): void => {
          callback(err as BusinessError, 0);
      });
  }

  function getAllRxBytes(): Promise<long> {
      return new Promise<long>((resolve, reject) => {
          taskpool.execute((): long => {
          return getAllRxBytesSync();
      }).then((content: NullishType) => {
          resolve(content as long);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
  });
  }

  export native function getAllTxBytesSync(): long;

  function getAllTxBytes(callback: AsyncCallback<long>): void {
      let p1 = taskpool.execute((): long => {
          return getAllTxBytesSync();
      })
      p1.then((content: NullishType) => {
          callback(new BusinessError(), content as long);
      }, (err: Error): void => {
          callback(err as BusinessError, 0);
      });
  }

  function getAllTxBytes(): Promise<long> {
      return new Promise<long>((resolve, reject) => {
          taskpool.execute((): long => {
          return getAllTxBytesSync();
      }).then((content: NullishType) => {
          resolve(content as long);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
  });
  }

  export native function getUidRxBytesSync(uid: long): long;

  function getUidRxBytes(uid: long, callback: AsyncCallback<long>): void {
      let p1 = taskpool.execute((): long => {
          return getUidRxBytesSync(uid);
      })
      p1.then((content: NullishType) => {
          callback(new BusinessError(), content as long);
      }, (err: Error): void => {
          callback(err as BusinessError, 0);
      });
  }

  function getUidRxBytes(uid: long): Promise<long> {
      return new Promise<long>((resolve, reject) => {
          taskpool.execute((): long => {
          return getUidRxBytesSync(uid);
      }).then((content: NullishType) => {
          resolve(content as long);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
  });
  }

  export native function getUidTxBytesSync(uid: long): long;

  function getUidTxBytes(uid: long, callback: AsyncCallback<long>): void {
      let p1 = taskpool.execute((): long => {
          return getUidTxBytesSync(uid);
      })
      p1.then((content: NullishType) => {
          callback(new BusinessError(), content as long);
      }, (err: Error): void => {
          callback(err as BusinessError, 0);
      });
  }

  function getUidTxBytes(uid: long): Promise<long> {
      return new Promise<long>((resolve, reject) => {
        taskpool.execute((): long => {
        return getUidTxBytesSync(uid);
      }).then((content: NullishType) => {
        resolve(content as long);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  // function on(type: 'netStatsChange', callback: Callback<NetStatsChangeInfo>): void;

  // function off(type: 'netStatsChange', callback?: Callback<NetStatsChangeInfo>): void;

  // function getTrafficStatsByIface(ifaceInfo: IfaceInfo, callback: AsyncCallback<NetStatsInfo>): void;

  // function getTrafficStatsByIface(ifaceInfo: IfaceInfo): Promise<NetStatsInfo>;

  // function getTrafficStatsByUid(uidInfo: UidInfo, callback: AsyncCallback<NetStatsInfo>): void;

  // function getTrafficStatsByUid(uidInfo: UidInfo): Promise<NetStatsInfo>;

  export native function getSockfdRxBytesSync(sockfd: int): long;

  function getSockfdRxBytes(sockfd: int, callback: AsyncCallback<long>): void {
    let p1 = taskpool.execute((): long => {
        return getSockfdRxBytesSync(sockfd);
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as long);
    }, (err: Error): void => {
        callback(err as BusinessError, 0);
    });
  }

  function getSockfdRxBytes(sockfd: int): Promise<long> {
    return new Promise<long>((resolve, reject) => {
          taskpool.execute((): long => {
          return getSockfdRxBytesSync(sockfd);
      }).then((content: NullishType) => {
          resolve(content as long);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
    });
  }

  export native function getSockfdTxBytesSync(sockfd: int): long;

  function getSockfdTxBytes(sockfd: int, callback: AsyncCallback<long>): void {
    let p1 = taskpool.execute((): long => {
        return getSockfdTxBytesSync(sockfd);
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as long);
    }, (err: Error): void => {
        callback(err as BusinessError, 0);
    });
  }

  function getSockfdTxBytes(sockfd: int): Promise<long> {
    return new Promise<long>((resolve, reject) => {
          taskpool.execute((): long => {
          return getSockfdTxBytesSync(sockfd);
      }).then((content: NullishType) => {
          resolve(content as long);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
    });
  }

  export interface IfaceInfo {
    iface: string;

    startTime: number;

    endTime: number;
  }

  export interface UidInfo {
    ifaceInfo: IfaceInfo;

    uid: number;
  }

  export interface NetStatsInfo {
    rxBytes: number;

    txBytes: number;

    rxPackets: number;

    txPackets: number;
  }

  export interface NetStatsChangeInfo {
    iface: string;
    uid?: number;
  }

  // export interface NetStatsInfoSequence {
  //   startTime: number;
  //   endTime: number;
  //   info: NetStatsInfo;
  // }

  // export interface UidNetStatsInfo {
  //   [uid: number]: NetStatsInfo;
  // }

  // export interface NetworkInfo {
  //   type: NetBearType;
  //   startTime: number;
  //   endTime: number;
  //   simId?: number;
  // }

  // function getTrafficStatsByNetwork(networkInfo: NetworkInfo): Promise<UidNetStatsInfo>;
  // function getTrafficStatsByUidNetwork(uid: number, networkInfo: NetworkInfo): Promise<NetStatsInfoSequence[]>;
}

function main() {
  try {
    let w = statistics.getAllRxBytesSync();
    console.log(w);
  } catch(e:BusinessError) {
    console.log(e);
  }
}
