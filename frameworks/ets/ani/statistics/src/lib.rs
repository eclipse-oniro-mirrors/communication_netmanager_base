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

mod statistics;
mod wrapper;

use ani_rs::ani_constructor;

ani_constructor! {
    namespace "L@ohos/net/statistics/statistics"
    [
        "getAllRxBytesSync" : statistics::get_all_rx_bytes,
        "getAllTxBytesSync" : statistics::get_all_tx_bytes,
        "getCellularRxBytesSync" : statistics::get_cellular_rx_bytes,
        "getCellularTxBytesSync" : statistics::get_cellular_tx_bytes,
        "getIfaceRxBytesSync" : statistics::get_iface_rx_bytes,
        "getIfaceTxBytesSync" : statistics::get_iface_tx_bytes,
        "getUidRxBytesSync" : statistics::get_uid_rx_bytes,
        "getUidTxBytesSync" : statistics::get_uid_tx_bytes,
        "getSockfdRxBytesSync" : statistics::get_sockfd_rx_bytes,
        "getSockfdTxBytesSync" : statistics::get_sockfd_tx_bytes,

    ]
}
