# 历史高危修复案例（Historical Failures & Lessons）

> 本文件是 netmanager_base 仓已核实的失效模式权威台账：每条含现象/根因/修复文件/验证要点，供改动前"对号入座"与排查时反查。新失效必须按此模板回写本文件（AGENTS.md §8），并同步 [task-routing.md](../routing/task-routing.md)。

## 案例台账（14 条，均经 git 历史核实）

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

## TOP 失效模式排行

| 排名 | 失效模式 | 代表提交 | 规避要点 |
|---|---|---|---|
| 1 | 并发/死锁/竞态 | `a98cd919`、`51129900`、`d0fda269`、`044d5d70`、`2103f660`、`080a440c`、`ea5291d7` | [concurrency-failure-modes.md](concurrency-failure-modes.md) 全文 |
| 2 | 空指针/IPC 对象生命周期 | `63961c40` | proxy/stub 输出对象一律判空再解引用 |
| 3 | 资源泄漏 | `f2e10ae5`、`1b8b67fb` | 异步/异常每条失败路径显式释放；泄漏修复必带长稳曲线 |
| 4 | 安全/权限/校验遗漏 | 持续防范（新增即回写本表） | [ipc-security-constraints.md](ipc-security-constraints.md) |
| 5 | 数据面越界读 | `c3d1d423` | ring buffer/数组先边界检查后读 |
| 6 | 多线程内存序 | `2103f660` | 共享状态 atomic 化，禁止裸读写 |
| 7 | SA 崩溃恢复残留 | `8c973dfb` | SA 重启后客户端状态全量重建 |
| 8 | 统计丢失 | `9ee4ebbf` | 统计链路改动必跑 bpf stats 用例 |
| 9 | 硬编码/观测规范 | `2ae80b5e` | 常量只在统一定义处取值 |

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
