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

#include <algorithm>
#include <cerrno>
#include <fcntl.h>
#include <filesystem>
#include <functional>
#include <iostream>
#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <map>
#include <memory.h>
#include <string>
#include <sys/mount.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <vector>

#include "bpf_def.h"
#include "bpf_loader.h"
#include "elf_types.hpp"
#include "elfio.hpp"
#include "elfio_relocation.hpp"
#include "net_manager_constants.h"
#include "netnative_log_wrapper.h"
#include "securec.h"

#define DEFINE_SECTION_NAME(name) \
    {                             \
        name, strlen(name)        \
    }

#define DEFINE_PROG_TYPE(progName, progType) \
    {                                        \
        progName, progType                   \
    }
#define DEFINE_ATTACH_TYPE(progName, attachType, needExpectedAttach) \
    {                                                               \
        progName, attachType, needExpectedAttach                    \
    }

namespace OHOS::NetManagerStandard {
static constexpr const char *BPF_DIR = "/sys/fs/bpf";
static constexpr const char *CGROUP_DIR = "/sys/fs/cgroup";
static constexpr const char *MAPS_DIR = "/sys/fs/bpf/netsys/maps";
static constexpr const char *PROGS_DIR = "/sys/fs/bpf/netsys/progs";

/* Name of the shared bpf-to-bpf subprogram section. Noinline helpers called
 * by several prog sections are emitted by LLVM into .text exactly once, and
 * every prog section references them through pseudo calls. */
static constexpr const char *TEXT_SECTION_NAME = ".text";
/* R_BPF_INSN_DISP32 (value 10): pc-relative relocation carried on the imm of
 * a bpf-to-bpf pseudo call insn (opcode 0x85, src_reg = BPF_PSEUDO_CALL).
 * The call target is a subprogram inside the shared .text section; it is
 * rebased by ElfLoader::MergeTextInsns() while the .text insns are appended
 * to the prog insn array (BPF_PROG_LOAD accepts a single linear insn array). */
static constexpr unsigned R_BPF_INSN_DISP32 = 10U;

// There is no limit to the size of SECTION_NAMES.
static const struct SectionName {
    const char *sectionName;
    size_t sectionNameLength;
} SECTION_NAMES[] = {
    DEFINE_SECTION_NAME("kprobe/"),
    DEFINE_SECTION_NAME("kretprobe/"),
    DEFINE_SECTION_NAME("tracepoint/"),
    DEFINE_SECTION_NAME("raw_tracepoint/"),
    DEFINE_SECTION_NAME("xdp"),
    DEFINE_SECTION_NAME("perf_event/"),
    DEFINE_SECTION_NAME("socket"),
    DEFINE_SECTION_NAME("cgroup/"),
    DEFINE_SECTION_NAME("sockops"),
    DEFINE_SECTION_NAME("sk_skb"),
    DEFINE_SECTION_NAME("sk_msg"),
    DEFINE_SECTION_NAME("cgroup_skb"),
    DEFINE_SECTION_NAME("xdp_packet_parser"),
    DEFINE_SECTION_NAME("schedcls"),
    DEFINE_SECTION_NAME("classifier"),
    DEFINE_SECTION_NAME("cgroup_sock"),
    DEFINE_SECTION_NAME("cgroup_addr"),
};

static const constexpr struct {
    const char *event;
    bpf_prog_type progType;
} PROG_TYPES[] = {
    DEFINE_PROG_TYPE("socket", BPF_PROG_TYPE_SOCKET_FILTER),
    DEFINE_PROG_TYPE("cgroup_skb", BPF_PROG_TYPE_CGROUP_SKB),
    DEFINE_PROG_TYPE("xdp", BPF_PROG_TYPE_XDP),
    DEFINE_PROG_TYPE("xdp_packet_parser", BPF_PROG_TYPE_XDP),
    DEFINE_PROG_TYPE("schedcls", BPF_PROG_TYPE_SCHED_CLS),
    DEFINE_PROG_TYPE("classifier", BPF_PROG_TYPE_SCHED_CLS),
    DEFINE_PROG_TYPE("cgroup_sock", BPF_PROG_TYPE_CGROUP_SOCK),
    DEFINE_PROG_TYPE("cgroup_addr", BPF_PROG_TYPE_CGROUP_SOCK_ADDR),
};

static const constexpr struct {
    const char *progName;
    bpf_attach_type attachType;
    bool needExpectedAttach;
} PROG_ATTACH_TYPES[] = {
    DEFINE_ATTACH_TYPE("cgroup_sock_inet_create_socket", BPF_CGROUP_INET_SOCK_CREATE, false),
    DEFINE_ATTACH_TYPE("cgroup_sock_inet_release_socket", BPF_CGROUP_INET_SOCK_RELEASE, true),
    DEFINE_ATTACH_TYPE("cgroup_skb_uid_ingress", BPF_CGROUP_INET_INGRESS, false),
    /* fw-on dual attach: the *_dns progs attach ahead of the main progs
     * (ELF section order, see LoadProgs); BPF_F_ALLOW_MULTI stacks them on
     * the same cgroup with AND verdict semantics. */
    DEFINE_ATTACH_TYPE("cgroup_skb_uid_ingress_dns", BPF_CGROUP_INET_INGRESS, false),
    DEFINE_ATTACH_TYPE("cgroup_skb_uid_egress", BPF_CGROUP_INET_EGRESS, false),
    DEFINE_ATTACH_TYPE("cgroup_skb_uid_egress_dns", BPF_CGROUP_INET_EGRESS, false),
    DEFINE_ATTACH_TYPE("cgroup_addr_bind4", BPF_CGROUP_INET4_BIND, true),
    DEFINE_ATTACH_TYPE("cgroup_addr_bind6", BPF_CGROUP_INET6_BIND, true),
    DEFINE_ATTACH_TYPE("cgroup_addr_connect4", BPF_CGROUP_INET4_CONNECT, true),
    DEFINE_ATTACH_TYPE("cgroup_addr_connect6", BPF_CGROUP_INET6_CONNECT, true),
    DEFINE_ATTACH_TYPE("cgroup_addr_sendmsg4", BPF_CGROUP_UDP4_SENDMSG, true),
    DEFINE_ATTACH_TYPE("cgroup_addr_sendmsg6", BPF_CGROUP_UDP6_SENDMSG, true),
};

int32_t g_sockFd = -1;

struct BpfMapData {
    BpfMapData() : fd(0)
    {
        if (memset_s(&def, sizeof(def), 0, sizeof(def)) != EOK) {
            NETNATIVE_LOGE("memset_s error");
        }
    }

    int32_t fd;
    std::string name;
    bpf_map_def def{};
};

template <typename type> inline uint64_t PtrToU64(const type ptr)
{
    return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(ptr));
}

