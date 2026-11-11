# AGENTS.md — netmanager_base 领域知识入口（AI Agent 必读）

> 组件 `@ohos/netmanager_base` | subsystem `communication` | SysCap `SystemCapability.Communication.NetManager.Core` | 权限中枢 `utils/common_utils/include/netmanager_base_permission.h` | hisysevent domain `NETMANAGER`
> 本文件是本仓 AI Agent 工作的第一入口。领域知识库位于 `docs/knowledge/`（14 篇，见 §3）。**编辑任何代码前必须完成 §5 编辑前声明。**

## 1. 项目定位

- 职责一句话：为系统与应用提供网络连接管理（注册/评选/探测）、网络策略（Uid 规则/背景策略/防火墙）、流量统计与限额，以及 netsysnative 内核网络通道（路由/DNS/带宽/防火墙的落地执行）。
- 形态：**双进程多 SA 的系统服务仓**——`netmanager` 进程承载管理面 SA（1151/1152/1153），`netsysnative` 进程承接内核面执行（SA 1158，profile 全部 8 个在本仓 `sa_profile/`）。
- SA 全景：1151 net_conn_manager、1152 net_policy_manager、1153 net_stats_manager、1158 netsys_native_manager 为本仓实现；**1154 tether / 1155 vpn / 1157 ethernet 的 profile 在本仓、实现在 netmanager_ext 仓**；1156 dns_resolver 仅 profile。
- 与 netstack 的关系：netstack 协议栈经 `NetConnClient` 获取网络/代理信息（编译期宏 `HAS_NETMANAGER_BASE` 控制接入），本仓是被依赖方；改连接/回调语义须评估 netstack 降级路径。
- 特性开关集中在 `netmanager_base_config.gni`：`netmanager_base_enable_feature_net_firewall`、`_sysvpn`、`_hosts`、`netmanager_base_enable_public_dns_server`、`_pac_proxy`、`_traffic_statistic`、`netmanager_base_share_traffic_limit_enable`（多默认 false）、`_extended_features`（默认 true）；**开关相关代码两种配置都要编译测试**。
- 分层（自上而下，依赖只能向下）：
  1. JS API 声明：`interfaces/kits/js/`；NDK：`interfaces/kits/c/netconnclient`
  2. NAPI 层：`frameworks/js/napi/{connection,netpolicy,netstats,network,common}`
  3. 客户端 inner API：`frameworks/native/{netconnclient,netpolicyclient,netstatsclient,netmanagernative,netsysclient}` + `interfaces/innerkits/`
  4. 服务端 SA stub：`services/{netconnmanager,netpolicymanager,netstatsmanager}`
  5. 内核通道：`services/netsyscontroller` → NetsysNativeClient --IPC--> SA 1158（netsysnative 进程）
  6. 数据面/内核：`services/netmanagernative/{manager,netsys,bpf}` + `bpf/bpf_progs` + netlink/iptables
- 多语言绑定与主链路平行：`frameworks/ets/ani`（ArkTS Rust ANI）、`frameworks/cj`（仓颉）、`interfaces/kits/c/`（NDK）。**同一能力变更需同步所有绑定。**
- 观测：`hisysevent.yaml`（domain `NETMANAGER`，FAULT/BEHAVIOR/STATISTIC）。

## 2. 任务到路径映射

