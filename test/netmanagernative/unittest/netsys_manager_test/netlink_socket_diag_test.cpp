/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "net_manager_constants.h"
#include "netlink_socket_diag.cpp"
#include "netlink_socket_diag.h"
#include <net/if.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>
#include <cstring>
#include <sys/uio.h>
#include <unistd.h>

#include "fwmark.h"
#include "netmanager_base_common_utils.h"
#include "netnative_log_wrapper.h"
#include "securec.h"
namespace OHOS {
namespace nmd {
namespace {
using namespace testing::ext;
}
class NetlinkSocketDiagTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetlinkSocketDiagTest::SetUpTestCase() {}

void NetlinkSocketDiagTest::TearDownTestCase() {}

void NetlinkSocketDiagTest::SetUp() {}

void NetlinkSocketDiagTest::TearDown() {}

HWTEST_F(NetlinkSocketDiagTest, InLookBack001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    uint32_t a = 0x7f000000;
    bool result = netLinkSocketDiag.InLookBack(a);
    EXPECT_EQ(result, true);
}

HWTEST_F(NetlinkSocketDiagTest, InLookBack002, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    uint32_t a = 0x6f000000;
    bool result = netLinkSocketDiag.InLookBack(a);
    EXPECT_EQ(result, false);
}

HWTEST_F(NetlinkSocketDiagTest, ExecuteDestroySocket001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    uint8_t proto = IPPROTO_TCP;
    const inet_diag_msg *msg = nullptr;
    int32_t result = netLinkSocketDiag.ExecuteDestroySocket(proto, msg);
    EXPECT_EQ(result, NETMANAGER_ERR_LOCAL_PTR_NULL);
}

HWTEST_F(NetlinkSocketDiagTest, ExecuteDestroySocket002, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    uint8_t proto = IPPROTO_TCP;
    inet_diag_msg temp_msg;
    temp_msg.idiag_family = AF_INET;
    temp_msg.idiag_state = TCP_ESTABLISHED;
    const inet_diag_msg *msg = &temp_msg;
    int32_t result = netLinkSocketDiag.ExecuteDestroySocket(proto, msg);
    EXPECT_EQ(result, NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(NetlinkSocketDiagTest, ExecuteDestroySocket003, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    uint8_t proto = IPPROTO_TCP;
    inet_diag_msg temp_msg;
    temp_msg.idiag_family = AF_INET;
    temp_msg.idiag_state = TCP_ESTABLISHED;
    const inet_diag_msg *msg = &temp_msg;
    NetLinkSocketDiag::SockDiagRequest request;
    request.nlh_.nlmsg_type = SOCK_DESTROY;
    request.nlh_.nlmsg_flags = NLM_F_REQUEST;
    request.nlh_.nlmsg_len = 10;

    write(-1, &request, sizeof(request));
    int32_t result = netLinkSocketDiag.ExecuteDestroySocket(proto, msg);
    EXPECT_EQ(result, NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(NetlinkSocketDiagTest, GetErrorFromKernel001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    int32_t fd = 1;
    errno = EAGAIN;
    NetLinkSocketDiag::Ack ack;
    recv(fd, &ack, sizeof(ack), MSG_DONTWAIT | MSG_PEEK);
    int32_t result = netLinkSocketDiag.GetErrorFromKernel(fd);
    EXPECT_NE(result, NETMANAGER_SUCCESS);
}

HWTEST_F(NetlinkSocketDiagTest, IsMatchNetwork001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_INET;
    msg.id.idiag_src[0] = inet_addr("192.168.1.10");
    msg.id.idiag_dst[0] = inet_addr("192.168.1.20");

    EXPECT_TRUE(netLinkSocketDiag.IsMatchNetwork(&msg, "192.168.1.10"));
    EXPECT_TRUE(netLinkSocketDiag.IsMatchNetwork(&msg, "192.168.1.20"));
    EXPECT_FALSE(netLinkSocketDiag.IsMatchNetwork(&msg, "192.168.1.30"));
}

HWTEST_F(NetlinkSocketDiagTest, IsMatchNetwork002, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_INET;
    msg.id.idiag_src[0] = inet_addr("10.0.0.1");
    msg.id.idiag_dst[0] = inet_addr("10.0.0.2");

    EXPECT_FALSE(netLinkSocketDiag.IsMatchNetwork(&msg, "192.168.1.10"));
}

HWTEST_F(NetlinkSocketDiagTest, IsMatchNetwork003, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_INET6;
    inet_pton(AF_INET6, "2001:db8::1", &msg.id.idiag_src);
    inet_pton(AF_INET6, "2001:db8::2", &msg.id.idiag_dst);

    EXPECT_TRUE(netLinkSocketDiag.IsMatchNetwork(&msg, "2001:db8::1"));
    EXPECT_TRUE(netLinkSocketDiag.IsMatchNetwork(&msg, "2001:db8::2"));
    EXPECT_FALSE(netLinkSocketDiag.IsMatchNetwork(&msg, "2001:db8::3"));
}

HWTEST_F(NetlinkSocketDiagTest, IsMatchNetwork004, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_INET6;
    inet_pton(AF_INET6, "::1", &msg.id.idiag_src);
    inet_pton(AF_INET6, "::2", &msg.id.idiag_dst);

    EXPECT_FALSE(netLinkSocketDiag.IsMatchNetwork(&msg, "2001:db8::1"));
}

HWTEST_F(NetlinkSocketDiagTest, IsMatchNetwork005, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_INET;
    msg.id.idiag_src[0] = inet_addr("192.168.1.10");

    EXPECT_FALSE(netLinkSocketDiag.IsMatchNetwork(&msg, "2001:db8::1"));
}

HWTEST_F(NetlinkSocketDiagTest, IsMatchNetwork006, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_INET6;
    msg.id.idiag_src[0] = 0;
    msg.id.idiag_src[1] = 0;
    msg.id.idiag_src[2] = htonl(0xffff);
    msg.id.idiag_src[3] = inet_addr("192.168.1.10");
 
    EXPECT_TRUE(netLinkSocketDiag.IsMatchNetwork(&msg, "192.168.1.10"));
}

HWTEST_F(NetlinkSocketDiagTest, SetSocketDestroyType001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    int socketType = static_cast<int>(SocketDestroyType::DESTROY_SPECIAL_CELLULAR);
    netLinkSocketDiag.SetSocketDestroyType(socketType);
    EXPECT_EQ(netLinkSocketDiag.socketDestroyType_, SocketDestroyType::DESTROY_SPECIAL_CELLULAR);

    socketType = static_cast<int>(SocketDestroyType::DESTROY_DEFAULT_CELLULAR);
    netLinkSocketDiag.SetSocketDestroyType(socketType);
    EXPECT_EQ(netLinkSocketDiag.socketDestroyType_, SocketDestroyType::DESTROY_DEFAULT_CELLULAR);

    socketType = static_cast<int>(SocketDestroyType::DESTROY_DEFAULT);
    netLinkSocketDiag.SetSocketDestroyType(socketType);
    EXPECT_EQ(netLinkSocketDiag.socketDestroyType_, SocketDestroyType::DESTROY_DEFAULT);
}

HWTEST_F(NetlinkSocketDiagTest, CloseNetlinkSocket001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    netLinkSocketDiag.dumpSock_ = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_INET_DIAG);
    netLinkSocketDiag.destroySock_ = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_INET_DIAG);
    netLinkSocketDiag.CloseNetlinkSocket();
    EXPECT_EQ(netLinkSocketDiag.dumpSock_, -1);
    EXPECT_EQ(netLinkSocketDiag.destroySock_, -1);
}

HWTEST_F(NetlinkSocketDiagTest, StripV4MappedPrefix001, TestSize.Level1)
{
    std::string result = NetLinkSocketDiag::StripV4MappedPrefix("::ffff:192.168.1.1");
    EXPECT_EQ(result, "192.168.1.1");
}

HWTEST_F(NetlinkSocketDiagTest, StripV4MappedPrefix002, TestSize.Level1)
{
    std::string result = NetLinkSocketDiag::StripV4MappedPrefix("192.168.1.1");
    EXPECT_EQ(result, "192.168.1.1");
}

HWTEST_F(NetlinkSocketDiagTest, StripV4MappedPrefix003, TestSize.Level1)
{
    std::string result = NetLinkSocketDiag::StripV4MappedPrefix("2001:db8::1");
    EXPECT_EQ(result, "2001:db8::1");
}

HWTEST_F(NetlinkSocketDiagTest, StripV4MappedPrefix004, TestSize.Level1)
{
    std::string result = NetLinkSocketDiag::StripV4MappedPrefix("::ffff:invalid");
    EXPECT_EQ(result, "::ffff:invalid");
}

HWTEST_F(NetlinkSocketDiagTest, StripV4MappedPrefix005, TestSize.Level1)
{
    std::string result = NetLinkSocketDiag::StripV4MappedPrefix("::1");
    EXPECT_EQ(result, "::1");
}

HWTEST_F(NetlinkSocketDiagTest, StripV4MappedPrefix006, TestSize.Level1)
{
    std::string result = NetLinkSocketDiag::StripV4MappedPrefix("");
    EXPECT_EQ(result, "");
}

HWTEST_F(NetlinkSocketDiagTest, StripV4MappedPrefix007, TestSize.Level1)
{
    std::string result = NetLinkSocketDiag::StripV4MappedPrefix("::ffff:10.0.0.1");
    EXPECT_EQ(result, "10.0.0.1");
}

HWTEST_F(NetlinkSocketDiagTest, StripV4MappedPrefix008, TestSize.Level1)
{
    std::string result = NetLinkSocketDiag::StripV4MappedPrefix("::ffff:127.0.0.1");
    EXPECT_EQ(result, "127.0.0.1");
}

HWTEST_F(NetlinkSocketDiagTest, GetTcpNetPortStatesInfoV4Mapped001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_INET6;
    msg.id.idiag_src[0] = 0;
    msg.id.idiag_src[1] = 0;
    msg.id.idiag_src[2] = htonl(0xffff);
    msg.id.idiag_src[3] = inet_addr("192.168.1.10");
    msg.id.idiag_dst[0] = 0;
    msg.id.idiag_dst[1] = 0;
    msg.id.idiag_dst[2] = htonl(0xffff);
    msg.id.idiag_dst[3] = inet_addr("10.0.0.1");
    msg.id.idiag_sport = htons(12345);
    msg.id.idiag_dport = htons(80);
    msg.idiag_uid = 1000;
    msg.idiag_state = TCP_ESTABLISHED;

    NetManagerStandard::NetPortStatesInfo netPortStatesInfo;
    bool ret = netLinkSocketDiag.GetTcpNetPortStatesInfo(&msg, netPortStatesInfo, 100);
    EXPECT_TRUE(ret);
    ASSERT_EQ(netPortStatesInfo.tcpNetPortStatesInfo_.size(), 1U);
    EXPECT_EQ(netPortStatesInfo.tcpNetPortStatesInfo_[0].tcpLocalIp_, "192.168.1.10");
    EXPECT_EQ(netPortStatesInfo.tcpNetPortStatesInfo_[0].tcpRemoteIp_, "10.0.0.1");
}

HWTEST_F(NetlinkSocketDiagTest, GetTcpNetPortStatesInfoV6Native001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_INET6;
    inet_pton(AF_INET6, "2001:db8::1", &msg.id.idiag_src);
    inet_pton(AF_INET6, "2001:db8::2", &msg.id.idiag_dst);
    msg.id.idiag_sport = htons(12345);
    msg.id.idiag_dport = htons(80);
    msg.idiag_uid = 1000;
    msg.idiag_state = TCP_ESTABLISHED;

    NetManagerStandard::NetPortStatesInfo netPortStatesInfo;
    bool ret = netLinkSocketDiag.GetTcpNetPortStatesInfo(&msg, netPortStatesInfo, 100);
    EXPECT_TRUE(ret);
    ASSERT_EQ(netPortStatesInfo.tcpNetPortStatesInfo_.size(), 1U);
    EXPECT_EQ(netPortStatesInfo.tcpNetPortStatesInfo_[0].tcpLocalIp_, "2001:db8::1");
    EXPECT_EQ(netPortStatesInfo.tcpNetPortStatesInfo_[0].tcpRemoteIp_, "2001:db8::2");
}

HWTEST_F(NetlinkSocketDiagTest, GetTcpNetPortStatesInfoNull001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    NetManagerStandard::NetPortStatesInfo netPortStatesInfo;
    bool ret = netLinkSocketDiag.GetTcpNetPortStatesInfo(nullptr, netPortStatesInfo, 100);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetlinkSocketDiagTest, GetTcpNetPortStatesInfoV4Direct001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_INET;
    msg.id.idiag_src[0] = inet_addr("192.168.1.10");
    msg.id.idiag_dst[0] = inet_addr("10.0.0.1");
    msg.id.idiag_sport = htons(12345);
    msg.id.idiag_dport = htons(80);
    msg.idiag_uid = 1000;
    msg.idiag_state = TCP_ESTABLISHED;

    NetManagerStandard::NetPortStatesInfo netPortStatesInfo;
    bool ret = netLinkSocketDiag.GetTcpNetPortStatesInfo(&msg, netPortStatesInfo, 100);
    EXPECT_TRUE(ret);
    ASSERT_EQ(netPortStatesInfo.tcpNetPortStatesInfo_.size(), 1U);
    EXPECT_EQ(netPortStatesInfo.tcpNetPortStatesInfo_[0].tcpLocalIp_, "192.168.1.10");
    EXPECT_EQ(netPortStatesInfo.tcpNetPortStatesInfo_[0].tcpRemoteIp_, "10.0.0.1");
}

HWTEST_F(NetlinkSocketDiagTest, GetUdpNetPortStatesInfoV4Mapped001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_INET6;
    msg.id.idiag_src[0] = 0;
    msg.id.idiag_src[1] = 0;
    msg.id.idiag_src[2] = htonl(0xffff);
    msg.id.idiag_src[3] = inet_addr("192.168.1.20");
    msg.id.idiag_sport = htons(54321);
    msg.idiag_uid = 2000;

    NetManagerStandard::NetPortStatesInfo netPortStatesInfo;
    bool ret = netLinkSocketDiag.GetUdpNetPortStatesInfo(&msg, netPortStatesInfo, 200);
    EXPECT_TRUE(ret);
    ASSERT_EQ(netPortStatesInfo.udpNetPortStatesInfo_.size(), 1U);
    EXPECT_EQ(netPortStatesInfo.udpNetPortStatesInfo_[0].udpLocalIp_, "192.168.1.20");
}

HWTEST_F(NetlinkSocketDiagTest, GetTcpNetPortStatesInfoInvalidAddr001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_UNSPEC;

    NetManagerStandard::NetPortStatesInfo netPortStatesInfo;
    bool ret = netLinkSocketDiag.GetTcpNetPortStatesInfo(&msg, netPortStatesInfo, 100);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetlinkSocketDiagTest, GetUdpNetPortStatesInfoInvalidAddr001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_UNSPEC;

    NetManagerStandard::NetPortStatesInfo netPortStatesInfo;
    bool ret = netLinkSocketDiag.GetUdpNetPortStatesInfo(&msg, netPortStatesInfo, 200);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetlinkSocketDiagTest, GetUdpNetPortStatesInfoV6Native001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_INET6;
    inet_pton(AF_INET6, "fe80::1", &msg.id.idiag_src);
    msg.id.idiag_sport = htons(54321);
    msg.idiag_uid = 2000;

    NetManagerStandard::NetPortStatesInfo netPortStatesInfo;
    bool ret = netLinkSocketDiag.GetUdpNetPortStatesInfo(&msg, netPortStatesInfo, 200);
    EXPECT_TRUE(ret);
    ASSERT_EQ(netPortStatesInfo.udpNetPortStatesInfo_.size(), 1U);
    EXPECT_EQ(netPortStatesInfo.udpNetPortStatesInfo_[0].udpLocalIp_, "fe80::1");
}

HWTEST_F(NetlinkSocketDiagTest, GetUdpNetPortStatesInfoNull001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    NetManagerStandard::NetPortStatesInfo netPortStatesInfo;
    bool ret = netLinkSocketDiag.GetUdpNetPortStatesInfo(nullptr, netPortStatesInfo, 200);
    EXPECT_FALSE(ret);
}

HWTEST_F(NetlinkSocketDiagTest, GetUdpNetPortStatesInfoV4Direct001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    inet_diag_msg msg;
    msg.idiag_family = AF_INET;
    msg.id.idiag_src[0] = inet_addr("192.168.1.20");
    msg.id.idiag_sport = htons(54321);
    msg.idiag_uid = 2000;

    NetManagerStandard::NetPortStatesInfo netPortStatesInfo;
    bool ret = netLinkSocketDiag.GetUdpNetPortStatesInfo(&msg, netPortStatesInfo, 200);
    EXPECT_TRUE(ret);
    ASSERT_EQ(netPortStatesInfo.udpNetPortStatesInfo_.size(), 1U);
    EXPECT_EQ(netPortStatesInfo.udpNetPortStatesInfo_[0].udpLocalIp_, "192.168.1.20");
}
namespace {
constexpr uint32_t TEST_LOCAL_PORT = 12345;
constexpr uint32_t TEST_REMOTE_PORT = 51302;

SocketTuple MakeTestSocketTuple(uint8_t proto = IPPROTO_TCP, uint8_t family = AF_INET)
{
    SocketTuple tuple;
    tuple.proto = proto;
    tuple.family = family;
    tuple.localAddress = "127.0.0.1";
    tuple.localPort = TEST_LOCAL_PORT;
    tuple.remoteAddress = "127.0.0.1";
    tuple.remotePort = TEST_REMOTE_PORT;
    return tuple;
}

size_t FillNlMsgDone(char *buf)
{
    auto *nlh = reinterpret_cast<nlmsghdr *>(buf);
    nlh->nlmsg_len = NLMSG_LENGTH(0);
    nlh->nlmsg_type = NLMSG_DONE;
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_seq = 0;
    nlh->nlmsg_pid = 0;
    return NLMSG_ALIGN(nlh->nlmsg_len);
}

size_t FillNlMsgError(char *buf, int32_t error)
{
    auto *nlh = reinterpret_cast<nlmsghdr *>(buf);
    nlh->nlmsg_len = NLMSG_LENGTH(sizeof(nlmsgerr));
    nlh->nlmsg_type = NLMSG_ERROR;
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_seq = 0;
    nlh->nlmsg_pid = 0;
    auto *err = reinterpret_cast<nlmsgerr *>(NLMSG_DATA(nlh));
    err->error = error;
    return NLMSG_ALIGN(nlh->nlmsg_len);
}

size_t FillNlMsgSocket(char *buf)
{
    auto *nlh = reinterpret_cast<nlmsghdr *>(buf);
    nlh->nlmsg_len = NLMSG_LENGTH(sizeof(inet_diag_msg));
    nlh->nlmsg_type = 0x1234;
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_seq = 0;
    nlh->nlmsg_pid = 0;
    auto *msg = reinterpret_cast<inet_diag_msg *>(NLMSG_DATA(nlh));
    msg->idiag_family = AF_INET;
    msg->idiag_state = 1;
    msg->id.idiag_sport = htons(TEST_LOCAL_PORT);
    msg->id.idiag_dport = htons(TEST_REMOTE_PORT);
    return NLMSG_ALIGN(nlh->nlmsg_len);
}
} // namespace

HWTEST_F(NetlinkSocketDiagTest, DestroySocketByTupleInvalidProto001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    EXPECT_EQ(netLinkSocketDiag.DestroySocketByTuple(MakeTestSocketTuple(IPPROTO_UDP)),
        NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetlinkSocketDiagTest, DestroySocketByTupleInvalidFamily001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    EXPECT_EQ(netLinkSocketDiag.DestroySocketByTuple(MakeTestSocketTuple(IPPROTO_TCP, AF_UNSPEC)),
        NETMANAGER_ERR_PARAMETER_ERROR);
}

HWTEST_F(NetlinkSocketDiagTest, DestroySocketByTupleInvalidAddress001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    SocketTuple tuple = MakeTestSocketTuple();
    tuple.localAddress = "invalid";
    EXPECT_EQ(netLinkSocketDiag.DestroySocketByTuple(tuple), NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(NetlinkSocketDiagTest, DestroySocketByTupleSuccess001, TestSize.Level1)
{
    int32_t dumpFds[2] = {-1, -1};
    int32_t destroyFds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, dumpFds), 0);
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, destroyFds), 0);
    // Inject a fake kernel dump response (one matched socket + DONE) into the dump peer.
    char buf[512] = {0};
    size_t len = FillNlMsgSocket(buf);
    len += FillNlMsgDone(buf + len);
    ASSERT_EQ(write(dumpFds[1], buf, len), static_cast<ssize_t>(len));

    NetLinkSocketDiag netLinkSocketDiag;
    netLinkSocketDiag.dumpSock_ = dumpFds[0];
    netLinkSocketDiag.destroySock_ = destroyFds[0];
    EXPECT_EQ(netLinkSocketDiag.DestroySocketByTuple(MakeTestSocketTuple()), NETMANAGER_SUCCESS);
    EXPECT_EQ(netLinkSocketDiag.socketsDestroyed_, 1);

    // Verify the SOCK_DESTROY request built from the dumped socket tuple.
    NetLinkSocketDiag::SockDiagRequest destroyRequest = {};
    ssize_t got = recv(destroyFds[1], &destroyRequest, sizeof(destroyRequest), MSG_DONTWAIT);
    ASSERT_EQ(got, static_cast<ssize_t>(sizeof(destroyRequest)));
    EXPECT_EQ(destroyRequest.nlh_.nlmsg_type, SOCK_DESTROY);
    EXPECT_EQ(destroyRequest.req_.sdiag_family, AF_INET);
    EXPECT_EQ(destroyRequest.req_.sdiag_protocol, IPPROTO_TCP);
    EXPECT_EQ(destroyRequest.req_.id.idiag_sport, htons(TEST_LOCAL_PORT));
    EXPECT_EQ(destroyRequest.req_.id.idiag_dport, htons(TEST_REMOTE_PORT));

    close(dumpFds[1]);
    close(destroyFds[1]);
}

HWTEST_F(NetlinkSocketDiagTest, EnsureNetlinkSocketReuse001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    ASSERT_TRUE(netLinkSocketDiag.CreateNetlinkSocket());
    int32_t dumpSock = netLinkSocketDiag.dumpSock_;
    int32_t destroySock = netLinkSocketDiag.destroySock_;
    EXPECT_TRUE(netLinkSocketDiag.EnsureNetlinkSocket());
    EXPECT_EQ(netLinkSocketDiag.dumpSock_, dumpSock);
    EXPECT_EQ(netLinkSocketDiag.destroySock_, destroySock);
}

HWTEST_F(NetlinkSocketDiagTest, EnsureNetlinkSocketRecreate001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    ASSERT_TRUE(netLinkSocketDiag.CreateNetlinkSocket());
    netLinkSocketDiag.dumpSock_ = -1;
    EXPECT_TRUE(netLinkSocketDiag.EnsureNetlinkSocket());
    EXPECT_GE(netLinkSocketDiag.dumpSock_, 0);
    EXPECT_GE(netLinkSocketDiag.destroySock_, 0);
}

HWTEST_F(NetlinkSocketDiagTest, SendTupleDumpRequestWriteFail001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    netLinkSocketDiag.dumpSock_ = -1;
    EXPECT_EQ(netLinkSocketDiag.SendTupleDumpRequest(MakeTestSocketTuple()), NETMANAGER_ERR_INTERNAL);
}

HWTEST_F(NetlinkSocketDiagTest, SendTupleDumpRequestSuccess001, TestSize.Level1)
{
    int32_t fds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, fds), 0);
    NetLinkSocketDiag netLinkSocketDiag;
    netLinkSocketDiag.dumpSock_ = fds[0];
    EXPECT_EQ(netLinkSocketDiag.SendTupleDumpRequest(MakeTestSocketTuple()), NETMANAGER_SUCCESS);
    close(fds[1]);
}

HWTEST_F(NetlinkSocketDiagTest, ProcessSockDiagTupleDumpResponseReadFail001, TestSize.Level1)
{
    NetLinkSocketDiag netLinkSocketDiag;
    netLinkSocketDiag.dumpSock_ = -1;
    EXPECT_EQ(netLinkSocketDiag.ProcessSockDiagTupleDumpResponse(IPPROTO_TCP), NETMANAGER_ERROR);
}

HWTEST_F(NetlinkSocketDiagTest, ProcessSockDiagTupleDumpResponseDone001, TestSize.Level1)
{
    int32_t fds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, fds), 0);
    char buf[256] = {0};
    size_t len = FillNlMsgDone(buf);
    ASSERT_EQ(write(fds[1], buf, len), static_cast<ssize_t>(len));
    NetLinkSocketDiag netLinkSocketDiag;
    netLinkSocketDiag.dumpSock_ = fds[0];
    EXPECT_EQ(netLinkSocketDiag.ProcessSockDiagTupleDumpResponse(IPPROTO_TCP), NETMANAGER_SUCCESS);
    close(fds[1]);
}

