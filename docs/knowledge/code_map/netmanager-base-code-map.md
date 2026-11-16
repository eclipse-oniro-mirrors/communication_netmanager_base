# netmanager_base 总体代码地图

> 本文是 netmanager_base 仓的全局导航图：给出五层分层、目录职责、双进程 8 SA 总览、错误码/权限/观测三大公共基线、以及与 netstack/netmanager_ext 的边界。任何任务动手前先在此定位"代码在哪一层、归哪个 SA、先读哪篇专题文档"。

## 1. 组件定位与形态

- 职责：为系统与应用提供网络连接管理（注册/评选/探测）、网络策略（Uid 规则/防火墙）、流量统计与限额、以及 netsysnative 内核网络通道（路由/DNS/带宽/防火墙的落地执行）。
- 形态：**双进程多 SA 的系统服务仓**——`netmanager` 进程承载管理面 SA，`netsysnative` 进程承接内核面执行；另有 `frameworks` 客户端库与 JS/NDK/ANI/cj 多语言绑定。
- 与 netstack 的关系：netstack 的 HTTP 栈经 `NetConnClient` 获取代理与网络信息（编译期宏 `HAS_NETMANAGER_BASE` 控制接入），本仓是被依赖方。
- 与 netmanager_ext 的关系：本仓持有 1154/1155/1157 三个 SA 的 profile，但**实现代码在 netmanager_ext 仓**（网络共享/VPN/以太网）；修改这三类能力要去 ext 仓改。
- 特性开关集中在 `netmanager_base_config.gni` 的 `declare_args()`（如 `netmanager_base_enable_feature_net_firewall`、`netmanager_base_enable_feature_sysvpn`、`netmanager_base_enable_pac_proxy`、`netmanager_base_share_traffic_limit_enable`、`netmanager_base_extended_features`，完整清单见 [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md) 末节）。
- 观测面：`hisysevent.yaml` 声明 `NETMANAGER` domain（FAULT/BEHAVIOR/STATISTIC 事件），见 §7。

## 2. 分层架构（自上而下，依赖只能向下）

```
JS API 声明        interfaces/kits/js/@ohos.net.{connection,policy,statistics}.d.ts + @system.network.d.ts
   ↓
NAPI 层            frameworks/js/napi/{connection,netpolicy,netstats,network,common}
   ↓
客户端（inner API） frameworks/native/{netconnclient,netpolicyclient,netstatsclient,netmanagernative}
                   + interfaces/innerkits/{include,netconnclient,netpolicyclient,netstatsclient,netmanagernative}
   ↓
服务端（SA stub）   services/{netconnmanager,netpolicymanager,netstatsmanager}(netmanager 进程)
   ↓
内核通道           services/netsyscontroller → NetsysNativeClient --IPC--> NetsysNativeService（netsysnative 进程）
   ↓
数据面/内核        services/netmanagernative/{bpf,netsys,manager} + bpf/bpf_progs（BPF 程序）+ netlink/iptables
```

- 多语言绑定与主链路平行：`frameworks/ets/ani`（ArkTS Rust ANI，connection/policy/statistics）、`frameworks/cj`（仓颉，connection/statistics）、`interfaces/kits/c/netconnclient`（NDK）、`common/ani_rs`（Rust 支撑库）。同一能力变更需同步所有绑定。
- 架构不变量详见 [../expert/architecture-invariants.md](../expert/architecture-invariants.md)。

## 3. 二级目录职责表

