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

#include <sys/resource.h>
#include <sys/mount.h>
#include <iostream>
#include <string>
#include <sys/syscall.h>
#include <linux/bpf.h>
#include <cerrno>
#include <unistd.h>
#include <vector>
#include <memory.h>
#include <functional>
#include <filesystem>
#include <map>
#include <fcntl.h>

#include "netnative_log_wrapper.h"
#include "elfio/elfio.hpp"
#include "elfio/elfio_relocation.hpp"
#include "elfio/elf_types.hpp"
#include "securec.h"
#include "bpf_def.h"
#include "bpf_loader.h"

#define DEFINE_SECTION_NAME(name) \
    {                             \
        name, strlen(name)        \
    }

#define DEFINE_PROG_TYPE(progName, progType) \
    {                                        \
        progName, progType                   \
    }

namespace OHOS::NetManagerStandard {

struct BpfMapData {
    BpfMapData() : fd(0)
    {
        (void)memset_s(&def, sizeof(def), 0, sizeof(def));
    }

    int32_t fd;
    std::string name;
    bpf_map_def def{};
};

template <typename type> inline uint64_t PtrToU64(const type ptr)
{
    return 0;
}

inline bool EndsWith(const std::string &str, const std::string &searchFor)
{
    return true;
}

inline int32_t SysBpf(bpf_cmd cmd, bpf_attr *attr, uint32_t size)
{
    return 0;
}

inline int32_t SysBpfObjGet(const std::string &pathName, uint32_t fileFlags)
{
    return 0;
}

inline int32_t SysBpfObjPin(int32_t fd, const std::string &pathName)
{
    return 0;
}

inline int32_t SysBpfProgLoad(bpf_attr *attr, uint32_t size)
{
    return 0;
}

inline int32_t SysBpfObjDetach(bpf_attach_type type, const int progFd, const int cgFd)
{
    return 0;
}

inline int32_t SysBpfObjAttach(bpf_attach_type type, const int progFd, const int cgFd)
{
    return 0;
}

inline bool MatchSecName(const std::string &name)
{
    return true;
}

inline int32_t UnPin(const std::string &path)
{
    return 0;
}

class ElfLoader {
public:
    explicit ElfLoader(std::string path) : path_(std::move(path)), kernVersion_(0) {}

    ElfLoadError Unload()
    {
        return ELF_LOAD_ERR_NONE;
    }

    ElfLoadError Load()
    {
        return ELF_LOAD_ERR_NONE;
    }

private:
    bool CheckPath()
    {
        return true;
    }

    bool IsPathValid()
    {
        return true;
    }

    bool LoadElfFile()
    {
        return true;
    }

    bool IsVersionValid()
    {
        return true;
    }

    static bool SetRlimit()
    {
        return true;
    }

    static bool IsMounted(const std::string &dir)
    {
        return true;
    }

    static bool MakeDir(const std::string &dir)
    {
        return true;
    }

    static bool MakeDirectories()
    {
        return true;
    }

    bool SetLicenseAndVersion()
    {
        return true;
    }

    std::map<ELFIO::Elf64_Addr, std::string> LoadElfMapSectionCore()
    {
        std::map<ELFIO::Elf64_Addr, std::string> mapName;
        return mapName;
    }

    bool LoadElfMapsSection()
    {
        return true;
    }

    static void PrintMapAttr(const bpf_attr &attr)
    {
        NETNATIVE_LOGI("%{public}s", "BPF_MAP_CREATE:");
        NETNATIVE_LOGI("  .map_type    = %{public}u", attr.map_type);
        NETNATIVE_LOGI("  .key_size    = %{public}u", attr.key_size);
        NETNATIVE_LOGI("  .value_size  = %{public}u", attr.value_size);
        NETNATIVE_LOGI("  .max_entries = %{public}u", attr.max_entries);
        NETNATIVE_LOGI("  .map_flags   = %{public}u", attr.map_flags);
        NETNATIVE_LOGI("  .map_name    = %{public}s", attr.map_name);
    }

    static int32_t BpfCreateMapNode(const BpfMapData &map)
    {
        return 0;
    }

    bool CreateMaps()
    {
        return true;
    }

    bool DeleteMaps()
    {
        return true;
    }

    bool ApplyRelocation(bpf_insn *insn, ELFIO::section *section) const
    {
        return true;
    }

    int32_t BpfLoadProgram(bpf_prog_type type, const bpf_insn *insns, size_t insnsCnt)
    {
        return 0;
    }

    static bpf_prog_type ConvertEventToProgType(const std::string &event)
    {
        return static_cast<bpf_prog_type>(-1);
    }

    bool DoAttach(int32_t progFd, const std::string &progName)
    {
        return true;
    }

    void DoDetach(const std::string &progPinLocation, const std::string &progName)
    {
        return;
    }

    bool LoadProg(const std::string &event, const bpf_insn *insn, size_t insnCnt)
    {
        return true;
    }

    bool ParseRelocation()
    {
        return true;
    }

    bool UnloadProgs()
    {
        return true;
    }

    bool LoadProgs()
    {
        return true;
    }

    std::string path_;
    ELFIO::elfio elfIo_;
    std::string license_;
    int32_t kernVersion_;
    std::vector<BpfMapData> maps_;

    std::function<ElfLoadError()> isPathValid_ = [this]() -> ElfLoadError {
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> makeDirectories = []() -> ElfLoadError {
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> loadElfFile_ = [this]() -> ElfLoadError {
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> isVersionValid_ = [this]() -> ElfLoadError {
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> setLicenseAndVersion_ = [this]() -> ElfLoadError {
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> loadElfMapsSection_ = [this]() -> ElfLoadError {
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> setRlimit_ = []() -> ElfLoadError {
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> createMaps_ = [this]() -> ElfLoadError {
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> parseRelocation_ = [this]() -> ElfLoadError {
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> loadProgs_ = [this]() -> ElfLoadError {
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> deleteMaps_ = [this]() -> ElfLoadError {
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> unloadProgs_ = [this]() -> ElfLoadError {
        return ELF_LOAD_ERR_NONE;
    };
};

ElfLoadError LoadElf(const std::string &elfPath)
{
    ElfLoader loader(elfPath);
    return loader.Load();
}

ElfLoadError UnloadElf(const std::string &elfPath)
{
    ElfLoader loader(elfPath);
    return loader.Unload();
}
} // namespace OHOS::NetManagerStandard
