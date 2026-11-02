# 策略 / 统计 / BPF 数据面地图

> 本文覆盖三条数据链：① 网络策略与防火墙规则下发链；② 流量统计采集与落库链；③ 流量限额通知链；以及 `bpf/bpf_progs/` 四个内核态 BPF 程序的职责分工与历史红点。修改策略、统计、BPF 相关代码前必读。

## 1. 链①：策略下发 → 防火墙规则 → BPF 数据面

```
JS/NDK 调用（@ohos.net.policy；系统侧调用 NetPolicyClient）
  → frameworks/native/netpolicyclient/src/net_policy_service_proxy.cpp
  → services/netpolicymanager/src/stub/net_policy_service_stub.cpp
      （权限映射表 g_codeNPS：PolicyInterfaceCode → Permission::MANAGE_NET_STRATEGY，
        见 net_policy_service_stub.cpp:30 起；漏登记=越权风险，见 ../routing/ipc-extension-routing.md）
  → services/netpolicymanager/src/net_policy_service.cpp（NetPolicyService）
  → services/netpolicymanager/src/core/net_policy_core.cpp（策略骨架：UidRule/UidPolicy/BackgroundPolicy 数据源与查询）
  → services/netpolicymanager/src/firewall_rule/firewall_rule.cpp（FirewallRule，203 行）
      ├─ device_idle_firewall_rule.cpp（设备空闲白名单）
      ├─ power_save_firewall_rule.cpp（省电白名单）
      └─ idle_deny_firewall_rule.cpp（空闲拒绝名单）
  → NetsysController（services/netsyscontroller/src/netsys_controller.cpp）
  → NetsysNativeClient --IPC--> NetsysNativeService（SA 1158）
  → services/netmanagernative/src/manager/firewall_manager.cpp（防火墙管理）/ manager/bandwidth_manager.cpp（带宽带宽/限速）
  → services/netmanagernative/bpf/src/bpf_netfirewall.cpp（网络防火墙 BPF map 写入）
      └─ include/netfirewall/（netfirewall_ct_map.h 连接跟踪表、netfirewall_ct_def.h、netfirewall_types.h 等结构）
```

- 限速语义由 `LimitAction`（超限后限速/断网/提醒）驱动，触发点在链②的限额通知（§3）。
- `firewall_rule` 子目录是策略侧对规则的组装层，真正的 map 写入在 netsysnative 侧 `bpf_netfirewall.cpp` + `manager/firewall_manager.cpp`——两层不可只改一层。
- 特性开关 `netmanager_base_enable_feature_net_firewall`（`netmanager_base_config.gni`，默认 false）控制防火墙能力面与 `NET_FIREWALL_*` 维测事件；`netmanager_base_enable_netsys_access_policy_diag_listen` 控制访问策略诊断监听。
- 策略相关 IPC 命令字：`interfaces/innerkits/netpolicyclient/include/policy_ipc_interface_code.h`（PolicyInterfaceCode）。

## 2. 链②：流量统计采集 → 落库

```
内核态采集：bpf/bpf_progs/netstats.c（BPF 程序，按 uid/iface 打点流量计数）
  → BPF maps（bpf/ 目录下 bpf_reader / bpf_syscall_wrapper 封装读取通道）
  → 用户态读取：services/netmanagernative/bpf/src/bpf_stats.cpp（BpfStats，bpf map 访问封装）
  → netsysnative 侧暴露统计查询（NetsysInterfaceCode，interfaces/innerkits/netmanagernative/include/netsys_ipc_interface_code.h）
  → base 侧拉取：services/netsyscontroller（NetsysController/NetsysNativeClient）
  → services/netstatsmanager/src/net_stats_service.cpp（NetStatsService，2647 行，红区：统计生命周期与查询入口）
  → services/netstatsmanager/src/net_stats_cached.cpp（NetStatsCached，1393 行：缓存最新周期数据）
  → 落库：net_stats_sqlite_statement.cpp + net_stats_database_helper.cpp（SQLite）
      ├─ net_stats_history.cpp（历史查询）
      ├─ net_stats_calibrate.cpp（校准）
      ├─ net_stats_data_handler.cpp / net_stats_data_clone.cpp / net_stats_info（数据加工/克隆）
      └─ net_stats_traffic_plan_service.cpp / net_stats_settings_observer.cpp（流量计划与设置监听）
```

- 查询入口面向应用：`frameworks/js/napi/netstats` → NetStatsClient（`frameworks/native/netstatsclient/src/proxy/net_stats_service_proxy.cpp`）→ SA 1153。
- 统计链对"数据一致性"要求极高：**历史失效 9ee4ebbf（"流量管理统计丢失修改"，DTS2026072771613）** 即发生在 `net_stats_cached.h/.cpp` 与 `net_stats_service.cpp`（缓存时机/字段缺漏导致统计丢失）。改动缓存与落库时序必须绑定 `test/netstatsmanager/unittest` 与统计一致性回归。
- `netmanager_base_enable_traffic_statistic`（默认 false）控制应用粒度流量统计特性面。

