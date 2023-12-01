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

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_proxy.h"
#include "net_mgr_log_wrapper.h"
#include "net_manager_constants.h"
#include "network_security_config.h"
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fstream>
#include <dirent.h>
#include <cstdio>
#include "openssl/hmac.h"
#include "openssl/evp.h"

namespace OHOS {
namespace NetManagerStandard {

const std::string TAG_NETWORK_SECURITY_CONFIG("network-security-config");
const std::string TAG_BASE_CONFIG("base-config");
const std::string TAG_DOMAIN_CONFIG("domain-config");
const std::string TAG_TRUST_ANCHORS("trust-anchors");
const std::string TAG_CERTIFICATES("certificates");
const std::string TAG_DOMAINS("domains");
const std::string TAG_INCLUDE_SUBDOMAINS("include-subdomains");
const std::string TAG_NAME("name");
const std::string TAG_PIN_SET("pin-set");
const std::string TAG_EXPIRATION("expiration");
const std::string TAG_PIN("pin");
const std::string TAG_DIGEST_ALGORITHM("digest-algorithm");
const std::string TAG_DIGEST("digest");

const std::string REHASHD_CA_CERTS_DIR("/data/storage/el2/base/haps/entry/files/rehashed_ca_certs");
#ifdef WINDOWS_PLATFORM
const char OS_PATH_SEPARATOR = '\\';
#else
const char OS_PATH_SEPARATOR = '/';
#endif


sptr<AppExecFwk::BundleMgrProxy> GetBundleMgrProxy()
{
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        NETMGR_LOG_E("fail to get system ability mgr.");
        return nullptr;
    }

    auto remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        NETMGR_LOG_E("fail to get bundle manager proxy.");
        return nullptr;
    }

    return iface_cast<AppExecFwk::BundleMgrProxy>(remoteObject);
}

bool Endswith(const std::string &str, const std::string &suffix)
{
    if (str.length() < suffix.length()) {
        return false;
    }

    if (str.rfind(suffix) == (str.length() - suffix.length())) {
        return true;
    }

    return false;
}

NetworkSecurityConfig& NetworkSecurityConfig::GetInstance()
{
    static auto instance = new NetworkSecurityConfig();
    return *instance;
}

bool NetworkSecurityConfig::IsCACertFileName(const char *fileName)
{
    std::string str;
    auto ext = strrchr(fileName, '.');
    if (ext != nullptr) {
        str = ext + 1;
    }

    for(auto &c: str) {
        c = tolower(c);
    }

    return (str == "pem" || str == "crt");
}

void NetworkSecurityConfig::GetCAFilesFromPath(const std::string caPath, std::vector<std::string> &caFiles)
{
    DIR *dir = opendir(caPath.c_str());
    if(dir == nullptr) {
        NETMGR_LOG_E("open CA path[%{public}s] fail. [%{public}d]", caPath.c_str(), errno);
        return;
    }

    struct dirent *entry = readdir(dir);
    while(entry != nullptr) {
        if (IsCACertFileName(entry->d_name)) {
            if (caPath[caPath.length() - 1] != OS_PATH_SEPARATOR) {
                caFiles.push_back(caPath + OS_PATH_SEPARATOR + entry->d_name);
            } else {
                caFiles.push_back(caPath + entry->d_name);
            }
        }
        NETMGR_LOG_D("Read CA File [%{public}s] from CA Path ", entry->d_name);
        entry = readdir(dir);
    }

    closedir(dir);
}

void NetworkSecurityConfig::AddSurfixToCACertFileName(const std::string &caPath,
    std::set<std::string> &allFileNames, std::string &caFile)
{
    uint32_t count = 0;
    for (auto &fileName: allFileNames) {
        if (fileName.find(caFile) != std::string::npos) {
            count++;
        }
    }

    caFile = caFile + '.' + std::to_string(count);
    allFileNames.insert(caFile);
}

