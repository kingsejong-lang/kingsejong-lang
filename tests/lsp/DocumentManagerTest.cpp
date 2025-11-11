/**
 * @file DocumentManagerTest.cpp
 * @brief DocumentManager 테스트
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * TDD 방식: 테스트를 먼저 작성하고 구현합니다.
 */

#include <gtest/gtest.h>
#include "lsp/DocumentManager.h"

using namespace kingsejong::lsp;

/**
 * @class DocumentManagerTest
 * @brief DocumentManager 테스트 픽스처
 */
class DocumentManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<DocumentManager>();
    }

    std::unique_ptr<DocumentManager> manager;
};

// ============================================================================
// 문서 열기 테스트
// ============================================================================

TEST_F(DocumentManagerTest, ShouldOpenDocument) {
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "변수 x = 10";
    int version = 1;

    // Act
    manager->openDocument(uri, content, version);

    // Assert
    const auto* doc = manager->getDocument(uri);
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(doc->uri, uri);
    EXPECT_EQ(doc->content, content);
    EXPECT_EQ(doc->version, version);
}

TEST_F(DocumentManagerTest, ShouldOpenMultipleDocuments) {
    // Arrange & Act
    manager->openDocument("file:///test1.ksj", "변수 x = 10", 1);
    manager->openDocument("file:///test2.ksj", "변수 y = 20", 1);
    manager->openDocument("file:///test3.ksj", "변수 z = 30", 1);

    // Assert
    EXPECT_NE(manager->getDocument("file:///test1.ksj"), nullptr);
    EXPECT_NE(manager->getDocument("file:///test2.ksj"), nullptr);
    EXPECT_NE(manager->getDocument("file:///test3.ksj"), nullptr);
}

TEST_F(DocumentManagerTest, ShouldNotOpenDuplicateDocument) {
    // Arrange
    std::string uri = "file:///test.ksj";
    manager->openDocument(uri, "변수 x = 10", 1);

    // Act & Assert - 중복 열기 시 예외
    EXPECT_THROW(manager->openDocument(uri, "변수 y = 20", 1), std::runtime_error);
}

// ============================================================================
// 문서 수정 테스트
// ============================================================================

TEST_F(DocumentManagerTest, ShouldUpdateDocument) {
    // Arrange
    std::string uri = "file:///test.ksj";
    manager->openDocument(uri, "변수 x = 10", 1);

    // Act
    manager->updateDocument(uri, "변수 x = 20", 2);

    // Assert
    const auto* doc = manager->getDocument(uri);
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(doc->content, "변수 x = 20");
    EXPECT_EQ(doc->version, 2);
}

TEST_F(DocumentManagerTest, ShouldTrackVersionChanges) {
    // Arrange
    std::string uri = "file:///test.ksj";
    manager->openDocument(uri, "변수 x = 10", 1);

    // Act - 여러 번 수정
    manager->updateDocument(uri, "변수 x = 20", 2);
    manager->updateDocument(uri, "변수 x = 30", 3);
    manager->updateDocument(uri, "변수 x = 40", 4);

    // Assert
    const auto* doc = manager->getDocument(uri);
    EXPECT_EQ(doc->version, 4);
    EXPECT_EQ(doc->content, "변수 x = 40");
}

TEST_F(DocumentManagerTest, ShouldThrowOnUpdateNonExistentDocument) {
    // Arrange
    std::string uri = "file:///nonexistent.ksj";

    // Act & Assert
    EXPECT_THROW(manager->updateDocument(uri, "content", 1), std::runtime_error);
}

// ============================================================================
// 문서 닫기 테스트
// ============================================================================

TEST_F(DocumentManagerTest, ShouldCloseDocument) {
    // Arrange
    std::string uri = "file:///test.ksj";
    manager->openDocument(uri, "변수 x = 10", 1);

    // Act
    manager->closeDocument(uri);

    // Assert
    EXPECT_EQ(manager->getDocument(uri), nullptr);
}

TEST_F(DocumentManagerTest, ShouldNotThrowOnCloseNonExistentDocument) {
    // Arrange
    std::string uri = "file:///nonexistent.ksj";

    // Act & Assert - 없는 문서 닫기는 무시
    EXPECT_NO_THROW(manager->closeDocument(uri));
}

TEST_F(DocumentManagerTest, ShouldCloseAllDocuments) {
    // Arrange
    manager->openDocument("file:///test1.ksj", "content1", 1);
    manager->openDocument("file:///test2.ksj", "content2", 1);
    manager->openDocument("file:///test3.ksj", "content3", 1);

    // Act
    manager->closeDocument("file:///test1.ksj");
    manager->closeDocument("file:///test2.ksj");
    manager->closeDocument("file:///test3.ksj");

    // Assert
    EXPECT_EQ(manager->getDocument("file:///test1.ksj"), nullptr);
    EXPECT_EQ(manager->getDocument("file:///test2.ksj"), nullptr);
    EXPECT_EQ(manager->getDocument("file:///test3.ksj"), nullptr);
}

// ============================================================================
// 문서 조회 테스트
// ============================================================================

TEST_F(DocumentManagerTest, ShouldReturnNullForNonExistentDocument) {
    // Arrange
    std::string uri = "file:///nonexistent.ksj";

    // Act
    const auto* doc = manager->getDocument(uri);

    // Assert
    EXPECT_EQ(doc, nullptr);
}

