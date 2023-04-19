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

#ifndef BPF_UTILS_H
#define BPF_UTILS_H

#include <string>

#include <arpa/inet.h>
#include <linux/bpf.h>
#include <sys/syscall.h>

#include <cerrno>
#include <fcntl.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <unistd.h>

#include "securec.h"

#include "netnative_log_wrapper.h"

namespace OHOS {
namespace Bpf {
constexpr const int SYS_RET_ERR = -1;
constexpr const int SYS_RET_SUCCESS = 0;
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
    uint64_t value;
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
    return true;
}

inline int32_t SysBpf(bpf_cmd cmd, bpf_attr *attr, uint32_t size)
{
    return SYS_RET_SUCCESS;
}

inline int32_t SysBpfObjPin(int32_t fd, const std::string &pathName)
{
    return SYS_RET_SUCCESS;
}

inline int32_t SysBpfProgLoad(bpf_attr *attr, uint32_t size)
{
    return SYS_RET_SUCCESS;
}

inline int32_t SysBpfObjAttach(bpf_attach_type type, const int prog_fd, const int cg_fd)
{
    return SYS_RET_SUCCESS;
}
} // namespace Bpf
} // namespace OHOS
#endif // BPF_UTILS_H
