/**
 * @file manual_lexer_test.cpp
 * @brief Lexer 수동 테스트 (GoogleTest 없이)
 */

#include <iostream>
#include <string>
#include "lexer/Lexer.h"
#include "lexer/Token.h"

using namespace kingsejong::lexer;

void printToken(const Token& token) {
    std::cout << "Type: " << tokenTypeToString(token.type)
              << ", Literal: \"" << token.literal << "\"" << std::endl;
}

void testBasicTokens() {
    std::cout << "\n=== Test 1: 기본 토큰 ===" << std::endl;
    std::string input = "123 + 456";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    printToken(tok1);

    Token tok2 = lexer.nextToken();
    printToken(tok2);

    Token tok3 = lexer.nextToken();
    printToken(tok3);
}

void testKoreanKeywords() {
    std::cout << "\n=== Test 2: 한글 키워드 ===" << std::endl;
    std::string input = "정수 나이 = 15";
    Lexer lexer(input);

    while (true) {
        Token tok = lexer.nextToken();
        printToken(tok);
        if (tok.type == TokenType::EOF_TOKEN) break;
    }
}

void testJosa() {
    std::cout << "\n=== Test 3: 조사 인식 ===" << std::endl;
    std::string input = "배열을 정렬하고";
    Lexer lexer(input);

    while (true) {
        Token tok = lexer.nextToken();
        printToken(tok);
        if (tok.type == TokenType::EOF_TOKEN) break;
    }
}

void testRangeExpression() {
    std::cout << "\n=== Test 4: 범위 표현 ===" << std::endl;
    std::string input = "1부터 10까지";
    Lexer lexer(input);

    while (true) {
        Token tok = lexer.nextToken();
        printToken(tok);
        if (tok.type == TokenType::EOF_TOKEN) break;
    }
}

void testString() {
    std::cout << "\n=== Test 5: 문자열 리터럴 ===" << std::endl;
    std::string input = "\"안녕하세요\"";
    Lexer lexer(input);

    Token tok = lexer.nextToken();
    printToken(tok);
}

void testRealCode() {
    std::cout << "\n=== Test 6: 실제 코드 스니펫 ===" << std::endl;
    std::string input = R"(
정수 합 = 0
i가 1부터 10까지 반복 {
    합 = 합 + i
}
    )";

    Lexer lexer(input);

    int count = 0;
    while (true) {
        Token tok = lexer.nextToken();
        if (tok.type == TokenType::EOF_TOKEN) break;

        std::cout << "[" << ++count << "] ";
        printToken(tok);
    }

    std::cout << "Total tokens: " << count << std::endl;
}

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "KingSejong Lexer 수동 테스트" << std::endl;
    std::cout << "==================================" << std::endl;

    try {
        testBasicTokens();
        testKoreanKeywords();
        testJosa();
        testRangeExpression();
        testString();
        testRealCode();

        std::cout << "\n==================================" << std::endl;
        std::cout << "✅ 모든 테스트 성공!" << std::endl;
        std::cout << "==================================" << std::endl;

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "\n❌ 테스트 실패: " << e.what() << std::endl;
        return 1;
    }
}
