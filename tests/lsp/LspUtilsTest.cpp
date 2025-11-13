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
