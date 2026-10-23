/**
 * @file NetworkSecurityManager.cpp
 * @brief 네트워크 보안 관리자 구현
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "security/NetworkSecurityManager.h"
#include <regex>
#include <algorithm>
#include <sstream>

namespace kingsejong {
namespace security {

// 정적 멤버 초기화
std::unordered_set<std::string> NetworkSecurityManager::allowedDomains_;
std::unordered_set<std::string> NetworkSecurityManager::blockedDomains_;
bool NetworkSecurityManager::ssrfProtectionEnabled_ = true;  // 기본적으로 활성화
int NetworkSecurityManager::rateLimitPerMinute_ = 0;  // 0 = 무제한
std::unordered_map<std::string, std::vector<std::chrono::steady_clock::time_point>> NetworkSecurityManager::requestHistory_;
std::mutex NetworkSecurityManager::mutex_;

bool NetworkSecurityManager::checkHttpAccess(const std::string& url)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // 1. 빈 URL 차단
    if (url.empty())
    {
        return false;
    }

    // 2. HTTP/HTTPS 프로토콜만 허용
    if (url.find("http://") != 0 && url.find("https://") != 0)
    {
        return false;
    }

    // 3. 호스트 추출
    std::string host = extractHost(url);
    if (host.empty())
    {
        return false;
    }

    // 4. 블랙리스트 확인 (최우선)
    if (isDomainBlocked(host))
    {
        return false;
    }

    // 5. SSRF 방지 확인
    if (ssrfProtectionEnabled_)
    {
        // Private IP 차단
        if (isPrivateIP(host))
        {
            return false;
        }

        // 민감한 호스트 차단 (localhost, metadata 등)
        if (isSensitiveHost(host))
        {
            return false;
        }
    }

    // 6. Rate limiting 확인
    if (!checkRateLimit(url))
    {
        return false;
    }

    // 7. 화이트리스트 확인 (선택적, 현재는 보조적 역할)
    // 실제 화이트리스트 전용 모드가 필요하면 별도 플래그 추가

    return true;
}

void NetworkSecurityManager::allowDomain(const std::string& domain)
{
    std::lock_guard<std::mutex> lock(mutex_);
    allowedDomains_.insert(domain);
}

void NetworkSecurityManager::blockDomain(const std::string& domain)
{
    std::lock_guard<std::mutex> lock(mutex_);
    blockedDomains_.insert(domain);
}

void NetworkSecurityManager::setRateLimit(int requestsPerMinute)
{
    std::lock_guard<std::mutex> lock(mutex_);
    rateLimitPerMinute_ = requestsPerMinute;
}

void NetworkSecurityManager::setSSRFProtection(bool enabled)
{
    std::lock_guard<std::mutex> lock(mutex_);
    ssrfProtectionEnabled_ = enabled;
}

void NetworkSecurityManager::reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    allowedDomains_.clear();
    blockedDomains_.clear();
    ssrfProtectionEnabled_ = true;
    rateLimitPerMinute_ = 0;
    requestHistory_.clear();
}

std::string NetworkSecurityManager::extractHost(const std::string& url)
{
    // URL에서 호스트 추출
    // 예: http://example.com:8080/path -> example.com
    // 예: https://192.168.1.1/api -> 192.168.1.1

    std::string host;

    // 프로토콜 제거
    size_t start = 0;
    if (url.find("http://") == 0)
    {
        start = 7;  // "http://" 길이
    }
    else if (url.find("https://") == 0)
    {
        start = 8;  // "https://" 길이
    }
    else
    {
        return "";
    }

    // authority 부분 (첫 '/' 앞까지)
    size_t authorityEnd = url.find('/', start);
    if (authorityEnd == std::string::npos)
    {
        authorityEnd = url.length();
    }
    std::string authority = url.substr(start, authorityEnd - start);

    // userinfo 제거: http://user:pw@host/ 에서 host 만 남긴다.
    // 이것을 빼면 "user@127.0.0.1" 전체가 호스트로 보여 SSRF 검사를 지나친다.
    size_t at = authority.rfind('@');
    if (at != std::string::npos)
    {
        authority = authority.substr(at + 1);
    }

    if (!authority.empty() && authority[0] == '[')
    {
        // IPv6 리터럴: [::1]:8080 -> ::1
        size_t close = authority.find(']');
        if (close == std::string::npos)
        {
            return "";
        }
        host = authority.substr(1, close - 1);
    }
    else
    {
        // 포트 제거
        size_t colon = authority.find(':');
        host = (colon == std::string::npos) ? authority : authority.substr(0, colon);
    }

    // 소문자로 변환 (도메인은 대소문자 구분 없음)
    std::transform(host.begin(), host.end(), host.begin(), ::tolower);

    return host;
}

bool NetworkSecurityManager::isPrivateIP(const std::string& host)
{
    int octets[4];
    if (!parseIPv4(host, octets))
    {
        // IPv4가 아니면 도메인 이름 (private IP 아님)
        return false;
    }

    // 127.0.0.0/8 (127.0.0.0 ~ 127.255.255.255)
    if (octets[0] == 127)
    {
        return true;
    }

    // 10.0.0.0/8 (10.0.0.0 ~ 10.255.255.255)
    if (octets[0] == 10)
    {
        return true;
    }

    // 172.16.0.0/12 (172.16.0.0 ~ 172.31.255.255)
    if (octets[0] == 172 && octets[1] >= 16 && octets[1] <= 31)
    {
        return true;
    }

    // 192.168.0.0/16 (192.168.0.0 ~ 192.168.255.255)
    if (octets[0] == 192 && octets[1] == 168)
    {
        return true;
    }

    // 169.254.0.0/16 (Link-local, AWS metadata: 169.254.169.254)
    if (octets[0] == 169 && octets[1] == 254)
    {
        return true;
    }

    return false;
}

bool NetworkSecurityManager::isSensitiveHost(const std::string& host)
{
    // localhost 변형
    // IPv6 loopback/unspecified 와 IPv4 unspecified. 0.0.0.0 은 대부분의 OS 에서
    // 로컬 인터페이스로 연결되므로 127.0.0.1 과 같이 취급한다.
    if (host == "::1" || host == "::" || host == "0.0.0.0" || host == "0:0:0:0:0:0:0:1")
    {
        return true;
    }
    // IPv6 link-local(fe80::/10) 과 unique-local(fc00::/7). 문자열 접두 비교다.
    if (host.find(':') != std::string::npos
        && (host.rfind("fe80:", 0) == 0 || host.rfind("fc", 0) == 0 || host.rfind("fd", 0) == 0))
    {
        return true;
    }
    // IPv4-mapped IPv6 (::ffff:127.0.0.1) 는 안쪽 IPv4 로 다시 검사한다.
    if (host.rfind("::ffff:", 0) == 0)
    {
        return isPrivateIP(host.substr(7)) || isSensitiveHost(host.substr(7));
    }

    static const std::vector<std::string> sensitiveHosts = {
        "localhost",
        "localhost.localdomain",
        "ip6-localhost",
        "ip6-loopback",
    };

    for (const auto& sensitive : sensitiveHosts)
    {
        if (host == sensitive)
        {
            return true;
        }
    }

    // metadata 서비스 IP는 isPrivateIP에서 처리 (169.254.169.254)

    return false;
}

bool NetworkSecurityManager::isDomainBlocked(const std::string& host)
{
    for (const auto& blocked : blockedDomains_)
    {
        if (matchDomain(host, blocked))
        {
            return true;
        }
    }
    return false;
}

bool NetworkSecurityManager::isDomainAllowed(const std::string& host)
{
    for (const auto& allowed : allowedDomains_)
    {
        if (matchDomain(host, allowed))
        {
            return true;
        }
    }
    return false;
}

bool NetworkSecurityManager::checkRateLimit(const std::string& url)
{
    // Rate limit이 0이면 무제한
    if (rateLimitPerMinute_ == 0)
    {
        return true;
    }

    auto now = std::chrono::steady_clock::now();
    auto& history = requestHistory_[url];

    // 1분 이상 경과한 요청 제거
    auto oneMinuteAgo = now - std::chrono::minutes(1);
    history.erase(
        std::remove_if(history.begin(), history.end(),
            [oneMinuteAgo](const auto& time) { return time < oneMinuteAgo; }),
        history.end()
    );

    // 현재 요청 수 확인
    if (static_cast<int>(history.size()) >= rateLimitPerMinute_)
    {
        return false;  // Rate limit 초과
    }

    // 현재 요청 기록
    history.push_back(now);

    return true;
}

bool NetworkSecurityManager::parseIPv4(const std::string& ip, int octets[4])
{
    // IPv4 주소 파싱: "192.168.1.1" -> [192, 168, 1, 1]
    std::istringstream iss(ip);
    char dot;

    if (!(iss >> octets[0] >> dot >> octets[1] >> dot >> octets[2] >> dot >> octets[3]))
    {
        return false;
    }

    // 각 옥텟이 0-255 범위인지 확인
    for (int i = 0; i < 4; i++)
    {
        if (octets[i] < 0 || octets[i] > 255)
        {
            return false;
        }
    }

    // 추가 문자가 없는지 확인
    char extra;
    if (iss >> extra)
    {
        return false;
    }

    return true;
}

bool NetworkSecurityManager::matchDomain(const std::string& host, const std::string& pattern)
{
    // 와일드카드 도메인 매칭
    // 예: "*.example.com"은 "api.example.com"과 매칭되지만 "example.com"과는 매칭 안 됨

    // 정확한 매칭
    if (host == pattern)
    {
        return true;
    }

    // 와일드카드 패턴 (*.example.com)
    if (pattern.find("*.") == 0)
    {
        std::string baseDomain = pattern.substr(2);  // "*.example.com" -> "example.com"

        // host가 ".baseDomain"으로 끝나는지 확인
        if (host.length() > baseDomain.length() + 1)  // 최소 "x.baseDomain"
        {
            std::string suffix = "." + baseDomain;
            if (host.length() >= suffix.length())
            {
                std::string hostSuffix = host.substr(host.length() - suffix.length());
                if (hostSuffix == suffix)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

} // namespace security
} // namespace kingsejong
