/**
 * @file RenameTest.cpp
 * @brief Rename 기능 테스트
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * textDocument/rename 요청 처리 테스트
 */

#include <gtest/gtest.h>
#include "lsp/LanguageServer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace kingsejong::lsp;

class RenameTest : public ::testing::Test
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
     * @brief rename 요청 헬퍼
     */
    json requestRename(const std::string& uri, int line, int character, const std::string& newName)
    {
        json renameRequest = {
            {"jsonrpc", "2.0"},
            {"id", 400},
            {"method", "textDocument/rename"},
            {"params", {
                {"textDocument", {{"uri", uri}}},
                {"position", {
                    {"line", line},
                    {"character", character}
                }},
                {"newName", newName}
            }}
        };
        return server.handleRequest(renameRequest);
    }
};

// ============================================================================
// 변수 이름 변경 테스트
// ============================================================================

TEST_F(RenameTest, ShouldRenameVariable)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content =
        "정수 x = 10\n"
        "정수 y = x + 5\n"
        "정수 z = x * 2";

    openDocument(uri, content);

    // Act - 변수 x를 newValue로 변경
    json response = requestRename(uri, 0, 3, "newValue");

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    ASSERT_TRUE(result.contains("changes"));
    ASSERT_TRUE(result["changes"].contains(uri));

    auto edits = result["changes"][uri];
    ASSERT_TRUE(edits.is_array());
    EXPECT_GE(edits.size(), 3u);  // 정의 1개 + 참조 2개

    // 각 edit이 올바른 형식인지 확인
    for (const auto& edit : edits) {
        ASSERT_TRUE(edit.contains("range"));
        ASSERT_TRUE(edit.contains("newText"));
        EXPECT_EQ(edit["newText"], "newValue");
    }
}

TEST_F(RenameTest, ShouldRenameVariableFromReference)
{
    // Arrange - 참조 위치에서 rename 요청
    std::string uri = "file:///test.ksj";
    std::string content =
        "정수 x = 10\n"
        "정수 y = x + 5";

    openDocument(uri, content);

    // Act - 참조된 x 위치에서 변경
    json response = requestRename(uri, 1, 7, "num");

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    ASSERT_TRUE(result["changes"].contains(uri));

    auto edits = result["changes"][uri];
    EXPECT_GE(edits.size(), 2u);  // 정의 + 참조
}

// ============================================================================
// 함수 이름 변경 테스트
// ============================================================================

TEST_F(RenameTest, ShouldRenameFunction)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content =
        "함수 더하기(a, b) {\n"
        "    반환 a + b\n"
        "}\n"
        "정수 x = 더하기(1, 2)\n"
        "정수 y = 더하기(3, 4)";

    openDocument(uri, content);

    // Act - 함수 이름을 add로 변경
    json response = requestRename(uri, 0, 3, "add");

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    ASSERT_TRUE(result["changes"].contains(uri));

    auto edits = result["changes"][uri];
    EXPECT_GE(edits.size(), 3u);  // 정의 1개 + 호출 2개

    for (const auto& edit : edits) {
        EXPECT_EQ(edit["newText"], "add");
    }
}

TEST_F(RenameTest, ShouldRenameFunctionFromCallSite)
{
    // Arrange - 호출 위치에서 rename
    std::string uri = "file:///test.ksj";
    std::string content =
        "함수 더하기(a, b) {\n"
        "    반환 a + b\n"
        "}\n"
        "정수 x = 더하기(1, 2)";

    openDocument(uri, content);

    // Act - 호출 위치에서 변경
    json response = requestRename(uri, 3, 8, "sum");

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    auto edits = result["changes"][uri];
    EXPECT_GE(edits.size(), 2u);
}

// ============================================================================
// 매개변수 이름 변경 테스트
// ============================================================================

TEST_F(RenameTest, ShouldRenameParameter)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content =
        "함수 더하기(a, b) {\n"
        "    정수 결과 = a + b\n"
        "    반환 결과\n"
        "}";

    openDocument(uri, content);

    // Act - 매개변수 a를 x로 변경
    json response = requestRename(uri, 1, 12, "x");

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    auto edits = result["changes"][uri];
    EXPECT_GE(edits.size(), 2u);  // 정의 + 사용

    for (const auto& edit : edits) {
        EXPECT_EQ(edit["newText"], "x");
    }
}

