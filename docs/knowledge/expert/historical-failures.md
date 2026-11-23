# 历史高危修复案例（Historical Failures & Lessons）

> 本文件是 netmanager_base 仓已核实的失效模式权威台账：每条含现象/根因/修复文件/验证要点，供改动前"对号入座"与排查时反查。新失效必须按此模板回写本文件（AGENTS.md §8），并同步 [task-routing.md](../routing/task-routing.md)。

## 案例台账（34 条：14 条经 git 历史核实 + 20 条 2026-11 fw 战役实测回填）

### 死锁-事件管理器两阶段锁冲突（a98cd919，2025-01-13）

- 现象：钉钉 Top1 问题——事件密集时进程卡死。
- 根因：`event_manager.cpp` 回调分发与监听器注册/注销使用两阶段锁，锁获取顺序冲突形成死锁环。
- 修复文件：`event_manager.cpp`
- 验证要点：高频注册/注销监听 × 事件爆发压测；改动事件分发锁作用域必须在并行销毁监听场景下重验。
- 教训：**回调分发不得持有服务侧长锁**；锁顺序全局约定见 [concurrency-failure-modes.md](concurrency-failure-modes.md)。

### 死锁-流量缓存查询概率死锁（51129900，2025-12-08）

- 现象：DTS2025120122477，应用查询流量统计时概率性阻塞。
- 根因：`net_stats_cached.cpp` 缓存查询与后台刷写锁粒度交叉，特定时序下互等。
- 修复文件：`net_stats_cached.cpp`
- 验证要点：`net_stats_cached_service_test` 全量 + 查询 × 刷写并发压测（统计类改动必带并发场景）。

### 竞态-多线程 netSuppliers 容器损坏（d0fda269，2025-02-16）

- 现象：网络注册并发场景崩溃，map 结构损坏。
- 根因：`net_conn_service.cpp` 的 `netSuppliers` 容器被多个客户端线程并发读写，无锁保护。
- 修复文件：`net_conn_service.cpp`
- 验证要点：修复引入 ffrt rwlock；并发注册/注销 supplier 压测 + `net_conn_service_test` 回归。
- 教训：服务端容器跨线程访问必须显式加锁（读多写少用 `ffrt::shared_mutex` 类读写锁）。

### 竞态-napi 模块多线程 map 崩溃（044d5d70，2025-01-24）

- 现象：`frameworks/js/napi/network/` 回调路径偶发崩溃。
- 根因：`network_exec.cpp` 内部 map 被工作线程与 JS 线程并发访问。
- 修复文件：`network_exec.cpp`
- 验证要点：`netmanager_utils_test` + napi 并发调用压测。

### 内存序-probe 线程 curl 对象崩溃（2103f660，2024-12-27）

- 现象：网络探测线程偶发崩溃于 curl 调用。
- 根因：`probe_thread.cpp` 跨线程共享的标志位/指针对象在无内存序保证下读写，工作线程读到半初始化对象。
- 修复文件：`probe_thread.cpp`
- 验证要点：探测启动 × 停止 × 网络切换循环压测。
- 教训：跨线程共享状态一律 `std::atomic`（配显式 memory order）或加锁，❌ 裸 bool/裸指针。

### SA 崩溃恢复-状态残留（8c973dfb，2026-01-16）

- 现象：netmanager 进程崩溃自动重启后，网络状态错乱（supplier 丢失/重复）。
- 根因：SA 重启后客户端侧 `net_conn_client.cpp`/`net_supplier.cpp` 未清理重建旧状态，残留脏数据继续参与评选。
- 修复文件：`net_conn_client.cpp` + `net_supplier.cpp`
- 验证要点：杀 netmanager 进程后自动恢复场景回归；supplier 重注册链路核对。
- 教训：SA 重启=客户端必须假定全部远端状态已丢失，走重建而非续用。

### 空指针-netsys 代理调用未判空（63961c40，2026-02-05）

- 现象：调用 netsysnative 服务偶发空指针崩溃。
- 根因：`netsys_native_service_proxy.cpp` 中 IPC 返回对象/回执读取链路上多处未判空即解引用。
- 修复文件：`netsys_native_service_proxy.cpp`（约 278 行判空加固重构）
- 验证要点：`netsys_native_manager_test` + 异常回执注入；proxy 类新增方法默认输出判空审查。

### 越界读-BPF 防火墙环形缓冲（c3d1d423，2025-10-25）

- 现象：防火墙拦截日志读取越界，内核侧报错/数据错乱。
- 根因：`bpf_netfirewall.cpp` 从 ring buffer 读记录时长度未按实际有效数据截断，`bpf_ring_buffer.cpp` 边界检查不足。
- 修复文件：`bpf_netfirewall.cpp` + `bpf_ring_buffer.cpp`
- 验证要点：`netsysnetfirewall_fuzzer` + `nfqueue_fuzzer` 全量；恶意/畸形长度注入用例。
- 教训：数据面"先校验长度，再读数据"——边界检查必须在使用数据之前。

### 泄漏-napi 异步资源未释放（f2e10ae5，2026-04-13）

- 现象：DTS2026032818910，长期运行内存持续增长。
- 根因：`base_async_work.h` 异步工作入队失败/取消分支未释放上下文与 async_work 句柄。
- 修复文件：`base_async_work.h`
- 验证要点：入队失败注入用例；长稳内存曲线对比（泄漏类必带前后曲线）。
- 教训：异步申请资源的**每条失败路径**都必须显式走释放，对照 netstack 侧 dcfe2f33 同形态教训。

### 泄漏-fd 未关闭（1b8b67fb）

- 现象：长稳运行 fd 数量缓慢增长。
- 根因：异常分支文件描述符未关闭。
- 验证要点：长稳 fd 数量曲线对比；新增 fd 使用点接入 RAII/守护类。

### 统计丢失（9ee4ebbf，2026-08-27）

- 现象：部分场景流量统计数据缺失/清零异常。
- 根因：统计链路（BPF→bpf_stats→NetStatsService→net_stats_cached）时序问题。
- 验证要点：`netsys_bpf_stats_test` + `net_stats_cached_service_test`；统计口径前后对比。

### 规范-硬编码 domain（2ae80b5e）

- 现象：hilog/上报 domain 硬编码散落，违反观测规范。
- 教训：观测常量（domain/事件名）统一定义处取值，`hisysevent.yaml` 与公共头文件为唯一来源。

### race 警告清理（080a440c）

- 现象：静态/编译期检测出数据竞争警告点。
- 教训：**race 警告即隐患**，不允许以"暂时没事"搁置；发现即修并补并发用例。

### defaultNetSupplier 锁缺失（ea5291d7）

- 现象：DefaultNet 评选路径偶发异常。
- 根因：`defaultNetSupplier` 相关读写未完整纳入锁保护。
- 验证要点：供应商分数并发更新 × 评选触发压测。

### 测试代码失效模式——fuzz 用例三大陷阱（2026-10-28 fuzz 治理发现，测试代码同样回写）

- **parcel copy-paste 陷阱（12 高危 + 5 低危，`net_conn_client_fuzzer.cpp`）**：缺参变体的 `StaticXxxProcess` 调用复制全参调用后只改变量名未改实参（传了 `dataParcelWithAllParam`）→ 缺参 parcel 连 interface token 都没有 → stub 侧 descriptor mismatch 早退，**用例表面执行实际零覆盖**；反向形态是缺参 parcel 内误写全参（`dataParcel.WriteString` 而非 `dataParcelNoXxx.WriteString`）退化为重复调用。规避：写完变体后逐个核对实参变量名与 parcel 名一致；评审时重点看 `No*/WithOut*` 变体的实参。
- **GetData 无符号下溢（fuzz 工具代码）**：`size > g_fuzzSize - g_fuzzPos` 当 `g_fuzzPos > g_fuzzSize` 时下溢成大数绕过检查 → OOB 读。规避：先判 `g_fuzzPos >= g_fuzzSize` 再做减法（同款问题同时修了 netconnclient/netbasebranch/isdeadflow 范本三处）。
- **VLA 负索引写（fuzz 工具代码）**：`char cstr[strlen]; cstr[strlen - 1] = '\0';` 在 `strlen <= 0` 时负索引越界写。规避：改 `std::array<char, STR_LEN>` + 参数边界检查；fuzz 工具函数入口必须自证输入合法。
- 关联教训：0 命中接口的三类根因——①fuzzer 未链权限 mock（conn stub 每命令权限表必失败）；②stub 内联 uid 硬校验（UID_COLLABORATION/UID_RSS/uids_ 白名单）；③parcel 写错导致 token 缺失。前两类已用 link-mock（`common_mock_netmanager_permission.cpp`）+ fuzz 专属宏 `NETMANAGER_FUZZ_TEST_ENABLE`（仅 fuzz target defines，`#ifndef` 包裹原逻辑，产品零影响）放通。

### BPF loader 同 boot 重载 split-brain（2026-11-14 U5 检视发现，存量隐患未触发）

- 现象（潜在）：`bpf_loader.cpp:523-544` `CreateMaps` 在 pin 已存在时保留**新建** fd 供程序重定位、旧 pin 原样留给用户态——同一次 boot 内不经 `UnloadElf` 的二次 `Load` 会造成 BPF 程序用新匿名 map、用户态（BpfMapper）开旧 pin 的分裂。
- 触发条件：fw-on 镜像下 netsysnative 服务重启且不走 UnloadElf 路径（服务崩溃恢复后重载）。OTA（重启）路径无此问题（bpffs 清空重建）。
- 后果：规则写入/老化 sweep 落到旧 map，BPF 数据面读新 map → 规则"静默不生效"类疑难。
- 关联：U5 的 domain_value 8→24 字节升级依赖"OTA=重启=bpffs 清空"路径，不受影响；`DomainCacheAgingSweep` 读 map 失败已有 -1 容忍（与 M10 基线一致）。
- 处置建议：loader 重载路径补"pin 已存在时优先复用 pin fd 或先 unpin"的统一策略；改动前先读 `docs/knowledge/code_map/netsys-native-map.md`。

