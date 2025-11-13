/**
 * @file SourceCodeViewerTest.cpp
 * @brief SourceCodeViewer 테스트
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * TDD 방식: 테스트를 먼저 작성하고 구현합니다.
 */

#include <gtest/gtest.h>
#include "debugger/SourceCodeViewer.h"
#include <fstream>
#include <filesystem>

using namespace kingsejong::debugger;
namespace fs = std::filesystem;

/**
 * @class SourceCodeViewerTest
 * @brief SourceCodeViewer 테스트 픽스처
 */
class SourceCodeViewerTest : public ::testing::Test {
protected:
    void SetUp() override {
        viewer = std::make_unique<SourceCodeViewer>();

        // 테스트용 임시 파일 생성
        testFile = fs::temp_directory_path() / "test_source.ksj";
        createTestFile();
    }

    void TearDown() override {
        // 임시 파일 삭제
        if (fs::exists(testFile)) {
            fs::remove(testFile);
        }
    }

    void createTestFile() {
        std::ofstream file(testFile);
        file << "# 테스트 프로그램\n";           // 1
        file << "x = 10\n";                      // 2
        file << "y = 20\n";                      // 3
        file << "\n";                            // 4
        file << "함수 더하기(a, b) {\n";         // 5
        file << "    결과 = a + b\n";           // 6
        file << "    결과를 반환\n";             // 7
        file << "}\n";                           // 8
        file << "\n";                            // 9
        file << "z = 더하기(x, y)\n";           // 10
        file << "z를 출력\n";                    // 11
        file.close();
    }

    std::unique_ptr<SourceCodeViewer> viewer;
    fs::path testFile;
};

// ============================================================================
// 파일 로드 테스트
// ============================================================================

TEST_F(SourceCodeViewerTest, ShouldLoadFile) {
    // Act
    bool result = viewer->loadFile(testFile.string());

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(viewer->getLoadedFile(), testFile.string());
    EXPECT_EQ(viewer->getTotalLines(), 11u);
}

TEST_F(SourceCodeViewerTest, ShouldFailToLoadNonexistentFile) {
    // Act
    bool result = viewer->loadFile("/nonexistent/file.ksj");

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(viewer->getTotalLines(), 0u);
}

TEST_F(SourceCodeViewerTest, ShouldClearPreviousContentOnReload) {
    // Arrange
    viewer->loadFile(testFile.string());
    viewer->setCurrentLine(5);
    viewer->addBreakpoint(3);

    // Act - 다른 파일 로드
    viewer->loadFile(testFile.string());

    // Assert - 이전 상태는 유지됨 (파일 내용만 교체)
    EXPECT_EQ(viewer->getTotalLines(), 11u);
}

// ============================================================================
// 라인 가져오기 테스트
// ============================================================================

TEST_F(SourceCodeViewerTest, ShouldGetSingleLine) {
    // Arrange
    viewer->loadFile(testFile.string());

    // Act
    auto lines = viewer->getLines(2, 0);

    // Assert
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "x = 10");
}

TEST_F(SourceCodeViewerTest, ShouldGetLinesWithContext) {
    // Arrange
    viewer->loadFile(testFile.string());

    // Act - 3번 라인 주변 1줄씩 (2, 3, 4번 라인)
    auto lines = viewer->getLines(3, 1);

    // Assert
    ASSERT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[0], "x = 10");          // 2
    EXPECT_EQ(lines[1], "y = 20");          // 3
    EXPECT_EQ(lines[2], "");                // 4
}

TEST_F(SourceCodeViewerTest, ShouldGetLinesWithLargerContext) {
    // Arrange
    viewer->loadFile(testFile.string());

    // Act - 6번 라인 주변 2줄씩 (4, 5, 6, 7, 8번 라인)
    auto lines = viewer->getLines(6, 2);

    // Assert
    ASSERT_EQ(lines.size(), 5u);
    EXPECT_EQ(lines[0], "");                   // 4
    EXPECT_EQ(lines[1], "함수 더하기(a, b) {"); // 5
    EXPECT_EQ(lines[2], "    결과 = a + b");   // 6
    EXPECT_EQ(lines[3], "    결과를 반환");     // 7
    EXPECT_EQ(lines[4], "}");                  // 8
}

