# 网络连接链路地图

> 本文拆解 SA 1151（NetConnService）的核心链路：网络注册链、DefaultNet 评选、网络探测链、应用侧回调链与 NetConnState 状态机。修改连接相关代码（供应商注册/评分/探测/回调）前必读。

## 1. 网络注册链（supplier 注册 → 内核落地）

```
硬件/系统服务（wifi、蜂窝等注册方）
  → NetConnClient::RegisterNetSupplier（interfaces/innerkits/netconnclient/include/net_conn_client.h）
  → IPC：CMD_NM_REGISTER_NET_SUPPLIER（interfaces/innerkits/netconnclient/include/proxy/conn_ipc_interface_code.h）
  → frameworks/native/netconnclient/src/proxy/net_conn_service_proxy.cpp（2607 行，红区）
  → services/netconnmanager/src/stub/net_conn_service_stub.cpp（2416 行，红区；权限映射后分发）
  → services/netconnmanager/src/net_conn_service.cpp（4787 行，红区）NetConnService::RegisterNetSupplier
       ├─ 创建 NetSupplier（services/netconnmanager/src/net_supplier.cpp，722 行；score/ident/bearerType 挂在此）
       ├─ 挂到 Network（services/netconnmanager/src/network.cpp，1243 行）
       └─ 后续 UpdateNetCaps / UpdateNetSupplierInfo / UpdateNetLinkInfo 持续刷新
  → UpdateNetLinkInfo 时经 services/netsyscontroller/src/netsys_controller.cpp（NetsysController，2040 行）
  → services/netsyscontroller/src/netsys_native_client.cpp（NetsysNativeClient，2376 行，红区）
  → IPC 跨进程 → services/netmanagernative/src/netsys_native_service.cpp（NetsysNativeService，netsysnative 进程）
  → src/manager/conn_manager.cpp（连接创建/销毁）+ src/manager/route_manager.cpp（路由表落地）
  → src/netsys/（netlink_msg.cpp / netlink_socket.cpp 封装 netlink；physical_network.cpp 按网络类型写入规则）
  → 内核（netlink/route table）
```

要点：

- supplier 的 `ident_`（如 wifi iface 名）与 `score_` 字段定义在 `net_supplier_info.h`（`NetSupplierInfo` 结构），是评选与策略裁剪的关键输入。
- `Network` 内持有 supplier 集合与 connection 全国状态；netlink 侧以 netId 关联。
- `services/netconnmanager/src/net_dns_resolve.cpp`、`nat464_service.cpp`、`net_pac_manager.cpp` 在同目录旁路服务于 DNS/CLAT/PAC 场景。

## 2. DefaultNet 评选（supplier score 加权）

- 入口场景：每次 supplier 的 score/可用性变化（`UpdateNetSupplierInfo`/`SupplierInvalid`）触发评选。
- 核心逻辑在 `net_conn_service.cpp`：
  - `FindBestNetworkForRequest`（net_conn_service.cpp:1400 附近）按请求选网；
  - 遍历比较 bestScore（net_conn_service.cpp:1587-1612：逐 supplier 比较 `score > bestScore` 并记录 bestSupplier）；
  - 切换动作 `NetConnService::MakeDefaultNetWork(sptr<NetSupplier> &oldSupplier, sptr<NetSupplier> &newSupplier)`（net_conn_service.cpp:1989），处理新默认网上线与旧网失联清理。
- 评选输入：supplier 有效分数（发起方给的 score 加上本仓按能力/验证状态的加权）、`NetCap` 集合（如 `NET_CAPABILITY_VALIDATED`=16、`NET_CAPABILITY_INTERNET`=12）、bearerType。
- 评出后：默认 netId 更新，`NET_CONN_SUPPLER_STAT`/`NET_CONN_REQUEST_STAT` 维测事件上报（hiSysEvent，domain `NETMANAGER`），应用回调见 §4。
- 红线：评分与切换逻辑位于 4787 行大文件 `net_conn_service.cpp`，任何改动必须绑定 `test/netconnmanager/unittest` 回归；错乱的选举会造成默认网乒乓切换。

## 3. 网络探测链（NetMonitor → probe_thread → curl）

```
NetConnService（创建/触发探测）
  → services/netconnmanager/src/net_monitor.cpp（NetMonitor，615 行：探测状态机与重试调度）
  → services/netconnmanager/src/probe_thread.cpp（probe_thread，190 行：探测线程封装）
  → services/netconnmanager/src/net_http_probe.cpp（NetHttpProbe，512 行：HTTP 探测实现，经 utils/net_http_client 的 NetHttpClient 发起请求）
  → 结果回填：net_http_probe_result.cpp（探测结果）→ NetMonitor 更新网络 VALIDATED/PORTAL/INVALID 能力位
  → 上报 CMD_NM_NET_DETECTION_RESPONSE 回 NetConnService → 应用回调（netAvailable/netCapabilitiesChanged）
```

