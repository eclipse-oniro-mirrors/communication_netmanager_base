# SA 架构专篇：双进程 8 SA 总览与 profile 约束

> 本文覆盖 netmanager_base 全部 8 个 SA（1151~1158）的架构场景：每个 SA 的 ID/profile/实现类/进程/职责/实现仓、进程拓扑、启动与依赖关系、base↔ext 分工原则、以及 sa_profile 修改注意事项。新增或修改 SA 行为、调整 profile 前必读。

## 1. 8 SA 详细表

| SA ID | so（profile libpath） | profile 文件 | 实现类 | 进程 | 职责 | 实现仓 |
|---|---|---|---|---|---|---|
| 1151 | `libnet_conn_manager.z.so` | `sa_profile/1151.json` | `NetConnService`（`services/netconnmanager/src/net_conn_service.cpp`，4787 行） | netmanager | 网络注册（supplier）、Network 维护、DefaultNet 评选、网络探测、HTTP 代理/PAC、上报 `NET_CONN_*` 事件 | 本仓 |
| 1152 | `libnet_policy_manager.z.so` | `sa_profile/1152.json` | `NetPolicyService`（`services/netpolicymanager/src/net_policy_service.cpp`） | netmanager | Uid 规则/Uid 策略/后台策略/限额策略、防火墙规则下发；profile 带 `extension:["backup","restore"]` | 本仓 |
| 1153 | `libnet_stats_manager.z.so` | `sa_profile/1153.json` | `NetStatsService`（`services/netstatsmanager/src/net_stats_service.cpp`，2647 行） | netmanager | 流量统计生命周期、SQLite 落库、历史查询、限额触发通知；profile 带 `extension:["backup","restore"]` | 本仓 |
| 1154 | `libnet_tether_manager.z.so` | `sa_profile/1154.json` | 实现在 netmanager_ext | netmanager | 网络共享（WiFi/USB/蓝牙 tether，events `NET_SHARING_*`）；本仓仅持有 profile | **netmanager_ext** |
| 1155 | `libnet_vpn_manager.z.so` | `sa_profile/1155.json` | 实现在 netmanager_ext | netmanager | 系统 VPN/用户 VPN 管理（events `NET_VPN_CONNECT_FAULT`/`VPN_OPERATE_FAULT`）；本仓仅持有 profile；**`run-on-create: false`**（唯一按需启动） | **netmanager_ext** |
| 1156 | `libdns_resolver_manager.z.so` | `sa_profile/1156.json` | 无 | netmanager | DNS resolver 管理位；**本仓仅保留 profile 占位，无实现代码**。DNS 实际解析在 SA 1158 的 `dns_manager`/`dnsresolv` 与 `NetsysController` 通道执行 | **仅 profile** |
| 1157 | `libnet_ethernet_manager.z.so` | `sa_profile/1157.json` | 实现在 netmanager_ext | netmanager | 以太网设备/配置管理；本仓公共头 `services/common/include/net_ethernet_base_service.h` 定义基类接口 | **netmanager_ext** |
| 1158 | `libnetsys_native_manager.z.so` | `sa_profile/1158.json` | `NetsysNativeService`（`services/netmanagernative/src/netsys_native_service.cpp`，1879 行） | **netsysnative** | 内核网络通道唯一落地点：路由/带宽带宽管控/防火墙/DNS/接口/共享/netlink/BPF 访问。详解见 [netsys-native-map.md](netsys-native-map.md) | 本仓 |

补充：`services/netstatslimitntf`（`src/net_stats_notification.cpp`）是限额**通知**实现目录，不注册为独立 SA，由 NetStatsService 侧经适配器（`services/netstatsmanager/src/net_stats_limit_notification_adapter.cpp`）驱动。

## 2. 进程拓扑图

```
应用进程                                netmanager 进程                       netsysnative 进程
┌──────────────────────────┐   IPC   ┌─────────────────────────────┐   IPC  ┌───────────────────────────┐
│ JS: @ohos.net.connection │ ──────> │ SA 1151 NetConnService      │        │                           │
│ JS: @ohos.net.policy     │         │ SA 1152 NetPolicyService    │        │  SA 1158                  │
│ JS: @ohos.net.statistics │         │ SA 1153 NetStatsService     │        │  NetsysNativeService      │
│ NDK: interfaces/kits/c   │         │                             │        │   ├─ manager/（路由/带宽/  │
│ ANI: frameworks/ets/ani  │         │ SA 1154 tether   [ext 实现] │        │   │   防火墙/DNS/接口/共享) │
│ cj : frameworks/cj       │         │ SA 1155 vpn      [ext 实现] │        │   ├─ netsys/（netlink/    │
└──────────────────────────┘         │ SA 1156 dns_resolver[仅占位]│        │   │   iptables/dnsresolv)   │
        │                            │ SA 1157 ethernet[ext 实现] │        │   ├─ bpf/（bpf_stats/     │
        │ 经                          │                             │        │   │   bpf_netfirewall)     │
        │ frameworks/native/*client  │ services/netsyscontroller   │ ─────> │   └─ fwmarkclient/        │
        │ （NetConnClient/           │   NetsysController →        │        │        │                  │
        │  NetPolicyClient/          │   NetsysNativeClient        │        │        ▼                  │
        │  NetStatsClient）          │   （2376 行）               │        │  bpf/BPF maps + netlink    │
└──────────────────────────┘         └─────────────────────────────┘        └───────────────────────────┘
                                              │
                                              ▼
                                    （ext 侧另有独立进程承载扩展能力实现，
                                      对应 1154/1155/1157 的 so 由 netmanager_ext 构建）
```

