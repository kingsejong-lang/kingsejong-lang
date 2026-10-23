/**
 * @file LspUtilsTest.cpp
 * @brief LspUtils 테스트
 * @author KingSejong Team
 * @date 2025-11-13
 */

#include <gtest/gtest.h>
#include "lsp/LspUtils.h"

using namespace kingsejong::lsp;

class LspUtilsTest : public ::testing::Test {};

TEST_F(LspUtilsTest, CharacterToByteOffsetASCII)
{
    std::string line = "hello world";

    EXPECT_EQ(characterToByteOffset(line, 0), 0u);   // 'h'
    EXPECT_EQ(characterToByteOffset(line, 5), 5u);   // ' '
    EXPECT_EQ(characterToByteOffset(line, 6), 6u);   // 'w'
}

TEST_F(LspUtilsTest, CharacterToByteOffsetKorean)
{
    std::string line = "정수 x";
    // UTF-8: 정(3) 수(3) 공백(1) x(1) = total 8 bytes
    // UTF-16: 정(1) 수(1) 공백(1) x(1) = total 4 code units

    EXPECT_EQ(characterToByteOffset(line, 0), 0u);   // '정' 시작
    EXPECT_EQ(characterToByteOffset(line, 1), 3u);   // '수' 시작
    EXPECT_EQ(characterToByteOffset(line, 2), 6u);   // 공백
    EXPECT_EQ(characterToByteOffset(line, 3), 7u);   // 'x'
}

TEST_F(LspUtilsTest, CharacterToByteOffsetMixed)
{
    std::string line = "정수 y = x + 5";
    // UTF-8: 정(3) 수(3) 공백(1) y(1) 공백(1) =(1) 공백(1) x(1) 공백(1) +(1) 공백(1) 5(1)
    // UTF-16: 정(1) 수(1) 공백(1) y(1) 공백(1) =(1) 공백(1) x(1) 공백(1) +(1) 공백(1) 5(1)

    std::cout << "Line: " << line << std::endl;
    std::cout << "Line length (bytes): " << line.length() << std::endl;

    // Character positions (UTF-16)
    EXPECT_EQ(characterToByteOffset(line, 0), 0u);   // '정'
    EXPECT_EQ(characterToByteOffset(line, 1), 3u);   // '수'
    EXPECT_EQ(characterToByteOffset(line, 2), 6u);   // ' '
    EXPECT_EQ(characterToByteOffset(line, 3), 7u);   // 'y'
    EXPECT_EQ(characterToByteOffset(line, 4), 8u);   // ' '
    EXPECT_EQ(characterToByteOffset(line, 5), 9u);   // '='
    EXPECT_EQ(characterToByteOffset(line, 6), 10u);  // ' '
    EXPECT_EQ(characterToByteOffset(line, 7), 11u);  // 'x'
    EXPECT_EQ(characterToByteOffset(line, 8), 12u);  // ' '
    EXPECT_EQ(characterToByteOffset(line, 9), 13u);  // '+'
}

TEST_F(LspUtilsTest, ExtractWordAtOffsetASCII)
{
    std::string line = "hello world test";

    EXPECT_EQ(extractWordAtOffset(line, 0), "hello");   // 'h'
    EXPECT_EQ(extractWordAtOffset(line, 2), "hello");   // 'l'
    EXPECT_EQ(extractWordAtOffset(line, 6), "world");   // 'w'
    EXPECT_EQ(extractWordAtOffset(line, 12), "test");   // 't'
}

TEST_F(LspUtilsTest, ExtractWordAtOffsetKorean)
{
    std::string line = "정수 x = 10";

    EXPECT_EQ(extractWordAtOffset(line, 0), "정수");    // '정'
    EXPECT_EQ(extractWordAtOffset(line, 3), "정수");    // '수'
    EXPECT_EQ(extractWordAtOffset(line, 7), "x");      // 'x'
}

TEST_F(LspUtilsTest, ExtractWordWithLspPosition)
{
    std::string line = "정수 y = x + 5";

    // Character 7 (UTF-16) = byte offset 11 = 'x'
    size_t offset = characterToByteOffset(line, 7);
    std::cout << "Character 7 -> byte offset: " << offset << std::endl;

    std::string word = extractWordAtOffset(line, offset);
    std::cout << "Word at offset " << offset << ": " << word << std::endl;

    EXPECT_EQ(word, "x");
}

TEST_F(LspUtilsTest, ExtractWordAtVariousPositions)
{
    std::string line = "정수 y = x + 5";

    // Test each character position
    for (int i = 0; i < 12; ++i) {
        size_t offset = characterToByteOffset(line, i);
        std::string word = extractWordAtOffset(line, offset);
        std::cout << "Char " << i << " (byte " << offset << "): '"
                  << word << "'" << std::endl;
    }
}

// ============================================================================
// UTF-16 보조 함수 (심볼 위치와 rename 범위에 쓴다)
// ============================================================================

TEST(LspUtilsUtf16Test, ByteColumnToUtf16CountsKoreanAsOneUnit)
{
    // "정수 x" : 정수 = 6바이트, 공백 1, x 는 바이트 열 8 (1-based) = UTF-16 3 (0-based)
    EXPECT_EQ(kingsejong::lsp::byteColumnToUtf16("정수 x = 10", 8), 3);
    EXPECT_EQ(kingsejong::lsp::byteColumnToUtf16("abc", 1), 0);
    EXPECT_EQ(kingsejong::lsp::byteColumnToUtf16("abc", 99), 3);
}

TEST(LspUtilsUtf16Test, Utf16LengthCountsSurrogatePairs)
{
    EXPECT_EQ(kingsejong::lsp::utf16Length("abc"), 3);
    EXPECT_EQ(kingsejong::lsp::utf16Length("변수"), 2);
    EXPECT_EQ(kingsejong::lsp::utf16Length("\xF0\x9F\x98\x80"), 2);  // 😀
}

TEST(LspUtilsUtf16Test, WordStartOffsetFindsIdentifierStart)
{
    std::string line = "정수 합계 = 1";
    size_t inside = 7 + 3;  // "합계" 의 두 번째 글자 안
    EXPECT_EQ(kingsejong::lsp::wordStartOffset(line, inside), 7u);
    EXPECT_EQ(kingsejong::lsp::wordStartOffset("x = 1", 0), 0u);
}
