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

#ifndef MOCK_REMOTE_OBJECT_H
#define MOCK_REMOTE_OBJECT_H
 
namespace OHOS {
namespace NetManagerStandard {
 
class MockRemoteObject : public IRemoteObject {
public:
    MockRemoteObject() : IRemoteObject(u"MockRemoteObject") {}
    ~MockRemoteObject() {}
    MOCK_METHOD0(GetObjectRefCount, int32_t(void));
    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel&, MessageParcel&, MessageOption&));
    MOCK_CONST_METHOD0(IsProxyObject, bool(void));
    MOCK_CONST_METHOD0(IsObjectDead, bool(void));
    MOCK_METHOD0(GetInterfaceDescriptor, std::u16string(void));
    MOCK_CONST_METHOD0(CheckObjectLegality, bool(void));
    MOCK_METHOD1(AddDeathRecipient, bool(const sptr<DeathRecipient>&));
    MOCK_METHOD1(RemoveDeathRecipient, bool(const sptr<DeathRecipient>&));
    MOCK_CONST_METHOD1(Marshalling, bool(Parcel&));
    MOCK_METHOD0(AsInterface, sptr<IRemoteBroker>(void));
    MOCK_METHOD2(Dump, int(int, const std::vector<std::u16string>&));
};
 
} // namespace NetManagerStandard
} // namespace OHOS
#endif // MOCK_REMOTE_OBJECT_H