/**
 * @file LanguageServerTest.cpp
 * @brief LanguageServer 테스트
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * TDD 방식: 테스트를 먼저 작성하고 구현합니다.
 */

#include <gtest/gtest.h>
#include <sstream>
#include "lsp/LanguageServer.h"
#include "lsp/JsonRpc.h"
#include <nlohmann/json.hpp>

using namespace kingsejong::lsp;
using json = nlohmann::json;

/**
 * @class LanguageServerTest
 * @brief LanguageServer 테스트 픽스처
 */
class LanguageServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        server = std::make_unique<LanguageServer>();
    }

    std::unique_ptr<LanguageServer> server;
};

// ============================================================================
// Initialize 핸들러 테스트
// ============================================================================

TEST_F(LanguageServerTest, ShouldHandleInitializeRequest) {
    // Arrange
    json request = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {
            {"processId", 1234},
            {"rootUri", "file:///test/project"},
            {"capabilities", {}}
        }}
    };

    // Act
    json response = server->handleRequest(request);

    // Assert
    ASSERT_TRUE(response.contains("result"));
    ASSERT_TRUE(response["result"].contains("capabilities"));
    EXPECT_EQ(response["id"], 1);
}

TEST_F(LanguageServerTest, ShouldReturnServerCapabilities) {
    // Arrange
    json request = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {}}
    };

    // Act
    json response = server->handleRequest(request);

    // Assert
    auto capabilities = response["result"]["capabilities"];
    EXPECT_TRUE(capabilities.contains("textDocumentSync"));
    EXPECT_TRUE(capabilities.contains("completionProvider"));
}

TEST_F(LanguageServerTest, ShouldSetInitializedState) {
    // Arrange
    json initRequest = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {}}
    };

    // Act
    server->handleRequest(initRequest);

    // Assert - initialized notification should succeed
    json notification = {
        {"jsonrpc", "2.0"},
        {"method", "initialized"},
        {"params", {}}
    };
    EXPECT_NO_THROW(server->handleRequest(notification));
}

// ============================================================================
// textDocument/didOpen 핸들러 테스트
// ============================================================================

TEST_F(LanguageServerTest, ShouldHandleDidOpenNotification) {
    // Arrange - initialize first
    json initRequest = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {}}
    };
    server->handleRequest(initRequest);

    json notification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", {
            {"textDocument", {
                {"uri", "file:///test.ksj"},
                {"languageId", "kingsejong"},
                {"version", 1},
                {"text", "변수 x = 10"}
            }}
        }}
    };

    // Act & Assert - notification should not throw
    EXPECT_NO_THROW(server->handleRequest(notification));
}

TEST_F(LanguageServerTest, ShouldStoreOpenedDocument) {
    // Arrange
    json initRequest = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {}}
    };
    server->handleRequest(initRequest);

    json notification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", {
            {"textDocument", {
                {"uri", "file:///test.ksj"},
                {"languageId", "kingsejong"},
                {"version", 1},
                {"text", "변수 x = 10"}
            }}
        }}
    };

    // Act
    server->handleRequest(notification);

    // Assert - document should be stored
    const auto* doc = server->getDocument("file:///test.ksj");
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(doc->uri, "file:///test.ksj");
    EXPECT_EQ(doc->content, "변수 x = 10");
    EXPECT_EQ(doc->version, 1);
}

// ============================================================================
// textDocument/didChange 핸들러 테스트
// ============================================================================

TEST_F(LanguageServerTest, ShouldHandleDidChangeNotification) {
    // Arrange - open document first
    json initRequest = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {}}
    };
    server->handleRequest(initRequest);

    json openNotification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", {
            {"textDocument", {
                {"uri", "file:///test.ksj"},
                {"languageId", "kingsejong"},
                {"version", 1},
                {"text", "변수 x = 10"}
            }}
        }}
    };
    server->handleRequest(openNotification);

    json changeNotification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didChange"},
        {"params", {
            {"textDocument", {
                {"uri", "file:///test.ksj"},
                {"version", 2}
            }},
            {"contentChanges", {
                {{"text", "변수 x = 20"}}
            }}
        }}
    };

    // Act & Assert
    EXPECT_NO_THROW(server->handleRequest(changeNotification));
}

TEST_F(LanguageServerTest, ShouldUpdateDocumentContent) {
    // Arrange
    json initRequest = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {}}
    };
    server->handleRequest(initRequest);

    json openNotification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", {
            {"textDocument", {
                {"uri", "file:///test.ksj"},
                {"languageId", "kingsejong"},
                {"version", 1},
                {"text", "변수 x = 10"}
            }}
        }}
    };
    server->handleRequest(openNotification);

    json changeNotification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didChange"},
        {"params", {
            {"textDocument", {
                {"uri", "file:///test.ksj"},
                {"version", 2}
            }},
            {"contentChanges", {
                {{"text", "변수 x = 20"}}
            }}
        }}
    };

    // Act
    server->handleRequest(changeNotification);

    // Assert
    const auto* doc = server->getDocument("file:///test.ksj");
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(doc->content, "변수 x = 20");
    EXPECT_EQ(doc->version, 2);
}

TEST_F(LanguageServerTest, ShouldHandleMultipleContentChanges) {
    // Arrange
    json initRequest = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {}}
    };
    server->handleRequest(initRequest);

    json openNotification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", {
            {"textDocument", {
                {"uri", "file:///test.ksj"},
                {"languageId", "kingsejong"},
                {"version", 1},
                {"text", "변수 x = 10"}
            }}
        }}
    };
    server->handleRequest(openNotification);

    // Act - multiple changes
    server->handleRequest({
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didChange"},
        {"params", {
            {"textDocument", {{"uri", "file:///test.ksj"}, {"version", 2}}},
            {"contentChanges", {{{"text", "변수 x = 20"}}}}
        }}
    });

    server->handleRequest({
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didChange"},
        {"params", {
            {"textDocument", {{"uri", "file:///test.ksj"}, {"version", 3}}},
            {"contentChanges", {{{"text", "변수 x = 30"}}}}
        }}
    });

    // Assert
    const auto* doc = server->getDocument("file:///test.ksj");
    EXPECT_EQ(doc->version, 3);
    EXPECT_EQ(doc->content, "변수 x = 30");
}

