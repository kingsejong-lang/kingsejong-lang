/**
 * @file LexerTest.cpp
 * @brief Lexer 클래스 테스트
 * @author KingSejong Team
 * @date 2025-11-06
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "lexer/Token.h"

using namespace kingsejong::lexer;

/**
 * @test 기본 정수 토큰화 테스트
 */
TEST(LexerTest, TokenizeInteger)
{
    std::string input = "123 456 789";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    EXPECT_EQ(tok1.type, TokenType::INTEGER);
    EXPECT_EQ(tok1.literal, "123");

    Token tok2 = lexer.nextToken();
    EXPECT_EQ(tok2.type, TokenType::INTEGER);
    EXPECT_EQ(tok2.literal, "456");

    Token tok3 = lexer.nextToken();
    EXPECT_EQ(tok3.type, TokenType::INTEGER);
    EXPECT_EQ(tok3.literal, "789");

    Token tokEOF = lexer.nextToken();
    EXPECT_EQ(tokEOF.type, TokenType::EOF_TOKEN);
}

/**
 * @test 실수 토큰화 테스트
 */
TEST(LexerTest, TokenizeFloat)
{
    std::string input = "3.14 2.718 0.5";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    EXPECT_EQ(tok1.type, TokenType::FLOAT);
    EXPECT_EQ(tok1.literal, "3.14");

    Token tok2 = lexer.nextToken();
    EXPECT_EQ(tok2.type, TokenType::FLOAT);
    EXPECT_EQ(tok2.literal, "2.718");

    Token tok3 = lexer.nextToken();
    EXPECT_EQ(tok3.type, TokenType::FLOAT);
    EXPECT_EQ(tok3.literal, "0.5");
}

/**
 * @test 산술 연산자 토큰화 테스트
 */
TEST(LexerTest, TokenizeArithmeticOperators)
{
    std::string input = "+ - * / %";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::PLUS);
    EXPECT_EQ(lexer.nextToken().type, TokenType::MINUS);
    EXPECT_EQ(lexer.nextToken().type, TokenType::ASTERISK);
    EXPECT_EQ(lexer.nextToken().type, TokenType::SLASH);
    EXPECT_EQ(lexer.nextToken().type, TokenType::PERCENT);
}

/**
 * @test 비교 연산자 토큰화 테스트
 */
TEST(LexerTest, TokenizeComparisonOperators)
{
    std::string input = "== != < > <= >=";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::EQ);
    EXPECT_EQ(lexer.nextToken().type, TokenType::NOT_EQ);
    EXPECT_EQ(lexer.nextToken().type, TokenType::LT);
    EXPECT_EQ(lexer.nextToken().type, TokenType::GT);
    EXPECT_EQ(lexer.nextToken().type, TokenType::LE);
    EXPECT_EQ(lexer.nextToken().type, TokenType::GE);
}

/**
 * @test 논리 연산자 토큰화 테스트
 */
TEST(LexerTest, TokenizeLogicalOperators)
{
    std::string input = "! && ||";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::NOT);
    EXPECT_EQ(lexer.nextToken().type, TokenType::AND);
    EXPECT_EQ(lexer.nextToken().type, TokenType::OR);
}

/**
 * @test 할당 연산자 토큰화 테스트
 */
TEST(LexerTest, TokenizeAssignment)
{
    std::string input = "x = 5";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    EXPECT_EQ(tok1.type, TokenType::IDENTIFIER);
    EXPECT_EQ(tok1.literal, "x");

    Token tok2 = lexer.nextToken();
    EXPECT_EQ(tok2.type, TokenType::ASSIGN);

    Token tok3 = lexer.nextToken();
    EXPECT_EQ(tok3.type, TokenType::INTEGER);
    EXPECT_EQ(tok3.literal, "5");
}

/**
 * @test 괄호 토큰화 테스트
 */
TEST(LexerTest, TokenizeParentheses)
{
    std::string input = "( ) { } [ ]";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::LPAREN);
    EXPECT_EQ(lexer.nextToken().type, TokenType::RPAREN);
    EXPECT_EQ(lexer.nextToken().type, TokenType::LBRACE);
    EXPECT_EQ(lexer.nextToken().type, TokenType::RBRACE);
    EXPECT_EQ(lexer.nextToken().type, TokenType::LBRACKET);
    EXPECT_EQ(lexer.nextToken().type, TokenType::RBRACKET);
}