要点：

- **跨进程边界只有一条正道**：base 侧任何服务要触达内核网络能力，必须经 `services/netsyscontroller` 的 `NetsysNativeClient`（`services/netsyscontroller/src/netsys_native_client.cpp`）IPC 到 SA 1158，禁止绕行。
- 1151/1152/1153 之间同进程互调（如 NetConnService 触发策略查询），仍走 client proxy 形态保持解耦。

## 3. SA 启动与依赖关系

| SA | run-on-create | 启动/依赖要点 |
|---|---|---|
| 1151 | true | netmanager 进程拉起即注册；依赖 1158 就绪后才能完成 supplier 的 NetLinkInfo 落地（经 NetsysController）；`CMD_NM_SYSTEM_READY` 由系统就绪流程触发 |
| 1152 | true | 依赖 1151 的网络事件（`net_policy_listener.cpp`）与 1158 的防火墙/带宽通道；backup/restore 扩展用于策略库备份 |
| 1153 | true | 依赖 1158 的 BPF 统计数据源（`bpf_stats.cpp`）经 netsyscontroller 拉取；SQLite 本地落库 |
| 1154/1155 | true / **false** | 1155 是唯一 `run-on-create: false`（按需启动）；实现在 ext 仓，启动时序以 ext 侧声明为准 |
| 1156 | true | 仅 profile 占位，无本仓实现，无启动逻辑 |
| 1157 | true | 实现在 ext 仓；本仓 `services/common` 提供基类头 |
| 1158 | true | netsysnative 进程独立拉起；加载 `bpf/bpf_progs` BPF 程序并维护 BPF maps；是 1151/1152/1153 的下游依赖 |

依赖链条（管理面 → 内核面）：

```
JS/NDK → client proxy → 1151/1152/1153（netmanager 进程）
             └── services/netsyscontroller/NetsysController
                     └── NetsysNativeClient --IPC--> 1158（netsysnative 进程）
                                 └── manager/netsys → netlink / iptables / BPF maps / bpf_progs
```

## 4. base↔ext 分工原则

- **profile 所有权**：1151~1158 的 profile 全部登记在本仓 `sa_profile/`（含 `BUILD.gn`）；ext 仓只实现 1154/1155/1157 的 so。
- **改 ext 能力的动作顺序**：行为/接口改动在 netmanager_ext 仓落代码；若涉及 SA 注册属性（进程名、libpath、run-on-create、extension）、依赖或错误码（220 系列）变化，必须回本仓同步改 `sa_profile/*.json` 与 `interfaces/innerkits/include/net_manager_constants.h`。
- **反向约束**：本仓不得为 1154/1155/1156/1157 添加服务端实现代码；公共接口放 `services/common/`（如 `net_ethernet_base_service.h`），ext 仓继承实现。
- **事件归属**：`hisysevent.yaml` 中 `NET_SHARING_*`/`NET_VPN_*`/`NET_MDNS_*` 等事件由 ext 侧发出，但声明维护在本仓——事件字段变更需两仓对齐。

## 5. sa_profile 修改注意事项

1. **进程字段**：`process` 只允许 `netmanager`（1151~1157）与 `netsysnative`（1158），加入新进程需评估 selinux/te 策略与系统拉起配置，属系统级变更。
2. **libpath**：必须与 so 构建目标一致（如 `services/netconnmanager/BUILD.gn` 的 `ohos_shared_library("net_conn_manager")`）；改 so 名要同步 profile 与 BUILD.gn，两边缺一即 SA 加载失败。
3. **run-on-create**：1155 当前为 `false`（按需启动）。把按需 SA 改为 true 会影响开机内存/启动时长指标，需评审。
4. **extension 字段**：1152/1153 声明 `["backup","restore"]`，与策略/统计数据备份能力绑定；删除该字段前确认备份框架不再引用。
5. **BUILD.gn 同步**：`sa_profile/BUILD.gn` 统一安装 profile；新增/删除 SA 文件必须同步该 BUILD.gn，否则产品镜像缺失 profile 导致 SA 无法注册。
6. **dump_level**：当前均为 `1`；调整 dump 级别影响 hidumper 输出内容，维测排障依赖此值。
7. **新增 SA**：先在本仓登记 profile 与 ID 分配（与 ext/鸿蒙 SA 管理方确认 ID 段），再落实现仓；禁止复用既有 ID。

## 关联知识

- [netmanager-base-code-map.md](netmanager-base-code-map.md)：全局分层与目录职责
- [netsys-native-map.md](netsys-native-map.md)：SA 1158 内部命令组与 proxy/stub 结构
- [net-conn-pipeline-map.md](net-conn-pipeline-map.md)：SA 1151 的注册链与 DefaultNet 评选
- [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md)：SA 上新增 IPC 命令的完整 checklist
- [../expert/architecture-invariants.md](../expert/architecture-invariants.md)：跨进程调用不变量
- [../expert/ipc-security-constraints.md](../expert/ipc-security-constraints.md)：stub 权限映射与安全约束

## 适用场景

- 新增/修改任一 SA 行为、调整 profile 字段（进程/libpath/run-on-create/extension）
- 排查"SA 未注册/加载失败/按需不拉起"类启动问题
- base 与 netmanager_ext 协同改动时确认两仓分工