| 路径 | 职责 | 关键文件 |
|---|---|---|
| `bundle.json` | 组件清单与部件级 feature 声明 | — |
| `netmanager_base_config.gni` | GN 编译参数与全部特性开关（`declare_args`） | — |
| `hisysevent.yaml` | 维测事件声明（domain `NETMANAGER`） | — |
| `sa_profile/` | 8 个 SA 的 profile（1151~1158.json） | 详见 [sa-architecture.md](sa-architecture.md) |
| `interfaces/kits/js` | 公共 JS API 声明 | `@ohos.net.connection.d.ts`、`@ohos.net.policy.d.ts`、`@ohos.net.statistics.d.ts`、`@system.network.d.ts` |
| `interfaces/kits/c` | NDK 头文件 | `netconnclient/` |
| `interfaces/innerkits/include` | 跨模块公共常量 | `net_manager_constants.h`（错误码） |
| `interfaces/innerkits/netconnclient` | 连接管理 inner API | `net_conn_client.h`（NetConnClient）、`net_all_capabilities.h`（NetCap/NetBearType）、`net_supplier_info.h`（NetConnState）、`net_handle.h`、`net_specifier.h`、`net_link_info.h`、`http_proxy.h`、`proxy/conn_ipc_interface_code.h`（IPC 命令字） |
| `interfaces/innerkits/netpolicyclient` | 策略 inner API | `policy_ipc_interface_code.h`（PolicyInterfaceCode） |
| `interfaces/innerkits/netstatsclient` | 统计 inner API | `proxy/stats_ipc_interface_code.h`（StatsInterfaceCode） |
| `interfaces/innerkits/netmanagernative` | netsys inner API | `netsys_ipc_interface_code.h`（NetsysInterfaceCode） |
| `frameworks/js/napi/connection` | 连接管理 NAPI exec | `connection_module/`、`connection_exec/`、`observer/`、`net_handle_interface/` |
| `frameworks/js/napi/netpolicy`、`netstats` | 策略/统计 NAPI exec | 按同名模块组织 |
| `frameworks/js/napi/network`、`common` | 网络信息查询 NAPI 与公共设施 | — |
| `frameworks/native/netconnclient` | 连接客户端库（SA 1151 的 proxy 侧） | `src/net_conn_client.cpp`、`src/proxy/net_conn_service_proxy.cpp`（2607 行） |
| `frameworks/native/netpolicyclient` | 策略客户端库（SA 1152 proxy 侧） | `src/net_policy_service_proxy.cpp` |
| `frameworks/native/netstatsclient` | 统计客户端库（SA 1153 proxy 侧） | `src/proxy/net_stats_service_proxy.cpp` |
| `frameworks/native/netmanagernative` | netsys proxy 与防火墙回调 | `netsys_native_service_proxy.cpp`（4548 行）、`netfirewall_callback_proxy.cpp` |
| `frameworks/native/netsysclient` | netsys 流量回调 proxy | `netsys_traffic_callback_proxy.cpp` |
| `frameworks/ets/ani` | ArkTS↔Rust ANI 绑定 | `connection/`、`policy/`、`statistics/` |
| `frameworks/cj` | 仓颉绑定 | `connection/`、`statistics/` |
| `common/ani_rs` | ANI Rust 支撑库 | — |
| `services/netconnmanager` | SA 1151 服务端（连接管理核心） | `src/net_conn_service.cpp`（4787 行）、`stub/net_conn_service_stub.cpp`（2416 行）、`net_supplier.cpp`、`network.cpp`、`net_monitor.cpp`、`probe_thread.cpp`、`net_http_probe.cpp` |
| `services/netpolicymanager` | SA 1152 服务端（策略） | `src/net_policy_service.cpp`、`core/`、`firewall_rule/`、`stub/net_policy_service_stub.cpp` |
| `services/netstatsmanager` | SA 1153 服务端（统计） | `src/net_stats_service.cpp`（2647 行）、`net_stats_cached.cpp`（1393 行）、`stub/` |
| `services/netstatslimitntf` | 限额通知（独立 junk 目录，非 SA） | `src/net_stats_notification.cpp` |
| `services/netmanagernative` | SA 1158 服务端（netsysnative 进程主体） | `src/netsys_native_service.cpp`（1879 行）、`src/netsys_native_service_stub.cpp`（3238 行）、`src/manager/`、`src/netsys/`、`bpf/`、`fwmarkclient/`、`netsys_bpf_map/` |
| `services/netsyscontroller` | base 侧内核通道控制器 | `src/netsys_controller.cpp`（2040 行）、`src/netsys_native_client.cpp`（2376 行，即 NetsysNativeClient） |
| `services/common` | 跨服务公共定义 | `net_ethernet_base_service.h` 等 |
| `bpf/bpf_progs` | 内核态 BPF C 程序 | `464xlat.c`、`net_permission.c`、`netstats.c`、`offload.c` |
| `bpf/{bpf_loader,bpf_reader,bpf_syscall_wrapper}` | BPF 加载/读取/系统调用封装 | — |
| `utils/common_utils` | 权限中枢与公共工具 | `include/netmanager_base_permission.h` |
| `utils/{errorcode_utils,napi_utils,bundle_utils,data_share}` | 错误码映射/NAPI 工具/包信息/data_share 封装 | — |
| `test` | 单测与 fuzzer | `test/<module>/unittest`、`test/fuzztest/`（见 §测试组织） |
| `test/fuzztest` | 各模块 fuzzer（组 target `test/fuzztest:fuzztest`，`--build-target` 实名见 [../verify/build-and-test.md](../verify/build-and-test.md) §2.2） | `NetConnClientFuzzTest`、`NetPolicyClientFuzzTest`、`NetStatsClientFuzzTest`、`NetSysNetFirewallFuzzTest`、`CommonFuzzTest`、`NfQueueFuzzTest` 等 10 个注册 target |

## 4. 双进程 8 SA 总览（简表）

