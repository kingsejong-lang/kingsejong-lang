/**
 * @file CompletionProviderTest.cpp
 * @brief CompletionProvider 테스트
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * TDD 방식: 테스트를 먼저 작성하고 구현합니다.
 */

#include <gtest/gtest.h>
#include "lsp/CompletionProvider.h"
#include "lsp/DocumentManager.h"
#include <algorithm>

using namespace kingsejong::lsp;

/**
 * @class CompletionProviderTest
 * @brief CompletionProvider 테스트 픽스처
 */
class CompletionProviderTest : public ::testing::Test {
protected:
    void SetUp() override {
        provider = std::make_unique<CompletionProvider>();
    }

    // Helper: 라벨로 완성 항목 찾기
    bool hasCompletion(const std::vector<CompletionProvider::CompletionItem>& items,
                      const std::string& label)
    {
        return std::any_of(items.begin(), items.end(),
            [&label](const auto& item) { return item.label == label; });
    }

    // Helper: 특정 종류의 항목 개수 세기
    int countKind(const std::vector<CompletionProvider::CompletionItem>& items,
                  CompletionProvider::CompletionItemKind kind)
    {
        return std::count_if(items.begin(), items.end(),
            [kind](const auto& item) { return item.kind == kind; });
    }

    std::unique_ptr<CompletionProvider> provider;
};

// ============================================================================
// 키워드 자동 완성 테스트
// ============================================================================

TEST_F(CompletionProviderTest, ShouldProvideKeywordCompletions) {
    // Arrange
    DocumentManager::Document doc("file:///test.ksj", "", 1);

    // Act
    auto items = provider->provideCompletions(doc, 0, 0);

    // Assert - 키워드가 포함되어야 함
    EXPECT_TRUE(hasCompletion(items, "변수"));
    EXPECT_TRUE(hasCompletion(items, "상수"));
    EXPECT_TRUE(hasCompletion(items, "함수"));
    EXPECT_TRUE(hasCompletion(items, "만약"));
    EXPECT_TRUE(hasCompletion(items, "반복"));
}

TEST_F(CompletionProviderTest, ShouldIncludeAllBasicKeywords) {
    // Arrange
    DocumentManager::Document doc("file:///test.ksj", "", 1);

    // Act
    auto items = provider->provideCompletions(doc, 0, 0);

    // Assert - 모든 기본 키워드 확인
    EXPECT_TRUE(hasCompletion(items, "변수"));
    EXPECT_TRUE(hasCompletion(items, "상수"));
    EXPECT_TRUE(hasCompletion(items, "함수"));
    EXPECT_TRUE(hasCompletion(items, "반환"));
    EXPECT_TRUE(hasCompletion(items, "만약"));
    EXPECT_TRUE(hasCompletion(items, "아니면"));
    EXPECT_TRUE(hasCompletion(items, "반복"));
    EXPECT_TRUE(hasCompletion(items, "계속"));
    EXPECT_TRUE(hasCompletion(items, "중단"));
    EXPECT_TRUE(hasCompletion(items, "출력"));
    EXPECT_TRUE(hasCompletion(items, "참"));
    EXPECT_TRUE(hasCompletion(items, "거짓"));
    EXPECT_TRUE(hasCompletion(items, "없음"));
}

TEST_F(CompletionProviderTest, KeywordsShouldHaveCorrectKind) {
    // Arrange
    DocumentManager::Document doc("file:///test.ksj", "", 1);

    // Act
    auto items = provider->provideCompletions(doc, 0, 0);

    // Assert - 키워드는 Keyword kind여야 함
    auto keywordItem = std::find_if(items.begin(), items.end(),
        [](const auto& item) { return item.label == "변수"; });

    ASSERT_NE(keywordItem, items.end());
    EXPECT_EQ(keywordItem->kind, CompletionProvider::CompletionItemKind::Keyword);
}

TEST_F(CompletionProviderTest, ShouldProvideKeywordDetails) {
    // Arrange
    DocumentManager::Document doc("file:///test.ksj", "", 1);

    // Act
    auto items = provider->provideCompletions(doc, 0, 0);

    // Assert - 키워드에 상세 설명이 있어야 함
    auto varItem = std::find_if(items.begin(), items.end(),
        [](const auto& item) { return item.label == "변수"; });

    ASSERT_NE(varItem, items.end());
    EXPECT_FALSE(varItem->detail.empty());
}

// ============================================================================
// 변수명 자동 완성 테스트
// ============================================================================

TEST_F(CompletionProviderTest, ShouldProvideVariableCompletions) {
    // Arrange
    std::string content = R"(
변수 이름 = "홍길동"
변수 나이 = 25
변수 주소 = "서울"
)";
    DocumentManager::Document doc("file:///test.ksj", content, 1);

    // Act
    auto items = provider->provideCompletions(doc, 4, 0);

    // Assert - 변수명이 포함되어야 함
    EXPECT_TRUE(hasCompletion(items, "이름"));
    EXPECT_TRUE(hasCompletion(items, "나이"));
    EXPECT_TRUE(hasCompletion(items, "주소"));
}

