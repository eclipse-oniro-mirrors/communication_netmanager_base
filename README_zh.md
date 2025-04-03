# Net Manager

## 简介

网络管理主要分为网络管理、策略管理、流量管理、网络共享、VPN管理以及以太网连接等模块，其中网络管理、策略管理、流量管理为基础服务，归档在netmanager_base仓，以太网连接、网络共享、VPN管理三个模块为可裁剪扩展模块，归档在netmanager_ext仓，netmanager_ext编译构建依赖netmanager_base库内容。如图1：网络管理架构图；

**图 1**  网络管理架构图

![net_manager_arch](figures/net_manager_arch.png)

## 目录

```
foundation/communication/netmanager_base/
├─figures                     # 架构图
├─frameworks                  # 接口实现
│  ├─js                       # JS接口
│  └─native                   # native接口
├─interfaces                  # 接口定义
│  ├─innerkits                # native接口
│  └─kits                     # JS接口
├─sa_profile                  # sa定义
├─services                    # IPC服务端实现
│  ├─common                   # 公用代码存放目录
│  ├─etc                      # 进程配置文件目录
│  ├─netconnmanager           # 网络管理核心代码目录
│  ├─netmanagernative         # 网络子系统服务端代码
│  ├─netpolicymanager         # 策略管理核心代码目录
│  ├─netstatsmanager          # 流量管理核心代码目录
│  ├─netsys_bpf_stats         # bpfReader为service提供功能接口目录
│  └─netsyscontroller         # netsys客户端代码目录
├─test                        # 测试代码
│  ├─fuzztest                 # FUZZ测试目录
│  ├─netconnmanager           # 网络管理单元测试目录
│  ├─netmanagernative         # 网络子系统服务端单元测试目录
│  ├─netpolicymanager         # 策略管理单元测试目录
│  ├─netstatsmanager          # 流量统计单元测试目录
│  └─netsys_bpf_stats         # bpfReader单元测试目录
└─utils                       # 公共功能
   ├─common_utils             # 共同库目录
   ├─log                      # 日志实现目录
   └─napi_utils               # 公用NAPI代码目录
```

## 约束

-    开发语言：C++ JS

## 接口说明