### BPF 数据面工具链与测试失效模式（2026-11-14 防火墙修复 U4-U7 实测，四条全是踩过的坑）

- **改 BPF 头必 rm netsys.o**：`ohos_bpf("netsys")` 是 ACTION 步骤，ninja 对其**头文件依赖追踪不完整**——改动 `bpf/include/netfirewall/*.h` 后 netsys.o 可能不重编，旧宏语义的程序继续被链接部署。铁律：触碰任何 BPF 头后 `rm -f out/.../services/netmanagernative/bpf/netsys.o` 强制重编（U4/U7 各炸过）。
- **C++ TU 编 BPF 头双陷阱**：①测试/fuzz TU 的 `bpf_map_lookup_elem` 桩返回 `StubBpfValue`（**无比较运算符**）——`lookup(...) != NULL` 编译失败，必须先赋类型化指针再比较（BPF 编译语义逐位不变）；②musl BPF 构建为兼容旧内核 UAPI 有 `#define source uh_sport` 类 shim——同 TU 内 `tcph.source` 会被宏**误伤替换**，TCP 头解析须读原始字节 `tcph_raw[20]` 再 memcpy。
- **BPF 端口语义铁律（字节序错位，真机失败案例 U7）**：端口比较/记录 key 一律用"结构体内存装载值"（`udph.source` 成员直读或 `__builtin_memcpy` 直拷），比较用 `bpf_ntohs`；**禁用大端重造算式** `((b0<<8)|b1)`——它在 LE 机上与结构体装载值互为字节交换，pending 记录 key 直接错位（`DnsSniffTcpQueryStagesRecord001` 首轮真机失败根因：key.sport=49152 vs 期望 192）。
- **测试单例状态污染（M10 真机）**：`NetsysBpfNetFirewall` 进程内单例的 `isBpfLoaded_` 被先前测试置位——"not loaded 路径"测试若写 `if (IsBpfLoaded())` 条件断言会在单例被污染时走入 loaded 分支而失败。正确形态：`SetBpfLoaded(false) → 断言 ERR → SetBpfLoaded(true) restore`（`SetEvalFallbackMode001` 家族模式）。

### BPF verifier 拒载 ctx 派生指针溢出重载（真机 EACCES，2026-11-14 M10 netsys.o 加载失败根因）

- **现象**：fw-on `netsys.o` 在真机（8425C，Linux 6.6.127-oh）加载失败 EACCES(13)——第一个 prog `cgroup_skb_uid_ingress` 即被拒 → `SetBpfLoaded(false)` → 防火墙整体未挂载（ping 直通）；fw-off 小 ELF（347 insns）同机正常。非权限/SELinux 问题（无 avc，loader 权限链完整）。
- **根因**（变体交叉验证锁定）：DNS 嗅探链多处内联点读 `skb->family`（ctx 偏移 88），clang -O2 将 `&skb->family` 的 GEP 地址 CSE 为单值，大 prog（ingress 5453 insns）寄存器压力下物化 `r += 88` 并把**修改后的 ctx 指针**溢出栈槽，重载后解引用——verifier 拒绝 "dereference of a modified ctx ptr" → EACCES（126 个指令级拒绝点全部同源）。无该构造的变体全部加载成功，有则全部 EACCES；真 ELF 反汇编 insn 14 `r9 += 88` 实证。
- **volatile 无效（真机实测否决）**：volatile 只挡 load 合并；GEP 是纯指令，地址 CSE 照样发生。
- **修复（family 单读+下传）**：`skb->family` 每程序入口只读一次（偏移折叠进 LDX 立即数，永不物化 ctx+N 指针），值作 `__u32 family` 标量沿调用链下传（标量可自由溢出）；`is_ipv4_format_skb(skb)` → `is_ipv4_format(family, skb)`，不留旧签名 wrapper；netsys.c 两个 prog 入口经 `skb_family()` 单读，stats 帮助函数与防火墙链全部改为首参收 family；测试/fuzz 调用点同步。
- **规避要点**：BPF C 代码**禁止多点直读同一 ctx 字段**（family/len 等热字段尤甚）——凡多处消费的 ctx 字段，程序入口单读一次+标量下传；改后 ELF 审计必查 `r? += <const>`（基址=ctx）物化+溢出重载解引用模式是否归零。
- **配套诊断（永久保留）**：`bpf_loader.cpp` 增 `LoadProgWithVerifierLog`——加载失败带 1MB verifier log 重试并分块打印尾部 16KB，保留原 errno（此前 EACCES 无任何 verifier 明细，只能盲猜）。

### BPF verifier 复杂度上限拒绝（真机 E2BIG，2026-11-14 fw-on netsys.o 构造 B 根因，紧随构造 A 之后暴露）

- **现象**：构造 A 修复后，fw-on `netsys.o` 加载在 `cgroup_skb_uid_ingress` 返回 E2BIG(7)，verifier 实测跑 440~680ms 后拒绝；egress 同样失败。fw-off 小 ELF（350 insns）同机加载成功（仅 901 insn states / 77 states / max_states_per_insn=2），证明内核与加载链路无恙。
- **根因**（verifier 日志原文定案，非推断）：`BPF program is too large. Processed 1000001 insn`——`BPF_COMPLEXITY_LIMIT_INSNS`（100 万，verifier.c:17145）。DNS 域名 key 构建代码（`netfirewall_domain.h` 域名游走）含**变步长标签游走**（步长=包内字节 `r1 += <label_len>`，0~63 数据依赖）+ 74 字节倒序交换循环：每轮迭代指针位置互异 → verifier state 天然互异 → 剪枝完全失效（`max_states_per_insn=113`），ingress/egress 双双烧满 100 万预算。拒绝现场反汇编对位 insns 650–682（ingress）/ 3145–3154（egress）；该"游走+交换"三联回边结构在 ingress 内联出现 3 次（queries/answers 等不同调用点）。
- **关键鉴别（勿再走弯路）**：E2BIG + **半秒级验证时长** = verifier 在逐 state 模拟循环、预算耗尽——**不是**"内核不支持多层变界循环"（同内核有界循环全支持；循环不支持类拒绝走 EINVAL/EACCES 且瞬时返回）。他人分析报告据此推"内核需补验证能力"是错误结论（死路）；正确路径 = 仓内压缩状态空间。
- **获取 verifier 原文的方法（长期入口）**：仓内 `bpf_loader.cpp` 的 `LoadProgWithVerifierLog`（构造 A 条目同款，随修复提交）——加载失败自动带 1MB log_buf 重试并打印日志尾部，真机 hilog 直接可见拒绝原因，不再盲猜 errno。如需离线秒级验证新 ELF（不部署 /system、不重启 netsysnative），可临时自建 freestanding 探针直调 `BPF_PROG_LOAD`（解析 ELF + 按生产参数建 map + 打 relocation + log_buf），方法同上，工具不随仓分发。
- **修复**（2026-11-15 真机全链路验证结案；**其中"域名 deny 生效"一句已废弃——见下一条目：该功能验证系假阳性，AC verdict 实被 name_len 腐蚀 bug 吞掉**）：多管齐下组合，全部在仓内、零内核改动——① **P2'' 双挂载拆分**：DNS 链（~13.5k insns）与策略链（174k ingress / 253k egress）拆成两个独立 prog，`BPF_F_ALLOW_MULTI` 挂同一 cgroup（dns 先挂），裁决 = 多 prog AND，gate 经 `DNS_GATE_SCRATCH` percpu map 跨 prog 传递——各自独立 verifier 预算替代共享 1M；② **AC 自动机重写（A'）**：域名匹配改锚定 Aho-Corasick（前缀/后缀双 trie、类编码 LEN/CHAR/TERM 消歧、rem 经 scratch 洗值防 tnum 平移），scan 状态空间从 >1M 压到 13.5k 级；③ **loader 死裁剪合并**（`MergeTextInsns`）：对每个 prog 按调用闭包死裁剪 `.text`（dns prog kept 5/7 函数、main kept 2/7），把保留函数追加进 prog 段并重定基伪调用（`bpf_loader.cpp` DISP32 skip + attach 表扩展）。真机实证：4 个 fw prog 全部 BPF_PROG_LOAD 成功 + 双挂载生产 cgroup，零 Invalid relo；功能级 def_act 翻转（curl 000 + ping sendto EPERM → allow 恢复）与域名 deny（2/3-label 域名解析被拒、对照放行）全部生效。
- **遗留开放项（2026-11-15 立案，未结）**：AC 转移表（`BPF_F_NO_PREALLOC` hash）批量写在 HongMeng Kernel 1.15 上**间歇 ENOMEM(12)**——非 rlimit/memcg（均 ∞），实测阈值 ~87 条且随 map 存量浮动，对照组 PREALLOC map 200 条零失败；HM 内核对 NO_PREALLOC 插入有专有记账预算。单域名规则表 13~16 条重试可过，**大规则集有生产风险**；须评估生产最大规则集条数与该预算的关系（必要时改用 PREALLOC 或分批+重试策略）。
- **规避要点**：BPF verifier 的 100 万 insn-state 预算是稀缺资源——**数据依赖步长的指针游走循环每轮必产生新状态**（剪枝对互异指针位置无效）；逐字节/逐标签处理循环须设计为"固定上界+迭代间状态可剪枝"（常量步长/标量计数器）；改后从 `LoadProgWithVerifierLog` 日志的 `processed N insns (limit 1000000)` 一行看余量。

