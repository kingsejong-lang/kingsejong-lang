/**
 * @file NetworkSecurityManager.h
 * @brief 네트워크 보안 관리자 - HTTP 접근 제어 및 SSRF 방지
 * @author KingSejong Team
 * @date 2025-11-16
 */

#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <mutex>

namespace kingsejong {
namespace security {

/**
 * @class NetworkSecurityManager
 * @brief 네트워크 보안 관리자 - HTTP 요청 보안 검증
 *
 * 기능:
 * - URL 유효성 검사
 * - SSRF 공격 방지 (Private IP 차단)
 * - Rate limiting (요청 속도 제한)
 * - 허용/차단 도메인 관리
 */
class NetworkSecurityManager
{
public:
    /**
     * @brief HTTP 요청 접근 권한 확인
     * @param url 요청할 URL
     * @return 접근 허용 여부
     */
    static bool checkHttpAccess(const std::string& url);

    /**
     * @brief 도메인 화이트리스트 추가
     * @param domain 허용할 도메인
     */
    static void allowDomain(const std::string& domain);

    /**
     * @brief 도메인 블랙리스트 추가
     * @param domain 차단할 도메인
     */
    static void blockDomain(const std::string& domain);

    /**
     * @brief Rate limit 설정
     * @param requestsPerMinute 분당 최대 요청 수
     */
    static void setRateLimit(int requestsPerMinute);

    /**
     * @brief SSRF 방지 활성화/비활성화
     * @param enabled SSRF 방지 활성화 여부
     */
    static void setSSRFProtection(bool enabled);

    /**
     * @brief 보안 관리자 초기화 (모든 설정 리셋)
     */
    static void reset();

private:
    /**
     * @brief URL에서 호스트 추출
     * @param url URL 문자열
     * @return 호스트 이름 또는 IP
     */
    static std::string extractHost(const std::string& url);

    /**
     * @brief Private IP 여부 확인
     * @param host 호스트 이름 또는 IP
     * @return Private IP이면 true
     */
    static bool isPrivateIP(const std::string& host);

    /**
     * @brief 민감한 호스트 여부 확인 (localhost, metadata 서비스 등)
     * @param host 호스트 이름
     * @return 민감한 호스트이면 true
     */
    static bool isSensitiveHost(const std::string& host);

    /**
     * @brief 도메인이 블랙리스트에 있는지 확인
     * @param host 호스트 이름
     * @return 블랙리스트 포함 여부
     */
    static bool isDomainBlocked(const std::string& host);

    /**
     * @brief 도메인이 화이트리스트에 있는지 확인
     * @param host 호스트 이름
     * @return 화이트리스트 포함 여부
     */
    static bool isDomainAllowed(const std::string& host);

    /**
     * @brief Rate limit 체크
     * @param url 요청 URL
     * @return Rate limit을 초과하지 않으면 true
     */
    static bool checkRateLimit(const std::string& url);

    /**
     * @brief IPv4 주소 파싱
     * @param ip IP 주소 문자열
     * @param octets 파싱된 옥텟 배열 (크기 4)
     * @return 파싱 성공 여부
     */
    static bool parseIPv4(const std::string& ip, int octets[4]);

    /**
     * @brief 도메인 매칭 (와일드카드 지원)
     * @param host 호스트 이름
     * @param pattern 패턴 (예: "*.example.com")
     * @return 매칭 여부
     */
    static bool matchDomain(const std::string& host, const std::string& pattern);

    // 전역 상태
    static std::unordered_set<std::string> allowedDomains_;    ///< 화이트리스트
    static std::unordered_set<std::string> blockedDomains_;    ///< 블랙리스트
    static bool ssrfProtectionEnabled_;                        ///< SSRF 방지 활성화
    static int rateLimitPerMinute_;                            ///< 분당 요청 제한
    static std::unordered_map<std::string, std::vector<std::chrono::steady_clock::time_point>> requestHistory_;  ///< 요청 기록
    static std::mutex mutex_;                                  ///< 스레드 안전성
};

} // namespace security
} // namespace kingsejong
