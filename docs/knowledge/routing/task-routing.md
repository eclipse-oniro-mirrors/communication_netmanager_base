# 任务场景路由

> 拿到一个任务不确定从哪里下手时，先在本文按场景查"先读什么、动哪里"，再进入对应 code_map/expert 文档或源码路径。本仓是双进程多 SA 仓，先定 SA 再定层，能少走弯路。

使用方法：按任务描述对号入座第一列 → 读"先读"列 → 按"动手路径"列定位代码。任务模糊时先用兜底行。

## 1. 场景路由表

| 要做的事 | 先读 | 动手路径 |
|---|---|---|
| 新增/修改 SA 行为（1151/1152/1153/1158 任一） | [../code_map/sa-architecture.md](../code_map/sa-architecture.md) + [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md) | 对应 `services/<模块>/src/net_*_service.cpp`；先确认该 SA 的职责边界与依赖方向 |
| 新增一条 IPC 命令（接口/命令字/权限/回调） | [ipc-extension-routing.md](ipc-extension-routing.md)（本仓特有，8 步 checklist） | 接口头 `interfaces/innerkits/*/include/*_ipc_interface_code.h` → proxy → stub → 权限映射 → 回调成对 → fuzzer → 错误码，缺步即事故 |
| 网络注册与 DefaultNet 评选 | [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §1-§2 | `services/netconnmanager/src/net_conn_service.cpp`（4787 行红区）、`net_supplier.cpp`、`network.cpp` |
| 网络探测（VALIDATED/PORTAL 判定） | [../code_map/net-conn-pipeline-map.md](../code_map/net-conn-pipeline-map.md) §3 | `services/netconnmanager/src/net_monitor.cpp`、`probe_thread.cpp`、`net_http_probe.cpp`；注意 2103f660 内存序教训 |
| 应用侧 JS API（连接/策略/统计） | `interfaces/kits/js/@ohos.net.{connection,policy,statistics}.d.ts` + [../expert/architecture-invariants.md](../expert/architecture-invariants.md) | `frameworks/js/napi/{connection,netpolicy,netstats}`；接口稳定性以 `interfaces/` 为基线 |
| 策略/防火墙规则 | [../code_map/policy-stats-bpf-map.md](../code_map/policy-stats-bpf-map.md) 链① | `services/netpolicymanager/src/{core,firewall_rule}` 两层 + netsys 侧 `manager/firewall_manager.cpp`，两层同步改 |
| 流量统计与限额通知 | [../code_map/policy-stats-bpf-map.md](../code_map/policy-stats-bpf-map.md) 链②③ | `services/netstatsmanager/src/net_stats_service.cpp`（2647 行）、`net_stats_cached.cpp`、`services/netstatslimitntf`；参考 9ee4ebbf 教训 |
| NETSYSNATIVE 内核通道（路由/带宽/接口/共享） | [../code_map/netsys-native-map.md](../code_map/netsys-native-map.md) | `services/netmanagernative/src/manager/*.cpp`；base 侧只能经 `services/netsyscontroller` 调用 |
| BPF 数据面（map/程序/ring buffer） | [../code_map/policy-stats-bpf-map.md](../code_map/policy-stats-bpf-map.md) §4-§5 | `bpf/bpf_progs/`、`services/netmanagernative/bpf/`；先看 c3d1d423 越界读教训 |
| 权限与错误码 | [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md) §5-§6 | 权限常量只在 `utils/common_utils/include/netmanager_base_permission.h` 取；错误码登记 `interfaces/innerkits/include/net_manager_constants.h` |
| 多语言绑定同步（ANI/仓颉/NDK） | [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md) §3 目录表 | `frameworks/ets/ani/`（connection/policy/statistics）、`frameworks/cj/`、`interfaces/kits/c/netconnclient`；改动接口后逐一核对 |
| SA profile 变更（与 ext 联动） | [../code_map/sa-architecture.md](../code_map/sa-architecture.md) §4-§5 | `sa_profile/*.json` + `sa_profile/BUILD.gn`；1154/1155/1157 实现在 netmanager_ext 仓，需两仓协同 |
| 崩溃/竞态/UAF 排查 | [../expert/historical-failures.md](../expert/historical-failures.md) + [../expert/concurrency-failure-modes.md](../expert/concurrency-failure-modes.md) | 优先核对红区清单（4787/4548/3238/2607/2416/2376 行六件套），按历史失效模式比对 |
| 新增测试用例 | `test/<module>/unittest` 与 `test/fuzztest/` 现有形态 | 单测按模块对号入座；新增 IPC 命令必须补 `netconnclient_fuzzer`/`netpolicyclient_fuzzer`/`netstatsclient_fuzzer`/`netsysclient_fuzzer` 对应 case |
| 不确定去处（兜底） | [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md) §3 目录职责表先扫一遍 | 按"管理面进 services/net*、内核面进 services/netmanagernative、客户端进 frameworks/native"三分法归位；仍不确定则按 SA 拆解 |
| 术语看不懂 | [vocabulary-routing.md](vocabulary-routing.md) | 词条表 + 枚举真实值速查表 |

## 2. 改动区域 → 必跑测试

| 改动区域 | 必跑测试 |
|---|---|
| 连接管理（SA 1151） | `test/netconnmanager/unittest` + `test/fuzztest/netconnclient_fuzzer` |
| 策略（SA 1152） | `test/netpolicymanager/unittest` + `test/fuzztest/netpolicyclient_fuzzer` |
| 统计（SA 1153） | `test/netstatsmanager/unittest` + `test/fuzztest/netstatsclient_fuzzer` |
| netsys 通道（SA 1158） | `test/netmanagernative/unittest` + `test/fuzztest/netsysclient_fuzzer` |
| BPF 统计 | `test/netsysbpfstats/unittest` |
| 公共/工具 | `test/netcommon/unittest`、`test/netmanagerutils/unittest`、`test/netmanagerconnection/unittest` |
| 防火墙 | `test/fuzztest/netsysnetfirewall_fuzzer`、`nfqueue_fuzzer` |

构建与测试命令见 [../verify/build-and-test.md](../verify/build-and-test.md)；验证循环与 Done 定义见 [../verify/verification-loop.md](../verify/verification-loop.md)。

## 3. 路由基本原则

- 先定进程再定层：能力落在 netmanager 进程还是 netsysnative 进程，决定改动起点（对照 [../code_map/sa-architecture.md](../code_map/sa-architecture.md) 拓扑）。
- 六大红区文件（`net_conn_service.cpp` 4787、`netsys_native_service_proxy.cpp` 4548、`netsys_native_service_stub.cpp` 3238、`net_conn_service_proxy.cpp` 2607、`net_stats_service.cpp` 2647、`net_conn_service_stub.cpp` 2416、`netsys_native_client.cpp` 2376）改动必须绑定回归测试。
- 跨进程链路的改动永远是三件套起步（接口头/命令字 → proxy → stub），回调再加两件；漏权限登记属安全红线，见 [ipc-extension-routing.md](ipc-extension-routing.md)。
- 涉及 ext 三 SA（1154/1155/1157）的行为改动在 netmanager_ext 仓落地，本仓只改 profile/常量。
- 遵守仓根 [AGENTS.md](../../../AGENTS.md) 的通用约束。

## 关联知识

- [vocabulary-routing.md](vocabulary-routing.md)：按术语触发
- [ipc-extension-routing.md](ipc-extension-routing.md)：IPC 命令扩展专路
- [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md)：全局地图
- [../code_map/sa-architecture.md](../code_map/sa-architecture.md)：SA 架构
- [../expert/ipc-security-constraints.md](../expert/ipc-security-constraints.md)：权限映射红线
- [../expert/historical-failures.md](../expert/historical-failures.md)：历史失效模式

## 适用场景

- 任务描述模糊、不确定改动落点与风险面
- 新成员熟悉"什么任务先读什么、动哪里"
- 评审他人改动时快速判断遗漏环节（如漏 fuzzer、漏权限登记）
