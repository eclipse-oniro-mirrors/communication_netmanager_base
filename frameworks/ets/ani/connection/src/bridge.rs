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

use std::{ffi::CStr, sync::OnceLock};

use ani_rs::{
    error::AniError,
    global::GlobalRef,
    objects::{AniClass, AniEnum, AniMethod, AniObject, AniRef},
    signature, AniEnv,
};

#[ani_rs::ani(path = "@ohos.net.connection.connection.NetConnectionInner")]
pub struct NetConnection {
    pub native_ptr: i64,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.Cleaner")]
pub struct Cleaner {
    pub ptr: i64,
}

#[ani_rs::ani]
pub struct NetSpecifier {
    pub net_capabilities: NetCapabilities,

    pub bearer_private_identifier: Option<String>,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.NetCapabilityInfoInner")]
pub struct NetCapabilityInfo {
    pub net_handle: NetHandle,
    pub net_cap: NetCapabilities,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.NetHandleInner")]
pub struct NetHandle {
    pub net_id: i32,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.NetCapabilitiesInner")]
pub struct NetCapabilities {
    pub link_up_bandwidth_kbps: Option<i32>,

    pub link_down_bandwidth_kbps: Option<i32>,

    pub network_cap: Option<Vec<NetCap>>,

    pub bearer_types: Vec<NetBearType>,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.NetConnectionPropertyInfoInner")]
pub struct NetConnectionPropertyInfo {
    pub net_handle: NetHandle,
    pub connection_properties: ConnectionProperties,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.NetBlockStatusInfoInner")]
pub struct NetBlockStatusInfo {
    pub net_handle: NetHandle,
    pub blocked: bool,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.NetCap")]
pub enum NetCap {
    NetCapabilityMms = 0,
    NetCapabilityNotMetered = 11,
    NetCapabilityInternet = 12,
    NetCapabilityNotVpn = 15,
    NetCapabilityValidated = 16,
    NetCapabilityPortal = 17,
    NetCapabilityCheckingConnectivity = 31,
}

impl NetCap {
    pub fn get_enum_index(&self) -> usize {
        match self {
            NetCap::NetCapabilityMms => 0,
            NetCap::NetCapabilityNotMetered => 1,
            NetCap::NetCapabilityInternet => 2,
            NetCap::NetCapabilityNotVpn => 3,
            NetCap::NetCapabilityValidated => 4,
            NetCap::NetCapabilityPortal => 5,
            NetCap::NetCapabilityCheckingConnectivity => 6,
        }
    }
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.NetBearType")]
pub enum NetBearType {
    BearerCellular = 0,
    BearerWifi = 1,
    BearerBluetooth = 2,
    BearerEthernet = 3,
    BearerVpn = 4,
}

impl NetBearType {
    pub fn get_enum_index(&self) -> usize {
        match self {
            NetBearType::BearerCellular => 0,
            NetBearType::BearerWifi => 1,
            NetBearType::BearerBluetooth => 2,
            NetBearType::BearerEthernet => 3,
            NetBearType::BearerVpn => 4,
        }
    }
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.ConnectionPropertiesInner")]
pub struct ConnectionProperties {
    pub interface_name: String,

    pub domains: String,

    pub link_addresses: Vec<LinkAddress>,

    pub dnses: Vec<NetAddress>,

    pub routes: Vec<RouteInfo>,

    pub mtu: i32,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.RouteInfoInner")]
pub struct RouteInfo {
    pub iface: String,

    pub destination: LinkAddress,

    pub gateway: NetAddress,

    pub has_gateway: bool,

    pub is_default_route: bool,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.LinkAddressInner")]
pub struct LinkAddress {
    pub address: NetAddress,
    pub prefix_length: i32,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.NetAddressInner")]
pub struct NetAddress {
    pub address: String,

    pub family: Option<i32>,

    pub port: Option<i32>,
}

#[ani_rs::ani(path = "@ohos.net.connection.connection.HttpProxyInner")]
pub struct HttpProxy {
    pub host: String,

    pub port: i32,

    pub username: Option<String>,