inline bool EndsWith(const std::string &str, const std::string &searchFor)
{
    if (searchFor.size() > str.size()) {
        return false;
    }

    std::string source = str.substr(str.size() - searchFor.size(), searchFor.size());
    return source == searchFor;
}

inline int32_t SysBpf(bpf_cmd cmd, bpf_attr *attr, uint32_t size)
{
    return static_cast<int32_t>(syscall(__NR_bpf, cmd, attr, size));
}

inline int32_t SysBpfObjGet(const std::string &pathName, uint32_t fileFlags)
{
    bpf_attr attr = {};
    if (memset_s(&attr, sizeof(attr), 0, sizeof(attr)) != EOK) {
        return NETMANAGER_ERROR;
    }
    attr.pathname = PtrToU64(pathName.c_str());
    attr.file_flags = fileFlags;
    return SysBpf(BPF_OBJ_GET, &attr, sizeof(attr));
}

inline int32_t SysBpfObjPin(int32_t fd, const std::string &pathName)
{
    bpf_attr attr = {};

    if (memset_s(&attr, sizeof(attr), 0, sizeof(attr)) != EOK) {
        return NETMANAGER_ERROR;
    }
    attr.pathname = PtrToU64(pathName.c_str());
    if (fd < 0) {
        return NETMANAGER_ERROR;
    }
    attr.bpf_fd = static_cast<uint32_t>(fd);

    return SysBpf(BPF_OBJ_PIN, &attr, sizeof(attr));
}

inline int32_t SysBpfProgLoad(bpf_attr *attr, uint32_t size)
{
    int32_t fd = SysBpf(BPF_PROG_LOAD, attr, size);
    while (fd < 0 && errno == EAGAIN) {
        fd = SysBpf(BPF_PROG_LOAD, attr, size);
    }

    return fd;
}

inline int32_t SysBpfObjDetach(bpf_attach_type type, const int progFd, const int cgFd)
{
    bpf_attr attr = {};

    if (memset_s(&attr, sizeof(attr), 0, sizeof(attr)) != EOK) {
        return NETMANAGER_ERROR;
    }
    attr.target_fd = cgFd;
    attr.attach_bpf_fd = progFd;
    attr.attach_type = type;

    return SysBpf(BPF_PROG_DETACH, &attr, sizeof(attr));
}

inline int32_t SysBpfObjAttach(bpf_attach_type type, const int progFd, const int cgFd)
{
    bpf_attr attr = {};

    if (memset_s(&attr, sizeof(attr), 0, sizeof(attr)) != EOK) {
        return NETMANAGER_ERROR;
    }
    attr.target_fd = cgFd;
    attr.attach_bpf_fd = progFd;
    attr.attach_type = type;
    attr.attach_flags = BPF_F_ALLOW_MULTI;

    return SysBpf(BPF_PROG_ATTACH, &attr, sizeof(attr));
}

inline bool MatchSecName(const std::string &name)
{
    auto matchFunc = [name](const SectionName &sec) -> bool {
        if (name.size() < sec.sectionNameLength) {
            return false;
        }
        return memcmp(name.c_str(), sec.sectionName, sec.sectionNameLength) == 0;
    };
    auto size = sizeof(SECTION_NAMES) / sizeof(SectionName);
    return std::any_of(SECTION_NAMES, SECTION_NAMES + size, matchFunc);
}

/* Prog pin names are the ELF section names with '/' replaced by '_'. */
inline std::string NormalizeProgName(const std::string &sectionName)
{
    std::string progName = sectionName;
    std::replace(progName.begin(), progName.end(), '/', '_');
    return progName;
}

inline int32_t UnPin(const std::string &path)
{
    return unlink(path.c_str());
}

class ElfLoader {
public:
    explicit ElfLoader(std::string path) : path_(std::move(path)), kernVersion_(0) {}

    ElfLoadError Unload() const
    {
        const struct {
            uint32_t index;
            const char *infoMsg;
            std::function<ElfLoadError()> fun;
            const char *errorMsg;
        } funList[]{
            {1, "path is valid", isPathValid_, "path is not valid"},
            {2, "load elf file ok", loadElfFile_, "load elf file failed"},
            {3, "load elf map section ok", loadElfMapsSection_, "load elf map section failed"},
            {4, "delete maps ok", deleteMaps_, "delete maps failed"},
            {5, "unload progs ok", unloadProgs_, "unload progs ok"},
        };

        for (const auto &fun : funList) {
            auto ret = fun.fun();
            if (ret != ELF_LOAD_ERR_NONE) {
                NETNATIVE_LOGE("error msg is %{public}s", fun.errorMsg);
                return static_cast<ElfLoadError>(ret);
            }
            NETNATIVE_LOGI("the %{public}u step: %{public}s", fun.index, fun.infoMsg);
        }

        return ELF_LOAD_ERR_NONE;
    }

    ElfLoadError Load() const
    {
        const struct {
            uint32_t index;
            const char *infoMsg;
            std::function<ElfLoadError()> fun;
            const char *errorMsg;
        } funList[]{
            {1, "path is valid", isPathValid_, "path is not valid"},
            {2, "make directories fs ok", makeDirectories, "make directories fs failed"},
            {3, "load elf file ok", loadElfFile_, "load elf file failed"},
            {4, "version is valid", isVersionValid_, "version is not valid"},
            {5, "set license and version ok", setLicenseAndVersion_, "set license and version failed"},
            {6, "load elf map section ok", loadElfMapsSection_, "load elf map section failed"},
            {7, "set rlimit ok", setRlimit_, "set rlimit failed"},
            {8, "create maps ok", createMaps_, "create maps failed"},
            {9, "parse relocation ok", parseRelocation_, "parse relocation failed"},
            {10, "load progs ok", loadProgs_, "load progs failed"},
        };
        for (const auto &fun : funList) {
            auto ret = fun.fun();
            if (ret != ELF_LOAD_ERR_NONE) {
                NETNATIVE_LOGE("error msg is %{public}s", fun.errorMsg);
                return static_cast<ElfLoadError>(ret);
            }
            NETNATIVE_LOGI("the %{public}u step: %{public}s", fun.index, fun.infoMsg);
        }

        return ELF_LOAD_ERR_NONE;
    }

private:
    bool CheckPath()
    {
        if (path_.empty() || !std::filesystem::exists(path_) || std::filesystem::is_directory(path_)) {
            return false;
        }

        return true;
    }

    bool IsPathValid()
    {
        if (!CheckPath()) {
            return false;
        }
        return EndsWith(path_, ".o");
    }

    bool LoadElfFile()
    {
        return elfIo_.load(path_);
    }