- 探测线程独立于主服务线程运行，**curl 全局状态与探测结果共享变量存在跨线程访问**——历史上探测线程 curl 崩溃被定位为内存序问题（提交 **2103f660**，"网络探测的线程curl出现crash，推测内存序"，DTS2024122016392）。改动 `probe_thread.cpp`/`net_http_probe.cpp` 的共享变量时必须使用原子/std::atomic 或锁，并跑并发回归。
- `net_http_probe.cpp` 自 2026-11 起不再直接持有 curl 句柄：探测/PAC 下载/代理保活统一经共享 HTTP 客户端 `utils/net_http_client`（`NetHttp::NetHttpClient`，curl easy/multi 封装，同步 `Execute` + 异步 `Submit`，引擎线程空闲即退出、回调不持锁）。curl 细节全部收敛在 `libnet_http_client.so`，业务层只做请求构造与响应判定；新增 HTTP 调用方应复用该客户端而不是散写 curl。
- `net_dual_stack_probe.cpp`（双栈探测）与 `net_trace_route_probe.cpp`（ traceroute）是探测族的扩展实现，与 `net_http_probe.cpp` 共用线程封装。
- 探测事件：`NET_CONN_MONITOR_FAULT`/`NET_CONN_MONITOR_STAT`（hisysevent.yaml）。

## 4. 应用侧回调链（JS API → RegisterNetConnCallback → 事件回吐）

```
JS @ohos.net.connection（interfaces/kits/js/@ohos.net.connection.d.ts）
  → frameworks/js/napi/connection/connection_module（模块注册）
  → frameworks/js/napi/connection/connection_exec + observer（netAvailable/netCapabilitiesChanged/... 分发）
  → interfaces/innerkits/netconnclient/include/net_conn_client.h:59 NetConnClient::GetInstance()
  → NetConnClient::RegisterNetConnCallback（net_conn_client.h:165 / 带 NetSpecifier 的重载 net_conn_client.h:177）
  → proxy：CMD_NM_REGISTER_NET_CONN_CALLBACK（/BY_SPECIFIER）
  → services/netconnmanager：回调注册登记（net_activate.cpp 处理 NetActivate 激活请求）
  → NetConnService 侧事件变化 → INetConnCallback 回调对象
  → frameworks/native/netconnclient/src/proxy/net_conn_callback_stub.cpp（客户端侧 stub 收事件）
  → NAPI observer → JS 回调（EventTrigger 主线程投递，utils/napi_utils）
```

要点：

- 客户端的 callback stub 与服务端的 callback proxy 必须**成对增删**：服务端持 proxy 推事件，客户端持 stub 收事件，漏改一侧即事件丢失或 IPC 崩溃（规则详见 [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md) 第 ⑤ 步）。
- `net_conn_callback_proxy_wrapper.cpp`（服务端侧）包装回调升级；`i_net_monitor_callback.h` 是探测回调接口。
- JS 侧枚举语义见 §5；`NetSpecifier`（`net_specifier.h`）用于按能力/承载过滤回调。

## 5. NetConnState 状态机（`net_supplier_info.h:25`）

| 值 | 常量 | 含义 |
|---|---|---|
| 0 | `NET_CONN_STATE_UNKNOWN` | 未知 |
| 1 | `NET_CONN_STATE_IDLE` | 空闲 |
| 2 | `NET_CONN_STATE_CONNECTING` | 连接中 |
| 3 | `NET_CONN_STATE_CONNECTED` | 已连接 |
| 4 | `NET_CONN_STATE_DISCONNECTING` | 断开中 |
| 5 | `NET_CONN_STATE_DISCONNECTED` | 已断开 |

- 状态由 supplier 侧维护并随 NetSupplierInfo/NetLinkInfo 更新；能力位（NetCap/NetBearType）定义见 [../routing/vocabulary-routing.md](../routing/vocabulary-routing.md) 枚举速查表。
- 竞态风险：状态迁移可能与评选/探测并发，写路径集中在 `net_conn_service.cpp`（锁边界与历史竞态案例见 [../expert/concurrency-failure-modes.md](../expert/concurrency-failure-modes.md)）。

## 6. 红区文件（改动必须绑定回归）

| 文件 | 行数 | 风险 |
|---|---|---|
| `services/netconnmanager/src/net_conn_service.cpp` | 4787 | 连接核心：注册/评选/探测调度全在此，大文件高并发 |
| `frameworks/native/netconnclient/src/proxy/net_conn_service_proxy.cpp` | 2607 | Parcel 序列化集合，读写不对称即 IPC 数据错乱 |
| `services/netconnmanager/src/stub/net_conn_service_stub.cpp` | 2416 | 权限映射与分发（含 `CMD_NM_SET_INTERNET_PERMISSION` 的 uid 检查，见 net_conn_service_stub.cpp:317 附近），漏登记=越权 |
| `services/netsyscontroller/src/netsys_native_client.cpp` | 2376 | 跨进程咽喉，回执/回调生命周期敏感 |

## 关联知识

- [netmanager-base-code-map.md](netmanager-base-code-map.md)：全局分层
- [sa-architecture.md](sa-architecture.md)：SA 1151 在进程拓扑中的位置
- [netsys-native-map.md](netsys-native-map.md)：NetLinkInfo 落地端的命令组结构
- [../expert/historical-failures.md](../expert/historical-failures.md)：探测线程崩溃（2103f660）等历史失效模式
- [../expert/concurrency-failure-modes.md](../expert/concurrency-failure-modes.md)：评选/探测并发与锁边界
- [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md)：回调成对新增与权限登记流程
- [../routing/vocabulary-routing.md](../routing/vocabulary-routing.md)：NetSupplier/ident/score 等术语

## 适用场景

- 修改网络注册、能力更新、DefaultNet 评选或探测逻辑前定位改动点
- 排查"默认网不切换/乒乓切换/探测误报/回调丢失"类链路问题
- 评估 proxy/stub/callback 三端是否需同步修改
