# 并发与死锁失效模式（Concurrency Failure Modes）

> 并发是本仓第一热点失效模式（台账 14 条中 7 条，见 [historical-failures.md](historical-failures.md)）。本仓独有的高发组合：**多客户端并发 IPC × napi 回调线程 × probe 探测线程 × ffrt/eventhandler 混用**，叠加双进程（netmanager/netsysnative）跨进程对象生命周期。改任何 `services/` 容器、事件分发、探测线程、锁作用域前必读。

## 高发线程组合

| 组合 | 冲突点 | 历史案例 |
|---|---|---|
| napi 工作线程 × JS 线程 | exec 模块内部 map 被 job 线程与 JS 线程并发读写 | `044d5d70`（network_exec.cpp） |
| 多客户端并发 IPC × 服务容器 | supplier 等核心容器无锁并发读写 | `d0fda269`（net_conn_service.cpp） |
| 事件分发 × 监听器注册/注销 | 两阶段锁顺序冲突成死锁环 | `a98cd919`（event_manager.cpp，钉钉 Top1） |
| 统计查询 × 后台刷写 | 缓存锁粒度交叉互等 | `51129900`（net_stats_cached.cpp） |
| probe 线程 × 主流程 | 共享标志/指针无内存序保证 | `2103f660`（probe_thread.cpp） |
| 供应商评分更新 × DefaultNet 评选 | defaultNetSupplier 读写未完整入锁 | `ea5291d7` |

## 模式详解

### 模式一：服务端裸容器并发访问

```cpp
// ❌ 错误写法（d0fda269 修复形态）：多客户端线程并发直接读写容器
bool NetConnService::SupplierExists(netHandle handle) {
    return netSuppliers_.find(handle) != netSuppliers_.end();   // 无锁读
}

// ✅ 正确写法：读写锁完整覆盖（修复引入 ffrt rwlock 形态）
bool NetConnService::SupplierExists(netHandle handle) {
    std::shared_lock<std::shared_mutex> lock(supplierMutex_);   // 读锁
    return netSuppliers_.find(handle) != netSuppliers_.end();
}
```

- 规则：`services/` 侧任何被多个 IPC 入口触及的容器（supplier 集合、callback 列表、策略表）**必须显式绑定读写锁或互斥锁**；读多写少首选 `std::shared_mutex`/ffrt 等价物。
- ⚠️ 新增服务端公开方法时的自查问题："这个方法会被几个进程、几个线程、什么时序调到？"

### 模式二：两阶段锁死锁（事件分发）
 
```text
❌ 死锁形态（a98cd919）：线程 A 持 listenersMutex → 等待 emitMutex
                       线程 B 持 emitMutex     → 等待 listenersMutex
✅ 修复形态：统一锁序（listeners 管理与 emit 分发共用一把读写锁，或固定获取顺序）；
   分发路径锁内只做"拷贝 listener 列表"，真正的回调执行放锁外
```

- 规则：**回调分发不得持服务侧长锁**；锁内拷贝数据、锁外执行回调（同 netstack `event_manager` 的 shared/unique 双态语义）。

### 模式三：缓存读写锁粒度交叉

- `net_stats_cached.cpp`（51129900）：查询接口与后台刷新对同一批 cache 桶加锁粒度/顺序不一致，特定时序互等。
- 规则：缓存类组件统一"桶粒度 + 固定升序加锁"，或单一粗锁 + 短临界区；**新增查询入口时先核对已有刷写路径的锁序**。

### 模式四：跨线程共享状态无内存序

```cpp
// ❌ 错误写法（2103f660 修复形态）：
bool probeRunning_ = false;            // 探测线程与控制线程共享，裸 bool
CurlHandle *probeCurl_ = nullptr;       // 裸指针跨线程传递

// ✅ 正确写法：
std::atomic<bool> probeRunning_{false};
std::shared_ptr<CurlHandle> probeCurl_;  // 或完整锁保护 + atomic 发布
```

- 规则：线程间共享的运行标志、句柄指针一律 `std::atomic`/智能指针 + 显式内存序，❌ 裸基本类型与裸指针。

### 模式五：SA 崩溃恢复状态残留

- `8c973dfb`：netmanager 进程重启后，客户端 `net_conn_client.cpp`/`net_supplier.cpp` 残留旧 supplier 状态，脏数据继续参与 DefaultNet 评选。
- 规则：**SA 死亡回调（samgr 通知）= 全量作废本地远端状态并重建**；客户端不得假定远端存活期间挂起的请求语义仍有效。

## ffrt 与 eventhandler 混用警示

- 本仓并发原语历史上有 ffrt 与 eventhandler 双体系并存（如 d0fda269 修复采用 ffrt rwlock），不同模块线程模型不同。
- ⚠️ 跨模块传递任务/等待句柄前先确认两侧同属一个运行时；混用任务句柄=未定义行为。
- 锁选型随所在模块既有体系，不引入第三套并发原语。

## 触发场景 × 预防模式速查表

| 改动触发场景 | 必做预防 | 回归要求 |
|---|---|---|
| 新增 IPC 接口修改服务容器 | 读写锁覆盖所有读写点 | 并发 IPC 压测（多客户端同调） |
| 新增/修改事件或回调分发 | 锁序文档化；分发锁外执行 | 注册/注销 × 事件爆发压测 |
| 新增缓存查询入口 | 核对刷写路径锁粒度与顺序 | 查询 × 刷写并发压测 |
| 新增线程（探测/后台任务） | 共享状态 atomic 化 | 启停循环 + 异常注入压测 |
| SA 重启恢复逻辑 | 状态作废重建，禁止续用 | 杀进程恢复场景回归 |
| 任何 race 警告 | 立即修复，禁止搁置（080a440c 教训） | 补最小复现并发用例 |

## 关联知识

- [历史高危修复案例](historical-failures.md)：上述模式的完整案情
- [IPC 与安全约束](ipc-security-constraints.md)：承载并发的 proxy/stub 结构约束
- [架构不变量](architecture-invariants.md)：双进程职责与通道边界
- [验证循环与 Done 定义](../verify/verification-loop.md)：并发自测场景要求
- [AGENTS.md](../../../AGENTS.md)：§6.4 陷阱排行

## 适用场景

- 改 `net_conn_service.cpp`、`net_stats_cached.cpp`、`event_manager.cpp`、`probe_thread.cpp` 等红区文件前对号预判；
- 新增服务端方法/线程/事件分发/缓存查询入口时的设计自查；
- 死锁/竞态类线上问题排查时按模式反查历史修复。
