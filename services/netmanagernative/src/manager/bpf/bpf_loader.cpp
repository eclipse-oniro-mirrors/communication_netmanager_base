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

#include "bpf_loader.h"

#include <algorithm>
#include <dirent.h>
#include "elfio/elfio_relocation.hpp"

namespace OHOS {
namespace Bpf {
BpfLoader BpfLoader::instance_;

constexpr const char *ELF_SUFFIX = ".o";
constexpr const uint64_t PROG_ADDR_ALIGN = 0x08;
constexpr const uint64_t REL_PROG_SIZE = 0x30;

BpfLoader &BpfLoader::GetInstance()
{
    return instance_;
}

bool BpfLoader::HandleElfFiles(const std::string &elfDir)
{
    bool retVal = true;
    dirent *drt = nullptr;
    DIR *dir = nullptr;

    if ((dir = opendir(elfDir.c_str())) != nullptr) {
        while ((drt = readdir(dir)) != nullptr) {
            std::string fn = drt->d_name;
            if (!EndsWith(fn, ELF_SUFFIX)) {
                continue;
            }

            std::string path(elfDir);
            path += fn;

            bool ret = LoadElf(path.c_str());
            if (!ret) {
                NETNATIVE_LOGE("Failed to load prog: %{public}s", path.c_str());
                retVal = ret;
            } else {
                NETNATIVE_LOGI("Loaded prog: %{public}s", path.c_str());
            }
        }
        closedir(dir);
    }
    return retVal;
}

bool BpfLoader::LoadElf(const std::string &path)
{
    int32_t nrMaps = 0;

    if (!elfIo_.load(path.c_str())) {
        NETNATIVE_LOGE("Failed to load elf file %{public}s, errno = %{public}d", path.c_str(), errno);
        return false;
    }

    if (elfIo_.get_version() != EV_CURRENT) {
        NETNATIVE_LOGE("Failed to identify the elf file version, got version = %{public}d", elfIo_.get_version());
        return false;
    }

    if (!SetLicenseAndVersion()) {
        NETNATIVE_LOGE("Failed to set license and kernel version");
        return false;
    }

    if (!CreateMaps(nrMaps)) {
        NETNATIVE_LOGE("Failed to create maps");
        return false;
    }

    if (!LoadProgs(nrMaps)) {
        NETNATIVE_LOGE("Failed to load progs");
        return false;
    }

    return true;
}

bool BpfLoader::SetLicenseAndVersion()
{
    for (const auto &section : elfIo_.sections) {
        if (section->get_name() == "license") {
            license_ = section->get_data();
            NETNATIVE_LOGI("license = %{public}s", license_.c_str());
            if (license_.empty()) {
                NETNATIVE_LOGE("Failed to get license: errno = %{public}d", errno);
                return false;
            }
        } else if (section->get_name() == "version") {
            kernVersion_ = atoi(section->get_data());
            NETNATIVE_LOGI("kernVersion = %{public}d", kernVersion_);
            if (!kernVersion_) {
                NETNATIVE_LOGE("Failed to get kernel version: errno = %{public}d", errno);
                return false;
            }
        }
    }

    return true;
}

bool BpfLoader::LoadProgs(int32_t &nrMaps)
{
    bpf_insn *insns = nullptr;

    for (const auto &section : elfIo_.sections) {
        if (section->get_type() == SHT_PROGBITS && section->get_addr_align() == PROG_ADDR_ALIGN) {
            insns = (bpf_insn *)(section->get_data());
        }

        if (section->get_type() == SHT_REL && section->get_size() == REL_PROG_SIZE) {
            if (!ParseReloAndApply(insns, section, mapData_, nrMaps)) {
                continue;
            }
        }
    }

#ifdef ENABLE_ELFIO
    constexpr const char *SOCKET_NAME = "socket";
    constexpr const char *CGROUP_SKB_NAME = "cgroup_skb";
    for (const auto &section : elfIo_.sections) {
        if (!section->get_name().compare(0, strlen(SOCKET_NAME), SOCKET_NAME) ||
            !section->get_name().compare(0, strlen(CGROUP_SKB_NAME), CGROUP_SKB_NAME)) {
            if (!progLoader_.LoadAndAttach(section->get_name(), reinterpret_cast<const bpf_insn *>(section->get_data()),
                                           section->get_size())) {
                return false;
            }
        }
    }
#endif

    return true;
}

bool BpfLoader::CreateMaps(int32_t &nrMaps)
{
#ifdef ENABLE_ELFIO
    nrMaps = mapCreator_.LoadElfMapsSection(mapData_, elfIo_);
    NETNATIVE_LOGI("nrMaps = %{public}d", nrMaps);

    if (nrMaps < 0) {
        NETNATIVE_LOGE("Error: Failed loading ELF maps (errno: %{public}d): %{public}s", nrMaps, strerror(-nrMaps));
        return false;
    }
    if (!mapCreator_.CreateMaps(mapData_, nrMaps)) {
        NETNATIVE_LOGE("Failed to create maps: errno = %{public}d", errno);
        return false;
    }
#endif
    return true;
}

bool BpfLoader::ParseReloAndApply(bpf_insn *insn, ELFIO::section *sec, BpfMapData *maps, int32_t &nrMaps) const
{
    if (!insn) {
        NETNATIVE_LOGE("insn is null!");
        return false;
    }

    ELFIO::Elf64_Addr offset;
    ELFIO::Elf64_Addr symbolValue;
    std::string symbolName;
    ELFIO::Elf_Word type;
    ELFIO::Elf_Sxword addend;
    ELFIO::Elf_Sxword calcValue;
    ELFIO::relocation_section_accessor reloc(elfIo_, sec);

    for (uint32_t i = 0; i < sec->get_size() / sec->get_entry_size(); i++) {
        reloc.get_entry(i, offset, symbolValue, symbolName, type, addend, calcValue);
        uint32_t insnIdx = offset / sizeof(bpf_insn);
        int32_t mapIdx;
        bool match = false;

        if (insn[insnIdx].code != (BPF_LD | BPF_IMM | BPF_DW)) {
            NETNATIVE_LOGE("Invalid relo for insn[%{public}u].code 0x%{public}x", insnIdx, insn[insnIdx].code);
            return false;
        }
        insn[insnIdx].src_reg = BPF_PSEUDO_MAP_FD;

        for (mapIdx = 0; mapIdx < nrMaps; mapIdx++) {
            if (maps[mapIdx].name == symbolName) {
                match = true;
                break;
            }
        }
        if (!match) {
            NETNATIVE_LOGE("Invalid relo for insn[%{public}u] no map_data match", insnIdx);
            return false;
        }
        insn[insnIdx].imm = maps[mapIdx].fd;
    }
    return true;
}
} // namespace Bpf
} // namespace OHOS