/**
 * @test 구분자 토큰화 테스트
 */
TEST(LexerTest, TokenizeDelimiters)
{
    std::string input = ", ; :";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::COMMA);
    EXPECT_EQ(lexer.nextToken().type, TokenType::SEMICOLON);
    EXPECT_EQ(lexer.nextToken().type, TokenType::COLON);
}

/**
 * @test 문자열 리터럴 토큰화 테스트 (큰따옴표)
 */
TEST(LexerTest, TokenizeStringDoubleQuote)
{
    std::string input = "\"안녕하세요\"";
    Lexer lexer(input);

    Token tok = lexer.nextToken();
    EXPECT_EQ(tok.type, TokenType::STRING);
    EXPECT_EQ(tok.literal, "안녕하세요");
}

/**
 * @test 문자열 리터럴 토큰화 테스트 (작은따옴표)
 */
TEST(LexerTest, TokenizeStringSingleQuote)
{
    std::string input = "'반갑습니다'";
    Lexer lexer(input);

    Token tok = lexer.nextToken();
    EXPECT_EQ(tok.type, TokenType::STRING);
    EXPECT_EQ(tok.literal, "반갑습니다");
}

/**
 * @test 이스케이프 시퀀스 테스트
 */
TEST(LexerTest, TokenizeStringEscape)
{
    std::string input = "\"첫줄\\n둘째줄\\t탭\"";
    Lexer lexer(input);

    Token tok = lexer.nextToken();
    EXPECT_EQ(tok.type, TokenType::STRING);
    EXPECT_EQ(tok.literal, "첫줄\n둘째줄\t탭");
}

/**
 * @test 한글 조사 토큰화 테스트 - 을/를
 */
TEST(LexerTest, TokenizeJosaEulReul)
{
    std::string input = "을 를";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    EXPECT_EQ(tok1.type, TokenType::JOSA_EUL);
    EXPECT_EQ(tok1.literal, "을");

    Token tok2 = lexer.nextToken();
    EXPECT_EQ(tok2.type, TokenType::JOSA_REUL);
    EXPECT_EQ(tok2.literal, "를");
}

/**
 * @test 한글 조사 토큰화 테스트 - 이/가
 */
TEST(LexerTest, TokenizeJosaIGa)
{
    std::string input = "이 가";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    EXPECT_EQ(tok1.type, TokenType::JOSA_I);
    EXPECT_EQ(tok1.literal, "이");

    Token tok2 = lexer.nextToken();
    EXPECT_EQ(tok2.type, TokenType::JOSA_GA);
    EXPECT_EQ(tok2.literal, "가");
}

/**
 * @test 한글 조사 토큰화 테스트 - 은/는
 */
TEST(LexerTest, TokenizeJosaEunNeun)
{
    std::string input = "은 는";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    EXPECT_EQ(tok1.type, TokenType::JOSA_EUN);
    EXPECT_EQ(tok1.literal, "은");

    Token tok2 = lexer.nextToken();
    EXPECT_EQ(tok2.type, TokenType::JOSA_NEUN);
    EXPECT_EQ(tok2.literal, "는");
}

/**
 * @test 한글 조사 토큰화 테스트 - 의
 */
TEST(LexerTest, TokenizeJosaUi)
{
    std::string input = "의";
    Lexer lexer(input);

    Token tok = lexer.nextToken();
    EXPECT_EQ(tok.type, TokenType::JOSA_UI);
    EXPECT_EQ(tok.literal, "의");
}

/**
 * @test 한글 조사 토큰화 테스트 - 로/으로
 */
TEST(LexerTest, TokenizeJosaRoEuro)
{
    std::string input = "로 으로";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    EXPECT_EQ(tok1.type, TokenType::JOSA_RO);
    EXPECT_EQ(tok1.literal, "로");

    Token tok2 = lexer.nextToken();
    EXPECT_EQ(tok2.type, TokenType::JOSA_EURO);
    EXPECT_EQ(tok2.literal, "으로");
}

/**
 * @test 한글 조사 토큰화 테스트 - 에서, 에
 */
TEST(LexerTest, TokenizeJosaEsoE)
{
    std::string input = "에서 에";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    EXPECT_EQ(tok1.type, TokenType::JOSA_ESO);
    EXPECT_EQ(tok1.literal, "에서");

    Token tok2 = lexer.nextToken();
    EXPECT_EQ(tok2.type, TokenType::JOSA_E);
    EXPECT_EQ(tok2.literal, "에");
}

