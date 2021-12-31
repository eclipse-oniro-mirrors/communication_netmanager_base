#ifndef __INCLUDE_DHCP_CONTROLLER_H__
#define __INCLUDE_DHCP_CONTROLLER_H__

#include "i_dhcp_result_notify.h"
#include "dhcp_service.h"
#include "i_notify_callback.h"

namespace OHOS {
namespace nmd {
class DhcpController {
public:
    class DhcpControllerResultNotify : public OHOS::Wifi::IDhcpResultNotify {
    public:
        explicit DhcpControllerResultNotify(DhcpController &dhcpController);
        ~DhcpControllerResultNotify() override;
        void OnSuccess(int status, const std::string &ifname, OHOS::Wifi::DhcpResult &result) override;
        void OnFailed(int status, const std::string &ifname, const std::string &reason) override;
        void OnSerExitNotify(const std::string& ifname) override;

    private:
        DhcpController &dhcpController_;
    };
public:
    DhcpController();
    ~DhcpController();

    int32_t RegisterNotifyCallback(sptr<OHOS::NetdNative::INotifyCallback> &callback);
    void StartDhcpClient(const std::string &iface, bool bIpv6);
    void StopDhcpClient(const std::string &iface, bool bIpv6);

    void Process(const std::string &iface, const std::string &ipAddr, const std::string &gateWay, const std::string &subNet,
        const std::string &route1, const std::string &route2, const std::string &dns1, const std::string &dns2);
private:
    std::unique_ptr<OHOS::Wifi::IDhcpService> dhcpService_ = nullptr;
    std::unique_ptr<DhcpControllerResultNotify> dhcpResultNotify_ = nullptr;
    sptr<OHOS::NetdNative::INotifyCallback> callback_ = nullptr;
};
} // namespace nmd
} // namespace OHOS
#endif // !__INCLUDE_DHCP_CONTROLLER_H__