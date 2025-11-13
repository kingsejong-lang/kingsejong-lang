/**
 * @file ServerCapabilitiesTest.cpp
 * @brief LSP 서버 capabilities 테스트
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * initialize 응답에 모든 고급 기능이 포함되어 있는지 확인
 */

#include <gtest/gtest.h>
#include "lsp/LanguageServer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace kingsejong::lsp;

class ServerCapabilitiesTest : public ::testing::Test
{
protected:
    LanguageServer server;
};

TEST_F(ServerCapabilitiesTest, ShouldAdvertiseAllCapabilities)
{
    // Arrange - initialize 요청
    json initRequest = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {}}
    };

    // Act
    json response = server.handleRequest(initRequest);

    // Assert - 응답 구조 검증
    ASSERT_TRUE(response.contains("id"));
    EXPECT_EQ(response["id"], 1);

    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_TRUE(result.contains("capabilities"));
    auto capabilities = result["capabilities"];

    // 기본 기능
    EXPECT_TRUE(capabilities.contains("textDocumentSync"));
    EXPECT_EQ(capabilities["textDocumentSync"], 1);  // Full sync

    // 자동 완성
    EXPECT_TRUE(capabilities.contains("completionProvider"));
    EXPECT_TRUE(capabilities["completionProvider"].is_object());

    // 고급 기능 - Go to Definition
    EXPECT_TRUE(capabilities.contains("definitionProvider"));
    EXPECT_TRUE(capabilities["definitionProvider"].get<bool>());

    // 고급 기능 - Hover Information
    EXPECT_TRUE(capabilities.contains("hoverProvider"));
    EXPECT_TRUE(capabilities["hoverProvider"].get<bool>());

    // 고급 기능 - Find References
    EXPECT_TRUE(capabilities.contains("referencesProvider"));
    EXPECT_TRUE(capabilities["referencesProvider"].get<bool>());

    // 고급 기능 - Rename
    EXPECT_TRUE(capabilities.contains("renameProvider"));
    EXPECT_TRUE(capabilities["renameProvider"].get<bool>());
}

TEST_F(ServerCapabilitiesTest, ShouldReturnServerInfo)
{
    // Arrange
    json initRequest = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {}}
    };

    // Act
    json response = server.handleRequest(initRequest);

    // Assert - serverInfo 검증
    ASSERT_TRUE(response.contains("result"));
    auto result = response["result"];

    ASSERT_TRUE(result.contains("serverInfo"));
    auto serverInfo = result["serverInfo"];

    EXPECT_TRUE(serverInfo.contains("name"));
    EXPECT_EQ(serverInfo["name"], "kingsejong-lsp");

    EXPECT_TRUE(serverInfo.contains("version"));
    EXPECT_EQ(serverInfo["version"], "0.3.2");
}

TEST_F(ServerCapabilitiesTest, ShouldSetInitializedFlag)
{
    // Arrange
    EXPECT_FALSE(server.isInitialized());

    json initRequest = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {}}
    };

    // Act
    server.handleRequest(initRequest);

    // Assert
    EXPECT_TRUE(server.isInitialized());
}
