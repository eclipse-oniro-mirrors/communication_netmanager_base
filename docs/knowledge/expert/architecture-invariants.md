# 架构不变量（expert）

> 本文列出 netmanager_base **必须始终保持**的结构性不变量。任何改动使下列陈述由真变假，一律视为破坏性变更，需在方案评审中显式豁免。
> 全局分层图与目录职责见 [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md)；跨进程接口的做法见 [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md)。

## INV-1 内核访问唯一通道

管理面/数据面触达内核（netlink/iptables/BPF map）只有一条合法路径：

```
services/netconnmanager | netpolicymanager | netstatsmanager
  → services/netsyscontroller/src/netsys_controller.cpp     （NetsysController 门面，2040 行）
  → services/netsyscontroller/src/netsys_native_client.cpp  （NetsysNativeClient，IPC 客户端，2376 行）
  → SA 1158（netsysnative 进程：netsys_native_service_stub.cpp → NetsysNativeService）
  → manager/* / netsys/*（netlink、iptables）/ bpf/*、bpf/bpf_progs
```

❌ 违反示例：

```cpp
// 任何 service/业务代码直接 include netsysnative proxy 或裸调 iptables/netlink
NetsysNativeServiceProxy::GetInstance()->SetInterfaceUp(...);   // 绕过 controller
system("ip route add ...");                                      // 数据面脱管
```

✅ 正确做法：经 `NetsysController` 门面方法调用；内核面新增能力时在 controller + SA 1158 两侧成对扩展（见 [../code_map/netsys-native-map.md](../code_map/netsys-native-map.md)）。

- 为什么：`mock_netsys_native_client.cpp` 依赖该通道做单测替换；绕过即失去可测性，且内核态与用户态锁/生命周期失去统一管理（firewall/bpf 资源由 1158 进程持有）。
- 违反后果：产品上多进程同时改 iptables/BPF ⇒ 规则互相覆盖；单测全线 mock 失效。

## INV-2 双进程职责边界

| 进程 | 承载 | 禁止 |
|---|---|---|
| `netmanager` | 管理面 SA：1151 连接、1152 策略、1153 统计（并承载 1154/1155/1156/1157 profile 对应服务的进程位） | 直接操作数据面（iptables/BPF/netlink） |
| `netsysnative` | 内核面执行：1158（路由/DNS/带宽/防火墙/BPF） | 承载面向应用的管理语义 |

84 字 SA 拓扑详情见 [../code_map/sa-architecture.md](../code_map/sa-architecture.md)。任何"把命令从 1158 挪到 1151"或反向的调整，都改变权限模型与故障域，必须走架构评审。

## INV-3 proxy/stub 成对与固定目录

- 常规位置：`frameworks/native/<模块>/src/proxy/` ↔ `services/<模块>/src/stub/`。
- 例外清单（**现状如此，新增代码不得再添例外**）：`frameworks/native/netpolicyclient/src/net_policy_service_proxy.cpp`（`src/` 直下）、`frameworks/native/netmanagernative/netsys_native_service_proxy.cpp`（模块根目录）。
- 回调对：客户端 `*_callback_stub`（收）↔ 服务端 `*_callback_proxy`（发），两端注册命令字同步。
- ⛔ 任何单侧变更（改命令不动 proxy、只加 proxy 不加 stub 分支）都直接破坏 INV-3，同时触发 [ipc-security-constraints.md](ipc-security-constraints.md) 的契约红线。

## INV-4 接口与常量基线在 `interfaces/`

- 公共 API（JS/NDK/innerkits）以 `interfaces/` 为稳定性基线；行为变更须对照四套绑定（`frameworks/js/napi/`、`frameworks/ets/ani/`、`frameworks/cj/`、`interfaces/kits/c/`）逐一核对后才能合入。
- IPC 命令字枚举只在 `interfaces/innerkits/<模块>/include/` 尾部追加（禁止复用/插队）。
- 错误码唯一登记点 `interfaces/innerkits/include/net_manager_constants.h`；权限唯一登记点 `utils/common_utils/include/netmanager_base_permission.h`；观测唯一声明点 `hisysevent.yaml`（domain `NETMANAGER`）。三大基线**禁止散落定义**。

