# Net Manager

## 简介

网络管理介绍：

    网络管理模块作为电话子系统可裁剪部件，主要分为连接管理、策略管理、流量管理、网络共享、VPN管理、网络共享以及以太网连接等模块；如图1：网络管理架构图；

**图 1**  网络管理架构图

![net_conn_manager_arch_zh](figures/net_conn_manager_arch_zh.png)

## 目录

```
foundation/communication/netmanager_base/
├─figures
├─frameworks
│  ├─js
│  │  └─napi
│  │      ├─common
│  │      ├─connection
│  │      │  ├─async_context
│  │      │  │  ├─include
│  │      │  │  └─src
│  │      │  ├─async_work
│  │      │  │  ├─include
│  │      │  │  └─src
│  │      │  ├─connection_exec
│  │      │  │  ├─include
│  │      │  │  └─src
│  │      │  ├─connection_module
│  │      │  │  ├─include
│  │      │  │  └─src
│  │      │  ├─constant
│  │      │  │  └─include
│  │      │  └─options
│  │      │      ├─include
│  │      │      └─src
│  │      ├─netconn
│  │      │  ├─include
│  │      │  └─src
│  │      ├─netpolicy
│  │      │  ├─include
│  │      │  └─src
│  │      └─netstats
│  │          ├─include
│  │          └─src
│  └─native
│      ├─dnsresolverclient
│      │  └─src
│      │      └─proxy
│      ├─netconnclient
│      │  └─src
│      │      └─proxy
│      ├─netmanagernative
│      ├─netpolicyclient
│      │  └─src
│      │      └─proxy
│      └─netstatsclient
│          └─src
│              └─proxy
├─interfaces
│  ├─innerkits
│  │  ├─dnsresolverclient
│  │  │  └─include
│  │  │      └─proxy
│  │  ├─include
│  │  ├─netconnclient
│  │  │  └─include
│  │  │      └─proxy
│  │  ├─netmanagernative
│  │  │  └─include
│  │  ├─netpolicyclient
│  │  │  └─include
│  │  │      └─proxy
│  │  └─netstatsclient
│  │      └─include
│  │          └─proxy
│  └─kits
│      └─js
├─prebuilts
│  └─librarys
│      └─netd
│          ├─arm
│          ├─arm64
│          └─include
│              ├─common
│              │  └─include
│              └─net_mgr_native
│                  └─include
├─sa_profile
├─services
│  ├─common
│  │  ├─include
│  │  └─src
│  ├─dnsresolvermanager
│  │  ├─include
│  │  │  └─stub
│  │  └─src
│  │      └─stub
│  ├─etc
│  │  └─init
│  ├─netconnmanager
│  │  ├─include
│  │  │  └─stub
│  │  └─src
│  │      └─stub
│  ├─netdcontroller
│  │  ├─include
│  │  └─src
│  ├─netmanagernative
│  │  ├─include
│  │  └─src
│  ├─netpolicymanager
│  │  ├─include
│  │  │  └─stub
│  │  └─src
│  │      └─stub
│  └─netstatsmanager
│      ├─include
│      │  └─stub
│      └─src
│          └─stub
├─test
│  ├─dnsresolvermanager
│  │  └─unittest
│  │      └─dns_resolver_manager_test
│  ├─netconnmanager
│  │  ├─mock
│  │  └─unittest
│  │      └─net_conn_manager_test
│  ├─netmanagernative
│  │  └─unittest
│  ├─netpolicymanager
│  │  └─unittest
│  │      └─net_policy_manager_test
│  └─netstatsmanager
│      └─unittest
│          └─net_stats_manager_test
└─utils
    ├─base_async_work
    │  └─include
    ├─base_context
    │  ├─include
    │  └─src
    ├─common_utils
    │  ├─include
    │  └─src
    ├─event_manager
    │  ├─include
    │  └─src
    ├─log
    │  ├─include
    │  └─src
    ├─module_template
    │  ├─include
    │  └─src
    └─napi_utils
        ├─include
        └─src
```

## 约束

-    软件层，需要以下子系统和服务配合使用：蜂窝数据、WiFi系统、安全子系统、软总线子系统、USB子系统、电源管理子系统等；
-    硬件层，需要搭载的设备支持以下硬件：可以进行独立蜂窝通信的Modem以及SIM卡；

## 接口说明

