# IPC / 权限 / Parcel 安全约束（expert）

> 本文是所有跨进程接口（proxy/stub/回调/权限/错误码）改动的**安全红线**。
> 姊妹篇 [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md) 给出"怎么做的 8 步"；本文回答"为什么必须这么做、漏做会出什么事故"。

## 1. 权限中枢：一个文件，一个出口

权限字符串常量**唯一来源**是 `utils/common_utils/include/netmanager_base_permission.h` 的 `Permission` 命名空间，共 18 个常量（全表见 [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md) §6）。

统一校验入口（`NetManagerPermission`）：

| 入口 | 用途 |
|---|---|
| `CheckPermission` | 单权限校验（stub 层标准入口） |
| `CheckPermissionWithCache` | 高频命令的带缓存校验 |
| `IsSystemCaller` | 仅系统调用者（系统 API 的第一道门） |
| `CheckNetSysInternalPermission` | netsys 内部命令（SA 1158 面） |
| `CheckUidPermission` | uid 白名单校验（如 `UID_FOUNDATION`） |
| `GetApiVersion` | API 版本语义门（配合系统 API 判定） |

⛔ **IPC-S1 禁止绕过权限中枢**：任何 stub 分支不得手写权限字符串字面量、不得自行调用 `AccessTokenKit` 散落校验。

- 依据：权限集中登记是本仓安全评审的检查基线；散落校验无法随权限变更收敛。
- 漏做后果：权限常量改名/收紧时漏改散点 ⇒ 校验形同虚设；评审工具按 `netmanager_base_permission.h` 扫描，散点直接判违规。

## 2. stub 权限映射表（`g_codeNPS` 模式）

权限校验**集中在 stub 层**，通过"命令字 → 权限"映射表实现：

```
// services/netpolicymanager/src/stub/net_policy_service_stub.cpp:30 起（模式参考）
const std::map<uint32_t, const char *> g_codeNPS = {
    {PolicyInterfaceCode::CMD_SET_POLICY_BY_UID, Permission::MANAGE_NET_STRATEGY},
    ...
};
// 公开自查询类命令另行登记空权限表（g_pubCodeNPS），不得混入 g_codeNPS
```

✅ 新增管理面命令：映射表登记 `Permission::Xxx` → `OnRemoteRequest` 分支首行 `CheckPermission` 拒绝即回 `ERR_PERMISSION_DENIED` 相关码。
✅ 新增公开查询命令：登记进公开表，并确认**不携带**任何改状态/读敏感数据的语义。
❌ 直接在 service 实现层补权限（stub 放行）：任何绕过 stub 直达 service 的调用路径（进程内直调、mock、自测 gadget）都能穿透校验。

特殊分支参考：`services/netconnmanager/src/stub/net_conn_service_stub.cpp:317` 附近的 `CMD_NM_SET_INTERNET_PERMISSION`——除 `Permission::CONNECTIVITY_INTERNAL` 外叠加 uid 检查（`IPCSkeleton::GetCallingUid()` + `CheckUidPermission`）。**"权限 + 特定 uid"双门的命令，两道都必须保留**。

⛔ **IPC-S2 新命令不登记映射表 = 越权漏洞**（漏登记使任意三方应用可调管理面；误把管理命令登进公开表同样是漏洞）。此条是本仓历史安全缺陷排名第一的来源。

## 3. Parcel 对称与判空（proxy↔stub 契约）

**写读逐字段对称**是 IPC 唯一契约，容不下"多写少读"：

```
proxy（写）                          stub（读）
MessageParcel data;                  data.ReadInterfaceToken() 必须与
data.WriteInterfaceToken(...);       WriteInterfaceToken 成对
if (!data.WriteInt32(v)) {...}       if (!data.ReadInt32(v)) { return ERR; }
```

✅ 三条铁律：
1. `WriteInterfaceToken`/`ReadInterfaceToken` 首位成对；漏 token 直接走默认分支"没反应"。
2. **每次 `Read*` 判空**，失败立即返回错误码，不得继续用未初始化值；`Write*` 失败同样不得继续发送。
3. 字段顺序、数量两端口径一致；改字段必须 proxy/stub 同一提交内双侧变更。