| 任务类型 | 先看这里 | 原因 |
|---|---|---|
| 新增/修改 IPC 命令（proxy/stub/回调/权限/错误码） | [docs/knowledge/routing/ipc-extension-routing.md](docs/knowledge/routing/ipc-extension-routing.md) | 本仓最高频任务：8 步 checklist，漏权限登记=安全漏洞 |
| 公共 JS API / NDK 变更 | `interfaces/kits/` + [api 基线不变量](docs/knowledge/expert/architecture-invariants.md) | 接口稳定性基线在 `interfaces/` |
| 网络注册 / DefaultNet / 回调 / 探测 | [docs/knowledge/code_map/net-conn-pipeline-map.md](docs/knowledge/code_map/net-conn-pipeline-map.md) | 连接链最长，含 napi 入口与红区标注 |
| 策略 / 防火墙 / 背景策略 / 限额 | [docs/knowledge/code_map/policy-stats-bpf-map.md](docs/knowledge/code_map/policy-stats-bpf-map.md) | 三链汇聚处 |
| 流量统计 / 缓存 / 落库 | [docs/knowledge/code_map/policy-stats-bpf-map.md](docs/knowledge/code_map/policy-stats-bpf-map.md) + [历史台账](docs/knowledge/expert/historical-failures.md) | 51129900 概率死锁历史区 |
| netsysnative / 路由 / DNS / 带宽 / BPF map | [docs/knowledge/code_map/netsys-native-map.md](docs/knowledge/code_map/netsys-native-map.md) | 1158 服务解剖与 proxy/stub 三件套 |
| BPF 数据面程序 | `bpf/bpf_progs/` + [历史台账](docs/knowledge/expert/historical-failures.md) | c3d1d423 越界读教训 |
| SA 注册 / profile 变更 | `sa_profile/` + [代码地图](docs/knowledge/code_map/sa-architecture.md) | 1154/1155/1157 实现在 ext，须跨仓联动 |
| 权限 / 错误码 / 观测事件 | `utils/common_utils/include/netmanager_base_permission.h`、`interfaces/innerkits/include/net_manager_constants.h`、`hisysevent.yaml` | 三大公共基线，禁止散落定义 |
| NAPI 基础设施 / 多语言绑定 | `frameworks/js/napi/` + `frameworks/ets/ani/` + `frameworks/cj/` | js/ani/cj/NDK 四套绑定共享生命周期规则 |
| 崩溃 / 竞态 / 死锁排查 | [docs/knowledge/expert/historical-failures.md](docs/knowledge/expert/historical-failures.md) + [并发失效模式](docs/knowledge/expert/concurrency-failure-modes.md) | 按失效形态直接对号历史案例 |
| 新增任务类型（不确定去处） | [docs/knowledge/routing/task-routing.md](docs/knowledge/routing/task-routing.md) | 场景路由总表 |

## 3. 知识索引（场景路由）

全部知识文档位于 `docs/knowledge/`，按场景先读一篇再动手：

| 场景 | 先读 |
|---|---|
| 了解全仓分层 / 双进程 8 SA | [netmanager-base-code-map.md](docs/knowledge/code_map/netmanager-base-code-map.md) |
| SA 详解 / 进程拓扑 / profile 约束 | [sa-architecture.md](docs/knowledge/code_map/sa-architecture.md) |
| 网络注册与探测全链路 | [net-conn-pipeline-map.md](docs/knowledge/code_map/net-conn-pipeline-map.md) |
| 策略 / 统计 / BPF 数据面 | [policy-stats-bpf-map.md](docs/knowledge/code_map/policy-stats-bpf-map.md) |
| netsysnative 服务解剖 | [netsys-native-map.md](docs/knowledge/code_map/netsys-native-map.md) |
| 不确定任务归类 | [task-routing.md](docs/knowledge/routing/task-routing.md) |
| 术语 / 枚举值速查 | [vocabulary-routing.md](docs/knowledge/routing/vocabulary-routing.md) |
| IPC 命令扩展 8 步 checklist | [ipc-extension-routing.md](docs/knowledge/routing/ipc-extension-routing.md) |
| IPC / 权限 / Parcel 安全红线 | [ipc-security-constraints.md](docs/knowledge/expert/ipc-security-constraints.md) |
| 并发 / 死锁 / 竞态模式 | [concurrency-failure-modes.md](docs/knowledge/expert/concurrency-failure-modes.md) |
| 架构不变量 | [architecture-invariants.md](docs/knowledge/expert/architecture-invariants.md) |
| 历史高危修复台账 | [historical-failures.md](docs/knowledge/expert/historical-failures.md) |
| 构建与测试命令 | [build-and-test.md](docs/knowledge/verify/build-and-test.md) |
| 验证循环与 Done 定义 | [verification-loop.md](docs/knowledge/verify/verification-loop.md) |

## 4. 词汇触发路由

任务描述中出现以下术语时，先读 [vocabulary-routing.md](docs/knowledge/routing/vocabulary-routing.md) 对应条目：