X509 *NetworkSecurityConfig::ReadCertFile(const std::string &fileName)
{
    std::ifstream certFile(fileName.c_str());
    if(!certFile.is_open()) {
        NETMGR_LOG_E("Fail to read cert fail [%{public}s]", fileName.c_str());
        return nullptr;
    }

    certFile.seekg(0,std::ios::end);
    auto certSize = certFile.tellg();

    certFile.seekg(0, std::ios::beg);
    auto certData = new char[certSize];
    certFile.read(certData, certSize);
    certFile.close();

    BIO *bio = BIO_new_mem_buf(certData, certSize);
    if (bio == nullptr) {
        NETMGR_LOG_E("Fail to call BIO_new_mem_buf");
        delete[] certData;
        return nullptr;
    }

    X509 *x509 = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
    if (x509 == nullptr) {
        NETMGR_LOG_E("Fail to call PEM_read_bio_X509.");
    }

    BIO_free(bio);
    delete[] certData;
    return x509;
}

std::string NetworkSecurityConfig::GetRehashedCADirName(const std::string &caPath)
{
    constexpr unsigned int HASHED_DIR_NAME_HEX_LEN = 16;
    unsigned char hashedHex[HASHED_DIR_NAME_HEX_LEN] = {0};

    HMAC(EVP_sha1(), nullptr, 0, 
         reinterpret_cast<const unsigned char *>(caPath.c_str()), caPath.size(),
         hashedHex, const_cast<unsigned int *>(&HASHED_DIR_NAME_HEX_LEN));

    /* Encode the hashed string by base16 */
    constexpr unsigned int HASHED_DIR_NAME_LEN = HASHED_DIR_NAME_HEX_LEN * 2;
    unsigned char hashedStr[HASHED_DIR_NAME_LEN + 1] = {0};
    auto inBuf = hashedHex;
    auto outBuf = hashedStr;
    for (unsigned int inLen = 0;  inLen < sizeof(hashedHex); inBuf++, inLen++) {
        unsigned char base16Pair[] = {*inBuf / 16, *inBuf % 16};
        for (unsigned int i = 0; i < sizeof(base16Pair); i++) {
            if (base16Pair[i] < 10) {
                *outBuf++ = '0' + base16Pair[i];
            } else {
                *outBuf++ = 'a' + base16Pair[i] - 10;
            }
        }
    }

    std::string hashedDirName(reinterpret_cast<char *>(hashedStr));
    return hashedDirName;
}

std::string NetworkSecurityConfig::BuildRehasedCAPath(const std::string &caPath)
{
    if (access(REHASHD_CA_CERTS_DIR.c_str(), F_OK) == -1) {
        if (mkdir(REHASHD_CA_CERTS_DIR.c_str(), S_IRWXU | S_IRWXG) == -1) {
            NETMGR_LOG_E("Fail to make a rehased caCerts dir [%{public}d]", errno);
            return "";
        }
    }

    auto dirName = GetRehashedCADirName(caPath);
    auto rehashedCertpath = REHASHD_CA_CERTS_DIR + OS_PATH_SEPARATOR + dirName;
    if (access(rehashedCertpath.c_str(), F_OK) == -1) {
        if (mkdir(rehashedCertpath.c_str(), S_IRWXU | S_IRWXG) == -1) {
            NETMGR_LOG_E("Fail to make a rehased caCerts dir [%{public}d]", errno);
            return "";
        }
    }

    NETMGR_LOG_D("Build dir [%{public}s]", rehashedCertpath.c_str());
    return rehashedCertpath;
}

std::string NetworkSecurityConfig::GetRehasedCAPath(const std::string &caPath)
{
    if (access(REHASHD_CA_CERTS_DIR.c_str(), F_OK) == -1) {
        return "";
    }

    auto dirName = GetRehashedCADirName(caPath);
    if (dirName.empty()) {
        return "";
    }

    auto rehashedCertpath = REHASHD_CA_CERTS_DIR + OS_PATH_SEPARATOR + dirName;
    if (access(rehashedCertpath.c_str(), F_OK) == -1) {
        return "";
    }

    return rehashedCertpath;
}