### BPF 扫描间暂存契约被腐蚀：AC verdict 整体丢失（2026-11-15 DT 复测暴露 + 干净真机实验实锤）

- **现象**：A' AC 改版后 DT `netsys_netfirewall_test` 28 个用例失败（Ac 扫描 8 个、DNS 链 10 个、环境类 ~14 个）；真机功能验证"域名 deny 拦截"曾判通过。**该"通过"是假阳性**：干净重验（服务重启清空 DNS 代理缓存 + 全新鲜域名 + 显式 defact allow/allow + acdump 证实 trie 正确在 map）证明修复前 deny 不拦（curl 200）——旧结论被 GFW（wikipedia exit 28，中国网络）与 DNS 代理缓存/当时状态混淆。
- **根因**（host 复现器直接执行真实扫描代码实证）：`ac_suffix_scan`（`netfirewall_domain.h`）借 `key->prefixlen` 做循环内 idx 洗钱暂存（verifier 要求），但**退出不恢复 staged name_len**。生产调用方四连扫（prefixExact→prefixWild→suffixExact→suffixWild，查询/响应路径同构）中：①suffixExact 死路（该 appuid 无规则）在 peel 处写 `name_len-1` 后 return → suffixWild 读脏长度、错误锚点死走 → **wild 后缀规则对 appuid≠0 永不匹配**；②suffixExact 完整走留 0xFFFF → suffixWild 越界返回 AC_PARSE_FAIL(0xFFFFFFFF) → `combined==AC_PARSE_FAIL` → **整个 verdict（含已命中的 deny/allow flag）被 OR 吞掉**；③parse_dns_response 四扫后再读 prefixlen 计算 answers 偏移 → 读脏值 bail → **DNS 学习全断**。
- **修复**：仅在 2 个非 FAIL 退出点恢复 staged name_len（suffix TERM-miss `return 0` + 循环出口）；AC_PARSE_FAIL 路径不恢复（任何 FAIL 使 combined=全一，调用方在读 name_len 前就 bail，恢复无意义且烧 verifier 预算）。**verifier 预算红线实录**：5 恢复点版本使 egress prog E2BIG（processed 1,000,001/1,000,000——只超 1 条 insn），收敛 2 点后通过。修复后真机重验：deny example.com 真拦（curl exit 6），对照新鲜域名正常解析——**首次真实的 AC deny 功能验证**。
- **连带教训（三条）**：①"扫描间暂存契约"类共享 scratch 字段（此处 prefixlen 双职：staged 长度 + 洗钱暂存）任何退出路径都必须保持契约值；②功能验证须消除环境混淆（服务重启清 DNS 代理缓存、全新未查询域名、显式默认动作、避免 GFW 域名做样本、对照域名须新鲜）；③测试与产品同仓演进时，机制替换（legacy PASS map → AC trie）必须同步迁移既有测试的 stub 建模——本次 8 个 HEAD 既有 DNS 测试按旧机制建模全部误报失败（语义在真实系统等价，纯 stub 错配）。
- **工具链教训**：host 复现器（include 同一 BPF 头 + stub map 表）能在秒级判定"内核行为 vs 静态分析"分歧，先复现再改产品代码；测试侧 helper 也有 bug 史——AcDecodePattern 解码序与 builder 发射序相反、容量测试 LCG 有周期 26 碰撞（2200 规则塌缩 26 种），测试助手本身需要差分验证。

### 测试设备无法给系统服务换载未签名 so（XPM 代码签名，2026-11-15 部署调查定案）

- **现象**：真机验证新编译 `libnetsys_bpf_utils.z.so` 时，三种部署向量全部失败：① overlay 写 `/system` → EACCES；② `LD_LIBRARY_PATH` 对系统服务无效（OHOS musl namespace ini 解析成功时 `.path` 完全覆盖，ini 前置目录即全量替换搜索路径）；③ ini 前置 `/data/local/tmp` + `setenforce 0` → `Xpm check failed errno 13`，且 musl 对"找到但加载失败的 so"整体 abort（不落下一搜索目录）→ 依赖它的 SA（1158 netsysnative）连带 down。
- **根因**：HM 内核 XPM=代码签名机制（dm-verity 校验 exec-mmap），**系统服务结构性无法加载未签名 so**；shell/debug 进程在 /data/local/tmp 有豁免。生产设备 so 走镜像签名无此问题——纯测试向量限制。
- **正确验证姿势（长期入口）**：root 侧 harness 二进制（从 `/data/local/tmp` 运行，`LD_LIBRARY_PATH=/data/local/tmp/libs` 指向新 so）+ `nsenter -t <netsysnative_pid> -m` 进服务 mount ns（拿 bpffs/cgroup2 视图），即可在真机以生产上下文驱动新 so 的 `LoadElf/UnloadElf` 与防火墙 API。XPM 对 shell 豁免不受 nsenter 影响（换的只是 mount ns）。
- **连带陷阱**：①独立进程（未经 `LoadElf` 第 7 步 set rlimit 的进程）写 BPF map 时须自行 `setrlimit(RLIMIT_MEMLOCK)` 对齐生产进程形态；②`UnloadElf` 会拔 `/sys/fs/bpf/netsys` 全部 pins 并 detach 全部已挂 progs（含服务的）——用后须 kill netsysnative 重启自愈回基线；③拔/重 pin 后 pin 属主变化，服务进程对 stats map pin 的写会被 transfs/HMAC 拒绝（`inode permission failed`），同样靠服务重启恢复。

### 运行中系统服务的已映射 so 不可就地 cp 替换（2026-11-15 两次服务 SIGSEGV 实锤）

- **现象**：真机往 `/system/lib64/platformsdk/` cp 新编译 `libnetsys_bpf_utils.z.so` 的瞬间，运行中的 netsysnative 立即 cppcrash（07:08 pid 40599、10:25 pid 48860，故障日志 `FAULT-BETA-RELIABILITY-CPP_CRASH-*-20261115102528` 已归档）。
- **根因**：服务对旧 so 的文件映射仍按页缺页加载；`cp` 截断+改写同一 inode 后，后续缺页在旧偏移读到新文件内容 → 指令错位 → SIGSEGV/SIGBUS。overlayfs copy-up 与直接覆写均如此。
- **正确姿势**：换系统 so 必须 **先 kill 服务再推文件**（重启后按新 so 干净加载），或先推到临时名再 `mv` 原子替换 + 立即重启服务（mv 换 inode 时旧映射仍指向旧 inode，相对安全但服务仍须重启才能用新 so）。绝不允许"服务运行中直接 cp 覆盖其已映射 so"。
- **云真机环境教训（2026-11-15 设备被平台回收实录）**：①共享测试设备 reboot 前必读 `/dev/block/by-name/misc` 的 BCB——存在 `boot_*` 命令说明平台维护/OTA 流程 pending，reboot 会落进 updater ramdisk（本例 `boot_eupdater`，且该 ramdisk 每次启动把 BCB 写回自锁，清零无效）；②长时 fuzz/UT 高负载可触发 POWERTHERMAL 异常 → WiFi 掉线/平台接管——高负载实验后先验 `cat /proc/net/wireless`（status 非 0000 才算关联）再继续结论性实验；③WiFi disassociated 时 wlan0 的 IP/路由是陈旧残留，勿据其判断"网络配置完好"。

### P4 ct 按方向独立评估回归：非回环回程包被入向默认动作误杀（2026-11-15 真机 hilog 断链定位）

- **现象**：默认入向拒绝（in=1）+ 仅出向 IP 放行规则（RULE_OUT）下，浏览器 preconnect 到放行 IP 的 TCP 连接全部失败：出向 SYN 已按 OUT 规则放行（无 egress intercept 日志），但回程 SYN-ACK 被 `ingress intercept` + SOCK_DESTROY（`send_sock_tcp_reset`）逐个杀掉，三次握手永不完成 → Chrome 重试循环 → `net_error -103`（CONNECTION_ABORTED）连发、页面超时。**波及面**：所有「入向默认拒绝 + 出向放行规则」配置（家长控制/企业管控最常见形态）下任何 TCP 连接都无法建立；UDP 无握手不受此形态影响但回程同样被杀。
- **根因**：P4（commit 3c243efc）把「ct 命中即放行」改为「本方向已评估才放行」（`netfirewall.h` 两 hook 的 ct 早退分支）时，未为非回环连接的回程包保留对向裁决继承。断链五步：①出向 SYN 过 OUT 规则 → `ct_create_entry(CT_EGRESS)` 只置 `egress_evaluated=1`；②回程 SYN-ACK ct 命中但 `ingress_evaluated=0` → 落 INGRESS 全量评估；③仅 OUT 规则时 `SetFirewallIpRules` 对空 inRules 不写 IN 方向 map，IN 维度 map + in_wildcard_map 全空；④`lookup_dim_map` 的 ZERO_BITMAP 兜底把每个维度清零 → 无规则命中；⑤落默认动作 `DEFAULT_ACTION_MAP[uid].inaction=DROP` → `log_intercept` + `send_sock_tcp_reset`。老代码「ct 命中即放行」的有状态回程豁免语义被移除且无继承通道。TC-E（deny-all + IP 规则）当时被测试工具缺陷 D3 阻塞未真机验证，为暴露盲区。
- **修复**：两 hook 的 ct 早退分支增加「**非回环** ct + 对向已评估 → PASS」（有状态防火墙回程豁免恢复）；回环 ct 不继承（P4 跨空间回环隔离目标保留——接收方 inaction 仍按方向独立评估）。ct 键经 `get_match_tuple` 的 EGRESS 交换归一为「远端为源」朝向，两方向同键。
- **验证要点**：deny-all(in) + 仅 OUT IP 放行 → 访问该 IP 可通；纯入向首包（无 ct）仍拦；**跨空间回环仍按接收方策略评估**（防回归倒退）；单测补 ct 双向首包 × 仅单方向规则 × 回环/非回环矩阵；真机 `bpftool map dump` ct_map flags 字节（bit3 egress_evaluated / bit4 ingress_evaluated）确认中间态。
- **教训**：**对称性语义收紧行为变更必须过「对向流量」象限矩阵**——按方向独立评估的回归面在「本方向规则 × 对向包」组合空间，不在单方向内部；报告自证的「零回归红线」若无该象限用例即为空承诺。

