/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <array>
#include <securec.h>
#include <thread>

#include "net_base_branch_fuzzer.h"

#include "curl/curl.h"
#include "net_mgr_log_wrapper.h"

#define private public
#include "net_http_probe.h"
#include "net_policy_rule.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseBranchFuzzData = nullptr;
size_t g_baseBranchFuzzSize = 0;
size_t g_baseBranchFuzzPos = 0;
constexpr int TWO = 2;
constexpr size_t STR_LEN = 10;
} // namespace

template <class T> T GetNetBranchFuzzData()
{
    T object{};
    size_t objectSize = sizeof(object);
    if (g_baseBranchFuzzData == nullptr || g_baseBranchFuzzPos >= g_baseBranchFuzzSize ||
        objectSize > g_baseBranchFuzzSize - g_baseBranchFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, g_baseBranchFuzzData + g_baseBranchFuzzPos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_baseBranchFuzzPos += objectSize;
    return object;
}

std::string GetStringFromData(int strlen)
{
    if (strlen <= 0 || static_cast<size_t>(strlen) > STR_LEN) {
        return {};
    }
    std::array<char, STR_LEN> cstr {};
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = GetNetBranchFuzzData<char>();
    }
    cstr[strlen - 1] = '\0';
    return std::string(cstr.data());
}

SecureData GetSecureDataFromData(int8_t strlen)
{
    SecureData secureData;
    if (strlen <= 0 || static_cast<size_t>(strlen) > STR_LEN) {
        return secureData;
    }
    std::array<char, STR_LEN> cstr {};
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = GetNetBranchFuzzData<char>();
    }
    cstr[strlen - 1] = '\0';
    secureData.append(cstr.data(), strlen - 1);
    return secureData;
}

void NetHttpProbeBranchFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    int32_t testId = GetNetBranchFuzzData<int32_t>();
    std::shared_ptr<NetHttpProbe> instance_ =
        std::make_shared<NetHttpProbe>(testId, NetBearType::BEARER_DEFAULT, NetLinkInfo(), ProbeType::PROBE_HTTP);
    instance_->GetHttpProbeResult();
    instance_->GetHttpsProbeResult();
    std::string host = GetStringFromData(STR_LEN);
    HttpProxy httpProxy = {host, 0, {}};
    NetLinkInfo info;
    std::string ifaceName = GetStringFromData(STR_LEN);
    info.ifaceName_ = ifaceName;
    instance_->netLinkInfo_ = info;
    instance_->UpdateGlobalHttpProxy(httpProxy);
    std::string httpUrl = GetStringFromData(STR_LEN);
    std::string httpsUrl = GetStringFromData(STR_LEN);
    instance_->SendProbe(PROBE_HTTP_HTTPS, httpUrl, httpsUrl);
    instance_->CheckCurlGlobalInitState();
    ProbeType probeType = ProbeType::PROBE_HTTP_HTTPS;
    instance_->InitHttpCurl(probeType);
    instance_->CleanHttpCurl();
    instance_->ExtractDomainFormUrl(httpUrl);
    std::string testString = "";
    instance_->GetAddrInfo(testString);
    instance_->SetResolveOption(probeType, testString, testString, testId);
    instance_->SetResolveOption(probeType, "test", testString, testId);
    testString = GetStringFromData(STR_LEN);
    instance_->GetAddrInfo(testString);
    instance_->SetCurlOptions(probeType, httpUrl, httpsUrl);
    CURL *curl = nullptr;
    instance_->SetHttpOptions(probeType, curl, testString);
    bool useHttpProxy = GetNetBranchFuzzData<bool>();
    instance_->SetProxyOption(probeType, useHttpProxy);
    instance_->SetResolveOption(probeType, testString, testString, testId);
    instance_->SendDnsProbe(probeType, testString, testString, useHttpProxy);
    instance_->SendHttpProbeRequest();
    instance_->RecvHttpProbeResponse();
    instance_->LoadProxy(testString, testId);
}

void NetPolicyRuleBranchFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    bool isForeground = (static_cast<int>(data[0]) % TWO) ? true : false;
    std::shared_ptr<NetPolicyRule> netPolicyRule = std::make_shared<NetPolicyRule>();
    uint32_t testId = GetNetBranchFuzzData<uint32_t>();
    netPolicyRule->UpdateForegroundUidList(testId, isForeground);
    std::string message = GetStringFromData(STR_LEN);
    netPolicyRule->GetDumpMessage(message);

    auto policyEvent = std::make_shared<PolicyEvent>();
    netPolicyRule->HandleEvent(testId, policyEvent);
    netPolicyRule->IsValidNetPolicy(testId);
    uint32_t netsysCtrl = GetNetBranchFuzzData<uint32_t>();
    netPolicyRule->NetsysCtrl(testId, netsysCtrl);
    netPolicyRule->BuildTransCondition(testId, netsysCtrl);
}
} // namespace NetManagerStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::NetManagerStandard::NetHttpProbeBranchFuzzTest(data, size);
    OHOS::NetManagerStandard::NetPolicyRuleBranchFuzzTest(data, size);
    return 0;
}
