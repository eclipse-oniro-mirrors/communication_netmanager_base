# 编译与测试命令（verify）

> 本文给出 netmanager_base 的编译方式、单测/fuzzer target 实名表、特性开关与观测点。改动后的最小验证集选择见 [verification-loop.md](verification-loop.md)。

## 1. 编译命令

```bash
# SA 目标（编译期用）
./build.sh --product-name rk3568 --build-target net_conn_manager --ccache
./build.sh --product-name rk3568 --build-target net_policy_manager --ccache
./build.sh --product-name rk3568 --build-target net_stats_manager --ccache
./build.sh --product-name rk3568 --build-target netmanagernative --ccache

# 聚合组
./build.sh --product-name rk3568 --build-target netmanager_base_services --ccache   # services 组
./build.sh --product-name rk3568 --build-target unittest --ccache                   # 全量测试触发方式之一
```

- 测试聚合 target：`//foundation/communication/netmanager_base/test:unittest`。
- **特性开关相关代码（见 §3）必须在开/关两种配置下都编译通过**；按形态切换可用 `--gn-args` 传开关（以 `netmanager_base_config.gni` 的 `declare_args` 实名为准）。

## 2. 测试 target 实名表

### 2.1 单元测试（按 `test/<module>/unittest/` 目录分组，target 实名已核对 BUILD.gn）

| 模块目录 | target 实名 |
|---|---|
| netconnmanager | `net_conn_manager_test`、`net_conn_service_test`、`net_conn_service_regional_test`、`net_conn_multi_test`、`net_conn_multi_vnic_test`、`net_supplier_test`、`net_factoryreset_callback_test`、`net_info_test`、`netmgr_distributed_test`、`net_access_policy_database_test`（布局在 netpolicymanager，见下）、`net_pac_manager_test`、`net_client_pac_file_url_test`、`local_proxy_server_test`、`pac_http_test`、`proxy_switch_mode_test`、`networkshare_manager_test` |
| nethttpclient | `net_http_client_test`（共享 HTTP 客户端 `utils/net_http_client`：同步/异步/超时/取消/代理/TLS/resolve 注入） |
| netpolicymanager | `net_policy_service_test`、`net_policy_service_nocfi_test`、`net_policy_manager_test`、`net_access_policy_database_test` |
| netstatsmanager | `net_stats_manager_test`、`net_stats_service_new_test`、`net_stats_cached_service_test`、`net_stats_service_common_test`、`net_stats_traffic_plan_service_test`、`net_stats_data_clone_test`、`net_stats_mock_data` |
| netmanagernative | `netsys_native_manager_test`（同 BUILD 内另有 `netsys_native_manager_fault_through_test`、`netsys_native_manager_netsys_native_service_test`）、`netsys_controller_test`、`netsys_native_client_test`、`netsys_native_service_proxy_test`、`netsys_dnsresolv_test`、`netsys_dnsresolv_mock_test`、`firewall_manager_test`、`firewall_manager_class_test`、`bandwidth_manager_test`、`bandwidth_manager_class_test`、`netsys_netfirewall_test`、`netsys_wrapper_test`、`netsys_distributed_test`、`resolver_config_test`、`conn_manager_test`、`nat464_clat_test`、`wearable_distributed_net_manager_test`、`suffix_match_trie_test` |
| netsysbpfstats | `netsys_bpf_stats_test` |
| netmanagerutils | `netmanager_utils_test`、`netmanager_datashare_test`、`netmanager_datashare_iface_test` |
| netmanagerconnection | `icu_helper_test` |
| netcommon | `net_common_test` |

> 备注：新增 target 后回填本表；目录间少量交叉以各 `BUILD.gn` 实名为准（如 `net_access_policy_database_test` 布局在 netpolicymanager）。

### 2.2 Fuzzer（组 target `test/fuzztest:fuzztest`，以下为 `--build-target` 实名，已核对 `ohos_fuzztest` 声明）

| fuzzer 实名 | 覆盖面 |
|---|---|
| `NetConnClientFuzzTest` | SA 1151 proxy 组包/解包红区 |
| `NetPolicyClientFuzzTest` | SA 1152 proxy 组包/解包红区 |
| `NetStatsClientFuzzTest` | SA 1153 proxy 组包/解包红区 |
| `NetSysNetFirewallFuzzTest` | 防火墙命令/stub |
| `NfQueueFuzzTest` | nfqueue 数据面回调（**仅 `netmanager_base_enable_feature_net_firewall=true` 时入组**） |
| `CommonFuzzTest` | 公共工具/权限入口 |
| `EnableVnicNetworkFuzzTest` | netsys vnic 使能分支 |
| `SetPresetDomainsFuzzTest` | netsys 预置域名 IPC 命令（`NETSYS_SET_PRESET_DOMAINS` stub 解包红区） |
| `IsDeadFlowResetTargetBundleFuzzTest` | 死流复位目标判定分支 |
| `NetBaseBranchFuzzTest` | netsys 分支综合入口 |
| `UpdateNetCapsFuzzTest` | 网络能力更新分支 |