    pub password: Option<String>,

    pub exclusion_list: Vec<String>,
}

pub trait ToAniValue {
    fn to_ani_value<'local>(
        env: &AniEnv<'local>,
        value: Self,
    ) -> Result<AniObject<'local>, AniError>;
}

const NETADDRESS_CLASS_NAME: &CStr = unsafe {
    CStr::from_bytes_with_nul_unchecked(b"@ohos.net.connection.connection.NetAddressInner\0")
};

const NETADDRESS_CTOR_SIG: &CStr =
    unsafe { CStr::from_bytes_with_nul_unchecked(b"C{std.core.String}ii:\0") };

const LINKADDRESS_CLASS_NAME: &CStr = unsafe {
    CStr::from_bytes_with_nul_unchecked(b"@ohos.net.connection.connection.LinkAddressInner\0")
};

const LINKADDRESS_CTOR_SIG: &CStr = unsafe {
    CStr::from_bytes_with_nul_unchecked(b"C{@ohos.net.connection.connection.NetAddress}i:\0")
};

const ROUTEINFO_CLASS_NAME: &CStr = unsafe {
    CStr::from_bytes_with_nul_unchecked(b"@ohos.net.connection.connection.RouteInfoInner\0")
};

const ROUTEINFO_CTOR_SIG: &CStr = unsafe {
    CStr::from_bytes_with_nul_unchecked(b"C{std.core.String}C{@ohos.net.connection.connection.LinkAddress}C{@ohos.net.connection.connection.NetAddress}zz:\0")
};

const CONNECTIONPROPERTIES_CLASS_NAME: &CStr = unsafe {
    CStr::from_bytes_with_nul_unchecked(
        b"@ohos.net.connection.connection.ConnectionPropertiesInner\0",
    )
};

const CONNECTIONPROPERTIES_CTOR_SIG: &CStr = unsafe {
    CStr::from_bytes_with_nul_unchecked(b"C{std.core.String}C{std.core.String}C{std.core.Array}C{std.core.Array}C{std.core.Array}i:\0")
};

const NETCAPABILITIES_CLASS_NAME: &CStr = unsafe {
    CStr::from_bytes_with_nul_unchecked(b"@ohos.net.connection.connection.NetCapabilitiesInner\0")
};

const NETCAPABILITIES_CTOR_SIG: &CStr =
    unsafe { CStr::from_bytes_with_nul_unchecked(b"iiC{std.core.Array}C{std.core.Array}:\0") };

const NETCAP_ENUM_NAME: &CStr =
    unsafe { CStr::from_bytes_with_nul_unchecked(b"@ohos.net.connection.connection.NetCap\0") };

const NETBEARTYPE_ENUM_NAME: &CStr = unsafe {
    CStr::from_bytes_with_nul_unchecked(b"@ohos.net.connection.connection.NetBearType\0")
};

pub struct ConnectionPropertiesClassCache {
    net_address_class_ref: GlobalRef<AniClass<'static>>,
    net_address_ctor_method: AniMethod<'static>,
    link_address_class_ref: GlobalRef<AniClass<'static>>,
    link_address_ctor_method: AniMethod<'static>,
    route_info_class_ref: GlobalRef<AniClass<'static>>,
    route_info_ctor_method: AniMethod<'static>,
    connection_properties_class_ref: GlobalRef<AniClass<'static>>,
    connection_properties_ctor_method: AniMethod<'static>,
}

unsafe impl Send for ConnectionPropertiesClassCache {}
unsafe impl Sync for ConnectionPropertiesClassCache {}

static G_CONNECTIONPROPERTIES_CLASS_CACHE: OnceLock<ConnectionPropertiesClassCache> =
    OnceLock::new();

pub struct NetCapabilitiesClassCache {
    net_capabilities_class_ref: GlobalRef<AniClass<'static>>,
    net_capabilities_ctor_method: AniMethod<'static>,
    net_cap_enum_ref: GlobalRef<AniEnum<'static>>,
    net_bear_type_enum_ref: GlobalRef<AniEnum<'static>>,
}

unsafe impl Send for NetCapabilitiesClassCache {}
unsafe impl Sync for NetCapabilitiesClassCache {}

static G_NETCAPABILITIES_CLASS_CACHE: OnceLock<NetCapabilitiesClassCache> = OnceLock::new();

fn get_class_and_method(
    env: &AniEnv,
    class_name: &CStr,
    ctor_name: &CStr,
) -> (GlobalRef<AniClass<'static>>, AniMethod<'static>) {
    let class = env.find_class(class_name).unwrap();
    let raw_method = env
        .find_method_with_signature(&class, signature::CTOR, ctor_name)
        .unwrap()
        .as_raw();
    let ctor_method = AniMethod::from_raw(raw_method);
    let class_ref = class.into_global(env).unwrap();
    (class_ref, ctor_method)
}

fn get_connection_properties_cache(env: &AniEnv) -> &'static ConnectionPropertiesClassCache {
    G_CONNECTIONPROPERTIES_CLASS_CACHE.get_or_init(|| {
        let (net_address_class_ref, net_address_ctor_method) =
            get_class_and_method(env, NETADDRESS_CLASS_NAME, NETADDRESS_CTOR_SIG);
        let (link_address_class_ref, link_address_ctor_method) =
            get_class_and_method(env, LINKADDRESS_CLASS_NAME, LINKADDRESS_CTOR_SIG);
        let (route_info_class_ref, route_info_ctor_method) =
            get_class_and_method(env, ROUTEINFO_CLASS_NAME, ROUTEINFO_CTOR_SIG);
        let (connection_properties_class_ref, connection_properties_ctor_method) =
            get_class_and_method(
                env,
                CONNECTIONPROPERTIES_CLASS_NAME,
                CONNECTIONPROPERTIES_CTOR_SIG,
            );

        ConnectionPropertiesClassCache {
            net_address_class_ref,
            net_address_ctor_method,
            link_address_class_ref,
            link_address_ctor_method,
            route_info_class_ref,
            route_info_ctor_method,
            connection_properties_class_ref,
            connection_properties_ctor_method,
        }
    })
}

fn get_net_capabilities_cache(env: &AniEnv) -> &'static NetCapabilitiesClassCache {
    G_NETCAPABILITIES_CLASS_CACHE.get_or_init(|| {
        let (net_capabilities_class_ref, net_capabilities_ctor_method) =
            get_class_and_method(env, NETCAPABILITIES_CLASS_NAME, NETCAPABILITIES_CTOR_SIG);
        let net_cap_enum_ref = env
            .find_enum(NETCAP_ENUM_NAME)
            .unwrap()
            .into_global(env)
            .unwrap();
        let net_bear_type_enum_ref = env
            .find_enum(NETBEARTYPE_ENUM_NAME)
            .unwrap()
            .into_global(env)
            .unwrap();

        NetCapabilitiesClassCache {
            net_capabilities_class_ref,
            net_capabilities_ctor_method,
            net_cap_enum_ref,
            net_bear_type_enum_ref,
        }
    })
}

impl ToAniValue for NetAddress {
    fn to_ani_value<'local>(
        env: &AniEnv<'local>,
        value: NetAddress,
    ) -> Result<AniObject<'local>, AniError> {
        let class_cache = get_connection_properties_cache(env);
        let param1 = env.convert_std_string(&value.address)?;
        let param2 = value.family.unwrap_or(0);
        let param3 = value.port.unwrap_or(0);
        env.new_object_with_ctor_method(
            &class_cache.net_address_class_ref,
            &class_cache.net_address_ctor_method,
            (param1, param2, param3),
        )
    }
}

impl ToAniValue for LinkAddress {
    fn to_ani_value<'local>(
        env: &AniEnv<'local>,
        value: LinkAddress,
    ) -> Result<AniObject<'local>, AniError> {
        let class_cache = get_connection_properties_cache(env);
        let param1 = NetAddress::to_ani_value(env, value.address)?;
        let param2 = value.prefix_length;
        env.new_object_with_ctor_method(
            &class_cache.link_address_class_ref,
            &class_cache.link_address_ctor_method,
            (param1, param2),
        )
    }
}

impl ToAniValue for RouteInfo {
    fn to_ani_value<'local>(
        env: &AniEnv<'local>,
        value: RouteInfo,
    ) -> Result<AniObject<'local>, AniError> {
        let class_cache = get_connection_properties_cache(env);
        let param1 = env.convert_std_string(&value.iface)?;
        let param2 = LinkAddress::to_ani_value(env, value.destination)?;
        let param3 = NetAddress::to_ani_value(env, value.gateway)?;
        let param4 = value.has_gateway;
        let param5 = value.is_default_route;
        env.new_object_with_ctor_method(
            &class_cache.route_info_class_ref,
            &class_cache.route_info_ctor_method,
            (param1, param2, param3, param4, param5),
        )
    }
}

impl<T: ToAniValue> ToAniValue for Vec<T> {
    fn to_ani_value<'local>(
        env: &AniEnv<'local>,
        value: Self,
    ) -> Result<AniObject<'local>, AniError> {
        let array_size = value.len();
        let array = env.new_array::<AniRef>(array_size)?;
        for (index, val) in value.into_iter().enumerate() {
            let ani_ref = T::to_ani_value(env, val)?;
            env.array_set::<AniRef>(&array, index, ani_ref.into())?;
        }
        Ok(array.into())
    }
}

impl ToAniValue for ConnectionProperties {
    fn to_ani_value<'local>(
        env: &AniEnv<'local>,
        value: ConnectionProperties,
    ) -> Result<AniObject<'local>, AniError> {
        let class_cache = get_connection_properties_cache(env);
        let param1 = env.convert_std_string(&value.interface_name)?;
        let param2 = env.convert_std_string(&value.domains)?;
        let param3 = Vec::<LinkAddress>::to_ani_value(env, value.link_addresses)?;
        let param4 = Vec::<NetAddress>::to_ani_value(env, value.dnses)?;
        let param5 = Vec::<RouteInfo>::to_ani_value(env, value.routes)?;
        let param6 = value.mtu;
        env.new_object_with_ctor_method(
            &class_cache.connection_properties_class_ref,
            &class_cache.connection_properties_ctor_method,
            (param1, param2, param3, param4, param5, param6),
        )
    }
}

