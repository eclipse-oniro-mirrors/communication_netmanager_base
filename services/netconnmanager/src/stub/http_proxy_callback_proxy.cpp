/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "http_proxy_callback_proxy.h"
#include "conn_ipc_interface_code.h"
#include "net_conn_constants.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t HttpProxyCallbackProxy::OnHttpProxyChange()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return NETMANAGER_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return NETMANAGER_ERR_IPC_CONNECT_STUB_FAIL;
    }
    MessageParcel reply;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    return remote->SendRequest(static_cast<uint32_t>(HttpProxyCallbackInterfaceCode::ON_HTTP_PROXY_CHANGE),
        data, reply, option);
}
} // namespace NetManagerStandard
} // namespace OHOS
