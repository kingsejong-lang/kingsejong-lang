/**
 * @file HoverTest.cpp
 * @brief Hover Information 기능 테스트
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * textDocument/hover 요청 처리 테스트
 */

#include <gtest/gtest.h>
#include "lsp/LanguageServer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace kingsejong::lsp;

class HoverTest : public ::testing::Test
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
     * @brief hover 요청 헬퍼
     */
    json requestHover(const std::string& uri, int line, int character)
    {
        json hoverRequest = {
            {"jsonrpc", "2.0"},
            {"id", 200},
            {"method", "textDocument/hover"},
            {"params", {
                {"textDocument", {{"uri", uri}}},
                {"position", {
                    {"line", line},
                    {"character", character}
                }}
            }}
        };
        return server.handleRequest(hoverRequest);
    }
};

// ============================================================================
// 변수 Hover 테스트
// ============================================================================

TEST_F(HoverTest, ShouldShowVariableTypeInfo)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "정수 x = 10\n정수 y = x + 5";

    openDocument(uri, content);

    // Act - 변수 x에 hover
    json response = requestHover(uri, 0, 3);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    ASSERT_TRUE(result.contains("contents"));

    // contents는 string 또는 object일 수 있음
    if (result["contents"].is_string()) {
        std::string contents = result["contents"];
        EXPECT_TRUE(contents.find("x") != std::string::npos);
        EXPECT_TRUE(contents.find("정수") != std::string::npos);
    } else {
        ASSERT_TRUE(result["contents"].contains("value"));
        std::string value = result["contents"]["value"];
        EXPECT_TRUE(value.find("x") != std::string::npos);
        EXPECT_TRUE(value.find("정수") != std::string::npos);
    }
}

TEST_F(HoverTest, ShouldShowVariableInfoOnReference)
{
    // Arrange - 변수 참조 위치에서 hover
    std::string uri = "file:///test.ksj";
    std::string content = "정수 x = 10\n정수 y = x + 5";

    openDocument(uri, content);

    // Act - 참조된 x에 hover (line 1, char 7)
    json response = requestHover(uri, 1, 7);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    ASSERT_TRUE(result.contains("contents"));
}

// ============================================================================
// 함수 Hover 테스트
// ============================================================================

TEST_F(HoverTest, ShouldShowFunctionSignature)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content =
        "함수 더하기(a, b) {\n"
        "    반환 a + b\n"
        "}\n"
        "정수 결과 = 더하기(10, 20)";

    openDocument(uri, content);

    // Act - 함수 정의에서 hover
    json response = requestHover(uri, 0, 3);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    ASSERT_TRUE(result.contains("contents"));

    // 함수 시그니처 확인
    std::string contents;
    if (result["contents"].is_string()) {
        contents = result["contents"];
    } else {
        contents = result["contents"]["value"];
    }

    EXPECT_TRUE(contents.find("더하기") != std::string::npos);
    EXPECT_TRUE(contents.find("함수") != std::string::npos);
}

TEST_F(HoverTest, ShouldShowFunctionInfoOnCall)
{
    // Arrange - 함수 호출 위치에서 hover
    std::string uri = "file:///test.ksj";
    std::string content =
        "함수 더하기(a, b) {\n"
        "    반환 a + b\n"
        "}\n"
        "정수 결과 = 더하기(10, 20)";

    openDocument(uri, content);

    // Act - 함수 호출에서 hover
    json response = requestHover(uri, 3, 8);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    ASSERT_TRUE(result.contains("contents"));
}

// ============================================================================
// 매개변수 Hover 테스트
// ============================================================================

TEST_F(HoverTest, ShouldShowParameterInfo)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content =
        "함수 더하기(a, b) {\n"
        "    반환 a + b\n"
        "}";

    openDocument(uri, content);

    // Act - 매개변수 a에 hover (함수 본문 내)
    json response = requestHover(uri, 1, 7);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    ASSERT_TRUE(result.contains("contents"));

    std::string contents;
    if (result["contents"].is_string()) {
        contents = result["contents"];
    } else {
        contents = result["contents"]["value"];
    }

    EXPECT_TRUE(contents.find("a") != std::string::npos);
}

// ============================================================================
// Null 반환 테스트
// ============================================================================

TEST_F(HoverTest, ShouldReturnNullForUnknownSymbol)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "정수 x = 10";

    openDocument(uri, content);

    // Act - 정의되지 않은 심볼에 hover
    json response = requestHover(uri, 0, 10);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    EXPECT_TRUE(response["result"].is_null());
}

TEST_F(HoverTest, ShouldReturnNullForInvalidPosition)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "정수 x = 10";

    openDocument(uri, content);

    // Act - 유효하지 않은 위치
    json response = requestHover(uri, 10, 10);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    EXPECT_TRUE(response["result"].is_null());
}

// ============================================================================
// 타입 정보 테스트
// ============================================================================

TEST_F(HoverTest, ShouldShowTypeForStringVariable)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "문자열 이름 = \"세종대왕\"";

    openDocument(uri, content);

    // Act
    json response = requestHover(uri, 0, 4);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    ASSERT_TRUE(result.contains("contents"));

    std::string contents;
    if (result["contents"].is_string()) {
        contents = result["contents"];
    } else {
        contents = result["contents"]["value"];
    }

    EXPECT_TRUE(contents.find("이름") != std::string::npos);
    EXPECT_TRUE(contents.find("문자열") != std::string::npos);
}
