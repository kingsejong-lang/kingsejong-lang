/**
 * @file GoToDefinitionTest.cpp
 * @brief Go to Definition 기능 테스트
 * @author KingSejong Team
 * @date 2025-11-13
 *
 * textDocument/definition 요청 처리 테스트
 */

#include <gtest/gtest.h>
#include "lsp/LanguageServer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace kingsejong::lsp;

class GoToDefinitionTest : public ::testing::Test
{
protected:
    LanguageServer server;

    void SetUp() override
    {
        // 서버 초기화
        json initRequest = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "initialize"},
            {"params", {}}
        };
        server.handleRequest(initRequest);

        json initializedNotif = {
            {"jsonrpc", "2.0"},
            {"method", "initialized"},
            {"params", {}}
        };
        server.handleRequest(initializedNotif);
    }

    /**
     * @brief 문서 열기 헬퍼
     */
    void openDocument(const std::string& uri, const std::string& content)
    {
        json openNotif = {
            {"jsonrpc", "2.0"},
            {"method", "textDocument/didOpen"},
            {"params", {
                {"textDocument", {
                    {"uri", uri},
                    {"languageId", "kingsejong"},
                    {"version", 1},
                    {"text", content}
                }}
            }}
        };
        server.handleRequest(openNotif);
    }

    /**
     * @brief definition 요청 헬퍼
     */
    json requestDefinition(const std::string& uri, int line, int character)
    {
        json defRequest = {
            {"jsonrpc", "2.0"},
            {"id", 100},
            {"method", "textDocument/definition"},
            {"params", {
                {"textDocument", {{"uri", uri}}},
                {"position", {
                    {"line", line},
                    {"character", character}
                }}
            }}
        };
        return server.handleRequest(defRequest);
    }
};

// ============================================================================
// 변수 정의로 이동 테스트
// ============================================================================

TEST_F(GoToDefinitionTest, ShouldFindVariableDefinition)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content =
        "정수 x = 10\n"
        "정수 y = x + 5";  // x 참조 (line 1, char 7)

    openDocument(uri, content);

    // Act - x 참조 위치에서 정의 요청
    // "정수 y = x + 5"에서 UTF-16 character 7이 'x'
    json response = requestDefinition(uri, 1, 7);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_TRUE(result.is_object());
    EXPECT_EQ(result["uri"], uri);
    EXPECT_EQ(result["range"]["start"]["line"], 0);  // 정의는 line 0
}

TEST_F(GoToDefinitionTest, ShouldFindVariableDefinitionAtDeclaration)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "정수 x = 10";

    openDocument(uri, content);

    // Act - 정의 위치에서 요청해도 정의로 이동
    json response = requestDefinition(uri, 0, 3);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    EXPECT_EQ(result["uri"], uri);
    EXPECT_EQ(result["range"]["start"]["line"], 0);
}

// ============================================================================
// 함수 정의로 이동 테스트
// ============================================================================

TEST_F(GoToDefinitionTest, ShouldFindFunctionDefinition)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content =
        "함수 더하기(a, b) {\n"
        "    반환 a + b\n"
        "}\n"
        "정수 결과 = 더하기(10, 20)";  // 더하기 호출 (line 3)

    openDocument(uri, content);

    // Act - 함수 호출 위치에서 정의 요청
    json response = requestDefinition(uri, 3, 8);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    EXPECT_EQ(result["uri"], uri);
    EXPECT_EQ(result["range"]["start"]["line"], 0);  // 함수 정의는 line 0
}

TEST_F(GoToDefinitionTest, ShouldFindFunctionDefinitionWithVarDecl)
{
    // Arrange - 변수 선언 스타일 함수
    std::string uri = "file:///test.ksj";
    std::string content =
        "정수 합 = 함수(a, b) {\n"
        "    반환 a + b\n"
        "}\n"
        "정수 x = 합(3, 5)";

    openDocument(uri, content);

    // Act
    json response = requestDefinition(uri, 3, 7);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    EXPECT_EQ(result["uri"], uri);
    EXPECT_EQ(result["range"]["start"]["line"], 0);
}

