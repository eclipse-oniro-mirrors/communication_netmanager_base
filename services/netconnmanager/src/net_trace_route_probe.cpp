/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "net_trace_route_probe.h"
#include <cctype>
#include <dirent.h>
#include <cerrno>
#include <fcntl.h>
#include <fnmatch.h>
#include <grp.h>
#include <cinttypes>
#include <climits>
#include <cmath>
#include <paths.h>
#include <pwd.h>
#include <regex.h>
#include <sched.h>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <strings.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <termios.h>
#include <ctime>
#include <unistd.h>
#include <utime.h>
 
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
 
#include <clocale>
#include <cwchar>
#include <cwctype>

#include <thread>
 
#include <sys/ioctl.h>
 
#include <ifaddrs.h>
#include <netinet/ip_icmp.h>
#include "ffrt.h"

#define TRACE_ROUTE_DATA_SIZE 1024

#define ICMP_ECHO_REQUEST 8
#define ICMPV6_ECHO_REQUEST 128
#define HTTP_STATUS_401 401

namespace OHOS {
namespace NetManagerStandard {

typedef struct IpInfo {
    int ttl;
    std::string ip;
    int delay[5];
    std::string rtt;
} IpInfo;

static long long now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec*1000 + ts.tv_nsec/1000000;
}

unsigned short traceRoute_cksum(uint16_t *data, int len)
{
    uint32_t sum = 0;
    uint16_t answer = 0;

    // 累加所有 16-bit 字
    while (len > 1) {
        sum += *data++;
        len -= 2;
    }

    // 如果剩余 1 字节（奇数长度），补零并累加
    if (len == 1) {
        *reinterpret_cast<uint8_t *>(&answer) = *reinterpret_cast<uint8_t *>(data);
        sum += answer;
    }

    // 回卷溢出位（carry-around）
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    // 取反码
    answer = ~sum;
    return answer;
}

static int wait_response(int fd, int flag)
{
    struct pollfd pfd;

    pfd.fd = fd;
    pfd.events = POLLIN;
    if (poll(&pfd, 1, 1000) <= 0) {
        return -1;
    }
    return 0;
}

void ComputeRtt(struct IpInfo &ipinfo)
{
    // 初始化变量
    int max_value = ipinfo.delay[0];
    int min_value = ipinfo.delay[0];
    int sum = 0.0;

    // 遍历数组，计算最大值、最小值和总和
    for (int i = 0; i < 5; ++i) {
        if (ipinfo.delay[i] > max_value) {
            max_value = ipinfo.delay[i];
        }
        if (ipinfo.delay[i] < min_value) {
            min_value = ipinfo.delay[i];
        }
        sum += ipinfo.delay[i];
    }

    int avg = sum / 5; // 计算平均值
     
    // 计算标准差
    int variance_sum = 0.0;
    for (int i = 0; i < 5; ++i) {
        variance_sum += (ipinfo.delay[i] - avg) * (ipinfo.delay[i] - avg);
    }
    int variance = variance_sum / 5;
    int standard_deviation = sqrt(variance);
    ipinfo.rtt = std::to_string(max_value) + ";" + std::to_string(min_value) + ";" + std::to_string(avg)
        + ";" + std::to_string(standard_deviation) + " ";
}

std::string getIPAddress(struct addrinfo *ai)
{
    std::string host;
    if (ai->ai_family == AF_INET) {
        auto addr = reinterpret_cast<sockaddr_in *>(ai->ai_addr);
        char ip[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
        host = ip;
    } else if (ai->ai_family == AF_INET6) {
        auto addr = reinterpret_cast<sockaddr_in6 *>(ai->ai_addr);
        char ip[INET6_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
        host = ip;
    }
    return host;
}

void TimeOutHandle(struct IpInfo &ipinfo, struct addrinfo *ai, int count)
{
    if (ipinfo.ip == "" ) {
        ipinfo.ip = "*.*.*.*";
        for (int i = 0; i < 5; i++) {
            ipinfo.delay[i] = 1000;
        }
        return;
    }
    ipinfo.delay[count] = 1000;
}

void ReSend(struct IpInfo &ipinfo, int i)
{
    struct addrinfo info = {0};
    struct addrinfo *ai = nullptr;
    int family = AF_UNSPEC;
    info.ai_family = family;
    const char *dest = ipinfo.ip.c_str();
    if (getaddrinfo(dest, nullptr, &info, &ai) < 0){
        return;
    }
	if (ai == nullptr) {
		return;
	}
    int sockfd = socket(ai->ai_family, SOCK_DGRAM, (ai->ai_family == AF_INET) ? IPPROTO_ICMP : IPPROTO_ICMPV6);
    if (sockfd < 0) {
        return;
    }
    unsigned char buffer[sizeof(struct icmphdr) + TRACE_ROUTE_DATA_SIZE] = {0};   /* icmp header and data */
    struct icmphdr *ih = reinterpret_cast<struct icmphdr*>(buffer);
    ih->type = (ai->ai_family == AF_INET) ? ICMP_ECHO_REQUEST : ICMPV6_ECHO_REQUEST;
    ih->code = 0;
    ih->un.echo.id = getpid();
    ih->un.echo.sequence = ipinfo.ttl;
    ih->checksum = 0;
    ih->checksum = traceRoute_cksum(reinterpret_cast<uint16_t *>(ih), sizeof(ih));
    long long time_send = now();
    if (sendto(sockfd, buffer, sizeof(buffer), 0, ai->ai_addr, sizeof(*(ai->ai_addr))) < 0) {
        close(sockfd);
        return;
    }
    if (wait_response(sockfd, 0) < 0) {
        TimeOutHandle(ipinfo, ai, i+1); // 超时处理
        close(sockfd);
        return;
    }
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);
    char recv_buffer[1024];
    if (recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0,
        reinterpret_cast<struct sockaddr *>(&src_addr), &addr_len) <= 0) {
        close(sockfd);
        return;
    }
    long long time_recv = now() - time_send;
    ipinfo.rtt[i] = time_recv; // 记录rtt
    close(sockfd);
    return;
}

void Send(struct IpInfo &ipinfo)
{
    std::vector<ffrt::task_handle> tasks(4);  // 创建ffrt数组
    
    // 循环创建ffrt
    for (uint i = 0; i < 4; ++i) {
        auto task = ffrt::submit_h([ &, i]() {  
            ReSend(ipinfo, i); 
            }, {}, {}, {ffrt::task_attr().name(("ReSend" + std::to_string(i)).c_str())});
        tasks[i] = std::move(task);
    }

    // 等待所有ffrt完成
    for (auto& task : tasks) {
        ffrt::wait({task});  // 等待ffrt结束
    }
    ComputeRtt(ipinfo);
    return;
}

void CreateTasks(std::vector<struct IpInfo> &ipinfo)
{
    std::vector<ffrt::task_handle> tasks(ipinfo.size());  // 创建ffrt数组
    for (uint i = 0; i < ipinfo.size(); ++i) {
        if (ipinfo[i].ip == "*.*.*.*") {
            continue;
        }
        struct IpInfo &info = ipinfo[i];
        auto task = ffrt::submit_h([ & ]() { Send(info); 
            }, {}, {}, {ffrt::task_attr().name(("Send" + std::to_string(i)).c_str())});
        tasks[i] = std::move(task);
    }
    for (auto& task : tasks) {
        ffrt::wait({task});  // 等待ffrt结束
    }
}

void recv(struct IpInfo &info, std::vector<struct IpInfo> &ipinfo, int sockfd, long long time_send, int family)
{
    char recv_buffer[1024];
    if (family == AF_INET) {
        struct sockaddr_in src_addr;
        socklen_t addr_len = sizeof(src_addr);
        ssize_t received = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0,
            reinterpret_cast<struct sockaddr *>(&src_addr), &addr_len);
        char src_ip[INET_ADDRSTRLEN]; 
        inet_ntop(AF_INET, &(src_addr.sin_addr), src_ip, INET_ADDRSTRLEN);
        info.ip = std::string(src_ip); // 记录IP地址
    } else if (family == AF_INET6) {
        struct sockaddr_in6 src_addr;
        socklen_t addr_len = sizeof(src_addr);
        ssize_t received = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0,
            reinterpret_cast<struct sockaddr *>(&src_addr), &addr_len);
        char src_ip[INET6_ADDRSTRLEN]; 
        inet_ntop(AF_INET6, &(src_addr.sin6_addr), src_ip, INET6_ADDRSTRLEN);
        info.ip = std::string(src_ip); // 记录IPV6地址 
    }
    long long time_recv = now() - time_send;
    info.delay[0] = time_recv;
    ipinfo.push_back(info); // 将info对象添加到vector中
}

