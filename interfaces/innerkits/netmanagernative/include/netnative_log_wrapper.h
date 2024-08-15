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

#ifndef NETNATIVE_LOG_WRAPPER_H
#define NETNATIVE_LOG_WRAPPER_H

#include <string>
#include <memory>
#include "hilog/log.h"

#undef LOG_TAG
#ifndef NETMGRNATIVE_LOG_TAG
#define LOG_TAG "NetsysNativeService"
#else
#define LOG_TAG NETMGRNATIVE_LOG_TAG
#endif

#define FILENAME (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define PRINT_NATIVE_LOG(op, fmt, ...)                                                                               \
    (void)HILOG_##op(LOG_CORE, "[%{public}s:%{public}d]" fmt,  \
                                    FILENAME, __LINE__, ##__VA_ARGS__)

#define NETNATIVE_LOG_D(fmt, ...) PRINT_NATIVE_LOG(DEBUG, fmt, ##__VA_ARGS__)
#define NETNATIVE_LOGE(fmt, ...) PRINT_NATIVE_LOG(ERROR, fmt, ##__VA_ARGS__)
#define NETNATIVE_LOGW(fmt, ...) PRINT_NATIVE_LOG(WARN, fmt, ##__VA_ARGS__)
#define NETNATIVE_LOGI(fmt, ...) PRINT_NATIVE_LOG(INFO, fmt, ##__VA_ARGS__)
#define NETNATIVE_LOGF(fmt, ...) PRINT_NATIVE_LOG(FATAL, fmt, ##__VA_ARGS__)

#endif // NETNATIVE_LOG_WRAPPER_H