std::string NetworkSecurityConfig::ReHashCAPathForX509(const std::string &caPath)
{
    std::set<std::string> allFiles;
    std::vector<std::string> caFiles;

    auto rehashedCertpath = BuildRehasedCAPath(caPath);
    if (rehashedCertpath.empty()) {
        return rehashedCertpath;
    }

    GetCAFilesFromPath(caPath, caFiles);

    for (auto &caFile: caFiles) {
        auto x509 = ReadCertFile(caFile);
        if (x509 == nullptr) {
            continue;
        }

        constexpr int X509_HASH_LEN = 16;
        char buf[X509_HASH_LEN] = {0};
        sprintf(buf, "%08lx", X509_subject_name_hash(x509));
        X509_free(x509);

        std::string hashName(buf);
        AddSurfixToCACertFileName(rehashedCertpath, allFiles, hashName);

        std::string rehashedCaFile = rehashedCertpath + OS_PATH_SEPARATOR + hashName;
        std::ifstream src(caFile);
        std::ofstream dst(rehashedCaFile);
        dst << src.rdbuf();
        NETMGR_LOG_D("Rehased cert generated. [%{public}s]", rehashedCaFile.c_str());
    }

    return rehashedCertpath;
}

bool NetworkSecurityConfig::ValidateDate(const std::string &dateStr)
{
    return true;
}

int32_t NetworkSecurityConfig::GetConfig()
{
    std::string json;
    if (configCached_) {
        return NETMANAGER_SUCCESS;
    }

    auto ret = GetJsonFromBundle(json);
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Failed to get json from bundler manager.");
        return ret;
    }

    ret = ParseJsonConfig(json);
    if (ret != NETMANAGER_SUCCESS) {
        return ret;
    }

    configCached_ = true;
    return NETMANAGER_SUCCESS;
}

int32_t NetworkSecurityConfig::GetJsonFromBundle(std::string &jsonProfile)
{
    sptr<AppExecFwk::BundleMgrProxy> bundleMgrProxy = GetBundleMgrProxy();
    if (bundleMgrProxy == nullptr) {
        NETMGR_LOG_E("Failed to get bundle manager proxy.");
        return NETMANAGER_ERR_INTERNAL;
    }

    AppExecFwk::BundleInfo bundleInfo;
    auto ret = bundleMgrProxy->GetBundleInfoForSelf(0, bundleInfo);
    if (ret != ERR_OK) {
        NETMGR_LOG_E("GetSelfBundleName: bundleName get fail.");
        return NETMANAGER_ERR_INTERNAL;
    }

    ret = bundleMgrProxy->GetJsonProfile(AppExecFwk::ProfileType::NETWORK_PROFILE,
        bundleInfo.name, bundleInfo.entryModuleName, jsonProfile);
    if (ret != ERR_OK) {
        NETMGR_LOG_D("No network_config profile configured in bundle manager.[%{public}d]", ret);
        return NETMANAGER_SUCCESS;
    }

    return NETMANAGER_SUCCESS;
}

void NetworkSecurityConfig::ParseJsonTrustAnchors(const Json::Value &root, TrustAnchors &trustAnchors)
{
    if (!root.isArray()) {
        return;
    }

    auto size = root.size();
    for (uint32_t i = 0; i < size; i++) {
        if (root[i].isMember(TAG_CERTIFICATES.c_str()) && root[i][TAG_CERTIFICATES.c_str()].isString()) {
            auto cert_path = root[i][TAG_CERTIFICATES.c_str()].asString();
            trustAnchors.certs_.push_back(cert_path);
        }
    }

    return;
}

void NetworkSecurityConfig::ParseJsonDomains(const Json::Value &root, std::vector<Domain> &domains)
{
    if (!root.isArray()) {
        return;
    }

    auto size = root.size();
    for (uint32_t i = 0; i < size; i++) {
        Domain domain;
        if (!root[i].isMember(TAG_NAME.c_str()) || !root[i][TAG_NAME.c_str()].isString()) {
            continue;
        }
        domain.domainName_ = root[i][TAG_NAME.c_str()].asString();

        if (root[i].isMember(TAG_INCLUDE_SUBDOMAINS.c_str()) && root[i][TAG_INCLUDE_SUBDOMAINS.c_str()].isBool()) {
            domain.includeSubDomains_ = root[i][TAG_INCLUDE_SUBDOMAINS.c_str()].asBool();
        } else {
            domain.includeSubDomains_ = true;
        }

        domains.push_back(domain);
    }

    return;
}

