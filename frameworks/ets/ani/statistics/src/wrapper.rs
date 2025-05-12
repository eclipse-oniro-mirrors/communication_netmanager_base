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

use cxx::let_cxx_string;

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
}

#[cxx::bridge(namespace = "OHOS::NetManagerAni")]
mod ffi {
    unsafe extern "C++" {
        include!("net_stats_client.h");
        include!("statistics_ani.h");

        #[namespace = "OHOS::NetManagerStandard"]
        type NetStatsClient;

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

    }
}
