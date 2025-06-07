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
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

static const int32_t ANI_ERROR_CODE = 3;

constexpr const char *NET_HANDLE_INNER = "L@ohos/net/connection/connection/NetHandleInner;";
constexpr const char *BUSINESS_ERROR = "L@ohos/base/BusinessError;";
constexpr const char *CTOR = "<ctor>";

static const std::unordered_map<int32_t, std::string> errorMap = {
    {201, "Permission denied."},
    {2100002, "Failed to connect to the service."},
    {2100003, "System internal error."},
};

ani_string ANIUtils_StdStringToANIString(ani_env *env, std::string str)
{
    ani_string result_string{};
    env->String_NewUTF8(str.c_str(), str.size(), &result_string);
    return result_string;
}

ani_class GetClass(ani_env *env, const char *className)
{
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        std::cerr << "Not found '" << className << "'" << std::endl;
    }
    return cls;
}
ani_method GetMethod(ani_env *env, ani_class cls, const char *methodName, const char *methodSignature)
{
    ani_method method;
    if (ANI_OK != env->Class_FindMethod(cls, methodName, methodSignature, &method)) {
        std::cerr << "Not found method '" << methodName << methodSignature << "'" << std::endl;
    }
    return method;
}

ani_object ObjectNew(ani_env *env, const char *className, const char *methodSignature)
{
    ani_class cls = GetClass(env, className);
    ani_method ctor = GetMethod(env, cls, CTOR, methodSignature);
    ani_object result;
    env->Object_New(cls, ctor, &result);
    return result;
}

void throwBusinessError(ani_env *env, int32_t code, std::string &msg)
{
    ani_class businessError = GetClass(env, BUSINESS_ERROR);
    ani_method businessErrorCtor = GetMethod(env, businessError, CTOR, "DLescompat/Error;:V");
    ani_class errorcls = GetClass(env, "Lescompat/Error;");
    ani_method ctor = GetMethod(env, errorcls, CTOR, "Lstd/core/String;:V");
    ani_string errorMsg = ANIUtils_StdStringToANIString(env, msg);

    ani_object errorObj;
    env->Object_New(errorcls, ctor, &errorObj, errorMsg);

    ani_object businessErrorObj;
    env->Object_New(businessError, businessErrorCtor, &businessErrorObj, errorObj);
    env->ThrowError(static_cast<ani_error>(businessErrorObj));
}

static ani_object getDefaultNetSync([[maybe_unused]] ani_env *env)
{
    OHOS::NetManagerStandard::NetHandle nethandle;
    int32_t ret = OHOS::NetManagerStandard::NetConnClient::GetInstance().GetDefaultNet(nethandle);
    if (ret == 0) {
        ani_object nethandleInner = ObjectNew(env, NET_HANDLE_INNER, nullptr);
        ani_class cls = GetClass(env, NET_HANDLE_INNER);
        ani_method setter = GetMethod(env, cls, "<set>netId", nullptr);
        env->Object_CallMethod_Void(nethandleInner, setter, ani_double(nethandle.GetNetId()));
        return nethandleInner;
    } else {
        ani_ref undefined;
        env->GetUndefined(&undefined);

        std::string msg;
        auto it = errorMap.find(ret);
        if (it != errorMap.end()) {
            msg = it->second;
        } else {
            msg = "Unknown error.";
        }
        throwBusinessError(env, ret, msg);
        return static_cast<ani_object>(undefined);
    }
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return (ani_status)ANI_ERROR_CODE;
    }

    ani_namespace connection;
    env->FindNamespace("L@ohos/net/connection/connection;", &connection);

    std::array methods = {
        ani_native_function{"getDefaultNetSync", nullptr, reinterpret_cast<void *>(getDefaultNetSync)},
    };

    if (ANI_OK != env->Namespace_BindNativeFunctions(connection, methods.data(), methods.size())) {
        return (ani_status)ANI_ERROR_CODE;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}