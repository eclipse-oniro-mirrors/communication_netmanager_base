/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "bpf_prog_loader.h"

#include "bpf_map_creator.h"

namespace OHOS {
namespace Bpf {
namespace {
constexpr int32_t BPF_LOG_BUF_SIZE = UINT32_MAX >> 8;
constexpr const char *CGROUP_DIR = "/sys/fs/cgroup";
constexpr const char *CGROUP_SKB_UID_INGRESS_PROG_NAME = "cgroup_skb_uid_ingress";
constexpr const char *CGROUP_SKB_UID_EGRESS_PROG_NAME = "cgroup_skb_uid_egress";
char g_bpfLogbuf[BPF_LOG_BUF_SIZE] = {0};
} // namespace

bool BpfProgLoader::LoadAndAttach(const std::string &event, const bpf_insn *prog, int32_t size)
{
    bpf_prog_type progType;
    constexpr const char *socketName = "socket";
    constexpr const char *cgroupSkbName = "cgroup_skb";
    if (!event.compare(0, strlen(socketName), socketName)) {
        progType = BPF_PROG_TYPE_SOCKET_FILTER;
    } else if (!event.compare(0, strlen(cgroupSkbName), cgroupSkbName)) {
        progType = BPF_PROG_TYPE_CGROUP_SKB;
    } else {
        NETNATIVE_LOGE("Unknown event %{public}s", event.c_str());
        return false;
    }

    std::string license();
    int32_t kernVer = 0;
    size_t insnsCnt = size / sizeof(bpf_insn);
    int32_t progFd = BpfLoadProg(progType, prog, insnsCnt, license, kernVer);
    if (progFd < 0) {
        NETNATIVE_LOGE("Failed to load bpf prog, error: %{public}d, errmsg: %{public}s, g_bpfLogbuf: %{public}s", errno,
                       strerror(errno), g_bpfLogbuf);
        return false;
    }
    bool ret = AttachProg(progFd, event);
    close(progFd);
    return ret;
}

bool BpfProgLoader::AttachProg(int32_t progFd, const std::string &event) const
{
    std::string progName = event;
    replace(progName.begin(), progName.end(), '/', '_');
    std::string progPinLocation = std::string("/sys/fs/bpf/prog_netsys_") + progName;
    if (access(progPinLocation.c_str(), F_OK) == 0) {
        NETNATIVE_LOGI("prog: %{public}s has already been pinned", progPinLocation.c_str());
    } else if (SysBpfObjPin(progFd, progPinLocation)) {
        NETNATIVE_LOGE("Failed to pin prog: %{public}s, errno = %{public}d", progPinLocation.c_str(), errno);
        return false;
    }
    char realPath[PATH_MAX] = {};
    if (realpath(CGROUP_DIR, realPath) == nullptr) {
        NETNATIVE_LOGE("The error path is %{public}s", CGROUP_DIR);
        return false;
    }
    int cgroupFd = open(CGROUP_DIR, O_DIRECTORY | O_RDONLY | O_CLOEXEC);
    if (cgroupFd < 0) {
        NETNATIVE_LOGE("open /sys/fs/cgroup failed, errno = %{public}d", errno);
        return false;
    }

    if (CGROUP_SKB_UID_INGRESS_PROG_NAME == progName) {
        if (SysBpfObjAttach(BPF_CGROUP_INET_INGRESS, progFd, cgroupFd) < SYS_RET_SUCCESS) {
            NETNATIVE_LOGE("Attach %{pulbic}s failed, errno: %{public}d", progName.c_str(), errno);
            close(cgroupFd);
            return false;
        }
    }
    if (CGROUP_SKB_UID_EGRESS_PROG_NAME == progName) {
        if (SysBpfObjAttach(BPF_CGROUP_INET_EGRESS, progFd, cgroupFd) < SYS_RET_SUCCESS) {
            NETNATIVE_LOGE("Attach %{pulbic}s failed, errno: %{public}d", progName.c_str(), errno);
            close(cgroupFd);
            return false;
        }
    }
    close(cgroupFd);
    return true;
}

int32_t BpfProgLoader::BpfLoadProg(bpf_prog_type type, const bpf_insn *insns, size_t insnsCnt, std::string &license,
                                   uint32_t kernVersion) const
{
    BpfLoadProgAttr loadAttr;

    errno_t result = memset_s(&loadAttr, sizeof(BpfLoadProgAttr), '\0', sizeof(BpfLoadProgAttr));
    if (result != EOK) {
        NETNATIVE_LOGE("Call memset_s failed, errno: %{public}d", errno);
        return SYS_RET_ERR;
    }

    loadAttr.progType = type;
    loadAttr.expectedAttachType = BPF_CGROUP_INET_INGRESS;
    loadAttr.insns = insns;
    loadAttr.insnsCnt = insnsCnt;
    loadAttr.license = license;
    loadAttr.kernVersion = kernVersion;
    return BpfLoadProgXattr(&loadAttr);
}

void BpfProgLoader::InitProgAttr(const BpfLoadProgAttr *loadAttr, bpf_attr &attr) const
{
    errno_t result = memset_s(&attr, sizeof(attr), 0, sizeof(attr));
    if (result != EOK) {
        NETNATIVE_LOGE("Call memset_s failed, errno: %{public}d", errno);
        return;
    }

    attr.prog_type = loadAttr->progType;
    attr.expected_attach_type = loadAttr->expectedAttachType;
    attr.prog_ifindex = loadAttr->progIfindex;
    attr.kern_version = loadAttr->kernVersion;
    attr.insn_cnt = static_cast<uint32_t>(loadAttr->insnsCnt);
    attr.insns = PtrToU64(loadAttr->insns);
    attr.license = PtrToU64(loadAttr->license.c_str());
    attr.log_level = loadAttr->logLevel;
    if (loadAttr->logLevel) {
        attr.log_buf = PtrToU64(g_bpfLogbuf);
        attr.log_size = BPF_LOG_BUF_SIZE;
    } else {
        attr.log_buf = PtrToU64(nullptr);
        attr.log_size = 0;
    }

    attr.prog_btf_fd = loadAttr->progBtfFd;
    attr.func_info_rec_size = loadAttr->funcInfoRecSize;
    attr.func_info_cnt = loadAttr->funcInfoCnt;
    attr.func_info = PtrToU64(loadAttr->funcInfo);
    attr.line_info_rec_size = loadAttr->lineInfoRecSize;
    attr.line_info_cnt = loadAttr->lineInfoCnt;
    attr.line_info = PtrToU64(loadAttr->lineInfo);

    if (!loadAttr->name.empty()) {
        loadAttr->name.copy(attr.prog_name, (loadAttr->name.size() < (BPF_OBJ_NAME_LEN - 1) ? loadAttr->name.size()
                                             : (BPF_OBJ_NAME_LEN - 1)));
    }
    attr.prog_flags = loadAttr->progFlags;
}

int32_t BpfProgLoader::BpfLoadProgXattr(const BpfLoadProgAttr *loadAttr) const
{
    bpf_attr attr;

    if (!loadAttr) {
        return -EINVAL;
    }

    if (loadAttr->logLevel) {
        return -EINVAL;
    }
    InitProgAttr(loadAttr, attr);
    int32_t fd = SysBpfProgLoad(&attr, sizeof(attr));
    if (fd >= SYS_RET_SUCCESS) {
        return fd;
    }
    attr.log_buf = PtrToU64(g_bpfLogbuf);
    attr.log_size = BPF_LOG_BUF_SIZE;
    attr.log_level = 1;
    g_bpfLogbuf[0] = 0;
    fd = SysBpfProgLoad(&attr, sizeof(attr));
    return fd;
}
} // namespace Bpf
} // namespace OHOS
