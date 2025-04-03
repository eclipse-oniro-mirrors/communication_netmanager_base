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


import type { AsyncCallback, Callback } from './@ohos.base';
import type http from './@ohos.net.http';

// import type socket from './@ohos.net.socket';

type ObserverEventType = 'netAvailable' | 'netBlockStatusChange' | 'netCapabilitiesChange' | 'netConnectionPropertiesChange' | 'netLost' | 'netUnavailable'

export interface NetCapabilities {
  linkUpBandwidthKbps?: number;

  linkDownBandwidthKbps?: number;

  networkCap?: Array<NetCap>;

  bearerTypes: Array<NetBearType>;
}

export interface NetConnectionPropertyInfo {
  netHandle: NetHandle;
  connectionProperties: ConnectionProperties;
}

export interface NetBlockStatusInfo {
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

  mtu: number;
}

export interface RouteInfo {
  interface_: string;

  destination: LinkAddress;

  gateway: NetAddress;

  hasGateway: boolean;

  isDefaultRoute: boolean;
}

export interface LinkAddress {
  address: NetAddress;
  prefixLength: number;
}

export interface NetAddress {
  address: string;

  family?: number; 

  port?: number; 
}

export interface HttpProxy {
  host: string;

  port: number;

  username?: string;

  password?: string;

  exclusionList: Array<string>;
}

export interface NetConnection {
  on(type: ObserverEventType, callback: Callback<object>): void;

  register(callback: AsyncCallback<void>): void;

  unregister(callback: AsyncCallback<void>): void;
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
  netId: number;

  // bindSocket(socketParam: TCPSocket | UDPSocket, callback: AsyncCallback<void>): void;

  // bindSocket(socketParam: TCPSocket | UDPSocket): Promise<void>;

  // getAddressesByName(host: string, callback: AsyncCallback<Array<NetAddress>>): void;

  // getAddressesByName(host: string): Promise<Array<NetAddress>>;

  // getAddressByName(host: string, callback: AsyncCallback<NetAddress>): void;

  // getAddressByName(host: string): Promise<NetAddress>;
}

export default namespace connection {
  // type TCPSocket = socket.TCPSocket;

  // type UDPSocket = socket.UDPSocket;

  loadLibrary("connection_ani") 
  export native function getDefaultNetSync(): NetHandle;

  // function createNetConnection(netSpecifier?: NetSpecifier, timeout?: number): NetConnection;

  // function getDefaultNet(callback: AsyncCallback<NetHandle>): void;

  // function getDefaultNet(): Promise<NetHandle>;

  // function getDefaultNetSync(): NetHandle 

  // function getAllNets(callback: AsyncCallback<Array<NetHandle>>): void;

  // function getAllNets(): Promise<Array<NetHandle>>;

  // function getAllNetsSync(): Array<NetHandle>;

  // function getNetCapabilities(netHandle: NetHandle, callback: AsyncCallback<NetCapabilities>): void;

  // function getNetCapabilities(netHandle: NetHandle): Promise<NetCapabilities>;

  // function getNetCapabilitiesSync(netHandle: NetHandle): NetCapabilities;

  // function hasDefaultNet(callback: AsyncCallback<boolean>): void;

  // function hasDefaultNet(): Promise<boolean>;

  // function hasDefaultNetSync(): boolean;

  // function getDefaultHttpProxy(callback: AsyncCallback<HttpProxy>): void;

  // function getDefaultHttpProxy(): Promise<HttpProxy>;



  // function getConnectionProperties(netHandle: NetHandle, callback: AsyncCallback<ConnectionProperties>): void;

  // function getConnectionProperties(netHandle: NetHandle): Promise<ConnectionProperties>;

  // function getConnectionPropertiesSync(netHandle: NetHandle): ConnectionProperties;

  // function isDefaultNetMetered(callback: AsyncCallback<boolean>): void;

  // function isDefaultNetMetered(): Promise<boolean>;

  // function isDefaultNetMeteredSync(): boolean;

  // function enableAirplaneMode(callback: AsyncCallback<void>): void;

  // function enableAirplaneMode(): Promise<void>;

  // function disableAirplaneMode(callback: AsyncCallback<void>): void;

  // function disableAirplaneMode(): Promise<void>;

  // function reportNetConnected(netHandle: NetHandle, callback: AsyncCallback<void>): void;

  // function reportNetConnected(netHandle: NetHandle): Promise<void>;

  // function reportNetDisconnected(netHandle: NetHandle, callback: AsyncCallback<void>): void;

  // function reportNetDisconnected(netHandle: NetHandle): Promise<void>;

  // function getAddressesByName(host: string, callback: AsyncCallback<Array<NetAddress>>): void;

  // function getAddressesByName(host: string): Promise<Array<NetAddress>>;

  // function getAppNet(callback: AsyncCallback<NetHandle>): void;

  // function getAppNet(): Promise<NetHandle>;

  // function getAppNetSync(): NetHandle;

  // function setAppNet(netHandle: NetHandle, callback: AsyncCallback<void>): void;

  // function setAppNet(netHandle: NetHandle): Promise<void>;

  // function getGlobalHttpProxy(callback: AsyncCallback<HttpProxy>): void;

  // function getGlobalHttpProxy(): Promise<HttpProxy>;

  // function setAppHttpProxy(httpProxy: HttpProxy): void;

  // function setGlobalHttpProxy(httpProxy: HttpProxy, callback: AsyncCallback<void>): void;

  // function setGlobalHttpProxy(httpProxy: HttpProxy): Promise<void>;

  // function addCustomDnsRule(host: string, ip: Array<string>, callback: AsyncCallback<void>): void;

  // function addCustomDnsRule(host: string, ip: Array<string>): Promise<void>;

  // function removeCustomDnsRule(host: string, callback: AsyncCallback<void>): void;

  // function removeCustomDnsRule(host: string): Promise<void>;

  // function clearCustomDnsRules(callback: AsyncCallback<void>): void;

  // function clearCustomDnsRules(): Promise<void>;

  // function factoryReset(): Promise<void>;
}

class NetHandleInner implements NetHandle{
   netId: double = 0;

  // bindSocket(socketParam: TCPSocket | UDPSocket, callback: AsyncCallback<void>): void;

  // bindSocket(socketParam: TCPSocket | UDPSocket): Promise<void>;

  // getAddressesByName(host: string, callback: AsyncCallback<Array<NetAddress>>): void;

  // getAddressesByName(host: string): Promise<Array<NetAddress>>;

  // getAddressByName(host: string, callback: AsyncCallback<NetAddress>): void;

  // getAddressByName(host: string): Promise<NetAddress>;
}