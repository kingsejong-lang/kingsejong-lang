/**
 * @file ParserLL2Test.cpp
 * @brief Parser LL(2) Lookahead 기능 테스트
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include <gtest/gtest.h>
#include "parser/Parser.h"
#include "lexer/Lexer.h"

using namespace kingsejong;
using namespace kingsejong::parser;
using namespace kingsejong::lexer;

// ============================================================================
// LL(2) Lookahead 기본 기능 테스트
// ============================================================================

/**
 * @test peek2TokenIs() 헬퍼 함수 테스트
 *
 * peek2TokenIs()가 올바르게 2칸 앞의 토큰 타입을 확인하는지 검증
 */
TEST(ParserLL2Test, ShouldCheckPeek2Token)
{
    // Arrange - "함수 이름(x)"
    std::string input = "함수 이름(x)";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act & Assert
    // Parser 생성 직후:
    // curToken = "함수"
    // peekToken = "이름"
    // peekPeekToken = "("

    // NOTE: Parser는 private 메서드만 있으므로,
    // 실제로는 parseProgram()을 통해 간접적으로 검증해야 함
    // 이 테스트는 peekPeekToken_이 올바르게 초기화되는지 확인하는 용도

    auto program = parser.parseProgram();
    EXPECT_EQ(parser.errors().size(), 0);
    EXPECT_NE(program, nullptr);
}

/**
 * @test 함수 선언 vs 함수 호출 구분
 *
 * LL(2) lookahead를 통해 함수 선언과 함수 호출을 명확히 구분할 수 있는지 검증
 */
TEST(ParserLL2Test, ShouldDistinguishFunctionDeclarationFromCall)
{
    // Arrange - 함수 선언: "함수 더하기(a, b) { ... }"
    std::string input = R"(
        함수 더하기(a, b) {
            a + b
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    ASSERT_EQ(parser.errors().size(), 0);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    // 함수 선언이 할당문으로 파싱됨
    auto assignStmt = dynamic_cast<ast::AssignmentStatement*>(program->statements()[0].get());
    ASSERT_NE(assignStmt, nullptr);
    EXPECT_EQ(assignStmt->varName(), "더하기");

    // 함수 리터럴인지 확인
    auto funcLiteral = dynamic_cast<const ast::FunctionLiteral*>(assignStmt->value());
    ASSERT_NE(funcLiteral, nullptr);
    EXPECT_EQ(funcLiteral->parameters().size(), 2);
}

/**
 * @test 타입 키워드 다음 토큰 구분
 *
 * "정수 변수 = 값" vs "정수(값)" 구분
 */
TEST(ParserLL2Test, ShouldDistinguishTypeDeclarationFromTypeConversion)
{
    // Arrange - 변수 선언: "정수 숫자 = 42"
    std::string input1 = "정수 숫자 = 42;";
    Lexer lexer1(input1);
    Parser parser1(lexer1);

    // Act
    auto program1 = parser1.parseProgram();

    // Assert - 변수 선언으로 파싱됨
    ASSERT_EQ(parser1.errors().size(), 0);
    ASSERT_NE(program1, nullptr);
    ASSERT_EQ(program1->statements().size(), 1);

    auto varDecl = dynamic_cast<ast::VarDeclaration*>(program1->statements()[0].get());
    ASSERT_NE(varDecl, nullptr);
    EXPECT_EQ(varDecl->varName(), "숫자");
    EXPECT_EQ(varDecl->typeName(), "정수");

    // Arrange - 타입 변환 함수 호출: "정수(3.14)"
    std::string input2 = "정수(3.14)";
    Lexer lexer2(input2);
    Parser parser2(lexer2);

    // Act
    auto program2 = parser2.parseProgram();

    // Assert - 함수 호출 표현식으로 파싱됨
    ASSERT_EQ(parser2.errors().size(), 0);
    ASSERT_NE(program2, nullptr);
    ASSERT_EQ(program2->statements().size(), 1);

    auto exprStmt = dynamic_cast<ast::ExpressionStatement*>(program2->statements()[0].get());
    ASSERT_NE(exprStmt, nullptr);

    auto callExpr = dynamic_cast<const ast::CallExpression*>(exprStmt->expression());
    ASSERT_NE(callExpr, nullptr);
}

/**
 * @test 연속된 토큰 패턴 인식
 *
 * LL(2)로 3개 토큰 패턴을 인식할 수 있는지 검증
 */
TEST(ParserLL2Test, ShouldRecognizeThreeTokenPattern)
{
    // Arrange - "i가 1부터 5까지 반복한다 { ... }"
    std::string input = R"(
        i가 1부터 5까지 반복한다 {
            출력(i)
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    ASSERT_EQ(parser.errors().size(), 0);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto rangeFor = dynamic_cast<ast::RangeForStatement*>(program->statements()[0].get());
    ASSERT_NE(rangeFor, nullptr);
    EXPECT_EQ(rangeFor->varName(), "i");
}

// ============================================================================
// LL(2) 엣지 케이스 테스트
// ============================================================================

/**
 * @test 중첩 함수 선언
 *
 * 중첩된 함수 선언에서도 lookahead가 올바르게 동작하는지 검증
 */
TEST(ParserLL2Test, ShouldHandleNestedFunctionDeclarations)
{
    // Arrange
    std::string input = R"(
        함수 외부(x) {
            함수 내부(y) {
                x + y
            }
            내부(10)
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    ASSERT_EQ(parser.errors().size(), 0);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);
}

/**
 * @test 여러 문장 연속 파싱
 *
 * 여러 문장이 연속될 때도 lookahead가 올바르게 유지되는지 검증
 */
TEST(ParserLL2Test, ShouldMaintainLookaheadAcrossMultipleStatements)
{
    // Arrange
    std::string input = R"(
        정수 a = 10;
        정수 b = 20;
        함수 더하기(x, y) { x + y }
        더하기(a, b)
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    ASSERT_EQ(parser.errors().size(), 0);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 4);
}

/**
 * @test EOF 근처에서 lookahead
 *
 * 입력 끝 근처에서 peek2Token이 EOF를 올바르게 처리하는지 검증
 */
TEST(ParserLL2Test, ShouldHandlePeek2NearEOF)
{
    // Arrange - 짧은 입력
    std::string input = "42";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    ASSERT_EQ(parser.errors().size(), 0);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);
}