void NetworkSecurityConfig::ParseJsonPinSet(const Json::Value &root, PinSet &pinSet)
{
    if (root.isNull()) {
        return;
    }

    if (root.isMember(TAG_EXPIRATION.c_str()) && root[TAG_EXPIRATION.c_str()].isString()) {
        pinSet.expiration_ = root[TAG_EXPIRATION.c_str()].asString();
    }

    if (!root.isMember(TAG_PIN.c_str()) || !root[TAG_PIN.c_str()].isArray()) {
        return;
    }

    auto pinRoot = root[TAG_PIN.c_str()];
    auto size = pinRoot.size();
    for (uint32_t i = 0; i < size; i++) {
        if (pinRoot[i].isMember(TAG_DIGEST_ALGORITHM.c_str()) &&
            pinRoot[i][TAG_DIGEST_ALGORITHM.c_str()].isString() &&
            pinRoot[i].isMember(TAG_DIGEST.c_str()) &&
            pinRoot[i][TAG_DIGEST.c_str()].isString()) {
            Pin pin;
            pin.digestAlgorithm_ = pinRoot[i][TAG_DIGEST_ALGORITHM.c_str()].asString();
            pin.digest_ = pinRoot[i][TAG_DIGEST.c_str()].asString();
            pinSet.pins_.push_back(pin);
        }
    }

    return;
}

void NetworkSecurityConfig::ParseJsonBaseConfig(const Json::Value &root, BaseConfig &baseConfig)
{
    if (root.isNull()) {
        return;
    }
    ParseJsonTrustAnchors(root[TAG_TRUST_ANCHORS.c_str()], baseConfig.trustAnchors_);
}

void NetworkSecurityConfig::ParseJsonDomainConfigs(const Json::Value &root, std::vector<DomainConfig> &domainConfigs)
{
    if (!root.isArray()) {
        return;
    }

    auto size = root.size();
    for (uint32_t i = 0; i < size; i++) {
        DomainConfig domainConfig;
        ParseJsonDomains(root[i][TAG_DOMAINS.c_str()], domainConfig.domains_);
        ParseJsonTrustAnchors(root[i][TAG_TRUST_ANCHORS.c_str()], domainConfig.trustAnchors_);
        ParseJsonPinSet(root[i][TAG_PIN_SET.c_str()], domainConfig.pinSet_);

        domainConfigs.push_back(domainConfig);
    }

    return;
}

int32_t NetworkSecurityConfig::ParseJsonConfig(const std::string &content)
{
    if (content.empty()) {
        return NETMANAGER_SUCCESS;
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    JSONCPP_STRING errs;

    bool isSuccess = reader->parse(content.c_str(), content.c_str() + content.length(), &root, &errs);
    if (!isSuccess) {
        NETMGR_LOG_E("Failed to parse network json profile.");
        return NETMANAGER_ERR_INTERNAL;
    }

    if (root.isNull() || !root.isMember(TAG_NETWORK_SECURITY_CONFIG.c_str())) {
        return NETMANAGER_SUCCESS;
    }

    ParseJsonBaseConfig(root[TAG_NETWORK_SECURITY_CONFIG.c_str()][TAG_BASE_CONFIG.c_str()], baseConfig_);

    ParseJsonDomainConfigs(root[TAG_NETWORK_SECURITY_CONFIG.c_str()][TAG_DOMAIN_CONFIG.c_str()], domainConfigs_);

    return NETMANAGER_SUCCESS;
}

int32_t NetworkSecurityConfig::GetPinSetForHostName(const std::string &hostname, std::string &pins)
{
    if (hostname.empty()) {
        NETMGR_LOG_E("Failed to get pinset, hostname is empty.");
        return NETMANAGER_SUCCESS;
    }

    auto ret = GetConfig();
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Failed to get pinset config.");
        return NETMANAGER_SUCCESS;
    }

    PinSet *pPinSet = nullptr;
    for (auto &domainConfig: domainConfigs_) {
        for (auto &domain: domainConfig.domains_) {
            if (hostname == domain.domainName_) {
                pPinSet = &domainConfig.pinSet_;
                break;
            } else if (domain.includeSubDomains_ && Endswith(hostname, domain.domainName_)) {
                pPinSet = &domainConfig.pinSet_;
                break;
            }
        }
        if (pPinSet != nullptr) {
            break;
        }
    }

    if (pPinSet == nullptr) {
        return NETMANAGER_SUCCESS;
    }

    if (!ValidateDate(pPinSet->expiration_)) {
        return NETMANAGER_ERR_PERMISSION_DENIED;
    }

    std::stringstream ss;
    for (auto &pin: pPinSet->pins_) {
        ss << pin.digestAlgorithm_ << "//" << pin.digest_ << ";";
    }

    pins = ss.str();
    if (!pins.empty()) {
        pins.pop_back();
    }

    return NETMANAGER_SUCCESS;
}