### AC 构建器拒收前导 `*.` 模糊模式：fuzzy 域名规则永不生效（2026-11-15 真机 hilog 实锤）

- **现象**：fuzzy 域名规则 `*.baidu.com`（ALLOW 或 DENY）完全不生效：默认拒绝 + fuzzy ALLOW 下子域解析被拦（浏览器 `ERR_NAME_NOT_RESOLVED`）；默认允许 + fuzzy DENY 下不拦。日志铁证 `BuildAcDomainTables: skip invalid domain: ... domain=*.baidu.com`。附带次生缺陷：DNS 预热引擎把 `*.baidu.com` 当字面域名解析（上游垃圾查询 + `RunDnsPrewarmTask: resolve failed` 噪声循环）。
- **根因**：A' AC 改版的 `BuildPattern` 沿用 legacy `GetDomainHashKey` 的「剥所有 `*`」，但 legacy 用反转串 LPM——前导 `.` 恰好成为子域锚（`*.baidu.com` → 反转 `"moc.udiab."` 匹配子域、不匹配裸域，见规格扩大 commit 1535d64b）；AC 版剥 `*` 后 `.baidu.com` 首 label 为空 → 被判非法整条跳过 → fuzzy 规则从 AC 表静默消失（无 fuzzy 规则时 DENY/ALLOW 都形同虚设）。预热注册表过滤只挡尾部 `*`（前缀规则不可枚举），前导 `*` 混入待解析列表。
- **修复**：`BuildPattern` 识别前导 `*`（suffix 规则）→ 剥 `*.`、以 MORE 伪类（匹配任意单 class，编码「至少还有一个前置 label」——label 必有 ≥1 字符，故 LEN 后 ≥1 class ⟺ ≥1 前置 label）结尾；AC trie 为纯锚定下降无 fail 链，MORE 在 `Emit` 时展开为全 class 转移行（LEN 1..63 + CHAR 0x255 ≈ 320 行/规则，`AC_TRANS_MAX_ENTRIES=262144` 预算充足），**BPF 扫描侧零改动、零 verifier 预算影响**。语义对齐 legacy：`*.baidu.com` 仅匹配子域、`baidu.com`（无星）匹配裸域+子域。预热过滤改为「含 `*` 即跳过」。
- **验证要点**：`*.example.com` 构建成功、扫描层匹配 `a.example.com`/不匹配 `example.com`；`example.com`（fuzzy 无星）匹配裸域+子域；差分测试 `agreed` 列表补 `*.example.com`（原列表缺失即漏测盲区）；真机 fuzzy DENY/ALLOW 行为恢复出厂语义。
- **教训**：**机制替换的语义面必须逐形态 diff**——legacy 输入串里「看起来多余的字符」可能承载语义（前导点=子域锚）；builder 的 `skip invalid domain` 日志是「规则静默丢失」的前兆信号，出现即配置面与执行面脱节，必须当作缺陷排查而非噪声。

### C5 系统身份收拢过度豁免：root/系统应用流量逃逸用户规则（2026-11-15 用户规格质询定案）

- **现象**：默认允许 + 显式 DENY ICMP 规则下，root（su）ping 规则目标 IP 能通（战役提交 3c243efc 前该场景被拦）；deny-all 用户下系统应用（uid=appId）流量不受规则管。
- **根因**：`get_user_id`（`netfirewall_match.h`）把 `uid<200000` 全体收拢 `FIREWALL_SYSTEM_UID(1)` + 启动写 `DEFAULT_ACTION_MAP[1]=allow`。立论依据「无 socket 内核生成包（RST/ICMP error）被折算前台用户、default-deny 用户误杀内核控制包」**错误**：本内核无 socket/归属失败的 `bpf_get_socket_uid` 读数为 0xFFFFFFFF（P4 中孚实测），代入出厂代码 `0xFFFFFFFF/200000=21473` 幽灵用户**先于**前台折算分支返回——内核包出厂走幽灵放行，**从未**经过 `uid<200000` 折算分支。C5 改动唯一实际效果=豁免真实系统 socket（appId/root/shell/守护），违反「规则不区分系统应用与三方应用」规格（出厂 `CURRENT_UID_MAP` 前台折算，规则生效）。次生不一致：预热守护流量 BPF 侧学习条目 uid=1 永不匹配任何应用查询（uid=100）。
- **修复（F6）**：纯恢复出厂三路——`0xFFFFFFFF→UID_UNATTRIBUTED`（保留 P4 修复）、真实 `uid/200000>0→真实 userId`、其余真实 `uid<200000→CURRENT_UID_MAP` 前台折算（miss→DEFAULT_USER_ID=100，写入链 `SetFirewallCurrentUserId` 现成）；不引入任何新增豁免机制；`DEFAULT_ACTION_MAP[SYSTEM=1]` 空置保留治理预留 key；预热耦合读码确认无阻（守护查询折算前台后走逐域门四条件仍可过门）。
- **验证要点**：`GetUserIdFolding001`（折算三路矩阵）+ `SystemIdentityFoldsToForeground001`（hook 级 root socket 被前台 default-deny-out 拦）；真机 root ping 命中 deny ICMP 规则被拦=规格恢复；deny-all 下系统应用流量受管。
- **教训**：**「误杀向」修复必须先核实旧路径真实行为再定豁免面**——本例旧路径（幽灵 21473 放行）与声称行为（前台折算被拦）不符，豁免面从「内核包」扩大到「整个 uid<200000」没有受益者核对；改动群体分层（假 uid 哨兵 / 真实低 uid / 真实高 uid）与处置矩阵须在设计文档显式列出。

### 有状态控制面 API 多进程拼接场景：异步 DnsSrvSync 清掉他进程写入（2026-11-15 F4 门假失败定案，测试方法论失效模式）

- **现象**：真机验证域名门（deny-all 默认 + baidu.com allow 规则 + 受管 resolver 集齐备）时，放行域查询恒被 egress DROP，多轮窗口全灭、且门四条件（默认 DROP/端口 53/DNS_SERVER 命中/gate ctx）逐项排查全部"应满足"。曾误判为门/prog 挂载顺序缺陷。
- **根因（非产品缺陷）**：`SetFirewallDomainRules`（`bpf_netfirewall.cpp` ~line 641）与 `NotifyDnsRuleChanged`/`NotifyDnsDefaultActionChanged` 都会 `ffrt::submit(RunDnsServerSyncTask(userId))` **异步重同步**；该 sync 以**进程内 registry**（`dnsUserHasAllowDomain_`+`dnsUserDefaultDeny_`）判定 `DnsServerSyncNeeded`（= hasAllow && egress defaultDeny），不满足即 `ClearDnsServerMaps`。harness 每个 API 调用=**新进程**（registry 为空）：`dnssrv` 进程直写 7 条授权后，后续 `domrule` 进程的异步 sync 因自己 registry 缺 `defaultDeny` 判定"不需授权"→ **把前一进程刚写的条目全部清空** → 门条件③ `MatchDnsServer` 恒 miss → 放行域查询全灭。产品流（netsysnative 单长命进程）registry 连贯且 provider 自动 arm 基础 resolver 链，无此问题。
- **证据链（观察面锚定法）**：①`gateslot` percpu 残留 `{is_query=1, allowrst=1}` → egress_dns prog 在跑、AC 扫描判 allow、gate ctx 已暂存；②`pendingslot` 残留 `valid=0` 且 txid 匹配本窗口查询、去向 IP 全在受管集 → main prog 在 dns prog **之后**跑（挂载顺序正确）且 `flush(false)` 已执行；③`mapcount` 显示 `dnssrv` 后 dns_server_v4_map=7 条仍在。三者与"查询仍 DROP"的唯一自洽解释=窗口内异步任务清表。单进程 `fwsetup`（defact→domrule→sleep→dnssrv）复测：tieba.baidu.com 连通（服务端 403=已建链）、hm.baidu.com 200、未放行域 exit 6、domain_ipv4 学习 3 条——**门+学习链首次真机端到端实证通过**。
- **教训**：**有状态控制面 API（规则/默认动作/授权联动）的真机场景验证禁止多进程逐条调用拼接**——进程内 registry + 异步联动会把别的进程写的状态当陈旧数据清掉；场景搭建必须单进程完成（或最后一步直写不被 sync 触碰的表）。诊断时优先用 percpu scratch 残留/定向 mapcount 等**观察面锚定**，而非反复重跑黑盒窗口。

### 防火墙真机观测工具链六坑（2026-11-15 F4/F2 排查实录，全部踩过）