TEST_F(SourceCodeViewerTest, ShouldHandleBoundaryAtStart) {
    // Arrange
    viewer->loadFile(testFile.string());

    // Act - 1번 라인 주변 2줄 (경계 초과 방지)
    auto lines = viewer->getLines(1, 2);

    // Assert - 1, 2, 3번 라인만 반환
    ASSERT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[0], "# 테스트 프로그램");
    EXPECT_EQ(lines[1], "x = 10");
    EXPECT_EQ(lines[2], "y = 20");
}

TEST_F(SourceCodeViewerTest, ShouldHandleBoundaryAtEnd) {
    // Arrange
    viewer->loadFile(testFile.string());

    // Act - 11번 라인 주변 2줄 (경계 초과 방지)
    auto lines = viewer->getLines(11, 2);

    // Assert - 9, 10, 11번 라인만 반환
    ASSERT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[0], "");                // 9
    EXPECT_EQ(lines[1], "z = 더하기(x, y)"); // 10
    EXPECT_EQ(lines[2], "z를 출력");        // 11
}

TEST_F(SourceCodeViewerTest, ShouldReturnEmptyForInvalidLine) {
    // Arrange
    viewer->loadFile(testFile.string());

    // Act
    auto lines = viewer->getLines(100, 2);

    // Assert
    EXPECT_TRUE(lines.empty());
}

TEST_F(SourceCodeViewerTest, ShouldReturnEmptyWhenNoFileLoaded) {
    // Act
    auto lines = viewer->getLines(1, 2);

    // Assert
    EXPECT_TRUE(lines.empty());
}

// ============================================================================
// 현재 라인 테스트
// ============================================================================

TEST_F(SourceCodeViewerTest, ShouldSetCurrentLine) {
    // Act
    viewer->setCurrentLine(5);

    // Assert
    EXPECT_EQ(viewer->getCurrentLine(), 5);
}

TEST_F(SourceCodeViewerTest, ShouldStartWithNoCurrentLine) {
    // Assert
    EXPECT_EQ(viewer->getCurrentLine(), 0);
}

TEST_F(SourceCodeViewerTest, ShouldUpdateCurrentLine) {
    // Arrange
    viewer->setCurrentLine(5);

    // Act
    viewer->setCurrentLine(10);

    // Assert
    EXPECT_EQ(viewer->getCurrentLine(), 10);
}

// ============================================================================
// 브레이크포인트 테스트
// ============================================================================

TEST_F(SourceCodeViewerTest, ShouldAddBreakpoint) {
    // Act
    viewer->addBreakpoint(3);

    // Assert
    EXPECT_TRUE(viewer->hasBreakpoint(3));
}

TEST_F(SourceCodeViewerTest, ShouldAddMultipleBreakpoints) {
    // Act
    viewer->addBreakpoint(3);
    viewer->addBreakpoint(5);
    viewer->addBreakpoint(10);

    // Assert
    EXPECT_TRUE(viewer->hasBreakpoint(3));
    EXPECT_TRUE(viewer->hasBreakpoint(5));
    EXPECT_TRUE(viewer->hasBreakpoint(10));
    EXPECT_FALSE(viewer->hasBreakpoint(4));
}

TEST_F(SourceCodeViewerTest, ShouldRemoveBreakpoint) {
    // Arrange
    viewer->addBreakpoint(3);

    // Act
    viewer->removeBreakpoint(3);

    // Assert
    EXPECT_FALSE(viewer->hasBreakpoint(3));
}

TEST_F(SourceCodeViewerTest, ShouldHandleRemovingNonexistentBreakpoint) {
    // Act & Assert - 예외 없이 처리
    EXPECT_NO_THROW(viewer->removeBreakpoint(100));
}

TEST_F(SourceCodeViewerTest, ShouldNotHaveBreakpointByDefault) {
    // Assert
    EXPECT_FALSE(viewer->hasBreakpoint(1));
    EXPECT_FALSE(viewer->hasBreakpoint(5));
}

// ============================================================================
// 포맷팅 테스트
// ============================================================================

TEST_F(SourceCodeViewerTest, ShouldFormatSimpleLine) {
    // Arrange
    viewer->loadFile(testFile.string());

    // Act
    std::string formatted = viewer->format(2, 0);

    // Assert
    EXPECT_NE(formatted.find("2"), std::string::npos);  // 라인 번호 포함
    EXPECT_NE(formatted.find("x = 10"), std::string::npos);  // 내용 포함
}

