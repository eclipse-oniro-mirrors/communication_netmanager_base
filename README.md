# Net Manager<a name="EN-US_TOPIC_0000001105058232"></a>

-    [简介](#简介)
-    [目录](#目录)
-    [约束](#约束)
-    [接口说明](#接口说明)
-    [使用说明](#使用说明)
-    [相关仓](#相关仓)



## 简介

网络管理介绍：

​    网络管理模块作为电话子系统可裁剪部件，主要分为连接管理、策略管理、流量管理、网络共享、VPN管理、网络共享以及以太网连接等模块；如图1：网络管理架构图；

**图 1**  网络管理架构图

![net_conn_manager_arch_zh](figures\net_conn_manager_arch_zh.png)

## 目录

```
foundation/communication/netmanager_base/
├─figures
├─frameworks
│  ├─js
│  │  └─napi
│  │      ├─common
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
│  └─innerkits
│      ├─dnsresolverclient
│      │  └─include
│      │      └─proxy
│      ├─include
│      ├─netconnclient
│      │  └─include
│      │      └─proxy
│      ├─netmanagernative
│      │  └─include
│      ├─netpolicyclient
│      │  └─include
│      │      └─proxy
│      └─netstatsclient
│          └─include
│              └─proxy
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
    └─log
        ├─include
        └─src
```

## 约束

-    开发语言：C++
-    软件层，需要以下子系统和服务配合使用：蜂窝数据、WiFi系统、安全子系统、软总线子系统、USB子系统、电源管理子系统等；
-    硬件层，需要搭载的设备支持以下硬件：可以进行独立蜂窝通信的Modem以及SIM卡；

## 接口说明

| 接口名                                                       | 接口说明                        | 所需权限                           |
| :----------------------------------------------------------- | ------------------------------- | ---------------------------------- |
| `function` [setUidPolicy](#1.2) `(uid: number, policy: NetUidPolicy, callback: AsyncCallback<NetPolicyResultCode>): void` | 设置UID与对应的策略信息         | `ohos.permission.GET_NETWORK_INFO` |
| `function` [setUidPolicy](#1.3) `(uid: number, policy: NetUidPolicy): Promise<NetPolicyResultCode>` | 设置UID与对应的策略信息         | `ohos.permission.GET_NETWORK_INFO` |
| `function` [getUidPolicy](#1.4) `(uid: number, callback: AsyncCallback<NetUidPolicy>): void` | 根据UID获取对应的策略信息       | `ohos.permission.GET_NETWORK_INFO` |
| `function` [getUidPolicy](#1.5) `(uid: number): Promise<NetUidPolicy>` | 根据UID获取对应的策略信息       | `ohos.permission.GET_NETWORK_INFO` |
| `function` [getUids](#1.6) `(policy: NetUidPolicy, callback: AsyncCallback<Array<uint32>>): void` | 获取使用该策略信息的UID         | `ohos.permission.GET_NETWORK_INFO` |
| `function` [getUids](#1.7) `(policy: NetUidPolicy): Promise<Array<uint32>>` | 获取使用该策略信息的UID         | `ohos.permission.GET_NETWORK_INFO` |
| `function` [isUidNetAccess](#1.8) `(uid: number, metered: boolean, callback: AsyncCallback<boolean>): void` | 根据UID与跃点情况确定是否可访问 | `ohos.permission.GET_NETWORK_INFO` |
| `function` [isUidNetAccess](#1.9) `(uid: number, metered: boolean): Promise<boolean>` | 根据UID与跃点情况确定是否可访问 | `ohos.permission.GET_NETWORK_INFO` |
| `function` [isUidNetAccess](#1.10) `(uid: number, ifaceName: string, callback: AsyncCallback<boolean>): void` | 根据UID与接口名确定是否可访问   | `ohos.permission.GET_NETWORK_INFO` |
| `function` [isUidNetAccess](#1.11) `(uid: number, ifaceName: string): Promise<boolean>` | 根据UID与接口名确定是否可访问   | `ohos.permission.GET_NETWORK_INFO` |
| `function` [SetInterfaceConfiguration](#1.12) `(iface: string, ic: InterfaceConfiguration, callback: AsyncCallback<number>): void` | 设置网络接口配置信息            | `ohos.permission.GET_NETWORK_INFO` |
| `function` [SetInterfaceConfiguration](#1.13) `(iface: string, ic: InterfaceConfiguration): Promise<number>` | 设置网络接口配置信息            | `ohos.permission.GET_NETWORK_INFO` |
| `function` [GetInterfaceConfiguration](#1.14) `(iface: string, callback: AsyncCallback<InterfaceConfiguration>): void` | 获得该接口的配置信息            | `ohos.permission.GET_NETWORK_INFO` |
| `function` [GetInterfaceConfiguration](#1.15) `(iface: string): Promise<InterfaceConfiguration>` | 获得该接口的配置信息            | `ohos.permission.GET_NETWORK_INFO` |
| `function` [Whether2Activate](#1.16) `(iface: string, callback: AsyncCallback<number>): void` | 判断接口是否已激活              | `ohos.permission.GET_NETWORK_INFO` |
| `function` [Whether2Activate](#1.17) `(iface: string): Promise<number>` | 判断接口是否已激活              | `ohos.permission.GET_NETWORK_INFO` |
| `function` [GetActivateInterfaces](#1.18) `(callback: AsyncCallback<Array<string>>): void` | 获取活动的网络接口              | `ohos.permission.GET_NETWORK_INFO` |
| `function` [GetActivateInterfaces](#1.19) `(void): Promise<Array<string>>` | 获取活动的网络接口              | `ohos.permission.GET_NETWORK_INFO` |
| `function` [GetAddressesByName](#1.20) `(hostName: string, callback: AsyncCallback<Array<string>>): void` | 根据域名获取地址信息            | `ohos.permission.GET_NETWORK_INFO` |
| `function` [GetAddressesByName](#1.21) `(hostName: string): Promise<Array<string>>` | 根据域名获取地址信息            | `ohos.permission.GET_NETWORK_INFO` |

## 使用说明

以设置UID与对应的策略信息setUidPolicy接口为例：

`setUidPolicy(uid: number, policy: NetUidPolicy, callback: AsyncCallback<NetPolicyResultCode>): void`

设置UID与对应的策略信息，以`callback`的方式异步返回执行结果。

* 参数

  | 参数名     | 类型                                                         | 必填 | 说明                                                         |
  | ---------- | ------------------------------------------------------------ | :--- | ------------------------------------------------------------ |
  | `uid `     | `number`                                                     | 是   | UID                                                          |
  | `policy `  | [NetUidPolicy](#NetUidPolicy)                                | 是   | 对应的策略信息                                               |
  | `callback` | `AsyncCallback<`[NetPolicyResultCode](#NetPolicyResultCode)`>` | 是   | 设置UID与对应的策略信息的异步回调方法，回调值类型详情见[NetPolicyResultCode](#NetPolicyResultCode)说明。 |

 

* 示例

  ```js
  netpolicy.setUidPolicy(100, 200, (err,data) => {
      if(err){
          console.log("data.setUidPolicy = "+ err);
          return;
      }
      console.log("data.setUidPolicy success setUidPolicy = "+ data );
  });
  ```

## 相关仓

netmanager_base

netmanager_ext

[电话服务子系统](https://gitee.com/openharmony/docs/blob/master/zh-cn/readme/%E7%94%B5%E8%AF%9D%E6%9C%8D%E5%8A%A1%E5%AD%90%E7%B3%BB%E7%BB%9F.md)

[ telephony_cellular_data](https://gitee.com/openharmony/telephony_cellular_data/blob/master/README.md)

