/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "netsys_client.h"

#include <errno.h>
#include <sys/socket.h>

#include "app_net_client.h"
#include "dns_config_client.h"
#include "hilog/log_c.h"
#include <netdb.h>
#include <securec.h>
#include <stdbool.h>
#include <sys/select.h>
#include <sys/un.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

#undef LOG_TAG
#ifndef NETMGRNATIVE_LOG_TAG
#define LOG_TAG "NetsysNativeService"
#else
#define LOG_TAG NETMGRNATIVE_LOG_TAG
#endif

#ifdef __cplusplus
extern "C" {
#endif

static volatile uint8_t g_allowInternet = 1;
int64_t lastDnsQueryPollSendTime = 0;
static uint32_t g_curDnsStoreSize = 0;
static struct DnsCacheInfo g_dnsCaches[MAX_DNS_CACHE_SIZE];
static int64_t g_dnsReportTime[TOTAL_FAIL_CAUSE_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};
static int64_t g_lastDnsErrorReportTime = 0;

pthread_spinlock_t g_dnsReportLock;
pthread_spinlock_t g_dnsReportTimeLock;

void DisallowInternet(void)
{
    g_allowInternet = 0;
}

uint8_t IsAllowInternet(void)
{
    return g_allowInternet;
}

static inline uint32_t Min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

static inline int CloseSocketReturn(int sock, int ret)
{
    close(sock);
    return ret;
}

void MakeDefaultDnsServer(char *server, size_t length)
{
    int ret = memset_s(server, length, 0, DEFAULT_SERVER_LENTH);
    if (ret < 0) {
        DNS_CONFIG_PRINT("MakeDefaultDnsServer memset_s failed");
        return;
    }

    ret = sprintf_s(server, length, "%d.%d.%d.%d", DEFAULT_SERVER_NAME, DEFAULT_SERVER_NAME, DEFAULT_SERVER_NAME,
                    DEFAULT_SERVER_NAME);
    if (ret != 0) {
        DNS_CONFIG_PRINT("MakeDefaultDnsServer sprintf_s failed");
    }
}

static bool NonBlockConnect(int sock, struct sockaddr *addr, socklen_t addrLen)
{
    int ret = connect(sock, addr, addrLen);
    if (ret >= 0) {
        return true;
    }
    if (errno != EINPROGRESS) {
        return false;
    }

    fd_set set = {0};
    FD_ZERO(&set);
    FD_SET(sock, &set);
    struct timeval timeout = {
        .tv_sec = DEFAULT_CONNECT_TIMEOUT,
        .tv_usec = 0,
    };

    ret = select(sock + 1, NULL, &set, NULL, &timeout);
    if (ret < 0) {
        DNS_CONFIG_PRINT("select error: %s", strerror(errno));
        return false;
    } else if (ret == 0) {
        DNS_CONFIG_PRINT("timeout!");
        return false;
    }

    int err = 0;
    socklen_t optLen = sizeof(err);
    ret = getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *)(&err), &optLen);
    if (ret < 0 || err != 0) {
        return false;
    }
    return true;
}

static int CreateConnectionToNetSys(void)
{
    int32_t sockFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockFd < 0) {
        DNS_CONFIG_PRINT("socket failed %d", errno);
        return -errno;
    }
    if (!MakeNonBlock(sockFd)) {
        DNS_CONFIG_PRINT("MakeNonBlock failed");
        return CloseSocketReturn(sockFd, -errno);
    }

    struct sockaddr_un address = {0};
    address.sun_family = AF_UNIX;

    if (strcpy_s(address.sun_path, sizeof(address.sun_path), DNS_SOCKET_PATH) != 0) {
        DNS_CONFIG_PRINT("str copy failed ");
        return CloseSocketReturn(sockFd, -1);
    }

    if (!NonBlockConnect(sockFd, (struct sockaddr *)&address, sizeof(address))) {
        return CloseSocketReturn(sockFd, -errno);
    }

    return sockFd;
}