- **dumpfw 挂死**：`GetAllKeys` 全量遍历大 map（LPM/ct）会超时挂死并连带占住 harness 二进制（text file busy）；改用 `mapcount`（BPF_MAP_GET_NEXT_KEY 定向计数）+ `mapdump`（指定 map 的 key/value dump）。推新 harness 前先 `kill -9 $(pidof elfload_harness)`。
- **拦截日志字段是交换后 tuple**：`egress intercept: sport=53 dport=36398` 中 sport=53 是**原目的端口**（egress match_tuple 端口/地址双交换）——`grep 'dport=53'` 找查询方向永远扑空；ingress 侧不交换，方向语义相反。
- **真实 resolver 池 ≠ /etc/resolv.conf**：shell 工具 getaddrinfo 走 netsysnative resolver 配置；经验采集（allow 态多域解析 + pendingslot 残留并集）实测池 = **223.6.6.6（主力）、180.76.76.76、114.114.114.114**，resolv.conf 里的 114+8.8.8.8 不是实际全量。arm 授权集缺主力 server 时门同样恒关——先采池再 arm。
- **pendingslot 解码布局**：`dns_query_pending` = skb_addr@0(u64) + valid@8(u16) + **key@12**（对齐到 4：txid@12/sport@16/dport@18/saddr@20/daddr@24）+ uid@28 + appuid@32；value_size=64、percpu 读取 buffer 须 round_up(value,8)×nr_cpus。
- **GET_NEXT_KEY 全零 key anchor 盲区（2026-11-15 F2 排查补坑）**：遍历工具首查把全零 key 当"无前键"传入——HASH map 语义是"返回该 key 的**下一个**"，key 0（wildcard role 0）存在时被永久跳过，且遍历提前 break 会漏数——F2 期"out_wildcard_map 只有 1 条零值"假象曾误判为产品缺陷，显式 key 直查（`maplook`）证明全 8 role 条目和值**完全正确**。教训：**观测工具的遍历实现本身要被验证**（显式 key 交叉核对）才能下"产品缺陷"结论。
- **观测采样必须在被观测对象存活期间**（2026-11-15 F1 内存实验补坑）：mapcreate 旧版在进程退出后由 shell 采 MemFree——map 已随 fd 销毁，读到的是销毁后残值，据此得出"PREALLOC 惰性 ~200KB"**错误结论**；进程内分阶段采样（写 10 万→删→重写）+ 控制组（16384=1.6MB 线性、NO_PREALLOC=3.9MB 仅桶）才钉死真实语义=**建表全量预留 ~106B/条**。教训：生命周期类测量必须"对象存活期间采样 + 退出后对照 + 规模控制组"三件套。
- **负控目标先采基线再上岗**：本网络 1.1.1.1（ICMP 100% loss + TCP 超时）和 114.114.114.114（ICMP 100% loss）本身不可达，不能当 deny 负控；实测可用负控 = 180.76.76.76（ping 23ms）+ example.net IP（TCP 200）。deny 窗口会断 WiFi 保活（NO-CARRIER 掉链，需手动重连）——窗口必须单脚本短开、**脚本内自动恢复**，勿留窗口做人肉排查。
- **疑似产品缺陷（开放项）**：用户"清除全部规则"后 egress 维度表残留 rule#0（bit0）条目（out_uid_map[100]、out_sport_map×16 range、out_proto[6=TCP]、out_iface["wlan0"]、out_saddr_map[180.101.51.73/32]，out_action_map[1] 值全零）——**清规则不清维度表**。当前分析：`match_action_key` 的 AND 链会被无条目维度归零（ZERO_BITMAP 兜底），残留不会直接裁决放行/拦截，但属配置面与执行面脱节，待核实 ext 清除路径后定缺陷或治理项。（更正注记：当时记录的"out_wildcard_map[4] 值全零"系 anchor 盲区误读，见上条；wildcard 表实际内容需以显式 key 直查为准。）

### DNS 应答缓存旁路防火墙裁决（2026-11-15 晚发现并修复，cache2 命中不再过 egress）

- **现象**：deny 域名规则翻转后，此前 allow 期解析过的域名照常连通（curl 0.13s 403 放行）；用从未查询过的子域同批测试则正常被拦——差异即缓存命中。musl `getaddrinfo_ext` 先走 netsysnative cache2 IPC，**命中直接回包、不经 egress 再裁决**，allow-era 应答在新 deny 规则下继续服务。
- **修复模式**：规则/默认动作**变更完成后冲刷全量 DNS 缓存**——`DnsParamCache::FlushAllDnsCache`（遍历 serverConfigMap_ 每 netId 清应答/Nodata/Ipv6UidBlackList）经 provider 注入 bpf 层，挂在 `SetFirewallRules` / `ClearFirewallRules` / `SetFirewallDefaultAction` 三个变更点完成之后（均在 rulesMutex_ 作用域外调用，锁序 cacheMutex_→dnsServerMutex_ 单向）；注册点在 `DnsManager::RegisterFirewallDnsProviders`（`NetManagerNative::Init` 启动路径，fw 宏门控内，provider 为无状态单例 lambda）。
- **连带发现（服务重启行为）**：服务重启（unload 路径销毁 pinned maps → 新进程空表重建）会清空全部规则/默认动作，且 DNS sniff 链会在重启后把首个走 wire 的查询重新写回缓存——**重启 ≠ 保留规则**；跨重启的"缓存残留"排查要先分清是持久化还是 sniff 重建（本仓应答缓存**无落盘**，重启后缓存来自 sniff 回填）。
- **教训**：任何"命中即返回"的旁路面（缓存/快照/短路径）与安全裁决面并存时，**裁决依据变更（规则/默认动作/策略翻转）必须连带失效旁路面**；测试矩阵必须含"翻转前已解析过"的暖缓存样本，只测冷查询发现不了这类缺陷。
- 残留：进程外调试工具（直调 .so）设置规则不经过服务进程，服务缓存不会被冲刷——这是调试工具语义边界而非产品缺口（产品路径规则必经服务 IPC）；复测须走 IPC 向量（signApp）或 UT。

### 一个修复横跨两个产物 so（2026-11-15 晚部署实录，轮换完整性坑）

- **现象**：缓存冲刷修复代码分属两个构建产物——`bpf_netfirewall.cpp`（挂接点，编入 libnetsys_bpf_utils.z.so）与 `dns_manager.cpp` + `dns_param_cache.cpp`（注册+冲刷实现，编入 libnetsys_native_manager.z.so）。只轮换前者时链路静默半残：挂接点调用空 provider，无日志无报错，服务照常运行。
- **教训**：跨 so 的联动修复上真机前必须先做**产物映射表**（改动文件 → BUILD.gn target → out 产物名），逐文件核对"该轮换哪些 so"；半部署的验证结论（"没有冲刷日志"）会被误读成修复无效。诊断手段：对部署产物做符号级验证（含注册 lambda 的静态符号表、flush 接口导入符号）再跑验证。

### 自研 IPC 客户端工具的三种 ABI 崩溃形态（2026-11-15 晚 fwipc 三轮实录，工具链失效模式）

- **形态一（依赖闭包不全 + RPATH 劫持）**：out 版工具库只拷直接依赖，间接依赖（libipc_common 等）被系统库 RPATH 解析到 chipset 分区 → 同进程混载两代 RefBase/sptr 实现 → `~RefBase` 段错误。闭包必须**递归解析全部 NEEDED**（排除 libc/libc++）后整集部署。
- **形态二（设备旧 so × 新源码 headers）**：用设备原装 so 链接、拿工作区新 headers 编译——内联构造按新布局写成员、so 内析构按旧布局读，`~NetsysNativeServiceProxy` 内 `~sptr` 段错误。headers 与 so 必须同源同版本。
- **形态三（trunk 客户端栈 × 商用版设备 samgr 服务）**：即使闭包完整、同源一致，**trunk samgr 客户端对 SP20 设备 samgr 服务发起 GetSystemAbility，返回的 remote 对象本身损坏**（对象首 qword=0x20 非 vtable，任何 vtable 访问即 SEGV@0，CFI 类型检查指令 `ldur [vptr,#-32]` 处崩）——版本代差在协议层，非部署问题。
- **定案**：跨版本驱动的 IPC 向量验证，正确路径是**设备同栈客户端**（产品自身链路，如 signApp）或**匹配基线的完整环境**；自研工具只适用服务端 .so 已轮换到工作区版本的场景。可用替代验证：UT 二进制直接上设备跑（与 out 产物全链一致）+ 代码路径审计。
- **附带技巧**：排查这类崩溃，进程内 `dlopen(RTLD_NOLOAD)+dladdr` 打印实际加载库路径、后台暂停 + `/proc/<pid>/maps` 全量核对、faultlogger 崩溃栈反汇编对符号，三步足够定位；stdout 必须无缓冲（`setbuf(stdout,NULL)`），否则崩溃前输出丢失误导定位。

### F5 定型：deny 命中 UDP sendto 本就同步 EPERM，残余时延全在 musl 重试（2026-11-15 晚跨仓定案）

- **推翻原前提**："UDP 静默丢包无信号"不成立——cgroup egress DROP 对 UDP sendto 是**同步 EPERM（0-1ms）**，探针实测。curl ~3.8s / 浏览器 30-40s 的残余时延 = **musl `res_msend` 把 EPERM 当普通失败按 poll 时间表重试**（musl 源码实锤；App 与 shell 均进程内直发 UDP，无 dnsproxy 中转）。
- **helper 157 destroy 运行时无效（runtime-inert）**：verifier 放行 ≠ 运行时生效——探针证明 destroy 后 poll 仍等满 6s、SO_ERROR=0、socket 存活。两位点保留（注释已写 best-effort 降级），ms 级收口在 musl 侧（EPERM 视为致命错误）= **third_party/musl 跨仓动作项**。
- **教训**：seccomp/verifier 类"放行"只证明可加载，**运行时语义必须行为级探针验证**；"工具报错差异"（ping 对 v6 dgram 无 `sendto: EPERM` 提示）不等于语义差异，须事件日志（`egress intercept: protocol=58`）定案。

