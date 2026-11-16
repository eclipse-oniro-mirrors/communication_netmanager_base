# 验证循环与 Done 定义（verify）

> 本文定义"改动 → 验证 → 达成 Done"的闭环：按任务类型选最小验证集、按顺序执行、全部通过才可宣称完成；不通过走 §5 回退。
> 编译命令与 target 实名表见 [build-and-test.md](build-and-test.md)。

## 1. 验证顺序（从近到远，前一步失败不继续）

1. 编译：对应模块 target 两态（特性开关开/关）均过（[build-and-test.md](build-and-test.md) §1/§3）。
2. 单测：本任务最小验证集（§2 矩阵）全绿。
3. Fuzzer：改动 IPC 面/解析面时补跑/补写对应 fuzzer case（§2 末列）。
4. 静态检查：无新增编译告警；如有 clang-tidy/LSP 门禁，无新增 error。
5. 接口核对：触碰 `interfaces/` 任意声明时，`frameworks/js/napi/`、`frameworks/ets/ani/`、`frameworks/cj/`、`interfaces/kits/c/` 四套绑定逐一比对。
6. 文档回写：新失效模式/新 target/新术语按 AGENTS.md §8 回写知识库。

## 2. 按任务类型的最小验证矩阵

| 改动区域 | 最小单测集 | Fuzzer/补充 |
|---|---|---|
| IPC 命令新增/修改（proxy/stub/回调/权限/错误码） | 所属模块单测（如 conn 改动跑 `net_conn_service_test`+`net_conn_manager_test`） | 对应 client fuzzer 新 case 必补；按 [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md) §2 自检表打勾 |
| 连接管理（services/netconnmanager、frameworks/native/netconnclient） | `net_conn_manager_test`、`net_conn_service_test`、`net_conn_service_regional_test`、`net_conn_multi_test`、`net_supplier_test` | `NetConnClientFuzzTest`；改 DefaultNet/评选逻辑自测并发注册场景 |
| 策略/防火墙（netpolicymanager + 1158 防火墙链） | `net_policy_service_test`、`net_policy_manager_test`、`firewall_manager_test`、`bandwidth_manager_test` | `NetPolicyClientFuzzTest`、`NetSysNetFirewallFuzzTest`；防火墙开关两态必测 |
| 流量统计/缓存/落库（netstatsmanager） | `net_stats_manager_test`、`net_stats_service_new_test`、`net_stats_cached_service_test` | `NetStatsClientFuzzTest`；改 `net_stats_cached.cpp`（1393 行红区，51129900 历史区）绑概率死锁回归 |
| netsysnative/内核通道（netmanagernative、netsyscontroller） | `netsys_native_manager_test`、`netsys_controller_test`、`netsys_dnsresolv_test`、`netsys_native_client_test` | `EnableVnicNetworkFuzzTest` 等 netsys 分支 fuzzer；改 `probe_thread.cpp` 绑内存序并发用例（2103f660） |
| BPF 数据面（bpf/） | `netsys_bpf_stats_test`、`nat464_clat_test` | `UpdateNetCapsFuzzTest`、`NetSysNetFirewallFuzzTest`；内核态程序需真机加载验证 |
| napi/公共工具（frameworks/js、utils） | `netmanager_utils_test`、`icu_helper_test`、`net_common_test` | `CommonFuzzTest`；异步改动核对 `base_async_work.h` 生命周期（f2e10ae5） |
| SA profile / 跨仓联动（sa_profile、1154/1155/1157） | 本仓编译两态 + ext 仓对应用例（在 ext 仓跑） | 两仓配套 MR 一起评审，禁止只合一边 |
| 多语言绑定（ani/cj/NDK） | 对应绑定用例 + 主链路单测 | 四套绑定的 API 行为一致性人工核对 |

> 表中 target 实名均已核对（[build-and-test.md](build-and-test.md) §2）；跑前如怀疑 target 变动，以 `test/<module>/unittest/BUILD.gn` 为最终口径。

## 3. 并发自测要求（一次性补充）

历史 TOP1 失效是并发/死锁（[../expert/historical-failures.md](../expert/historical-failures.md)）。凡改动以下区域，除矩阵外**必须构造并发场景自测**并记录结果：

- 容器遍历中触发回调（注册/注销供应商、状态推送）：并发注册 × 回调风暴；
- 持锁调用外部接口（IPC/DB/回调）：两把锁交叉用多线程压测；
- `probe_thread.cpp`、`event_manager.cpp`、napi 工作线程：长循环 + 频繁启停；
- SA 崩溃恢复路径（8c973dfb）：kill 进程后重启验证残留状态清理。

模式对照见 [../expert/concurrency-failure-modes.md](../expert/concurrency-failure-modes.md)（模式一~五）。

## 4. Done 定义（全部满足才算完成）

1. 编译成功：模块 target + 相关聚合组无错误；**特性开关两态均过**（INV-8）。
2. §2 矩阵对应测试全部通过，§3 适用时并发自测通过。
3. 静态检查无新增 error/告警。
4. 未破坏 `interfaces/` 基线（JS/NDK/innerkits/ani/cj 五面核对）。
5. 未违反 AGENTS.md §6.1 任何禁止事项；需确认项（§6.2）已向用户确认。
6. 代码已格式化（`clang-format` 校验）；新增知识已按 AGENTS.md §8 回写。

## 5. 回退方案

- 验证不通过且短期无法修复：`git revert` 对应提交，不在主干留带病代码。
- **回退后必须**：把失效模式与教训回写 [../expert/historical-failures.md](../expert/historical-failures.md)，并在 [../routing/task-routing.md](../routing/task-routing.md) 增加路由条目（若为新形态）。
- 跨仓改动（1154/1155/1157 联动）回退必须两仓同步 revert，避免 profile 与实现脱节。

## 6. 提交前检查清单

| 项 | 勾 |
|---|---|
| §2 矩阵执行记录（target + 结果）已留存 | ☐ |
| 红区大文件改动已绑回归用例（[build-and-test.md](build-and-test.md) §5 清单） | ☐ |
| 新 IPC 命令 8 步 checklist 全勾（ipc-extension-routing §2） | ☐ |
| 命令字/错误码/权限/事件四处登记齐全 | ☐ |
| auto-generated 段未改动；删除代码均已确认非生成物 | ☐ |
| 提交信息含需求/缺陷单号（DTS/SR），未夹带无关文件 | ☐ |

## 关联知识

- [build-and-test.md](build-and-test.md)：编译命令、target 实名、开关与观测口径
- [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md)：IPC 改动的 8 步自检
- [../expert/concurrency-failure-modes.md](../expert/concurrency-failure-modes.md)：并发自测场景设计参考
- [../expert/historical-failures.md](../expert/historical-failures.md)：回退教训登记处

## 适用场景

- 任何代码改动完成后、宣称"完成"之前
- 评审他人 MR 时按 §2/§6 核对验证证据
- 测试失败决定修复或回退时按 §5 执行
