/**
 * @file JsonRpcTest.cpp
 * @brief JSON-RPC 통신 테스트
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * TDD 방식: 테스트를 먼저 작성하고 구현합니다.
 */

#include <gtest/gtest.h>
#include "lsp/JsonRpc.h"
#include <sstream>

using namespace kingsejong::lsp;

/**
 * @class JsonRpcTest
 * @brief JsonRpc 테스트 픽스처
 */
class JsonRpcTest : public ::testing::Test {
protected:
    void SetUp() override {
        rpc = std::make_unique<JsonRpc>();
    }

    std::unique_ptr<JsonRpc> rpc;
};

// ============================================================================
// 메시지 파싱 테스트
// ============================================================================

TEST_F(JsonRpcTest, ShouldParseSimpleRequest) {
    // Arrange
    std::string message = R"({"jsonrpc":"2.0","id":1,"method":"initialize"})";

    // Act
    auto request = rpc->parse(message);

    // Assert
    EXPECT_EQ(request["jsonrpc"], "2.0");
    EXPECT_EQ(request["id"], 1);
    EXPECT_EQ(request["method"], "initialize");
}

TEST_F(JsonRpcTest, ShouldParseRequestWithParams) {
    // Arrange
    std::string message = R"({
        "jsonrpc":"2.0",
        "id":2,
        "method":"textDocument/completion",
        "params":{"textDocument":{"uri":"file:///test.ksj"},"position":{"line":0,"character":5}}
    })";

    // Act
    auto request = rpc->parse(message);

    // Assert
    EXPECT_EQ(request["method"], "textDocument/completion");
    EXPECT_TRUE(request.contains("params"));
    EXPECT_EQ(request["params"]["position"]["line"], 0);
    EXPECT_EQ(request["params"]["position"]["character"], 5);
}

TEST_F(JsonRpcTest, ShouldParseNotification) {
    // Arrange - 알림은 id가 없음
    std::string message = R"({
        "jsonrpc":"2.0",
        "method":"textDocument/didOpen",
        "params":{"textDocument":{"uri":"file:///test.ksj","text":"변수 x = 10"}}
    })";

    // Act
    auto notification = rpc->parse(message);

    // Assert
    EXPECT_EQ(notification["method"], "textDocument/didOpen");
    EXPECT_FALSE(notification.contains("id"));
}

TEST_F(JsonRpcTest, ShouldThrowOnInvalidJson) {
    // Arrange
    std::string invalidJson = "{ invalid json }";

    // Act & Assert
    EXPECT_THROW(rpc->parse(invalidJson), std::runtime_error);
}

// ============================================================================
// 메시지 직렬화 테스트
// ============================================================================

TEST_F(JsonRpcTest, ShouldSerializeResponse) {
    // Arrange
    nlohmann::json response;
    response["jsonrpc"] = "2.0";
    response["id"] = 1;
    response["result"]["capabilities"]["textDocumentSync"] = 1;

    // Act
    std::string message = rpc->serialize(response);

    // Assert
    EXPECT_TRUE(message.find("\"jsonrpc\":\"2.0\"") != std::string::npos);
    EXPECT_TRUE(message.find("\"id\":1") != std::string::npos);
    EXPECT_TRUE(message.find("\"result\"") != std::string::npos);
}

TEST_F(JsonRpcTest, ShouldSerializeError) {
    // Arrange
    nlohmann::json error;
    error["jsonrpc"] = "2.0";
    error["id"] = 1;
    error["error"]["code"] = -32600;
    error["error"]["message"] = "Invalid Request";

    // Act
    std::string message = rpc->serialize(error);

    // Assert
    EXPECT_TRUE(message.find("\"error\"") != std::string::npos);
    EXPECT_TRUE(message.find("-32600") != std::string::npos);
}

// ============================================================================
// Content-Length 헤더 테스트
// ============================================================================

