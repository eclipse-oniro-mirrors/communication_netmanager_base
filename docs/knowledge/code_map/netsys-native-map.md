# netsysnative 服务解剖（SA 1158）

> 本文覆盖 netsysnative 进程的全部能力面：`NetsysNativeService` 命令组划分、proxy/stub/client 三件套对称结构、BPF map 访问封装、netlink 封装、以及与其他 SA 的调用关系。任何要"碰内核网络"的任务（路由/带宽/防火墙/DNS/共享/接口）先读本文。

## 1. 服务定位与命令组划分

`NetsysNativeService`（`services/netmanagernative/src/netsys_native_service.cpp`，1879 行）是内核网络通道的唯一服务端（SA 1158，netsysnative 进程，profile `sa_profile/1158.json`）。命令字定义在 `interfaces/innerkits/netmanagernative/include/netsys_ipc_interface_code.h`（`NetsysInterfaceCode`），按业务域分派到 `src/manager/` 的模块化子系统：

| 子系统 | 实现文件（`services/netmanagernative/src/manager/`） | 职责 |
|---|---|---|
| 连接管理 | `conn_manager.cpp` | 网络创建/销毁、接口纳入网络 |
| 路由 | `route_manager.cpp` | 路由表增删改、默认路由写入（配合 `netroute` 语义） |
| 带宽带宽 | `bandwidth_manager.cpp` | 限速/配额管控的 iptables 与规则下发 |
| 防火墙 | `firewall_manager.cpp` | 防火墙规则管理（数据面在 `bpf/src/bpf_netfirewall.cpp`） |
| DNS | `dns_manager.cpp` | DNS 解析管理（配合 `src/netsys/dnsresolv/`：`dns_param_cache.cpp`、`dns_proxy_listen.cpp`、`dns_proxy_request_socket.cpp`、`dns_quality_diag.cpp`、`dns_resolv_config.cpp` 等） |
| 网络共享 | `sharing_manager.cpp` | tether 场景的地址/路由/iptables 配置（DNS 服务配合 `src/dhcp_controller.cpp` 与 dhcp 仓） |
| 接口管理 | `interface_manager.cpp` | iface up/down/地址/MAC 配置 |
| CLAT | `clat_manager.cpp` | NAT464 控制面（数据面 `bpf_progs/464xlat.c` 与 `src/netsys/clatd.cpp`） |
| 流量 | `traffic_manager.cpp` | 流量回调/统计分发（配合 `bpf/src/bpf_stats.cpp`） |
| VPN | `vpn_manager.cpp`、`multi_vpn_manager.cpp`、`vnic_manager.cpp` | 系统/多 VPN 与虚拟网卡（`src/netsys/system_vpn_wrapper.cpp` 支撑） |
| 分布网 | `distributed_manager.cpp`、`wearable_distributed_net_manager.cpp` | 分布式/穿戴网络扩展 |
| MPTCP | `mptcp_manager.cpp` | 多路径 TCP 配置 |

声明/诊断回调：`net_diag_callback_proxy.cpp`（网络诊断回调上行）、`notify_callback_proxy.cpp`、`netsys_traffic_callback_proxy.cpp`（流量事件）、`netfirewall_callback_stub.cpp`（防火墙事件回传给调用方）。

## 2. proxy / stub / client 三件套（全红区，读写必须对称）

```
services/netsyscontroller/src/netsys_controller.cpp（NetsysController，2040 行：base 侧门面）
  → services/netsyscontroller/src/netsys_native_client.cpp（NetsysNativeClient，2376 行：IPC 客户端+回调句柄管理）
       │ 写方向：Parcel 组包（写出顺序必须与 stub 读入顺序严格对称）
       ▼
  IPC → services/netmanagernative/src/netsys_native_service.cpp（分发）
       → services/netmanagernative/src/netsys_native_service_stub.cpp（3238 行：OnRemoteRequest 分支与权限/uid 检查）
       → manager/netsys 子系统落地
       │ 读方向：回执 Parcel（对端 proxy 按同样顺序解包）
       ▼
  frameworks/native/netmanagernative/netsys_native_service_proxy.cpp（4548 行：每命令一个方法的 proxy 实现，红区）
```

| 角色 | 文件 | 行数 | 说明 |
|---|---|---|---|
| proxy | `frameworks/native/netmanagernative/netsys_native_service_proxy.cpp` | 4548 | 注意：netsys 的 proxy 位于模块根目录（不同于 netconnclient 的 `src/proxy/` 习惯） |
| stub | `services/netmanagernative/src/netsys_native_service_stub.cpp` | 3238 | 命令分发/权限核对 |
| 服务实现 | `services/netmanagernative/src/netsys_native_service.cpp` | 1879 | 业务分派到 manager |
| client | `services/netsyscontroller/src/netsys_native_client.cpp` | 2376 | base 侧唯一合法调用入口（含 mock：`mock_netsys_native_client.cpp`） |

- 三个大文件均为红区：proxy 改一个字段顺序、stub 忘改对应分支，都会造成 IPC 解包错乱/静默失败。新增命令的完整 checklist 见 [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md)。
- `frameworks/native/netsysclient/netsys_traffic_callback_proxy.cpp` 与 `frameworks/native/netmanagernative/netfirewall_callback_proxy.cpp` 是事件回传的配套 proxy（回调成对规则同上）。

## 3. BPF map 访问封装

