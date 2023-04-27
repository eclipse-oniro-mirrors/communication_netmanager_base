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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
    }

private:
    static uint32_t BpfFdToU32(const int mapFd)
    {
        return 0;
    }

    static uint64_t BpfMapPathNameToU64(const std::string &pathName)
    {
        return 0;
    }

    static uint64_t BpfMapKeyToU64(const Key &key)
    {
        return 0;
    }

    static uint64_t BpfMapValueToU64(const Value &value)
    {
        return 0;
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

    int GetNextKeyFromStatsMap(const Key &curkey, Key &nextKey) const
    {
        return 0;
    }

    Value ReadValueFromMap(const Key key) const
    {
        return 0;
    }

    /**
     * Is has map fd
     *
     * @return bool true:has map fd false:not have
     */
    [[nodiscard]] bool IsValid() const
    {
        return 0;
    }

    /**
     * Read Value From Map
     *
     * @param key the key of map
     * @return Value value corresponding to key
     */
    [[nodiscard]] int Read(const Key &key, Value &val) const
    {
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
        return 0;
    }

    /**
     * Get all keys
     *
     * @return key of list
     */
    [[nodiscard]] std::vector<Key> GetAllKeys() const
    {
        return 0;
    }

    [[nodiscard]] int Clear(const std::vector<Key> &keys) const
    {
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
        return 0;
    }

private:
    int32_t mapFd_ = 0;
};
} // namespace OHOS::NetManagerStandard
#endif /* CONNECTIVITY_EXT_BPF_MAPPER_H */