| SA ID | 名称（so） | 进程 | 实现仓 |
|---|---|---|---|
| 1151 | `libnet_conn_manager.z.so`（NetConnService） | netmanager | 本仓 |
| 1152 | `libnet_policy_manager.z.so`（NetPolicyService） | netmanager | 本仓 |
| 1153 | `libnet_stats_manager.z.so`（NetStatsService） | netmanager | 本仓 |
| 1154 | `libnet_tether_manager.z.so` | netmanager | **netmanager_ext** |
| 1155 | `libnet_vpn_manager.z.so` | netmanager | **netmanager_ext** |
| 1156 | `libdns_resolver_manager.z.so` | netmanager | **仅 profile**（无本仓实现） |
| 1157 | `libnet_ethernet_manager.z.so` | netmanager | **netmanager_ext** |
| 1158 | `libnetsys_native_manager.z.so`（NetsysNativeService） | netsysnative | 本仓 |

每 SA 的 profile 文件、实现类、依赖关系与进程拓扑详见 [sa-architecture.md](sa-architecture.md)。

## 5. 关键常量与错误码（`interfaces/innerkits/include/net_manager_constants.h`）

| 段 | 代表值 | 含义 |
|---|---|---|
| JS 通用段 | `201` 权限拒绝、`202` 非系统调用、`401` 参数错误、`801` 能力不支持、`0` 成功 | 抛给 JS 层的标准错误码 |
| 内部段（199 系列） | `19900001` NETMANAGER_ERR_PARAMETER_INVALID、`19900002` NETMANAGER_ERR_SYSTEM_INTERNAL | 内部通用错误 |
| IPC 基础段（2100001~2100301 区间） | `2100001` 参数无效、`2100002` 操作失败、`2100003` 内部错误、`2100101~2100105` 内存操作失败、`2100201~2100209` Parcel/IPC 连接失败、`2100301` 用法错误（另有 `2100400`/`2100401` iface/地址类） | 本仓 IPC 主错误段；防火墙 `FIREWALL_ERR_INVALID_PARAMETER` 等复用 2100001~2100003 |
| ext 段（220 系列） | `2200001~2200208`、`2201001~2201009`（以太网）、`2202002~2202013`（网络共享）、`2203001~2206001`（VPN）、`2204001~2204010`（mDNS） | 供给 netmanager_ext 使用，本仓只定义不落地 |
| 防火墙/流量过滤段 | `29400000~29400007`（FIREWALL_*）、`29410101~29410105`（TRAFFICFILTER_*） | `FEATURE_NET_FIREWALL_ENABLE` 下生效 |
| EAP 段 | `33200001~33200099` | EAP 认证错误 |

新增错误码必须登记在此头文件，登记规则与流程见 [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md) 第 ⑧ 步。

## 6. 权限中枢（`utils/common_utils/include/netmanager_base_permission.h`）

- `Permission` 命名空间集中定义本仓全部权限字符串常量，**新增 IPC 权限只允许从这里取值，禁止散落硬编码**：
  `GET_NETWORK_INFO`、`GET_ETHERNET_LOCAL_MAC`、`INTERNET`、`CONNECTIVITY_INTERNAL`、`GET_NETSTATS_SUMMARY`、`MANAGE_NET_STRATEGY`、`MANAGE_VPN`、`GET_NETWORK_STATS`、`NETSYS_INTERNAL`、`MANAGE_ENTERPRISE_WIFI_CONNECTION`、`SET_PAC_URL`、`SET_NET_EXT_ATTRIBUTE`、`GET_NETWORK_LOCATION`、`GET_NETWORK_APPROXIMATE_LOCATION`、`ACCESS_NET_TRACE_INFO`、`GET_IP_MAC_INFO`、`SEND_MESSAGE`、`CONNECTIVITY_USE_RESTRICTED_NETWORKS`。
- 校验入口 `NetManagerPermission`：`CheckPermission` / `CheckPermissionWithCache` / `IsSystemCaller` / `CheckNetSysInternalPermission` / `CheckUidPermission`（uid 白名单）/ `GetApiVersion`。
- stub 侧权限映射表模式（`g_codeNPS`，services/netpolicymanager/src/stub/net_policy_service_stub.cpp 起）与 uid 检查（services/netconnmanager/src/stub/net_conn_service_stub.cpp 的 `CMD_NM_SET_INTERNET_PERMISSION` 分支）详见 [../expert/ipc-security-constraints.md](../expert/ipc-security-constraints.md) 与 [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md) 第 ④ 步。

## 7. 观测（`hisysevent.yaml`，domain `NETMANAGER`）

