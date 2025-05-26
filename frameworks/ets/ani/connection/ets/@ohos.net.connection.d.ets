/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
import { BusinessError }  from '@ohos.base';

// import type http from './@ohos.net.http';
// import type socket from './@ohos.net.socket';

export default namespace connection {
  loadLibrary("connection_ani") 

  export class Cleaner {
    private ptr: long = 0
    constructor(ptr:long) {
        this.ptr = ptr
    }
    native clean(): void
  }

  let destroyRegister = new FinalizationRegistry<Cleaner>((cleaner: Cleaner) => {cleaner.clean()})
  let unregisterToken = new object()

  // type HttpRequest = http.HttpRequest;

  // type TCPSocket = socket.TCPSocket;

  // type UDPSocket = socket.UDPSocket;

  export native function createNetConnection(netSpecifier?: NetSpecifier, timeout?: int): NetConnection;

  export function getDefaultNet(callback: AsyncCallback<NetHandle>): void {
    let p1 = taskpool.execute((): NetHandle => {
        return getDefaultNetSync();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as NetHandle);
    }, (err: BusinessError): void => {
        callback(err, new NetHandleInner);
    });
  }

  export function getDefaultNet(): Promise<NetHandle> {
      return new Promise<NetHandle>((resolve, reject) => {
          taskpool.execute((): NetHandle => {
              return getDefaultNetSync();
          }).then((content: NullishType) => {
              resolve(content as NetHandle);
          }, (err: BusinessError): void => {
              reject(err);
          });
      });
  }

  export native function getDefaultNetSync(): NetHandle;

  export function getAllNets(callback: AsyncCallback<Array<NetHandle>>): void {
    let p1 = taskpool.execute((): Array<NetHandle> => {
        return getAllNetsSync();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as Array<NetHandle>);
    }, (err: BusinessError): void => {
        callback(err, new Array<NetHandle>());
    });
  }

  export function getAllNets(): Promise<Array<NetHandle>> {
    return new Promise<Array<NetHandle>>((resolve, reject) => {
        taskpool.execute((): Array<NetHandle> => {
            return getAllNetsSync();
        }).then((content: NullishType) => {
            resolve(content as Array<NetHandle>);
        }, (err: BusinessError): void => {
            reject(err);
        });
    });
  }

  export native function getAllNetsSync(): Array<NetHandle>;

  export function getConnectionProperties(netHandle: NetHandle, callback: AsyncCallback<ConnectionProperties>): void {
    let p1 = taskpool.execute((): ConnectionProperties => {
        return getConnectionPropertiesSync(netHandle);
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as ConnectionProperties);
    }, (err: BusinessError): void => {
        callback(err, new ConnectionPropertiesInner());
    });
  }

  export function getConnectionProperties(netHandle: NetHandle): Promise<ConnectionProperties> {
    return new Promise<ConnectionProperties>((resolve, reject) => {
        taskpool.execute((): ConnectionProperties => {
            return getConnectionPropertiesSync(netHandle);
        }).then((content: NullishType) => {
            resolve(content as ConnectionProperties);
        }, (err: BusinessError): void => {
            reject(err);
        });
    });
  }

  export native function getConnectionPropertiesSync(netHandle: NetHandle): ConnectionProperties;

  export function getNetCapabilities(netHandle: NetHandle, callback: AsyncCallback<NetCapabilities>): void {
    let p1 = taskpool.execute((): NetCapabilities => {
        return getNetCapabilitiesSync(netHandle);
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as NetCapabilities);
    }, (err: BusinessError): void => {
        callback(err, new NetCapabilitiesInner);
    });
  }

  export function getNetCapabilities(netHandle: NetHandle): Promise<NetCapabilities> {
    return new Promise<NetCapabilities>((resolve, reject) => {
        taskpool.execute((): NetCapabilities => {
            return getNetCapabilitiesSync(netHandle);
        }).then((content: NullishType) => {
            resolve(content as NetCapabilities);
        }, (err: BusinessError): void => {
            reject(err);
        });
    });
  }

  export native function getNetCapabilitiesSync(netHandle: NetHandle): NetCapabilities;

  // function setNetExtAttribute(netHandle: NetHandle, netExtAttribute: string): Promise<void>;
 
  // function setNetExtAttributeSync(netHandle: NetHandle, netExtAttribute: string): void;
     
  // function getNetExtAttribute(netHandle: NetHandle): Promise<string>;
     
  // function getNetExtAttributeSync(netHandle: NetHandle): string;
    
  export function isDefaultNetMetered(callback: AsyncCallback<boolean>): void {
    let p1 = taskpool.execute((): boolean => {
        return isDefaultNetMeteredSync();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as boolean);
    }, (err: BusinessError): void => {
        callback(err, false);
    });
  }

  export function isDefaultNetMetered(): Promise<boolean> {
      return new Promise<boolean>((resolve, reject) => {
        taskpool.execute((): boolean => {
            return isDefaultNetMeteredSync();
        }).then((content: NullishType) => {
            resolve(content as boolean);
        }, (err: BusinessError): void => {
            reject(err);
        });
    });
  }

  export native function isDefaultNetMeteredSync(): boolean;

  export function hasDefaultNet(callback: AsyncCallback<boolean>): void {
    let p1 = taskpool.execute((): boolean => {
        return hasDefaultNetSync();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as boolean);
    }, (err: BusinessError): void => {
        callback(err, false);
    });
  } 

  export function hasDefaultNet(): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
        taskpool.execute((): boolean => {
            return hasDefaultNetSync();
        }).then((content: NullishType) => {
            resolve(content as boolean);
        }, (err: BusinessError): void => {
            reject(err);
        });
    });
  }

  export native function hasDefaultNetSync(): boolean;

  export native function enableAirplaneModeSync():void;

  export function enableAirplaneMode(callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
       return enableAirplaneModeSync();
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: BusinessError): void => {
        callback(err, undefined);
    });
  }

  export function enableAirplaneMode(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        taskpool.execute((): undefined => {
          return enableAirplaneModeSync();
        }).then(() => {
            resolve(undefined);
        }, (err: BusinessError): void => {
            reject(err);
        });
    });
  }

  export native function disableAirplaneModeSync():void;

  export function disableAirplaneMode(callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
      return disableAirplaneModeSync();
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: BusinessError): void => {
        callback(err, undefined);
    });
  }

  export function disableAirplaneMode(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        taskpool.execute((): undefined => {
          return disableAirplaneModeSync();
        }).then((content: NullishType) => {
            resolve(undefined);
        }, (err: BusinessError): void => {
            reject(err);
        });
    });
  }

  export function reportNetConnected(netHandle: NetHandle, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
        return reportNetConnectedSync(netHandle);
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: BusinessError): void => {
        callback(err, undefined);
    });
  }

  export function reportNetConnected(netHandle: NetHandle): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        taskpool.execute((): undefined => {
            return reportNetConnectedSync(netHandle);
        }).then((content: NullishType) => {
            resolve(undefined);
        }, (err: BusinessError): void => {
            reject(err);
        });
    });
  }

  export native function reportNetConnectedSync(netHandle: NetHandle): void;

  export function reportNetDisconnected(netHandle: NetHandle, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
       return reportNetDisconnectedSync(netHandle)
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: BusinessError): void => {
        callback(err, undefined);
    });
  }

  export function reportNetDisconnected(netHandle: NetHandle): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        taskpool.execute((): undefined => {
           return reportNetDisconnectedSync(netHandle);
        }).then((content: NullishType) => {
            resolve(undefined);
        }, (err: BusinessError): void => {
            reject(err);
        });
    });
  }

  export native function reportNetDisconnectedSync(netHandle: NetHandle): void;

  export function getAddressesByName(host: string, callback: AsyncCallback<Array<NetAddress>>): void{
    let p1 = taskpool.execute((): Array<NetAddress> => {
        return getAddressesByNameSync(host);
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as Array<NetAddress>);
    }, (err: BusinessError): void => {
        callback(err, new Array<NetAddress>);
    });
  }

  export function getAddressesByName(host: string): Promise<Array<NetAddress>> {
    return new Promise<Array<NetAddress>>((resolve, reject) => {
        taskpool.execute((): Array<NetAddress> => {
            return getAddressesByNameSync(host);
        }).then((content: NullishType) => {
            resolve(content as Array<NetAddress>);
        }, (err: BusinessError): void => {
            reject(err);
        });
    });
  }

  export native function getAddressesByNameSync(host: string): Array<NetAddress>;

  export function getAppNet(callback: AsyncCallback<NetHandle>): void {
    let p1 = taskpool.execute((): NetHandle => {
        return getAppNetSync();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as NetHandle );
    }, (err: BusinessError): void => {
        callback(err, new NetHandleInner);
    });
  }

  export function getAppNet(): Promise<NetHandle> {
    return new Promise<NetHandle>((resolve, reject) => {
      taskpool.execute((): NetHandle => {
          return getAppNetSync();
      }).then((content: NullishType) => {
          resolve(content as NetHandle);
      }, (err: BusinessError): void => {
          reject(err);
      });
    });
  }

  export native function getAppNetSync(): NetHandle;

  export function setAppNet(netHandle: NetHandle, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
        return setAppNetSync(netHandle);
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: BusinessError): void => {
        callback(err, undefined);
    });
  }

  export function setAppNet(netHandle: NetHandle): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): undefined => {
          return setAppNetSync(netHandle);
      }).then(() => {
          resolve(undefined);
      }, (err: BusinessError): void => {
          reject(err);
      });
    });
  }

  export native function setAppNetSync(netHandle: NetHandle): void;

  function getDefaultHttpProxy(callback: AsyncCallback<HttpProxy>): void {
    let p1 = taskpool.execute((): HttpProxy => {
        return getDefaultHttpProxySync();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as HttpProxy);
    }, (err: BusinessError): void => {
        callback(err, new HttpProxyInner);
    });
  }

  function getDefaultHttpProxy(): Promise<HttpProxy> {
    return new Promise<HttpProxy>((resolve, reject) => {
        taskpool.execute((): HttpProxy => {
            return getDefaultHttpProxySync();
        }).then((content: NullishType) => {
            resolve(content as HttpProxy);
        }, (err: BusinessError): void => {
            reject(err);
        });
    });
  }

  export native function getDefaultHttpProxySync(): HttpProxy;

  export function getGlobalHttpProxy(callback: AsyncCallback<HttpProxy>): void {
    let p1 = taskpool.execute((): HttpProxy => {
        return getGlobalHttpProxySync();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as HttpProxy);
    }, (err: BusinessError): void => {
        callback(err, new HttpProxyInner);
    });
  }

  export function getGlobalHttpProxy(): Promise<HttpProxy> {
    return new Promise<HttpProxy>((resolve, reject) => {
        taskpool.execute((): HttpProxy => {
            return getGlobalHttpProxySync();
        }).then((content: NullishType) => {
            resolve(content as HttpProxy);
        }, (err: BusinessError): void => {
            reject(err);
        });
    });
  }

  export native function getGlobalHttpProxySync(): HttpProxy;

  export native function setAppHttpProxy(httpProxy: HttpProxy): void;

  export function setGlobalHttpProxy(httpProxy: HttpProxy, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
        return setGlobalHttpProxySync(httpProxy);
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: BusinessError): void => {
        callback(err, undefined);
    });
  }

  export function setGlobalHttpProxy(httpProxy: HttpProxy): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): undefined => {
          return setGlobalHttpProxySync(httpProxy);
      }).then(() => {
          resolve(undefined);
      }, (err: BusinessError): void => {
          reject(err);
      }); 
    });
  }

  export native function setGlobalHttpProxySync(httpProxy: HttpProxy): void;

  export native function setPacUrl(pacUrl: string): void;

  export native function getPacUrl(): string;

  export function addCustomDnsRule(host: string, ip: Array<string>, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
         return addCustomDnsRuleSync(host ,ip);
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: BusinessError): void => {
        callback(err, undefined);
    });
  }

  export function addCustomDnsRule(host: string, ip: Array<string>): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): undefined => {
          return addCustomDnsRuleSync(host, ip);
      }).then(() => {
          resolve(undefined);
      }, (err: BusinessError): void => {
          reject(err);
      }); 
    });
  }

  export native function addCustomDnsRuleSync(host: string, ip: Array<string>): void;

  export function removeCustomDnsRule(host: string, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
        return removeCustomDnsRuleSync(host);
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: BusinessError): void => {
        callback(err, undefined);
    });
  }

  export function removeCustomDnsRule(host: string): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): undefined => {
          return removeCustomDnsRuleSync(host)
      }).then(() => {
          resolve(undefined);
      }, (err: BusinessError): void => {
          reject(err);
      }); 
    });
  }

  export native function removeCustomDnsRuleSync(host:string): void;

  export function clearCustomDnsRules(callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
         return clearCustomDnsRulesSync();
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: BusinessError): void => {
        callback(err, undefined);
    });
  }

  export function clearCustomDnsRules(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): undefined => {
          return clearCustomDnsRulesSync();
      }).then(() => {
          resolve(undefined);
      }, (err: BusinessError): void => {
          reject(err);
      }); 
    });
  }

  export native function clearCustomDnsRulesSync(): void;

  export native function factoryResetSync():void;

  export function factoryReset(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): undefined => {
          return factoryResetSync();
      }).then(() => {
          resolve(undefined);
      }, (err: BusinessError): void => {
          reject(err);
      }); 
    });
  }

  export interface NetConnection {
      on(type: 'netAvailable' | 'netBlockStatusChange' | 'netCapabilitiesChange' | 'netConnectionPropertiesChange' | 'netLost' | 'netUnavailable' , callback: Object): void;

      register(callback: AsyncCallback<void>): void;

      unregister(callback: AsyncCallback<void>): void;
  }

  class NetConnectionInner implements NetConnection{
    private  nativePtr:long = 0;
    private cleaner: Cleaner | null = null;

    constructor(context:long) {
        if(this.nativePtr == 0){
             this.nativePtr = context;
        }
        this.registerCleaner(this.nativePtr)
    }

    registerCleaner(ptr: long): void {
        this.cleaner = new Cleaner(ptr)
        destroyRegister.register(this, this.cleaner!, unregisterToken);
    }
    unregisterCleaner(): void {
        destroyRegister.unregister(unregisterToken);
    }

    on(type: 'netAvailable' | 'netBlockStatusChange' | 'netCapabilitiesChange' | 'netConnectionPropertiesChange' | 'netLost' | 'netUnavailable' , callback: Object): void {
      if (type == 'netAvailable') {
        this.onNetAvailable(callback as Callback<NetHandle>);
      } else if (type == 'netBlockStatusChange') {
        this.onNetBlockStatusChange(callback as Callback<NetBlockStatusInfo>);
      } else if (type == 'netCapabilitiesChange') {
        this.onNetCapabilitiesChange(callback as Callback<NetCapabilityInfo>);
      } else if (type == 'netConnectionPropertiesChange') {
        this.onNetConnectionPropertiesChange(callback as Callback<NetConnectionPropertyInfo>);
      } else if (type == 'netLost') {
        this.onNetLost(callback as Callback<NetHandle>);
      } else if (type == 'netUnavailable') {
        this.onNetUnavailable(callback as Callback<void>);
      }
    }

    native onNetAvailable(callback: Callback<NetHandle>):void;
    native onNetBlockStatusChange(callback: Callback<NetBlockStatusInfo>):void;
    native onNetCapabilitiesChange(callback: Callback<NetCapabilityInfo>):void;
    native onNetConnectionPropertiesChange(callback: Callback<NetConnectionPropertyInfo>):void;
    native onNetLost(callback: Callback<NetHandle>):void;
    native onNetUnavailable(callback: Callback<void>):void;

    native registerSync():void;
    register(callback: AsyncCallback<void>): void {
      let p1 = taskpool.execute((): undefined => {
         return this.registerSync();
      })
      p1.then(() => {
          callback(new BusinessError(), undefined);
      }, (err: BusinessError): void => {
          callback(err, undefined);
      });
    }

    native unregisterSync():void;

    unregister(callback: AsyncCallback<void>): void {
      let p1 = taskpool.execute((): undefined => {
         return this.unregisterSync();
      })
      p1.then(() => {
          callback(new BusinessError(), undefined);
      }, (err: BusinessError): void => {
          callback(err, undefined);
      });
    }
  }

  export interface NetSpecifier {
    netCapabilities: NetCapabilities;

    bearerPrivateIdentifier?: string;
  }

  export interface NetCapabilityInfo {
    netHandle: NetHandle;

    netCap: NetCapabilities;
  }

  export interface NetHandle {
    netId: int;

    // bindSocket(socketParam: TCPSocket | UDPSocket, callback: AsyncCallback<void>): void;

    // bindSocket(socketParam: TCPSocket | UDPSocket): Promise<void>;

    // getAddressesByName(host: string, callback: AsyncCallback<Array<NetAddress>>): void;

    // getAddressesByName(host: string): Promise<Array<NetAddress>>;

    getAddressByName(host: string, callback: AsyncCallback<NetAddress>): void;

    getAddressByName(host: string): Promise<NetAddress>;
  }

  export class NetHandleInner implements NetHandle{
    netId: int;
    static { loadLibrary("connection_ani") }

    native getAddressByNameSync(host:string): NetAddress;

    getAddressByName(host: string, callback: AsyncCallback<NetAddress>): void {
      let p1 = taskpool.execute((): NetAddress => {
        return this.getAddressByNameSync(host);
      })
      p1.then((content: NullishType) => {
          callback(new BusinessError(), content as NetAddress);
      }, (err: BusinessError): void => {
          callback(err, new NetAddressInner);
      });
    }

    getAddressByName(host: string): Promise<NetAddress> {
      return new Promise<NetAddress>((resolve, reject) => {
        taskpool.execute((): NetAddress => {
            return this.getAddressByNameSync(host);
        }).then((content: NullishType) => {
            resolve(content as NetAddress);
        }, (err: BusinessError): void => {
            reject(err);
        });
    });
    }
  }

  export interface NetCapabilities {
    linkUpBandwidthKbps?: int;

    linkDownBandwidthKbps?: int;

    networkCap?: Array<NetCap>;

    bearerTypes: Array<NetBearType>;
  }

  export class NetCapabilitiesInner implements NetCapabilities{
    linkUpBandwidthKbps?: int;

    linkDownBandwidthKbps?: int;

    networkCap?: Array<NetCap>;

    bearerTypes: Array<NetBearType>;
  }

  export interface NetConnectionPropertyInfo {
    netHandle: NetHandle;
    connectionProperties: ConnectionProperties;
  }

  export class NetConnectionPropertyInfoInner implements NetConnectionPropertyInfo {
    netHandle: NetHandle;
    connectionProperties: ConnectionProperties;
  }

  export interface NetBlockStatusInfo {
    netHandle: NetHandle;
    blocked: boolean;
  }

  export class NetBlockStatusInfoInner implements NetBlockStatusInfo{
    netHandle: NetHandle;
    blocked: boolean;
  }
  
  export enum NetCap {
    NET_CAPABILITY_MMS = 0,

    NET_CAPABILITY_NOT_METERED = 11,

    NET_CAPABILITY_INTERNET = 12,

    NET_CAPABILITY_NOT_VPN = 15,

    NET_CAPABILITY_VALIDATED = 16,
    NET_CAPABILITY_PORTAL = 17,

    NET_CAPABILITY_CHECKING_CONNECTIVITY = 31
  }

  export enum NetBearType {
    BEARER_CELLULAR = 0,

    BEARER_WIFI = 1,

    BEARER_BLUETOOTH = 2,
 
    BEARER_ETHERNET = 3,

    BEARER_VPN = 4,
  }

  export interface ConnectionProperties {
    interfaceName: string;
    domains: string;
    linkAddresses: Array<LinkAddress>;

    dnses: Array<NetAddress>;

    routes: Array<RouteInfo>;

    mtu: int;
  }

  class ConnectionPropertiesInner implements ConnectionProperties{
    interfaceName: string;
    domains: string;
    linkAddresses: Array<LinkAddress>;

    dnses: Array<NetAddress>;

    routes: Array<RouteInfo>;

    mtu: int;
  }

  export interface RouteInfo {
    iface: string;

    destination: LinkAddress;

    gateway: NetAddress;

    hasGateway: boolean;

    isDefaultRoute: boolean;
  }

  class RouteInfoInner implements RouteInfo {
    iface: string;

    destination: LinkAddress = new LinkAddressInner;

    gateway: NetAddress = new NetAddressInner;

    hasGateway: boolean;

    isDefaultRoute: boolean;
  }

  export interface LinkAddress {
    address: NetAddress;
    prefixLength: int;
  }

  class LinkAddressInner implements LinkAddress {
    address: NetAddress = new NetAddressInner;
    prefixLength: int;
  }

  export interface NetAddress {
    address: string;

    family?: int; 

    port?: int; 
  }

  class NetAddressInner implements NetAddress {
    address: string;

    family?: int; 

    port?: int; 
  }

  export interface HttpProxy {
    host: string;

    port: int;

    username?: string;

    password?: string;

    exclusionList: Array<string>;
  }

  export class HttpProxyInner implements HttpProxy{
    host: string;

    port: int;

    username?: string;

    password?: string;

    exclusionList: Array<string>;
  }
}
