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
| 防火墙 DNS 域名门/DNS 服务器 map/预热引擎（随 fw 主开关常开，原 `FEATURE_FIREWALL_DNS_PER_DOMAIN` 子开关已删） | `FIREWALL_DNS_FIX_DESIGN_V3.md`（方案）+ `FIREWALL_FIX_EXECUTION_PLAN.md` §12/§13（执行/验证/检视台账） | `services/netmanagernative/bpf/include/netfirewall/{types,map,map_def,event,match,domain}.h` + `bpf_netfirewall.{h,cpp}` + `dns_param_cache` + `dns_manager`；触碰门/嗅探必复跑 `netsys_netfirewall_test` 的 DnsGate*/DnsSniff* 用例；工具链铁律见 [../expert/historical-failures.md](../expert/historical-failures.md) BPF 工具链条目（rm netsys.o/StubBpfValue/TCP 端口语义/单例污染） |
| 防火墙 ct 回程被杀/方向规则组合不生效（如「入向默认拒绝+仅出向放行 → 连接握手失败」） | [../expert/historical-failures.md](../expert/historical-failures.md)「P4 ct 按方向独立评估回归」条目 | `netfirewall.h` 两 hook 的 ct 早退分支（对向继承仅限非回环）+ `netfirewall_ct_def.h` evaluated 位；改 ct 语义必过「ct 双向首包 × 仅单方向规则 × 回环/非回环」矩阵 |
| 防火墙模糊域名规则不生效/被静默跳过（`*.domain.com` 形态） | [../expert/historical-failures.md](../expert/historical-failures.md)「AC 构建器拒收前导 `*.` 模糊模式」条目 | `bpf_netfirewall.h` 的 `BuildPattern`/`Emit`（MORE 伪类）+ 预热注册表过滤；出现 `skip invalid domain` 日志即配置面与执行面脱节，按缺陷排查；语义基线=legacy 规格扩大（1535d64b）：`*.x.com` 仅子域、`x.com` 裸域+子域 |
| wildcard 规则部分生效/特定形态误拦误放（部分标签星 `*.a.b.com`/尾星 `x.*`/SKIP-MORE 展开交错） | [../expert/historical-failures.md](../expert/historical-failures.md)「wildcard 四连修」条目（2026-11-17：leadingFragment 求值顺序 + SKIP/MORE 展开边互撞 + 孤儿 Emit 过滤） | `bpf_netfirewall.h` 的 `BuildPattern`（派生判定求值点必须在 `subdomainOnly` 擦除**之前**）/`InsertAt`/`MergeInto`/`CopySubtree`（展开节点吸收具体边不变量）/`Emit`（BFS 可达性过滤）；改动必跑 `AcScanSkipEdgeCollision001`（双插入序）/`AcScanMoreEdgeCollision001` 与主机 harness 四连扫；测试过滤器禁止 `c & 0x100` 位掩码（误捕 SKIP=0x301/MORE=0x300 伪类），用显式排除清单 |
| DNS 域名门双命中（deny 通配 × allow 精确同 qname 命中）裁决方向疑问/「冲突消解」类改动 | [../expert/historical-failures.md](../expert/historical-failures.md)「双命中裁决反转事故：defaultactionrst 枚举极性误读」条目（2026-11-17 定案） | 裁决已定版且与 IP 位图链同语义：**跟随默认策略方向**（默认拒绝→显式 deny 赢；默认放行→显式 allow 赢），实现位 `netfirewall_domain.h` parse_dns_query/parse_dns_response 两处分支 + `netfirewall_match.h` match_action 的 Conflict & Repetition Algorithm（两处刻意同版）；**极性陷阱**：`defaultactionrst` 是 `MatchDefaultActionMap()` 透传的原始 `sk_action` 枚举（SK_DROP=0/SK_PASS=1，1=默认**放行**），不是布尔；改裁决前先跑锁位用例 `DnsSniffDoubleHitDefaultAllow001`/`DnsSniffDoubleHitDefaultDeny001`/`DnsSniffDoubleHitResponseFollowsDefault001`（`AcDnsEnvConflict` baidu 形态）与 IP 位图链 `MatchActionConflictMatrix001`（默认两态 × 三命中形态六行），现状绿即勿动分支；位图断言注意 `bitmap_and` 原地改写 `key->val`——一行一新鲜 key |
| 部署本地构建 .so 后目标服务 dlopen 失败（`MUSL-LDSO: mmap fix failed ... errno=13` EACCES，且旧内容同 inode 可加载） | [../expert/historical-failures.md](../expert/historical-failures.md)「开发设备 XPM 执行保护 EACCES」条目（2026-11-17 定案） | 非 .so 缺陷：HM 内核 XPM（`/proc/sys/kernel/xpm/xpm_mode`，产品=5）拒绝对系统服务进程映射未签名本地构建。root `echo 1 > /proc/sys/kernel/xpm/xpm_mode` 后重启目标进程；注意 key_enable 服务开机会重置为 5（重启后需重降级或恢复镜像原件）；ninja 直编 .so 与 CBuild 产物字节一致、无需重走 CBuild；诊断快路径：自制 mtest 在 shell 上下文逐段 mmap 新旧文件对照（shell 上下文不受该检查影响） |
| DT 模块报 unavailable（非用例失败）/ 设备跑 UT 后全网断链 | [../expert/historical-failures.md](../expert/historical-failures.md)「DnsManager 测试 SIGABRT」+「DT UT 污染共享验证设备」条目 | 单用例 terminate（`enable_shared_from_this` 栈构造）会杀整个模块二进制，先搜模块 log；`netsys_netfirewall_test`/`netsys_native_manager_test` 真实加载 BPF 并写内核态，只在专属设备跑、跑后须重启恢复 |
| 防火墙死代码/死链清理（重写替换后的残留：孤儿函数/死 map/用户空间写链/死宏/过期 stub） | [../expert/historical-failures.md](../expert/historical-failures.md)「替换式重写半拆链」条目（2026-11-16 清理定案） | 动刀前先全仓 grep 每个符号的**读/写/测试 stub 三类端点**（BPF 读者死了≠用户空间写链死了≠测试 stub 死了）；观测字段删除前核对 innerkits 终点（`InterceptRecord.domain` 是 parcel 契约）；删 map 后改 `netfirewall*.h` 必 `rm netsys.o`；fuzz/UT stub 分支与契约注释同版更新；GetDomainHashKey 保留=AC builder 差分基线，勿删 |
| 防火墙 UT 套件改造/新写用例（netsys_netfirewall_test 两 TU） | [../expert/historical-failures.md](../expert/historical-failures.md)「类静态开关 × 条件断言 = 顺序耦合假失败」条目（2026-11-16 改造定案） | 隔离契约在 `test/.../netsys_netfirewall_test/firewall_test_env.h`：用例一律 `NewFirewallInstance()`（例外=DnsPrewarm 两用例，预热引擎硬绑单例）；开关只经 `FirewallLoadScope` RAII；**禁止条件断言**（map 前置由夹具 LoadElf 保证）；类静态 `isBpfLoaded_` 构造即重置；残留由 TearDownTestCase 清扫兜底；TCP 面报文用 `TcpHeaderDns`（带 doff 字节）勿用 `TcpHeader` |
| 流量统计与限额通知 | [../code_map/policy-stats-bpf-map.md](../code_map/policy-stats-bpf-map.md) 链②③ | `services/netstatsmanager/src/net_stats_service.cpp`（2647 行）、`net_stats_cached.cpp`、`services/netstatslimitntf`；参考 9ee4ebbf 教训 |
| NETSYSNATIVE 内核通道（路由/带宽/接口/共享） | [../code_map/netsys-native-map.md](../code_map/netsys-native-map.md) | `services/netmanagernative/src/manager/*.cpp`；base 侧只能经 `services/netsyscontroller` 调用 |
| BPF 数据面（map/程序/ring buffer） | [../code_map/policy-stats-bpf-map.md](../code_map/policy-stats-bpf-map.md) §4-§5 | `bpf/bpf_progs/`、`services/netmanagernative/bpf/`；先看 c3d1d423 越界读教训 |
| BPF 程序加载失败（真机 EACCES/E2BIG，`LoadElf is N≠0`） | [../expert/historical-failures.md](../expert/historical-failures.md) 两条 verifier 拒载条目（ctx 派生指针 CSE→EACCES；复杂度上限→E2BIG） | 先拿 verifier 原文再定位：仓内 `bpf_loader.cpp` 的 `LoadProgWithVerifierLog` 失败时自动带日志重试，真机 hilog 直接可见拒绝原因；errno 语义：EACCES=构造类拒绝（查 ctx 派生指针物化+溢出重载），**E2BIG+数百 ms 验证时长=复杂度上限 100 万**（查数据依赖步长的指针游走循环）；改 BPF 头后必 `rm netsys.o` 强制重编 |
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
| 防火墙 BPF 数据面/域名门/DNS_SERVER map | `test/netmanagernative/unittest/netsys_netfirewall_test`（netsys_netfirewall_test）+ `test/fuzztest/netsysnetfirewall_fuzzer`；fw 开/关两态编译（`netmanager_base_enable_feature_net_firewall`，fw-off 时 `netsys_netfirewall_test`/`suffix_match_trie_test` 等防火墙目标在构建图中缺席为预期），换 flag 走 CBuild build_system.sh 全流程 |

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
