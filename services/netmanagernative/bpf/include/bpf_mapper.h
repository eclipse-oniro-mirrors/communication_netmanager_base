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

#ifndef CONNECTIVITY_EXT_BPF_MAPPER_H
#define CONNECTIVITY_EXT_BPF_MAPPER_H

#include <cerrno>
#include <linux/bpf.h>
#include <linux/unistd.h>
#include <cstdint>
#include <fcntl.h>
#include <sys/stat.h>
#include <string>
#include <unistd.h>
#include <functional>
#include <memory>
#include <linux/if_ether.h>
#include <atomic>
#include <vector>

#include "securec.h"
#include "netnative_log_wrapper.h"

namespace OHOS::NetManagerStandard {
template <class Key, class Value> class BpfMapperImplement {
public:
    BpfMapperImplement<Key, Value>() = default;

    static int GetFirstKey(const int mapFd, Key &key)
    {
        bpf_attr bpfAttr{};
        if (memset_s(&bpfAttr, sizeof(bpfAttr), 0, sizeof(bpfAttr)) != EOK) {
            return -1;
        }
        bpfAttr.map_fd = BpfFdToU32(mapFd);
        bpfAttr.key = 0;
        bpfAttr.next_key = BpfMapKeyToU64(key);
        return BpfSyscall(BPF_MAP_GET_NEXT_KEY, bpfAttr);
    }

    /**
     * Get the Next Key From Map
     *
     * @param mapFd map fd
     * @param key the key of Bpf Map
     * @param next_key the key of Bpf Map
     * @return int return next key
     */
    static int GetNextKey(const int mapFd, const Key &key, Key &nextKey)
    {
        bpf_attr bpfAttr{};
        if (memset_s(&bpfAttr, sizeof(bpfAttr), 0, sizeof(bpfAttr)) != EOK) {
            return -1;
        }
        bpfAttr.map_fd = BpfFdToU32(mapFd);
        bpfAttr.key = BpfMapKeyToU64(key);
        bpfAttr.next_key = BpfMapKeyToU64(nextKey);
        return BpfSyscall(BPF_MAP_GET_NEXT_KEY, bpfAttr);
    }

    /**
     * Bpf Syscall
     *
     * @param cmd which command need to execute
     * @param attr union consists of various anonymous structures
     * @return int return the result of executing the command
     */
    static int BpfSyscall(int cmd, const bpf_attr &attr)
    {
        return static_cast<int>(syscall(__NR_bpf, cmd, &attr, sizeof(attr)));
    }

    /**
     * Write Value To Bpf Map
     *
     * @param mapFd map fd
     * @param key the key of Bpf Map
     * @param value the value of Bpf Map
     * @param flags map flag
     * @return int true:write success false:failure
     */
    static int UpdateElem(const int mapFd, const Key &key, const Value &value, uint64_t flags)
    {
        bpf_attr bpfAttr{};
        if (memset_s(&bpfAttr, sizeof(bpfAttr), 0, sizeof(bpfAttr)) != EOK) {
            return -1;
        }
        bpfAttr.map_fd = BpfFdToU32(mapFd);
        bpfAttr.key = BpfMapKeyToU64(key);
        bpfAttr.value = BpfMapValueToU64(value);
        bpfAttr.flags = flags;
        return BpfSyscall(BPF_MAP_UPDATE_ELEM, bpfAttr);
    }

    /**
     * LookUp Elem From Map
     *
     * @param mapFd map fd
     * @param key the key of Bpf Map
     * @param value the value of Bpf Map
     * @return int true:find success false:failure
     */
    static int LookUpElem(const int mapFd, const Key &key, const Value &value)
    {
        bpf_attr bpfAttr{};
        if (memset_s(&bpfAttr, sizeof(bpfAttr), 0, sizeof(bpfAttr)) != EOK) {
            return -1;
        }
        bpfAttr.map_fd = BpfFdToU32(mapFd);
        bpfAttr.key = BpfMapKeyToU64(key);
        bpfAttr.value = BpfMapValueToU64(value);
        return BpfSyscall(BPF_MAP_LOOKUP_ELEM, bpfAttr);
    }

    /**
     * Delete Elem From Map
     *
     * @param mapFd map fd
     * @param key the key of Bpf Map
     * @return int true:delete success false:failure
     */
    static int DeleteElem(const int mapFd, const Key &key)
    {
        bpf_attr bpfAttr{};
        if (memset_s(&bpfAttr, sizeof(bpfAttr), 0, sizeof(bpfAttr)) != EOK) {
            return -1;
        }
        bpfAttr.map_fd = BpfFdToU32(mapFd);
        bpfAttr.key = BpfMapKeyToU64(key);
        return BpfSyscall(BPF_MAP_DELETE_ELEM, bpfAttr);
    }

    /**
     * Get Bpf Object By PathName
     *
     * @param pathName bpf map path
     * @param fileFlags file flags
     * @return int return map file descriptor
     */
    static int BpfObjGet(const std::string &pathName, uint32_t fileFlags)
    {
        bpf_attr bpfAttr{};
        if (memset_s(&bpfAttr, sizeof(bpfAttr), 0, sizeof(bpfAttr)) != EOK) {
            return -1;
        }
        bpfAttr.pathname = BpfMapPathNameToU64(pathName);
        bpfAttr.file_flags = fileFlags;
        return BpfSyscall(BPF_OBJ_GET, bpfAttr);
    }

    /**
     * Get the Map Fd
     *
     * @param pathName bpf map path
     * @param objFlags obj flags
     * @return int return map file descriptor
     */
    static int GetMap(const std::string &pathName, uint32_t objFlags)
    {
        return BpfObjGet(pathName, objFlags);
    }

private:
    static uint32_t BpfFdToU32(const int mapFd)
    {
        return static_cast<uint32_t>(mapFd);
    }

    static uint64_t BpfMapPathNameToU64(const std::string &pathName)
    {
        return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(pathName.c_str()));
    }