### 场景 9/场景 1 定案：ICMPv6 deny 拦截 = F6 同根因；D1 全域 EACCES 修复后未复现（2026-11-15 晚）

- **ICMPv6**：ICMPv6 deny 规则下 ping6 ::1 / fe80 均 100% 丢包被拦（`egress intercept: protocol=58` 事件铁证），清规则恢复；v6 无 `sendto: EPERM` 提示是 ping 工具对 v6 dgram 的报错差异，非缺陷。#7/#8/#9 三场景同一根因（F6 修复覆盖），无独立问题。
- **D1（App 全域 EACCES）**：修复后环境（setuid 真实 App uid、冷/热域名、ADDRCONFIG 开关全矩阵探针）**未复现**，大概率已被 R2/F6 修复；残余不确定性（真 App netId/netstack 层）留给 signApp 复测闭环。

### 规格对齐改判方向：把规格要求的保留语义当成待改项（2026-11-16 第二版批A精确还原定案）

- **现象**：规格对齐第一版执行把 deny 分支、dns_sniff_drop、allowrst 透传、MatchDefaultActionMap、门④ 五处存量语义当"待修项"改掉；第二版核对规格后**全部精确还原**（N1，5 文件对 HEAD 残差 0），仅保留 853 扩展件，门条件落为非对称 `(53&&allowrst)||853`。
- **还原中的再精化**：`dns_sniff_drop` 销毁语义最终为 **egress-only**（仅 egress 拦截点销毁 socket；ingress 丢弃不销毁——socket 可能仍服务其他在途查询，且与用户态 `NotifyEgressIntercept` 的 EGRESS 过滤对齐）——"还原"不是无脑回滚，逐拦截点核销毁方向。
- **教训**：①判决/拦截/销毁/透传语义属**规格面**，规格含糊或与直觉相反时先对规格文本与**存量测试用例**（用例=规格实例）再动代码，禁止按实现直觉"顺手改语义"；②"对齐/还原"类任务动手前先建 diff 锚点基线，收尾用**残差核对**（目标文件与基线 diff=0）自证，而非肉眼比对；③销毁类动作必须区分拦截点方向（egress/ingress），销毁错方向会误杀复用 socket 的在途查询。

### DnsManager 测试 SIGABRT：栈构造 × enable_shared_from_this 杀死整个模块二进制（2026-11-16 存量缺陷定案）

- **现象**：DT 上 `netsys_native_manager_test`（948 用例）整模块报 **unavailable**，`summary_report.xml` 无任何用例结果；连续多轮同崩。
- **根因**：`DnsManagerTest.SetFirewallRules002` 在测试栈上直接构造 `DnsManager`（该类 `enable_shared_from_this`），RULE_DOMAIN 路径调用 `shared_from_this()` 抛 `bad_weak_ptr` → `std::terminate` 杀死整个测试进程 → DT 只能报模块级 unavailable，**一个用例崩溃掩盖全模块 900+ 用例的真实结果**。
- **修复**：`SetFirewallRules002/003`、`EeffectDomainRules001` 三用例改 `make_shared` 托管构造；异步断言（ffrt submit 的联动）前补 100ms sleep 窗口；补 `<thread>/<chrono>` 头。
- **教训**：①被测类 `enable_shared_from_this` 时，测试夹具**一律 `make_shared` 托管**，禁止栈构造/裸 new；②DT 模块报 unavailable（而非用例失败）时，先到模块 log 里搜 `terminate`/`bad_weak_ptr`/SIGABRT 定位单用例崩溃，不要反复重跑；③测试触发的 ffrt 异步联动，断言前须有同步屏障或 sleep 窗口，否则断言读到联动前状态；④失败归因先做**基线三分法**（对昨日同套件基线分：存量同败/设备态波动/本次引入），本次 12 败 = 存量 8 + 波动 1 + 本次 3，避免把存量缺陷当新回归误修。

### DT UT 进程在共享验证设备上真实加载 BPF → 残留挂载 + 空 maps fail-closed 全丢弃断网（2026-11-16 自伤事故实录）

- **现象**：用户正在验证的设备上跑第二轮 UT 后**全网断链**（WiFi 全丢弃），用户被迫重启恢复。
- **根因链（四因叠加）**：①`netsys_netfirewall_test`/`netsys_native_manager_test` 二进制在设备执行时**真实调用 BPF loader 加载 netsys.o 并挂 cgroup 程序，进程退出后挂载不自动消失**；②服务进程此前已加载同一程序 → 同一 attach 点多副本 `BPF_F_ALLOW_MULTI` **判决相与**；③测试进程自身 maps 为空 → 空 maps fail-closed 参与相与 → **全部 DROP 断网**；④随后裸跑 `elfload_harness`（未 `nsenter` 进 netsysnative mount ns）重复 LoadElf 覆盖 pins，加剧污染；测试进程还会直接写共享内核 maps，污染用户验证数据。
- **教训**：①这两个 UT 套件是"真实加载 BPF + 写内核态"的测试，**禁止在他人正在验证的共享设备上跑**；确需跑须先征得使用者同意、跑后重启设备恢复；②`elfload_harness` 必须以 `LD_LIBRARY_PATH=... nsenter -t $(pidof netsysnative) -m` 方式运行，裸跑=重复加载覆盖服务 pins；③"模块 unavailable + 设备网络异常同时出现"时先怀疑测试污染共享态，再查产品缺陷；④测试二进制与设备产物**版本联动**——两测试二进制动态链接 `/system/lib64/platformsdk/libnetsys_bpf_utils.z.so`，设备 so 未同步轮换时新断言必假失败；部署三件套 = `netsys.o` + 两 so + 服务重启；⑤重启 = 单次干净加载 + 全新 maps（规则/默认动作/缓存全清，无残留）——既是本事故的恢复手段，也是"规则是否有残留"排查的判据。

### 防火墙 BPF 死链清理：重写后"死写者+活读者"半拆链 = 静默行为退化（2026-11-16 清理定案）

- **现象**：AC 重写（HEAD 3c243efc）落地后，legacy 域名匹配链（parse_queries_name/parse_queries/match_domain_suffix/match_domain_prefix）全仓零调用，但只删了"读侧的一部分"——四张 legacy 域名 map（DOMAIN_PASS/DENY/PREFIX_PASS/PREFIX_DENY）的 BPF 读者死在死代码里，**用户空间写链（SetFirewallDomainRules→SetSingleDomainRule→SetBpfFirewallDomain*Rules 每规则逐条写）与 ClearDomainRules 清表仍在全量运行**（纯冗余开销）；更隐蔽的是 DOMAIN_DATA_KEY_MAP：唯一 BPF 写入方在死代码里，`log_intercept_event` 仍读它 → **deny 拦截事件的 domainData 自 AC 重写起恒为空（观测面静默退化，无任何告警）**；DNS fuzzer 同样：stub 仍喂 DOMAIN_PASS_MAP（死分支），AC 表未 stub → allow/deny 判决恒 0 → 应答学习深路径不可达。
- **清理**（2026-11-16，死代码全量拆除）：legacy 四函数、五张死 map、用户空间 legacy 写链/清表链、`intercept_event.domainData` 字段+DecodeDomainFromKey+`log_intercept_event`（与 `log_intercept` 合并）、`FIREWALL_SYSTEM_UID` 死宏（用户空间孪生 `FIREWALL_SYSTEM_USER_ID` 是 innerkits 契约保留）、utils.h 两个孤儿函数、**级联死宏 `DNS_DOMAIN_LEN_MIN`（最后使用者随 DecodeDomainFromKey 删除）**、**event.h 对 map.h/utils.h 的两个冗余 include（map.h 本为已删的 DOMAIN_DATA_KEY_MAP 而引入；两 include 删除后 event.h↔domain.h 循环包含断开）**；GetDomainHashKey 保留作 AC builder 差分基线；innerkits `InterceptRecord.domain` 字段不动（parcel 契约），仅不再赋值。二轮全量孤儿扫描（types.h 全部宏/map.h 全部表/各头文件全部 static 函数/类方法/fuzz 与 UT stub）：match.h/ct.h 的 16 个 intra-only 助手与 `ReconcileBpfMap*` 模板簇均为活链（外部引用≠0 或模板体内互调），无新增死代码。
- **教训**：①"替换式重写"收尾必须做**全链残差核对**：新链上线后，旧链的 map/宏/用户空间写链/测试 stub/事件读者逐端点 grep 清零——半拆链的代价不是"无害死代码"，是**行为已变而无人知晓**（本例：观测面丢 domain 且白付每规则 4 次写 map）；②"保留可逆性"注释在重写转正（设备验证+生产路径）后即失效，须及时转正删除，DCE 只救 BPF 侧体积、救不了用户空间写开销与读者歧义；③被死链拖着的**观测字段**（domainData）删除前先核对消费终点是否 innerkits 契约（InterceptRecord.domain 是 parcel 字段不可删，只能断开赋值链并注释说明）；④fuzzer/UT 的 stub 环境随被测实现重写**同步过期**——stub 喂的 map 没人读=深路径不可达，重写落地时 fuzz 契约注释与 stub 分支必须同版更新，"深路径不可达"要么修复要么明示。
- **遗留决策项**（未随清理处理，留档）：①deny 拦截事件恢复携带 domain 需 AC deny 路径新增 per-skb map 写入（verifier 预算待评估）；②fuzzer 深路径恢复需"任意 qname 强制 allow"的 AC stub 方案（固定规则无法匹配任意 fuzz 输入，需设计）；elf map 数 70→65，部署侧 loader 按 ELF maps section 自动 pin，服务重启即干净重建。

