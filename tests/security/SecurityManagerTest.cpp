/**
 * @file SecurityManagerTest.cpp
 * @brief SecurityManager 테스트
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include <gtest/gtest.h>
#include "security/SecurityManager.h"
#include <filesystem>

using namespace kingsejong::security;
namespace fs = std::filesystem;

class SecurityManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 각 테스트 전에 SecurityManager 리셋
        SecurityManager::reset();
    }

    void TearDown() override {
        // 테스트 후 정리
        SecurityManager::reset();
    }
};

// ============================================================================
// 기본 모드 테스트
// ============================================================================

TEST_F(SecurityManagerTest, ShouldDefaultToTrustedMode) {
    EXPECT_EQ(SecurityManager::getMode(), SecurityMode::TRUSTED);
}

TEST_F(SecurityManagerTest, ShouldSetSecurityMode) {
    SecurityManager::setMode(SecurityMode::SANDBOX);
    EXPECT_EQ(SecurityManager::getMode(), SecurityMode::SANDBOX);

    SecurityManager::setMode(SecurityMode::UNTRUSTED);
    EXPECT_EQ(SecurityManager::getMode(), SecurityMode::UNTRUSTED);

    SecurityManager::setMode(SecurityMode::TRUSTED);
    EXPECT_EQ(SecurityManager::getMode(), SecurityMode::TRUSTED);
}

// ============================================================================
// TRUSTED 모드 테스트
// ============================================================================

TEST_F(SecurityManagerTest, TrustedModeShouldAllowAllFileAccess) {
    SecurityManager::setMode(SecurityMode::TRUSTED);

    // 어떤 파일 경로든 허용되어야 함
    EXPECT_TRUE(SecurityManager::checkFileAccess("/etc/passwd", FileOperation::READ));
    EXPECT_TRUE(SecurityManager::checkFileAccess("/tmp/test.txt", FileOperation::WRITE));
    EXPECT_TRUE(SecurityManager::checkFileAccess("/home/user/secret.dat", FileOperation::DELETE));
}

// ============================================================================
// SANDBOX 모드 - 기본 경로 제한
// ============================================================================

TEST_F(SecurityManagerTest, SandboxModeShouldDenyUnauthorizedAccess) {
    SecurityManager::setMode(SecurityMode::SANDBOX);

    // 허용되지 않은 경로는 거부되어야 함
    EXPECT_FALSE(SecurityManager::checkFileAccess("/etc/passwd", FileOperation::READ));
    EXPECT_FALSE(SecurityManager::checkFileAccess("/home/user/file.txt", FileOperation::WRITE));
}

TEST_F(SecurityManagerTest, SandboxModeShouldAllowWhitelistedPath) {
    SecurityManager::setMode(SecurityMode::SANDBOX);
    SecurityManager::allowPath("/tmp/safe/");

    // 화이트리스트에 있는 경로는 허용
    EXPECT_TRUE(SecurityManager::checkFileAccess("/tmp/safe/test.txt", FileOperation::READ));
    EXPECT_TRUE(SecurityManager::checkFileAccess("/tmp/safe/data.json", FileOperation::WRITE));

    // 화이트리스트 외부는 거부
    EXPECT_FALSE(SecurityManager::checkFileAccess("/tmp/other/file.txt", FileOperation::READ));
    EXPECT_FALSE(SecurityManager::checkFileAccess("/etc/passwd", FileOperation::READ));
}

TEST_F(SecurityManagerTest, ShouldAllowMultipleWhitelistedPaths) {
    SecurityManager::setMode(SecurityMode::SANDBOX);
    SecurityManager::allowPath("/tmp/safe/");
    SecurityManager::allowPath("/home/user/allowed/");

    EXPECT_TRUE(SecurityManager::checkFileAccess("/tmp/safe/file1.txt", FileOperation::READ));
    EXPECT_TRUE(SecurityManager::checkFileAccess("/home/user/allowed/file2.txt", FileOperation::WRITE));

    // 허용되지 않은 경로는 여전히 거부
    EXPECT_FALSE(SecurityManager::checkFileAccess("/home/user/other/file.txt", FileOperation::READ));
}

// ============================================================================
// 블랙리스트 테스트
// ============================================================================

TEST_F(SecurityManagerTest, ShouldBlockBlacklistedPath) {
    SecurityManager::setMode(SecurityMode::TRUSTED);
    SecurityManager::blockPath("/etc/");

    // 블랙리스트 경로는 거부
    EXPECT_FALSE(SecurityManager::checkFileAccess("/etc/passwd", FileOperation::READ));
    EXPECT_FALSE(SecurityManager::checkFileAccess("/etc/shadow", FileOperation::READ));

    // 블랙리스트 외부는 허용
    EXPECT_TRUE(SecurityManager::checkFileAccess("/tmp/file.txt", FileOperation::WRITE));
}

TEST_F(SecurityManagerTest, BlacklistShouldOverrideWhitelist) {
    SecurityManager::setMode(SecurityMode::SANDBOX);
    SecurityManager::allowPath("/tmp/");
    SecurityManager::blockPath("/tmp/dangerous/");

    // 화이트리스트에 있지만 블랙리스트에도 있으면 거부
    EXPECT_FALSE(SecurityManager::checkFileAccess("/tmp/dangerous/bad.txt", FileOperation::READ));

    // 화이트리스트에만 있으면 허용
    EXPECT_TRUE(SecurityManager::checkFileAccess("/tmp/safe.txt", FileOperation::WRITE));
}

// ============================================================================
// 파일 작업 타입별 제한
// ============================================================================

TEST_F(SecurityManagerTest, ShouldAllowReadOnlyInReadOnlyMode) {
    SecurityManager::setMode(SecurityMode::SANDBOX);
    SecurityManager::allowPath("/tmp/data/");
    SecurityManager::setReadOnly("/tmp/data/");

    // READ는 허용
    EXPECT_TRUE(SecurityManager::checkFileAccess("/tmp/data/file.txt", FileOperation::READ));

    // WRITE, DELETE는 거부
    EXPECT_FALSE(SecurityManager::checkFileAccess("/tmp/data/file.txt", FileOperation::WRITE));
    EXPECT_FALSE(SecurityManager::checkFileAccess("/tmp/data/file.txt", FileOperation::DELETE));
}

// ============================================================================
// 경로 정규화 테스트
// ============================================================================

TEST_F(SecurityManagerTest, ShouldNormalizePathsWithDotDot) {
    SecurityManager::setMode(SecurityMode::SANDBOX);
    SecurityManager::allowPath("/tmp/safe/");

    // Path traversal 시도 방어
    EXPECT_FALSE(SecurityManager::checkFileAccess("/tmp/safe/../etc/passwd", FileOperation::READ));
    EXPECT_FALSE(SecurityManager::checkFileAccess("/tmp/safe/../../etc/passwd", FileOperation::READ));
}

TEST_F(SecurityManagerTest, ShouldNormalizeRelativePaths) {
    SecurityManager::setMode(SecurityMode::SANDBOX);

    // 현재 작업 디렉토리를 허용
    std::string cwd = fs::current_path().string();
    SecurityManager::allowPath(cwd + "/");

    // 상대 경로도 정규화되어야 함
    EXPECT_TRUE(SecurityManager::checkFileAccess("./test.txt", FileOperation::READ));
    EXPECT_TRUE(SecurityManager::checkFileAccess("subdir/file.txt", FileOperation::READ));
}

// ============================================================================
// UNTRUSTED 모드 테스트
// ============================================================================

TEST_F(SecurityManagerTest, UntrustedModeShouldBlockSensitivePaths) {
    SecurityManager::setMode(SecurityMode::UNTRUSTED);

    // 민감한 경로는 거부
    EXPECT_FALSE(SecurityManager::checkFileAccess("/etc/passwd", FileOperation::READ));
    EXPECT_FALSE(SecurityManager::checkFileAccess("/etc/shadow", FileOperation::READ));
    EXPECT_FALSE(SecurityManager::checkFileAccess("/root/file.txt", FileOperation::READ));

    // /tmp는 허용
    EXPECT_TRUE(SecurityManager::checkFileAccess("/tmp/file.txt", FileOperation::READ));
    EXPECT_TRUE(SecurityManager::checkFileAccess("/tmp/file.txt", FileOperation::WRITE));
}

// ============================================================================
// 리셋 테스트
// ============================================================================

TEST_F(SecurityManagerTest, ResetShouldClearAllSettings) {
    SecurityManager::setMode(SecurityMode::SANDBOX);
    SecurityManager::allowPath("/tmp/safe/");
    SecurityManager::blockPath("/etc/");

    SecurityManager::reset();

    // TRUSTED 모드로 돌아가야 함
    EXPECT_EQ(SecurityManager::getMode(), SecurityMode::TRUSTED);

    // 모든 경로가 허용되어야 함
    EXPECT_TRUE(SecurityManager::checkFileAccess("/etc/passwd", FileOperation::READ));
    EXPECT_TRUE(SecurityManager::checkFileAccess("/tmp/file.txt", FileOperation::WRITE));
}

// ============================================================================
// 에러 케이스
// ============================================================================

TEST_F(SecurityManagerTest, ShouldHandleEmptyPath) {
    SecurityManager::setMode(SecurityMode::SANDBOX);

    EXPECT_FALSE(SecurityManager::checkFileAccess("", FileOperation::READ));
}

TEST_F(SecurityManagerTest, ShouldHandleInvalidPath) {
    SecurityManager::setMode(SecurityMode::SANDBOX);

    // 잘못된 경로도 안전하게 처리
    EXPECT_NO_THROW(SecurityManager::checkFileAccess("/invalid\0path", FileOperation::READ));
}

// ============================================================================
// 실제 사용 시나리오
// ============================================================================

TEST_F(SecurityManagerTest, ScenarioWebApplicationSandbox) {
    // 웹 애플리케이션 시나리오: /var/www/uploads만 허용
    SecurityManager::setMode(SecurityMode::SANDBOX);
    SecurityManager::allowPath("/var/www/uploads/");

    // 업로드 디렉토리는 허용
    EXPECT_TRUE(SecurityManager::checkFileAccess("/var/www/uploads/image.jpg", FileOperation::WRITE));

    // 시스템 파일은 거부
    EXPECT_FALSE(SecurityManager::checkFileAccess("/etc/passwd", FileOperation::READ));
    EXPECT_FALSE(SecurityManager::checkFileAccess("/var/www/html/index.php", FileOperation::WRITE));
}

TEST_F(SecurityManagerTest, ScenarioDataProcessingPipeline) {
    // 데이터 처리 파이프라인: 읽기만 허용, 특정 출력 디렉토리만 쓰기 허용
    SecurityManager::setMode(SecurityMode::SANDBOX);
    SecurityManager::allowPath("/data/input/");
    SecurityManager::setReadOnly("/data/input/");
    SecurityManager::allowPath("/data/output/");

    // 입력 데이터 읽기 허용
    EXPECT_TRUE(SecurityManager::checkFileAccess("/data/input/data.csv", FileOperation::READ));

    // 입력 데이터 쓰기 거부
    EXPECT_FALSE(SecurityManager::checkFileAccess("/data/input/data.csv", FileOperation::WRITE));

    // 출력 데이터 쓰기 허용
    EXPECT_TRUE(SecurityManager::checkFileAccess("/data/output/result.json", FileOperation::WRITE));
}