    static uint64_t BpfMapKeyToU64(const Key &key)
    {
        return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(&key));
    }

    static uint64_t BpfMapValueToU64(const Value &value)
    {
        return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(&value));
    }
};

template <class Key, class Value> class BpfMapper {
public:
    BpfMapper<Key, Value>() = default;
    ~BpfMapper<Key, Value>()
    {
        mapFd_ = -1;
    }
    BpfMapper<Key, Value>(const std::string &pathName, uint32_t flags)
    {
        int mapFd = BpfMapperImplement<Key, Value>::GetMap(pathName, flags);
        if (mapFd >= 0) {
            mapFd_ = mapFd;
        }
    }

    /**
     * Is has map fd
     *
     * @return bool true:has map fd false:not have
     */
    [[nodiscard]] bool IsValid() const
    {
        return mapFd_ >= 0;
    }

    /**
     * Read Value From Map
     *
     * @param key the key of map
     * @return Value value corresponding to key
     */
    [[nodiscard]] int Read(const Key &key, Value &val) const
    {
        Value value{};
        if (BpfMapperImplement<Key, Value>::LookUpElem(mapFd_, key, value) < 0) {
            return -1;
        }
        val = value;
        return 0;
    }

    /**
     * WriteValue
     *
     * @param key the key want to write
     * @param value the value want to write
     * @param flags map flag
     * @return int 0 if OK
     */
    [[nodiscard]] int Write(const Key &key, const Value &value, uint64_t flags) const
    {
        return BpfMapperImplement<Key, Value>::UpdateElem(mapFd_, key, value, flags);
    }

    /**
     * Get all keys
     *
     * @return key of list
     */
    [[nodiscard]] std::vector<Key> GetAllKeys() const
    {
        Key key{};
        if (BpfMapperImplement<Key, Value>::GetFirstKey(mapFd_, key) < 0) {
            return {};
        }
        std::vector<Key> keys;
        keys.emplace_back(key);

        Key nextKey{};
        while (BpfMapperImplement<Key, Value>::GetNextKey(mapFd_, key, nextKey) >= 0) {
            key = nextKey;
            keys.emplace_back(key);
        }
        return keys;
    }

    [[nodiscard]] int Clear(const std::vector<Key> &keys) const
    {
        for (const auto &k : keys) {
            if (Delete(k) < 0) {
                return -1;
            }
        }
        return 0;
    }

    /**
     * DeleteEntryFromMap
     *
     * @param deleteKey the key need to delete
     * @return int 0 if OK
     */
    [[nodiscard]] int Delete(const Key &deleteKey) const
    {
        return BpfMapperImplement<Key, Value>::DeleteElem(mapFd_, deleteKey);
    }

private:
    int32_t mapFd_ = 0;
};
} // namespace OHOS::NetManagerStandard
#endif /* CONNECTIVITY_EXT_BPF_MAPPER_H */