/**
 * @test 범위 키워드 토큰화 테스트
 */
TEST(LexerTest, TokenizeRangeKeywords)
{
    std::string input = "부터 까지 미만 초과 이하 이상";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::BUTEO);
    EXPECT_EQ(lexer.nextToken().type, TokenType::KKAJI);
    EXPECT_EQ(lexer.nextToken().type, TokenType::MIMAN);
    EXPECT_EQ(lexer.nextToken().type, TokenType::CHOGA);
    EXPECT_EQ(lexer.nextToken().type, TokenType::IHA);
    EXPECT_EQ(lexer.nextToken().type, TokenType::ISANG);
}

/**
 * @test 반복 키워드 토큰화 테스트
 */
TEST(LexerTest, TokenizeLoopKeywords)
{
    std::string input = "번 반복 각각";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::BEON);
    EXPECT_EQ(lexer.nextToken().type, TokenType::BANBOKK);
    EXPECT_EQ(lexer.nextToken().type, TokenType::GAKGAK);
}

/**
 * @test 제어문 키워드 토큰화 테스트
 */
TEST(LexerTest, TokenizeControlKeywords)
{
    std::string input = "만약 아니면 동안";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::MANYAK);
    EXPECT_EQ(lexer.nextToken().type, TokenType::ANIMYEON);
    EXPECT_EQ(lexer.nextToken().type, TokenType::DONGAN);
}

/**
 * @test 함수 키워드 토큰화 테스트
 */
TEST(LexerTest, TokenizeFunctionKeywords)
{
    std::string input = "함수 반환";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::HAMSU);
    EXPECT_EQ(lexer.nextToken().type, TokenType::BANHWAN);
}

/**
 * @test 타입 키워드 토큰화 테스트
 */
TEST(LexerTest, TokenizeTypeKeywords)
{
    std::string input = "정수 실수 문자 문자열 논리";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::JEONGSU);
    EXPECT_EQ(lexer.nextToken().type, TokenType::SILSU);
    EXPECT_EQ(lexer.nextToken().type, TokenType::MUNJA);
    EXPECT_EQ(lexer.nextToken().type, TokenType::MUNJAYEOL);
    EXPECT_EQ(lexer.nextToken().type, TokenType::NONLI);
}

/**
 * @test 불리언 리터럴 토큰화 테스트
 */
TEST(LexerTest, TokenizeBooleanLiterals)
{
    std::string input = "참 거짓";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::CHAM);
    EXPECT_EQ(lexer.nextToken().type, TokenType::GEOJIT);
}

/**
 * @test 메서드 체이닝 키워드 토큰화 테스트
 */
TEST(LexerTest, TokenizeMethodChainingKeywords)
{
    std::string input = "하고 하라";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::HAGO);
    EXPECT_EQ(lexer.nextToken().type, TokenType::HARA);
}

/**
 * @test 한글 식별자 토큰화 테스트
 */
TEST(LexerTest, TokenizeKoreanIdentifier)
{
    std::string input = "나이 이름 합계";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    EXPECT_EQ(tok1.type, TokenType::IDENTIFIER);
    EXPECT_EQ(tok1.literal, "나이");

    Token tok2 = lexer.nextToken();
    EXPECT_EQ(tok2.type, TokenType::IDENTIFIER);
    EXPECT_EQ(tok2.literal, "이름");

    Token tok3 = lexer.nextToken();
    EXPECT_EQ(tok3.type, TokenType::IDENTIFIER);
    EXPECT_EQ(tok3.literal, "합계");
}

/**
 * @test 영문 식별자 토큰화 테스트
 */
TEST(LexerTest, TokenizeEnglishIdentifier)
{
    std::string input = "variable count_items myVar";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    EXPECT_EQ(tok1.type, TokenType::IDENTIFIER);
    EXPECT_EQ(tok1.literal, "variable");

    Token tok2 = lexer.nextToken();
    EXPECT_EQ(tok2.type, TokenType::IDENTIFIER);
    EXPECT_EQ(tok2.literal, "count_items");

    Token tok3 = lexer.nextToken();
    EXPECT_EQ(tok3.type, TokenType::IDENTIFIER);
    EXPECT_EQ(tok3.literal, "myVar");
}