TEST_F(CompletionProviderTest, VariablesShouldHaveCorrectKind) {
    // Arrange
    std::string content = "변수 x = 10";
    DocumentManager::Document doc("file:///test.ksj", content, 1);

    // Act
    auto items = provider->provideCompletions(doc, 1, 0);

    // Assert
    auto varItem = std::find_if(items.begin(), items.end(),
        [](const auto& item) { return item.label == "x"; });

    ASSERT_NE(varItem, items.end());
    EXPECT_EQ(varItem->kind, CompletionProvider::CompletionItemKind::Variable);
}

TEST_F(CompletionProviderTest, ShouldNotDuplicateVariables) {
    // Arrange - 같은 변수가 여러 번 선언됨
    std::string content = R"(
변수 x = 10
변수 y = 20
변수 x = 30
)";
    DocumentManager::Document doc("file:///test.ksj", content, 1);

    // Act
    auto items = provider->provideCompletions(doc, 4, 0);

    // Assert - x는 한 번만 나와야 함
    int xCount = std::count_if(items.begin(), items.end(),
        [](const auto& item) { return item.label == "x"; });
    EXPECT_EQ(xCount, 1);
}

TEST_F(CompletionProviderTest, ShouldHandleKoreanVariableNames) {
    // Arrange
    std::string content = R"(
변수 한글변수 = 100
변수 숫자123 = 200
변수 _언더스코어 = 300
)";
    DocumentManager::Document doc("file:///test.ksj", content, 1);

    // Act
    auto items = provider->provideCompletions(doc, 4, 0);

    // Assert
    EXPECT_TRUE(hasCompletion(items, "한글변수"));
    EXPECT_TRUE(hasCompletion(items, "숫자123"));
    EXPECT_TRUE(hasCompletion(items, "_언더스코어"));
}

// ============================================================================
// 함수명 자동 완성 테스트
// ============================================================================

TEST_F(CompletionProviderTest, ShouldProvideFunctionCompletions) {
    // Arrange
    std::string content = R"(
함수 더하기(a, b) {
    반환 a + b
}

함수 곱하기(x, y) {
    반환 x * y
}
)";
    DocumentManager::Document doc("file:///test.ksj", content, 1);

    // Act
    auto items = provider->provideCompletions(doc, 8, 0);

    // Assert
    EXPECT_TRUE(hasCompletion(items, "더하기"));
    EXPECT_TRUE(hasCompletion(items, "곱하기"));
}

TEST_F(CompletionProviderTest, FunctionsShouldHaveCorrectKind) {
    // Arrange
    std::string content = "함수 테스트() {}";
    DocumentManager::Document doc("file:///test.ksj", content, 1);

    // Act
    auto items = provider->provideCompletions(doc, 1, 0);

    // Assert
    auto funcItem = std::find_if(items.begin(), items.end(),
        [](const auto& item) { return item.label == "테스트"; });

    ASSERT_NE(funcItem, items.end());
    EXPECT_EQ(funcItem->kind, CompletionProvider::CompletionItemKind::Function);
}

TEST_F(CompletionProviderTest, ShouldProvideFunctionDetails) {
    // Arrange
    std::string content = "함수 계산(a, b) { 반환 a + b }";
    DocumentManager::Document doc("file:///test.ksj", content, 1);

    // Act
    auto items = provider->provideCompletions(doc, 1, 0);

    // Assert - 함수 시그니처가 detail에 포함되어야 함
    auto funcItem = std::find_if(items.begin(), items.end(),
        [](const auto& item) { return item.label == "계산"; });

    ASSERT_NE(funcItem, items.end());
    EXPECT_FALSE(funcItem->detail.empty());
    EXPECT_TRUE(funcItem->detail.find("a") != std::string::npos);
    EXPECT_TRUE(funcItem->detail.find("b") != std::string::npos);
}

// ============================================================================
// 혼합 완성 테스트
// ============================================================================

TEST_F(CompletionProviderTest, ShouldProvideAllCompletionTypes) {
    // Arrange
    std::string content = R"(
변수 x = 10
함수 계산() { 반환 x }
)";
    DocumentManager::Document doc("file:///test.ksj", content, 1);

    // Act
    auto items = provider->provideCompletions(doc, 3, 0);

    // Assert - 키워드, 변수, 함수 모두 포함
    int keywordCount = countKind(items, CompletionProvider::CompletionItemKind::Keyword);
    int variableCount = countKind(items, CompletionProvider::CompletionItemKind::Variable);
    int functionCount = countKind(items, CompletionProvider::CompletionItemKind::Function);

    EXPECT_GT(keywordCount, 0);
    EXPECT_GT(variableCount, 0);
    EXPECT_GT(functionCount, 0);

    EXPECT_TRUE(hasCompletion(items, "변수"));   // 키워드
    EXPECT_TRUE(hasCompletion(items, "x"));      // 변수
    EXPECT_TRUE(hasCompletion(items, "계산"));   // 함수
}