TEST_F(SourceCodeViewerTest, ShouldFormatWithCurrentLineMarker) {
    // Arrange
    viewer->loadFile(testFile.string());
    viewer->setCurrentLine(3);

    // Act
    std::string formatted = viewer->format(3, 1);

    // Assert
    EXPECT_NE(formatted.find("→"), std::string::npos);  // 현재 라인 마커
    EXPECT_NE(formatted.find("y = 20"), std::string::npos);
}

TEST_F(SourceCodeViewerTest, ShouldFormatWithBreakpointMarker) {
    // Arrange
    viewer->loadFile(testFile.string());
    viewer->addBreakpoint(3);

    // Act
    std::string formatted = viewer->format(3, 1);

    // Assert
    EXPECT_NE(formatted.find("●"), std::string::npos);  // 브레이크포인트 마커
}

TEST_F(SourceCodeViewerTest, ShouldFormatWithBothMarkers) {
    // Arrange
    viewer->loadFile(testFile.string());
    viewer->setCurrentLine(3);
    viewer->addBreakpoint(3);

    // Act
    std::string formatted = viewer->format(3, 1);

    // Assert
    EXPECT_NE(formatted.find("→"), std::string::npos);  // 현재 라인 우선
}

TEST_F(SourceCodeViewerTest, ShouldFormatMultipleLines) {
    // Arrange
    viewer->loadFile(testFile.string());
    viewer->setCurrentLine(6);
    viewer->addBreakpoint(5);

    // Act
    std::string formatted = viewer->format(6, 2);

    // Assert
    EXPECT_NE(formatted.find("4"), std::string::npos);
    EXPECT_NE(formatted.find("5"), std::string::npos);
    EXPECT_NE(formatted.find("6"), std::string::npos);
    EXPECT_NE(formatted.find("7"), std::string::npos);
    EXPECT_NE(formatted.find("8"), std::string::npos);
    EXPECT_NE(formatted.find("●"), std::string::npos);  // 5번 라인
    EXPECT_NE(formatted.find("→"), std::string::npos);  // 6번 라인
}

TEST_F(SourceCodeViewerTest, ShouldReturnEmptyFormatWhenNoFileLoaded) {
    // Act
    std::string formatted = viewer->format(1, 2);

    // Assert
    EXPECT_TRUE(formatted.empty());
}

// ============================================================================
// clear() 테스트
// ============================================================================

TEST_F(SourceCodeViewerTest, ShouldClearAllState) {
    // Arrange
    viewer->loadFile(testFile.string());
    viewer->setCurrentLine(5);
    viewer->addBreakpoint(3);
    viewer->addBreakpoint(7);

    // Act
    viewer->clear();

    // Assert
    EXPECT_EQ(viewer->getTotalLines(), 0u);
    EXPECT_EQ(viewer->getCurrentLine(), 0);
    EXPECT_FALSE(viewer->hasBreakpoint(3));
    EXPECT_FALSE(viewer->hasBreakpoint(7));
    EXPECT_TRUE(viewer->getLoadedFile().empty());
}

// ============================================================================
// RAII 및 메모리 안전성 테스트
// ============================================================================

TEST_F(SourceCodeViewerTest, ShouldNotLeakOnRepeatedLoading) {
    // AddressSanitizer로 메모리 누수 검증
    for (int i = 0; i < 100; i++) {
        viewer->loadFile(testFile.string());
        viewer->setCurrentLine(i % 10 + 1);
        viewer->addBreakpoint(i % 5 + 1);
        viewer->format(5, 2);
    }

    // Assert
    SUCCEED();
}

TEST_F(SourceCodeViewerTest, ShouldHandleMoveSemantics) {
    // Arrange
    viewer->loadFile(testFile.string());
    viewer->setCurrentLine(5);
    viewer->addBreakpoint(3);

    // Act - move assignment
    auto viewer2 = std::make_unique<SourceCodeViewer>();
    *viewer2 = std::move(*viewer);

    // Assert
    EXPECT_EQ(viewer2->getTotalLines(), 11u);
    EXPECT_EQ(viewer2->getCurrentLine(), 5);
    EXPECT_TRUE(viewer2->hasBreakpoint(3));
}
