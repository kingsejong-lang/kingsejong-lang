/**
 * @file LintDiagnosticsTest.cpp
 * @brief LSP 진단에 린터 이슈가 실리는지 확인하는 테스트
 * @author KingSejong Team
 * @date 2026-10-29
 *
 * 구문 에러가 없는 문서에서 린터 규칙 결과가 source "kingsejong-lint", code 규칙 이름으로
 * publishDiagnostics 에 덧붙는지, .ksjlint.json 과 kingsejong.lint.enable 설정을 따르는지 본다.
 */

#include <gtest/gtest.h>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>
#include "lsp/DiagnosticsProvider.h"
#include "lsp/DocumentManager.h"
#include "lsp/LanguageServer.h"
#include "lsp/LspUtils.h"
#include <nlohmann/json.hpp>

using namespace kingsejong::lsp;
namespace fs = std::filesystem;

namespace {

/// 진단 목록 중 린터가 낸 것만
std::vector<DiagnosticsProvider::Diagnostic> lintOnly(
    const std::vector<DiagnosticsProvider::Diagnostic>& diagnostics)
{
    std::vector<DiagnosticsProvider::Diagnostic> out;
    for (const auto& d : diagnostics) {
        if (d.source == "kingsejong-lint") out.push_back(d);
    }
    return out;
}

/// 경로를 file:// URI 로 (한글 등 ASCII 밖의 바이트는 퍼센트 인코딩)
std::string toFileUri(const std::string& path)
{
    static const char* hex = "0123456789ABCDEF";
    std::string uri = "file://";
    for (unsigned char c : path) {
        if (std::isalnum(c) || c == '/' || c == '-' || c == '_' || c == '.') {
            uri.push_back(static_cast<char>(c));
        } else {
            uri.push_back('%');
            uri.push_back(hex[c >> 4]);
            uri.push_back(hex[c & 0xF]);
        }
    }
    return uri;
}

} // namespace

/**
 * @class LintDiagnosticsTest
 * @brief 임시 디렉터리(이름에 한글 포함)에 문서를 두고 진단을 계산하는 픽스처
 */
class LintDiagnosticsTest : public ::testing::Test {
protected:
    void SetUp() override {
        const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
        dir = fs::temp_directory_path() / (std::string("ksj_lsp_lint_") + info->name()) / "린트";
        fs::remove_all(dir.parent_path());
        fs::create_directories(dir);
        uri = toFileUri((dir / "예제.ksj").string());
        provider.setLintEnabled(true);
    }

    void TearDown() override {
        std::error_code ec;
        fs::remove_all(dir.parent_path(), ec);
    }

    void writeConfig(const std::string& json) {
        std::ofstream(dir / ".ksjlint.json") << json;
    }

    std::vector<DiagnosticsProvider::Diagnostic> diagnose(const std::string& text) {
        DocumentManager::Document doc(uri, text, 1);
        return provider.provideDiagnostics(doc);
    }

    fs::path dir;
    std::string uri;
    DiagnosticsProvider provider;
};

/**
 * @test 쓰지 않는 변수는 warning 진단이 되고, 밑줄은 변수 이름(UTF-16 단위)에 그어진다
 */
TEST_F(LintDiagnosticsTest, ShouldReportUnusedVariableAsWarning) {
    auto diagnostics = diagnose("출력(1)\n    정수 값 = 2\n");

    ASSERT_EQ(diagnostics.size(), 1u);
    const auto& d = diagnostics[0];
    EXPECT_EQ(d.source, "kingsejong-lint");
    EXPECT_EQ(d.code, "unused-variable");
    EXPECT_EQ(d.severity, DiagnosticsProvider::DiagnosticSeverity::Warning);
    // 린터는 선언 위치 2:5 와 이름 "값" 을 보고한다 -> LSP 0-based 1:7, "값" 한 글자
    EXPECT_EQ(d.startLine, 1);
    EXPECT_EQ(d.startCharacter, 7);
    EXPECT_EQ(d.endLine, 1);
    EXPECT_EQ(d.endCharacter, 8);
    EXPECT_NE(d.message.find("값"), std::string::npos);
}

