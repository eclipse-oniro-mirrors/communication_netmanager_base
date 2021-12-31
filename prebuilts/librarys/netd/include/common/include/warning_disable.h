/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef INCLUDE_WARNING_DISABLE_H__
#define INCLUDE_WARNING_DISABLE_H__

#if defined(__GNUC__) || defined(__clang__)
#define DO_PRAGMA(X) _Pragma(#X)
#define DISABLE_WARNING_PUSH DO_PRAGMA(GCC diagnostic push)
#define DISABLE_WARNING_POP DO_PRAGMA(GCC diagnostic pop)
#define DISABLE_WARNING(warningName) DO_PRAGMA(GCC diagnostic ignored warningName)

#define DISABLE_WARNING_OLD_STYLE_CAST DISABLE_WARNING("-Wold-style-cast")
#define DISABLE_WARNING_MISSING_FIELD_INITIALIZERS DISABLE_WARNING("-Wmissing-field-initializers")
#define DISABLE_WARNING_SIGN_CONVERSION DISABLE_WARNING("-Wsign-conversion")
#define DISABLE_WARNING_IMPLICIT_INT_CONVERSION DISABLE_WARNING("-Wimplicit-int-conversion")
#define DISABLE_WARNING_SIGN_COMPARE DISABLE_WARNING("-Wsign-compare")
#define DISABLE_WARNING_SHORTEN_64_TO_32 DISABLE_WARNING("-Wshorten-64-to-32")
#define DISABLE_WARNING_CAST_ALIGN DISABLE_WARNING("-Wcast-align")
#define DISABLE_WARNING_UNUSED_PARAMETER DISABLE_WARNING("-Wunused-parameter")
#define DISABLE_WARNING_UNUSED_VARIABLE DISABLE_WARNING("-Wunused-variable")
#define DISABLE_WARNING_C99_EXTENSIONS DISABLE_WARNING("-Wc99-extensions")
// other warnings you want to deactivate...

#else
#define DISABLE_WARNING_PUSH
#define DISABLE_WARNING_POP
#define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
#define DISABLE_WARNING_UNREFERENCED_FUNCTION
// other warnings you want to deactivate...

#endif

#endif // !INCLUDE_WARNING_DISABLE_H__