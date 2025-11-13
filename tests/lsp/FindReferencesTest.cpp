/**
 * @file FindReferencesTest.cpp
 * @brief Find References 기능 테스트
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * textDocument/references 요청 처리 테스트
 */

#include <gtest/gtest.h>
#include "lsp/LanguageServer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace kingsejong::lsp;

class FindReferencesTest : public ::testing::Test
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
     * @brief references 요청 헬퍼
     */
    json requestReferences(const std::string& uri, int line, int character, bool includeDeclaration = true)
    {
        json refRequest = {
            {"jsonrpc", "2.0"},
            {"id", 300},
            {"method", "textDocument/references"},
            {"params", {
                {"textDocument", {{"uri", uri}}},
                {"position", {
                    {"line", line},
                    {"character", character}
                }},
                {"context", {
                    {"includeDeclaration", includeDeclaration}
                }}
            }}
        };
        return server.handleRequest(refRequest);
    }
};

// ============================================================================
// 변수 참조 찾기 테스트
// ============================================================================

TEST_F(FindReferencesTest, ShouldFindVariableReferences)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content =
        "정수 x = 10\n"
        "정수 y = x + 5\n"
        "정수 z = x * 2";  // x는 총 3번 사용 (정의 1번, 참조 2번)

    openDocument(uri, content);

    // Act - 변수 x의 정의 위치에서 references 요청
    json response = requestReferences(uri, 0, 3, true);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_TRUE(result.is_array());
    EXPECT_GE(result.size(), 2u);  // 최소 2개 (참조 2번)

    // 정의 포함 시 3개
    int referenceCount = 0;

    for (const auto& location : result) {
        ASSERT_TRUE(location.contains("uri"));
        ASSERT_TRUE(location.contains("range"));
        EXPECT_EQ(location["uri"], uri);

        int line = location["range"]["start"]["line"];
        if (line > 0) {  // line 0은 정의
            referenceCount++;
        }
    }

    EXPECT_GE(referenceCount, 2);  // 참조는 최소 2개
}

TEST_F(FindReferencesTest, ShouldExcludeDeclarationWhenRequested)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content =
        "정수 x = 10\n"
        "정수 y = x + 5\n"
        "정수 z = x * 2";

    openDocument(uri, content);

    // Act - includeDeclaration = false
    json response = requestReferences(uri, 0, 3, false);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_TRUE(result.is_array());

    // 정의는 포함하지 않음
    for (const auto& location : result) {
        int line = location["range"]["start"]["line"];
        EXPECT_NE(line, 0);  // 정의 위치(line 0)는 없어야 함
    }
}

// ============================================================================
// 함수 참조 찾기 테스트
// ============================================================================

TEST_F(FindReferencesTest, ShouldFindFunctionReferences)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content =
        "함수 더하기(a, b) {\n"
        "    반환 a + b\n"
        "}\n"
        "정수 x = 더하기(1, 2)\n"
        "정수 y = 더하기(3, 4)";  // 더하기 함수 2번 호출

    openDocument(uri, content);

    // Act - 함수 정의에서 references 요청
    json response = requestReferences(uri, 0, 3, true);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_TRUE(result.is_array());
    EXPECT_GE(result.size(), 2u);  // 최소 2개의 호출

    int callCount = 0;
    for (const auto& location : result) {
        int line = location["range"]["start"]["line"];
        if (line > 0) {  // 정의 이후의 호출
            callCount++;
        }
    }

    EXPECT_GE(callCount, 2);
}

TEST_F(FindReferencesTest, ShouldFindReferencesFromCallSite)
{
    // Arrange - 호출 위치에서 references 요청
    std::string uri = "file:///test.ksj";
    std::string content =
        "함수 더하기(a, b) {\n"
        "    반환 a + b\n"
        "}\n"
        "정수 x = 더하기(1, 2)\n"
        "정수 y = 더하기(3, 4)";

    openDocument(uri, content);

    // Act - 함수 호출 위치에서 요청
    json response = requestReferences(uri, 3, 8, true);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_TRUE(result.is_array());
    EXPECT_GE(result.size(), 2u);  // 정의 + 다른 호출들
}

// ============================================================================
// 매개변수 참조 찾기 테스트
// ============================================================================

TEST_F(FindReferencesTest, ShouldFindParameterReferences)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content =
        "함수 더하기(a, b) {\n"
        "    정수 결과 = a + b\n"
        "    반환 결과\n"
        "}";

    openDocument(uri, content);

    // Act - 매개변수 a 참조 찾기
    json response = requestReferences(uri, 1, 12, true);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_TRUE(result.is_array());
    EXPECT_GE(result.size(), 1u);  // 최소 1개 (함수 본문에서 사용)
}

// ============================================================================
// 스코프 처리 테스트
// ============================================================================

TEST_F(FindReferencesTest, ShouldHandleLocalAndGlobalScope)
{
    // Arrange - 전역 x와 로컬 x
    std::string uri = "file:///test.ksj";
    std::string content =
        "정수 x = 10\n"
        "함수 테스트() {\n"
        "    정수 x = 20\n"
        "    정수 y = x + 5\n"  // 로컬 x 참조
        "}\n"
        "정수 z = x + 1";  // 전역 x 참조

    openDocument(uri, content);

    // Act - 전역 x의 참조 찾기
    json response = requestReferences(uri, 0, 3, true);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_TRUE(result.is_array());

    // 전역 x의 참조는 line 0과 line 5에만 있어야 함
    for (const auto& location : result) {
        int line = location["range"]["start"]["line"];
        EXPECT_TRUE(line == 0 || line == 5);  // line 2, 3은 로컬 x
    }
}

TEST_F(FindReferencesTest, ShouldFindLocalVariableReferences)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content =
        "정수 x = 10\n"
        "함수 테스트() {\n"
        "    정수 x = 20\n"
        "    정수 y = x + 5\n"
        "}\n"
        "정수 z = x + 1";

    openDocument(uri, content);

    // Act - 로컬 x의 참조 찾기 (line 2)
    json response = requestReferences(uri, 2, 7, true);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_TRUE(result.is_array());

    // 로컬 x의 참조는 line 2와 line 3에만 있어야 함
    for (const auto& location : result) {
        int line = location["range"]["start"]["line"];
        EXPECT_TRUE(line == 2 || line == 3);  // 로컬 x만
    }
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(FindReferencesTest, ShouldReturnEmptyForUnusedVariable)
{
    // Arrange - 사용되지 않는 변수
    std::string uri = "file:///test.ksj";
    std::string content = "정수 x = 10\n정수 y = 20";

    openDocument(uri, content);

    // Act - x의 참조 찾기 (정의만 있고 사용은 없음)
    json response = requestReferences(uri, 0, 3, false);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_TRUE(result.is_array());
    EXPECT_EQ(result.size(), 0u);  // 참조 없음
}

TEST_F(FindReferencesTest, ShouldReturnEmptyForUnknownSymbol)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "정수 x = 10";

    openDocument(uri, content);

    // Act - 존재하지 않는 심볼
    json response = requestReferences(uri, 0, 10, true);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_TRUE(result.is_array());
    EXPECT_EQ(result.size(), 0u);
}

TEST_F(FindReferencesTest, ShouldReturnEmptyForInvalidPosition)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "정수 x = 10";

    openDocument(uri, content);

    // Act - 유효하지 않은 위치
    json response = requestReferences(uri, 10, 10, true);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_TRUE(result.is_array());
    EXPECT_EQ(result.size(), 0u);
}
