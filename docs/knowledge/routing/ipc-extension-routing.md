# IPC 命令扩展路由（本仓特有）

> 本文覆盖"给某个 SA 新增一条 IPC 命令"的完整场景：从命令字定义到 fuzzer 覆盖的 8 步 checklist，每步给出漏做后果。新增/修改任何跨进程接口（proxy/stub/回调/权限/错误码）前必读；漏任何一步都可能直接形成漏洞或线上事故。

适用对象：SA 1151（ConnInterfaceCode）、1152（PolicyInterfaceCode）、1153（StatsInterfaceCode）、1158（NetsysInterfaceCode）四套 IPC 面。

## 0. 目录规则与文件对照（动手前先记住）

| 角色 | 常规位置 | 已验证的三个例外 |
|---|---|---|
| proxy | `frameworks/native/<模块>/src/proxy/` | ① `frameworks/native/netpolicyclient/src/net_policy_service_proxy.cpp`（在 `src/` 直下）；② `frameworks/native/netmanagernative/netsys_native_service_proxy.cpp`（在模块根目录） |
| stub | `services/<模块>/src/stub/` | — |
| 命令字枚举 | `interfaces/innerkits/<模块>/include/`（conn 在 `include/proxy/conn_ipc_interface_code.h`；stats 在 `include/proxy/stats_ipc_interface_code.h`） | — |
| 权限常量 | `utils/common_utils/include/netmanager_base_permission.h`（唯一来源） | — |
| 错误码 | `interfaces/innerkits/include/net_manager_constants.h`（唯一来源） | — |

## 1. 八步 Checklist

### ① 接口/命令字定义（接口头 enum）

在对应 IPC 枚举（如 `ConnInterfaceCode`/`PolicyInterfaceCode`/`StatsInterfaceCode`/`NetsysInterfaceCode`）末尾追加命令字；同时补接口虚方法（如 `i_net_conn_service.h`/`i_net_policy_service.h`/`i_net_stats_service.h`/`netsys_native_service.h` 体系）。

- **漏做会发生什么**：命令字冲突或复用旧值，会把新请求打进旧分支，造成数据错解/越权执行旧逻辑。

### ② proxy 方法实现（客户端组包）

在 `frameworks/native/<模块>/`（目录例外见 §0）新增 proxy 方法：组 `MessageParcel` → `SendRequest` → 读回执。**写 Parcel 的字段顺序必须与 stub 读入顺序逐字段对称**（含 `WriteInterfaceToken`/`ReadInterfaceToken`）。

- **漏做会发生什么**：顺序不对称 ⇒ 对端按错字段解包，产生垃圾参数、野指针长度、静默返回错数据；回执读取不对称 ⇒ 应用拿到错误结果码。这是 2607 行 `net_conn_service_proxy.cpp`、4548 行 `netsys_native_service_proxy.cpp` 红区的头号事故来源。

### ③ stub OnRemoteRequest 分支（服务端分发）

在 `services/<模块>/src/stub/` 的 `OnRemoteRequest` 增加命令分支与处理函数，分支内按 ① 之外的顺序读 Parcel、调用 service 实现、写回执。

- **漏做会发生什么**：命令落到 `IPCObjectStub::OnRemoteRequest` 默认分支，表现为"调了没反应"；或漏读一个字段导致后续请求流全部错位（同类事故在 2416 行 `net_conn_service_stub.cpp` 上多次出现）。

### ④ stub 权限映射表登记（安全步，不可跳过）

把"命令字 → 权限"登记进 stub 的映射表。模式参考：

- `net_policy_service_stub.cpp:30` 起的 `g_codeNPS`（`std::map<uint32_t, const char*>`：命令字 → `Permission::MANAGE_NET_STRATEGY`；公开自查询类命令入 `g_pubCodeNPS` 空权限表）；
- `net_conn_service_stub.cpp` 的特殊分支：`CMD_NM_SET_INTERNET_PERMISSION` 在 net_conn_service_stub.cpp:317 附近做 uid 检查（`IPCSkeleton::GetCallingUid()` + `UID_FOUNDATION`）+ `Permission::CONNECTIVITY_INTERNAL`；
- **权限常量只允许取自 `netmanager_base_permission.h` 的 `Permission` 命名空间**（全表见 [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md) §6），校验走 `NetManagerPermission::CheckPermission*`/`CheckUidPermission`。

- **漏做会发生什么**：**越权风险=安全漏洞**：任意三方应用可直接调用管理面接口改策略/改默认网/读统计，属高危缺陷；需系统权限的命令误入公开表同样是漏洞。

### ⑤ 回调成对新增（若有回调）

服务端要反向通知客户端时：客户端侧新增 **callback stub**（收事件，如 `frameworks/native/netconnclient/src/proxy/net_conn_callback_stub.cpp` 形态），服务端配套新增 **callback proxy**（发事件，如 `services/netconnmanager` 侧 wrapper / `services/netpolicymanager/src/stub/net_policy_callback_proxy.cpp` 形态），两端注册命令字同步。

- **漏做会发生什么**：漏 stub=事件黑洞（注册成功但永远收不到）；漏 proxy 或生命周期管理=悬挂指针/UAF（回调对象先于服务端析构）；只加一端=联调期才暴露的半成品接口。

### ⑥ 内核侧联动（需要 netsysnative 配套时）

管理面新命令若要落地内核（规则/路由/带宽等），传导路径固定：