| 术语 | 含义 |
|---|---|
| `NetConnService` | SA 1151 服务核心（`net_conn_service.cpp`，4787 行红区） |
| `NetSupplier` / `ident` / supplier score | 网络能力提供者抽象 / bearer 标识 / DefaultNet 评选权重 |
| `NetMonitor` / `NetDetection` | 网络探测与 PORTAL/VALIDATED 判定 |
| `NetsysController` / `NetsysNativeClient` | base 侧内核通道门面（`netsys_controller.cpp` 2040 行）/ IPC 客户端（2376 行） |
| `NetsysNativeService` | SA 1158 服务体（netsysnative 进程，业务进程禁止直连） |
| `Fwmark` / `FwmarkClient` | 内核 mark 位，策略路由执行点 |
| `BpfStats` | 流量统计 BPF 链（bpf_progs/netstats.c → bpf_stats → SQLite） |
| `QuotaPolicy` / `LimitAction` | 流量限额策略与触发动作 |
| `FirewallRule` | 防火墙规则链（policy 侧防火墙 → bpf_netfirewall） |
| `CLAT` / `NAT464` | IPv4-over-IPv6 转换（`bpf/bpf_progs/464xlat.c`） |
| `NetManagerCenter` | 客户端共享管理中枢 |
| `HttpProxy` / PAC | 代理下发与 PAC 脚本求值（curl/jerryscript） |
| `g_codeNPS` | stub 命令字→权限映射表模式（新增 IPC 命令必登记） |

## 5. 编辑前声明要求

Agent 在编辑任何文件前，**必须在回复中显式回答以下三项**（缺一不可，答不出必须先读 §3 对应文档）：

1. **本次任务类别**：IPC 命令变更 / SA 服务逻辑 / 策略与防火墙 / 统计与限额 / netsysnative 内核通道 / BPF 数据面 / 权限与错误码 / NAPI 与多语言绑定 / profile 与跨仓联动 / 测试 / 文档（多选可叠加）。
2. **已读哪些知识文档**：列出本次已读的 `docs/knowledge/` 文档路径；至少包含任务类别对应的 §3 场景文档一篇。
3. **已知约束中哪些适用本任务**：从 §6 中点名适用的条目（如「适用：内核访问必须走 NetsysController 唯一通道；适用：新增命令须登记 g_codeNPS 权限表」）。

## 6. 约束和边界

### 6.1 禁止事项（红线）

- 禁止绕过 `services/netsyscontroller` 直连 netsysnative proxy 或直接操作 netlink/BPF map/iptables——**内核访问唯一通道**。
- 禁止新增未经 stub 权限映射表登记的 IPC 命令；权限字符串只从 `netmanager_base_permission.h` 取，**禁止散落硬编码**。
- 禁止 proxy 与 stub 单侧变更（含回调：callback stub 与 callback proxy 必须成对）。
- 禁止在 napi 工作线程直接调用 JS 回调（必须切回 JS 线程，与 netstack `napi_utils` 同源模式）。
- 禁止硬编码进程名/域名（历史违规 2ae80b5e）。
- 禁止删除/注释失败测试用例使测试通过。
- 禁止未经用户确认提交代码。
- 禁止修改 auto-generated 代码段（IDL/工具生成物）。

### 6.2 需确认规则（改动前向用户确认）

- 改动红区大文件须确认配套回归已跑（§7.2）：`net_conn_service.cpp`（4787 行）、`netsys_native_service_proxy.cpp`（4548 行）、`netsys_native_service_stub.cpp`（3238 行）、`net_stats_service.cpp`（2647 行）、`net_conn_service_proxy.cpp`（2607 行）、`net_conn_service_stub.cpp`（2416 行）、`netsys_controller.cpp`（2040 行）、`netsys_native_client.cpp`（2376 行）。
- 改 `probe_thread.cpp`（历史上 2103f660 内存序崩溃）须确认探测线程并发用例通过。
- 改 stub 权限映射表须安全评审确认（漏登记/错登记=越权漏洞）。
- 改特性开关相关代码须确认两种开关配置均编译并测试通过。

### 6.3 架构不变量

- 内核访问唯一通道：`NetsysController → NetsysNativeClient → SA 1158`；业务进程内不得出现数据面操作。
- 权限校验集中于 stub 层（`g_codeNPS` 映射表模式，`net_policy_service_stub.cpp:30` 起；`net_conn_service_stub.cpp:317` uid 检查分支）。
- proxy/stub 成对存在于固定目录（`frameworks/native/<模块>/src/proxy/` ↔ `services/<模块>/src/stub/`，例外清单见 [ipc-extension-routing.md](docs/knowledge/routing/ipc-extension-routing.md) §0）。
- 接口稳定性以 `interfaces/`（kits+innerkits）为基线；错误码统一登记 `net_manager_constants.h`（本仓 IPC 主段 2100001~2100301）。
- 观测事件统一走 `hisysevent.yaml`（domain 固定 `NETMANAGER`）。

### 6.4 项目特定陷阱（历史失效模式排行）