- `services/netmanagernative/bpf/include/bpf_mapper.h`：map 句柄封装基类。
- `services/netmanagernative/bpf/src/bpf_stats.cpp`：流量统计 map 读取（喂给 SA 1153，链路见 [policy-stats-bpf-map.md](policy-stats-bpf-map.md) 链②）。
- `services/netmanagernative/bpf/src/bpf_netfirewall.cpp` + `bpf_ring_buffer.cpp`：防火墙规则 map 写入与事件 ring buffer（历史越界读修复 c3d1d423 在此）。
- `services/netmanagernative/netsys_bpf_map/`：各业务 BPF map 结构定义集中地。
- 内核态程序与加载：`bpf/bpf_progs/`（464xlat.c/net_permission.c/netstats.c/offload.c）+ `bpf/bpf_loader`、`bpf/bpf_reader`、`bpf/bpf_syscall_wrapper`。
- 写 BPF 相关代码时注意 libbpf API 兼容与 map 冻结/热更语义；`netmanager_base_support_ebpf_memory_miniaturization` 开关影响 BPF 内存占用形态。

## 4. netlink 封装（`services/netmanagernative/src/netsys/`）

| 文件 | 封装对象 |
|---|---|
| `netlink_msg.cpp` / `netlink_socket.cpp` | netlink 消息构造与 socket 收发（路由/接口事件通道） |
| `netlink_socket_diag.cpp` | socket 诊断（inet diag） |
| `net_diag_wrapper.cpp` | 网络诊断（ping/traceroute 类） |
| `iptables_wrapper.cpp` | iptables 命令封装（带宽/防火墙规则的传统通道） |
| `fwmark_network.cpp` + `fwmarkclient/`（`services/netmanagernative/fwmarkclient/`） | Fwmark（netId 标记）网络管理与 fwmark client 库 |
| `physical_network.cpp` / `virtual_network.cpp` / `local_network.cpp` / `netsys_network.cpp` | 三类网络的规则面实现 |
| `net_manager_native.cpp` | netsys 内部 native 管理聚合 |
| `netsys_client.c` / `netsys_tcp_client.c` / `netsys_udp_transfer.cpp` | netsys 自有 socket 通道（C 实现，注意生命周期） |
| `clatd.cpp` / `clatd_packet_converter.cpp` / `clat_utils.cpp` | CLAT 收发包转换 |
| `dnsresolv/` | DNS 代理解析栈（见 §1 DNS 行） |
| `system_vpn_wrapper.cpp` | 系统 VPN 配套封装 |

## 5. 与其他 SA 的调用关系

```
SA 1151 NetConnService ──NetLinkInfo/能力更新──┐
SA 1152 NetPolicyService ──规则下发───────────┤
SA 1153 NetStatsService ──统计拉取───────────┼──> NetsysController(NetsysNativeClient) ──IPC──> SA 1158
ext 侧服务（1154 共享/1155 VPN/1157 以太网）──┘                                          │
                                                                                         ▼
                                             manager/* + netsys/* → netlink / iptables / BPF
                                             事件回传：notify/traffic/netfirewall/net_diag callback
```

- base 内任何服务触达 1158 都必须经 `services/netsyscontroller`（NetsysController 是唯一门面，mock 可替换：`mock_netsys_native_client.cpp`），在测试中据此可注入 mock。
- ext 侧服务同样经 netsyscontroller 通道使用 1158 的共享/VPN/以太网底层能力；1158 的回调 proxy/stub 也是跨 SA 成对出现（漏一侧=事件黑洞）。
- DNS 链路：SA 1156 仅 profile 占位，实际解析由 1158 的 dns_manager/dnsresolv 承接，经 `NetsysController` 提供 `netsys_dns_report_callback.h` 上报结果。

## 6. 红区与改动守则

| 文件 | 行数 | 守则 |
|---|---|---|
| `frameworks/native/netmanagernative/netsys_native_service_proxy.cpp` | 4548 | 任何 Parcel 字段增删必须与 stub 同步 |
| `services/netmanagernative/src/netsys_native_service_stub.cpp` | 3238 | 新命令必须做权限/uid 核对并补 fuzzer case（`netsysclient_fuzzer`） |
| `services/netsyscontroller/src/netsys_native_client.cpp` | 2376 | 回执与回调生命周期敏感（base 侧咽喉） |
| `services/netmanagernative/bpf/src/{bpf_netfirewall,bpf_ring_buffer}.cpp` | — | 历史越界读（c3d1d423），改边界必须复核 |

## 关联知识

- [netmanager-base-code-map.md](netmanager-base-code-map.md)：分层与目录基线
- [sa-architecture.md](sa-architecture.md)：SA 1158 与进程拓扑
- [net-conn-pipeline-map.md](net-conn-pipeline-map.md)：上游注册链与 NetLinkInfo
- [policy-stats-bpf-map.md](policy-stats-bpf-map.md)：防火墙/统计/BPF 三链详解
- [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md)：新增 netsys 命令 checklist
- [../expert/concurrency-failure-modes.md](../expert/concurrency-failure-modes.md)：netsys 线程与回调并发
- [../expert/architecture-invariants.md](../expert/architecture-invariants.md)：netsyscontroller 唯一通道不变量

## 适用场景

- 新增/修改内核网络命令（路由、带宽、防火墙、DNS、共享、接口等）前定位两端改动点
- 排查"netmanager 下发的规则在内核不生效/IPC 返回错数据/事件回调丢失"类问题
- 评估 proxy/stub/client/回调四件套的同步改动范围