> ⚠️ 口径警示：`test/fuzztest/netsysclient_fuzzer/` 目录仅存 `net_sys_client_fuzzer.cpp` 源文件、**无 BUILD.gn，未注册进 fuzz 组（不可构建）**——不要在验证矩阵里引用它；如需启用须先补 BUILD.gn 并登记组 deps。

**新增 IPC 命令必须给对应 fuzzer 补 case**（见 [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md) 第 ⑦ 步）。

## 3. 特性开关（`netmanager_base_config.gni`）

| 开关 | 默认 | 影响面 |
|---|---|---|
| `netmanager_base_enable_feature_net_firewall` | false | 294 系列错误码、`NetSysNetFirewallFuzzTest`、FirewallRule/BPF 防火墙链；`NfQueueFuzzTest` 仅在开启时进 fuzz 组 |
| `netmanager_base_enable_feature_sysvpn` | false | 1155 面 sysvpn 相关命令 |
| `netmanager_base_enable_feature_hosts` | false | hosts 特性分支 |
| `netmanager_base_enable_public_dns_server` | false | 公共 DNS server 逻辑 |
| `netmanager_base_enable_pac_proxy` | false | PAC 求值（curl/jerryscript，`net_pac_manager.cpp`） |
| `netmanager_base_enable_traffic_statistic` | false | 流量统计特性面 |
| `netmanager_base_share_traffic_limit_enable` | false | 共享限额（`services/netstatslimitntf` 通知链） |
| `netmanager_base_extended_features` | true | 扩展特性总闸 |

run 开关专项测试时按对应模块矩阵（§2.1）选 target；**两种开关配置都要过**（[../expert/architecture-invariants.md](../expert/architecture-invariants.md) INV-8）。

## 4. 观测点与维测开关

- `hisysevent.yaml`（domain `NETMANAGER`）：连接 `NET_CONN_SUPPLER_FAULT/STAT`、`NET_CONN_REQUEST_*`、`NET_CONN_MONITOR_*`；共享 `NET_SHARING_SETUP/CANCEL_FAULT`；VPN `NET_VPN_CONNECT_FAULT`/`VPN_OPERATE_FAULT`；防火墙 `NET_FIREWALL_CONF_*/REQ_*/INIT_FAULT`；mDNS `NET_MDNS_REQUEST_*`；HTTP 性能 `HTTP_PERF`/`NET_STACK_HTTP_RESPONSE_ERROR`。
- 新增事件必须同步 `hisysevent.yaml`；级别与参数对齐邻近既有事件。
- 排障抓取：hilog domain 过滤 `NETMANAGER` + hisysevent 查询命令按平台工具手册。

## 5. 红区大文件（改动必绑回归）

| 文件 | 行数 |
|---|---|
| `services/netconnmanager/src/net_conn_service.cpp` | 4787 |
| `frameworks/native/netmanagernative/netsys_native_service_proxy.cpp` | 4548 |
| `services/netmanagernative/src/netsys_native_service_stub.cpp` | 3238 |
| `services/netstatsmanager/src/net_stats_service.cpp` | 2647 |
| `frameworks/native/netconnclient/src/proxy/net_conn_service_proxy.cpp` | 2607 |
| `services/netconnmanager/src/stub/net_conn_service_stub.cpp` | 2416 |
| `services/netsyscontroller/src/netsys_native_client.cpp` | 2376 |
| `services/netsyscontroller/src/netsys_controller.cpp` | 2040 |
| `services/netmanagernative/src/netsys_native_service.cpp` | 1879 |
| `services/netstatsmanager/src/net_stats_cached.cpp` | 1393 |

红区行为改动按 [verification-loop.md](verification-loop.md) §2 对应行绑定并发/回归用例（历史案例见 [../expert/historical-failures.md](../expert/historical-failures.md)）。

## 关联知识

- [verification-loop.md](verification-loop.md)：按任务类型选最小验证集、Done 定义与回退流程
- [../code_map/netmanager-base-code-map.md](../code_map/netmanager-base-code-map.md)：§9 测试组织与模块目录对照
- [../routing/ipc-extension-routing.md](../routing/ipc-extension-routing.md)：新命令的 fuzzer 覆盖要求
- [../expert/architecture-invariants.md](../expert/architecture-invariants.md)：开关两态不变量

## 适用场景

- 改动后选择编译/测试命令跑最小验证集
- 新增 test target 后回填本文 §2 实名表
- 排障时查观测事件与抓取口径