1. **并发/竞态/死锁**（第一热点）：napi 回调线程 × 服务容器、probe 线程、ffrt/eventhandler 混用——修复案例 a98cd919（钉钉 Top1 死锁，`event_manager.cpp` 两阶段锁）、51129900（DTS2025120122477 流量查询概率死锁，`net_stats_cached.cpp`）、d0fda269（netSuppliers map 多线程损坏，`net_conn_service.cpp` 引入 ffrt rwlock）、044d5d70（`network_exec.cpp` 多线程 map 崩溃）、8c973dfb（崩溃恢复状态残留）、080a440c（race）、ea5291d7（defaultNetSupplier 锁）。此区修复必须配套并发回归用例。
2. 空指针/IPC 对象生命周期：63961c40（`netsys_native_service_proxy.cpp` 278 行判空重构）。
3. 资源泄漏：f2e10ae5（DTS2026032818910 napi 异步资源泄漏，`base_async_work.h`）、1b8b67fb（fd 泄漏）。
4. 安全/权限/参数校验遗漏（详见 [ipc-security-constraints.md](docs/knowledge/expert/ipc-security-constraints.md)）。
5. 数据面越界读：c3d1d423（`bpf_netfirewall.cpp` + `bpf_ring_buffer.cpp`）。
6. 多线程内存序：2103f660（`probe_thread.cpp`）。
7. 统计丢失：9ee4ebbf。

### 6.5 Agent 失败模式（自我检查）

- 只改服务端忘改 proxy 侧（或反之）→ 按 [ipc-extension-routing.md](docs/knowledge/routing/ipc-extension-routing.md) §2 自检表逐项核对。
- 只改 native 忘改 js/ani/cj/NDK 绑定 → 改任何接口后逐一核对 `frameworks/js/napi/`、`frameworks/ets/ani/`、`frameworks/cj/`、`interfaces/kits/c/`。
- 改 `sa_profile/` 忘与 netmanager_ext 联动（1154/1155/1157 实现在 ext 仓）。
- 死锁修复时在持锁回调中新增同步调用 → 先读 [concurrency-failure-modes.md](docs/knowledge/expert/concurrency-failure-modes.md) 两阶段锁模式。

## 7. 构建和验证

### 7.1 构建（裸脚本 `build_system.sh` + `CBuild` 分布式构建，不使用 `build.sh`）

极速空间构建机上直接用裸脚本构建，由 `CBuild` 客户端提交分布式构建（`build.sh --product-name rk3568` 那套命令在本环境不可用）：

```bash
cd /srv/workspace/<基线工作区>/code
CBuild ./build_system.sh --abi-type generic_generic_arm_64only --device-type general_all_phone_standard \
  --ccache --build-target netmanager_base --build-variant root
```

- 常用子目标（gn 实名，`--build-target` 可重复传）：`net_conn_manager`、`net_policy_manager`、`net_stats_manager`、`netmanagernative`；services 组：`netmanager_base_services`；全量测试聚合组：`//foundation/communication/netmanager_base/test:unittest`。
- 分布式构建排队 + 编译耗时较长（空 out 全量约 45 分钟以上）：**等待/超时时间直接拉长**，后台执行并轮询日志（`nohup ... > /tmp/build.log 2>&1 &`），不要因等待超时中途终止。
- 完整 target 实名表见 [build-and-test.md](docs/knowledge/verify/build-and-test.md)。

### 7.2 按任务类型的验证表（改动 → 必跑测试）

**怎么跑（DT 框架 `test/testfwk/developer_test`，参考 dt-test 约定）**

```bash
# 1) 先编测试产物：§7.1 同一条命令，把 --build-target 换成下表中的测试 target
# 2) 再跑（框架固定在 <源码根>/test/testfwk/developer_test 下执行）
cd /srv/workspace/<基线工作区>/code/test/testfwk/developer_test
./start.sh run -t UT -p system_arm64_default -tp netmanager_base  # 整部件
./start.sh run -t UT -p system_arm64_default -tp netmanager_base -ts net_conn_service_test  # 单套件
./start.sh run -t FUZZ -p system_arm64_default -tp netmanager_base -ts netconnclient_fuzzer  # 模糊
```

