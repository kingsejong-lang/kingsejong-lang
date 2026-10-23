/**
 * @file BuiltinSecurityTest.cpp
 * @brief 파일·디렉토리·DB·환경변수 내장 함수가 SecurityManager 를 거치는지 확인
 *
 * SANDBOX 모드에 화이트리스트가 없으면 모든 경로 접근이 거부돼야 한다. 이 테스트는
 * 각 내장 함수가 파일시스템을 건드리기 전에 검사에서 예외를 던지는지 본다.
 */

#include <gtest/gtest.h>
#include <filesystem>
#include <cstdlib>
#include "evaluator/Builtin.h"
#include "evaluator/Value.h"
#include "security/SecurityManager.h"

using namespace kingsejong;
using namespace kingsejong::evaluator;
using namespace kingsejong::security;
namespace fs = std::filesystem;

class BuiltinSecurityTest : public ::testing::Test {
protected:
    fs::path probe;

    void SetUp() override {
        Builtin::registerAllBuiltins();
        SecurityManager::reset();
        probe = fs::temp_directory_path() / "ksj-builtin-security-probe";
        fs::remove_all(probe);
        SecurityManager::setMode(SecurityMode::SANDBOX);
    }

    void TearDown() override {
        SecurityManager::reset();
        fs::remove_all(probe);
    }

    static Value call(const std::string& name, const std::vector<Value>& args) {
        return Builtin::getBuiltin(name).asBuiltinFunction()(args);
    }

    Value path() const { return Value::createString(probe.string()); }
};

TEST_F(BuiltinSecurityTest, LineReadIsDeniedInSandbox) {
    EXPECT_THROW(call("줄별_읽기", {path()}), std::runtime_error);
}

TEST_F(BuiltinSecurityTest, DirectoryCreateIsDeniedInSandbox) {
    EXPECT_THROW(call("디렉토리_생성", {path()}), std::runtime_error);
    EXPECT_FALSE(fs::exists(probe)) << "검사가 디렉토리를 만든 뒤에 돌았다";
}

TEST_F(BuiltinSecurityTest, DirectoryRemoveIsDeniedInSandbox) {
    SecurityManager::reset();
    fs::create_directories(probe / "child");
    SecurityManager::setMode(SecurityMode::SANDBOX);
    EXPECT_THROW(call("디렉토리_삭제", {path()}), std::runtime_error);
    EXPECT_TRUE(fs::exists(probe / "child")) << "검사 없이 지워졌다";
}

TEST_F(BuiltinSecurityTest, DirectoryListingIsDeniedInSandbox) {
    EXPECT_THROW(call("디렉토리_목록", {path()}), std::runtime_error);
    EXPECT_THROW(call("디렉토리_나열", {path()}), std::runtime_error);
    EXPECT_THROW(call("디렉토리_변경", {path()}), std::runtime_error);
    EXPECT_THROW(call("디렉토리인가", {path()}), std::runtime_error);
    EXPECT_THROW(call("파일인가", {path()}), std::runtime_error);
    EXPECT_THROW(call("경로_존재하는가", {path()}), std::runtime_error);
}

TEST_F(BuiltinSecurityTest, DatabaseOpenIsDeniedInSandbox) {
    EXPECT_THROW(call("DB_열기", {Value::createString((probe / "x.db").string())}), std::runtime_error);
    EXPECT_FALSE(fs::exists(probe / "x.db"));
}

TEST_F(BuiltinSecurityTest, EnvironmentWritesAreDeniedInSandbox) {
    EXPECT_THROW(call("환경변수_쓰기", {Value::createString("KSJ_SEC_TEST"), Value::createString("1")}), std::runtime_error);
    EXPECT_THROW(call("환경변수_삭제", {Value::createString("KSJ_SEC_TEST")}), std::runtime_error);
    EXPECT_EQ(std::getenv("KSJ_SEC_TEST"), nullptr);
}

TEST_F(BuiltinSecurityTest, AllowedPathWorksInSandbox) {
    SecurityManager::allowPath(fs::temp_directory_path().string());
    EXPECT_NO_THROW(call("디렉토리_생성", {path()}));
    EXPECT_TRUE(fs::exists(probe));
    EXPECT_NO_THROW(call("디렉토리_삭제", {path()}));
    EXPECT_FALSE(fs::exists(probe));
}

TEST_F(BuiltinSecurityTest, TrustedModeAllowsEnvironmentWrites) {
    SecurityManager::reset();
    EXPECT_NO_THROW(call("환경변수_쓰기", {Value::createString("KSJ_SEC_TEST"), Value::createString("1")}));
    EXPECT_NO_THROW(call("환경변수_삭제", {Value::createString("KSJ_SEC_TEST")}));
}
