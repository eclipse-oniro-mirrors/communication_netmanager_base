# 词汇触发路由

> 任务描述、需求文档或 MR 评审中出现下表术语时，先读"指向"的文档对应小节再动手。术语是本仓的行话，每个词背后都绑定一组文件与约束；文末附已核实的枚举真实值速查表。

使用方法：在任务描述中搜到表中术语 → 按"含义"判断是否相关 → 跳转"指向"列。若术语未命中，先查 [task-routing.md](task-routing.md) 的场景表，再看 [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md) 的目录职责表。

## 1. 词条表

| 术语 | 含义 | 指向 |
|---|---|---|
| `NetSupplier` | 网络"供应商"对象：硬件/系统网络能力的注册载体（score/ident/bearerType/netId 挂载点，`services/netconnmanager/src/net_supplier.cpp`，722 行） | [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §1 |
| `NetActivate` | 网络激活请求：按 NetSpecifier 申请满足条件的网络（`services/netconnmanager/include/net_activate.h`） | [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §4 |
| `NetMonitor`/`NetDetection` | 网络探测状态机与探测请求（`services/netconnmanager/src/net_monitor.cpp`，615 行；探测命令字 `CMD_NM_NET_DETECTION`/`CMD_NM_NET_DETECTION_RESPONSE`） | [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §3 |
| `NetLinkInfo` | 网络链路信息（地址/路由/DNS/iface），注册链中从 supplier 刷入并落地内核（`interfaces/innerkits/netconnclient/include/net_link_info.h`） | [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §1 |
| `NetSpecifier`/`NetHandle` | 应用按能力/承载过滤回调的申请条件与"网络句柄"（`net_specifier.h` / `net_handle.h`；`RegisterNetConnCallback` 带 specifier 重载见 net_conn_client.h:177） | [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §4 |
| ident | supplier 的身份标识字符串（如 iface 名），策略/防火墙按 ident 匹配规则（字段 `NetSupplierInfo::ident_`） | [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §1-§2 |
| supplier score | 供应商分数：DefaultNet 评选的加权输入（`NetSupplierInfo::score_`；bestScore 遍历比较见 net_conn_service.cpp:1587-1612） | [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §2 |
| `NetConnState` | 网络连接状态机（UNKNOWN~DISCONNECTED 六态，`net_supplier_info.h:25`） | 本文 §2 速查表 + [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §5 |
| `NetCap`/`NetCapacity` | 网络能力位集合（MMS/INTERNET/VALIDATED/PORTAL 等，`net_all_capabilities.h:26`，上限 `NET_CAPABILITY_END=32`） | 本文 §2 速查表 |
| `NetBearType` | 网络承载类型（蜂窝/WiFi/蓝牙/以太网/VPN/WIFI_AWARE，`net_all_capabilities.h:57`） | 本文 §2 速查表 |
| `UidRule`/`UidPolicy`/`BackgroundPolicy` | 按 uid 的网络访问规则、策略与后台总开关（枚举 `NetUidRule`/`NetUidPolicy`/`NetBackgroundPolicy` 定义在 `interfaces/innerkits/netpolicyclient/include/net_policy_constants.h:37/:46/:61`；服务端语义 `services/netpolicymanager/src/core/`） | [../code_map/policy-stats-bpf-map.md](../code_map/policy-stats-bpf-map.md) 链① |
| `QuotaPolicy`/`LimitAction` | 流量配额策略与超限动作（提醒/断网/限速；`net_policy_constants.h:85` LimitAction） | [../code_map/policy-stats-bpf-map.md](../code_map/policy-stats-bpf-map.md) 链③ |
| `FirewallRule` | 防火墙规则组装层（`services/netpolicymanager/include/firewall_rule.h:30` + `src/firewall_rule/` 三套名单规则） | [../code_map/policy-stats-bpf-map.md](../code_map/policy-stats-bpf-map.md) 链① |
| `NetsysController` | base 侧内核通道唯一门面（`services/netsyscontroller/src/netsys_controller.cpp`，2040 行；mock 可注入） | [../code_map/netsys-native-map.md](../code_map/netsys-native-map.md) §2 |
| `NetsysNativeService` | SA 1158 服务端，内核网络能力唯一落地点（`services/netmanagernative/src/netsys_native_service.cpp`） | [../code_map/netsys-native-map.md](../code_map/netsys-native-map.md) 全篇 |
| `NetsysNativeClient` | base 侧调用 SA 1158 的 IPC 客户端（`services/netsyscontroller/src/netsys_native_client.cpp`，2376 行红区） | [../code_map/netsys-native-map.md](../code_map/netsys-native-map.md) §2 |
| Fwmark/FwmarkClient | 内核 socket 的 netId 标记机制与其客户端库（`services/netmanagernative/src/netsys/fwmark_network.cpp` + `services/netmanagernative/fwmarkclient/`，配置路径见 netmanager_base_config.gni `FWMARK_CLIENT_DIR`） | [../code_map/netsys-native-map.md](../code_map/netsys-native-map.md) §4 |
| `BpfStats` | BPF 流量统计 map 的用户态读取封装（`services/netmanagernative/bpf/src/bpf_stats.cpp`） | [../code_map/policy-stats-bpf-map.md](../code_map/policy-stats-bpf-map.md) 链② |
| CLAT/NAT464 | IPv4↔IPv6 翻译（数据面 `bpf/bpf_progs/464xlat.c`，用户态 `netsys/clatd.cpp` + `manager/clat_manager.cpp`，另有 netconnmanager 的 `nat464_service.cpp`） | [../code_map/policy-stats-bpf-map.md](../code_map/policy-stats-bpf-map.md) §4 |
| `NetManagerCenter` | 策略侧规则/数据中心（`services/netpolicymanager/include/net_policy_base.h:69` `GetNetCenterInst()`，NetPolicyService 聚合取用） | [../code_map/policy-stats-bpf-map.md](../code_map/policy-stats-bpf-map.md) 链① |
| `HttpProxy`/PAC | HTTP 代理结构与 PAC 脚本代理（`interfaces/innerkits/netconnclient/include/http_proxy.h:30`；PAC 管理 `services/netconnmanager/src/net_pac_manager.cpp`、本地代理解析 `net_pac_local_proxy_server.cpp`；特性开关 `netmanager_base_enable_pac_proxy`；PAC 求值依赖 curl/jerryscript） | [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §1 要点 |
| AirplaneMode | 飞行模式开关（`NetConnService::SetAirplaneMode`，net_conn_service.h:306；stub 侧命令分支 `services/netconnmanager/src/stub/net_conn_service_stub.cpp`） | [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §1 |
| `INetConnCallback` | 连接回调接口：netAvailable/netCapabilitiesChanged/netBlockedStatusChange 等（客户端 stub `frameworks/native/netconnclient/src/proxy/net_conn_callback_stub.cpp` 与服务端 wrapper 成对） | [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §4 |
| `NetConnClient` | 应用/NDK 侧统一入口（`interfaces/innerkits/netconnclient/include/net_conn_client.h:59` GetInstance）；netstack 也经它取网络信息 | [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §4 |
| `netstatslimitntf` | 限额通知实现目录（`services/netstatslimitntf/src/net_stats_notification.cpp`，非独立 SA） | [../code_map/policy-stats-bpf-map.md](../code_map/policy-stats-bpf-map.md) 链③ |
| `g_codeNPS` | policy stub 的"命令字 → 权限"映射表模式（`services/netpolicymanager/src/stub/net_policy_service_stub.cpp:30` 起） | [../expert/ipc-security-constraints.md](../expert/ipc-security-constraints.md) + [ipc-extension-routing.md](ipc-extension-routing.md) 第 ④ 步 |
| `NetsysInterfaceCode`/`ConnInterfaceCode`/`PolicyInterfaceCode`/`StatsInterfaceCode` | 四套 IPC 命令字枚举（`interfaces/innerkits/*/include/`，新增命令在此登记） | [ipc-extension-routing.md](ipc-extension-routing.md) 第 ① 步 |
| 权限常量族 | 全部 IPC 权限字符串只从 `utils/common_utils/include/netmanager_base_permission.h` 的 `Permission` 命名空间取 | [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md) §6 |
| `NETMANAGER` domain | hiSysEvent 维测事件域（`hisysevent.yaml`：NET_CONN_*/NET_SHARING_*/NET_FIREWALL_* 等） | [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md) §7 |
| 2103f660 | 探测线程 curl 崩溃（内存序）历史提交，红点参照 | [../expert/historical-failures.md](../expert/historical-failures.md) |
| c3d1d423 / 9ee4ebbf | BPF 越界读修复 / 统计丢失修复历史提交 | [../code_map/policy-stats-bpf-map.md](../code_map/policy-stats-bpf-map.md) §5 |

## 2. 枚举真实值速查表（已对照源码核实）

### NetCap（`interfaces/innerkits/netconnclient/include/net_all_capabilities.h:26`）

| 常量 | 值 |
|---|---|
| `NET_CAPABILITY_MMS` | 0 |
| `NET_CAPABILITY_SUPL` | 1 |
| `NET_CAPABILITY_DUN` | 2 |
| `NET_CAPABILITY_IA` | 3 |
| `NET_CAPABILITY_XCAP` | 4 |
| `NET_CAPABILITY_BIP` | 5 |
| `NET_CAPABILITY_NOT_METERED` | 11 |
| `NET_CAPABILITY_INTERNET` | 12 |
| `NET_CAPABILITY_NOT_VPN` | 15 |
| `NET_CAPABILITY_VALIDATED` | 16 |
| `NET_CAPABILITY_PORTAL` | 17 |
| `NET_CAPABILITY_END` | 32（上限，新增能力不得越界） |

（另：`INTERNAL_DEFAULT=18`、`SNSSAI1~6=19~24`、`INTERNAL_CHANNEL=25`、`OEM_PAID=26`、`OEM_PRIVATE=27`、`CHECKING_CONNECTIVITY=31`。）

### NetBearType（`net_all_capabilities.h:57`）

| 常量 | 值 |
|---|---|
| `BEARER_CELLULAR` | 0 |
| `BEARER_WIFI` | 1 |
| `BEARER_BLUETOOTH` | 2 |
| `BEARER_ETHERNET` | 3 |
| `BEARER_VPN` | 4 |
| `BEARER_WIFI_AWARE` | 5 |
| `BEARER_DEFAULT` | 上限哨兵值，无实际含义 |

### NetConnState（`interfaces/innerkits/netconnclient/include/net_supplier_info.h:25`）

| 常量 | 值 |
|---|---|
| `NET_CONN_STATE_UNKNOWN` | 0 |
| `NET_CONN_STATE_IDLE` | 1 |
| `NET_CONN_STATE_CONNECTING` | 2 |
| `NET_CONN_STATE_CONNECTED` | 3 |
| `NET_CONN_STATE_DISCONNECTING` | 4 |
| `NET_CONN_STATE_DISCONNECTED` | 5 |

### 其他枚举（值分布见对应头文件，未经核实不列数值）

| 枚举 | 定义位置 | 核实命令 |
|---|---|---|
| `NetUidPolicy` / `NetUidRule` / `NetBackgroundPolicy` / `LimitAction` / `RemindType` / `PolicyTransCondition` / `NetPolicyResultCode` | `interfaces/innerkits/netpolicyclient/include/net_policy_constants.h`（枚举起点约 :30/:37/:46/:61/:72/:85/:94） | `grep -n "enum" interfaces/innerkits/netpolicyclient/include/net_policy_constants.h` |
| `ConnInterfaceCode` 命令字全集 | `interfaces/innerkits/netconnclient/include/proxy/conn_ipc_interface_code.h` | `grep -n "CMD_NM_" interfaces/innerkits/netconnclient/include/proxy/conn_ipc_interface_code.h` |
| `PolicyInterfaceCode` 命令字全集 | `interfaces/innerkits/netpolicyclient/include/policy_ipc_interface_code.h` | `grep -n "CMD_NPS_" interfaces/innerkits/netpolicyclient/include/policy_ipc_interface_code.h` |
| `StatsInterfaceCode` 命令字全集 | `interfaces/innerkits/netstatsclient/include/proxy/stats_ipc_interface_code.h` | `grep -n "CMD_" interfaces/innerkits/netstatsclient/include/proxy/stats_ipc_interface_code.h` |
| `NetsysInterfaceCode` 命令字全集 | `interfaces/innerkits/netmanagernative/include/netsys_ipc_interface_code.h` | `grep -n "CMD_" interfaces/innerkits/netmanagernative/include/netsys_ipc_interface_code.h` |
| 错误码全集（199/210/220/294/332 系列） | `interfaces/innerkits/include/net_manager_constants.h` | `grep -n "= 19\|= 21\|= 22\|= 29\|= 33" interfaces/innerkits/include/net_manager_constants.h` |

## 术语维护规则

- 新增模块/能力引入新行话时：更新本表与仓根 [AGENTS.md](../../../AGENTS.md) 的词汇路由章节，两表保持同源。
- 术语"指向"只允许指向真实存在的文档/文件；文档只增不删，过期术语标注沿用原文保留。
- 速查表只收**已对照源码核实**的数值；新增行必须先 `grep`/`read` 头文件核实再写入，禁止凭记忆填值。
- 与崩溃强相关的术语（探测内存序、ring buffer 越界、统计丢失）同时路由到 [../expert/historical-failures.md](../expert/historical-failures.md)。

## 关联知识

- [task-routing.md](task-routing.md)：按任务场景路由
- [ipc-extension-routing.md](ipc-extension-routing.md)：IPC 命令扩展专路
- [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md)：术语对应文件的全局位置

## 适用场景

- 阅读 PR/MR 描述、评审意见或需求文档时遇到行话
- 写 commit message / 知识文档前校准术语口径
- 排查能力位/状态/错误码数值语义时查速查表