/**
 * @test 쓰지 않는 매개변수는 함수 선언 첫머리가 아니라 그 매개변수 이름에 밑줄을 긋는다
 */
TEST_F(LintDiagnosticsTest, ShouldUnderlineUnusedParameterName) {
    auto diagnostics = lintOnly(diagnose("함수 더하기(a, b) {\n    반환 a\n}\n출력(더하기(1, 2))\n"));

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].code, "no-unused-parameter");
    EXPECT_EQ(diagnostics[0].startLine, 0);
    EXPECT_EQ(diagnostics[0].startCharacter, 10);
    EXPECT_EQ(diagnostics[0].endCharacter, 11);
}

/**
 * @test naming-convention 은 선언 줄의 변수 이름을 가리킨다 (전에는 줄 정보가 없어 첫 줄에 떴다)
 */
TEST_F(LintDiagnosticsTest, ShouldPlaceNamingConventionOnDeclaration) {
    auto diagnostics = lintOnly(diagnose("출력(1)\n정수 MyVar = 1\n출력(MyVar)\n"));

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].code, "naming-convention");
    EXPECT_EQ(diagnostics[0].startLine, 1);
    EXPECT_EQ(diagnostics[0].startCharacter, 3);
    EXPECT_EQ(diagnostics[0].endCharacter, 8);
}

/**
 * @test 이름 뒤에 조사가 붙어 있어도(`i가`) 이름 부분에만 밑줄을 긋는다
 */
TEST_F(LintDiagnosticsTest, ShouldUnderlineNameBeforeParticle) {
    auto diagnostics = lintOnly(diagnose("정수 i = 0\n출력(i)\ni가 1부터 3까지 {\n    출력(i)\n}\n"));

    const DiagnosticsProvider::Diagnostic* shadow = nullptr;
    for (const auto& d : diagnostics) {
        if (d.code == "no-shadowing") shadow = &d;
    }
    ASSERT_NE(shadow, nullptr);
    EXPECT_EQ(shadow->startLine, 2);
    EXPECT_EQ(shadow->startCharacter, 0);
    EXPECT_EQ(shadow->endCharacter, 1);
}

/**
 * @test 문서 디렉터리의 .ksjlint.json 이 규칙을 끄면 그 진단이 사라진다
 */
TEST_F(LintDiagnosticsTest, ShouldRespectConfigThatDisablesRule) {
    writeConfig(R"({"rules": {"unused-variable": "off"}})");

    EXPECT_TRUE(lintOnly(diagnose("정수 x = 1\n")).empty());
}

/**
 * @test .ksjlint.json 은 상위 디렉터리에서도 찾고, 심각도 덮어쓰기를 따른다
 */
TEST_F(LintDiagnosticsTest, ShouldFindConfigInParentDirectoryAndOverrideSeverity) {
    std::ofstream(dir.parent_path() / ".ksjlint.json") << R"({"rules": {"unused-variable": "error"}})";

    auto diagnostics = lintOnly(diagnose("정수 x = 1\n"));
    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].severity, DiagnosticsProvider::DiagnosticSeverity::Error);
}

/**
 * @test 린터를 끄면 린터 진단이 없다
 */
TEST_F(LintDiagnosticsTest, ShouldNotLintWhenDisabled) {
    provider.setLintEnabled(false);

    EXPECT_TRUE(diagnose("정수 x = 1\n").empty());
}

/**
 * @test 구문 에러가 있으면 구문 진단만 있고 린터 진단은 없다
 */
TEST_F(LintDiagnosticsTest, ShouldNotLintDocumentWithParseErrors) {
    auto diagnostics = diagnose("정수 x = 1\n정수 y = @\n");

    ASSERT_FALSE(diagnostics.empty());
    EXPECT_TRUE(lintOnly(diagnostics).empty());
    EXPECT_EQ(diagnostics[0].source, "kingsejong");
}

// ============================================================================
// LanguageServer: publishDiagnostics JSON 과 설정
// ============================================================================

namespace {

nlohmann::json initializeRequest(const nlohmann::json& initializationOptions)
{
    nlohmann::json params = {{"capabilities", nlohmann::json::object()}};
    if (!initializationOptions.is_null()) {
        params["initializationOptions"] = initializationOptions;
    }
    return {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "initialize"}, {"params", params}};
}