- `-p` 非交互必传 `system_arm64_default`（不传默认 phone 会解析错 out 目录；本工作区经 `out/ohos_config.json` 兜底到 `out/generic_generic_arm_64only/general_all_phone_standard`）；`-t` 取 `UT`/`FUZZ`；`-ts` 多套件在 native 侧用逗号分隔（分号只对 HAP/XTS 生效）。
- 失败用例从 `reports/<时间戳>/summary_report.xml` 取 `classname`+`name`，用 `-tc Class.case` 重跑（`--retry` 只在交互模式有效，一次性调用必报 No history command exsit）。
- 结论看 `grep "Summary result" <log>`；设备先 `hdc list targets` 可见再跑（`config/user_config.xml` 未配 ip/port 时用它），看不到设备先查设备/hdc server，不要反复重跑。
- 整部件 UT / 任何 FUZZ 必须后台：`setsid nohup ./start.sh run ... > /tmp/dt.log 2>&1 < /dev/null &`（不用 setsid 会被工具超时连带杀掉），轮询 `pgrep -f "python3 -m main"`；单次执行超 ~61 分钟会被宿主静默终止，长任务按 ≤6 模块分批（FUZZ 每 fuzzer 默认 300s）。

| 改动区域 | 必跑测试 target |
|---|---|
| 网络连接管理（services/netconnmanager、frameworks/native/netconnclient） | `net_conn_manager_test`、`net_conn_service_test`、`net_conn_multi_test`、`net_supplier_test`、`netconnclient_fuzzer` |
| 策略/防火墙（netpolicymanager） | `net_policy_service_test`、`net_policy_manager_test`、`firewall_manager_test`、`bandwidth_manager_test`、`netpolicyclient_fuzzer` |
| 流量统计（netstatsmanager） | `net_stats_manager_test`、`net_stats_service_new_test`、`net_stats_cached_service_test`、`netsys_bpf_stats_test`、`netstatsclient_fuzzer` |
| netsysnative / 内核通道（netmanagernative、netsyscontroller） | `netsys_native_manager_test`、`netsys_controller_test`、`netsys_dnsresolv_test`、`netsysnetfirewall_fuzzer`、`setpresetdomains_fuzzer`、`enablevnicnetwork_fuzzer`（fuzzer 实名见 build-and-test.md §2.2；`nfqueue_fuzzer` 受防火墙开关门控） |
| BPF 数据面（bpf/） | `netsys_bpf_stats_test`、`netsysnetfirewall_fuzzer`、`updatenetcaps_fuzzer` |
| napi / 公共工具 | `netmanager_utils_test`、`common_fuzzer` + 对应模块 client fuzzer |
| IPC 权限/错误码 | 对应模块测试 + 全部新增 case 补进对应 fuzzer |

### 7.3 Done 定义（全部满足才算完成）

1. 构建成功（§7.1 命令无错误；开关相关代码两种配置都过）。
2. §7.2 对应测试全部通过。
3. LSP/静态检查无新增 error、无新增编译警告。
4. 未破坏 `interfaces/` 基线（SDK/NDK/innerkits 四套绑定逐面核对）。
5. 未违反 §6.1 任何禁止事项。
6. 代码已格式化（提交前 `clang-format` 校验）。

### 7.4 回退方案

- 验证不通过且短期无法修复：`git revert` 对应提交，不留在主干带病代码。
- **回退后必须把失效模式与教训回写** [historical-failures.md](docs/knowledge/expert/historical-failures.md)，并同步 [task-routing.md](docs/knowledge/routing/task-routing.md) 增加路由条目。

## 8. 文档维护

目录结构与更新规则：

```
docs/knowledge/
├── code_map/    # 代码地图：netmanager-base-code-map.md, sa-architecture.md, net-conn-pipeline-map.md, policy-stats-bpf-map.md, netsys-native-map.md
├── routing/     # 路由：task-routing.md, vocabulary-routing.md, ipc-extension-routing.md
├── expert/      # 专家约束：ipc-security-constraints.md, concurrency-failure-modes.md, architecture-invariants.md, historical-failures.md
└── verify/      # 验证：build-and-test.md, verification-loop.md
```

更新规则：

- **新失效模式**（线上/测试新发现的崩溃、死锁、泄漏形态）必须回写 `expert/historical-failures.md`，并在 `routing/task-routing.md` 与 `code_map/crash 类场景` 增加路由条目。
- 新增 SA/目录/测试 target：更新对应 `code_map/` 文档与本文 §2、§7 表格。
- 新增术语/命令字/权限：更新 `vocabulary-routing.md` 与本文 §4 表格。
- 修改构建命令/测试 target：同步 `verify/build-and-test.md`、`verification-loop.md` 与本文 §7。
- 知识文档只增不删；过期内容标注 `已废弃` 并保留原文（历史教训不可丢弃）。
