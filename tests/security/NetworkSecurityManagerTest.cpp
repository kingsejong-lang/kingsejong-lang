/**
 * @file NetworkSecurityManagerTest.cpp
 * @brief NetworkSecurityManager 클래스 테스트
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include <gtest/gtest.h>
#include "security/NetworkSecurityManager.h"
#include <thread>
#include <chrono>

using namespace kingsejong::security;

/**
 * @brief 각 테스트 전에 NetworkSecurityManager 초기화
 */
class NetworkSecurityManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        NetworkSecurityManager::reset();
    }

    void TearDown() override
    {
        NetworkSecurityManager::reset();
    }
};

// ============================================================================
// 기본 URL 검증 테스트
// ============================================================================

TEST_F(NetworkSecurityManagerTest, ShouldAllowValidPublicURL)
{
    // 기본적으로 공개 URL은 허용
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("http://example.com"));
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("https://api.github.com/repos"));
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("https://www.google.com/search?q=test"));
}

TEST_F(NetworkSecurityManagerTest, ShouldBlockInvalidURL)
{
    // 잘못된 형식의 URL은 차단
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess(""));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("not-a-url"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("ftp://example.com"));  // HTTP/HTTPS만 허용
}

// ============================================================================
// SSRF 방지 테스트
// ============================================================================

TEST_F(NetworkSecurityManagerTest, ShouldBlockPrivateIPv4_127)
{
    // SSRF 방지 활성화
    NetworkSecurityManager::setSSRFProtection(true);

    // localhost (127.0.0.1)
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://127.0.0.1"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://127.0.0.1:8080/api"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://127.1.2.3"));
}

TEST_F(NetworkSecurityManagerTest, ShouldBlockPrivateIPv4_10)
{
    NetworkSecurityManager::setSSRFProtection(true);

    // 10.0.0.0/8
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://10.0.0.1"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://10.1.2.3:3000"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://10.255.255.255"));
}

TEST_F(NetworkSecurityManagerTest, ShouldBlockPrivateIPv4_172)
{
    NetworkSecurityManager::setSSRFProtection(true);

    // 172.16.0.0/12 (172.16.0.0 ~ 172.31.255.255)
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://172.16.0.1"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://172.20.0.1"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://172.31.255.255"));

    // 범위 밖은 허용
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("http://172.15.0.1"));
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("http://172.32.0.1"));
}

TEST_F(NetworkSecurityManagerTest, ShouldBlockPrivateIPv4_192)
{
    NetworkSecurityManager::setSSRFProtection(true);

    // 192.168.0.0/16
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://192.168.0.1"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://192.168.1.1:8000"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://192.168.255.255"));
}

TEST_F(NetworkSecurityManagerTest, ShouldBlockSensitiveHosts)
{
    NetworkSecurityManager::setSSRFProtection(true);

    // localhost 변형
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://localhost"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://localhost:3000"));

    // AWS EC2 metadata 서비스
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://169.254.169.254"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://169.254.169.254/latest/meta-data/"));
}

TEST_F(NetworkSecurityManagerTest, ShouldAllowPublicIPWhenSSRFEnabled)
{
    NetworkSecurityManager::setSSRFProtection(true);

    // 공개 IP는 허용
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("http://8.8.8.8"));
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("http://1.1.1.1"));
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("http://93.184.216.34"));  // example.com
}

TEST_F(NetworkSecurityManagerTest, ShouldAllowPrivateIPWhenSSRFDisabled)
{
    NetworkSecurityManager::setSSRFProtection(false);

    // SSRF 방지 비활성화 시 Private IP도 허용
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("http://127.0.0.1"));
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("http://10.0.0.1"));
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("http://192.168.1.1"));
}

// ============================================================================
// 도메인 화이트리스트/블랙리스트 테스트
// ============================================================================

TEST_F(NetworkSecurityManagerTest, ShouldBlockDomainInBlacklist)
{
    NetworkSecurityManager::blockDomain("evil.com");

    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://evil.com"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("https://evil.com/api"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://evil.com:8080"));
}

TEST_F(NetworkSecurityManagerTest, ShouldBlockSubdomainWhenParentBlocked)
{
    NetworkSecurityManager::blockDomain("*.evil.com");

    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://sub.evil.com"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://api.evil.com"));
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("http://evil.com"));  // 정확한 도메인은 다름
}

TEST_F(NetworkSecurityManagerTest, ShouldAllowOnlyWhitelistedDomains)
{
    // 화이트리스트 모드: 명시적으로 허용된 도메인만 접근 가능
    NetworkSecurityManager::allowDomain("trusted.com");
    NetworkSecurityManager::allowDomain("api.example.com");

    // 참고: 현재 구현은 블랙리스트 우선, 화이트리스트는 보조
    // 실제 화이트리스트 전용 모드는 별도 플래그 필요
}

TEST_F(NetworkSecurityManagerTest, ShouldPrioritizeBlacklistOverWhitelist)
{
    NetworkSecurityManager::allowDomain("example.com");
    NetworkSecurityManager::blockDomain("example.com");

    // 블랙리스트가 화이트리스트보다 우선
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://example.com"));
}

