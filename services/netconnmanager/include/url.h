/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef NET_CONN_URL_H
#define NET_CONN_URL_H

#include <string>

namespace OHOS {
namespace NetManagerStandard {
class Url {
public:
    Url(const std::string &url);

    Url(const Url &other);

    ~Url();

    std::string ToString() const;

    std::string GetHost() const;

    std::string GetPath() const;

    int32_t GetPort();

    Url &operator=(const std::string &url);
    bool operator==(const Url &other) const;

private:
    std::string url_;
    std::string host_;
    std::string path_;
    int32_t port_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_URL_H
