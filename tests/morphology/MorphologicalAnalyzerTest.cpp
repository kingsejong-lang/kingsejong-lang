/**
 * @file MorphologicalAnalyzerTest.cpp
 * @brief MorphologicalAnalyzer 클래스 단위 테스트
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include <gtest/gtest.h>
#include "morphology/MorphologicalAnalyzer.h"

using namespace kingsejong::morphology;

// ============================================================================
// 조사 분리 테스트
// ============================================================================

TEST(MorphologicalAnalyzerTest, ShouldSeparateOneCharacterJosa)
{
    MorphologicalAnalyzer analyzer;

    auto result = analyzer.analyze("배열을");

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].surface, "배열을");
    EXPECT_EQ(result[0].base, "배열");
    EXPECT_EQ(result[0].josa, "");
    EXPECT_EQ(result[1].surface, "배열을");
    EXPECT_EQ(result[1].base, "을");
    EXPECT_EQ(result[1].josa, "");
}

TEST(MorphologicalAnalyzerTest, ShouldSeparateTwoCharacterJosa)
{
    MorphologicalAnalyzer analyzer;

    auto result = analyzer.analyze("함수에서");

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].base, "함수");
    EXPECT_EQ(result[1].base, "에서");
}

TEST(MorphologicalAnalyzerTest, ShouldNotSeparateNounEnding이)
{
    MorphologicalAnalyzer analyzer;

    // "나이"는 명사이므로 분리하지 않음
    auto result = analyzer.analyze("나이");

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].base, "나이");
}

TEST(MorphologicalAnalyzerTest, ShouldNotSeparateBuiltinVar)
{
    MorphologicalAnalyzer analyzer;

    // "경로"는 builtin 변수이므로 분리하지 않음
    auto result = analyzer.analyze("경로");

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].base, "경로");
}

TEST(MorphologicalAnalyzerTest, ShouldNotSeparateBuiltinFunc)
{
    MorphologicalAnalyzer analyzer;

    // "디렉토리인가"는 builtin 함수이므로 분리하지 않음
    auto result = analyzer.analyze("디렉토리인가");

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].base, "디렉토리인가");
}

// ============================================================================
// 특수 케이스 테스트
// ============================================================================

TEST(MorphologicalAnalyzerTest, ShouldHandleUnknownWord)
{
    MorphologicalAnalyzer analyzer;

    // 사전에 없는 단어
    auto result = analyzer.analyze("알수없는단어");

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].base, "알수없는단어");
}

TEST(MorphologicalAnalyzerTest, ShouldHandleUnknownWordWithJosa)
{
    MorphologicalAnalyzer analyzer;

    // 사전에 없는 단어 + 조사
    auto result = analyzer.analyze("새단어를");

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].base, "새단어");
    EXPECT_EQ(result[1].base, "를");
}

TEST(MorphologicalAnalyzerTest, ShouldHandleJosaOnly)
{
    MorphologicalAnalyzer analyzer;

    // 조사만 있는 경우
    auto result = analyzer.analyze("을");

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].base, "을");
}

TEST(MorphologicalAnalyzerTest, ShouldHandleEmptyString)
{
    MorphologicalAnalyzer analyzer;

    auto result = analyzer.analyze("");

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].base, "");
}

// ============================================================================
// 언더스코어/숫자 뒤 명사 테스트
// ============================================================================

TEST(MorphologicalAnalyzerTest, ShouldNotSeparateAfterUnderscore)
{
    MorphologicalAnalyzer analyzer;

    // "_넓이"는 조사 분리하지 않음 (언더스코어 뒤)
    auto result = analyzer.analyze("원의_넓이");

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].base, "원의_넓이");
}

TEST(MorphologicalAnalyzerTest, ShouldNotSeparateAfterDigit)
{
    MorphologicalAnalyzer analyzer;

    // 숫자 뒤 한글은 조사 분리하지 않음
    auto result = analyzer.analyze("값1이");

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].base, "값1이");
}

// ============================================================================
// 복잡한 케이스 테스트
// ============================================================================

TEST(MorphologicalAnalyzerTest, ShouldHandleComplexCase1)
{
    MorphologicalAnalyzer analyzer;

    // "배열을" -> "배열" + "을"
    auto result = analyzer.analyze("배열을");

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].base, "배열");
    EXPECT_EQ(result[1].base, "을");
}

TEST(MorphologicalAnalyzerTest, ShouldHandleComplexCase2)
{
    MorphologicalAnalyzer analyzer;

    // "정렬하다" -> builtin 함수, 분리 안 함
    auto result = analyzer.analyze("정렬하다");

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].base, "정렬하다");
}

TEST(MorphologicalAnalyzerTest, ShouldHandleComplexCase3)
{
    MorphologicalAnalyzer analyzer;

    // "값을" -> "값" + "을"
    auto result = analyzer.analyze("값을");

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].base, "값");
    EXPECT_EQ(result[1].base, "을");
}

// ============================================================================
// 메모리 안전성 테스트
// ============================================================================

TEST(MorphologicalAnalyzerTest, ShouldNotLeakMemoryOnRepeatedOperations)
{
    // AddressSanitizer로 실행 시 메모리 누수 검출
    for (int i = 0; i < 1000; i++)
    {
        MorphologicalAnalyzer analyzer;
        analyzer.analyze("배열을");
        analyzer.analyze("함수에서");
        analyzer.analyze("나이");
        analyzer.analyze("디렉토리인가");
    }
    // RAII로 자동 정리됨
}

TEST(MorphologicalAnalyzerTest, ShouldHandleMultipleAnalyzers)
{
    // 여러 Analyzer 인스턴스가 독립적으로 작동해야 함
    MorphologicalAnalyzer analyzer1;
    MorphologicalAnalyzer analyzer2;

    auto result1 = analyzer1.analyze("배열을");
    auto result2 = analyzer2.analyze("함수를");

    ASSERT_EQ(result1.size(), 2);
    ASSERT_EQ(result2.size(), 2);
    EXPECT_EQ(result1[0].base, "배열");
    EXPECT_EQ(result2[0].base, "함수");
}

// ============================================================================
// UTF-8 한글 처리 테스트
// ============================================================================

TEST(MorphologicalAnalyzerTest, ShouldHandleUTF8KoreanCharacters)
{
    MorphologicalAnalyzer analyzer;

    // 3바이트 UTF-8 한글 문자 처리
    auto result = analyzer.analyze("한글을");

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].base, "한글");
    EXPECT_EQ(result[1].base, "을");
}