## INV-5 多语言绑定与主链路平行推进

`interfaces/kits/js` + `frameworks/js/napi`（JS）、`frameworks/ets/ani`（ArkTS Rust ANI）、`frameworks/cj`（仓颉）、`interfaces/kits/c/netconnclient`（NDK）描述同一能力。**能力变更不同步绑定 = 接口割裂**——但不改变 ABI/语义的纯内部实现不需动绑定。判断口径：改了 `interfaces/` 下任何声明 → 四套全核对。

## INV-6 base ↔ ext 分工（跨仓不变量）

- 本仓 `sa_profile/` 持有 **1154 tether / 1155 vpn / 1157 ethernet** 的 profile 与 220 系列错误码定义，但**实现代码在 netmanager_ext 仓**；1156 dns_resolver 仅有 profile。
- 修改这三个 SA 的 profile、错误码或 IPC 面语义 ⇒ 必须与 ext 仓联动变更（两仓配套 MR，见 [../code_map/sa-architecture.md](../code_map/sa-architecture.md) §4）。
- 反向约束：ext 侧新增能力若需 base 内核通道，必须经本仓 `NetsysController` → SA 1158 落地，不得在 ext 进程内自行抓数据面。

## INV-7 回调回 JS 线程的唯一通道

NAPI 异步结果/事件回调统一经 `utils/napi_utils`（与 netstack 同源模式）切回 JS 线程；⛔ 禁止在工作线程直接调 JS 回调。历史投标：044d5d70（`network_exec.cpp` 多线程 map 崩溃）、f2e10ae5（napi 异步资源泄漏，`base_async_work.h`）。详见 [concurrency-failure-modes.md](concurrency-failure-modes.md) 模式一/模式二与 [historical-failures.md](historical-failures.md)。

## INV-8 特性开关两态成立

`netmanager_base_config.gni` 全部特性开关（`netmanager_base_enable_feature_net_firewall`/`_sysvpn`/`_hosts`/`netmanager_base_enable_public_dns_server`/`_pac_proxy`/`_traffic_statistic`/`netmanager_base_share_traffic_limit_enable` 多默认 false、`netmanager_base_extended_features` 默认 true）下的条件编译代码，**在开/关两种配置下都必须编译通过且测试跑过**。条件分支只写一半 ⇒ 某形态产品 IPC 面残缺或直接编不过。

## 违反处置

| 不变量被破坏的典型信号 | 处置 |
|---|---|
| 单测里 mock 失效 / mock 对象没被走到 | 检查 INV-1（是否绕过了 controller） |
| 产品端"调了没反应"或字段错乱 | 检查 INV-3（proxy/stub 单侧或不对称） |
| arkts/仓颉/NDK 用例与新行为对不上 | 检查 INV-5（绑定漏同步） |
| ext 侧功能不生效/错误码对不上 | 检查 INV-6（两仓未联动） |
| 某形态编译失败或运行缺接口 | 检查 INV-8（开关单态） |

## 关联知识

- [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md)：分层、目录、三大基线全表
- [../code_map/sa-architecture.md](../code_map/sa-architecture.md)：8 SA 拓扑与 profile 修改约束
- [ipc-security-constraints.md](ipc-security-constraints.md)：INV-3/INV-4 的安全细则
- [concurrency-failure-modes.md](concurrency-failure-modes.md)：INV-7 背后的线程模型
- [../verify/verification-loop.md](../verify/verification-loop.md)：改动后按矩阵验证

## 适用场景

- 方案设计期判断"这样改是否破坏架构"
- 评审 MR 时识别越层/绕通道/单侧变更的风险信号
- 新人建立"哪些位置绝对不能放业务代码"的边界感