| 类型 | 接口 | 功能说明 |
| ---- | ---- | ---- |
| ohos.net.connection | function getDefaultNet(callback: AsyncCallback\<NetHandle>): void; |获取一个含有默认网络的netId的NetHandle对象，使用callback回调 |
| ohos.net.connection | function getDefaultNet(): Promise\<NetHandle>; |获取一个含有默认网络的netId的NetHandle对象，使用Promise回调 |
| ohos.net.connection | function getAllNets(callback: AsyncCallback\<Array\<NetHandle>>): void;| 获取所处于连接状态的网络的MetHandle对象列表，使用callback回调 |
| ohos.net.connection | function getAllNets(): Promise\<Array\<NetHandle>>;| 获取所有处于连接状态的网络的NetHandle对象列表，使用Promise回调 |
| ohos.net.connection | function getConnectionProperties(netHandle: NetHandle, callback: AsyncCallback\<ConnectionProperties>): void; |查询默认网络的链路信息，使用callback回调 |
| ohos.net.connection | function getConnectionProperties(netHandle: NetHandle): Promise\<ConnectionProperties>; |查询默认网络的链路信息，使用Promise回调 |
| ohos.net.connection | function getNetCapabilities(netHandle: NetHandle, callback: AsyncCallback\<NetCapabilities>): void; |查询默认网络的能力集信息，使用callback回调 |
| ohos.net.connection | function getNetCapabilities(netHandle: NetHandle): Promise\<NetCapabilities>; |查询默认网络的能力集信息，使用Promise回调 |
| ohos.net.connection | function hasDefaultNet(callback: AsyncCallback\<boolean>): void; |查询是否有默认网络，使用callback回调 |
| ohos.net.connection | function hasDefaultNet(): Promise\<boolean>; |查询是否有默认网络，使用Promise回调 |
| ohos.net.connection | function getAddressesByName(host: string, callback: AsyncCallback\<Array\<NetAddress>>): void; |使用对应网络解析域名，获取所有IP，使用callback回调 |
| ohos.net.connection | function getAddressesByName(host: string): Promise\<Array\<NetAddress>>; |使用默认网络解析域名，获取所有IP，使用Promise回调 |
| ohos.net.connection | function createNetConnection(netSpecifier?: NetSpecifier, timeout?: number): NetConnection; | 返回一个NetConnection对象，netSpecifier指定关注的网络的各项特征，timeout是超时时间(单位是毫秒)，netSpecifier是timeout的必要条件，两者都没有则表示关注默认网络 |
| ohos.net.connection | function enableAirplaneMode(callback: AsyncCallback\<void>): void; | 设置网络为飞行模式，使用callback回调 |
| ohos.net.connection | function enableAirplaneMode(): Promise\<void>;|设置网络为飞行模式，使用Promise回调 |
| ohos.net.connection | function disableAirplaneMode(callback: AsyncCallback\<void>): void;| 关闭网络飞行模式，使用callback回调 |
| ohos.net.connection | function disableAirplaneMode(): Promise\<void>;| 关闭网络飞行模式，使用Promise回调 |
| ohos.net.connection | function reportNetConnected(netHandle: NetHandle, callback: AsyncCallback\<void>): void;| 向网络管理报告网络处于可用状态，调用此接口说明应用程序认为网络的可用性（ohos.net.connection.NetCap.NET_CAPABILITY_VAILDATED）与网络管理不一致。使用callback回调 |
| ohos.net.connection | function reportNetConnected(netHandle: NetHandle): Promise\<void>;| 向网络管理报告网络处于可用状态，调用此接口说明应用程序认为网络的可用性（ohos.net.connection.NetCap.NET_CAPABILITY_VAILDATED）与网络管理不一致。使用Promise回调 |
| ohos.net.connection | function reportNetDisconnected(netHandle: NetHandle, callback: AsyncCallback\<void>): void;| 向网络管理报告网络处于不可用状态，调用此接口说明应用程序认为网络的可用性（ohos.net.connection.NetCap.NET_CAPABILITY_VAILDATED）与网络管理不一致。使用callback回调 |
| ohos.net.connection | function reportNetDisconnected(netHandle: NetHandle): Promise\<void>;| 向网络管理报告网络处于不可用状态，调用此接口说明应用程序认为网络的可用性（ohos.net.connection.NetCap.NET_CAPABILITY_VAILDATED）与网络管理不一致。使用Promise回调 |
| ohos.net.connection.NetHandle | bindSocket(socketParam: TCPSocket \| UDPSocket, callback: AsyncCallback\<void>): void; | 将TCPSocket或UDPSockett绑定到当前网络，使用callback回调 |
| ohos.net.connection.NetHandle | bindSocket(socketParam: TCPSocket \| UDPSocket): Promise\<void>;| 将TCPSocket或UDPSockett绑定到当前网络，使用Promise回调 |
| ohos.net.connection.NetHandle | getAddressesByName(host: string, callback: AsyncCallback\<Array\<NetAddress>>): void; |使用默认网络解析域名，获取所有IP，使用callback回调 |
| ohos.net.connection.NetHandle | getAddressesByName(host: string): Promise\<Array\<NetAddress>>; |使用对应网络解析域名，获取所有IP，使用Promise回调 |
| ohos.net.connection.NetHandle | getAddressByName(host: string, callback: AsyncCallback\<NetAddress>): void; |使用对应网络解析域名，获取一个IP，调用callbac |
| ohos.net.connection.NetHandle | getAddressByName(host: string): Promise\<NetAddress>; |使用对应网络解析域名，获取一个IP，使用Promise回调 |
| ohos.net.connection.NetConnection | on(type: 'netAvailable', callback: Callback\<NetHandle>): void; |监听收到网络可用的事件 |
| ohos.net.connection.NetConnection | on(type: 'netCapabilitiesChange', callback: Callback\<{ netHandle: NetHandle, netCap: NetCapabilities }>): void; |监听网络能力变化的事件 |
| ohos.net.connection.NetConnection | on(type: 'netConnectionPropertiesChange', callback: Callback\<{ netHandle: NetHandle, connectionProperties: ConnectionProperties }>): void; |监听网络连接信息变化的事件 |
| ohos.net.connection.NetConnection | on(type: 'netLost', callback: Callback\<NetHandle>): void; |监听网络丢失的事件 |
| ohos.net.connection.NetConnection | on(type: 'netUnavailable', callback: Callback\<void>): void; |监听网络不可用的事件 |
| ohos.net.connection.NetConnection | register(callback: AsyncCallback\<void>): void; |注册默认网络或者createNetConnection中指定的网络的监听 |
| ohos.net.connection.NetConnection | unregister(callback: AsyncCallback\<void>): void; |注销默认网络或者createNetConnection中指定的网络的监听 |
| @ohos.net.policy | function setBackgroundPolicy(allow: boolean, callback: AsyncCallback\<void>): void; | 设置后台网络策略，callback为回调函数 |
| @ohos.net.policy | function setBackgroundPolicy(allow: boolean): Promise\<void>; | 设置后台网络策略 |
| @ohos.net.policy | function getBackgroundPolicy(callback: AsyncCallback\<NetBackgroundPolicy>): void; | 获取后台网络限制策略，callback为回调函数 |
| @ohos.net.policy | function getBackgroundPolicy(): Promise\<NetBackgroundPolicy>; | 获取后台网络限制策略 |
| @ohos.net.policy | function setPolicyByUid(uid: number, policy: NetUidPolicy, callback: AsyncCallback\<void>): void; | 设置对应uid应用的访问计量网络的策略，callback为回调函数 |
| @ohos.net.policy | function setPolicyByUid(uid: number, policy: NetUidPolicy): Promise\<void>; | 设置对应uid应用的访问计量网络的策略 |
| @ohos.net.policy | function getPolicyByUid(uid: number, callback: AsyncCallback\<NetUidPolicy>): void; | 通过应用uid获取策略，callback为回调函数 |
| @ohos.net.policy | function getPolicyByUid(uid: number): Promise\<NetUidPolicy>; | 通过应用uid获取策略 |
| @ohos.net.policy | function getUidsByPolicy(policy: NetUidPolicy, callback: AsyncCallback\<Array\<number>>): void; | 通过策略获取设置这一策略的应用uid数组，callback为回调函数 |
| @ohos.net.policy | function getUidsByPolicy(policy: NetUidPolicy): Promise\<Array\<number>>; | 通过策略获取设置这一策略的应用uid数组 |
| @ohos.net.policy | function getNetQuotaPolicies(callback: AsyncCallback\<Array\<NetQuotaPolicy>>): void; | 获取计量网络策略，callback为回调函数 |
| @ohos.net.policy | function getNetQuotaPolicies(): Promise\<Array\<NetQuotaPolicy>>; | 获取计量网络策略 |
| @ohos.net.policy | function setNetQuotaPolicies(quotaPolicies: Array\<NetQuotaPolicy>, callback: AsyncCallback\<void>): void; | 设置计量网络策略，callback为回调函数 |
| @ohos.net.policy | function setNetQuotaPolicies(quotaPolicies: Array\<NetQuotaPolicy>): Promise\<void>; | 设置计量网络策略 |
| @ohos.net.policy | function restoreAllPolicies(simId: string, callback: AsyncCallback\<void>): void; | 重置对应sim卡id的蜂窝网络、后台网络策略、防火墙策略、应用对应的策略，callback为回调函数 |
| @ohos.net.policy | function restoreAllPolicies(simId: string): Promise\<void>; | 重置对应sim卡id的蜂窝网络、后台网络策略、防火墙策略、应用对应的策略 |
| @ohos.net.policy | function isUidNetAllowedIsMetered(uid: number, isMetered: boolean, callback: AsyncCallback\<boolean>): void; | 获取对应uid能否访问计量或非计量网络，callback为回调函数 |
| @ohos.net.policy | function isUidNetAllowedIsMetered(uid: number, isMetered: boolean): Promise\<boolean>; | 获取对应uid能否访问计量或非计量网络 |
| @ohos.net.policy | function isUidNetAllowedIface(uid: number, iface: string, callback: AsyncCallback\<boolean>): void; | 获取对应uid能否访问指定的iface的网络，callback为回调函数 |
| @ohos.net.policy | function isUidNetAllowedIface(uid: number, iface: string): Promise\<boolean>; | 获取对应uid能否访问指定的iface的网络 |
| @ohos.net.policy | function setDeviceIdleAllowlist(uid: number, isAllow: boolean, callback: AsyncCallback\<void>): void | 设置指定uid能应用是否在休眠防火墙的白名单，callback为回调函数 |
| @ohos.net.policy | function setDeviceIdleAllowlist(uid: number, isAllow: boolean): Promise\<void>; | 设置指定uid能应用是否在休眠防火墙的白名单 |
| @ohos.net.policy | function getDeviceIdleAllowlist(callback: AsyncCallback\<Array\<number>>): void | 获取休眠模式白名单所包含的uid数组，callback为回调函数 |
| @ohos.net.policy | function getDeviceIdleAllowlist(): Promise\<Array\<number>>; | 获取休眠模式白名单所包含的uid数组 |
| @ohos.net.policy | function getBackgroundPolicyByUid(uid: number, callback: AsyncCallback\<NetBackgroundPolicy>): void | 获取指定uid能否访问后台网络，callback为回调函数 |
| @ohos.net.policy | function getBackgroundPolicyByUid(uid: number): Promise\<NetBackgroundPolicy>; | 获取指定uid能否访问后台网络 |
| @ohos.net.policy | function resetPolicies(simId: string, callback: AsyncCallback\<void>): void; | 重置对应sim卡id的蜂窝网络、后台网络策略、防火墙策略、应用对应的策略，callback为回调函数 |
| @ohos.net.policy | function resetPolicies(simId: string): Promise\<void>; | 重置对应sim卡id的蜂窝网络、后台网络策略、防火墙策略、应用对应的策略 |
| @ohos.net.policy | function updateRemindPolicy(netType: NetBearType, simId: string, remindType: RemindType, callback: AsyncCallback\<void>): void | 更新提醒策略，netType为网络类型，simId为SIM卡id， remindType为提醒类型。callback为回调函数|
| @ohos.net.policy | function updateRemindPolicy(netType: NetBearType, simId: string, remindType: RemindType): Promise\<void>; | 更新提醒策略，netType为网络类型，simId为SIM卡id， remindType为提醒类型 |
| @ohos.net.policy | function on(type: 'netUidPolicyChange', callback: Callback\<{ uid: number, policy: NetUidPolicy }>): void; | 注册policy发生改变时的回调 |
| @ohos.net.policy | function off(type: 'netUidPolicyChange', callback: Callback\<void>): void; | 反注册policy发生改变时的回调 |
| @ohos.net.policy | function on(type: "netUidRuleChange", callback: Callback\<{ uid: number, rule: NetUidRule }>): void; | 注册rule发生改变时的回调 |
| @ohos.net.policy | function off(type: "netUidRuleChange", callback: Callback\<void>): void; | 反注册rule发生改变时的回调 |
| @ohos.net.policy | function on(type: "netMeteredIfacesChange", callback: Callback\<Array\<string>>): void; | 注册计量iface发生改变时的回调 |
| @ohos.net.policy | function off(type: "netMeteredIfacesChange", callback: Callback\<void>): void; | 反注册计量iface发生改变时的回调 |
| @ohos.net.policy | function on(type: "netQuotaPolicyChange", callback: Callback\<Array\<NetQuotaPolicy>>): void; | 注册计量网络策略发生改变时的回调 |
| @ohos.net.policy | function off(type: "netQuotaPolicyChange", callback: Callback\<void>): void; | 反注册计量网络策略发生改变时的回调 |
| @ohos.net.policy | function on(type: "netBackgroundPolicyChange", callback: Callback\<boolean>): void; | 注册后台网络策略发生改变时的回调 |
| @ohos.net.policy | function off(type: "netBackgroundPolicyChange", callback: Callback\<void>): void; | 反注册后台网络策略发生改变时的回调 |
| ohos.net.statistics | function getIfaceRxBytes(nic: string, callback: AsyncCallback\<number>): void; |查询指定网卡的下行流量数据，使用callback回调|
| ohos.net.statistics | function getIfaceRxBytes(nic: string): Promise\<number>; |获取指定网卡的下行流量数据，使用Promise回调 |
| ohos.net.statistics | function getIfaceTxBytes(nic: string, callback: AsyncCallback\<number>): void; |查询指定网卡的上行流量数据，使用callback回调 |
| ohos.net.statistics | function getIfaceRxBytes(nic: string): Promise\<number>; |获取指定网卡的上行流量数据，使用Promise回调 |
| ohos.net.statistics | function getCellularRxBytes(callback: AsyncCallback\<number>): void; |查询指蜂窝网的下行流量数据，使用callback回调 |
| ohos.net.statistics | function getCellularRxBytes(): Promise\<number>; |查询指蜂窝网的下行流量数据，使用Promise回调 |
| ohos.net.statistics | function getCellularTxBytes(callback: AsyncCallback\<number>): void; |查询蜂窝网的上行流量数据，使用callback回调 |
| ohos.net.statistics | function getCellularTxBytes(): Promise\<number>; |查询蜂窝网的上行流量数据，使用Promise回调 |
| ohos.net.statistics | function getAllRxBytes(callback: AsyncCallback\<number>): void; |查询所有网卡的下行流量数据，使用callback回调 |
| ohos.net.statistics | function getAllRxBytes(): Promise\<number>; |查询所有网卡的下行流量数据，使用Promise回调 |
| ohos.net.statistics | function getAllTxBytes(callback: AsyncCallback\<number>): void; |查询所有网卡的上行流量数据，使用callback回调 |
| ohos.net.statistics | function getAllTxBytes(): Promise\<number>; |查询所有网卡的上行流量数据，使用Promise回调 |
| ohos.net.statistics | function getUidRxBytes(uid: number, callback: AsyncCallback\<number>): void; |查询指定应用的下行流量数据，使用callback回调 |
| ohos.net.statistics | function getUidRxBytes(uid: number): Promise\<number>; |查询指定应用的下行流量数据，使用Promise回调 |
| ohos.net.statistics | function getUidTxBytes(uid: number, callback: AsyncCallback\<number>): void; |查询指定应用的上行流量数据，使用callback回调 |
| ohos.net.statistics | function getUidTxBytes(uid: number): Promise\<number>; |查询指定应用的上行流量数据，使用Promise回调 |