| 事件组 | 事件名示例 | 类型 |
|---|---|---|
| HTTP 性能 | `HTTP_PERF`（含 TOTAL_DNS_TIME/TOTAL_TCP_TIME/TOTAL_TLS_TIME 等）、`NET_STACK_HTTP_RESPONSE_ERROR` | STATISTIC / FAULT |
| 连接管理 | `NET_CONN_SUPPLER_FAULT`/`NET_CONN_SUPPLER_STAT`、`NET_CONN_REQUEST_FAULT`/`NET_CONN_REQUEST_STAT`、`NET_CONN_MONITOR_FAULT`/`NET_CONN_MONITOR_STAT` | FAULT / BEHAVIOR |
| 共享/分布网 | `NET_SHARING_SETUP_FAULT`、`NET_SHARING_CANCEL_FAULT`、`NET_START_DIST_NETWORK_FAULT`、`NET_REMOTE_DEVICE_INFO_*` | FAULT / BEHAVIOR |
| mDNS/VPN | `NET_MDNS_REQUEST_*`、`NET_VPN_CONNECT_FAULT`、`VPN_OPERATE_FAULT` | FAULT / BEHAVIOR |
| 防火墙 | `NET_FIREWALL_CONF_*`、`NET_FIREWALL_REQ_*`、`NET_FIREWALL_LOG_REQ_*`、`NET_FIREWALL_INIT_FAULT` | FAULT / BEHAVIOR |

新增维测事件需同步修改 `hisysevent.yaml`（domain 固定 `NETMANAGER`，FAULT/BEHAVIOR/STATISTIC 级别按既有邻近事件对齐）。

## 8. 与 netstack / netmanager_ext / netsysnative 的边界

- **对 netstack**：本仓是连接信息的提供方，`NetConnClient`（`interfaces/innerkits/netconnclient/include/net_conn_client.h:59` GetInstance）是对外统一入口；netstack 侧代理逻辑以 `HAS_NETMANAGER_BASE` 宏分叉，本仓改动连接回调语义时需评估 netstack 降级路径。
- **对 netmanager_ext**：本仓 `sa_profile/` 持有 1154/1155/1157 的 profile 与 220 系列错误码定义；能力本身（网络共享、VPN、以太网、mDNS 扩展）在 ext 仓实现。profile 与 BUILD 联动改动需两仓协同，原则见 [sa-architecture.md](sa-architecture.md) §4。
- **对 netsysnative**：本仓 `services/netsyscontroller` 是 base 侧唯一的内核通道封装（NetsysNativeClient 经 IPC 调 SA 1158）；除它之外的服务端代码**禁止**绕过 controller 直连 netsysnative proxy。
- **对依赖仓**：dhcp（`services/netmanagernative/src/dhcp_controller.cpp`）、libbpf、iptables、common_event_service、data_share/relational_store/sqlite（统计落库）、bundle_framework/os_account（uid 语义）、curl/jerryscript（PAC 求值）、ffrt/eventhandler——改动触碰这些接口时先确认 API 稳定性。

## 9. 测试组织

| 类型 | 位置 |
|---|---|
| 单元测试 | `test/<module>/unittest`：`netcommon`、`netconnmanager`、`netpolicymanager`、`netstatsmanager`、`netmanagernative`、`netmanagerconnection`、`netmanagerutils`、`netsysbpfstats` |
| Fuzzer | `test/fuzztest/`：注册 target 共 10 个（`NetConnClientFuzzTest`、`NetPolicyClientFuzzTest`、`NetStatsClientFuzzTest`、`NetSysNetFirewallFuzzTest`、`NfQueueFuzzTest`（开关门控）、`CommonFuzzTest`、`EnableVnicNetworkFuzzTest`、`IsDeadFlowResetTargetBundleFuzzTest`、`NetBaseBranchFuzzTest`、`UpdateNetCapsFuzzTest`）；另有 `netsysclient_fuzzer/` 目录无 BUILD.gn 不可构建 |
| 其他 | `test/mock`、`test/security`、`test/commonduplicatedcode`、`test/netcommon` |

## 关联知识

- [sa-architecture.md](sa-architecture.md)：8 SA 详解、进程拓扑与 profile 修改约束
- [net-conn-pipeline-map.md](net-conn-pipeline-map.md)：网络注册/DefaultNet/探测全链路
- [policy-stats-bpf-map.md](policy-stats-bpf-map.md)：策略/统计/BPF 数据面三链
- [netsys-native-map.md](netsys-native-map.md)：netsysnative 服务解剖
- [../routing/task-routing.md](../routing/task-routing.md)：按场景路由
- [../routing/vocabulary-routing.md](../routing/vocabulary-routing.md)：按术语路由
- [../expert/architecture-invariants.md](../expert/architecture-invariants.md)：分层与依赖不变量
- [../verify/build-and-test.md](../verify/build-and-test.md)：构建与测试命令

## 适用场景

- 第一次进入本仓，需要建立"双进程 8 SA"全局认知
- 不确定某任务属于哪个 SA、哪一层、先读哪篇文档
- 评估改动影响面（跨进程/跨仓/跨语言绑定）