// ============================================================================
// 매개변수 정의로 이동 테스트
// ============================================================================

TEST_F(GoToDefinitionTest, ShouldFindParameterDefinition)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content =
        "함수 더하기(a, b) {\n"
        "    반환 a + b\n"  // a 참조 (line 1)
        "}";

    openDocument(uri, content);

    // Act - 함수 본문에서 매개변수 a 참조
    json response = requestDefinition(uri, 1, 7);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    EXPECT_EQ(result["uri"], uri);
    EXPECT_EQ(result["range"]["start"]["line"], 0);  // 매개변수는 함수 선언 줄
}

// ============================================================================
// 스코프 처리 테스트
// ============================================================================

TEST_F(GoToDefinitionTest, ShouldHandleLocalAndGlobalScope)
{
    // Arrange - 전역 x와 로컬 x
    std::string uri = "file:///test.ksj";
    std::string content =
        "정수 x = 10\n"
        "함수 테스트() {\n"
        "    정수 x = 20\n"
        "    반환 x\n"  // 로컬 x 참조 (line 3)
        "}";

    openDocument(uri, content);

    // Act - 함수 내부의 x 참조
    // "    반환 x"에서 UTF-16 character 7이 'x'
    json response = requestDefinition(uri, 3, 7);

    // Assert - 로컬 x 정의를 찾아야 함 (line 2)
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    EXPECT_EQ(result["uri"], uri);
    EXPECT_EQ(result["range"]["start"]["line"], 2);  // 로컬 x (line 2)
}

// ============================================================================
// 심볼을 찾을 수 없는 경우
// ============================================================================

TEST_F(GoToDefinitionTest, ShouldReturnNullWhenSymbolNotFound)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "정수 x = 10";

    openDocument(uri, content);

    // Act - 존재하지 않는 위치
    json response = requestDefinition(uri, 5, 10);

    // Assert - null 반환
    ASSERT_TRUE(response.contains("result"));
    EXPECT_TRUE(response["result"].is_null());
}

TEST_F(GoToDefinitionTest, ShouldReturnNullForUndefinedSymbol)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "정수 y = x + 5";  // x가 정의되지 않음

    openDocument(uri, content);

    // Act - 정의되지 않은 x 위치
    json response = requestDefinition(uri, 0, 9);

    // Assert - null 반환 (심볼 테이블에 없음)
    ASSERT_TRUE(response.contains("result"));
    EXPECT_TRUE(response["result"].is_null());
}

// ============================================================================
// 복잡한 코드 테스트
// ============================================================================

TEST_F(GoToDefinitionTest, ShouldHandleComplexCode)
{
    // Arrange
    // TODO: 현재 SymbolCollector는 AST 노드에 실제 줄 번호 정보가 없어서
    //       statement를 세는 방식으로 줄 번호를 추적합니다.
    //       따라서 빈 줄이 있으면 줄 번호가 어긋납니다.
    //       추후 Lexer/Parser에 줄 번호 추적을 추가해야 합니다.
    std::string uri = "file:///test.ksj";
    std::string content =
        "정수 전역변수 = 100\n"
        "함수 계산(a, b) {\n"
        "    정수 결과 = a + b\n"
        "    반환 결과\n"
        "}\n"
        "함수 main() {\n"
        "    정수 x = 10\n"
        "    정수 y = 20\n"
        "    정수 합계 = 계산(x, y)\n"  // 계산 호출 (line 8)
        "}";

    openDocument(uri, content);

    // Act - 계산 함수 호출 위치
    // "    정수 합계 = 계산(x, y)"에서 UTF-16 character 12가 '계'
    json response = requestDefinition(uri, 8, 12);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    EXPECT_EQ(result["uri"], uri);
    EXPECT_EQ(result["range"]["start"]["line"], 1);  // 계산 함수 정의 (line 1)
}