完整的JS API说明以及实例代码请参考：[网络管理](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis/js-apis-net-connection.md)。

## 接口使用说明

### 注册默认网络或者createNetConnection中指定的网络的监听

* 示例
  ```javascript
  import connection from '@ohos.net.connection'
  ```
  ```javascript
  let connection = connection.createNetConnection()
  connection.on('netAvailable', function(data) {
    console.log(JSON.stringify(data))
  })
  connection.register(function (error) {
    if (error) {
      console.log(JSON.stringify(error))
    }
  })
  ```

### 获取所有注册的网络

* 示例
  ```javascript
  import connection from '@ohos.net.connection'
  ```
  ```javascript
  connection.getAllNets((err, data) => {
      this.callBack(err, data);
      if (data) {
          this.netList = data;
      }
  })
  ```

### 查询默认网络的链路信息

* 示例
  ```javascript
  import connection from '@ohos.net.connection'
  ```
  ```javascript
  connection.getConnectionProperties(this.netHandle, (err, data) => {
      this.callBack(err, data);
  })
  ```

### 使用对应网络解析域名，获取所有IP

* 示例
  ```javascript
  import connection from '@ohos.net.connection'
  ```
  ```javascript
  connection.getAddressesByName(this.host, (err, data) => {
      this.callBack(err, data);
  })
  ```

