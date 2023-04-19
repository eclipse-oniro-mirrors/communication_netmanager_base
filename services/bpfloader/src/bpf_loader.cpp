/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "bpf_loader.h"

#include <algorithm>
#include <dirent.h>

#include "elfio/elfio_relocation.hpp"

namespace OHOS {
namespace Bpf {
namespace {
constexpr const char *NETSYS_NAME = "netsys.o";
}
BpfLoader BpfLoader::instance_;
BpfLoader &BpfLoader::GetInstance()
{
    return instance_;
}

bool BpfLoader::HandleElfFiles(const std::string &elfDir)
{
    return 0;
}

bool BpfLoader::LoadElf(const std::string &path)
{
    return true;
}

bool BpfLoader::SetLicenseAndVersion()
{
    return true;
}

bool BpfLoader::LoadProgs(int32_t &nrMaps)
{
    return true;
}

bool BpfLoader::CreateMaps(int32_t &nrMaps)
{
    return true;
}

bool BpfLoader::ParseReloAndApply(bpf_insn *insn, ELFIO::section *sec, BpfMapData *maps, int32_t &nrMaps) const
{
    return true;
}
} // namespace Bpf
} // namespace OHOS
