// Copyright (C) 2025 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#[ani_rs::ani(path = "L@ohos/net/connection/connection/NetConnectionInner")]
pub struct NetConnection {
    pub native_ptr: i64,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/Cleaner")]
pub struct Cleaner {
    pub native_ptr: i64,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/NetSpecifier")]
pub struct NetSpecifier {
    pub net_capabilities: NetCapabilities,

    pub bearer_private_identifier: Option<String>,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/NetCapabilityInfo")]
pub struct NetCapabilityInfo {
    pub net_handle: NetHandle,
    pub net_cap: NetCapabilities,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/NetHandleInner")]
pub struct NetHandle {
    pub net_id: i32,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/NetCapabilitiesInner")]
pub struct NetCapabilities {
    pub link_up_bandwidth_kbps: Option<i32>,

    pub link_down_bandwidth_kbps: Option<i32>,

    pub network_cap: Option<Vec<NetCap>>,

    pub bearer_types: Vec<NetBearType>,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/NetConnectionPropertyInfo")]
pub struct NetConnectionPropertyInfo {
    pub net_handle: NetHandle,
    pub connection_properties: ConnectionProperties,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/NetBlockStatusInfoInner")]
pub struct NetBlockStatusInfo {
    pub net_handle: NetHandle,
    pub blocked: bool,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/NetCap")]
pub enum NetCap {
    NetCapabilityMms = 0,
    NetCapabilityNotMetered = 11,
    NetCapabilityInternet = 12,
    NetCapabilityNotVpn = 15,
    NetCapabilityValidated = 16,
    NetCapabilityPortal = 17,
    NetCapabilityCheckingConnectivity = 31,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/NetBearType")]
pub enum NetBearType {
    BearerCellular = 0,
    BearerWifi = 1,
    BearerBluetooth = 2,
    BearerEthernet = 3,
    BearerVpn = 4,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/ConnectionPropertiesInner")]
pub struct ConnectionProperties {
    pub interface_name: String,

    pub domains: String,

    pub link_addresses: Vec<LinkAddress>,

    pub dnses: Vec<NetAddress>,

    pub routes: Vec<RouteInfo>,

    pub mtu: i32,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/RouteInfoInner")]
pub struct RouteInfo {
    pub iface: String,

    pub destination: LinkAddress,

    pub gateway: NetAddress,

    pub has_gateway: bool,

    pub is_default_route: bool,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/LinkAddressInner")]
pub struct LinkAddress {
    pub address: NetAddress,
    pub prefix_length: i32,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/NetAddressInner")]
pub struct NetAddress {
    pub address: String,

    pub family: Option<i32>,

    pub port: Option<i32>,
}

#[ani_rs::ani(path = "L@ohos/net/connection/connection/HttpProxyInner")]
pub struct HttpProxy {
    pub host: String,

    pub port: i32,

    pub username: Option<String>,

    pub password: Option<String>,

    pub exclusion_list: Vec<String>,
}