| 类型 | 接口 | 接口类型(方法 &#124; 属性) | 功能说明 |
| ---- | ---- | ---- | ---- |
| ohos.net.connection | `function getDefaultNet(callback: AsyncCallback<NetHandle>): void;` | 方法 | 创建一个含有默认网络的netId的NetHandle对象，调用callback |
| ohos.net.connection | `function getDefaultNet(): Promise<NetHandle>;` | 方法 | 创建一个含有默认网络的netId的NetHandle对象，返回Promise |
| ohos.net.connection | `function getConnectionProperties(netHandle: NetHandle, callback: AsyncCallback<ConnectionProperties>): void;` | 方法 | 查询默认网络的链路信息，调用callback |
| ohos.net.connection | `function getConnectionProperties(netHandle: NetHandle): Promise<ConnectionProperties>;` | 方法 | 查询默认网络的链路信息，返回Promise |
| ohos.net.connection | `function getNetCapabilities(netHandle: NetHandle, callback: AsyncCallback<NetCapabilities>): void;` | 方法 | 查询默认网络的能力集信息，调用callback |
| ohos.net.connection | `function getNetCapabilities(netHandle: NetHandle): Promise<NetCapabilities>;` | 方法 | 查询默认网络的能力集信息，返回Promise |
| ohos.net.connection | `function hasDefaultNet(callback: AsyncCallback<boolean>): void;` | 方法 | 查询是否有默认网络，调用callback |
| ohos.net.connection | `function hasDefaultNet(): Promise<boolean>;` | 方法 | 查询是否有默认网络，返回Promise |
| ohos.net.connection | `function getAddressesByName(host: string, callback: AsyncCallback<Array<NetAddress>>): void;` | 方法 | 使用默认网络解析域名，获取所有IP，调用callback |
| ohos.net.connection | `function getAddressesByName(host: string): Promise<Array<NetAddress>>;` | 方法 | 使用默认网络解析域名，获取所有IP，返回Promise |
| ohos.net.connection.NetHandle | `netId` | 属性 | number类型，默认网络的ID |
| ohos.net.connection.NetHandle | `getAddressesByName(host: string, callback: AsyncCallback<Array<NetAddress>>): void;` | 方法 | 使用对应网络解析域名，获取所有IP，调用callback |
| ohos.net.connection.NetHandle | `getAddressesByName(host: string): Promise<Array<NetAddress>>;` | 方法 | 使用对应网络解析域名，获取所有IP，返回Promise |
| ohos.net.connection.NetHandle | `getAddressByName(host: string, callback: AsyncCallback<NetAddress>): void;` | 方法 | 使用对应网络解析域名，获取一个IP，调用callbac |
| ohos.net.connection.NetHandle | `getAddressByName(host: string): Promise<NetAddress>;` | 方法 | 使用对应网络解析域名，获取一个IP，返回Promise |
| ohos.net.connection | `function createNetConnection(netSpecifier?: NetSpecifier, timeout?: number): NetConnection;` | 方法 | 返回一个NetConnection对象，netSpecifier指定关注的网络的各项特征，timeout是超时时间，netSpecifier是timeout的必要条件，两者都没有则表示关注默认网络 |
| ohos.net.connection.NetConnection | `on(type: 'netAvailable', callback: Callback<NetHandle>): void;` | 方法 | 监听收到网络可用的事件 |
| ohos.net.connection.NetConnection | `on(type: 'netCapabilitiesChange', callback: Callback<{ netHandle: NetHandle, netCap: NetCapabilities }>): void;` | 方法 | 监听网络能力变化的事件 |
| ohos.net.connection.NetConnection | `on(type: 'netConnectionPropertiesChange', callback: Callback<{ netHandle: NetHandle, connectionProperties: ConnectionProperties }>): void;` | 方法 | 监听网络连接信息变化的事件 |
| ohos.net.connection.NetConnection | `on(type: 'netLost', callback: Callback<NetHandle>): void;` | 方法 | 监听网络丢失的事件 |
| ohos.net.connection.NetConnection | `on(type: 'netUnavailable', callback: Callback<void>): void;` | 方法 | 监听网络不可用的事件 |
| ohos.net.connection.NetConnection | `register(callback: AsyncCallback<void>): void;` | 方法 | 注册默认网络或者createNetConnection中指定的网络的监听 |
| ohos.net.connection.NetConnection | `unregister(callback: AsyncCallback<void>): void;` | 方法 | 注销默认网络或者createNetConnection中指定的网络的监听 |
| ohos.net.connection.NetSpecifier | `netCapabilities` | 属性 | NetCapabilities类型，网络能力集 |
| ohos.net.connection.NetSpecifier | `bearerPrivateIdentifier` | 属性 | string类型，网络标识符，WIFI网络的标识符是`wifi`，蜂窝网络的标识符是`slot0`(对应SIM卡1) |
| ohos.net.connection.NetCapabilities | `linkUpBandwidthKbps` | 属性 | number类型，带宽上限 |
| ohos.net.connection.NetCapabilities | `linkDownBandwidthKbps` | 属性 | number类型，带宽下限 |
| ohos.net.connection.NetCapabilities | `networkCap` | 属性 | NetCap类型的数组，表示支持哪些网络能力 |
| ohos.net.connection.NetCapabilities | `bearerTypes` | 属性 | NetBearType类型的数组，表示网络类型 |
| ohos.net.connection.NetCap | `NET_CAPABILITY_INTERNET = 12` | 属性 | 枚举，表示联网能力 |
| ohos.net.connection.NetCap | `NET_CAPABILITY_VALIDATED = 16` | 属性 | 枚举，表示网络可用 |
| ohos.net.connection.NetBearType | `BEARER_CELLULAR = 0` | 属性 | 枚举，表示WIFI网络 |
| ohos.net.connection.NetBearType | `BEARER_WIFI = 1` | 属性 | 枚举，表示蜂窝网络 |
| ohos.net.connection.ConnectionProperties | `interfaceName` | 属性 | string类型，表示网卡名称 |
| ohos.net.connection.ConnectionProperties | `domains` | 属性 | string类型，表示所属域，默认`""` |
| ohos.net.connection.ConnectionProperties | `linkAddresses` | 属性 | LinkAddress类型的数组，表示链路信息 |
| ohos.net.connection.ConnectionProperties | `routes` | 属性 | RouteInfo类型的数组，表示路由信息 |
| ohos.net.connection.ConnectionProperties | `mtu` | 属性 | number类型，最大传输单元 |
| ohos.net.connection.LinkAddress | `address` | 属性 | NetAddress类型，表示链路地址 |
| ohos.net.connection.LinkAddress | `prefixLength` | 属性 | number类型，表示地址前缀 |
| ohos.net.connection.RouteInfo | `interface` | 属性 | string类型，表示网卡名称 |
| ohos.net.connection.RouteInfo | `destination` | 属性 | LinkAddress类型，表示目的地址 |
| ohos.net.connection.RouteInfo | `gateway` | 属性 | NetAddress类型，表示网关地址 |
| ohos.net.connection.RouteInfo | `hasGateway` | 属性 | boolean类型，表示是否有网关 |
| ohos.net.connection.RouteInfo | `isDefaultRoute` | 属性 | boolean类型，表示是否是默认路由 |
| ohos.net.connection.NetAddress | `address` | 属性 | string类型，表示一个IPv4地址或者IPv6地址 |
| ohos.net.connection.NetAddress | `family` | 属性 | number类型，IPv4 = 1， IPv6 = 2, 默认IPv4 |
| ohos.net.connection.NetAddress | `port` | 属性 | number类型，端口，取值范围`[0, 65535]` |

## 使用说明

以使用默认网络解析域名为例：

`function getAddressesByName(host: string): Promise<Array<NetAddress>>;`

使用默认网络解析域名，以`Promise`的方式异步返回执行结果。

* 参数

  | 参数名 | 类型 | 必填 | 说明 |
  | ----- | ----- | ----- | ----- |
  | `host ` | `string` | 是 | 需要解析的域名 |

* 示例
  ```typescript
  import net_connection from "@ohos.net.connection" 
  ```
  ```js
  net_connection.getAddressesByName("www.example.com").then(function (addresses) {
    console.log(JSON.stringify(addresses))
  })
  ```

## 相关仓

[电话服务子系统](https://gitee.com/openharmony/docs/blob/master/zh-cn/readme/%E7%94%B5%E8%AF%9D%E6%9C%8D%E5%8A%A1%E5%AD%90%E7%B3%BB%E7%BB%9F.md)

[telephony_cellular_data](https://gitee.com/openharmony/telephony_cellular_data)

[netmanager_ext](https://gitee.com/openharmony/communication_netmanager_ext)

[wifi](https://gitee.com/openharmony/communication_wifi)

[napi](https://gitee.com/openharmony/ace_napi)
