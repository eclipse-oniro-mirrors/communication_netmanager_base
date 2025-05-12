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

use ani_rs::business_error::BusinessError;

use crate::wrapper::NetStatsClient;

#[ani_rs::native]
pub(crate) fn get_all_rx_bytes() -> Result<i64, BusinessError> {
    NetStatsClient::get_all_rx_bytes()
        .map(|v| v as i64)
        .map_err(|e| BusinessError::new(e, format!("failed to get all rx bytes")))
}

#[ani_rs::native]
pub(crate) fn get_all_tx_bytes() -> Result<i64, BusinessError> {
    NetStatsClient::get_all_tx_bytes()
        .map(|v| v as i64)
        .map_err(|e| BusinessError::new(e, format!("failed to get all tx bytes")))
}

#[ani_rs::native]
pub(crate) fn get_cellular_rx_bytes() -> Result<i64, BusinessError> {
    NetStatsClient::get_cellular_rx_bytes()
        .map(|v| v as i64)
        .map_err(|e| BusinessError::new(e, format!("failed to get cellular rx bytes")))
}

#[ani_rs::native]
pub(crate) fn get_cellular_tx_bytes() -> Result<i64, BusinessError> {
    NetStatsClient::get_cellular_tx_bytes()
        .map(|v| v as i64)
        .map_err(|e| BusinessError::new(e, format!("failed to get cellular tx bytes")))
}

#[ani_rs::native]
pub(crate) fn get_iface_rx_bytes(iface: String) -> Result<i64, BusinessError> {
    NetStatsClient::get_iface_rx_bytes(&iface)
        .map(|v| v as i64)
        .map_err(|e| BusinessError::new(e, format!("failed to get iface rx bytes")))
}

#[ani_rs::native]
pub(crate) fn get_iface_tx_bytes(iface: String) -> Result<i64, BusinessError> {
    NetStatsClient::get_iface_tx_bytes(&iface)
        .map(|v| v as i64)
        .map_err(|e| BusinessError::new(e, format!("failed to get iface tx bytes")))
}

#[ani_rs::native]
pub(crate) fn get_uid_rx_bytes(uid: i64) -> Result<i64, BusinessError> {
    NetStatsClient::get_uid_rx_bytes(uid as u32)
        .map(|v| v as i64)
        .map_err(|e| BusinessError::new(e, format!("failed to get uid rx bytes")))
}

#[ani_rs::native]
pub(crate) fn get_uid_tx_bytes(uid: i64) -> Result<i64, BusinessError> {
    NetStatsClient::get_uid_tx_bytes(uid as u32)
        .map(|v| v as i64)
        .map_err(|e| BusinessError::new(e, format!("failed to get uid tx bytes")))
}

#[ani_rs::native]
pub(crate) fn get_sockfd_rx_bytes(sockfd: i32) -> Result<i64, BusinessError> {
    NetStatsClient::get_sockfd_rx_bytes(sockfd)
        .map(|v| v as i64)
        .map_err(|e| BusinessError::new(e, format!("failed to get sockfd rx bytes")))
}

#[ani_rs::native]
pub(crate) fn get_sockfd_tx_bytes(sockfd: i32) -> Result<i64, BusinessError> {
    NetStatsClient::get_sockfd_tx_bytes(sockfd)
        .map(|v| v as i64)
        .map_err(|e| BusinessError::new(e, format!("failed to get sockfd tx bytes")))
}