⚠️ **IPC-S3 历史教训 63961c40**：`frameworks/native/netmanagernative/netsys_native_service_proxy.cpp`（4548 行红区）曾因 Parcel 读取不判空产生空指针/垃圾参数，修复为全量判空重构（278 行改动）。新代码引入"不判空的 Read"会在 fuzzer 中必然复现同类崩溃。

漏做后果速查：

| 违反 | 典型事故 |
|---|---|
| 写读不对称 | 对端按错字段解包 → 野指针长度、垃圾参数、静默错数据 |
| 漏判空 | 空指针崩溃；恶意 Parcel 越界读（安全缺陷） |
| 单侧加字段 | 另一侧 Token 之后流全部错位，后续所有命令失联 |

## 4. 回调接口的生命周期安全

回调（服务端反向通知客户端）是 UAF 高发面：

- callback stub（客户端收事件）与 callback proxy（服务端发事件）**必须成对新增**，注册命令字两端同步；
- 服务端持有回调 proxy 期间，客户端对象析构 ⇒ 悬挂引用；注册/注销命令与回调通知之间存在天然竞态，发送前须判活（如 IPC 死亡通知机制）；
- 事件回调里访问的容器必须与回调注册路径锁序一致（结合 [concurrency-failure-modes.md](concurrency-failure-modes.md) 模式二"两阶段锁"食用）。

⛔ **IPC-S4 禁止只加一端**：漏 callback stub = 事件黑洞；漏 proxy 判活 = UAF/崩溃。半成品接口在联调期才暴露，返工成本最高。

## 5. 错误码与观测纪律

- 对外失败语义**只允许**取自 `interfaces/innerkits/include/net_manager_constants.h`：本仓 IPC 主段 2100001~2100301（另有 199 内部段、2100400/2100401、防火墙 294 段、EAP 332 段，全表见 [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md) §5）。⛔ **IPC-S5 禁止自造裸数字**。
- 命令字枚举只允许追加在对应接口头尾部（`conn_ipc_interface_code.h`/`policy_ipc_interface_code.h`/`stats_ipc_interface_code.h`/`netsys_ipc_interface_code.h`），⛔ **IPC-S6 禁止复用/插队旧命令字**——复用旧值会把新请求打进旧分支（数据错解或越权执行旧逻辑）。
- 涉及新维测事件时同步 `hisysevent.yaml`（domain 固定 `NETMANAGER`），FAULT/BEHAVIOR/STATISTIC 级别对齐邻近事件。

## 6. 安全改动自检表

| 约束 | 检查点 | 勾 |
|---|---|---|
| IPC-S1 | 权限串取自 `netmanager_base_permission.h`，校验走 `NetManagerPermission` 入口 | ☐ |
| IPC-S2 | 新命令已在 `g_codeNPS`（或公开表）登记，语义归类正确 | ☐ |
| IPC-S2 | 双门命令（权限 + uid）两道校验齐全 | ☐ |
| IPC-S3 | proxy/stub Parcel 逐字段对称；全部 `Read*`/`Write*` 判空 | ☐ |
| IPC-S4 | callback stub + proxy 成对；服务端发送前判活 | ☐ |
| IPC-S5 | 新错误码已登记 `net_manager_constants.h` | ☐ |
| IPC-S6 | 命令字只在枚举尾部追加，未复用旧值 | ☐ |
| 配套 | 新命令 fuzzer case（`test/fuzztest/` 对应 fuzzer）已补 | ☐ |
| 配套 | 特性开关下的条件分支，两种配置均编译/测试 | ☐ |

## 关联知识

- [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md)：8 步 checklist 与目录对照（本文的"为什么"对应它的"怎么做"）
- [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md)：错误码/权限/观测三大基线全表
- [concurrency-failure-modes.md](concurrency-failure-modes.md)：回调容器竞态与两阶段锁
- [historical-failures.md](historical-failures.md)：63961c40 等案例台账
- [../verify/verification-loop.md](../verify/verification-loop.md)：验证矩阵与 Done 定义

## 适用场景

- 新增/修改任何 IPC 命令、回调、权限、错误码（先跑 [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md) 八步，再用本文过红线）
- 评审 IPC 类 MR：按 §6 自检表逐项查漏（漏权限登记、漏判空是最高频缺陷）
- fuzzer 报出 Parcel 越界/空指针时，按 §3 反推 proxy/stub 契约缺口
