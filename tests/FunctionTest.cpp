/**
 * @file FunctionTest.cpp
 * @brief 함수 정의 및 호출 테스트
 * @author KingSejong Team
 * @date 2025-11-08
 */

#include <gtest/gtest.h>
#include "parser/Parser.h"
#include "lexer/Lexer.h"
#include "evaluator/Evaluator.h"

using namespace kingsejong;

// ============================================================================
// 함수 파싱 테스트
// ============================================================================

TEST(FunctionTest, ShouldParseFunctionWithNoParameters)
{
    std::string input = "함수() { 반환 42; }";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);

    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0) << "파서 에러 발생";
    ASSERT_EQ(program->statements().size(), 1);

    auto exprStmt = dynamic_cast<ast::ExpressionStatement*>(program->statements()[0].get());
    ASSERT_NE(exprStmt, nullptr);

    auto funcLit = dynamic_cast<ast::FunctionLiteral*>(const_cast<ast::Expression*>(exprStmt->expression()));
    ASSERT_NE(funcLit, nullptr);

    EXPECT_EQ(funcLit->parameters().size(), 0);
    EXPECT_NE(funcLit->body(), nullptr);
}

TEST(FunctionTest, ShouldParseFunctionWithParameters)
{
    std::string input = "함수(a, b, c) { 반환 a + b + c; }";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);

    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0) << "파서 에러 발생";
    ASSERT_EQ(program->statements().size(), 1);

    auto exprStmt = dynamic_cast<ast::ExpressionStatement*>(program->statements()[0].get());
    ASSERT_NE(exprStmt, nullptr);

    auto funcLit = dynamic_cast<ast::FunctionLiteral*>(const_cast<ast::Expression*>(exprStmt->expression()));
    ASSERT_NE(funcLit, nullptr);

    ASSERT_EQ(funcLit->parameters().size(), 3);
    EXPECT_EQ(funcLit->parameters()[0], "a");
    EXPECT_EQ(funcLit->parameters()[1], "b");
    EXPECT_EQ(funcLit->parameters()[2], "c");
}

TEST(FunctionTest, ShouldParseFunctionCall)
{
    std::string input = "덧셈(5, 3)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);

    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0) << "파서 에러 발생";
    ASSERT_EQ(program->statements().size(), 1);

    auto exprStmt = dynamic_cast<ast::ExpressionStatement*>(program->statements()[0].get());
    ASSERT_NE(exprStmt, nullptr);

    auto callExpr = dynamic_cast<ast::CallExpression*>(const_cast<ast::Expression*>(exprStmt->expression()));
    ASSERT_NE(callExpr, nullptr);

    EXPECT_EQ(callExpr->arguments().size(), 2);
}

// ============================================================================
// 함수 평가 테스트
// ============================================================================

TEST(FunctionTest, ShouldEvaluateFunctionLiteral)
{
    std::string input = "함수(a, b) { 반환 a + b; }";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0);

    evaluator::Evaluator evaluator;
    auto result = evaluator.eval(program.get());

    EXPECT_TRUE(result.isFunction());

    auto func = result.asFunction();
    EXPECT_EQ(func->parameters().size(), 2);
    EXPECT_EQ(func->parameters()[0], "a");
    EXPECT_EQ(func->parameters()[1], "b");
}

TEST(FunctionTest, ShouldCallFunctionWithNoParameters)
{
    std::string input = R"(
        정수 함수이름 = 함수() {
            반환 42;
        };
        함수이름()
    )";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0) << "파서 에러 발생";

    evaluator::Evaluator evaluator;
    auto result = evaluator.eval(program.get());

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 42);
}

TEST(FunctionTest, ShouldCallFunctionWithParameters)
{
    std::string input = R"(
        정수 덧셈 = 함수(a, b) {
            반환 a + b;
        };
        덧셈(5, 3)
    )";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0) << "파서 에러 발생";

    evaluator::Evaluator evaluator;
    auto result = evaluator.eval(program.get());

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 8);
}

TEST(FunctionTest, ShouldCallFunctionWithMultipleParameters)
{
    std::string input = R"(
        정수 곱셈 = 함수(a, b, c) {
            반환 a * b * c;
        };
        곱셈(2, 3, 4)
    )";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0);

    evaluator::Evaluator evaluator;
    auto result = evaluator.eval(program.get());

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 24);
}

TEST(FunctionTest, ShouldSupportClosure)
{
    std::string input = R"(
        정수 외부변수 = 10;
        정수 함수이름 = 함수(a) {
            반환 a + 외부변수;
        };
        함수이름(5)
    )";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0);

    evaluator::Evaluator evaluator;
    auto result = evaluator.eval(program.get());

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 15);
}

TEST(FunctionTest, ShouldSupportRecursion)
{
    std::string input = R"(
        정수 팩토리얼 = 함수(n) {
            만약 (n <= 1) {
                반환 1;
            }
            반환 n * 팩토리얼(n - 1);
        };
        팩토리얼(5)
    )";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0);

    evaluator::Evaluator evaluator;
    auto result = evaluator.eval(program.get());

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 120);  // 5! = 120
}

TEST(FunctionTest, ShouldThrowOnArgumentCountMismatch)
{
    std::string input = R"(
        정수 덧셈 = 함수(a, b) {
            반환 a + b;
        };
        덧셈(5)
    )";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0);

    evaluator::Evaluator evaluator;

    EXPECT_THROW({
        evaluator.eval(program.get());
    }, std::runtime_error);
}

TEST(FunctionTest, ShouldThrowOnCallingNonFunction)
{
    std::string input = R"(
        정수 숫자 = 42;
        숫자()
    )";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0);

    evaluator::Evaluator evaluator;

    EXPECT_THROW({
        evaluator.eval(program.get());
    }, std::runtime_error);
}

TEST(FunctionTest, ShouldSupportFibonacci)
{
    std::string input = R"(
        정수 피보나치 = 함수(n) {
            만약 (n <= 1) {
                반환 n;
            }
            반환 피보나치(n - 1) + 피보나치(n - 2);
        };
        피보나치(10)
    )";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0);

    evaluator::Evaluator evaluator;
    auto result = evaluator.eval(program.get());

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 55);  // 피보나치(10) = 55
}

TEST(FunctionTest, ShouldSupportNestedFunctions)
{
    std::string input = R"(
        정수 외부함수 = 함수(x) {
            정수 내부함수 = 함수(y) {
                반환 x + y;
            };
            반환 내부함수(10);
        };
        외부함수(5)
    )";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0);

    evaluator::Evaluator evaluator;
    auto result = evaluator.eval(program.get());

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 15);
}