```
管理面 service（services/netconnmanager|netpolicymanager|netstatsmanager）
  → services/netsyscontroller/src/netsys_controller.cpp（NetsysController 门面）
  → services/netsyscontroller/src/netsys_native_client.cpp（NetsysNativeClient，IPC 客户端）
  → SA 1158 stub（services/netmanagernative/src/netsys_native_service_stub.cpp）
  → NetsysNativeService → manager/* → netsys/*（netlink/iptables）或 bpf/*（BPF map）
```

在 SA 1158 侧按 ①~④ 再走一遍（`NetsysInterfaceCode` 登记、`frameworks/native/netmanagernative/netsys_native_service_proxy.cpp` 与 stub 对称）。

- **漏做会发生什么**：管理面回了成功但内核无变化（"假成功"）；或绕过 controller 直连 client 破坏 `mock_netsys_native_client.cpp` 可测性并违反架构不变量（见 [../expert/architecture-invariants.md](../expert/architecture-invariants.md)）。

### ⑦ fuzzer 覆盖新增 case

为新增命令在 `test/fuzztest/` 对应 fuzzer 补 case：`NetConnClientFuzzTest` / `NetPolicyClientFuzzTest` / `NetStatsClientFuzzTest`（netsys 分支面为 `EnableVnicNetworkFuzzTest`/`IsDeadFlowResetTargetBundleFuzzTest`/`NetBaseBranchFuzzTest`/`UpdateNetCapsFuzzTest`；防火墙为 `NetSysNetFirewallFuzzTest`，`NfQueueFuzzTest` 受防火墙开关门控）。注意 `test/fuzztest/netsysclient_fuzzer/` 目录无 BUILD.gn、不可构建，勿引用。

- **漏做会发生什么**：恶意/畸形 Parcel（超长字符串、非法长度、错 token）直接打到新分支，边界缺陷上线后才暴露；新命令的红区入口（proxy 组包/stub 解包）无回归兜底。

### ⑧ 错误码登记

对外可见的失败语义在 `interfaces/innerkits/include/net_manager_constants.h` 登记：本仓 IPC 主段为 2100001~2100301（另有 19900001/19900002 内部段、2100400/2100401 iface/地址段；防火墙复用 2100001~2100003 + 294 系列；JS 层使用 201/202/401/801）。不得自造裸数字。

- **漏做会发生什么**：对端拿魔数无法归因；与既有码冲突后 JS 层错误提示错乱，`utils/errorcode_utils` 的错误映射同样失配。

## 2. 步骤自检表（提测前逐项打勾）

| 步骤 | 产物 | 勾 |
|---|---|---|
| ① | 命令字 + 接口虚方法 | ☐ |
| ② | proxy 方法（Parcel 对称） | ☐ |
| ③ | stub 分支 + service 实现 | ☐ |
| ④ | 权限映射表登记（`netmanager_base_permission.h` 取值） | ☐ |
| ⑤ | callback stub + proxy 成对 | ☐ |
| ⑥ | netsys 侧命令字/proxy/stub 同步 | ☐ |
| ⑦ | 对应 fuzzer 新 case | ☐ |
| ⑧ | 错误码登记 + 返回值核对 | ☐ |

## 3. 依赖仓与特性开关参考

改动若触碰下列依赖或开关，先确认接口稳定性与开关两态编译：

- **依赖仓**：dhcp（`services/netmanagernative/src/dhcp_controller.cpp`）、cellular_data + core_service（蜂窝测角）、netmanager_ext（SA 1154/1155/1157 实现）、libbpf（BPF 加载读取）、iptables（`netsys/iptables_wrapper.cpp`）、common_event_service（限额/共享事件广播）、data_share / relational_store / sqlite（`net_stats_sqlite_statement.cpp` 等落库）、bundle_framework / os_account（uid/包语义）、curl / jerryscript（PAC 脚本求值，`net_pac_manager.cpp`）、ffrt / eventhandler（并发与事件循环）。
- **特性开关**（`netmanager_base_config.gni`，非任务素材列出的开关以文件为准）：`netmanager_base_enable_feature_net_firewall`（默认 false）、`netmanager_base_enable_feature_sysvpn`（默认 false）、`netmanager_base_enable_feature_hosts`（默认 false）、`netmanager_base_enable_public_dns_server`（默认 false）、`netmanager_base_enable_pac_proxy`（默认 false）、`netmanager_base_enable_traffic_statistic`（默认 false）、`netmanager_base_share_traffic_limit_enable`（默认 false）、`netmanager_base_extended_features = true`。
- 开关控制的命令字分支在两种配置下都必须编译与测试；条件编译遗漏会导致某形态产品上 IPC 面不一致。

## 关联知识

- [task-routing.md](task-routing.md)：其他任务场景的路由入口
- [vocabulary-routing.md](vocabulary-routing.md)：命令字枚举/权限常量速查
- [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md)：连接链上的 IPC 三件套实例
- [../code_map/netsys-native-map.md](../code_map/netsys-native-map.md)：netsys proxy/stub/client 三件套
- [../code_map/sa-architecture.md](../code_map/sa-architecture.md)：四套 IPC 面所属 SA 与进程拓扑
- [../expert/ipc-security-constraints.md](../expert/ipc-security-constraints.md)：④⑤ 步背后的安全约束详解
- [../expert/architecture-invariants.md](../expert/architecture-invariants.md)：⑥ 步"唯一通道"不变量
- [../verify/build-and-test.md](../verify/build-and-test.md)：编译与测试命令

## 适用场景

- 给 1151/1152/1153/1158 任一 SA 新增接口/命令字/回调
- 评审 IPC 类改动时按 §2 自检表查漏（漏权限登记/漏 fuzzer 是最高频缺陷）
- 新命令需要触达内核（规则/路由/带宽）时确认 netsys 侧联动范围