TEST_F(JsonRpcTest, ShouldReadMessageWithHeader) {
    // Arrange
    std::string content = R"({"jsonrpc":"2.0","id":1,"method":"initialize"})";
    std::string header = "Content-Length: " + std::to_string(content.length()) + "\r\n\r\n";
    std::string fullMessage = header + content;

    std::istringstream input(fullMessage);

    // Act
    auto request = rpc->readMessage(input);

    // Assert
    EXPECT_EQ(request["method"], "initialize");
}

TEST_F(JsonRpcTest, ShouldWriteMessageWithHeader) {
    // Arrange
    nlohmann::json response;
    response["jsonrpc"] = "2.0";
    response["id"] = 1;
    response["result"] = nullptr;

    std::ostringstream output;

    // Act
    rpc->writeMessage(output, response);

    // Assert
    std::string result = output.str();
    EXPECT_TRUE(result.find("Content-Length:") == 0);  // 헤더가 맨 앞
    EXPECT_TRUE(result.find("\r\n\r\n") != std::string::npos);  // 헤더 구분자
    EXPECT_TRUE(result.find("\"jsonrpc\":\"2.0\"") != std::string::npos);
}

TEST_F(JsonRpcTest, ShouldThrowOnMissingHeader) {
    // Arrange - Content-Length 헤더 없음
    std::string invalidMessage = R"({"jsonrpc":"2.0"})";
    std::istringstream input(invalidMessage);

    // Act & Assert
    EXPECT_THROW(rpc->readMessage(input), std::runtime_error);
}

// ============================================================================
// 에러 응답 생성 테스트
// ============================================================================

TEST_F(JsonRpcTest, ShouldCreateErrorResponse) {
    // Arrange
    int id = 1;
    int code = -32601;  // Method not found
    std::string message = "Method not found";

    // Act
    auto error = rpc->createError(id, code, message);

    // Assert
    EXPECT_EQ(error["jsonrpc"], "2.0");
    EXPECT_EQ(error["id"], id);
    EXPECT_EQ(error["error"]["code"], code);
    EXPECT_EQ(error["error"]["message"], message);
}

TEST_F(JsonRpcTest, ShouldCreateSuccessResponse) {
    // Arrange
    int id = 1;
    nlohmann::json result;
    result["status"] = "ok";

    // Act
    auto response = rpc->createResponse(id, result);

    // Assert
    EXPECT_EQ(response["jsonrpc"], "2.0");
    EXPECT_EQ(response["id"], id);
    EXPECT_EQ(response["result"]["status"], "ok");
    EXPECT_FALSE(response.contains("error"));
}

// ============================================================================
// LSP 표준 에러 코드 테스트
// ============================================================================

TEST_F(JsonRpcTest, ShouldUseStandardErrorCodes) {
    // LSP 표준 에러 코드 정의
    EXPECT_EQ(JsonRpc::ErrorCode::ParseError, -32700);
    EXPECT_EQ(JsonRpc::ErrorCode::InvalidRequest, -32600);
    EXPECT_EQ(JsonRpc::ErrorCode::MethodNotFound, -32601);
    EXPECT_EQ(JsonRpc::ErrorCode::InvalidParams, -32602);
    EXPECT_EQ(JsonRpc::ErrorCode::InternalError, -32603);
}

// ============================================================================
// 멀티 메시지 처리 테스트
// ============================================================================

TEST_F(JsonRpcTest, ShouldReadMultipleMessages) {
    // Arrange
    std::string msg1 = R"({"jsonrpc":"2.0","id":1,"method":"initialize"})";
    std::string msg2 = R"({"jsonrpc":"2.0","method":"initialized"})";

    std::string fullInput =
        "Content-Length: " + std::to_string(msg1.length()) + "\r\n\r\n" + msg1 +
        "Content-Length: " + std::to_string(msg2.length()) + "\r\n\r\n" + msg2;

    std::istringstream input(fullInput);

    // Act
    auto request1 = rpc->readMessage(input);
    auto request2 = rpc->readMessage(input);

    // Assert
    EXPECT_EQ(request1["method"], "initialize");
    EXPECT_EQ(request2["method"], "initialized");
}
