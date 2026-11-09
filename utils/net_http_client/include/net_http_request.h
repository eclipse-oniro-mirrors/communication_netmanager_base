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

#ifndef NETMANAGER_BASE_NET_HTTP_REQUEST_H
#define NETMANAGER_BASE_NET_HTTP_REQUEST_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace OHOS {
namespace NetManagerStandard {
namespace NetHttp {

enum class HttpMethod : int32_t {
    GET = 0,
    POST = 1,
    PUT = 2,
    DELETE = 3,
    HEAD = 4,
};

enum class IpVersion : int32_t {
    AUTO = 0,
    IPV4_ONLY = 1,
    IPV6_ONLY = 2,
};

enum class HttpVersion : int32_t {
    DEFAULT = 0,
    HTTP2_TLS = 1,
};

enum class TlsVersion : int32_t {
    DEFAULT = 0,
    TLS_V1_2 = 1,
    TLS_V1_3 = 2,
};

struct HttpProxyConfig {
    /* Proxy host, may contain "http(s)://" prefix, plain host or ip is also accepted. */
    std::string host;
    uint16_t port = 0;
    /* Whether to tunnel through the proxy (CURLOPT_HTTPPROXYTUNNEL). */
    bool useTunnel = true;
    /* Proxy auth credentials, resolved by the caller (e.g. from NetProxyUserinfo). */
    std::string username;
    std::string password;
};

struct TlsOptions {
    /* When false, peer certificate and host name verification are both disabled. */
    bool verifyPeer = true;
    /* CA bundle file path (CURLOPT_CAINFO). Empty: use system default. */
    std::string caInfoPath;
    /* CA directory path (CURLOPT_CAPATH). Empty: not set. */
    std::string caDirPath;
    /* Minimum TLS version. DEFAULT: use curl default. */
    TlsVersion minVersion = TlsVersion::DEFAULT;
    /* Cipher suites for TLS1.2 and below. Empty: not set. */
    std::string cipherList;
    /* Cipher suites for TLS1.3. Empty: not set. */
    std::string tls13CipherList;
};

/*
 * Pure data description of one HTTP request. No dependency on any netmanager
 * internal types: interface binding, proxy credentials and dns pre-resolution
 * results are all passed in as plain data by the caller.
 */
struct HttpRequest {
    std::string url;
    HttpMethod method = HttpMethod::GET;
    /* Request headers, composed as "key: value" when sending. */
    std::map<std::string, std::string> headers;
    /* Request body for POST/PUT. Must stay valid only until Execute/Submit returns. */
    std::string body;
    /* Whole transfer timeout in ms. Non-positive: not set (curl default). */
    int32_t timeoutMs = 10000;
    /* Connect phase timeout in ms. Non-positive: not set (curl default). */
    int32_t connectTimeoutMs = 10000;
    /* Explicit port. 0: use the port carried in url. */
    uint16_t port = 0;
    /* Force IPv4 / IPv6. AUTO: let curl choose. */
    IpVersion ipVersion = IpVersion::AUTO;
    /* Outgoing interface name (CURLOPT_INTERFACE). Empty: not bound. */
    std::string bindInterface;
    /* Close the connection after the response (CURLOPT_FORBID_REUSE). */
    bool forbidReuse = false;
    /* Follow 3xx redirects (CURLOPT_FOLLOWLOCATION). Default off. */
    bool followRedirects = false;
    HttpVersion httpVersion = HttpVersion::DEFAULT;
    TlsOptions tls;
    /* Optional proxy config. nullopt: no proxy. */
    std::optional<HttpProxyConfig> proxy;
    /*
     * DNS pre-resolution entries for CURLOPT_RESOLVE, format "host:port:ip[,ip...]".
     * The caller resolves dns itself (e.g. via fwmark getaddrinfo_ext bound to a
     * specific netId) and injects the results here.
     */
    std::vector<std::string> resolves;
};

} // namespace NetHttp
} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_BASE_NET_HTTP_REQUEST_H