    bool IsVersionValid()
    {
        return elfIo_.get_version() == ELFIO::EV_CURRENT;
    }

    static bool SetRlimit()
    {
        rlimit r = {RLIM_INFINITY, RLIM_INFINITY};
        return setrlimit(RLIMIT_MEMLOCK, &r) >= 0;
    }

    static bool IsMounted(const std::string &dir)
    {
        std::ifstream ifs("/proc/mounts", std::ios::in);
        if (!ifs.is_open()) {
            return false;
        }

        std::string s;
        while (std::getline(ifs, s)) {
            if (s.find(dir) != std::string::npos) {
                ifs.close();
                return true;
            }
        }
        ifs.close();
        return false;
    }

    static bool MakeDir(const std::string &dir)
    {
        if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
            return true;
        }
        if (!std::filesystem::exists(dir)) {
            if (!std::filesystem::create_directories(std::filesystem::path(dir))) {
                NETNATIVE_LOGE("filesystem make dir err %{public}d", errno);
                return false;
            }
            return true;
        }
        if (!std::filesystem::is_directory(dir)) {
            NETNATIVE_LOGE("%{public}s is a file", dir.c_str());
            return false;
        }
        return true;
    }

    static bool MakeDirectories()
    {
        if (IsMounted(BPF_DIR) && std::filesystem::exists(MAPS_DIR) && std::filesystem::is_directory(MAPS_DIR) &&
            std::filesystem::exists(PROGS_DIR) && std::filesystem::is_directory(PROGS_DIR)) {
            NETNATIVE_LOGE("%{public}s", "bpf directories are exists");
            return true;
        }
        if (!IsMounted(BPF_DIR) && mount(BPF_DIR, BPF_DIR, "bpf", MS_RELATIME, nullptr) < 0) {
            NETNATIVE_LOGE("mount bpf fs failed: errno = %{public}d", errno);
            return false;
        }
        if (!MakeDir(MAPS_DIR)) {
            NETNATIVE_LOGE("can not make dir: %{public}s", MAPS_DIR);
            return false;
        }
        if (!MakeDir(PROGS_DIR)) {
            NETNATIVE_LOGE("can not make dir: %{public}s", PROGS_DIR);
            return false;
        }

        if (!IsMounted(CGROUP_DIR) && mount(CGROUP_DIR, CGROUP_DIR, "cgroup2", MS_RELATIME, nullptr) < 0) {
            NETNATIVE_LOGE("mount cgroup fs failed: errno = %{public}d", errno);
            return false;
        }
        return true;
    }

    bool SetLicenseAndVersion()
    {
        return std::all_of(elfIo_.sections.begin(), elfIo_.sections.end(), [this](const auto &section) {
            if (section->get_name() == "license") {
                license_ = section->get_data();
                if (license_.empty()) {
                    return false;
                }
            } else if (section->get_name() == "version") {
                try {
                    kernVersion_ = std::stoi(section->get_data());
                } catch (const std::invalid_argument& e) {
                    NETNATIVE_LOGE("invalid_argument");
                    return false;
                } catch (const std::out_of_range& e) {
                    NETNATIVE_LOGE("out_of_range");
                    return false;
                }
                if (kernVersion_ == 0) {
                    return false;
                }
            }

            return true;
        });
    }

    std::map<ELFIO::Elf64_Addr, std::string> LoadElfMapSectionCore()
    {
        std::map<ELFIO::Elf64_Addr, std::string> mapName;
        for (const auto &section : elfIo_.sections) {
            if (section->get_type() != ELFIO::SHT_SYMTAB && section->get_type() != ELFIO::SHT_DYNSYM) {
                continue;
            }
            ELFIO::symbol_section_accessor symbols(elfIo_, section.get());
            for (ELFIO::Elf_Xword i = 0; i < symbols.get_symbols_num(); i++) {
                std::string name;
                ELFIO::Elf64_Addr value = 0;
                ELFIO::Elf_Xword size = 0;
                unsigned char bind = 0;
                unsigned char type = 0;
                ELFIO::Elf_Half elfSection = 0;
                unsigned char other = 0;
                symbols.get_symbol(i, name, value, size, bind, type, elfSection, other);
                if (type != ELFIO::STT_OBJECT || !EndsWith(name, "_map")) {
                    continue;
                }
                if (mapName.find(value) != mapName.end()) {
                    return {};
                }
                mapName[value] = name;
            }
        }
        return mapName;
    }

    bool LoadElfMapsSection()
    {
        if (elfIo_.sections.size() == 0) {
            return false;
        }

        auto it = std::find_if(elfIo_.sections.begin(), elfIo_.sections.end(),
                               [](const auto &section) { return section->get_name() == "maps"; });
        if (it == elfIo_.sections.end()) {
            return true;
        }

        ELFIO::section *mapsSection = it->get();
        auto defs = reinterpret_cast<const bpf_map_def *>(mapsSection->get_data());
        auto mapNum = mapsSection->get_size() / sizeof(bpf_map_def);
        for (size_t i = 0; i < static_cast<size_t>(mapNum); i++) {
            BpfMapData map;
            map.def = defs[i];
            maps_.emplace_back(map);
        }
        auto mapName = LoadElfMapSectionCore();
        if (mapName.size() != maps_.size()) {
            return false;
        }
        size_t mapIndex = 0;
        for (const auto &[addr, name] : mapName) {
            maps_[mapIndex].name = name;
            ++mapIndex;
        }

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
        bpf_attr attr = {};
        if (memset_s(&attr, sizeof(attr), 0, sizeof(attr)) != EOK) {
            return NETMANAGER_ERROR;
        }
        attr.map_type = map.def.type;
        attr.key_size = map.def.key_size;
        attr.value_size = map.def.value_size;
        attr.max_entries = map.def.max_entries;
        attr.map_flags = map.def.map_flags;
        if (!map.name.empty()) {
            if (memcpy_s(attr.map_name, sizeof(attr.map_name) - 1, map.name.c_str(),
                         std::min<size_t>(map.name.size(), sizeof(attr.map_name) - 1)) != EOK) {
                NETNATIVE_LOGE("Failed copy map name %{public}s", map.name.c_str());
                return NETMANAGER_ERROR;
            }
        }
        attr.numa_node = (map.def.map_flags & static_cast<unsigned int>(BPF_F_NUMA_NODE)) ? map.def.numa_node : 0;
        PrintMapAttr(attr);

        auto fd = SysBpf(BPF_MAP_CREATE, &attr, sizeof(attr));
        if (fd < 0) {
            NETNATIVE_LOGE("__NR_bpf, BPF_MAP_CREATE failed %{public}d %{public}d %{public}d", __NR_bpf, fd, errno);
        }
        return fd;
    }

    bool CreateMaps()
    {
        for (auto &map : maps_) {
            auto fd = BpfCreateMapNode(map);
            if (fd < 0) {
                NETNATIVE_LOGE("Failed create map (%{public}s): %{public}d", map.name.c_str(), fd);
                return false;
            }

            map.fd = fd;
            std::string mapPinLocation = std::string(MAPS_DIR) + "/" + map.name;
            if (access(mapPinLocation.c_str(), F_OK) == 0) {
                NETNATIVE_LOGI("map: %{public}s has already been pinned", mapPinLocation.c_str());
            } else {
                if (SysBpfObjPin(fd, mapPinLocation) < 0) {
                    NETNATIVE_LOGE("Failed to pin map: %{public}s, errno = %{public}d", mapPinLocation.c_str(), errno);
                    return false;
                }
            }
        }
        return true;
    }

    bool DeleteMaps()
    {
        return std::all_of(maps_.begin(), maps_.end(), [](const auto &map) {
            std::string mapPinLocation = std::string(MAPS_DIR) + "/" + map.name;
            if (access(mapPinLocation.c_str(), F_OK) == 0) {
                auto ret = UnPin(mapPinLocation);
                return ret >= 0;
            }
            return true;
        });
    }

    bool ApplyRelocation(bpf_insn *insn, ELFIO::section *section) const
    {
        if (insn == nullptr || section == nullptr || section->get_entry_size() == 0) {
            return false;
        }

        auto size = section->get_size() / section->get_entry_size();
        if (size == 0) {
            return false;
        }

        ELFIO::Elf64_Addr offset = 0;
        ELFIO::Elf64_Addr symbolValue = 0;
        std::string symbolName;
        ELFIO::Elf_Word type = 0;
        ELFIO::Elf_Sxword addend = 0;
        ELFIO::Elf_Sxword calcValue = 0;
        ELFIO::relocation_section_accessor relocation(elfIo_, section);
        for (size_t i = 0; i < size; i++) {
            relocation.get_entry(i, offset, symbolValue, symbolName, type, addend, calcValue);
            if (type == R_BPF_INSN_DISP32) {
                /* bpf-to-bpf pseudo-call displacement, not a map reference:
                 * rebased by MergeTextInsns() when the .text insns are merged
                 * into the prog insn array during LoadProgs(). */
                continue;
            }
            uint32_t index = offset / sizeof(bpf_insn);
            if (insn[index].code != (BPF_LD | BPF_IMM | BPF_DW)) {
                NETNATIVE_LOGE("Invalid relo for insn[%{public}u].code 0x%{public}x 0x%{public}x", index,
                               insn[index].code, (BPF_LD | BPF_IMM | BPF_DW));
                continue;
            }

            size_t mapIdx;
            bool match = false;
            for (mapIdx = 0; mapIdx < maps_.size(); mapIdx++) {
                if (maps_[mapIdx].name == symbolName) {
                    match = true;
                    break;
                }
            }
            if (!match) {
                NETNATIVE_LOGE("Invalid relo for insn[%{public}u] no map_data match %{public}s index %{public}zu",
                               index, section->get_name().c_str(), i);
                continue;
            }
            insn[index].src_reg = BPF_PSEUDO_MAP_FD;
            insn[index].imm = maps_[mapIdx].fd;
        }
        return true;
    }

    /* Find the SHT_REL section (e.g. ".relcgroup_skb/uid/ingress" or ".rel.text")
     * that relocates the given target section, matched through its sh_info. */
    ELFIO::section *FindRelSection(const ELFIO::section *targetSec) const
    {
        for (const auto &section : elfIo_.sections) {
            if (section->get_type() != ELFIO::SHT_REL) {
                continue;
            }
            if (elfIo_.sections[section->get_info()] == targetSec) {
                return section.get();
            }
        }
        return nullptr;
    }

    /* A bpf-to-bpf pseudo call into the shared .text section shows up as an
     * R_BPF_INSN_DISP32 relocation in the prog's own relocation section. */
    bool HasTextCallRelocation(ELFIO::section *relSec) const
    {
        if (relSec == nullptr) {
            return false;
        }
        ELFIO::relocation_section_accessor relocation(elfIo_, relSec);
        for (ELFIO::Elf_Xword i = 0; i < relocation.get_entries_num(); i++) {
            ELFIO::Elf64_Addr offset = 0;
            ELFIO::Elf64_Addr symbolValue = 0;
            std::string symbolName;
            ELFIO::Elf_Word type = 0;
            ELFIO::Elf_Sxword addend = 0;
            ELFIO::Elf_Sxword calcValue = 0;
            relocation.get_entry(i, offset, symbolValue, symbolName, type, addend, calcValue);
            if (type == R_BPF_INSN_DISP32) {
                return true;
            }
        }
        return false;
    }

    /* One .text subprogram: a STT_FUNC symbol living in the .text section. */
    struct TextFunc {
        std::string name;
        size_t off = 0;    /* insn offset inside .text */
        size_t cnt = 0;    /* insn count */
        size_t newpos = 0; /* insn offset inside the merged array, valid when kept */
        bool kept = false;
    };

    /* .text function containing the given .text insn offset, -1 if none */
    static int TextFuncOf(const std::vector<TextFunc> &funcs, size_t insnOff)
    {
        for (size_t i = 0; i < funcs.size(); i++) {
            if (insnOff >= funcs[i].off && insnOff < funcs[i].off + funcs[i].cnt) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    /* Build the .text function table from the STT_FUNC symbols whose
     * section index is the .text section, sorted by insn offset. */
    bool BuildTextFuncTable(const ELFIO::section *textSec, std::vector<TextFunc> &funcs) const
    {
        auto textIdx = textSec->get_index();
        size_t textCnt = textSec->get_size() / sizeof(bpf_insn);
        for (const auto &section : elfIo_.sections) {
            if (section->get_type() != ELFIO::SHT_SYMTAB) {
                continue;
            }
            ELFIO::symbol_section_accessor symbols(elfIo_, section.get());
            for (ELFIO::Elf_Xword i = 0; i < symbols.get_symbols_num(); i++) {
                std::string name;
                ELFIO::Elf64_Addr value = 0;
                ELFIO::Elf_Xword size = 0;
                unsigned char bind = 0;
                unsigned char type = 0;
                ELFIO::Elf_Half elfSection = 0;
                unsigned char other = 0;
                symbols.get_symbol(i, name, value, size, bind, type, elfSection, other);
                if (type != ELFIO::STT_FUNC || elfSection != textIdx) {
                    continue;
                }
                if (value % sizeof(bpf_insn) != 0 || size % sizeof(bpf_insn) != 0) {
                    NETNATIVE_LOGE(".text function %{public}s misaligned (value %{public}lld size %{public}lld)",
                                   name.c_str(), static_cast<long long>(value), static_cast<long long>(size));
                    return false;
                }
                TextFunc func;
                func.name = name;
                func.off = value / sizeof(bpf_insn);
                func.cnt = size / sizeof(bpf_insn);
                if (func.cnt == 0 || func.off + func.cnt > textCnt) {
                    NETNATIVE_LOGE(".text function %{public}s out of section (off %{public}zu cnt %{public}zu, "
                                   ".text %{public}zu insns)",
                                   name.c_str(), func.off, func.cnt, textCnt);
                    return false;
                }
                funcs.push_back(func);
            }
        }
        if (funcs.empty()) {
            NETNATIVE_LOGE("%{public}s", "prog calls .text but no STT_FUNC symbols found in .text");
            return false;
        }
        std::sort(funcs.begin(), funcs.end(), [](const TextFunc &a, const TextFunc &b) { return a.off < b.off; });
        for (size_t i = 1; i < funcs.size(); i++) {
            if (funcs[i - 1].off + funcs[i - 1].cnt > funcs[i].off) {
                NETNATIVE_LOGE(".text functions %{public}s and %{public}s overlap", funcs[i - 1].name.c_str(),
                               funcs[i].name.c_str());
                return false;
            }
        }
        return true;
    }

    /* Dead-strip merge of the shared .text subprograms into the prog insn
     * array, and rebase of every bpf-to-bpf pseudo call displacement.
     *
     * BPF_PROG_LOAD takes a single linear insn array and the verifier
     * rejects unreachable insns: the prog sections reference different
     * subsets of the shared .text functions, so only the functions
     * reachable from THIS prog are appended (dead-strip). Algorithm
     * (device-verified against the netsys.o firewall programs):
     *  1. collect the prog -> .text pseudo calls: R_BPF_INSN_DISP32
     *     relocations in the prog's own relocation section;
     *  2. build the .text function table (STT_FUNC symbols);
     *  3. scan .text for pseudo calls (code 0x85, src_reg =
     *     BPF_PSEUDO_CALL; helper calls carry src_reg = 0 and their imm is
     *     a helper id - NOT an edge) and build caller -> callee edges;
     *  4. reachable closure starting from the prog's direct callees;
     *  5. append the kept functions, in original .text order, behind the
     *     prog insns;
     *  6. rebase the prog -> .text calls:
     *       target insn = st_value / sizeof(bpf_insn) + (s32)imm + 1
     *     (both spellings: the .text section symbol carries target - 1 in
     *     the imm, a FUNC symbol rides st_value with imm = -1)
     *       final imm   = new_target - insn_idx - 1
     *  7. rebase the intra-.text calls inside kept functions (they carry
     *     no relocation, the imm is the .text-internal displacement):
     *       final imm   = new_target - new_call - 1
     * Map relocations of both sections (R_BPF_64_64) were already patched
     * in place by ParseRelocation(), so the patched ld_imm64 insns ride
     * along with whichever functions are kept. Jumps/loads inside a
     * function stay valid: functions are copied as contiguous blocks. */
    bool MergeTextInsns(const ELFIO::section *progSec, ELFIO::section *relSec,
                        std::vector<bpf_insn> &mergedInsns) const
    {
        auto *textSec = elfIo_.sections[std::string_view(TEXT_SECTION_NAME)];
        if (textSec == nullptr || textSec->get_type() != ELFIO::SHT_PROGBITS ||
            (textSec->get_flags() & ELFIO::SHF_EXECINSTR) == 0) {
            NETNATIVE_LOGE("prog %{public}s calls .text subprograms but no executable .text section found",
                           progSec->get_name().c_str());
            return false;
        }

        size_t progCnt = progSec->get_size() / sizeof(bpf_insn);
        size_t textCnt = textSec->get_size() / sizeof(bpf_insn);
        const bpf_insn *progInsns = reinterpret_cast<const bpf_insn *>(progSec->get_data());
        const bpf_insn *textInsns = reinterpret_cast<const bpf_insn *>(textSec->get_data());
        if (progCnt == 0 || textCnt == 0 || progInsns == nullptr || textInsns == nullptr) {
            NETNATIVE_LOGE("empty prog or .text section: prog[%{public}s] %{public}zu insns, .text %{public}zu insns",
                           progSec->get_name().c_str(), progCnt, textCnt);
            return false;
        }
        NETNATIVE_LOGI(".text section: %{public}zu insns, prog[%{public}s] has %{public}zu insns", textCnt,
                       progSec->get_name().c_str(), progCnt);

        /* 1) prog -> .text pseudo calls (DISP32 relocations) */
        struct DirectCall {
            size_t insnIdx; /* call insn index inside the prog section */
            int64_t target; /* call target insn offset inside .text */
            std::string symbolName;
        };
        std::vector<DirectCall> directCalls;
        ELFIO::relocation_section_accessor relocation(elfIo_, relSec);
        for (ELFIO::Elf_Xword i = 0; i < relocation.get_entries_num(); i++) {
            ELFIO::Elf64_Addr offset = 0;
            ELFIO::Elf64_Addr symbolValue = 0;
            std::string symbolName;
            ELFIO::Elf_Word type = 0;
            ELFIO::Elf_Sxword addend = 0;
            ELFIO::Elf_Sxword calcValue = 0;
            if (!relocation.get_entry(i, offset, symbolValue, symbolName, type, addend, calcValue)) {
                NETNATIVE_LOGE("invalid relocation entry %{public}u in %{public}s", static_cast<unsigned>(i),
                               relSec->get_name().c_str());
                return false;
            }
            if (type != R_BPF_INSN_DISP32) {
                /* map relocations were already applied in place by ParseRelocation() */
                continue;
            }
            size_t insnIdx = offset / sizeof(bpf_insn);
            if (offset % sizeof(bpf_insn) != 0 || insnIdx >= progCnt) {
                NETNATIVE_LOGE("pseudo call relocation offset %{public}lld out of prog section %{public}s",
                               static_cast<long long>(offset), progSec->get_name().c_str());
                return false;
            }
            const bpf_insn &insn = progInsns[insnIdx];
            if (insn.code != (BPF_JMP | BPF_CALL) || insn.src_reg != BPF_PSEUDO_CALL) {
                NETNATIVE_LOGE("relocation on insn[%{public}zu] of %{public}s is not a bpf-to-bpf pseudo call: "
                               "code 0x%{public}x src_reg %{public}u",
                               insnIdx, progSec->get_name().c_str(), insn.code, static_cast<unsigned>(insn.src_reg));
                return false;
            }
            if (symbolValue % sizeof(bpf_insn) != 0) {
                NETNATIVE_LOGE("pseudo call symbol %{public}s has misaligned value %{public}lld", symbolName.c_str(),
                               static_cast<long long>(symbolValue));
                return false;
            }
            int64_t target = static_cast<int64_t>(symbolValue) / static_cast<int64_t>(sizeof(bpf_insn)) +
                             static_cast<int64_t>(insn.imm) + 1;
            if (target < 0 || target >= static_cast<int64_t>(textCnt)) {
                NETNATIVE_LOGE("pseudo call target %{public}lld out of .text section (%{public}zu insns)",
                               static_cast<long long>(target), textCnt);
                return false;
            }
            directCalls.push_back({insnIdx, target, symbolName});
        }
        if (directCalls.empty()) {
            NETNATIVE_LOGE("%{public}s", "no prog -> .text pseudo call relocations found");
            return false;
        }

        /* 2) .text function table (STT_FUNC symbols, sorted by offset) */
        std::vector<TextFunc> funcs;
        if (!BuildTextFuncTable(textSec, funcs)) {
            return false;
        }

        /* 3) intra-.text pseudo call edges */
        std::vector<std::vector<bool>> edge(funcs.size(), std::vector<bool>(funcs.size(), false));
        for (size_t i = 0; i < textCnt; i++) {
            const bpf_insn &insn = textInsns[i];
            if (insn.code != (BPF_JMP | BPF_CALL) || insn.src_reg != BPF_PSEUDO_CALL) {
                continue; /* helper calls carry src_reg = 0, their imm is a helper id */
            }
            int64_t t = static_cast<int64_t>(i) + static_cast<int64_t>(insn.imm) + 1;
            if (t < 0 || t >= static_cast<int64_t>(textCnt)) {
                NETNATIVE_LOGE("intra .text call at insn %{public}zu targets %{public}lld out of .text", i,
                               static_cast<long long>(t));
                return false;
            }
            int caller = TextFuncOf(funcs, i);
            int callee = TextFuncOf(funcs, static_cast<size_t>(t));
            if (caller < 0 || callee < 0) {
                NETNATIVE_LOGE("intra .text call at insn %{public}zu sits outside any .text function", i);
                return false;
            }
            edge[caller][callee] = true;
        }

        /* 4) reachable closure from the prog's direct callees */
        for (const auto &call : directCalls) {
            int f = TextFuncOf(funcs, static_cast<size_t>(call.target));
            if (f < 0) {
                NETNATIVE_LOGE("prog -> .text call target %{public}lld outside any .text function (symbol %{public}s)",
                               static_cast<long long>(call.target), call.symbolName.c_str());
                return false;
            }
            funcs[f].kept = true;
        }
        bool changed = true;
        while (changed) {
            changed = false;
            for (size_t a = 0; a < funcs.size(); a++) {
                if (!funcs[a].kept) {
                    continue;
                }
                for (size_t b = 0; b < funcs.size(); b++) {
                    if (edge[a][b] && !funcs[b].kept) {
                        funcs[b].kept = true;
                        changed = true;
                    }
                }
            }
        }

        /* 5) layout: kept functions in original .text order behind the prog insns */
        mergedInsns.assign(progInsns, progInsns + progCnt);
        size_t keptInsns = 0;
        size_t keptFuncs = 0;
        std::string keptNames;
        for (auto &f : funcs) {
            if (!f.kept) {
                continue;
            }
            f.newpos = mergedInsns.size();
            mergedInsns.insert(mergedInsns.end(), textInsns + f.off, textInsns + f.off + f.cnt);
            if (!keptNames.empty()) {
                keptNames += ", ";
            }
            keptNames += f.name;
            keptInsns += f.cnt;
            ++keptFuncs;
        }

        /* 6) rebase the prog -> .text pseudo calls (imm still original) */
        size_t rebasedCnt = 0;
        for (const auto &call : directCalls) {
            bpf_insn &insn = mergedInsns[call.insnIdx];
            int f = TextFuncOf(funcs, static_cast<size_t>(call.target));
            if (f < 0 || !funcs[f].kept) {
                NETNATIVE_LOGE("prog -> .text call target not kept (closure bug), symbol %{public}s",
                               call.symbolName.c_str());
                return false;
            }
            int64_t newTarget =
                static_cast<int64_t>(funcs[f].newpos) + (call.target - static_cast<int64_t>(funcs[f].off));
            int32_t newImm = static_cast<int32_t>(newTarget - static_cast<int64_t>(call.insnIdx) - 1);
            NETNATIVE_LOGI("patch pseudo call in prog[%{public}s]: insn[%{public}zu] imm %{public}d -> %{public}d, "
                           ".text target insn %{public}lld (symbol %{public}s)",
                           progSec->get_name().c_str(), call.insnIdx, insn.imm, newImm,
                           static_cast<long long>(call.target), call.symbolName.c_str());
            insn.imm = newImm;
            ++rebasedCnt;
        }

        /* 7) rebase the intra-.text calls inside kept functions (imm is
         * still the .text-internal displacement) */
        for (const auto &f : funcs) {
            if (!f.kept) {
                continue;
            }
            for (size_t i = f.off; i < f.off + f.cnt; i++) {
                bpf_insn &insn = mergedInsns[f.newpos + (i - f.off)];
                if (insn.code != (BPF_JMP | BPF_CALL) || insn.src_reg != BPF_PSEUDO_CALL) {
                    continue;
                }
                int64_t t = static_cast<int64_t>(i) + static_cast<int64_t>(insn.imm) + 1;
                int tf = (t >= 0 && t < static_cast<int64_t>(textCnt)) ? TextFuncOf(funcs, static_cast<size_t>(t)) : -1;
                if (tf < 0 || !funcs[tf].kept) {
                    NETNATIVE_LOGE("kept function %{public}s calls a non-kept function (closure bug)", f.name.c_str());
                    return false;
                }
                int64_t newCall = static_cast<int64_t>(f.newpos) + static_cast<int64_t>(i - f.off);
                int64_t newTarget =
                    static_cast<int64_t>(funcs[tf].newpos) + (t - static_cast<int64_t>(funcs[tf].off));
                insn.imm = static_cast<int32_t>(newTarget - newCall - 1);
                ++rebasedCnt;
            }
        }

        NETNATIVE_LOGI("prog[%{public}s] dead-strip merge: kept %{public}zu/%{public}zu .text functions, "
                       "%{public}zu insns appended (total %{public}zu), %{public}zu calls rebased, kept: %{public}s",
                       progSec->get_name().c_str(), keptFuncs, funcs.size(), keptInsns, mergedInsns.size(),
                       rebasedCnt, keptNames.c_str());
        return true;
    }

    /* Load the program with the BPF verifier log attached. Called only when the first load
     * attempt fails, so that the exact verifier rejection reason is printed into the log. */
    static int32_t LoadProgWithVerifierLog(const std::string &progName, bpf_attr attr)
    {
        constexpr uint32_t verifierLogSize = 1024U * 1024U;
        constexpr size_t logTailSize = 16U * 1024U;
        constexpr size_t logChunkSize = 512U;
        std::vector<char> verifierLog(verifierLogSize, 0);
        attr.log_buf = PtrToU64(verifierLog.data());
        attr.log_size = verifierLogSize;
        attr.log_level = 1; /* 1: verifier state log, the rejection reason is at the end */

        int32_t progFd = SysBpfProgLoad(&attr, sizeof(attr));
        std::string log(verifierLog.data(), strnlen(verifierLog.data(), verifierLogSize));
        NETNATIVE_LOGE("BPF verifier log: prog[%{public}s] loadResult[%{public}d] logSize[%{public}zu]",
                       progName.c_str(), progFd, log.size());
        size_t begin = 0;
        if (log.size() > logTailSize) {
            begin = log.size() - logTailSize;
            NETNATIVE_LOGE("BPF verifier log is too long, only print the last %{public}zu bytes", logTailSize);
        }
        for (size_t off = begin; off < log.size(); off += logChunkSize) {
            NETNATIVE_LOGE("verifier[%{public}zu/%{public}zu]: %{public}s", off, log.size(),
                           log.substr(off, logChunkSize).c_str());
        }
        return progFd;
    }

    int32_t BpfLoadProgram(std::string &progName, bpf_prog_type type, const bpf_insn *insns, size_t insnsCnt)
    {
        if (insns == nullptr) {
            return NETMANAGER_ERROR;
        }

        bpf_attr attr = {};
        if (memset_s(&attr, sizeof(attr), 0, sizeof(attr)) != EOK) {
            return NETMANAGER_ERROR;
        }
        attr.prog_type = type;
        if (kernVersion_ < 0) {
            return NETMANAGER_ERROR;
        }
        attr.kern_version = static_cast<uint32_t>(kernVersion_);
        attr.insn_cnt = static_cast<uint32_t>(insnsCnt);
        attr.insns = PtrToU64(insns);
        attr.license = PtrToU64(license_.c_str());
        for (const auto &prog : PROG_ATTACH_TYPES) {
            if (prog.progName != nullptr && progName == prog.progName) {
                if (prog.needExpectedAttach) {
                    attr.expected_attach_type = prog.attachType;
                }
                NETNATIVE_LOGW(
                    "BpfLoadProgram progName[%{public}s] attrProgType[%{public}u], "
                    "attrExpectedAttachType[%{public}u]",
                    progName.c_str(), attr.prog_type, attr.expected_attach_type);
                break;
            }
        }

        int32_t progFd = SysBpfProgLoad(&attr, sizeof(attr));
        if (progFd < NETSYS_SUCCESS) {
            int32_t originErrno = errno;
            progFd = LoadProgWithVerifierLog(progName, attr);
            if (progFd < NETSYS_SUCCESS) {
                errno = originErrno;
            }
        }
        return progFd;
    }

    static bpf_prog_type ConvertEventToProgType(const std::string &event)
    {
        for (const auto &prog : PROG_TYPES) {
            size_t size = strlen(prog.event);
            if (event.size() < size) {
                continue;
            }
            if (memcmp(event.c_str(), prog.event, size) == 0) {
                return prog.progType;
            }
        }
        return static_cast<bpf_prog_type>(NETMANAGER_ERROR);
    }

    static bool DoAttach(int32_t progFd, const std::string &progName)
    {
        if (progName.size() < 1) {
            NETNATIVE_LOGE("progName is null");
            return false;
        }
        NETNATIVE_LOG_D("The progName = %{public}s", progName.c_str());

        for (const auto &prog : PROG_ATTACH_TYPES) {
            if (prog.progName != nullptr && progName == prog.progName) {
                int cgroupFd = open(CGROUP_DIR, O_DIRECTORY | O_RDONLY | O_CLOEXEC);
                if (cgroupFd < 0) {
                    NETNATIVE_LOGE("open CGROUP_DIR failed: errno = %{public}d", errno);
                    return false;
                }

                if (SysBpfObjAttach(prog.attachType, progFd, cgroupFd) < NETSYS_SUCCESS) {
                    NETNATIVE_LOGE("attach %{public}s failed: errno = %{public}d", progName.c_str(), errno);
                    close(cgroupFd);
                    return false;
                }

                close(cgroupFd);
                return true;
            }
        }

        return true;
    }

    static void DoDetach(const std::string &progPinLocation, const std::string &progName)
    {
        if (progName.size() < 1) {
            NETNATIVE_LOGE("progName is null");
            return;
        }
        NETNATIVE_LOG_D("The progName = %{public}s", progName.c_str());

        for (const auto &prog : PROG_ATTACH_TYPES) {
            if (prog.progName != nullptr && progName == prog.progName) {
                int cgroupFd = open(CGROUP_DIR, O_DIRECTORY | O_RDONLY | O_CLOEXEC);
                if (cgroupFd < NETSYS_SUCCESS) {
                    NETNATIVE_LOGE("open CGROUP_DIR failed: errno = %{public}d", errno);
                    return;
                }

                auto progFd = SysBpfObjGet(progPinLocation, 0);
                if (progFd < NETSYS_SUCCESS) {
                    close(cgroupFd);
                    return;
                }

                if (SysBpfObjDetach(prog.attachType, progFd, cgroupFd) < NETSYS_SUCCESS) {
                    NETNATIVE_LOGE("detach %{public}s failed: errno = %{public}d", progName.c_str(), errno);
                    close(cgroupFd);
                    return;
                }

                close(cgroupFd);
                return;
            }
        }
    }

    bool LoadProg(const std::string &event, const bpf_insn *insn, size_t insnCnt)
    {
        auto progType = ConvertEventToProgType(event);
        if (progType < NETSYS_SUCCESS) {
            NETNATIVE_LOGE("unsupported program type: %{public}s", event.c_str());
            return false;
        }

        if (insn == nullptr) {
            NETNATIVE_LOGE("insn is null");
            return false;
        }

        std::string progName = NormalizeProgName(event);
        int32_t progFd = BpfLoadProgram(progName, progType, insn, insnCnt);
        if (progFd < NETSYS_SUCCESS) {
            NETNATIVE_LOGE("Failed to load bpf prog, error = %{public}d", errno);
            return false;
        }

        std::string progPinLocation = std::string(PROGS_DIR) + "/" + progName;
        if (access(progPinLocation.c_str(), F_OK) == 0) {
            NETNATIVE_LOGI("prog: %{public}s has already been pinned", progPinLocation.c_str());
        } else {
            if (SysBpfObjPin(progFd, progPinLocation) < NETSYS_SUCCESS) {
                NETNATIVE_LOGE("Failed to pin prog: %{public}s, errno = %{public}d", progPinLocation.c_str(), errno);
                close(progFd);
                return false;
            }
        }

        /* attach socket filter */
        if (progType == BPF_PROG_TYPE_SOCKET_FILTER) {
            if (g_sockFd < 0) {
                NETNATIVE_LOGE("create socket failed, %{public}d, err: %{public}d", g_sockFd, errno);
                close(progFd);
                /* return true to ignore this prog */
                return true;
            }
            if (setsockopt(g_sockFd, SOL_SOCKET, SO_ATTACH_BPF, &progFd, sizeof(progFd)) < 0) {
                NETNATIVE_LOGE("attach socket failed, err: %{public}d", errno);
                close(g_sockFd);
                g_sockFd = -1;
            }
            close(progFd);
            return true;
        } else {
            auto ret = DoAttach(progFd, progName);
            close(progFd);
            return ret;
        }
    }

    bool LoadProgSection(const ELFIO::section *progSec)
    {
        /* bpf-to-bpf subprograms: when the prog pseudo-calls into the shared
         * .text section, append the .text insns behind the prog insns and
         * rebase the call displacements before loading; otherwise load the
         * section data as before (ELFs without .text calls are unaffected). */
        auto relSec = FindRelSection(progSec);
        if (relSec != nullptr && HasTextCallRelocation(relSec)) {
            std::vector<bpf_insn> mergedInsns;
            if (!MergeTextInsns(progSec, relSec, mergedInsns)) {
                return false;
            }
            return LoadProg(progSec->get_name(), mergedInsns.data(), mergedInsns.size());
        }
        return LoadProg(progSec->get_name(), reinterpret_cast<const bpf_insn *>(progSec->get_data()),
                        progSec->get_size() / sizeof(bpf_insn));
    }

    bool ParseRelocation()
    {
        return std::all_of(elfIo_.sections.begin(), elfIo_.sections.end(), [this](auto &section) -> bool {
            if (section->get_type() != ELFIO::SHT_REL) {
                return true;
            }

            auto info = section->get_info();
            auto progSec = elfIo_.sections[info];
            if (progSec == nullptr) {
                return true;
            }

            if (progSec->get_type() != ELFIO::SHT_PROGBITS || ((progSec->get_flags() & ELFIO::SHF_EXECINSTR) == 0)) {
                return true;
            }

            auto insn = reinterpret_cast<bpf_insn *>(const_cast<char *>(progSec->get_data()));
            if (insn == nullptr) {
                return false;
            }
            if (!ApplyRelocation(insn, section.get())) {
                return false;
            }
            return true;
        });
    }

    bool UnloadProgs()
    {
        if (g_sockFd > 0) {
            close(g_sockFd);
            g_sockFd = -1;
        }
        return std::all_of(elfIo_.sections.begin(), elfIo_.sections.end(), [this](const auto &section) -> bool {
            if (!MatchSecName(section->get_name())) {
                return true;
            }

            std::string progName = section->get_name();
            std::replace(progName.begin(), progName.end(), '/', '_');
            std::string progPinLocation = std::string(PROGS_DIR) + "/" + progName;
            if (access(progPinLocation.c_str(), F_OK) == 0) {
                DoDetach(progPinLocation, progName);

                return UnPin(progPinLocation) < NETSYS_SUCCESS ? false : true;
            }
            return true;
        });
    }

    bool LoadProgs()
    {
        if (g_sockFd > 0) {
            close(g_sockFd);
        }
        g_sockFd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        /* elfIo_.sections preserves the ELF section order, so the progs are
         * loaded (and attached through LoadProg -> DoAttach) exactly in the
         * order the BPF build laid them out: the *_dns gate programs land
         * ahead of the main ingress/egress programs on the same cgroup
         * (BPF_F_ALLOW_MULTI, AND verdict semantics). */
        return std::all_of(elfIo_.sections.begin(), elfIo_.sections.end(), [this](const auto &section) -> bool {
            if (!MatchSecName(section->get_name())) {
                return true;
            }
            return LoadProgSection(section.get());
        });
    }

    std::string path_;
    ELFIO::elfio elfIo_;
    std::string license_;
    int32_t kernVersion_;
    std::vector<BpfMapData> maps_;

    std::function<ElfLoadError()> isPathValid_ = [this]() -> ElfLoadError {
        if (!IsPathValid()) {
            return ELF_LOAD_ERR_PATH_INVALID;
        }
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> makeDirectories = []() -> ElfLoadError {
        if (!MakeDirectories()) {
            return ELF_LOAD_ERR_MAKE_DIR_FAIL;
        }
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> loadElfFile_ = [this]() -> ElfLoadError {
        if (!LoadElfFile()) {
            return ELF_LOAD_ERR_LOAD_FILE_FAIL;
        }
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> isVersionValid_ = [this]() -> ElfLoadError {
        if (!IsVersionValid()) {
            return ELF_LOAD_ERR_GET_VERSION_FAIL;
        }
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> setLicenseAndVersion_ = [this]() -> ElfLoadError {
        if (!SetLicenseAndVersion()) {
            return ELF_LOAD_ERR_SELECT_LICENSE_AND_VERSION_FAIL;
        }
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> loadElfMapsSection_ = [this]() -> ElfLoadError {
        if (!LoadElfMapsSection()) {
            return ELF_LOAD_ERR_LOAD_MAP_SECTION_FAIL;
        }
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> setRlimit_ = []() -> ElfLoadError {
        if (!SetRlimit()) {
            return ELF_LOAD_ERR_SET_RLIMIT_FAIL;
        }
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> createMaps_ = [this]() -> ElfLoadError {
        if (!CreateMaps()) {
            return ELF_LOAD_ERR_CREATE_MAP_FAIL;
        }
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> parseRelocation_ = [this]() -> ElfLoadError {
        if (!ParseRelocation()) {
            return ELF_LOAD_ERR_PARSE_RELOCATION_FAIL;
        }
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> loadProgs_ = [this]() -> ElfLoadError {
        if (!LoadProgs()) {
            return ELF_LOAD_ERR_LOAD_PROGS_FAIL;
        }
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> deleteMaps_ = [this]() -> ElfLoadError {
        if (!DeleteMaps()) {
            return ELF_LOAD_ERR_DELETE_MAP_FAIL;
        }
        return ELF_LOAD_ERR_NONE;
    };

    std::function<ElfLoadError()> unloadProgs_ = [this]() -> ElfLoadError {
        if (!UnloadProgs()) {
            return ELF_LOAD_ERR_UNLOAD_PROGS_FAIL;
        }
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