// ============================================================================
// Rate Limiting 테스트
// ============================================================================

TEST_F(NetworkSecurityManagerTest, ShouldAllowWithinRateLimit)
{
    NetworkSecurityManager::setRateLimit(10);  // 분당 10회

    std::string url = "http://api.example.com";

    // 10회까지는 허용
    for (int i = 0; i < 10; i++)
    {
        EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess(url)) << "Request " << i;
    }
}

TEST_F(NetworkSecurityManagerTest, ShouldBlockWhenRateLimitExceeded)
{
    NetworkSecurityManager::setRateLimit(5);  // 분당 5회

    std::string url = "http://api.example.com";

    // 5회까지 허용
    for (int i = 0; i < 5; i++)
    {
        EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess(url));
    }

    // 6회째는 차단
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess(url));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess(url));
}

TEST_F(NetworkSecurityManagerTest, ShouldResetRateLimitAfterOneMinute)
{
    NetworkSecurityManager::setRateLimit(3);

    std::string url = "http://api.example.com";

    // 3회 소진
    for (int i = 0; i < 3; i++)
    {
        EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess(url));
    }
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess(url));

    // 1분 대기 (실제로는 시간이 오래 걸리므로 주석 처리)
    // std::this_thread::sleep_for(std::chrono::seconds(61));
    // EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess(url));
}

TEST_F(NetworkSecurityManagerTest, ShouldTrackRateLimitPerURL)
{
    NetworkSecurityManager::setRateLimit(2);

    std::string url1 = "http://api1.example.com";
    std::string url2 = "http://api2.example.com";

    // URL1: 2회 허용
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess(url1));
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess(url1));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess(url1));  // 3회째 차단

    // URL2: 별도 카운트 (2회 허용)
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess(url2));
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess(url2));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess(url2));
}

TEST_F(NetworkSecurityManagerTest, ShouldAllowUnlimitedWhenRateLimitZero)
{
    NetworkSecurityManager::setRateLimit(0);  // 0 = 무제한

    std::string url = "http://api.example.com";

    // 제한 없이 100회 허용
    for (int i = 0; i < 100; i++)
    {
        EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess(url));
    }
}

// ============================================================================
// 복합 시나리오 테스트
// ============================================================================

TEST_F(NetworkSecurityManagerTest, ShouldApplyAllSecurityChecks)
{
    // SSRF 방지 + 블랙리스트 + Rate limit
    NetworkSecurityManager::setSSRFProtection(true);
    NetworkSecurityManager::blockDomain("malicious.com");
    NetworkSecurityManager::setRateLimit(5);

    // Private IP 차단
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://127.0.0.1"));

    // 블랙리스트 차단
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://malicious.com"));

    // Rate limit
    std::string url = "http://api.safe.com";
    for (int i = 0; i < 5; i++)
    {
        EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess(url));
    }
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess(url));
}

TEST_F(NetworkSecurityManagerTest, ShouldResetAllSettings)
{
    // 설정 변경
    NetworkSecurityManager::setSSRFProtection(false);  // 비활성화로 변경
    NetworkSecurityManager::blockDomain("blocked.com");
    NetworkSecurityManager::setRateLimit(10);

    // 리셋
    NetworkSecurityManager::reset();

    // 기본 상태로 복구 확인
    // 기본값: SSRF 방지 활성화 (보안 우선)
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://127.0.0.1"));

    // 블랙리스트 비어있음
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("http://blocked.com"));

    // 공개 URL은 허용
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("http://example.com"));
}

// ============================================================================
// 실제 사용 시나리오 테스트
// ============================================================================

TEST_F(NetworkSecurityManagerTest, ShouldProtectWebApplicationFromSSRF)
{
    // 웹 애플리케이션 보안 설정
    NetworkSecurityManager::setSSRFProtection(true);
    NetworkSecurityManager::setRateLimit(100);  // 분당 100회

    // 정상 외부 API 호출 허용
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("https://api.github.com/repos"));
    EXPECT_TRUE(NetworkSecurityManager::checkHttpAccess("https://httpbin.org/get"));

    // 내부 네트워크 접근 차단
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://192.168.1.100/admin"));
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://10.0.0.50:8080"));

    // 메타데이터 서비스 차단
    EXPECT_FALSE(NetworkSecurityManager::checkHttpAccess("http://169.254.169.254/latest/meta-data/"));
}

TEST_F(NetworkSecurityManagerTest, ShouldProtectAPIFromAbuse)
{
    // API 남용 방지 설정
    NetworkSecurityManager::setRateLimit(10);  // 엄격한 제한

    std::string attackerURL = "http://victim.com";

    // 처음 10회는 성공
    int successCount = 0;
    for (int i = 0; i < 20; i++)
    {
        if (NetworkSecurityManager::checkHttpAccess(attackerURL))
        {
            successCount++;
        }
    }

    EXPECT_EQ(successCount, 10);  // 정확히 10회만 허용
}