int32_t NetworkSecurityConfig::GetTrustAnchorsForHostName(const std::string &hostname, std::vector<std::string> &certs)
{
    if (hostname.empty()) {
        NETMGR_LOG_E("Failed to get trust anchors, hostname is empty.");
        return NETMANAGER_SUCCESS;
    }

    auto ret = GetConfig();
    if (ret != NETMANAGER_SUCCESS) {
        NETMGR_LOG_E("Failed to get trust anchors.");
        return NETMANAGER_SUCCESS;
    }

    TrustAnchors *pTrustAnchors = nullptr;
    for (auto &domainConfig: domainConfigs_) {
        for (auto &domain: domainConfig.domains_) {
            if (hostname == domain.domainName_) {
                pTrustAnchors = &domainConfig.trustAnchors_;
                break;
            } else if (domain.includeSubDomains_ && Endswith(hostname, domain.domainName_)) {
                pTrustAnchors = &domainConfig.trustAnchors_;
                break;
            }
        }
        if (pTrustAnchors != nullptr) {
            break;
        }
    }

    if (pTrustAnchors == nullptr) {
        pTrustAnchors = &baseConfig_.trustAnchors_;
    }

    for (auto &certPath: pTrustAnchors->certs_) {
        auto rehashedCertpath = GetRehasedCAPath(certPath);
        if (rehashedCertpath.empty())
        {
            rehashedCertpath = ReHashCAPathForX509(certPath);
        }
        if (!rehashedCertpath.empty())
        {
            certs.push_back(rehashedCertpath);
        } else {
            certs.push_back(certPath);
        }
        NETMGR_LOG_D("Get cert [%{public}s] [%{public}s]", certPath.c_str(), rehashedCertpath.c_str());
    }

    return NETMANAGER_SUCCESS;
}

void NetworkSecurityConfig::DumpConfigs()
{
    NETMGR_LOG_I("DumpConfigs:baseConfig_.trustAnchors_.certs");
    for (auto &cert: baseConfig_.trustAnchors_.certs_) {
        NETMGR_LOG_I("[%{public}s]", cert.c_str());
    }

    NETMGR_LOG_I("DumpConfigs:domainConfigs_");
    for (auto &domainConfig: domainConfigs_) {
        NETMGR_LOG_I("=======================");
        for (auto &domain: domainConfig.domains_) {
            NETMGR_LOG_I("domainConfigs_.domains_[%{public}s][%{public}s]",
                         domain.domainName_.c_str(),
                         domain.includeSubDomains_ ? "include_subDomain" : "not_include_subDomain");
        }
        NETMGR_LOG_I("domainConfigs_.domains_.pinSet_.expiration_[%{public}s]",
                     domainConfig.pinSet_.expiration_.c_str());
        for (auto &pin: domainConfig.pinSet_.pins_) {
            NETMGR_LOG_I("domainConfigs_.domains_.pinSet_.pins[%{public}s][%{public}s]",
                         pin.digestAlgorithm_.c_str(), pin.digest_.c_str());
        }
        for (auto &cert: domainConfig.trustAnchors_.certs_) {
            NETMGR_LOG_I("domainConfigs_.domains_.trustAnchors_.certs_[%{public}s]", cert.c_str());
        }
    }
}

}
}