impl ToAniValue for NetCap {
    fn to_ani_value<'local>(
        env: &AniEnv<'local>,
        value: NetCap,
    ) -> Result<AniObject<'local>, AniError> {
        let index = value.get_enum_index();
        let class_cache = get_net_capabilities_cache(env);
        let enum_item = env.new_enum_item_by_index(&class_cache.net_cap_enum_ref, index)?;
        Ok(enum_item.into())
    }
}

impl ToAniValue for NetBearType {
    fn to_ani_value<'local>(
        env: &AniEnv<'local>,
        value: NetBearType,
    ) -> Result<AniObject<'local>, AniError> {
        let index = value.get_enum_index();
        let class_cache = get_net_capabilities_cache(env);
        let enum_item = env.new_enum_item_by_index(&class_cache.net_bear_type_enum_ref, index)?;
        Ok(enum_item.into())
    }
}

impl ToAniValue for NetCapabilities {
    fn to_ani_value<'local>(
        env: &AniEnv<'local>,
        value: NetCapabilities,
    ) -> Result<AniObject<'local>, AniError> {
        let class_cache = get_net_capabilities_cache(env);

        let param1 = value.link_up_bandwidth_kbps.unwrap_or(0);
        let param2 = value.link_down_bandwidth_kbps.unwrap_or(0);
        let param3 = Vec::<NetCap>::to_ani_value(env, value.network_cap.unwrap_or(Vec::new()))?;
        let param4 = Vec::<NetBearType>::to_ani_value(env, value.bearer_types)?;

        env.new_object_with_ctor_method(
            &class_cache.net_capabilities_class_ref,
            &class_cache.net_capabilities_ctor_method,
            (param1, param2, param3, param4),
        )
    }
}
