# Copyright (C) 2022 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_library(
    netmanager_base_utils
    SHARED
    common_utils/src/netmanager_base_common_utils.cpp
    napi_utils/src/netmanager_base_napi_utils.cpp
    event_manager/src/netmanager_base_event_listener.cpp
    event_manager/src/netmanager_base_event_manager.cpp
    base_context/src/netmanager_base_base_context.cpp
    module_template/src/netmanager_base_module_template.cpp
)