HWTEST_F(NetlinkSocketDiagTest, ProcessSockDiagTupleDumpResponseError001, TestSize.Level1)
{
    int32_t fds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, fds), 0);
    char buf[256] = {0};
    size_t len = FillNlMsgError(buf, -EPERM);
    ASSERT_EQ(write(fds[1], buf, len), static_cast<ssize_t>(len));
    NetLinkSocketDiag netLinkSocketDiag;
    netLinkSocketDiag.dumpSock_ = fds[0];
    EXPECT_EQ(netLinkSocketDiag.ProcessSockDiagTupleDumpResponse(IPPROTO_TCP), NETMANAGER_ERROR);
    close(fds[1]);
}

HWTEST_F(NetlinkSocketDiagTest, ProcessSockDiagTupleDumpResponseSocket001, TestSize.Level1)
{
    int32_t fds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, fds), 0);
    char buf[512] = {0};
    size_t len = FillNlMsgSocket(buf);
    len += FillNlMsgDone(buf + len);
    ASSERT_EQ(write(fds[1], buf, len), static_cast<ssize_t>(len));
    NetLinkSocketDiag netLinkSocketDiag;
    netLinkSocketDiag.dumpSock_ = fds[0];
    netLinkSocketDiag.destroySock_ = -1;
    EXPECT_EQ(netLinkSocketDiag.ProcessSockDiagTupleDumpResponse(IPPROTO_TCP), NETMANAGER_SUCCESS);
    close(fds[1]);
}
} // namespace nmd
} // namespace OHOS