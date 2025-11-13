/**
 * @file LexerUnderscoreTest.cpp
 * @brief Test underscore support in identifiers
 * @author KingSejong Team
 * @date 2025-11-14
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include <vector>

using namespace kingsejong::lexer;

TEST(LexerUnderscoreTest, ShouldTokenizeUnderscoreInIdentifier)
{
    // Arrange
    std::string input = "파일_읽기";
    Lexer lexer(input);

    // Act - collect all tokens
    std::vector<Token> tokens;
    Token tok = lexer.nextToken();
    while (tok.type != TokenType::EOF_TOKEN) {
        tokens.push_back(tok);
        tok = lexer.nextToken();
    }

    // Assert - should be ONE token
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].literal, "파일_읽기");
}

TEST(LexerUnderscoreTest, ShouldTokenizeASCIIUnderscoreIdentifier)
{
    // Arrange
    std::string input = "test_value";
    Lexer lexer(input);

    // Act
    std::vector<Token> tokens;
    Token tok = lexer.nextToken();
    while (tok.type != TokenType::EOF_TOKEN) {
        tokens.push_back(tok);
        tok = lexer.nextToken();
    }

    // Assert
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].literal, "test_value");
}

TEST(LexerUnderscoreTest, ShouldTokenizeFunctionCall)
{
    // Arrange
    std::string input = "디렉토리_생성(\"/tmp\")";
    Lexer lexer(input);

    // Act
    std::vector<Token> tokens;
    Token tok = lexer.nextToken();
    while (tok.type != TokenType::EOF_TOKEN) {
        tokens.push_back(tok);
        tok = lexer.nextToken();
    }

    // Assert - should be: IDENTIFIER, LPAREN, STRING, RPAREN
    ASSERT_EQ(tokens.size(), 4u);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].literal, "디렉토리_생성");
    EXPECT_EQ(tokens[1].type, TokenType::LPAREN);
    EXPECT_EQ(tokens[2].type, TokenType::STRING);
    EXPECT_EQ(tokens[3].type, TokenType::RPAREN);
}