// ============================================================================
// textDocument/didClose 핸들러 테스트
// ============================================================================

TEST_F(LanguageServerTest, ShouldHandleDidCloseNotification) {
    // Arrange
    json initRequest = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {}}
    };
    server->handleRequest(initRequest);

    json openNotification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", {
            {"textDocument", {
                {"uri", "file:///test.ksj"},
                {"languageId", "kingsejong"},
                {"version", 1},
                {"text", "변수 x = 10"}
            }}
        }}
    };
    server->handleRequest(openNotification);

    json closeNotification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didClose"},
        {"params", {
            {"textDocument", {
                {"uri", "file:///test.ksj"}
            }}
        }}
    };

    // Act & Assert
    EXPECT_NO_THROW(server->handleRequest(closeNotification));
}

TEST_F(LanguageServerTest, ShouldRemoveClosedDocument) {
    // Arrange
    json initRequest = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {}}
    };
    server->handleRequest(initRequest);

    json openNotification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", {
            {"textDocument", {
                {"uri", "file:///test.ksj"},
                {"languageId", "kingsejong"},
                {"version", 1},
                {"text", "변수 x = 10"}
            }}
        }}
    };
    server->handleRequest(openNotification);

    json closeNotification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didClose"},
        {"params", {
            {"textDocument", {
                {"uri", "file:///test.ksj"}
            }}
        }}
    };

    // Act
    server->handleRequest(closeNotification);

    // Assert
    const auto* doc = server->getDocument("file:///test.ksj");
    EXPECT_EQ(doc, nullptr);
}

// ============================================================================
// shutdown/exit 핸들러 테스트
// ============================================================================

TEST_F(LanguageServerTest, ShouldHandleShutdownRequest) {
    // Arrange
    json request = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "shutdown"},
        {"params", nullptr}
    };

    // Act
    json response = server->handleRequest(request);

    // Assert
    EXPECT_EQ(response["id"], 1);
    EXPECT_TRUE(response.contains("result"));
    EXPECT_TRUE(response["result"].is_null());
}

TEST_F(LanguageServerTest, ShouldHandleExitNotification) {
    // Arrange
    json notification = {
        {"jsonrpc", "2.0"},
        {"method", "exit"}
    };

    // Act & Assert - should not throw
    EXPECT_NO_THROW(server->handleRequest(notification));
}

TEST_F(LanguageServerTest, ShouldSetShutdownState) {
    // Arrange
    json shutdownRequest = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "shutdown"},
        {"params", nullptr}
    };

    // Act
    server->handleRequest(shutdownRequest);

    // Assert - isShutdown should return true
    EXPECT_TRUE(server->isShutdown());
}

// ============================================================================
// 에러 처리 테스트
// ============================================================================

TEST_F(LanguageServerTest, ShouldReturnErrorForUnknownMethod) {
    // Arrange
    json request = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "unknown/method"},
        {"params", {}}
    };

    // Act
    json response = server->handleRequest(request);

    // Assert
    EXPECT_TRUE(response.contains("error"));
    EXPECT_EQ(response["error"]["code"], -32601);  // MethodNotFound
}

TEST_F(LanguageServerTest, ShouldReturnErrorForInvalidParams) {
    // Arrange
    json request = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "textDocument/didOpen"},
        {"params", {}}  // Missing required fields
    };

    // Act
    json response = server->handleRequest(request);

    // Assert
    EXPECT_TRUE(response.contains("error"));
    EXPECT_EQ(response["error"]["code"], -32602);  // InvalidParams
}

// ============================================================================
// 통합 테스트
// ============================================================================

TEST_F(LanguageServerTest, ShouldHandleCompleteWorkflow) {
    // Arrange & Act
    // 1. Initialize
    json initResponse = server->handleRequest({
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {}}
    });
    ASSERT_TRUE(initResponse.contains("result"));

    // 2. Initialized notification
    server->handleRequest({
        {"jsonrpc", "2.0"},
        {"method", "initialized"},
        {"params", {}}
    });

    // 3. Open document
    server->handleRequest({
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", {
            {"textDocument", {
                {"uri", "file:///test.ksj"},
                {"languageId", "kingsejong"},
                {"version", 1},
                {"text", "변수 x = 10"}
            }}
        }}
    });

    // 4. Change document
    server->handleRequest({
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didChange"},
        {"params", {
            {"textDocument", {{"uri", "file:///test.ksj"}, {"version", 2}}},
            {"contentChanges", {{{"text", "변수 x = 20"}}}}
        }}
    });

    // 5. Close document
    server->handleRequest({
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didClose"},
        {"params", {
            {"textDocument", {{"uri", "file:///test.ksj"}}}
        }}
    });

    // 6. Shutdown
    json shutdownResponse = server->handleRequest({
        {"jsonrpc", "2.0"},
        {"id", 2},
        {"method", "shutdown"},
        {"params", nullptr}
    });
    EXPECT_TRUE(shutdownResponse["result"].is_null());

    // 7. Exit
    server->handleRequest({
        {"jsonrpc", "2.0"},
        {"method", "exit"}
    });

    // Assert
    EXPECT_TRUE(server->isShutdown());
}
