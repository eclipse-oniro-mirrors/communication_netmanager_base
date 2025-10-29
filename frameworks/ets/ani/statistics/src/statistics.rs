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

use crate::{bridge, error_code::convert_to_business_error, wrapper::NetStatsClient};
use ani_rs::business_error::BusinessError;
use std::collections::HashMap;

#[ani_rs::native]
pub fn get_all_rx_bytes() -> Result<i64, BusinessError> {
    NetStatsClient::get_all_rx_bytes()
        .map(|v| v as i64)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_all_tx_bytes() -> Result<i64, BusinessError> {
    NetStatsClient::get_all_tx_bytes()
        .map(|v| v as i64)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_cellular_rx_bytes() -> Result<i64, BusinessError> {
    NetStatsClient::get_cellular_rx_bytes()
        .map(|v| v as i64)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_cellular_tx_bytes() -> Result<i64, BusinessError> {
    NetStatsClient::get_cellular_tx_bytes()
        .map(|v| v as i64)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_iface_rx_bytes(iface: String) -> Result<i64, BusinessError> {
    NetStatsClient::get_iface_rx_bytes(&iface)
        .map(|v| v as i64)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_iface_tx_bytes(iface: String) -> Result<i64, BusinessError> {
    NetStatsClient::get_iface_tx_bytes(&iface)
        .map(|v| v as i64)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_uid_rx_bytes(uid: i64) -> Result<i64, BusinessError> {
    NetStatsClient::get_uid_rx_bytes(uid as u32)
        .map(|v| v as i64)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_uid_tx_bytes(uid: i64) -> Result<i64, BusinessError> {
    NetStatsClient::get_uid_tx_bytes(uid as u32)
        .map(|v| v as i64)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_sockfd_rx_bytes(sockfd: i32) -> Result<i64, BusinessError> {
    NetStatsClient::get_sockfd_rx_bytes(sockfd)
        .map(|v| v as i64)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_sockfd_tx_bytes(sockfd: i32) -> Result<i64, BusinessError> {
    NetStatsClient::get_sockfd_tx_bytes(sockfd)
        .map(|v| v as i64)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_traffic_stats_by_iface(
    ifaceInfo: bridge::IfaceInfo,
) -> Result<bridge::NetStatsInfo, BusinessError> {
    NetStatsClient::get_traffic_stats_by_iface(ifaceInfo.into())
        .map(|v| v as bridge::NetStatsInfo)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_traffic_stats_by_uid(
    uidInfo: bridge::UidInfo,
) -> Result<bridge::NetStatsInfo, BusinessError> {
    NetStatsClient::get_traffic_stats_by_uid(uidInfo.into())
        .map(|v| v as bridge::NetStatsInfo)
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_traffic_stats_by_network(
    networkInfo: bridge::AniNetworkInfo,
) -> Result<HashMap<i32, bridge::NetStatsInfo>, BusinessError> {
    NetStatsClient::get_traffic_stats_by_network(networkInfo.into())
        .map(|v: Vec<bridge::AniUidNetStatsInfoPair>| {
            v.into_iter()
                .map(|item| (item.uid, item.net_stats_info))
                .collect()
        })
        .map_err(convert_to_business_error)
}

#[ani_rs::native]
pub fn get_traffic_stats_by_uid_network(
    uid: i32,
    networkInfo: bridge::AniNetworkInfo,
) -> Result<Vec<bridge::AniNetStatsInfoSequenceItem>, BusinessError> {
    NetStatsClient::get_traffic_stats_by_uid_network(uid, networkInfo.into())
        .map(|v| v as Vec<bridge::AniNetStatsInfoSequenceItem>)
        .map_err(convert_to_business_error)
}