// ============================================================================
// 스코프 처리 테스트
// ============================================================================

TEST_F(RenameTest, ShouldRenameOnlyLocalVariable)
{
    // Arrange - 전역 x와 로컬 x
    std::string uri = "file:///test.ksj";
    std::string content =
        "정수 x = 10\n"
        "함수 테스트() {\n"
        "    정수 x = 20\n"
        "    정수 y = x + 5\n"
        "}\n"
        "정수 z = x + 1";

    openDocument(uri, content);

    // Act - 로컬 x만 변경
    json response = requestRename(uri, 2, 7, "localX");

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    auto edits = result["changes"][uri];

    // 로컬 x만 변경되어야 함 (line 2, 3)
    for (const auto& edit : edits) {
        int line = edit["range"]["start"]["line"];
        EXPECT_TRUE(line == 2 || line == 3);  // 로컬 x만
        EXPECT_NE(line, 0);  // 전역 x (line 0)는 제외
        EXPECT_NE(line, 5);  // 전역 x 참조 (line 5)도 제외
    }
}

TEST_F(RenameTest, ShouldRenameOnlyGlobalVariable)
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

    // Act - 전역 x만 변경
    json response = requestRename(uri, 0, 3, "globalX");

    // Assert
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    auto edits = result["changes"][uri];

    // 전역 x만 변경되어야 함 (line 0, 5)
    for (const auto& edit : edits) {
        int line = edit["range"]["start"]["line"];
        EXPECT_TRUE(line == 0 || line == 5);  // 전역 x만
        EXPECT_NE(line, 2);  // 로컬 x는 제외
        EXPECT_NE(line, 3);  // 로컬 x 참조도 제외
    }
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(RenameTest, ShouldReturnNullForUnknownSymbol)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "정수 x = 10";

    openDocument(uri, content);

    // Act - 존재하지 않는 심볼
    json response = requestRename(uri, 0, 10, "newName");

    // Assert
    ASSERT_TRUE(response.contains("result"));
    EXPECT_TRUE(response["result"].is_null());
}

TEST_F(RenameTest, ShouldReturnNullForInvalidPosition)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "정수 x = 10";

    openDocument(uri, content);

    // Act - 유효하지 않은 위치
    json response = requestRename(uri, 10, 10, "newName");

    // Assert
    ASSERT_TRUE(response.contains("result"));
    EXPECT_TRUE(response["result"].is_null());
}

TEST_F(RenameTest, ShouldHandleEmptyNewName)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "정수 x = 10";

    openDocument(uri, content);

    // Act - 빈 이름으로 변경 시도
    json response = requestRename(uri, 0, 3, "");

    // Assert - 빈 이름은 거부되어야 함
    ASSERT_TRUE(response.contains("result"));
    EXPECT_TRUE(response["result"].is_null());
}

// ============================================================================
// WorkspaceEdit 형식 검증
// ============================================================================

TEST_F(RenameTest, ShouldReturnValidWorkspaceEdit)
{
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "정수 x = 10\n정수 y = x + 5";

    openDocument(uri, content);

    // Act
    json response = requestRename(uri, 0, 3, "value");

    // Assert - WorkspaceEdit 형식 검증
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_FALSE(result.is_null());
    ASSERT_TRUE(result.contains("changes"));
    ASSERT_TRUE(result["changes"].is_object());

    auto changes = result["changes"];
    ASSERT_TRUE(changes.contains(uri));
    ASSERT_TRUE(changes[uri].is_array());

    // 각 TextEdit 검증
    for (const auto& edit : changes[uri]) {
        ASSERT_TRUE(edit.contains("range"));
        ASSERT_TRUE(edit.contains("newText"));

        auto range = edit["range"];
        ASSERT_TRUE(range.contains("start"));
        ASSERT_TRUE(range.contains("end"));

        ASSERT_TRUE(range["start"].contains("line"));
        ASSERT_TRUE(range["start"].contains("character"));
        ASSERT_TRUE(range["end"].contains("line"));
        ASSERT_TRUE(range["end"].contains("character"));
    }
}
