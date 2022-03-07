# Net Manager

## 简介

网络管理模块作为电话子系统可裁剪部件，主要分为连接管理、策略管理、流量管理、网络共享、VPN管理、网络共享以及以太网连接等模块；如图1：网络管理架构图；

**图 1**  网络管理架构图

![net_conn_manager_arch_zh](figures/net_conn_manager_arch_zh.png)

## 目录

```
foundation/communication/netmanager_base/
├─figures                     // 架构图
├─frameworks                  // 接口实现
│  ├─js                       // JS接口
│  └─native                   // native接口
├─interfaces                  // 接口定义
│  ├─innerkits                // JS接口
│  └─kits                     // native接口
├─sa_profile                  // sa定义
├─services                    // IPC服务端实现
├─test                        // 测试代码
└─utils                       // 实用工具
```

## 接口说明

| 类型 | 接口 | 功能说明 |
| ---- | ---- | ---- |
| ohos.net.connection | function getDefaultNet(callback: AsyncCallback\<NetHandle>): void; |创建一个含有默认网络的netId的NetHandle对象，调用callback |
| ohos.net.connection | function getDefaultNet(): Promise\<NetHandle>; |创建一个含有默认网络的netId的NetHandle对象，返回Promise |
| ohos.net.connection | function getConnectionProperties(netHandle: NetHandle, callback: AsyncCallback\<ConnectionProperties>): void; |查询默认网络的链路信息，调用callback |
| ohos.net.connection | function getConnectionProperties(netHandle: NetHandle): Promise\<ConnectionProperties>; |查询默认网络的链路信息，返回Promise |
| ohos.net.connection | function getNetCapabilities(netHandle: NetHandle, callback: AsyncCallback\<NetCapabilities>): void; |查询默认网络的能力集信息，调用callback |
| ohos.net.connection | function getNetCapabilities(netHandle: NetHandle): Promise\<NetCapabilities>; |查询默认网络的能力集信息，返回Promise |
| ohos.net.connection | function hasDefaultNet(callback: AsyncCallback\<boolean>): void; |查询是否有默认网络，调用callback |
| ohos.net.connection | function hasDefaultNet(): Promise\<boolean>; |查询是否有默认网络，返回Promise |
| ohos.net.connection | function getAddressesByName(host: string, callback: AsyncCallback\<Array\<NetAddress>>): void; |使用默认网络解析域名，获取所有IP，调用callback |
| ohos.net.connection | function createNetConnection(netSpecifier?: NetSpecifier, timeout?: number): NetConnection; |返回一个NetConnection对象，netSpecifier指定关注的网络的各项特征，timeout是超时时间，netSpecifier是timeout的必要条件，两者都没有则表示关注默认网络 |
| ohos.net.connection | function getAddressesByName(host: string): Promise\<Array\<NetAddress>>; |使用默认网络解析域名，获取所有IP，返回Promise |
| ohos.net.connection.NetHandle | getAddressesByName(host: string, callback: AsyncCallback\<Array\<NetAddress>>): void; |使用对应网络解析域名，获取所有IP，调用callback |
| ohos.net.connection.NetHandle | getAddressesByName(host: string): Promise\<Array\<NetAddress>>; |使用对应网络解析域名，获取所有IP，返回Promise |
| ohos.net.connection.NetHandle | getAddressByName(host: string, callback: AsyncCallback\<NetAddress>): void; |使用对应网络解析域名，获取一个IP，调用callbac |
| ohos.net.connection.NetHandle | getAddressByName(host: string): Promise\<NetAddress>; |使用对应网络解析域名，获取一个IP，返回Promise |
| ohos.net.connection.NetConnection | on(type: 'netAvailable', callback: Callback\<NetHandle>): void; |监听收到网络可用的事件 |
| ohos.net.connection.NetConnection | on(type: 'netCapabilitiesChange', callback: Callback\<{ netHandle: NetHandle, netCap: NetCapabilities }>): void; |监听网络能力变化的事件 |
| ohos.net.connection.NetConnection | on(type: 'netConnectionPropertiesChange', callback: Callback\<{ netHandle: NetHandle, connectionProperties: ConnectionProperties }>): void; |监听网络连接信息变化的事件 |
| ohos.net.connection.NetConnection | on(type: 'netLost', callback: Callback\<NetHandle>): void; |监听网络丢失的事件 |
| ohos.net.connection.NetConnection | on(type: 'netUnavailable', callback: Callback\<void>): void; |监听网络不可用的事件 |
| ohos.net.connection.NetConnection | register(callback: AsyncCallback\<void>): void; |注册默认网络或者createNetConnection中指定的网络的监听 |
| ohos.net.connection.NetConnection | unregister(callback: AsyncCallback\<void>): void; |注销默认网络或者createNetConnection中指定的网络的监听 |

## 使用说明

### 使用默认网络解析域名，以Promise的方式异步返回执行结果。

* 示例
  ```javascript
  import net_connection from "@ohos.net.connection" 
  ```
  ```javascript
  net_connection.getAddressesByName("www.example.com").then(function (addresses) {
    console.log(JSON.stringify(addresses))
  })
  ```

### 注册默认网络的监听。

* 示例
  ```javascript
  import net_connection from "@ohos.net.connection" 
  ```
  ```javascript
  let netConnection = net_connection.createNetConnection()
  netConnection.on('netAvailable', function(data) {
    console.log(JSON.stringify(data))
  })
  netConnection.register(function (error) {
    if (error) {
      console.log(JSON.stringify(error))
    }
  })
  ```

## 相关仓

[网络管理子系统](https://gitee.com/openharmony/docs/blob/master/zh-cn/readme/%E7%BD%91%E7%BB%9C%E7%AE%A1%E7%90%86%E5%AD%90%E7%B3%BB%E7%BB%9F.md)

**communication_netmanager_base**

[communication_netmanager_ext](https://gitee.com/openharmony/communication_netmanager_ext)

[communication_netstack](https://gitee.com/openharmony/communication_netstack)