static int doTraceRoute(struct addrinfo *ai, int32_t maxJumpNumber, int32_t packetsType, std::string &traceRouteInfo)
{
    std::vector<struct IpInfo> ipinfo;
    int sockfd = socket(ai->ai_family, SOCK_RAW, (ai->ai_family == AF_INET) ? IPPROTO_ICMP : IPPROTO_ICMPV6);
    if (sockfd < 0) {
        return HTTP_STATUS_401;
    }
    int32_t count = 0;
    for (int32_t ttl = 1; ttl <= maxJumpNumber; ttl++) {
        struct IpInfo info; // 每次循环创建一个IpInfo对象
        info.ttl = ttl;
        unsigned char buffer[sizeof(struct icmphdr) + TRACE_ROUTE_DATA_SIZE] = {0};   /* icmp header and data */
	    struct icmphdr *ih = reinterpret_cast<struct icmphdr*>(buffer);
        ih->type = (ai->ai_family == AF_INET) ? ICMP_ECHO_REQUEST : ICMPV6_ECHO_REQUEST;
        ih->code = 0;
        ih->un.echo.id = getpid();
        ih->un.echo.sequence = ttl;
        setsockopt(sockfd, (ai->ai_family == AF_INET) ? SOL_IP : SOL_IPV6,
            (ai->ai_family == AF_INET) ? IP_TTL : IPV6_UNICAST_HOPS, &ttl, sizeof(ttl));
        ih->checksum = 0;
        ih->checksum = traceRoute_cksum(reinterpret_cast<uint16_t*>(buffer), sizeof(buffer));
        long long time_send = now();
        ssize_t sent = sendto(sockfd, buffer, sizeof(buffer), 0,
            ai->ai_addr, ai->ai_addrlen);
        if (sent < 0) {
            continue;
        }
        int rc = wait_response(sockfd, 0);
		if (rc < 0) {
            count++;
            TimeOutHandle(info, ai, 0); // 超时处理
            ComputeRtt(info);
            ipinfo.push_back(info); // 将info对象添加到vector中
            if (count >= 3) { // 5跳超时，直接break
                break;
            }
            continue;
        }
        recv(info, ipinfo, sockfd, time_send, ai->ai_family);
        if (info.ip == getIPAddress(ai)) {
            break;
        }
    }
    CreateTasks(ipinfo);
    close(sockfd);
    for (uint i = 0; i < ipinfo.size(); ++i) {
        traceRouteInfo += std::to_string(ipinfo[i].ttl) + " " + ipinfo[i].ip + " " + ipinfo[i].rtt;
    }
    return 0;
}

int32_t QueryTraceRouteProbeResult(const std::string &destination, int32_t maxJumpNumber, int32_t packetsType,
    std::string &traceRouteInfo)
{
    struct addrinfo info = {0};
    struct addrinfo *ai = nullptr;
    int32_t family = AF_UNSPEC;

    info.ai_family = family;
    const char *dest = destination.c_str();
    int32_t rc = getaddrinfo(dest, nullptr, &info, &ai);
    if (rc < 0){
        return HTTP_STATUS_401;
    }
    
	if (ai == nullptr) {
		errno = EADDRNOTAVAIL;
		return HTTP_STATUS_401;
	}

    rc = doTraceRoute(ai, maxJumpNumber, packetsType, traceRouteInfo);

    if (ai != nullptr) {
        freeaddrinfo(ai);
    }
	return rc;
}

}
}