/**
 * @test 혼합 식별자 토큰화 테스트 (한글+영문)
 */
TEST(LexerTest, TokenizeMixedIdentifier)
{
    std::string input = "변수1 myVar2 숫자_개수";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    EXPECT_EQ(tok1.type, TokenType::IDENTIFIER);
    EXPECT_EQ(tok1.literal, "변수1");

    Token tok2 = lexer.nextToken();
    EXPECT_EQ(tok2.type, TokenType::IDENTIFIER);
    EXPECT_EQ(tok2.literal, "myVar2");

    Token tok3 = lexer.nextToken();
    EXPECT_EQ(tok3.type, TokenType::IDENTIFIER);
    EXPECT_EQ(tok3.literal, "숫자_개수");
}

/**
 * @test 종합 테스트 - 변수 선언
 */
TEST(LexerTest, TokenizeVariableDeclaration)
{
    std::string input = "정수 나이 = 15";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::JEONGSU);

    Token tok2 = lexer.nextToken();
    EXPECT_EQ(tok2.type, TokenType::IDENTIFIER);
    EXPECT_EQ(tok2.literal, "나이");

    EXPECT_EQ(lexer.nextToken().type, TokenType::ASSIGN);

    Token tok4 = lexer.nextToken();
    EXPECT_EQ(tok4.type, TokenType::INTEGER);
    EXPECT_EQ(tok4.literal, "15");
}

/**
 * @test 종합 테스트 - 조건문
 */
TEST(LexerTest, TokenizeIfStatement)
{
    std::string input = "만약 (나이 >= 18) { }";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().type, TokenType::MANYAK);
    EXPECT_EQ(lexer.nextToken().type, TokenType::LPAREN);

    Token tok3 = lexer.nextToken();
    EXPECT_EQ(tok3.type, TokenType::IDENTIFIER);
    EXPECT_EQ(tok3.literal, "나이");

    EXPECT_EQ(lexer.nextToken().type, TokenType::GE);

    Token tok5 = lexer.nextToken();
    EXPECT_EQ(tok5.type, TokenType::INTEGER);
    EXPECT_EQ(tok5.literal, "18");

    EXPECT_EQ(lexer.nextToken().type, TokenType::RPAREN);
    EXPECT_EQ(lexer.nextToken().type, TokenType::LBRACE);
    EXPECT_EQ(lexer.nextToken().type, TokenType::RBRACE);
}

/**
 * @test 종합 테스트 - 범위 표현
 */
TEST(LexerTest, TokenizeRangeExpression)
{
    std::string input = "1부터 10까지";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    EXPECT_EQ(tok1.type, TokenType::INTEGER);
    EXPECT_EQ(tok1.literal, "1");

    EXPECT_EQ(lexer.nextToken().type, TokenType::BUTEO);

    Token tok3 = lexer.nextToken();
    EXPECT_EQ(tok3.type, TokenType::INTEGER);
    EXPECT_EQ(tok3.literal, "10");

    EXPECT_EQ(lexer.nextToken().type, TokenType::KKAJI);
}

/**
 * @test 종합 테스트 - 조사 체인
 */
TEST(LexerTest, TokenizeJosaChain)
{
    std::string input = "배열을 정렬하고";
    Lexer lexer(input);

    Token tok1 = lexer.nextToken();
    EXPECT_EQ(tok1.type, TokenType::BAEYEOL);  // 배열 is now a keyword
    EXPECT_EQ(tok1.literal, "배열");

    Token tok2 = lexer.nextToken();
    EXPECT_EQ(tok2.type, TokenType::JOSA_EUL);
    EXPECT_EQ(tok2.literal, "을");

    Token tok3 = lexer.nextToken();
    EXPECT_EQ(tok3.type, TokenType::IDENTIFIER);
    EXPECT_EQ(tok3.literal, "정렬");

    Token tok4 = lexer.nextToken();
    EXPECT_EQ(tok4.type, TokenType::HAGO);
    EXPECT_EQ(tok4.literal, "하고");
}

/**
 * @test 종합 테스트 - 실제 코드 스니펫
 */
