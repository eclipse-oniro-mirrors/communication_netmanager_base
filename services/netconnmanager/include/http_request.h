/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <stdint.h>

namespace OHOS {
namespace NetManagerStandard {
class HttpRequest {
public:
    HttpRequest();
    ~HttpRequest();
    /**
     * @brief :set the interface name for HTTP request.
     *
     * @param ifaceName - interface name.[in]
     */
    void SetIfaceName(const std::string &ifaceName);
    /**
     * @brief : HTTP get header request.
     *
     * @param strUrl - HTTP request URL.[in]
     * @param strHeader - HTTP request header.[out]
     * @return int32_t 0:OK,otherwise:failure
     */
    int32_t HttpGetHeader(const std::string &strUrl, std::string &strHeader);
    /**
     * @brief : HttpGet request.
     *
     * @param strUrl - HTTP request URL.[in]
     * @param strResponse - HTTP request response.[out]
     * @return int32_t 0:OK,otherwise:failure
     */
    int32_t HttpGet(const std::string &strUrl, std::string &strResponse);
    /**
     * @brief : HttpPost request.
     *
     * @param strUrl - HTTP request URL.[in]
     * @param strData - data send in a POST request.[in]
     * @param strResponse - HTTP request response.[out]
     * @return int32_t 0:OK,otherwise:failure
     */
    int32_t HttpPost(const std::string &strUrl, const std::string &strData, std::string &strResponse);
    /**
     * @brief : Set the transfer timeout in milliseconds.
     *
     * @param timeout - timeout in milliseconds.[in]
     */
    void SetTransportTimeout(int64_t timeout);
    /**
     * @brief : Get total time of last transfer in milliseconds.
     */
    int64_t GetLastTotalTime() const;
private:
    enum class HttpReqType {
        HTTP_REQUEST_TYPE_GET,
        HTTP_REQUEST_TYPE_POST,
    };
    /**
     * @brief : Executes HTTP requests, GET or POST.
     *
     * @param type - Http request type.[in]
     * @param strUrl - HTTP request URL.[in]
     * @param strData - Data send in a POST request.[in]
     * @param strResponse - HTTP request response.[out]
     * @return int32_t 0:OK,otherwise:failure
     */
    int32_t HttpRequestExec(
        HttpReqType type, const std::string &strUrl, const std::string &strData, std::string &strResponse);
    /**
     * @brief : Only get HTTP header.
     *
     * @param strUrl - HTTP request URL.[in]
     * @param strHeader - HTTP request header.[out]
     * @return int32_t 0:OK,otherwise:failure
     */
    int32_t HttpRequestHeaderExec(const std::string &strUrl, std::string &strHeader);

    typedef void CURL;
    class CURLClean {
    public:
        void operator()(CURL *p) const;
    };
    /**
     * @brief : Parse http exec result.
     *
     * @param curl - curl smart pointer.[in]
     * @param rlt - result of HTTP request.[in]
     * @return int32_t 0:OK,otherwise:failure
     */
    int32_t ParseExecResult(const std::unique_ptr<CURL, CURLClean> &curl, int32_t rlt);
    /**
     * @brief : Set the HTTP request option.
     *
     * @param curl - curl smart pointer.[in]
     * @param type - HTTP request type.[in]
     * @param strUrl - HTTP request URL.[in]
     * @param strData - Data send in a POST request.[in]
     * @param strResponse - HTTP request response.[out]
     * @return int32_t 0:OK,otherwise:failure
     */
    int32_t SetCurlOpt(const std::unique_ptr<CURL, CURLClean> &curl, HttpReqType type, const std::string &strUrl,
        const std::string &strData, std::string &strResponse);
    /**
     * @brief : Set the HTTP header option.
     *
     * @param curl - curl smart pointer.[in]
     * @param strUrl - HTTP request URL.[in]
     * @param strHeader - HTTP request header response.[out]
     * @return int32_t 0:OK,otherwise:failure
     */
    int32_t SetCurlOptHeader(
        const std::unique_ptr<CURL, CURLClean> &curl, const std::string &strUrl, std::string &strHeader);
    /**
     * @brief : Set the HTTP request common option.
     *
     * @param curl - curl smart pointer.[in]
     * @param strUrl - HTTP request URL.[in]
     * @return int32_t 0:OK,otherwise:failure
     */
    int32_t SetCurlOptCommon(const std::unique_ptr<CURL, CURLClean> &curl, const std::string &strUrl);
    /**
     * @brief : data callback function
     *
     * @param data - data pointer.[in]
     * @param size - the size of each data element.[in]
     * @param nmemb - the size of data element.[in]
     * @param strBuffer - the buffer that writes the data.[out]
     * @return int32_t - the size of data in bytes.
     */
    static int32_t DataCallback(char *data, size_t size, size_t nmemb, std::string *strBuffer);
private:
    static constexpr int32_t URL_SIZE = 1024;
    static constexpr int64_t CONNECTION_TIMEOUT = 1500;
    static constexpr int64_t TRANS_OP_TIMEOUT = 2000;
    static constexpr int32_t MAX_CONNECT_NUM = 8;
    static constexpr int32_t DEFAULT_ERROR_SIZE = 256;
    int64_t connectionTimeout_ = CONNECTION_TIMEOUT;
    int64_t transOpTimeout_ = TRANS_OP_TIMEOUT;
    char errorBuffer_[DEFAULT_ERROR_SIZE] = {0};
    std::string ifaceName_;
    int64_t lastTransTime_ = 0;
};
}  // namespace NetManagerStandard
}  // namespace OHOS
#endif // HTTP_REQUEST_H