### 防火墙 UT 套件质量改造：类静态开关被首用例拨后不还原 × 条件断言 = 顺序耦合假失败（2026-11-16 改造定案）

- **现象**：`netsys_netfirewall_test` 163 用例 12 败长期"存量"：WriteSrc/DstPortBpfMap001、SetFirewallDomainRules001/002、WriteInterfaceBpfMapEmptyTest、WriteWildcardBpfMapTest 均为 `ret=0（写成功）× 期望 IsBpfLoaded()?0:-1（开关被污染为 false）` 假失败；InterfaceBitmapBuild002/003/005/006/InterfaceRebuildTest 是空名接口锚点删除后的陈旧期望；DnsGateCtxFill001 是测试助手 bug。
- **根因链**：①`isBpfLoaded_` 是**类静态成员**（所有实例共享）且**构造函数会把它重置 false**——任何用例构造实例都会翻转全局开关；②首个用例 `SetBpfLoaded(false)` 后不还原 → 全套 `IsBpfLoaded()?a:b` 条件断言被污染（map 实际可用 ret=0 vs 期望 -1）；环境差时反向"空虚通过"（写失败 -1 与开关 false 恰好匹配）——**同一断言在两种环境里都是错的**；③`appliedSnapshots_` 等单例状态跨用例持久 → 同 map 路径先写者走 full、后写者走 delta，用例顺序改变覆盖路径；④DNS 预热引擎（`RunDnsPrewarmTask` ffrt 任务与老化线程）**硬绑 GetInstance()**——fresh instance 注册 resolver 对任务不可见，预热类用例无法去单例。
- **改造**（2026-11-16，零生产码改动）：`firewall_test_env.h`（NewFirewallInstance + FirewallLoadScope RAII + 隔离契约）；夹具 SetUp 归一化开关、TearDownTestCase 零残留清扫；48 处 GetInstance→fresh instance（BUILD.gn `private=public` 使私有构造可测直达，快照/回调/provider 实例级隔离）；条件断言全部无条件化（fixture 统一 LoadElf 保证 map 可用，坏环境=响亮失败）；裸拨动全 RAII 化；陈旧期望按锚点删除语义修正；TcpHeader 助手 bug（第三参写在 flags 字节 13 致 doff=0 提前返回，改 TcpHeaderDns 置字节 12）。**12 败 → 163/163 全过**。
- **教训**：①**禁止条件断言**（`ret == env ? a : b`）——它把环境不确定性转译成"永真断言"，两种环境都测不出回归；环境前置要么夹具保证（失败即响亮），要么打桩；②类静态成员是隐式全局面——测试对它的每次写都必须 RAII 保存/恢复，且构造函数重置静态成员的副作用要当作隔离起点显式管理而非隐式依赖；③"§10 M10 兼容"注释追踪的环境依赖用例，修法是**统一 map 加载前置**而不是逐断言条件化；④单例硬绑的异步引擎（ffrt 任务/周期线程解析 GetInstance）测试侧只能单例+显式清理，去单例改造须生产码先行（本批以契约注释固化例外清单）；⑤测试助手族（TcpHeader vs TcpHeaderDns）语义相近极易混用——doff 字节与 flags 字节差一字节即整面短路，TCP 面用例必须用带 doff 的助手。

### 防火墙 wildcard 四连修：builder 求值顺序 / 展开边互撞 / 过滤器误捕 / 开发设备 XPM EACCES（2026-11-17 定案）

- **现象**：wildcard 通配符语义扩展后 `netsys_netfirewall_test` 4 败（162/166），且修复后新编 `libnetsys_bpf_utils.z.so` 部署设备 netsysnative dlopen 失败 `MUSL-LDSO: mmap fix failed errno=13`（EACCES）。
- **根因 ×4**：①`BuildPattern` 中 `leadingFragment` 在 `subdomainOnly` 擦除前导点**之后**求值 → `*.example.com` 误判 leadingFragment、锚丢 LEN(7)、MORE 吞裸域边界（#2/#3 败因）——派生判定依赖的输入被前置变换改变；②SKIP 展开行与共享节点具体 LEN 边**同 key 互撞**，单写 hash map 后写胜出（#4 败因）；③测试 gotFwd/gotRev 过滤器 `c & 0x100` 误捕 SKIP(0x301)/MORE(0x300) 伪类（#1 败因，纯测试侧）；④EACCES 与代码无关——**HM 内核 XPM 执行保护**（`/proc/sys/kernel/xpm/xpm_mode=5`）对系统服务进程 PROT_EXEC 文件映射做代码签名校验，未签名本地构建 .so 被拒。
- **修复**：①`leadingFragment` 求值前移到擦除之前；②Insert 侧建立「SKIP 节点吸收具体 LEN 边」不变量（MergeInto/CopySubtree 深合并 + erase 被吸收边）+ Emit 侧孤儿（BFS 不可达）节点过滤，MORE 碰撞同类修复；③两循环显式排除 SKIP/MORE 伪类；新增 `AcScanSkipEdgeCollision001`（双插入序）/`AcScanMoreEdgeCollision001` 回归用例；④设备侧 `echo 1 > /proc/sys/kernel/xpm/xpm_mode`（root）后重启进程即恢复（非产品缺陷，开发设备部署约束）。
- **EACCES 定案过程要点**（供复用）：同 inode 交替部署旧/新内容 4+ 次（同 boot）实锤内容相关、非环境漂移；旧内容单字节变体（RO 段/页 0 EI_PAD/exec 段 padding/尾字节）全部可加载，排除摘要白名单/大小门控/页级缓存等理论；自制 mtest（预留 + 逐段 MAP_FIXED + PROT_EXEC + 整文件 R+X）在 shell 上下文对新旧 .so 全部成功 → 失败仅在受信系统服务进程内 → 定位到 per-process 执行保护；musl 侧 `task_check_xpm`（`MAP_XPM=0x40` 探测）可过而真正 PT_LOAD 段映射被拒。ninja 直编 .so 与 CBuild 产物**字节一致**（native_manager md5 对拍证明无 ninja 外后处理），无需因部署问题重走 CBuild。
- **教训**：①builder 类纯函数改动，凡有"先变换输入再派生判定"的顺序依赖，锚点/裸域边界用例先行；②展开型伪边（SKIP/MORE）与具体边同 key 时必须定义吸收语义并配双插入序回归；③测试过滤器在类值域含伪类时禁用位掩码（`& 0x100` 必然误捕），改显式排除；④开发设备部署未签名本地构建 .so：先降 xpm_mode 再重启目标进程，验证口径不变（LoadElf is 0 + pins 齐全）；xpm_mode 会被开机 key_enable 服务重置，重启后需重降级或恢复镜像原件（备份在 `/data/local/tmp/fw_wild_stock_bak/`）。

### DNS 域名门双命中裁决「修复」反转事故：defaultactionrst 枚举极性误读（2026-11-17 定案，UT 拦截未流出）

- **现象**：「冲突消解跟随默认策略方向」裁决修复（`netfirewall_domain.h` parse_dns_query/parse_dns_response 两处 `!allowrst` 子句换分支）部署后 DT 首轮 3/175 败，全为新增双命中用例，且失败值恰是改动前语义的**精确反转**（默认放行+双命中→DROP、默认拒绝+双命中→PASS）。
- **根因**：`defaultactionrst` 承载 `MatchDefaultActionMap()` 返回的**原始 `sk_action` 枚举值**（SK_DROP=0 / SK_PASS=1，即 **1=默认放行**），被误读为"1=默认拒绝"布尔。按真实极性重推：原分支① `!defaultactionrst && denyrst` = 默认**拒绝**+命中 deny→拦（双命中 deny 赢）；原分支② `defaultactionrst && denyrst && !allowrst` = 默认**放行**+无对冲 deny→拦（双命中 allow 赢）——**原代码本已实现用户定版语义**，与 IP 位图链 Conflict & Repetition Algorithm（match_action：默认 DROP 时命中任一 drop 位即拦 / 默认 PASS 时命中任一 pass 位即放）完全一致；"修复"把正确实现反转。
- **修复**：回退两处分支到原结构，保留极性注释（`defaultactionrst` carries the RAW sk_action enum, NOT a boolean "default is deny"）；3 个新增双命中用例（DnsSniffDoubleHitDefaultAllow001/DefaultDeny001/ResponseFollowsDefault001，baidu 形态 `AcDnsEnvConflict`：deny `*.example.com` + allow `www.example.com`）保留为语义锁位，对回退后代码 175/175 全过。
- **教训**：①语义改动前先核实变量承载的是**枚举原值还是布尔**（`x rst` 命名不等于布尔，`sk_action` 直接透传时 1=SK_PASS）；②「实现 X 语义」类任务先写按目标语义断言的锁位 UT 矩阵、让现状代码先跑一遍——现状已绿则代码已符合、改分支必反转；③UT 报"改动前后语义互换"时第一假设是极性/映射误读，而非测试断言错；④DNS 域名门与 IP 位图链的裁决矩阵必须同版核对（两处实现刻意同语义，见 match_action Conflict & Repetition Algorithm 注释）。
- **补遗（同日冲突矩阵锁位用例实录）**：为补齐 IP 位图链双命中锁位缺口新增 `MatchActionConflictMatrix001`（默认两态 × deny-only/allow-only/double-hit 六行），首版 1 败——**被测函数原地改写入参**（`bitmap_and`/`bitmap_and_inv` 直接 MUTATE `key->val`）× 测试跨行复用同一输入结构体 = 行间状态泄漏假失败（row 1 把 `allowOnly` 改成 0、row 3 把 `doubleHit` 改成 0x1，后续行输入已非构造形态）；修正 = 每行 EXPECT 前新建独立位图结构体。复跑 176/176 全过，DNS 门与 IP 位图链两条裁决矩阵自此同版用例背书。另：UT 跑后仅重启 netsysnative 进程**不足以**清内核态 attach 残留（`attach cgroup_sock_inet_release_socket failed: errno=7` → `LoadElf is 10`、progs 21/30），排行榜 #18 的"跑后重启**设备**恢复"须按字面执行；本例恢复链 = 整机 reboot → `echo 1 > /proc/sys/kernel/xpm/xpm_mode` → 重启 netsysnative → LoadElf is 0 + 85 maps/30 progs。