TEST(LexerTest, TokenizeRealCodeSnippet)
{
    std::string input = R"(
        정수 합 = 0
        i가 1부터 10까지 반복 {
            합 = 합 + i
        }
    )";

    Lexer lexer(input);

    // 정수 합 = 0
    EXPECT_EQ(lexer.nextToken().type, TokenType::JEONGSU);
    EXPECT_EQ(lexer.nextToken().literal, "합");
    EXPECT_EQ(lexer.nextToken().type, TokenType::ASSIGN);
    EXPECT_EQ(lexer.nextToken().literal, "0");

    // i가 1부터 10까지 반복 {
    EXPECT_EQ(lexer.nextToken().literal, "i");
    EXPECT_EQ(lexer.nextToken().type, TokenType::JOSA_GA);
    EXPECT_EQ(lexer.nextToken().literal, "1");
    EXPECT_EQ(lexer.nextToken().type, TokenType::BUTEO);
    EXPECT_EQ(lexer.nextToken().literal, "10");
    EXPECT_EQ(lexer.nextToken().type, TokenType::KKAJI);
    EXPECT_EQ(lexer.nextToken().type, TokenType::BANBOKK);
    EXPECT_EQ(lexer.nextToken().type, TokenType::LBRACE);

    // 합 = 합 + i
    EXPECT_EQ(lexer.nextToken().literal, "합");
    EXPECT_EQ(lexer.nextToken().type, TokenType::ASSIGN);
    EXPECT_EQ(lexer.nextToken().literal, "합");
    EXPECT_EQ(lexer.nextToken().type, TokenType::PLUS);
    EXPECT_EQ(lexer.nextToken().literal, "i");

    // }
    EXPECT_EQ(lexer.nextToken().type, TokenType::RBRACE);
    EXPECT_EQ(lexer.nextToken().type, TokenType::EOF_TOKEN);
}

// # 주석 테스트
TEST(LexerTest, HashCommentShouldBeIgnored)
{
    std::string input = "# 이것은 주석입니다\n출력(\"Hello\")";
    Lexer lexer(input);

    // 주석은 무시되고 출력부터 시작
    EXPECT_EQ(lexer.nextToken().literal, "출력");
    EXPECT_EQ(lexer.nextToken().type, TokenType::LPAREN);
    EXPECT_EQ(lexer.nextToken().literal, "Hello");
    EXPECT_EQ(lexer.nextToken().type, TokenType::RPAREN);
    EXPECT_EQ(lexer.nextToken().type, TokenType::EOF_TOKEN);
}

// // 주석 테스트
TEST(LexerTest, DoubleSlashCommentShouldBeIgnored)
{
    std::string input = "// 이것은 주석입니다\n출력(\"Hello\")";
    Lexer lexer(input);

    // 주석은 무시되고 출력부터 시작
    EXPECT_EQ(lexer.nextToken().literal, "출력");
    EXPECT_EQ(lexer.nextToken().type, TokenType::LPAREN);
    EXPECT_EQ(lexer.nextToken().literal, "Hello");
    EXPECT_EQ(lexer.nextToken().type, TokenType::RPAREN);
    EXPECT_EQ(lexer.nextToken().type, TokenType::EOF_TOKEN);
}

// 여러 줄 주석 테스트
TEST(LexerTest, MultipleCommentsShouldBeIgnored)
{
    std::string input = "// 첫 번째 주석\n# 두 번째 주석\n출력(\"Test\")\n// 세 번째 주석";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().literal, "출력");
    EXPECT_EQ(lexer.nextToken().type, TokenType::LPAREN);
    EXPECT_EQ(lexer.nextToken().literal, "Test");
    EXPECT_EQ(lexer.nextToken().type, TokenType::RPAREN);
    EXPECT_EQ(lexer.nextToken().type, TokenType::EOF_TOKEN);
}

// 코드와 주석 혼합 테스트
TEST(LexerTest, CodeWithInlineComments)
{
    std::string input = "x = 5 // 변수 할당\ny = 10 # 또 다른 변수";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().literal, "x");
    EXPECT_EQ(lexer.nextToken().type, TokenType::ASSIGN);
    EXPECT_EQ(lexer.nextToken().literal, "5");

    EXPECT_EQ(lexer.nextToken().literal, "y");
    EXPECT_EQ(lexer.nextToken().type, TokenType::ASSIGN);
    EXPECT_EQ(lexer.nextToken().literal, "10");

    EXPECT_EQ(lexer.nextToken().type, TokenType::EOF_TOKEN);
}