## 3. 链③：限额通知（netstatslimitntf）

```
NetStatsService 检测到配额触顶（QuotaPolicy + LimitAction）
  → services/netstatsmanager/src/net_stats_limit_notification_adapter.cpp（到期改写与提醒适配）
  → services/netstatslimitntf/src/net_stats_notification.cpp（通知实现目录，非独立 SA）
  → 三种典型动作：限额提醒 / 断网（background 禁止）/ 限速（交链① bandwidth_manager 执行）
  → 系统通知/弹窗：net_stats_trafficLimit_dialog.cpp（超限对话框）配合 systemui/common_event_service
```

- 配额数据源：`QuotaPolicy`（按 ident/network 匹配的月配额与提醒阈值，接口见 netquotaclient inner API），限额策略经链①下发为内核侧限速/禁网规则。
- `netmanager_base_share_traffic_limit_enable`（默认 false）控制共享流量限额特性。

## 4. `bpf/bpf_progs/` 四个 BPF 程序

| 程序 | 职责 |
|---|---|
| `464xlat.c` | CLAT/NAT464：IPv4↔IPv6 地址转换数据面（配合用户态 `services/netmanagernative/src/netsys/clatd.cpp`、`clatd_packet_converter.cpp`、`clat_utils.cpp` 与 `manager/clat_manager.cpp`） |
| `net_permission.c` | 网络/uid 权限校验数据面：按 uid 网络访问许可过滤 |
| `netstats.c` | 流量统计数据面：按 uid/iface 打点计数，供链②读取 |
| `offload.c` | 硬件/软件 offload 辅助路径 |

- 加载与访问：`bpf/bpf_loader`（加载器）、`bpf/bpf_reader` + `bpf/bpf_syscall_wrapper`（读取通道）、`services/netmanagernative/bpf/include/bpf_mapper.h`（map 句柄封装基类）。
- `netsys_bpf_map`（`services/netmanagernative/netsys_bpf_map`）集中定义各业务 BPF map 结构。

## 5. 历史红点（改动前必查）

| 提交 | 位置 | 教训 |
|---|---|---|
| **c3d1d423**（"修改越界读"，DTS2025102143799） | `services/netmanagernative/bpf/src/bpf_netfirewall.cpp` + `bpf_ring_buffer.cpp` | BPF ring buffer 读取越界：循环边界/长度计算改动必须复核 buffer 元数据边界，越界读会污染相邻内存甚至崩溃 |
| **9ee4ebbf**（"流量管理统计丢失修改"，DTS2026072771613） | `services/netstatsmanager/include/net_stats_cached.h`、`src/net_stats_cached.cpp`、`src/net_stats_service.cpp` | 统计链的缓存/落库字段与时序缺漏直接造成用户可见的"流量统计丢失"；任何 cached/schema 改动须验证数据完整性 |
| 2103f660（探测线程内存序，见 [net-conn-pipeline-map.md](net-conn-pipeline-map.md) §3） | probe_thread/net_http_probe | 跨线程共享变量需内存序/原子语义 |

详细的失效模式与 revert 教训见 [../expert/historical-failures.md](../expert/historical-failures.md)；ring buffer/统计并发类问题的排查步骤见 [../expert/concurrency-failure-modes.md](../expert/concurrency-failure-modes.md)。

## 6. 红区文件

| 文件 | 行数 | 风险 |
|---|---|---|
| `services/netstatsmanager/src/net_stats_service.cpp` | 2647 | 统计查询/写入核心，时序敏感 |
| `services/netstatsmanager/src/net_stats_cached.cpp` | 1393 | 缓存层，历史统计丢失点（9ee4ebbf） |
| `services/netmanagernative/bpf/src/bpf_netfirewall.cpp` | — | 历史越界读点（c3d1d423） |
| `services/netmanagernative/bpf/src/bpf_ring_buffer.cpp` | — | 同上，ring buffer 边界 |

## 关联知识

- [netmanager-base-code-map.md](netmanager-base-code-map.md)：全局分层与错误码/权限基线
- [netsys-native-map.md](netsys-native-map.md)：firewall_manager/bandwidth_manager 与 BPF 访问封装
- [net-conn-pipeline-map.md](net-conn-pipeline-map.md)：ident/netId 的上游来源（supplier 注册链）
- [../expert/historical-failures.md](../expert/historical-failures.md)：本节红点的完整复盘
- [../routing/vocabulary-routing.md](../routing/vocabulary-routing.md)：UidRule/QuotaPolicy/FirewallRule/BpfStats/CLAT 术语
- [../verify/build-and-test.md](../verify/build-and-test.md)：策略/统计/BPF 相关测试命令

## 适用场景

- 修改策略规则（Uid/Background/Idle/PowerSave）、限额与限速行为前定位两层（policy 侧 + netsys 侧）改动点
- 修改流量统计查询、缓存、SQLite 落库与历史数据结构
- 排查"统计丢失/统计跳变/防火墙规则不生效/超限未提醒"类问题