## TOP 失效模式排行

| 排名 | 失效模式 | 代表提交 | 规避要点 |
|---|---|---|---|
| 1 | 并发/死锁/竞态 | `a98cd919`、`51129900`、`d0fda269`、`044d5d70`、`2103f660`、`080a440c`、`ea5291d7` | [concurrency-failure-modes.md](concurrency-failure-modes.md) 全文 |
| 2 | 空指针/IPC 对象生命周期 | `63961c40` | proxy/stub 输出对象一律判空再解引用 |
| 3 | 资源泄漏 | `f2e10ae5`、`1b8b67fb` | 异步/异常每条失败路径显式释放；泄漏修复必带长稳曲线 |
| 4 | 安全/权限/校验遗漏 | 持续防范（新增即回写本表） | [ipc-security-constraints.md](ipc-security-constraints.md) |
| 5 | 数据面越界读 | `c3d1d423` | ring buffer/数组先边界检查后读 |
| 6 | BPF 扫描间暂存契约腐蚀 | 2026-11-15 ac_suffix_scan（未提交） | 共享 scratch 字段双职时，所有退出路径恢复契约值；扫描链任何改动先跑四连扫场景用例 |
| 7 | 多线程内存序 | `2103f660` | 共享状态 atomic 化，禁止裸读写 |
| 8 | SA 崩溃恢复残留 | `8c973dfb` | SA 重启后客户端状态全量重建 |
| 9 | 统计丢失 | `9ee4ebbf` | 统计链路改动必跑 bpf stats 用例 |
| 10 | 硬编码/观测规范 | `2ae80b5e` | 常量只在统一定义处取值 |
| 11 | 豁免面扩大无受益者核对 | 2026-11-15 C5 过度豁免（3c243efc，F6 修复） | uid 群体改动（豁免/收拢）必须列出假 uid 哨兵/真实低 uid/真实高 uid 三路处置矩阵，并核实旧路径真实行为 |
| 12 | 有状态控制面多进程拼接被异步联动清态 | 2026-11-15 F4 门假失败（测试方法论） | 规则/默认动作/授权类 API 的真机场景须单进程搭建；异步 ffrt sync 以进程内 registry 判定，跨进程调用即碎片化 |
| 13 | 观测工具遍历缺陷误报产品缺陷 | 2026-11-15 F2 wildcard 假缺陷（maplook 直查证伪） | BPF map 遍历工具的 GET_NEXT_KEY 首键语义要先验证（显式 key 直查交叉核对）；"表里缺条目"类结论必须过直查关才能定性 |
| 14 | 旁路面命中跳过安全裁决 | 2026-11-15 DNS 缓存旁路 deny（cache2 命中不过 egress） | 缓存/快照/短路径与裁决面并存时，裁决依据（规则/默认动作）变更必须连带失效旁路面；测试矩阵必含暖缓存样本 |
| 15 | 跨版本客户端栈驱动商用设备 IPC | 2026-11-15 fwipc 三轮崩溃（trunk samgr 客户端 × SP20 服务 remote 损坏） | 跨版本 IPC 验证走设备同栈客户端或匹配基线环境；自研工具仅限服务端已轮换场景，部署前做产物映射表 |
| 16 | 规格对齐改判方向：把规格要求的保留语义当待改项 | 2026-11-16 批A五处精确还原（N1 残差 0） | 判决/拦截/销毁语义改动前先对规格文本+存量用例；还原类任务建 diff 锚点、收尾残差核对；销毁动作区分 egress/ingress 方向 |
| 17 | enable_shared_from_this 被测类栈构造 → bad_weak_ptr terminate 杀全模块 | 2026-11-16 DnsManager SIGABRT（存量） | 夹具一律 make_shared 托管；模块 unavailable 先搜模块 log 里单用例 terminate；异步联动断言前加同步屏障 |
| 18 | 真实 BPF 测试跑在共享验证设备：残留挂载+空表 fail-closed 断网 | 2026-11-16 DT 二轮自伤事故 | BPF 类 UT 只在专属设备跑、跑后重启恢复；elfload_harness 必须 nsenter；模块 unavailable+设备网络异常并现先查测试污染 |
| 19 | 替换式重写半拆链：死写者+活读者=观测面静默退化 | 2026-11-16 防火墙 AC 重写遗留死链清理 | 重写收尾全链残差核对（map/宏/写链/stub/读者逐端点 grep 清零）；"保留可逆性"注释转正后即删；观测字段删除前核对 innerkits 终点；fuzz/UT stub 随实现重写同版更新 |
| 20 | 类静态开关拨后不还原 × 条件断言 = 顺序耦合假失败 | 2026-11-16 防火墙 UT 12→0 改造 | 禁止 `ret == env ? a : b` 条件断言（环境前置要么夹具保证要么打桩）；类静态成员测试写必须 RAII 保存/恢复；单例硬绑异步引擎（ffrt/周期线程）测试侧只能单例+显式清理；TCP 面测试用带 doff 的头构造助手 |
| 21 | 派生判定求值点位于输入变换之后 → 语义误判 | 2026-11-17 wildcard 四连修 #2/#3（leadingFragment） | "先变换输入再派生判定"的顺序依赖要在 builder 改动中显式标注；改 BuildPattern 必跑锚点+裸域边界用例 |
| 22 | 展开型伪边与具体边同 key 互撞：单写 hash map 后写胜出 | 2026-11-17 wildcard 四连修 #4（SKIP/MORE 碰撞） | Insert 侧建立「展开节点吸收具体边」不变量 + Emit 侧 BFS 可达性过滤；必配双插入序回归用例 |
| 23 | 测试过滤器位掩码误捕伪类 | 2026-11-17 wildcard 四连修 #1（`c & 0x100` 误捕 SKIP/MORE） | 类值域含伪类（SKIP=0x301/MORE=0x300 等）时过滤用显式排除清单，禁止位掩码 |
| 24 | 开发设备部署未签名本地构建 .so：内核 XPM 执行保护 EACCES | 2026-11-17 libnetsys_bpf_utils mmap errno=13 疑难定案 | root `echo 1 > /proc/sys/kernel/xpm/xpm_mode` 后再重启目标进程（key_enable 服务开机会重置）；诊断顺序：同 inode 新旧内容对拍→shell 上下文逐段 mmap 排除→查 `/proc/sys/kernel/xpm/`；ninja 直编产物与 CBuild 字节一致、本身有效 |
| 25 | 枚举原值当布尔误读极性 → "修复"反转已正确语义 | 2026-11-17 双命中裁决反转（UT 拦截未流出，`defaultactionrst`=sk_action 透传，1=SK_PASS） | 语义改动前核实变量承载枚举原值还是布尔；先写目标语义锁位 UT 让现状代码跑一遍（现状绿=勿动分支）；"语义互换"型失败第一假设极性误读；DNS 域名门与 IP 位图链裁决矩阵同版核对 |
| 26 | 被测函数原地改写入参 × 测试跨断言复用输入结构体 = 行间状态泄漏假失败 | 2026-11-17 MatchActionConflictMatrix001 首版（`bitmap_and` MUTATE `key->val`，复用位图结构致第 5/6 行输入被前行改写） | 断言入参会被被测函数改写时（bitmap_and/and_inv 类原地 AND），每行 EXPECT 前新建独立输入结构体，禁止跨行复用；BPF 位图类断言一律"一行一新鲜 key" |
| 27 | UT 跑后仅重启服务进程不足以清内核态 BPF attach 残留 | 2026-11-17 LoadElf is 10（attach errno=7，progs 21/30），进程重启无效，须整机 reboot | 排行榜 #18 的"跑后重启设备恢复"按字面执行；恢复链 = reboot → 重降 xpm_mode（本地构建 .so 部署态）→ 重启 netsysnative → LoadElf is 0 + pins 齐全；"LoadElf is 10 + attach errno=7"签名 = UT 内核态残留，非产物缺陷 |

## 关联知识

- [并发与死锁失效模式](concurrency-failure-modes.md)
- [IPC 与安全约束](ipc-security-constraints.md)
- [架构不变量](architecture-invariants.md)
- [验证循环与 Done 定义](../verify/verification-loop.md)
- [AGENTS.md](../../../AGENTS.md)

## 适用场景

- 改动台账所列文件前，逐条对号预判风险；
- 线上崩溃/卡死/泄漏排查时按失效模式反查历史修复；
- 新失效发生或修复被质疑后，按本模板回写条目并更新排行表。
