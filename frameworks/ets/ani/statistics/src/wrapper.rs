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

use crate::bridge;
use crate::register::StatisticsCallback;
use cxx::let_cxx_string;
use cxx::UniquePtr;
use ffi::NetStatsChangeInfo;

pub struct NetStatsClient;

impl NetStatsClient {
    pub fn get_all_rx_bytes() -> Result<u64, i32> {
        let client = ffi::GetNetStatsClient(&mut 0);
        let mut bytes = 0;
        let ret = client.GetAllRxBytes(&mut bytes);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bytes)
    }

    pub fn get_all_tx_bytes() -> Result<u64, i32> {
        let client = ffi::GetNetStatsClient(&mut 0);
        let mut bytes = 0;
        let ret = client.GetAllTxBytes(&mut bytes);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bytes)
    }

    pub fn get_cellular_rx_bytes() -> Result<u64, i32> {
        let client = ffi::GetNetStatsClient(&mut 0);
        let mut bytes = 0;
        let ret = client.GetCellularRxBytes(&mut bytes);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bytes)
    }

    pub fn get_cellular_tx_bytes() -> Result<u64, i32> {
        let client = ffi::GetNetStatsClient(&mut 0);
        let mut bytes = 0;
        let ret = client.GetCellularTxBytes(&mut bytes);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bytes)
    }

    pub fn get_iface_rx_bytes(iface: &str) -> Result<u64, i32> {
        let client = ffi::GetNetStatsClient(&mut 0);
        let mut bytes = 0;
        let_cxx_string!(iface = iface);
        let ret = client.GetIfaceRxBytes(&mut bytes, &iface);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bytes)
    }

    pub fn get_iface_tx_bytes(iface: &str) -> Result<u64, i32> {
        let client = ffi::GetNetStatsClient(&mut 0);
        let mut bytes = 0;
        let_cxx_string!(iface = iface);
        let ret = client.GetIfaceTxBytes(&mut bytes, &iface);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bytes)
    }

    pub fn get_uid_rx_bytes(uid: u32) -> Result<u64, i32> {
        let client = ffi::GetNetStatsClient(&mut 0);
        let mut bytes = 0;
        let ret = client.GetUidRxBytes(&mut bytes, uid);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bytes)
    }

    pub fn get_uid_tx_bytes(uid: u32) -> Result<u64, i32> {
        let client = ffi::GetNetStatsClient(&mut 0);
        let mut bytes = 0;
        let ret = client.GetUidTxBytes(&mut bytes, uid);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bytes)
    }

    pub fn get_sockfd_rx_bytes(sockfd: i32) -> Result<u64, i32> {
        let client = ffi::GetNetStatsClient(&mut 0);
        let mut bytes = 0;
        let ret = client.GetSockfdRxBytes(&mut bytes, sockfd);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bytes)
    }

    pub fn get_sockfd_tx_bytes(sockfd: i32) -> Result<u64, i32> {
        let client = ffi::GetNetStatsClient(&mut 0);
        let mut bytes = 0;
        let ret = client.GetSockfdTxBytes(&mut bytes, sockfd);
        if ret != 0 {
            return Err(ret);
        }
        Ok(bytes)
    }

    pub fn get_traffic_stats_by_iface(
        iface_info: bridge::IfaceInfo,
    ) -> Result<bridge::NetStatsInfo, i32> {
        let mut ret = 0;
        let info = ffi::GetTrafficStatsByIface(&mut iface_info.into(), &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(info.into())
    }

    pub fn get_traffic_stats_by_uid(
        uid_info: bridge::UidInfo,
    ) -> Result<bridge::NetStatsInfo, i32> {
        let mut ret = 0;
        let info = ffi::GetTrafficStatsByUid(&mut uid_info.into(), &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(info.into())
    }

    pub fn get_traffic_stats_by_network(
        mut network_info: bridge::AniNetworkInfo,
    ) -> Result<Vec<bridge::AniUidNetStatsInfoPair>, i32> {
        let mut net_stats_infos_vec: std::vec::Vec<ffi::AniUidNetStatsInfoPair> = Vec::new();
        let ret =
            ffi::GetTrafficStatsByNetworkVec(&mut network_info.into(), &mut net_stats_infos_vec);
        if ret != 0 {
            return Err(ret);
        }
        Ok(net_stats_infos_vec.into_iter().map(Into::into).collect())
    }

    pub fn get_traffic_stats_by_uid_network(
        uid: i32,
        mut network_info: bridge::AniNetworkInfo,
    ) -> Result<Vec<bridge::AniNetStatsInfoSequenceItem>, i32> {
        let mut net_stats_info_sequence: std::vec::Vec<ffi::AniNetStatsInfoSequenceItem> =
            Vec::new();

        let ret = ffi::GetTrafficStatsByUidNetworkVec(
            &mut net_stats_info_sequence,
            uid.try_into().unwrap(),
            &mut network_info.into(),
        );
        if ret != 0 {
            return Err(ret);
        }

        Ok(net_stats_info_sequence
            .into_iter()
            .map(Into::into)
            .collect())
    }

    pub fn register_statis_callback(
        callback: StatisticsCallback,
    ) -> Result<StatisCallbackUnregister, i32> {
        let mut ret = 0;
        let unregister = ffi::RegisterStatisCallback(Box::new(callback), &mut ret);
        if ret != 0 {
            return Err(ret);
        }
        Ok(StatisCallbackUnregister::new(unregister))
    }
}

pub struct StatisCallbackUnregister {
    inner: UniquePtr<ffi::StatisCallbackUnregister>,
}

unsafe impl Send for StatisCallbackUnregister {}
unsafe impl Sync for StatisCallbackUnregister {}

impl StatisCallbackUnregister {
    fn new(inner: UniquePtr<ffi::StatisCallbackUnregister>) -> Self {
        Self { inner }
    }

    pub fn unregister(&mut self) -> Result<(), i32> {
        let ret = self.inner.pin_mut().Unregister();
        if ret != 0 {
            return Err(ret);
        }
        Ok(())
    }
}

impl From<ffi::NetStatsChangeInfo> for bridge::NetStatsChangeInfo {
    fn from(info: ffi::NetStatsChangeInfo) -> Self {
        bridge::NetStatsChangeInfo {
            iface: info.iface,
            uid: Some(info.uid),
        }
    }
}

impl From<ffi::NetStatsInfoInner> for bridge::NetStatsInfo {
    fn from(info: ffi::NetStatsInfoInner) -> Self {
        bridge::NetStatsInfo {
            rx_bytes: info.rx_bytes,
            tx_bytes: info.tx_bytes,
            rx_packets: info.rx_packets,
            tx_packets: info.tx_packets,
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
            ffi::NetBearType::BEARER_WIFI_AWARE => bridge::NetBearType::BearerWifiAware,
            ffi::NetBearType::BEARER_DEFAULT => bridge::NetBearType::BearerDefault,
            _ => unimplemented!(),
        }
    }
}

impl From<bridge::NetBearType> for ffi::NetBearType {
    fn from(value: bridge::NetBearType) -> Self {
        match value {
            bridge::NetBearType::BearerCellular => ffi::NetBearType::BEARER_CELLULAR,
            bridge::NetBearType::BearerWifi => ffi::NetBearType::BEARER_WIFI,
            bridge::NetBearType::BearerBluetooth => ffi::NetBearType::BEARER_BLUETOOTH,
            bridge::NetBearType::BearerEthernet => ffi::NetBearType::BEARER_ETHERNET,
            bridge::NetBearType::BearerVpn => ffi::NetBearType::BEARER_VPN,
            bridge::NetBearType::BearerWifiAware => ffi::NetBearType::BEARER_WIFI_AWARE,
            bridge::NetBearType::BearerDefault => ffi::NetBearType::BEARER_DEFAULT,
            _ => unimplemented!(),
        }
    }
}

impl From<ffi::AniNetworkInfo> for bridge::AniNetworkInfo {
    fn from(info: ffi::AniNetworkInfo) -> Self {
        bridge::AniNetworkInfo {
            type_: info.type_.into(),
            start_time: info.start_time,
            end_time: info.end_time,
            sim_id: Some(info.sim_id),
        }
    }
}

impl From<bridge::AniNetworkInfo> for ffi::AniNetworkInfo {
    fn from(info: bridge::AniNetworkInfo) -> Self {
        ffi::AniNetworkInfo {
            type_: info.type_.into(),
            start_time: info.start_time,
            end_time: info.end_time,
            sim_id: info.sim_id.unwrap_or_default(),
        }
    }
}

impl From<ffi::AniUidNetStatsInfoPair> for bridge::AniUidNetStatsInfoPair {
    fn from(value: ffi::AniUidNetStatsInfoPair) -> Self {
        bridge::AniUidNetStatsInfoPair {
            uid: value.uid,
            net_stats_info: value.net_stats_info.into(),
        }
    }
}

impl From<ffi::AniNetStatsInfoSequenceItem> for bridge::AniNetStatsInfoSequenceItem {
    fn from(value: ffi::AniNetStatsInfoSequenceItem) -> Self {
        bridge::AniNetStatsInfoSequenceItem {
            start_time: value.start_time,
            end_time: value.end_time,
            info: value.info.into(),
        }
    }
}

impl From<bridge::IfaceInfo> for ffi::IfaceInfo {
    fn from(info: bridge::IfaceInfo) -> Self {
        ffi::IfaceInfo {
            iface: info.iface,
            start_time: info.start_time,
            end_time: info.end_time,
        }
    }
}

impl From<bridge::UidInfo> for ffi::UidInfo {
    fn from(info: bridge::UidInfo) -> Self {
        ffi::UidInfo {
            uid: info.uid,
            iface_info: info.iface_info.into(),
        }
    }
}

#[cxx::bridge(namespace = "OHOS::NetManagerAni")]
pub mod ffi {
    #[namespace = "OHOS::NetManagerStandard"]
    #[repr(i32)]
    enum NetBearType {
        BEARER_CELLULAR = 0,
        BEARER_WIFI = 1,
        BEARER_BLUETOOTH = 2,
        BEARER_ETHERNET = 3,
        BEARER_VPN = 4,
        BEARER_WIFI_AWARE = 5,
        BEARER_DEFAULT,
    }

    pub struct AniNetworkInfo {
        pub type_: NetBearType,
        pub start_time: i32,
        pub end_time: i32,
        pub sim_id: i32,
    }

    pub struct NetStatsChangeInfo {
        pub iface: String,
        pub uid: i32,
    }
    pub struct NetStatsInfoInner {
        pub rx_bytes: i64,
        pub tx_bytes: i64,
        pub rx_packets: i64,
        pub tx_packets: i64,
    }

    pub struct AniNetStatsInfoSequenceItem {
        pub start_time: i32,
        pub end_time: i32,
        pub info: NetStatsInfoInner,
    }

    pub struct AniUidNetStatsInfoPair {
        pub uid: i32,
        pub net_stats_info: NetStatsInfoInner,
    }

    pub struct IfaceInfo {
        pub iface: String,
        pub start_time: i32,
        pub end_time: i32,
    }

    pub struct UidInfo {
        pub iface_info: IfaceInfo,
        pub uid: i32,
    }

    extern "Rust" {
        type StatisticsCallback;
        pub fn net_iface_stats_changed(self: &StatisticsCallback, info: NetStatsChangeInfo) -> i32;
        pub fn net_uid_stats_changed(self: &StatisticsCallback, info: NetStatsChangeInfo) -> i32;
    }

    unsafe extern "C++" {
        include!("net_stats_client.h");
        include!("statistics_ani.h");

        #[namespace = "OHOS::NetManagerStandard"]
        type NetStatsClient;
        #[namespace = "OHOS::NetManagerStandard"]
        type NetStatsInfo;
        type StatisCallbackUnregister;

        fn RegisterStatisCallback(
            callback: Box<StatisticsCallback>,
            ret: &mut i32,
        ) -> UniquePtr<StatisCallbackUnregister>;

        fn Unregister(self: &StatisCallbackUnregister) -> i32;

        fn GetNetStatsClient(_: &mut i32) -> Pin<&'static mut NetStatsClient>;

        fn GetAllRxBytes(self: Pin<&'static mut NetStatsClient>, bytes: &mut u64) -> i32;
        fn GetAllTxBytes(self: Pin<&'static mut NetStatsClient>, bytes: &mut u64) -> i32;

        fn GetCellularRxBytes(self: Pin<&'static mut NetStatsClient>, bytes: &mut u64) -> i32;
        fn GetCellularTxBytes(self: Pin<&'static mut NetStatsClient>, bytes: &mut u64) -> i32;

        fn GetIfaceRxBytes(
            self: Pin<&'static mut NetStatsClient>,
            bytes: &mut u64,
            iface: &CxxString,
        ) -> i32;
        fn GetIfaceTxBytes(
            self: Pin<&'static mut NetStatsClient>,
            bytes: &mut u64,
            iface: &CxxString,
        ) -> i32;

        fn GetUidRxBytes(self: Pin<&'static mut NetStatsClient>, bytes: &mut u64, uid: u32) -> i32;
        fn GetUidTxBytes(self: Pin<&'static mut NetStatsClient>, bytes: &mut u64, uid: u32) -> i32;

        fn GetSockfdRxBytes(
            self: Pin<&'static mut NetStatsClient>,
            bytes: &mut u64,
            sockfd: i32,
        ) -> i32;
        fn GetSockfdTxBytes(
            self: Pin<&'static mut NetStatsClient>,
            bytes: &mut u64,
            sockfd: i32,
        ) -> i32;

        fn GetTrafficStatsByIface(info: &mut IfaceInfo, ret: &mut i32) -> NetStatsInfoInner;

        fn GetTrafficStatsByUid(uidInfo: &mut UidInfo, ret: &mut i32) -> NetStatsInfoInner;

        fn GetTrafficStatsByNetworkVec(
            networkInfo: &mut AniNetworkInfo,
            infos: &mut Vec<AniUidNetStatsInfoPair>,
        ) -> i32;

        fn GetTrafficStatsByUidNetworkVec(
            net_stats_info_sequence: &mut Vec<AniNetStatsInfoSequenceItem>,
            uid: u32,
            networkInfo: &mut AniNetworkInfo,
        ) -> i32;

        fn GetErrorCodeAndMessage(error_code: &mut i32) -> String;
    }
}
