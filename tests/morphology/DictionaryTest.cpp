/**
 * @file DictionaryTest.cpp
 * @brief Dictionary 클래스 단위 테스트
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include <gtest/gtest.h>
#include "morphology/Dictionary.h"

using namespace kingsejong::morphology;

// ============================================================================
// 기본 기능 테스트
// ============================================================================

TEST(DictionaryTest, ShouldInitializeWithDefaultDictionary)
{
    Dictionary dict;

    // 조사 확인
    EXPECT_TRUE(dict.isJosa("을"));
    EXPECT_TRUE(dict.isJosa("를"));
    EXPECT_TRUE(dict.isJosa("이"));
    EXPECT_TRUE(dict.isJosa("가"));
    EXPECT_TRUE(dict.isJosa("에서"));
    EXPECT_TRUE(dict.isJosa("으로"));
}

TEST(DictionaryTest, ShouldRecognizeNouns)
{
    Dictionary dict;

    // 일반 명사
    EXPECT_TRUE(dict.isNoun("배열"));
    EXPECT_TRUE(dict.isNoun("함수"));
    EXPECT_TRUE(dict.isNoun("변수"));

    // 2글자 명사 (특수 케이스)
    EXPECT_TRUE(dict.isNoun("나이"));
    EXPECT_TRUE(dict.isNoun("거리"));
    EXPECT_TRUE(dict.isNoun("자리"));
}

TEST(DictionaryTest, ShouldRecognizeBuiltinVariables)
{
    Dictionary dict;

    EXPECT_TRUE(dict.isBuiltinVar("경로"));
    EXPECT_TRUE(dict.isBuiltinVar("절대경로"));
    EXPECT_TRUE(dict.isBuiltinVar("작업디렉토리"));
}

TEST(DictionaryTest, ShouldRecognizeBuiltinFunctions)
{
    Dictionary dict;

    EXPECT_TRUE(dict.isBuiltinFunc("디렉토리인가"));
    EXPECT_TRUE(dict.isBuiltinFunc("파일인가"));
    EXPECT_TRUE(dict.isBuiltinFunc("출력"));
    EXPECT_TRUE(dict.isBuiltinFunc("길이"));
    EXPECT_TRUE(dict.isBuiltinFunc("타입"));
}

TEST(DictionaryTest, ShouldReturnCorrectWordType)
{
    Dictionary dict;

    EXPECT_EQ(dict.getWordType("을"), WordType::JOSA);
    EXPECT_EQ(dict.getWordType("배열"), WordType::NOUN);
    EXPECT_EQ(dict.getWordType("경로"), WordType::BUILTIN_VAR);
    EXPECT_EQ(dict.getWordType("디렉토리인가"), WordType::BUILTIN_FUNC);
    EXPECT_EQ(dict.getWordType("길이"), WordType::BUILTIN_FUNC);
    EXPECT_EQ(dict.getWordType("unknown"), WordType::UNKNOWN);
}

// ============================================================================
// 단어 추가 테스트
// ============================================================================

TEST(DictionaryTest, ShouldAddNewNoun)
{
    Dictionary dict;

    EXPECT_FALSE(dict.isNoun("테스트"));

    dict.addNoun("테스트");

    EXPECT_TRUE(dict.isNoun("테스트"));
    EXPECT_EQ(dict.getWordType("테스트"), WordType::NOUN);
}

TEST(DictionaryTest, ShouldAddNewJosa)
{
    Dictionary dict;

    EXPECT_FALSE(dict.isJosa("커스텀조사"));

    dict.addJosa("커스텀조사");

    EXPECT_TRUE(dict.isJosa("커스텀조사"));
    EXPECT_EQ(dict.getWordType("커스텀조사"), WordType::JOSA);
}

TEST(DictionaryTest, ShouldAddNewBuiltinVar)
{
    Dictionary dict;

    EXPECT_FALSE(dict.isBuiltinVar("새변수"));

    dict.addBuiltinVar("새변수");

    EXPECT_TRUE(dict.isBuiltinVar("새변수"));
    EXPECT_EQ(dict.getWordType("새변수"), WordType::BUILTIN_VAR);
}

TEST(DictionaryTest, ShouldAddNewBuiltinFunc)
{
    Dictionary dict;

    EXPECT_FALSE(dict.isBuiltinFunc("새함수"));

    dict.addBuiltinFunc("새함수");

    EXPECT_TRUE(dict.isBuiltinFunc("새함수"));
    EXPECT_EQ(dict.getWordType("새함수"), WordType::BUILTIN_FUNC);
}

// ============================================================================
// 우선순위 테스트 (Builtin > Josa > Noun)
// ============================================================================

TEST(DictionaryTest, ShouldPrioritizeBuiltinFuncOverNoun)
{
    Dictionary dict;

    // "출력"은 builtin 함수이자 명사일 수 있음
    dict.addNoun("출력");
    dict.addBuiltinFunc("출력");

    // Builtin 함수가 우선
    EXPECT_EQ(dict.getWordType("출력"), WordType::BUILTIN_FUNC);
}

TEST(DictionaryTest, ShouldPrioritizeBuiltinVarOverNoun)
{
    Dictionary dict;

    dict.addNoun("경로");
    dict.addBuiltinVar("경로");

    // Builtin 변수가 우선
    EXPECT_EQ(dict.getWordType("경로"), WordType::BUILTIN_VAR);
}

// ============================================================================
// 부정 테스트 (존재하지 않는 단어)
// ============================================================================

TEST(DictionaryTest, ShouldReturnUnknownForUndefinedWord)
{
    Dictionary dict;

    EXPECT_FALSE(dict.isNoun("존재하지않는단어"));
    EXPECT_FALSE(dict.isJosa("존재하지않는조사"));
    EXPECT_FALSE(dict.isBuiltinVar("존재하지않는변수"));
    EXPECT_FALSE(dict.isBuiltinFunc("존재하지않는함수"));

    EXPECT_EQ(dict.getWordType("존재하지않는단어"), WordType::UNKNOWN);
}

// ============================================================================
// 특수 케이스 테스트
// ============================================================================

TEST(DictionaryTest, ShouldHandleEmptyString)
{
    Dictionary dict;

    EXPECT_FALSE(dict.isNoun(""));
    EXPECT_FALSE(dict.isJosa(""));
    EXPECT_EQ(dict.getWordType(""), WordType::UNKNOWN);
}

TEST(DictionaryTest, ShouldHandleMultiByteKoreanCharacters)
{
    Dictionary dict;

    // UTF-8 3바이트 한글 문자 처리
    dict.addNoun("한글");
    dict.addJosa("한테");

    EXPECT_TRUE(dict.isNoun("한글"));
    EXPECT_TRUE(dict.isJosa("한테"));
}

// ============================================================================
// 메모리 안전성 테스트
// ============================================================================

TEST(DictionaryTest, ShouldNotLeakMemoryOnRepeatedOperations)
{
    // AddressSanitizer로 실행 시 메모리 누수 검출
    for (int i = 0; i < 1000; i++)
    {
        Dictionary dict;
        dict.addNoun("테스트" + std::to_string(i));
        dict.addJosa("조사" + std::to_string(i));
        dict.addBuiltinVar("변수" + std::to_string(i));
        dict.addBuiltinFunc("함수" + std::to_string(i));

        dict.isNoun("테스트" + std::to_string(i));
        dict.getWordType("조사" + std::to_string(i));
    }
    // RAII로 자동 정리됨
}

TEST(DictionaryTest, ShouldHandleMultipleDictionaries)
{
    // 여러 Dictionary 인스턴스가 독립적으로 작동해야 함
    Dictionary dict1;
    Dictionary dict2;

    dict1.addNoun("단어1");
    dict2.addNoun("단어2");

    EXPECT_TRUE(dict1.isNoun("단어1"));
    EXPECT_FALSE(dict1.isNoun("단어2"));

    EXPECT_FALSE(dict2.isNoun("단어1"));
    EXPECT_TRUE(dict2.isNoun("단어2"));
}