nlohmann::json didOpen(const std::string& uri, const std::string& text)
{
    return {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", {{"textDocument", {
            {"uri", uri}, {"languageId", "kingsejong"}, {"version", 1}, {"text", text}
        }}}}
    };
}

} // namespace

/**
 * @test 서버는 기본으로 린터 진단을 보내며, code 와 source 를 JSON 에 싣는다
 */
TEST(LintLanguageServerTest, ShouldPublishLintDiagnosticWithCodeAndSource) {
    LanguageServer server;
    server.handleRequest(initializeRequest(nullptr));
    server.handleRequest(didOpen("file:///lint_default.ksj", "정수 x = 1\n"));

    auto notifications = server.takePendingNotifications();
    ASSERT_EQ(notifications.size(), 1u);
    auto diagnostics = notifications[0]["params"]["diagnostics"];
    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0]["source"], "kingsejong-lint");
    EXPECT_EQ(diagnostics[0]["code"], "unused-variable");
    EXPECT_EQ(diagnostics[0]["severity"], 2);
    EXPECT_EQ(diagnostics[0]["range"]["start"]["line"], 0);
    EXPECT_EQ(diagnostics[0]["range"]["start"]["character"], 3);
    EXPECT_EQ(diagnostics[0]["range"]["end"]["character"], 4);
}

/**
 * @test initializationOptions.lint.enable 이 false 면 린터 진단이 없다
 */
TEST(LintLanguageServerTest, ShouldNotLintWhenDisabledByInitializationOptions) {
    LanguageServer server;
    server.handleRequest(initializeRequest({{"lint", {{"enable", false}}}}));
    server.handleRequest(didOpen("file:///lint_off.ksj", "정수 x = 1\n"));

    auto notifications = server.takePendingNotifications();
    ASSERT_EQ(notifications.size(), 1u);
    EXPECT_TRUE(notifications[0]["params"]["diagnostics"].empty());
}

/**
 * @test 설정 변경으로 린터를 끄고 켜면 열린 문서의 진단을 다시 보낸다
 */
TEST(LintLanguageServerTest, ShouldRepublishOnConfigurationChange) {
    LanguageServer server;
    server.handleRequest(initializeRequest(nullptr));
    server.handleRequest(didOpen("file:///lint_toggle.ksj", "정수 x = 1\n"));
    ASSERT_EQ(server.takePendingNotifications()[0]["params"]["diagnostics"].size(), 1u);

    auto change = [](bool enable) {
        return nlohmann::json{
            {"jsonrpc", "2.0"},
            {"method", "workspace/didChangeConfiguration"},
            {"params", {{"settings", {{"kingsejong", {{"lint", {{"enable", enable}}}}}}}}}
        };
    };

    server.handleRequest(change(false));
    auto off = server.takePendingNotifications();
    ASSERT_EQ(off.size(), 1u);
    EXPECT_EQ(off[0]["params"]["uri"], "file:///lint_toggle.ksj");
    EXPECT_TRUE(off[0]["params"]["diagnostics"].empty());

    // 값이 그대로면 다시 보내지 않는다
    server.handleRequest(change(false));
    EXPECT_TRUE(server.takePendingNotifications().empty());

    server.handleRequest(change(true));
    auto on = server.takePendingNotifications();
    ASSERT_EQ(on.size(), 1u);
    EXPECT_EQ(on[0]["params"]["diagnostics"].size(), 1u);
}

// ============================================================================
// uriToFilePath
// ============================================================================

TEST(UriToFilePathTest, ShouldDecodePercentEncodedPath) {
    EXPECT_EQ(uriToFilePath("file:///home/%EC%84%B8%EC%A2%85/a%20b.ksj"), "/home/세종/a b.ksj");
}

TEST(UriToFilePathTest, ShouldStripSlashBeforeWindowsDrive) {
    EXPECT_EQ(uriToFilePath("file:///c%3A/work/a.ksj"), "c:/work/a.ksj");
}

TEST(UriToFilePathTest, ShouldReturnEmptyForNonFileScheme) {
    EXPECT_EQ(uriToFilePath("untitled:Untitled-1"), "");
}