TEST_F(DocumentManagerTest, ShouldGetDocumentContent) {
    // Arrange
    std::string uri = "file:///test.ksj";
    std::string content = "변수 x = 10\n출력(x)";
    manager->openDocument(uri, content, 1);

    // Act
    const auto* doc = manager->getDocument(uri);

    // Assert
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(doc->content, content);
}

// ============================================================================
// 모든 문서 조회 테스트
// ============================================================================

TEST_F(DocumentManagerTest, ShouldGetAllDocuments) {
    // Arrange
    manager->openDocument("file:///test1.ksj", "content1", 1);
    manager->openDocument("file:///test2.ksj", "content2", 1);

    // Act
    const auto& allDocs = manager->getAllDocuments();

    // Assert
    EXPECT_EQ(allDocs.size(), 2);
    EXPECT_NE(allDocs.find("file:///test1.ksj"), allDocs.end());
    EXPECT_NE(allDocs.find("file:///test2.ksj"), allDocs.end());
}

TEST_F(DocumentManagerTest, ShouldReturnEmptyWhenNoDocuments) {
    // Act
    const auto& allDocs = manager->getAllDocuments();

    // Assert
    EXPECT_TRUE(allDocs.empty());
}

// ============================================================================
// URI 처리 테스트
// ============================================================================

TEST_F(DocumentManagerTest, ShouldHandleComplexURI) {
    // Arrange
    std::string uri = "file:///path/to/project/src/main.ksj";
    manager->openDocument(uri, "content", 1);

    // Act
    const auto* doc = manager->getDocument(uri);

    // Assert
    EXPECT_NE(doc, nullptr);
    EXPECT_EQ(doc->uri, uri);
}

TEST_F(DocumentManagerTest, ShouldHandleWindowsStyleURI) {
    // Arrange
    std::string uri = "file:///C:/Users/test/project/main.ksj";
    manager->openDocument(uri, "content", 1);

    // Act
    const auto* doc = manager->getDocument(uri);

    // Assert
    EXPECT_NE(doc, nullptr);
    EXPECT_EQ(doc->uri, uri);
}

// ============================================================================
// 버전 일관성 테스트
// ============================================================================

TEST_F(DocumentManagerTest, ShouldMaintainVersionOrder) {
    // Arrange
    std::string uri = "file:///test.ksj";
    manager->openDocument(uri, "v1", 1);

    // Act
    manager->updateDocument(uri, "v2", 2);
    manager->updateDocument(uri, "v3", 3);

    // Assert
    const auto* doc = manager->getDocument(uri);
    EXPECT_EQ(doc->version, 3);
}

TEST_F(DocumentManagerTest, ShouldAllowNonSequentialVersions) {
    // Arrange - LSP 클라이언트가 비순차적 버전 전송 가능
    std::string uri = "file:///test.ksj";
    manager->openDocument(uri, "v1", 1);

    // Act
    manager->updateDocument(uri, "v10", 10);
    manager->updateDocument(uri, "v100", 100);

    // Assert
    const auto* doc = manager->getDocument(uri);
    EXPECT_EQ(doc->version, 100);
}

// ============================================================================
// RAII 및 메모리 안전성 테스트
// ============================================================================

TEST_F(DocumentManagerTest, ShouldNotLeakOnRepeatedOpenClose) {
    // AddressSanitizer로 메모리 누수 검증
    for (int i = 0; i < 1000; i++) {
        std::string uri = "file:///test" + std::to_string(i) + ".ksj";
        manager->openDocument(uri, "content", 1);
        manager->closeDocument(uri);
    }

    // Assert - 모두 닫혔는지 확인
    EXPECT_TRUE(manager->getAllDocuments().empty());
}

TEST_F(DocumentManagerTest, ShouldHandleMoveSemantics) {
    // Arrange
    manager->openDocument("file:///test.ksj", "content", 1);

    // Act - move assignment
    auto manager2 = std::make_unique<DocumentManager>();
    *manager2 = std::move(*manager);

    // Assert
    EXPECT_NE(manager2->getDocument("file:///test.ksj"), nullptr);
}

// ============================================================================
// 대용량 문서 테스트
// ============================================================================

TEST_F(DocumentManagerTest, ShouldHandleLargeDocument) {
    // Arrange - 10KB 문서
    std::string largeContent(10000, 'x');
    std::string uri = "file:///large.ksj";

    // Act
    manager->openDocument(uri, largeContent, 1);

    // Assert
    const auto* doc = manager->getDocument(uri);
    ASSERT_NE(doc, nullptr);
    EXPECT_EQ(doc->content.length(), 10000);
}

TEST_F(DocumentManagerTest, ShouldHandleEmptyDocument) {
    // Arrange
    std::string uri = "file:///empty.ksj";

    // Act
    manager->openDocument(uri, "", 1);

    // Assert
    const auto* doc = manager->getDocument(uri);
    ASSERT_NE(doc, nullptr);
    EXPECT_TRUE(doc->content.empty());
}

// ============================================================================
// 한글 문서 테스트
// ============================================================================

TEST_F(DocumentManagerTest, ShouldHandleKoreanContent) {
    // Arrange
    std::string uri = "file:///한글.ksj";
    std::string content = R"(
변수 이름 = "홍길동"
변수 나이 = 25
함수 인사하기() {
    출력("안녕하세요!")
}
)";

    // Act
    manager->openDocument(uri, content, 1);

    // Assert
    const auto* doc = manager->getDocument(uri);
    ASSERT_NE(doc, nullptr);
    EXPECT_TRUE(doc->content.find("홍길동") != std::string::npos);
    EXPECT_TRUE(doc->content.find("안녕하세요") != std::string::npos);
}