static bool MakeKey(const char *hostName, const char *serv, const struct addrinfo *hints,
                    char key[static MAX_KEY_LENGTH])
{
    if (serv && hints) {
        return sprintf_s(key, MAX_KEY_LENGTH, "%s %s %d %d %d %d", hostName, serv, hints->ai_family, hints->ai_flags,
                         hints->ai_protocol, hints->ai_socktype) > 0;
    }

    if (hints) {
        return sprintf_s(key, MAX_KEY_LENGTH, "%s %d %d %d %d", hostName, hints->ai_family, hints->ai_flags,
                         hints->ai_protocol, hints->ai_socktype) > 0;
    }

    if (serv) {
        return sprintf_s(key, MAX_KEY_LENGTH, "%s %s", hostName, serv) > 0;
    }

    return sprintf_s(key, MAX_KEY_LENGTH, "%s", hostName) > 0;
}

static int32_t NetSysGetResolvConfInternal(int sockFd, uint16_t netId, struct ResolvConfig *config)
{
    struct RequestInfo info = {
        .uid = getuid(),
        .command = GET_CONFIG,
        .netId = netId,
    };
    if (netId == 0 && GetNetForApp() > 0) {
        info.netId = (uint32_t)GetNetForApp();
    }
    DNS_CONFIG_PRINT("NetSysGetResolvConfInternal begin netid: %d", info.netId);
    if (!PollSendData(sockFd, (const char *)(&info), sizeof(info))) {
        HILOG_ERROR(LOG_CORE, "send failed %{public}d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollRecvData(sockFd, (char *)(config), sizeof(struct ResolvConfig))) {
        HILOG_ERROR(LOG_CORE, "receive failed %{public}d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (config->error < 0) {
        HILOG_ERROR(LOG_CORE, "get Config error: %{public}d", config->error);
        return CloseSocketReturn(sockFd, config->error);
    }

    DNS_CONFIG_PRINT("NetSysGetResolvConfInternal end netid: %d", info.netId);
    return CloseSocketReturn(sockFd, 0);
}

int32_t NetSysGetResolvConf(uint16_t netId, struct ResolvConfig *config)
{
    if (config == NULL) {
        DNS_CONFIG_PRINT("Invalid Param");
        return -EINVAL;
    }

    int sockFd = CreateConnectionToNetSys();
    if (sockFd < 0) {
        DNS_CONFIG_PRINT("NetSysGetResolvConf CreateConnectionToNetSys connect to netsys err: %d", errno);
        return -errno;
    }

    int32_t err = NetSysGetResolvConfInternal(sockFd, netId, config);
    if (err < 0) {
        DNS_CONFIG_PRINT("NetSysGetResolvConf NetSysGetResolvConfInternal err: %d", errno);
        return err;
    }

    if (strlen(config->nameservers[0]) == 0) {
        return -1;
    }
    return 0;
}

static int32_t NetsysSendKeyForCache(int sockFd, struct ParamWrapper param, struct RequestInfo info)
{
    char key[MAX_KEY_LENGTH] = {0};
    if (!MakeKey(param.host, param.serv, param.hint, key)) {
        return CloseSocketReturn(sockFd, -1);
    }

    DNS_CONFIG_PRINT("NetSysSetResolvCacheInternal begin netid: %d", info.netId);
    if (!PollSendData(sockFd, (const char *)(&info), sizeof(info))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    uint32_t nameLen = strlen(key) + 1;
    if (!PollSendData(sockFd, (const char *)&nameLen, sizeof(nameLen))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollSendData(sockFd, key, nameLen)) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }
    return 0;
};

static bool IsAbnormalAddress(uint32_t addr)
{
    uint32_t address1 = 0; // 0.0.0.0
    uint32_t address2 = 2130706433; // 127.0.0.1
    if (addr == address1 || addr == address2) {
        return true;
    }
    return false;
}

static int32_t NetSysGetResolvCacheInternal(int sockFd, uint16_t netId, const struct ParamWrapper param,
                                            struct AddrInfo addrInfo[static MAX_RESULTS], uint32_t *num)
{
    struct RequestInfo info = {
        .uid = getuid(),
        .command = GET_CACHE,
        .netId = netId,
    };
    if (netId == 0 && GetNetForApp() > 0) {
        info.netId = (uint32_t)GetNetForApp();
    }
    int32_t res = NetsysSendKeyForCache(sockFd, param, info);
    if (res < 0) {
        return res;
    }

    if (!PollRecvData(sockFd, (char *)num, sizeof(uint32_t))) {
        DNS_CONFIG_PRINT("read failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    *num = Min(*num, MAX_RESULTS);
    if (*num == 0) {
        return CloseSocketReturn(sockFd, 0);
    }

    if (!PollRecvData(sockFd, (char *)addrInfo, sizeof(struct AddrInfo) * (*num))) {
        DNS_CONFIG_PRINT("read failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    uint32_t validNum = 0;
    for (uint32_t resNum = 0; resNum < *num; resNum++) {
        if (addrInfo[resNum].aiFamily == AF_INET) {
            uint32_t addr = addrInfo[resNum].aiAddr.sin.sin_addr.s_addr;
            if (IsAbnormalAddress(addr)) {
                HILOG_ERROR(LOG_CORE,
                    "GetResolvCache get abnormal zero[%{public}d] netId[%{public}u]", (addr == 0), netId);
            }
            if (addr == 0) {
                continue;
            }
        }
        addrInfo[validNum] = addrInfo[resNum];
        validNum++;
    }
    *num = validNum;

    DNS_CONFIG_PRINT("NetSysGetResolvCacheInternal end netid: %d", info.netId);
    return CloseSocketReturn(sockFd, 0);
}

int32_t NetSysGetResolvCache(uint16_t netId, const struct ParamWrapper param,
                             struct AddrInfo addrInfo[static MAX_RESULTS], uint32_t *num)
{
    char *hostName = param.host;
    if (hostName == NULL || strlen(hostName) == 0 || num == NULL) {
        DNS_CONFIG_PRINT("Invalid Param");
        return -EINVAL;
    }

    int sockFd = CreateConnectionToNetSys();
    if (sockFd < 0) {
        DNS_CONFIG_PRINT("NetSysGetResolvCache CreateConnectionToNetSys connect to netsys err: %d", errno);
        return sockFd;
    }

    int err = NetSysGetResolvCacheInternal(sockFd, netId, param, addrInfo, num);
    if (err < 0) {
        DNS_CONFIG_PRINT("NetSysGetResolvCache NetSysGetResolvCacheInternal err: %d", errno);
        return err;
    }

    return 0;
}

static int32_t FillAddrInfo(struct AddrInfo addrInfo[static MAX_RESULTS], struct addrinfo *res)
{
    if (memset_s(addrInfo, sizeof(struct AddrInfo) * MAX_RESULTS, 0, sizeof(struct AddrInfo) * MAX_RESULTS) != 0) {
        return -1;
    }

    int32_t resNum = 0;
    for (struct addrinfo *tmp = res; tmp != NULL; tmp = tmp->ai_next) {
        addrInfo[resNum].aiFlags = tmp->ai_flags;
        addrInfo[resNum].aiFamily = tmp->ai_family;
        addrInfo[resNum].aiSockType = (uint32_t)(tmp->ai_socktype);
        addrInfo[resNum].aiProtocol = tmp->ai_protocol;
        addrInfo[resNum].aiAddrLen = tmp->ai_addrlen;
        if (memcpy_s(&addrInfo[resNum].aiAddr, sizeof(addrInfo[resNum].aiAddr), tmp->ai_addr, tmp->ai_addrlen) != 0) {
            DNS_CONFIG_PRINT("memcpy_s failed");
            return -1;
        }
        if (tmp->ai_canonname &&
            strcpy_s(addrInfo[resNum].aiCanonName, sizeof(addrInfo[resNum].aiCanonName), tmp->ai_canonname) != 0) {
            DNS_CONFIG_PRINT("strcpy_s failed");
            return -1;
        }
        if (addrInfo[resNum].aiFamily == AF_INET) {
            uint32_t addr = addrInfo[resNum].aiAddr.sin.sin_addr.s_addr;
            if (IsAbnormalAddress(addr)) {
                HILOG_ERROR(LOG_CORE, "SetDnsCache set abnormal zero[%{public}d]", (addr == 0));
            }
        }

        ++resNum;
        if (resNum >= MAX_RESULTS) {
            break;
        }
    }

    return resNum;
}

static int32_t FillQueryParam(struct queryparam *orig, struct QueryParam *dest)
{
    dest->type = orig->qp_type;
    dest->netId = orig->qp_netid;
    dest->mark = orig->qp_mark;
    dest->flags = orig->qp_flag;
    dest->qHook = NULL;
    return 0;
}

static int32_t NetSysSetResolvCacheInternal(int sockFd, uint16_t netId, const struct ParamWrapper param,
                                            struct addrinfo *res)
{
    struct RequestInfo info = {
        .uid = getuid(),
        .command = SET_CACHE,
        .netId = netId,
    };
    if (netId == 0 && GetNetForApp() > 0) {
        info.netId = (uint32_t)GetNetForApp();
    }
    int32_t result = NetsysSendKeyForCache(sockFd, param, info);
    if (result < 0) {
        return result;
    }

    struct AddrInfo addrInfo[MAX_RESULTS] = {};
    int32_t resNum = FillAddrInfo(addrInfo, res);
    if (resNum < 0) {
        return CloseSocketReturn(sockFd, -1);
    }

    if (!PollSendData(sockFd, (char *)&resNum, sizeof(resNum))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (resNum == 0) {
        return CloseSocketReturn(sockFd, 0);
    }

    if (!PollSendData(sockFd, (char *)addrInfo, sizeof(struct AddrInfo) * resNum)) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    return CloseSocketReturn(sockFd, 0);
}

int32_t NetSysSetResolvCache(uint16_t netId, const struct ParamWrapper param, struct addrinfo *res)
{
    char *hostName = param.host;
    if (hostName == NULL || strlen(hostName) == 0 || res == NULL) {
        DNS_CONFIG_PRINT("Invalid Param");
        return -EINVAL;
    }

    int sockFd = CreateConnectionToNetSys();
    if (sockFd < 0) {
        DNS_CONFIG_PRINT("NetSysSetResolvCache CreateConnectionToNetSys connect to netsys err: %d", errno);
        return sockFd;
    }

    int err = NetSysSetResolvCacheInternal(sockFd, netId, param, res);
    if (err < 0) {
        DNS_CONFIG_PRINT("NetSysSetResolvCache NetSysSetResolvCacheInternal err: %d", errno);
        return err;
    }

    return 0;
}

static int32_t NetSysIsIpv6EnableInternal(int sockFd, uint16_t netId, int *enable)
{
    struct RequestInfo info = {
        .uid = getuid(),
        .command = JUDGE_IPV6,
        .netId = netId,
    };
    if (!PollSendData(sockFd, (const char *)(&info), sizeof(info))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollRecvData(sockFd, (char *)enable, sizeof(int))) {
        DNS_CONFIG_PRINT("read failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    return CloseSocketReturn(sockFd, 0);
}

int NetSysIsIpv6Enable(uint16_t netId)
{
    int sockFd = CreateConnectionToNetSys();
    if (sockFd < 0) {
        DNS_CONFIG_PRINT("NetSysIsIpv6Enable CreateConnectionToNetSys connect to netsys err: %d", errno);
        return sockFd;
    }
    int enable = 0;
    int err = NetSysIsIpv6EnableInternal(sockFd, netId, &enable);
    if (err < 0) {
        return 0;
    }

    return enable;
}

static int32_t NetSysPostDnsResultPollSendData(int sockFd, int queryret, int32_t resNum, struct QueryParam *param,
                                               struct AddrInfo addrInfo[static MAX_RESULTS])
{
    if (!PollSendData(sockFd, (char *)&queryret, sizeof(int))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollSendData(sockFd, (char *)&resNum, sizeof(int32_t))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollSendData(sockFd, (char *)param, sizeof(struct QueryParam))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (resNum > 0) {
        if (!PollSendData(sockFd, (char *)addrInfo, sizeof(struct AddrInfo) * resNum)) {
            DNS_CONFIG_PRINT("send failed %d", errno);
            return CloseSocketReturn(sockFd, -errno);
        }
    }
    return CloseSocketReturn(sockFd, 0);
}

static int32_t NetSysPostDnsResultInternal(int sockFd, uint16_t netId, char* name, int usedtime, int queryret,
                                           struct addrinfo *res, struct queryparam *param)
{
    struct RequestInfo info = {
        .uid = getuid(),
        .command = POST_DNS_RESULT,
        .netId = netId,
    };

    int32_t uid = (int32_t)(getuid());
    int32_t pid = getpid();
    uint32_t nameLen = strlen(name) + 1;
    NETSYS_CLIENT_PRINT("NetSysPostDnsResultInternal uid %d, pid %d, netid %d pkg", uid, pid, netId);

    struct AddrInfo addrInfo[MAX_RESULTS] = {};
    struct QueryParam netparam = {};
    int32_t resNum = 0;
    if (queryret == 0) {
        resNum = FillAddrInfo(addrInfo, res);
    }
    if (resNum < 0) {
        return CloseSocketReturn(sockFd, -1);
    }
    FillQueryParam(param, &netparam);

    if (!PollSendData(sockFd, (const char *)(&info), sizeof(info))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollSendData(sockFd, (char *)&uid, sizeof(int32_t))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollSendData(sockFd, (char *)&pid, sizeof(int32_t))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollSendData(sockFd, (char *)&nameLen, sizeof(uint32_t))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollSendData(sockFd, name, (sizeof(char) * nameLen))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollSendData(sockFd, (char *)&usedtime, sizeof(int))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    return NetSysPostDnsResultPollSendData(sockFd, queryret, resNum, &netparam, addrInfo);
}

int32_t NetSysPostDnsResult(int netid, char* name, int usedtime, int queryret,
                            struct addrinfo *res, struct queryparam *param)
{
    if (name == NULL) {
        return -1;
    }

    int sockFd = CreateConnectionToNetSys();
    if (sockFd < 0) {
        DNS_CONFIG_PRINT("NetSysPostDnsResult CreateConnectionToNetSys connect to netsys err: %d", errno);
        return sockFd;
    }
    int err = NetSysPostDnsResultInternal(sockFd, netid, name, usedtime, queryret, res, param);
    if (err < 0) {
        return -1;
    }

    return 0;
}

static int32_t NetSysGetDefaultNetworkInternal(int sockFd, uint16_t netId, int32_t *currentNetId)
{
    struct RequestInfo info = {
        .uid = getuid(),
        .command = GET_DEFAULT_NETWORK,
        .netId = netId,
    };
    if (!PollSendData(sockFd, (const char *)(&info), sizeof(info))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollRecvData(sockFd, (char *)currentNetId, sizeof(int))) {
        DNS_CONFIG_PRINT("read failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }
    DNS_CONFIG_PRINT("currentNetId %d", *currentNetId);
    return CloseSocketReturn(sockFd, 0);
}

int32_t NetSysGetDefaultNetwork(uint16_t netId, int32_t* currentNetId)
{
    int sockFd = CreateConnectionToNetSys();
    int err = NetSysGetDefaultNetworkInternal(sockFd, netId, currentNetId);
    if (err < 0) {
        return -1;
    }

    return 0;
}

static int32_t NetSysBindSocketInternal(int sockFd, uint16_t netId, int32_t fd)
{
    struct RequestInfo info = {
        .uid = getuid(),
        .command = BIND_SOCKET,
        .netId = netId,
    };
    if (!PollSendData(sockFd, (const char *)(&info), sizeof(info))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollSendData(sockFd, (const char *)(&fd), sizeof(int32_t))) {
        DNS_CONFIG_PRINT("send failed %d", errno);
        return CloseSocketReturn(sockFd, -errno);
    }

    return CloseSocketReturn(sockFd, 0);
}

int32_t NetSysBindSocket(int32_t fd, uint32_t netId)
{
    int sockFd = CreateConnectionToNetSys();
    DNS_CONFIG_PRINT("NetSysBindSocket %d", fd);
    int err = NetSysBindSocketInternal(sockFd, netId, fd);
    if (err < 0) {
        return -1;
    }

    return 0;
}

static void FillFamilyQueryInfo(struct FamilyQueryInfoExt *extInfo, struct FamilyQueryInfo *info)
{
    extInfo->retCode = info->retCode;
    extInfo->isNoAnswer = info->isNoAnswer;
    extInfo->cname = info->cname;
    if (info->serverAddr && memcpy_s(extInfo->serverAddr, sizeof(extInfo->serverAddr),
        info->serverAddr, strlen(info->serverAddr) + 1) != 0) {
        HILOG_ERROR(LOG_CORE, "copy server error");
    }
}

static void FillDnsProcessInfo(char *srcAddr, struct DnsProcessInfo *processInfo,
    struct DnsProcessInfoExt *processInfoExt)
{
    processInfoExt->queryTime = processInfo->queryTime;
    processInfoExt->retCode = processInfo->retCode;
    processInfoExt->firstQueryEnd2AppDuration = processInfo->firstQueryEnd2AppDuration;
    processInfoExt->firstQueryEndDuration = processInfo->firstQueryEndDuration;
    processInfoExt->firstReturnType = processInfo->firstReturnType;
    processInfoExt->isFromCache = processInfo->isFromCache;
    processInfoExt->sourceFrom = processInfo->sourceFrom;
    if (memcpy_s(processInfoExt->hostname, sizeof(processInfoExt->hostname),
        processInfo->hostname, strlen(processInfo->hostname) + 1) != 0) {
        HILOG_ERROR(LOG_CORE, "copy hostname error");
    }
    if (srcAddr) {
        if (memcpy_s(processInfoExt->srcAddr, sizeof(processInfoExt->srcAddr),
            srcAddr, strlen(srcAddr) + 1) != 0) {
            HILOG_ERROR(LOG_CORE, "copy srcAddr error");
        }
    }
    FillFamilyQueryInfo(&(processInfoExt->ipv4QueryInfo), &(processInfo->ipv4QueryInfo));
    FillFamilyQueryInfo(&(processInfoExt->ipv6QueryInfo), &(processInfo->ipv6QueryInfo));
}

static int32_t GetDnsCacheSize(void)
{
    uint32_t size = 0;
    for (uint32_t i = 0; i < g_curDnsStoreSize; i++) {
        uint8_t addrSize = g_dnsCaches[i].addrSize;
        size += (sizeof(uint8_t) + sizeof(struct DnsProcessInfoExt) + addrSize * sizeof(struct AddrInfo));
    }
    return size;
}

static int32_t NetSysPostDnsQueryForOne(int sockFd, struct DnsCacheInfo dnsInfo)
{
    uint8_t addrSize = dnsInfo.addrSize;
    if (!PollSendData(sockFd, (char *)&addrSize, sizeof(uint8_t))) {
        return -errno;
    }

    if (!PollSendData(sockFd, (char *)&dnsInfo.dnsProcessInfo, sizeof(struct DnsProcessInfoExt))) {
        return -errno;
    }

    if (addrSize > 0) {
        if (!PollSendData(sockFd, (char *)dnsInfo.addrInfo, sizeof(struct AddrInfo) * addrSize)) {
            return -errno;
        }
    }
    return 0;
}

static int32_t NetSysPostDnsQueryResultInternal(void)
{
    int sockFd = CreateConnectionToNetSys();
    if (sockFd < 0) {
        return sockFd;
    }
    int32_t uid = (int32_t)(getuid());
    int32_t pid = getpid();
    struct RequestInfo info = {
        .uid = uid,
        .command = POST_DNS_QUERY_RESULT,
        .netId = 0,
    };
    uint32_t allDnsCacheSize = GetDnsCacheSize();
    if (!PollSendData(sockFd, (const char *)(&info), sizeof(info))) {
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollSendData(sockFd, (char *)&uid, sizeof(int32_t))) {
        return CloseSocketReturn(sockFd, -errno);
    }

    if (!PollSendData(sockFd, (char *)&pid, sizeof(int32_t))) {
        return CloseSocketReturn(sockFd, -errno);
    }
    if (!PollSendData(sockFd, (char *)&g_curDnsStoreSize, sizeof(int32_t))) {
        return CloseSocketReturn(sockFd, -errno);
    }
    if (!PollSendData(sockFd, (char *)&allDnsCacheSize, sizeof(int32_t))) {
        return CloseSocketReturn(sockFd, -errno);
    }
    for (uint32_t i = 0; i < g_curDnsStoreSize; i++) {
        int32_t ret = NetSysPostDnsQueryForOne(sockFd, g_dnsCaches[i]);
        if (ret < 0) {
            return CloseSocketReturn(sockFd, ret);
        }
    }
    CloseSocketReturn(sockFd, 0);
    pthread_spin_lock(&g_dnsReportLock);
    memset_s(&g_dnsCaches, sizeof(struct DnsCacheInfo) * MAX_DNS_CACHE_SIZE, 0,
        sizeof(struct DnsCacheInfo) * MAX_DNS_CACHE_SIZE);
    g_curDnsStoreSize = 0;
    pthread_spin_unlock(&g_dnsReportLock);
    return 0;
}

char *addr_to_string(const AlignedSockAddr *addr, char *buf, size_t len)
{
    switch (addr->sa.sa_family) {
        case AF_INET:
            if (inet_ntop(AF_INET, &addr->sin.sin_addr, buf, len) == NULL) {
                return NULL;
            }
            break;
        case AF_INET6:
            if (inet_ntop(AF_INET6, &addr->sin6.sin6_addr, buf, len) == NULL) {
                return NULL;
            }
            break;
        default:
            return NULL;
    }
    return buf;
}

bool IsSystemUid(void)
{
    int32_t uid = (int32_t)(getuid());
    if (uid == UID_PUSH || uid == UID_ACCOUNT) {
        return false;
    }
    return uid < MIN_APP_UID;
}

bool IsLoopbackAddr(struct AddrInfo addrInfo[static MAX_RESULTS], int32_t addrSize)
{
    if (addrSize == 0) {
        return false;
    }
    struct AddrInfo firstAddr = addrInfo[0];
    char addrBuf[INET6_ADDRSTRLEN];
    if (addr_to_string(&firstAddr.aiAddr, addrBuf, sizeof(addrBuf)) == NULL) {
        return false;
    }
    if (!strcmp(addrBuf, LOOP_BACK_ADDR1) || !strcmp(addrBuf, LOOP_BACK_ADDR2)) {
        return true;
    }
    return false;
}

bool IsAllCname(struct DnsProcessInfoExt *dnsProcessInfo)
{
    if (dnsProcessInfo->isFromCache) {
        return false;
    }
    return dnsProcessInfo->ipv4QueryInfo.cname && dnsProcessInfo->ipv6QueryInfo.cname;
}

bool IsAllNoAnswer(struct DnsProcessInfoExt *dnsProcessInfo)
{
    if (dnsProcessInfo->isFromCache || dnsProcessInfo->retCode != 0) {
        return false;
    }
    return dnsProcessInfo->ipv4QueryInfo.isNoAnswer && dnsProcessInfo->ipv6QueryInfo.isNoAnswer;
}

bool IsFailCauseAllowedReport(int failcause)
{
    if (failcause <= FAIL_CAUSE_NONE) {
        return false;
    }
    int index = failcause - 1;
    int64_t now = (int64_t)(time(NULL));
    return now - g_dnsReportTime[index] > FAIL_CAUSE_REPORT_INTERVAL;
}

int32_t GetQueryFailCause(struct DnsProcessInfoExt *dnsProcessInfo,
    struct AddrInfo addrInfo[static MAX_RESULTS], int32_t addrSize)
{
    if (dnsProcessInfo == NULL) {
        return FAIL_CAUSE_NONE;
    }
    if (dnsProcessInfo->retCode != 0
        && IsFailCauseAllowedReport(FAIL_CAUSE_QUERY_FAIL)) {
        return FAIL_CAUSE_QUERY_FAIL;
    }
    if (dnsProcessInfo->firstQueryEndDuration > QUERY_CALLBACK_RETURN_SLOW_THRESHOLD
        && IsFailCauseAllowedReport(FAIL_CAUSE_FIRST_RETURN_SLOW)) {
        return FAIL_CAUSE_FIRST_RETURN_SLOW;
    }
    if (dnsProcessInfo->firstQueryEnd2AppDuration > FIRST_RETURN_SLOW_THRESHOLD
        && IsFailCauseAllowedReport(FAIL_CAUSE_CALLBACK_RETURN_SLOW)) {
        return FAIL_CAUSE_CALLBACK_RETURN_SLOW;
    }
    if (IsLoopbackAddr(addrInfo, addrSize)
        && IsFailCauseAllowedReport(FAIL_CAUSE_RETURN_LOOPBACK_ADDR)) {
        return FAIL_CAUSE_RETURN_LOOPBACK_ADDR;
    }
    if (IsAllCname(dnsProcessInfo)
        && IsFailCauseAllowedReport(FAIL_CAUSE_RETURN_CNAME)) {
        return FAIL_CAUSE_RETURN_CNAME;
    }
    if (IsAllNoAnswer(dnsProcessInfo)
        && IsFailCauseAllowedReport(FAIL_CAUSE_RETURN_NO_ANSWER)) {
        return FAIL_CAUSE_RETURN_NO_ANSWER;
    }
    return FAIL_CAUSE_NONE;
}

int32_t NetsysPostDnsAbnormal(int32_t failcause, struct DnsCacheInfo dnsInfo)
{
    int sockFd = CreateConnectionToNetSys();
    if (sockFd < 0) {
        return sockFd;
    }
    int32_t uid = (int32_t)(getuid());
    int32_t pid = getpid();
    struct RequestInfo info = {
        .uid = uid,
        .command = POST_DNS_ABNORMAL_RESULT,
        .netId = 0,
    };
    if (!PollSendData(sockFd, (const char *)(&info), sizeof(info))) {
        return CloseSocketReturn(sockFd, -errno);
    }
    if (!PollSendData(sockFd, (char *)&uid, sizeof(int32_t))) {
        return CloseSocketReturn(sockFd, -errno);
    }
    if (!PollSendData(sockFd, (char *)&pid, sizeof(int32_t))) {
        return CloseSocketReturn(sockFd, -errno);
    }
    if (!PollSendData(sockFd, (char *)&failcause, sizeof(int32_t))) {
        return CloseSocketReturn(sockFd, -errno);
    }
    int32_t ret = NetSysPostDnsQueryForOne(sockFd, dnsInfo);
    return CloseSocketReturn(sockFd, ret);
}

void HandleQueryAbnormalReport(struct DnsProcessInfoExt dnsProcessInfo,
    struct AddrInfo addrInfo[static MAX_RESULTS], int32_t addrSize)
{
    if (IsSystemUid()) {
        return;
    }
    pthread_spin_lock(&g_dnsReportTimeLock);
    int64_t timeNow = (int64_t)(time(NULL));
    if (timeNow - g_lastDnsErrorReportTime < MIN_REPORT_INTERVAL) {
        pthread_spin_unlock(&g_dnsReportTimeLock);
        return;
    }
    int32_t failcause = GetQueryFailCause(&dnsProcessInfo, addrInfo, addrSize);
    if (failcause > FAIL_CAUSE_NONE) {
        g_dnsReportTime[failcause - 1] = timeNow;
        g_lastDnsErrorReportTime = timeNow;
        pthread_spin_unlock(&g_dnsReportTimeLock);
        struct DnsCacheInfo dnsInfo;
        dnsInfo.addrSize = addrSize;
        dnsInfo.dnsProcessInfo = dnsProcessInfo;
        if (memcpy_s(dnsInfo.addrInfo, sizeof(struct AddrInfo) * MAX_RESULTS,
            addrInfo, sizeof(struct AddrInfo) * MAX_RESULTS) != 0) {
            return;
        }
        NetsysPostDnsAbnormal(failcause, dnsInfo);
    } else {
        pthread_spin_unlock(&g_dnsReportTimeLock);
    }
}

int32_t NetSysPostDnsQueryResult(int netid, struct addrinfo *addr, char *srcAddr,
    struct DnsProcessInfo *processInfo)
{
    if (processInfo == NULL) {
        return -1;
    }
    if (processInfo->hostname == NULL) {
        return -1;
    }
    struct AddrInfo addrInfo[MAX_RESULTS] = {};
    int32_t resNum = 0;
    if (processInfo->retCode == 0) {
        resNum = FillAddrInfo(addrInfo, addr);
    }
    if (resNum < 0) {
        return -1;
    }
    struct DnsProcessInfoExt dnsProcessInfo;
    FillDnsProcessInfo(srcAddr, processInfo, &dnsProcessInfo);
    HandleQueryAbnormalReport(dnsProcessInfo, addrInfo, resNum);
    pthread_spin_lock(&g_dnsReportLock);
    if (g_curDnsStoreSize >= MAX_DNS_CACHE_SIZE) {
        pthread_spin_unlock(&g_dnsReportLock);
        return -1;
    }
    if (memcpy_s(g_dnsCaches[g_curDnsStoreSize].addrInfo, sizeof(struct AddrInfo) * MAX_RESULTS,
        addrInfo, sizeof(struct AddrInfo) * MAX_RESULTS) != 0) {
        pthread_spin_unlock(&g_dnsReportLock);
        return -1;
    }
    g_dnsCaches[g_curDnsStoreSize].addrSize = (uint8_t)resNum;
    g_dnsCaches[g_curDnsStoreSize].dnsProcessInfo = dnsProcessInfo;
    g_curDnsStoreSize++;
    int64_t timeNow = (int64_t)(time(NULL));
    if (lastDnsQueryPollSendTime == 0) {
        lastDnsQueryPollSendTime = timeNow;
        pthread_spin_unlock(&g_dnsReportLock);
        return 0;
    }
    if (timeNow - lastDnsQueryPollSendTime <  MIN_QUERY_REPORT_INTERVAL) {
        pthread_spin_unlock(&g_dnsReportLock);
        return 0;
    }
    lastDnsQueryPollSendTime = timeNow;
    pthread_spin_unlock(&g_dnsReportLock);
    NetSysPostDnsQueryResultInternal();
    return 0;
}

#ifdef __cplusplus
}
#endif