TEST_F(CompletionProviderTest, ShouldHandleLargeDocument) {
    // Arrange - 많은 변수와 함수
    std::string content;
    for (int i = 0; i < 50; i++) {
        content += "변수 var" + std::to_string(i) + " = " + std::to_string(i) + "\n";
    }
    for (int i = 0; i < 30; i++) {
        content += "함수 func" + std::to_string(i) + "() {}\n";
    }
    DocumentManager::Document doc("file:///test.ksj", content, 1);

    // Act
    auto items = provider->provideCompletions(doc, 100, 0);

    // Assert - 모든 항목이 포함되어야 함
    EXPECT_TRUE(hasCompletion(items, "var0"));
    EXPECT_TRUE(hasCompletion(items, "var49"));
    EXPECT_TRUE(hasCompletion(items, "func0"));
    EXPECT_TRUE(hasCompletion(items, "func29"));
}

// ============================================================================
// 빈 문서 테스트
// ============================================================================

TEST_F(CompletionProviderTest, ShouldProvideKeywordsForEmptyDocument) {
    // Arrange
    DocumentManager::Document doc("file:///test.ksj", "", 1);

    // Act
    auto items = provider->provideCompletions(doc, 0, 0);

    // Assert - 키워드만 제공되어야 함
    int keywordCount = countKind(items, CompletionProvider::CompletionItemKind::Keyword);
    int variableCount = countKind(items, CompletionProvider::CompletionItemKind::Variable);
    int functionCount = countKind(items, CompletionProvider::CompletionItemKind::Function);

    EXPECT_GT(keywordCount, 0);
    EXPECT_EQ(variableCount, 0);
    EXPECT_EQ(functionCount, 0);
}

TEST_F(CompletionProviderTest, ShouldHandleMultilineVariables) {
    // Arrange
    std::string content = R"(
변수 a = 10
변수 b = 20

변수 c = 30
)";
    DocumentManager::Document doc("file:///test.ksj", content, 1);

    // Act
    auto items = provider->provideCompletions(doc, 5, 0);

    // Assert
    EXPECT_TRUE(hasCompletion(items, "a"));
    EXPECT_TRUE(hasCompletion(items, "b"));
    EXPECT_TRUE(hasCompletion(items, "c"));
}

// ============================================================================
// 상수 완성 테스트
// ============================================================================

TEST_F(CompletionProviderTest, ShouldProvideConstantCompletions) {
    // Arrange
    std::string content = R"(
상수 PI = 3.14
상수 MAX = 100
변수 x = 10
)";
    DocumentManager::Document doc("file:///test.ksj", content, 1);

    // Act
    auto items = provider->provideCompletions(doc, 4, 0);

    // Assert - 상수도 변수처럼 완성되어야 함
    EXPECT_TRUE(hasCompletion(items, "PI"));
    EXPECT_TRUE(hasCompletion(items, "MAX"));
    EXPECT_TRUE(hasCompletion(items, "x"));
}

// ============================================================================
// 내장 함수 완성 테스트
// ============================================================================

TEST_F(CompletionProviderTest, ShouldProvideBuiltinFunctions) {
    // Arrange
    DocumentManager::Document doc("file:///test.ksj", "", 1);

    // Act
    auto items = provider->provideCompletions(doc, 0, 0);

    // Assert - 내장 함수 (출력, 타입, 길이)
    EXPECT_TRUE(hasCompletion(items, "출력"));
    EXPECT_TRUE(hasCompletion(items, "타입"));
    EXPECT_TRUE(hasCompletion(items, "길이"));
}

// ============================================================================
// 위치 기반 완성 테스트 (향후 확장)
// ============================================================================

TEST_F(CompletionProviderTest, ShouldProvideCompletionsAtAnyPosition) {
    // Arrange
    std::string content = R"(
변수 x = 10
변수 y = 20
)";
    DocumentManager::Document doc("file:///test.ksj", content, 1);

    // Act - 다양한 위치에서 완성 요청
    auto items1 = provider->provideCompletions(doc, 0, 0);
    auto items2 = provider->provideCompletions(doc, 1, 5);
    auto items3 = provider->provideCompletions(doc, 2, 10);

    // Assert - 모든 위치에서 완성 제공
    EXPECT_FALSE(items1.empty());
    EXPECT_FALSE(items2.empty());
    EXPECT_FALSE(items3.empty());
}

TEST_F(CompletionProviderTest, ShouldHandleInvalidPosition) {
    // Arrange
    std::string content = "변수 x = 10";
    DocumentManager::Document doc("file:///test.ksj", content, 1);

    // Act - 범위를 벗어난 위치
    auto items = provider->provideCompletions(doc, 100, 100);

    // Assert - 에러 없이 완성 제공 (최소한 키워드)
    EXPECT_FALSE(items.empty());
    EXPECT_TRUE(hasCompletion(items, "변수"));
}