### 设置后台网络策略

* 示例

  ```javascript
  import policy from '@ohos.net.policy'
  ```

  ```javascript
  policy.setBackgroundPolicy(Boolean(Number.parseInt(this.isBoolean)), (err, data) => {
      this.callBack(err, data);
  })
  ```

### 获取后台网络限制策略

* 示例

  ```javascript
  import policy from '@ohos.net.policy'
  ```
  ```javascript
  policy.getBackgroundPolicy((err, data) => {
      this.callBack(err, data);
  })
  ```

### 设置对应uid应用的访问计量网络的策略

* 示例

  ```javascript
  import policy from '@ohos.net.policy'
  ```
  ```javascript
  policy.setPolicyByUid(Number.parseInt(this.firstParam), Number.parseInt(this.currentNetUidPolicy), (err, data) => {
      this.callBack(err, data);
  })
  ```

### 设置指定uid能应用是否在休眠防火墙的白名单

* 示例

  ```javascript
  import policy from '@ohos.net.policy'
  ```

  ```javascript
  policy.setDeviceIdleAllowList(Number.parseInt(this.firstParam), Boolean(Number.parseInt(this.isBoolean)), (err, data) => {
      this.callBack(err, data);
  })

### 查询指定网卡的下行流量数据，以Promise的方式异步返回执行结果。

* 示例
  ```javascript
  import statistics from '@ohos.net.statistics'
  ```
  ```javascript
  statistics.getIfaceRxBytes("wlan0").then(function (addresses) {
    console.log(JSON.stringify(addresses))
  })
  ```

### 查询指定网卡的上行流量数据，以Promise的方式异步返回执行结果。。

* 示例
  ```javascript
  import statistics from '@ohos.net.statistics'
  ```
  ```javascript
  statistics.getIfaceTxBytes("wlan0").then(function (addresses) {
    console.log(JSON.stringify(addresses))
  })
  ```

## 相关仓

[网络管理子系统](https://gitee.com/openharmony/docs/blob/master/zh-cn/readme/%E7%BD%91%E7%BB%9C%E7%AE%A1%E7%90%86%E5%AD%90%E7%B3%BB%E7%BB%9F.md)

**communication_netmanager_base**

[communication_netmanager_ext](https://gitee.com/openharmony/communication_netmanager_ext)

[communication_netstack](https://gitee.com/openharmony/communication_netstack)

----------------------------------
foundation/communication/netmanager_base/test/netconnmanager/unittest/net_conn_manager_test/net_conn_service_ext_test.cpp
HWTEST_F(NetConnServiceExtTest, IsIfaceNameInUseTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    int32_t netId = 1;
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    std::shared_ptr<Network> network = std::make_shared<Network>(netId, netId, nullptr,
        NetBearType::BEARER_ETHERNET, nullptr);
    supplier->network_ = network;
    supplier->netSupplierInfo_.isAvailable_ = true;
    supplier->network_->netLinkInfo_.ifaceName_ = "rmnet0";
    netConnService->netSuppliers_.clear();
    netConnService->netSuppliers_[1] = supplier;
    auto ret = netConnService->IsIfaceNameInUse("rmnet0", 100);
    EXPECT_TRUE(ret);
}

HWTEST_F(NetConnServiceExtTest, GetNetCapabilitiesAsStringTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->netSuppliers_[0] = nullptr;
    uint32_t supplierId = 2;
    auto ret = netConnService->GetNetCapabilitiesAsString(supplierId);
    EXPECT_TRUE(ret.empty());

    supplierId = 0;
    ret = netConnService->GetNetCapabilitiesAsString(supplierId);
    EXPECT_TRUE(ret.empty());

    supplierId = 1;
    ret = netConnService->GetNetCapabilitiesAsString(supplierId);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(NetConnServiceExtTest, FindSupplierWithInternetByBearerTypeTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_FALSE(netConnService->netSuppliers_[1]->GetNetCaps().HasNetCap(NET_CAPABILITY_INTERNET));
    auto ret = netConnService->FindSupplierWithInternetByBearerType(NetBearType::BEARER_WIFI, TEST_IDENT);
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(NetConnServiceExtTest, DecreaseSupplierScoreTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t supplierId = 1;
    EXPECT_EQ(netConnService->netConnEventHandler_, nullptr);
    auto ret = netConnService->DecreaseSupplierScore(NetBearType::BEARER_WIFI, TEST_IDENT, supplierId);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, IncreaseSupplierScoreTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t supplierId = 1;
    EXPECT_EQ(netConnService->netConnEventHandler_, nullptr);
    auto ret = netConnService->IncreaseSupplierScore(supplierId);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, DecreaseSupplierScoreTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t supplierId = 1;
    netConnService->netConnEventHandler_ = std::make_shared<NetConnEventHandler>(netConnService->netConnEventRunner_);
    EXPECT_NE(netConnService->netConnEventHandler_, nullptr);
    auto ret = netConnService->DecreaseSupplierScore(NetBearType::BEARER_WIFI, TEST_IDENT, supplierId);
    EXPECT_NE(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, IncreaseSupplierScoreTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t supplierId = 1;
    EXPECT_NE(netConnService->netConnEventHandler_, nullptr);
    auto ret = netConnService->IncreaseSupplierScore(supplierId);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, DecreaseSupplierScoreAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t supplierId = 1;
    auto ret = netConnService->DecreaseSupplierScoreAsync(NetBearType::BEARER_WIFI, TEST_IDENT, supplierId);
    EXPECT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetConnServiceExtTest, IncreaseSupplierScoreAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t supplierId = 2;
    auto ret = netConnService->IncreaseSupplierScoreAsync(supplierId);
    EXPECT_EQ(ret, NETMANAGER_ERR_INVALID_PARAMETER);
}

HWTEST_F(NetConnServiceExtTest, FindSupplierToReduceScoreTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->defaultNetSupplier_ = nullptr;
    uint32_t supplierId = 2;
    std::vector<sptr<NetSupplier>> suppliers;
    auto ret = netConnService->FindSupplierToReduceScore(suppliers, supplierId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetConnServiceExtTest, OnRemoteDiedTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    EXPECT_EQ(netConnService->defaultNetSupplier_, nullptr);
    wptr<IRemoteObject> remoteObject = nullptr;
    netConnService->OnRemoteDied(remoteObject);
}

HWTEST_F(NetConnServiceExtTest, OnRemoteDiedTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    wptr<IRemoteObject> remoteObject = new MockNetIRemoteObject();
    EXPECT_NE(remoteObject, nullptr);
    netConnService->OnRemoteDied(remoteObject);
}

HWTEST_F(NetConnServiceExtTest, FindSupplierForConnectedTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::vector<sptr<NetSupplier>> suppliers = {nullptr};
    auto ret = netConnService->FindSupplierForConnected(suppliers);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(NetConnServiceExtTest, OnReceiveEventTest001, TestSize.Level1)
{
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    NetConnService::NetConnListener listener(subscribeInfo, nullptr);
    EXPECT_EQ(NetConnService::GetInstance()->defaultNetSupplier_, nullptr);
    EventFwk::CommonEventData eventData;
    listener.OnReceiveEvent(eventData);
}

HWTEST_F(NetConnServiceExtTest, EnableVnicNetworkTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    sptr<NetLinkInfo> netLinkInfo = new NetLinkInfo();
    const std::set<int32_t> uids;
    EXPECT_NE(netConnService->netConnEventHandler_, nullptr);
    auto ret = netConnService->EnableVnicNetwork(netLinkInfo, uids);
    EXPECT_NE(ret, NETMANAGER_ERROR);

    netConnService->netConnEventHandler_ = nullptr;
    ret = netConnService->EnableVnicNetwork(netLinkInfo, uids);
    EXPECT_EQ(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, EnableVnicNetworkAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    sptr<NetLinkInfo> netLinkInfo = new NetLinkInfo();
    const std::set<int32_t> uids;

    NetManagerStandard::INetAddr inetAddr;
    inetAddr.type_ = NetManagerStandard::INetAddr::IpType::IPV4;
    inetAddr.family_ = 0x01;
    inetAddr.address_ = "10.0.0.2.1";
    inetAddr.netMask_ = "255.255.255.0";
    inetAddr.hostName_ = "localhost";
    inetAddr.port_ = 80;
    inetAddr.prefixlen_ = 24;
    netLinkInfo->ifaceName_ = "vnic-tun";
    netLinkInfo->netAddrList_.push_back(inetAddr);
    netLinkInfo->mtu_ = 1500;

    auto ret = netConnService->EnableVnicNetworkAsync(netLinkInfo, uids);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

HWTEST_F(NetConnServiceExtTest, DisableVnicNetworkTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    auto ret = netConnService->DisableVnicNetwork();
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    netConnService->netConnEventHandler_ = std::make_shared<NetConnEventHandler>(netConnService->netConnEventRunner_);
    EXPECT_NE(netConnService->netConnEventHandler_, nullptr);
    ret = netConnService->DisableVnicNetwork();
    EXPECT_NE(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, EnableDistributedClientNetAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string virnicAddr;
    std::string iif;
    auto ret = netConnService->EnableDistributedClientNetAsync(virnicAddr, iif);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

HWTEST_F(NetConnServiceExtTest, EnableDistributedClientNetAsyncTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string virnicAddr = "192.168.1.300";
    std::string iif = "eth0";
    auto ret = netConnService->EnableDistributedClientNetAsync(virnicAddr, iif);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

HWTEST_F(NetConnServiceExtTest, EnableDistributedClientNetAsyncTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string virnicAddr = "192.168.1.5";
    std::string iif = "eth0";
    auto ret = netConnService->EnableDistributedClientNetAsync(virnicAddr, iif);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, EnableDistributedServerNetTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string iif = "eth0";
    std::string devIface = "bond0";
    std::string dstAddr = "192.168.1.100";
    auto tmpHandler = netConnService->netConnEventHandler_;
    netConnService->netConnEventHandler_ = nullptr;
    auto ret = netConnService->EnableDistributedServerNet(iif, devIface, dstAddr);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    netConnService->netConnEventHandler_ = tmpHandler;
    ret = netConnService->EnableDistributedServerNet(iif, devIface, dstAddr);
    EXPECT_NE(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, EnableDistributedServerNetAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string iif;
    std::string devIface;
    std::string dstAddr;
    auto ret = netConnService->EnableDistributedServerNetAsync(iif, devIface, dstAddr);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);

    iif = "eth0";
    ret = netConnService->EnableDistributedServerNetAsync(iif, devIface, dstAddr);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);

    devIface = "bond0";
    dstAddr = "192.168.1.300";
    ret = netConnService->EnableDistributedServerNetAsync(iif, devIface, dstAddr);
    EXPECT_EQ(ret, NET_CONN_ERR_INVALID_NETWORK);
}

HWTEST_F(NetConnServiceExtTest, EnableDistributedServerNetAsyncTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    std::string iif = "eth0";
    std::string devIface = "bond0";
    std::string dstAddr = "192.168.1.100";
    auto ret = netConnService->EnableDistributedServerNetAsync(iif, devIface, dstAddr);
    EXPECT_EQ(ret, NETMANAGER_ERR_OPERATION_FAILED);
}

HWTEST_F(NetConnServiceExtTest, DisableDistributedNetTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    auto tmpHandler = netConnService->netConnEventHandler_;
    netConnService->netConnEventHandler_ = nullptr;
    auto ret = netConnService->DisableDistributedNet(true);
    EXPECT_EQ(ret, NETMANAGER_ERROR);

    netConnService->netConnEventHandler_ = tmpHandler;
    ret = netConnService->DisableDistributedNet(true);
    EXPECT_NE(ret, NETMANAGER_ERROR);
}

HWTEST_F(NetConnServiceExtTest, DisableDistributedNetAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    auto ret = netConnService->DisableDistributedNetAsync(false);
    EXPECT_EQ(ret, NETMANAGER_ERR_OPERATION_FAILED);
}

HWTEST_F(NetConnServiceExtTest, CloseSocketsUidAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    int32_t netId = 0;
    uint32_t uid = 1;
    EXPECT_EQ(netConnService->networks_.find(netId), netConnService->networks_.end());
    auto ret = netConnService->CloseSocketsUidAsync(netId, uid);
    EXPECT_EQ(ret, NET_CONN_ERR_NETID_NOT_FOUND);

    netId = 1;
    EXPECT_EQ(netConnService->networks_[netId], nullptr);
    ret = netConnService->CloseSocketsUidAsync(netId, uid);
    EXPECT_EQ(ret, NET_CONN_ERR_NETID_NOT_FOUND);
}

HWTEST_F(NetConnServiceExtTest, SetAppIsFrozenedAsyncTest001, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t uid = 1;
    netConnService->netUidActivates_.clear();
    auto ret = netConnService->SetAppIsFrozenedAsync(uid, true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, SetAppIsFrozenedAsyncTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t uid = 1;
    bool isFrozened = false;
    std::vector<std::shared_ptr<NetActivate>> activates;
    sptr<NetSpecifier> specifier = nullptr;
    sptr<INetConnCallback> callback = nullptr;
    std::weak_ptr<INetActivateCallback> timeoutCallback;
    std::shared_ptr<AppExecFwk::EventHandler> handler = nullptr;
    auto active = std::make_shared<NetActivate>(specifier, callback, timeoutCallback, 0, handler);
    activates.push_back(active);
    activates[0]->SetIsAppFrozened(isFrozened);
    netConnService->netUidActivates_[uid] = activates;
    auto ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    isFrozened = true;
    ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    isFrozened = false;
    activates[0]->SetIsAppFrozened(true);
    activates[0]->SetLastCallbackType(CALL_TYPE_UNKNOWN);
    ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, SetAppIsFrozenedAsyncTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t uid = 1;
    bool isFrozened = false;
    auto &activates = netConnService->netUidActivates_[uid];
    activates[0]->SetLastCallbackType(CallbackType::CALL_TYPE_AVAILABLE);
    auto ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    activates[0]->SetLastCallbackType(CallbackType::CALL_TYPE_LOST);
    EXPECT_EQ(activates[0]->GetLastServiceSupply(), nullptr);
    ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    activates[0]->SetLastServiceSupply(supplier);
    EXPECT_EQ(activates[0]->GetNetCallback(), nullptr);
    ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    activates[0]->netConnCallback_ = new (std::nothrow) NetConnCallbackStubCb();
    ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, SetAppIsFrozenedAsyncTest004, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t uid = 1;
    bool isFrozened = false;
    auto &activates = netConnService->netUidActivates_[uid];
    activates[0]->SetServiceSupply(activates[0]->GetLastServiceSupply());
    activates[0]->SetLastCallbackType(CallbackType::CALL_TYPE_AVAILABLE);
    EXPECT_NE(activates[0]->GetServiceSupply(), nullptr);
    auto ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    activates[0]->SetLastCallbackType(CallbackType::CALL_TYPE_LOST);
    ret = netConnService->SetAppIsFrozenedAsync(uid, isFrozened);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, EnableAppFrozenedCallbackLimitationTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    netConnService->netConnEventHandler_ = nullptr;
    auto ret = netConnService->EnableAppFrozenedCallbackLimitation(true);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, SetReuseSupplierIdTest002, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t supplierId = 1;
    uint32_t reuseSupplierId = 1;
    netConnService->netSuppliers_.clear();
    netConnService->netSuppliers_[0] = nullptr;
    auto ret = netConnService->SetReuseSupplierId(supplierId, reuseSupplierId, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

HWTEST_F(NetConnServiceExtTest, SetReuseSupplierIdTest003, TestSize.Level1)
{
    auto netConnService = NetConnService::GetInstance();
    uint32_t supplierId = 1;
    uint32_t reuseSupplierId = 2;
    std::string netSupplierIdent;
    std::set<NetCap> netCaps;
    sptr<NetSupplier> supplier = new NetSupplier(BEARER_CELLULAR, netSupplierIdent, netCaps);
    supplier->supplierId_ = supplierId;
    netConnService->netSuppliers_[1] = supplier;
    auto ret = netConnService->SetReuseSupplierId(supplierId, reuseSupplierId, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    netConnService->netSuppliers_[1]->supplierId_ = reuseSupplierId;
    ret = netConnService->SetReuseSupplierId(supplierId, reuseSupplierId, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);

    netConnService->netSuppliers_[1]->supplierId_ = 0;
    ret = netConnService->SetReuseSupplierId(supplierId, reuseSupplierId, false);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}
