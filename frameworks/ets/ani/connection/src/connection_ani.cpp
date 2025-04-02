/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "net_conn_client.h"
#include <ani.h>
#include <array>
#include <iostream>
#include <optional>
#include <vector>

static const int32_t NINE = 9;
static const int32_t THREE = 3;

bool AniString2StdString(ani_env *env, ani_string str, std::string &out)
{
    ani_boolean isUndefined;
    env->Reference_IsUndefined(str, &isUndefined);
    if (isUndefined) {
        return false;
    }
    ani_size strSize;
    env->String_GetUTF8Size(str, &strSize);
    std::vector<char> buffer(strSize + 1);
    char *utf8Buffer = buffer.data();

    ani_size bytesWritten = 0;
    env->String_GetUTF8(str, utf8Buffer, strSize + 1, &bytesWritten);
    utf8Buffer[bytesWritten] = '\0';
    out = utf8Buffer;
    return true;
}

ani_class GetClass(ani_env *env, const char *className)
{
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        std::cerr << "Not found '" << className << "'" << std::endl;
        return nullptr;
    }
    return cls;
}
ani_method GetMethod(ani_env *env, ani_class cls, const char *methodName, const char *methodSignature)
{
    ani_method method;
    if (ANI_OK != env->Class_FindMethod(cls, methodName, methodSignature, &method)) {
        std::cerr << "Not found method '" << methodName << methodSignature << "'" << std::endl;
        return nullptr;
    }
    return method;
}

ani_object ObjectNew(ani_env *env, const char *className, const char *methodSignature)
{
    ani_class cls = GetClass(env, className);
    ani_method ctor = GetMethod(env, cls, "<ctor>", methodSignature);
    ani_object result;
    env->Object_New(cls, ctor, &result);
    return result;
}

static ani_object getDefaultNetSync([[maybe_unused]] ani_env *env)
{
    ani_object nethandleInner = ObjectNew(env, "L@ohos/net/connection/NetHandleInner;", nullptr);
    ani_class cls = GetClass(env, "L@ohos/net/connection/NetHandleInner;");
    ani_method setter = GetMethod(env, cls, "<set>netId", nullptr);
    OHOS::NetManagerStandard::NetHandle nethandle;
    int32_t ret = OHOS::NetManagerStandard::NetConnClient::GetInstance().GetDefaultNet(nethandle);
    if (ret != 0) {
        // 错误码
    }
    if (ANI_OK != env->Object_CallMethod_Void(nethandleInner, setter, ani_double(nethandle.GetNetId()))) {
    }
    return nethandleInner;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    static const char *className = "L@ohos/net/connection/connection;";

    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return (ani_status)NINE;
    }
    ani_class cls = GetClass(env, className);

    std::array methods = {
        ani_native_function{"getDefaultNetSync", nullptr, reinterpret_cast<void *>(getDefaultNetSync)},
    };

    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        std::cerr << "Cannot bind native methods to '" << className << "'" << std::endl;
        return (ani_status)THREE;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}