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

#ifndef BPF_UTILS_H
#define BPF_UTILS_H

#include <string>

#include <arpa/inet.h>
#include <sys/syscall.h>
#include <linux/bpf.h>

#include <cerrno>
#include <fcntl.h>
#include <net/if.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#include "netnative_log_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace Bpf {
struct BpfLoadMapDef {
    uint32_t type;
    uint32_t keySize;
    uint32_t valueSize;
    uint32_t maxEntries;
    uint32_t mapFlags;
    uint32_t innerMapIdx;
    uint32_t numaNode;
};

struct BpfMapData {
    int32_t fd;
    std::string name;
    BpfLoadMapDef def;
};

struct BpfCreateMapAttr {
    std::string name;
    bpf_map_type mapType;
    uint32_t mapFlags;
    uint32_t keySize;
    uint32_t valueSize;
    uint32_t maxEntries;
    uint32_t numaNode;
    uint32_t btfFd;
    uint32_t btfKeyTypeId;
    uint32_t btfValueTypeId;
    uint32_t mapIfindex;
    union {
        uint32_t innerMapFd;
        uint32_t btfVmlinuxValueTypeId;
    };
};

struct BpfLoadProgAttr {
    bpf_prog_type progType;
    bpf_attach_type expectedAttachType;
    const std::string name;
    const bpf_insn *insns;
    size_t insnsCnt;
    std::string license;
    union {
        uint32_t kernVersion;
        uint32_t attachProgFd;
    };
    union {
        uint32_t progIfindex;
        uint32_t attachBtfId;
    };
    uint32_t progBtfFd;
    uint32_t funcInfoRecSize;
    const unsigned long *funcInfo;
    uint32_t funcInfoCnt;
    uint32_t lineInfoRecSize;
    const unsigned long *lineInfo;
    uint32_t lineInfoCnt;
    uint32_t logLevel;
    uint32_t progFlags;
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
    return syscall(__NR_bpf, cmd, attr, size);
}

inline int32_t SysBpfObjPin(int32_t fd, const std::string &pathName)
{
    bpf_attr attr;

    (void)memset_s(&attr, sizeof(attr), '\0', sizeof(attr));
    attr.pathname = PtrToU64(pathName.c_str());
    attr.bpf_fd = fd;

    return SysBpf(BPF_OBJ_PIN, &attr, sizeof(attr));
}

inline int32_t SysBpfProgLoad(bpf_attr *attr, uint32_t size)
{
    int32_t fd;

    do {
        fd = SysBpf(BPF_PROG_LOAD, attr, size);
    } while (fd < 0 && errno == EAGAIN);

    return fd;
}

inline int32_t SysBpfObjAttach(bpf_attach_type type, const int prog_fd, const int cg_fd)
{
    bpf_attr attr;

    (void)memset_s(&attr, sizeof(attr), '\0', sizeof(attr));
    attr.target_fd = cg_fd;
    attr.attach_bpf_fd = prog_fd;
    attr.attach_type = type;

    return SysBpf(BPF_PROG_ATTACH, &attr, sizeof(attr));
}
} // namespace Bpf
} // namespace OHOS
#endif // BPF_UTILS_H
