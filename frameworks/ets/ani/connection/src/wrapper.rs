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

use std::ffi::CStr;

use ani_rs::AniEnv;
use ani_sys::ani_ref;
use cxx::let_cxx_string;
use ffi::NetHandle;
use serde::{Deserialize, Serialize};

use crate::bridge;

pub struct NetConnClient;

impl NetConnClient {
    pub fn get_default_net_handle() -> Result<bridge::NetHandle, i32> {
        let mut ret = 0;
        let net_handle = ffi::GetDefaultNetHandle(&mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(net_handle.into())
    }

    pub fn get_all_nets() -> Result<Vec<bridge::NetHandle>, i32> {
        let mut ret = 0;
        let net_handles = ffi::GetAllNets(&mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(net_handles.into_iter().map(Into::into).collect())
    }

    pub fn has_default_net() -> Result<bool, i32> {
        let mut ret = 0;
        let has_default_net = ffi::HasDefaultNet(&mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(has_default_net)
    }

    pub fn get_net_capabilities(handle: bridge::NetHandle) -> Result<bridge::NetCapabilities, i32> {
        let mut ret = 0;
        let net_capabilities = ffi::GetNetCapabilities(&handle.into(), &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(net_capabilities.into())
    }

    pub fn get_default_http_proxy() -> Result<bridge::HttpProxy, i32> {
        let mut ret = 0;
        let default_http_proxy = ffi::GetDefaultHttpProxy(&mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(default_http_proxy.into())
    }

    pub fn get_global_http_proxy() -> Result<bridge::HttpProxy, i32> {
        let mut ret = 0;
        let global_http_proxy = ffi::GetGlobalHttpProxy(&mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(global_http_proxy.into())
    }

    pub fn set_global_http_proxy(proxy: bridge::HttpProxy) -> Result<(), i32> {
        let ret = ffi::SetGlobalHttpProxy(&proxy.into());
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn get_app_net() -> Result<i32, i32> {
        let net_conn_client = ffi::GetNetConnClient(&mut 0);

        let mut net_id = 0;
        let ret = net_conn_client.GetAppNet(&mut net_id);
        if ret != 0 {
            return Err(ret);
        }
        Ok(net_id)
    }

    pub fn set_app_net(net_id: i32) -> Result<(), i32> {
        let net_conn_client = ffi::GetNetConnClient(&mut 0);

        let ret = net_conn_client.SetAppNet(net_id);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn set_airplane_mode(enable: bool) -> Result<(), i32> {
        let net_conn_client = ffi::GetNetConnClient(&mut 0);

        let ret = net_conn_client.SetAirplaneMode(enable);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn set_pac_url(pac_url: &str) -> Result<(), i32> {
        let net_conn_client = ffi::GetNetConnClient(&mut 0);

        let_cxx_string!(pac_url = pac_url);
        let ret = net_conn_client.SetPacUrl(&pac_url);
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn get_pac_url() -> Result<String, i32> {
        let net_conn_client = ffi::GetNetConnClient(&mut 0);

        let_cxx_string!(pac_url = "");
        let ret = net_conn_client.GetPacUrl(pac_url.as_mut());
        if ret != 0 {
            return Err(ret);
        }
        Ok(pac_url.to_string())
    }

    pub fn factory_reset_network() -> Result<(), i32> {
        let net_conn_client = ffi::GetNetConnClient(&mut 0);

        let ret = net_conn_client.FactoryResetNetwork();
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }

    pub fn is_default_net_metered() -> Result<bool, i32> {
        let net_conn_client = ffi::GetNetConnClient(&mut 0);

        let mut is_metered = false;
        let ret = ffi::isDefaultNetMetered(&mut is_metered);
        if ret != 0 {
            return Err(ret);
        }
        Ok(is_metered)
    }

    pub fn get_connection_properties(net_id: i32) -> Result<bridge::ConnectionProperties, i32> {
        let mut ret = 0;
        let connection_properties = ffi::GetConnectionProperties(net_id, &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(connection_properties.into())
    }

    pub fn get_addresses_by_name(host: &str, net_id: i32) -> Result<Vec<bridge::NetAddress>, i32> {
        let_cxx_string!(host = host);

        let mut ret = 0;
        let addresses = ffi::getAddressesByName(&host, net_id, &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(addresses.into_iter().map(Into::into).collect())
    }
}

impl From<NetHandle> for bridge::NetHandle {
    fn from(handle: NetHandle) -> Self {
        bridge::NetHandle {
            net_id: handle.net_id,
        }
    }
}

impl From<bridge::NetHandle> for NetHandle {
    fn from(handle: bridge::NetHandle) -> Self {
        NetHandle {
            net_id: handle.net_id,
        }
    }
}

impl From<ffi::NetCap> for bridge::NetCap {
    fn from(value: ffi::NetCap) -> Self {
        match value {
            ffi::NetCap::NET_CAPABILITY_MMS => bridge::NetCap::NetCapabilityMms,
            ffi::NetCap::NET_CAPABILITY_NOT_METERED => bridge::NetCap::NetCapabilityNotMetered,
            ffi::NetCap::NET_CAPABILITY_INTERNET => bridge::NetCap::NetCapabilityInternet,
            ffi::NetCap::NET_CAPABILITY_NOT_VPN => bridge::NetCap::NetCapabilityNotVpn,
            ffi::NetCap::NET_CAPABILITY_VALIDATED => bridge::NetCap::NetCapabilityValidated,
            ffi::NetCap::NET_CAPABILITY_PORTAL => bridge::NetCap::NetCapabilityPortal,
            ffi::NetCap::NET_CAPABILITY_CHECKING_CONNECTIVITY => {
                bridge::NetCap::NetCapabilityCheckingConnectivity
            }
            _ => unimplemented!(),
        }
    }
}

impl From<ffi::NetBearType> for bridge::NetBearType {
    fn from(value: ffi::NetBearType) -> Self {
        match value {
            ffi::NetBearType::BEARER_CELLULAR => bridge::NetBearType::BearerCellular,
            ffi::NetBearType::BEARER_WIFI => bridge::NetBearType::BearerWifi,
            ffi::NetBearType::BEARER_BLUETOOTH => bridge::NetBearType::BearerBluetooth,
            ffi::NetBearType::BEARER_ETHERNET => bridge::NetBearType::BearerEthernet,
            ffi::NetBearType::BEARER_VPN => bridge::NetBearType::BearerVpn,
            _ => unimplemented!(),
        }
    }
}

impl From<ffi::NetCapabilities> for bridge::NetCapabilities {
    fn from(value: ffi::NetCapabilities) -> Self {
        bridge::NetCapabilities {
            link_up_bandwidth_kbps: Some(value.linkUpBandwidthKbps),
            link_down_bandwidth_kbps: Some(value.linkDownBandwidthKbps),
            network_cap: Some(value.networkCap.into_iter().map(Into::into).collect()),
            bearer_types: value.bearerTypes.into_iter().map(Into::into).collect(),
        }
    }
}

impl From<ffi::HttpProxy> for bridge::HttpProxy {
    fn from(value: ffi::HttpProxy) -> Self {
        bridge::HttpProxy {
            host: value.host,
            port: value.port,
            username: Some(value.username),
            password: Some(value.password),
            exclusion_list: value.exclusionList,
        }
    }
}

impl From<ffi::LinkAddress> for bridge::LinkAddress {
    fn from(value: ffi::LinkAddress) -> Self {
        bridge::LinkAddress {
            address: bridge::NetAddress {
                address: value.address.address,
                family: if value.address.family == 0 {
                    None
                } else {
                    Some(value.address.family)
                },
                port: if value.address.port == 0 {
                    None
                } else {
                    Some(value.address.port)
                },
            },
            prefix_length: value.prefix_length,
        }
    }
}

impl From<ffi::NetAddress> for bridge::NetAddress {
    fn from(value: ffi::NetAddress) -> Self {
        bridge::NetAddress {
            address: value.address,
            family: if value.family == 0 {
                None
            } else {
                Some(value.family)
            },
            port: if value.port == 0 {
                None
            } else {
                Some(value.port)
            },
        }
    }
}

impl From<ffi::RouteInfo> for bridge::RouteInfo {
    fn from(value: ffi::RouteInfo) -> Self {
        bridge::RouteInfo {
            iface: value.interface,
            destination: value.destination.into(),
            gateway: value.gateway.into(),
            has_gateway: value.has_gateway,
            is_default_route: value.is_default_route,
        }
    }
}

impl From<ffi::ConnectionProperties> for bridge::ConnectionProperties {
    fn from(value: ffi::ConnectionProperties) -> Self {
        bridge::ConnectionProperties {
            interface_name: value.interface_name,
            domains: value.domains,
            link_addresses: value.link_addresses.into_iter().map(Into::into).collect(),
            dnses: value.dnses.into_iter().map(Into::into).collect(),
            routes: value.routes.into_iter().map(Into::into).collect(),
            mtu: value.mtu,
        }
    }
}

impl From<bridge::HttpProxy> for ffi::HttpProxy {
    fn from(value: bridge::HttpProxy) -> Self {
        ffi::HttpProxy {
            host: value.host,
            port: value.port,
            username: value.username.unwrap_or_default(),
            password: value.password.unwrap_or_default(),
            exclusionList: value.exclusion_list,
        }
    }
}

#[cxx::bridge(namespace = "OHOS::NetManagerAni")]
mod ffi {

    #[namespace = "OHOS::NetManagerStandard"]
    #[repr(i32)]
    pub enum NetCap {
        NET_CAPABILITY_MMS = 0,
        NET_CAPABILITY_NOT_METERED = 11,
        NET_CAPABILITY_INTERNET = 12,
        NET_CAPABILITY_NOT_VPN = 15,
        NET_CAPABILITY_VALIDATED = 16,
        NET_CAPABILITY_PORTAL = 17,
        NET_CAPABILITY_CHECKING_CONNECTIVITY = 31,
    }

    #[namespace = "OHOS::NetManagerStandard"]
    #[repr(i32)]
    pub enum NetBearType {
        BEARER_CELLULAR = 0,
        BEARER_WIFI = 1,
        BEARER_BLUETOOTH = 2,
        BEARER_ETHERNET = 3,
        BEARER_VPN = 4,
    }

    pub struct NetHandle {
        pub net_id: i32,
    }

    pub struct NetCapabilities {
        linkUpBandwidthKbps: i32,
        linkDownBandwidthKbps: i32,
        networkCap: Vec<NetCap>,
        bearerTypes: Vec<NetBearType>,
    }

    pub struct HttpProxy {
        host: String,
        port: i32,
        username: String,
        password: String,
        exclusionList: Vec<String>,
    }

    pub struct LinkAddress {
        pub address: NetAddress,
        pub prefix_length: i32,
    }

    pub struct NetAddress {
        pub address: String,
        pub family: i32,
        pub port: i32,
    }

    pub struct RouteInfo {
        pub interface: String,

        pub destination: LinkAddress,

        pub gateway: NetAddress,

        pub has_gateway: bool,

        pub is_default_route: bool,
    }

    pub struct ConnectionProperties {
        pub interface_name: String,
        pub domains: String,
        pub link_addresses: Vec<LinkAddress>,

        pub dnses: Vec<NetAddress>,

        pub routes: Vec<RouteInfo>,

        pub mtu: i32,
    }

    extern "Rust" {}
    unsafe extern "C++" {
        include!("connection_ani.h");
        include!("net_all_capabilities.h");

        #[namespace = "OHOS::NetManagerStandard"]
        type NetCap;
        #[namespace = "OHOS::NetManagerStandard"]
        type NetBearType;
        #[namespace = "OHOS::NetManagerStandard"]
        type NetConnClient;

        fn GetDefaultNetHandle(ret: &mut i32) -> NetHandle;
        fn GetAllNets(ret: &mut i32) -> Vec<NetHandle>;
        fn HasDefaultNet(ret: &mut i32) -> bool;
        fn GetNetCapabilities(handle: &NetHandle, ret: &mut i32) -> NetCapabilities;

        fn GetDefaultHttpProxy(ret: &mut i32) -> HttpProxy;
        fn GetGlobalHttpProxy(ret: &mut i32) -> HttpProxy;
        fn SetGlobalHttpProxy(proxy: &HttpProxy) -> i32;

        fn GetNetConnClient(_: &mut i32) -> Pin<&'static mut NetConnClient>;

        fn SetAppNet(self: Pin<&mut NetConnClient>, net_id: i32) -> i32;

        fn GetAppNet(self: Pin<&mut NetConnClient>, net_id: &mut i32) -> i32;

        fn SetAirplaneMode(self: Pin<&mut NetConnClient>, enable: bool) -> i32;

        fn isDefaultNetMetered(is_metered: &mut bool) -> i32;

        fn GetPacUrl(self: Pin<&mut NetConnClient>, pac_url: Pin<&mut CxxString>) -> i32;

        fn SetPacUrl(self: Pin<&mut NetConnClient>, pac_url: &CxxString) -> i32;

        fn FactoryResetNetwork(self: Pin<&mut NetConnClient>) -> i32;

        fn GetConnectionProperties(net_id: i32, ret: &mut i32) -> ConnectionProperties;

        fn getAddressesByName(host: &CxxString, net_id: i32, ret: &mut i32) -> Vec<NetAddress>;
    }
}
