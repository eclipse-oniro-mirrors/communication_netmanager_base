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

#ifndef BPF_LOADER_H
#define BPF_LOADER_H

#ifdef ENABLE_ELFIO
#include "bpf_map_creator.h"
#include "bpf_prog_loader.h"
#endif
#include "bpf_utils.h"
#include "elfio/elfio.hpp"

namespace OHOS {
namespace Bpf {
constexpr int32_t MAP_COUNT = 32;

class BpfLoader {
public:
    /**
     * Get the instance object.
     *
     * @return Returns the instance object.
     */
    static BpfLoader &GetInstance();

    /**
     * Handle elf files.
     *
     * @param elfDir The dir of elf files.
     * @return Returns true on success, false on failure.
     */
    bool HandleElfFiles(const std::string &elfDir);

    /**
     * Get the kernel version.
     *
     * @return Returns the kernel version.
     */
    int32_t GetKernVersion() const
    {
        return kernVersion_;
    }

    /**
     * Get the license.
     *
     * @return Returns the license.
     */
    const std::string &GetLicense() const
    {
        return license_;
    }

private:
    BpfLoader() = default;

    /**
     * Load the elf file.
     *
     * @param path The path of elf file.
     * @return Returns true on success, false on failure.
     */
    bool LoadElf(const std::string &path);
    bool SetLicenseAndVersion();
    bool LoadProgs(int32_t &nrMaps);
    bool CreateMaps(int32_t &nrMaps);
    bool ParseReloAndApply(bpf_insn *insn, ELFIO::section *sec, BpfMapData *maps, int32_t &nrMaps) const;

    static BpfLoader instance_;
#ifdef ENABLE_ELFIO
    BpfMapCreator mapCreator_;
    BpfProgLoader progLoader_;
#endif
    ELFIO::elfio elfIo_;

    std::string license_;
    int32_t kernVersion_ = 0;
    BpfMapData mapData_[MAP_COUNT];
};
} // namespace Bpf
} // namespace OHOS
#endif // BPF_LOADER_H