// 나눗셈과 주석 구분 테스트
TEST(LexerTest, SlashVsComment)
{
    std::string input = "a = 10 / 2\nb = 20 // 주석";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().literal, "a");
    EXPECT_EQ(lexer.nextToken().type, TokenType::ASSIGN);
    EXPECT_EQ(lexer.nextToken().literal, "10");
    EXPECT_EQ(lexer.nextToken().type, TokenType::SLASH);  // 나눗셈
    EXPECT_EQ(lexer.nextToken().literal, "2");

    EXPECT_EQ(lexer.nextToken().literal, "b");
    EXPECT_EQ(lexer.nextToken().type, TokenType::ASSIGN);
    EXPECT_EQ(lexer.nextToken().literal, "20");

    EXPECT_EQ(lexer.nextToken().type, TokenType::EOF_TOKEN);
}

// /* */ 멀티라인 주석 테스트
TEST(LexerTest, MultilineCommentShouldBeIgnored)
{
    std::string input = "/* 이것은\n여러 줄\n주석입니다 */\n출력(\"Hello\")";
    Lexer lexer(input);

    // 주석은 무시되고 출력부터 시작
    EXPECT_EQ(lexer.nextToken().literal, "출력");
    EXPECT_EQ(lexer.nextToken().type, TokenType::LPAREN);
    EXPECT_EQ(lexer.nextToken().literal, "Hello");
    EXPECT_EQ(lexer.nextToken().type, TokenType::RPAREN);
    EXPECT_EQ(lexer.nextToken().type, TokenType::EOF_TOKEN);
}

// 한 줄 멀티라인 주석 테스트
TEST(LexerTest, SingleLineMultilineComment)
{
    std::string input = "x = 5 /* 주석 */ + 3";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().literal, "x");
    EXPECT_EQ(lexer.nextToken().type, TokenType::ASSIGN);
    EXPECT_EQ(lexer.nextToken().literal, "5");
    EXPECT_EQ(lexer.nextToken().type, TokenType::PLUS);
    EXPECT_EQ(lexer.nextToken().literal, "3");
    EXPECT_EQ(lexer.nextToken().type, TokenType::EOF_TOKEN);
}

// 세 가지 주석 스타일 혼합
TEST(LexerTest, AllCommentStylesMixed)
{
    std::string input = "# 해시 주석\n// 슬래시 주석\n/* 멀티라인\n주석 */\n출력(\"Test\")";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().literal, "출력");
    EXPECT_EQ(lexer.nextToken().type, TokenType::LPAREN);
    EXPECT_EQ(lexer.nextToken().literal, "Test");
    EXPECT_EQ(lexer.nextToken().type, TokenType::RPAREN);
    EXPECT_EQ(lexer.nextToken().type, TokenType::EOF_TOKEN);
}

// 코드 사이 멀티라인 주석
TEST(LexerTest, MultilineCommentBetweenCode)
{
    std::string input = "a = 10\n/* 주석\n여러 줄 */\nb = 20";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().literal, "a");
    EXPECT_EQ(lexer.nextToken().type, TokenType::ASSIGN);
    EXPECT_EQ(lexer.nextToken().literal, "10");

    EXPECT_EQ(lexer.nextToken().literal, "b");
    EXPECT_EQ(lexer.nextToken().type, TokenType::ASSIGN);
    EXPECT_EQ(lexer.nextToken().literal, "20");

    EXPECT_EQ(lexer.nextToken().type, TokenType::EOF_TOKEN);
}

// 곱셈과 멀티라인 주석 구분
TEST(LexerTest, MultiplyVsMultilineComment)
{
    std::string input = "x = 5 * 2\ny = 3 /* 주석 */";
    Lexer lexer(input);

    EXPECT_EQ(lexer.nextToken().literal, "x");
    EXPECT_EQ(lexer.nextToken().type, TokenType::ASSIGN);
    EXPECT_EQ(lexer.nextToken().literal, "5");
    EXPECT_EQ(lexer.nextToken().type, TokenType::ASTERISK);  // 곱셈
    EXPECT_EQ(lexer.nextToken().literal, "2");

    EXPECT_EQ(lexer.nextToken().literal, "y");
    EXPECT_EQ(lexer.nextToken().type, TokenType::ASSIGN);
    EXPECT_EQ(lexer.nextToken().literal, "3");

    EXPECT_EQ(lexer.nextToken().type, TokenType::EOF_TOKEN);
}
