/**
 * @file AsyncAwaitTest.cpp
 * @brief 비동기 프로그래밍 (async/await) 테스트
 * @author KingSejong Team
 * @date 2025-11-19
 *
 * Phase 7.3: Async/Await 기능 테스트
 */

#include <gtest/gtest.h>
#include "parser/Parser.h"
#include "lexer/Lexer.h"
#include "evaluator/Evaluator.h"
#include "semantic/SemanticAnalyzer.h"

using namespace kingsejong;

// ============================================================================
// 파싱 테스트
// ============================================================================

TEST(AsyncAwaitTest, ShouldParseAsyncFunctionWithNoParameters)
{
    std::string input = "비동기 함수() { 반환 42 }";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);

    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0) << "파서 에러 발생";
    ASSERT_EQ(program->statements().size(), 1);

    auto exprStmt = dynamic_cast<ast::ExpressionStatement*>(program->statements()[0].get());
    ASSERT_NE(exprStmt, nullptr);

    auto asyncFuncLit = dynamic_cast<ast::AsyncFunctionLiteral*>(
        const_cast<ast::Expression*>(exprStmt->expression()));
    ASSERT_NE(asyncFuncLit, nullptr);

    EXPECT_EQ(asyncFuncLit->parameters().size(), 0);
    EXPECT_NE(asyncFuncLit->body(), nullptr);
}

TEST(AsyncAwaitTest, ShouldParseAsyncFunctionWithParameters)
{
    std::string input = "비동기 함수(a, b) { 반환 a + b }";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);

    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0) << "파서 에러 발생";

    auto exprStmt = dynamic_cast<ast::ExpressionStatement*>(program->statements()[0].get());
    ASSERT_NE(exprStmt, nullptr);

    auto asyncFuncLit = dynamic_cast<ast::AsyncFunctionLiteral*>(
        const_cast<ast::Expression*>(exprStmt->expression()));
    ASSERT_NE(asyncFuncLit, nullptr);

    ASSERT_EQ(asyncFuncLit->parameters().size(), 2);
    EXPECT_EQ(asyncFuncLit->parameters()[0], "a");
    EXPECT_EQ(asyncFuncLit->parameters()[1], "b");
}

TEST(AsyncAwaitTest, ShouldParseAwaitExpression)
{
    std::string input = "대기 어떤함수()";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);

    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0) << "파서 에러 발생";

    auto exprStmt = dynamic_cast<ast::ExpressionStatement*>(program->statements()[0].get());
    ASSERT_NE(exprStmt, nullptr);

    auto awaitExpr = dynamic_cast<ast::AwaitExpression*>(
        const_cast<ast::Expression*>(exprStmt->expression()));
    ASSERT_NE(awaitExpr, nullptr);

    EXPECT_NE(awaitExpr->argument(), nullptr);
}

TEST(AsyncAwaitTest, ShouldParseAsyncFunctionAssignment)
{
    std::string input = R"(
테스트 = 비동기 함수() {
    반환 100
}
)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);

    auto program = parser.parseProgram();

    ASSERT_EQ(parser.errors().size(), 0) << "파서 에러 발생";
    ASSERT_EQ(program->statements().size(), 1);

    auto assignStmt = dynamic_cast<ast::AssignmentStatement*>(program->statements()[0].get());
    ASSERT_NE(assignStmt, nullptr);

    EXPECT_EQ(assignStmt->varName(), "테스트");

    auto asyncFuncLit = dynamic_cast<ast::AsyncFunctionLiteral*>(
        const_cast<ast::Expression*>(assignStmt->value()));
    ASSERT_NE(asyncFuncLit, nullptr);
}

// ============================================================================
// 의미 분석 테스트
// ============================================================================

TEST(AsyncAwaitTest, ShouldAllowAwaitInsideAsyncFunction)
{
    std::string input = R"(
테스트 = 비동기 함수() {
    값 = 대기 10
    반환 값
}
)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);
    ASSERT_EQ(parser.errors().size(), 0);

    semantic::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result) << "async 함수 내부의 await는 허용되어야 합니다";
    EXPECT_EQ(analyzer.errors().size(), 0);
}

TEST(AsyncAwaitTest, ShouldRejectAwaitOutsideAsyncFunction)
{
    std::string input = R"(
값 = 대기 어떤함수()
)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);
    ASSERT_EQ(parser.errors().size(), 0);

    semantic::SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result) << "async 함수 외부의 await는 에러가 발생해야 합니다";
    EXPECT_GE(analyzer.errors().size(), 1);

    // 에러 메시지 확인
    bool foundAwaitError = false;
    for (const auto& err : analyzer.errors())
    {
        if (err.message.find("대기") != std::string::npos ||
            err.message.find("비동기") != std::string::npos)
        {
            foundAwaitError = true;
            break;
        }
    }
    EXPECT_TRUE(foundAwaitError) << "await 관련 에러 메시지가 있어야 합니다";
}

// ============================================================================
// 평가 테스트
// ============================================================================

TEST(AsyncAwaitTest, ShouldReturnPromiseFromAsyncFunction)
{
    std::string input = R"(
비동기_함수 = 비동기 함수() {
    반환 42
}
결과 = 비동기_함수()
)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);
    ASSERT_EQ(parser.errors().size(), 0);

    evaluator::Evaluator eval;
    eval.eval(program.get());

    auto result = eval.environment()->get("결과");
    EXPECT_TRUE(result.isPromise()) << "async 함수는 Promise를 반환해야 합니다";

    if (result.isPromise())
    {
        auto promise = result.asPromise();
        EXPECT_TRUE(promise->isSettled());
        EXPECT_EQ(promise->value().asInteger(), 42);
    }
}

TEST(AsyncAwaitTest, ShouldExtractValueWithAwait)
{
    std::string input = R"(
비동기_함수 = 비동기 함수() {
    반환 100
}
결과 = 대기 비동기_함수()
)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    evaluator::Evaluator eval;
    eval.eval(program.get());

    auto result = eval.environment()->get("결과");
    EXPECT_TRUE(result.isInteger()) << "await는 Promise에서 값을 추출해야 합니다";
    EXPECT_EQ(result.asInteger(), 100);
}

TEST(AsyncAwaitTest, ShouldChainPromiseWithThen)
{
    std::string input = R"(
비동기_함수 = 비동기 함수() {
    반환 10
}
결과 = 비동기_함수().then(함수(값) {
    반환 값 * 2
})
)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    evaluator::Evaluator eval;
    eval.eval(program.get());

    auto result = eval.environment()->get("결과");
    EXPECT_TRUE(result.isPromise()) << "then은 새 Promise를 반환해야 합니다";

    if (result.isPromise())
    {
        auto promise = result.asPromise();
        EXPECT_TRUE(promise->isSettled());
        EXPECT_EQ(promise->value().asInteger(), 20);
    }
}

TEST(AsyncAwaitTest, ShouldChainPromiseWithKoreanMethodName)
{
    std::string input = R"(
비동기_함수 = 비동기 함수() {
    반환 5
}
결과 = 비동기_함수().그러면(함수(값) {
    반환 값 + 10
})
)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    evaluator::Evaluator eval;
    eval.eval(program.get());

    auto result = eval.environment()->get("결과");
    EXPECT_TRUE(result.isPromise());

    if (result.isPromise())
    {
        auto promise = result.asPromise();
        EXPECT_TRUE(promise->isSettled());
        EXPECT_EQ(promise->value().asInteger(), 15);
    }
}

TEST(AsyncAwaitTest, ShouldPassParametersToAsyncFunction)
{
    std::string input = R"(
덧셈 = 비동기 함수(a, b) {
    반환 a + b
}
결과 = 대기 덧셈(3, 7)
)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    evaluator::Evaluator eval;
    eval.eval(program.get());

    auto result = eval.environment()->get("결과");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 10);
}

TEST(AsyncAwaitTest, ShouldHandleNestedAwait)
{
    std::string input = R"(
내부함수 = 비동기 함수() {
    반환 50
}
외부함수 = 비동기 함수() {
    값 = 대기 내부함수()
    반환 값 * 2
}
결과 = 대기 외부함수()
)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    evaluator::Evaluator eval;
    eval.eval(program.get());

    auto result = eval.environment()->get("결과");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 100);
}

TEST(AsyncAwaitTest, ShouldReturnNonPromiseValueDirectly)
{
    std::string input = R"(
값 = 대기 42
)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    evaluator::Evaluator eval;
    eval.eval(program.get());

    // await가 Promise가 아닌 값을 받으면 그대로 반환
    auto result = eval.environment()->get("값");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 42);
}

TEST(AsyncAwaitTest, ShouldChainMultipleThen)
{
    std::string input = R"(
비동기_함수 = 비동기 함수() {
    반환 2
}
결과 = 비동기_함수().then(함수(값) {
    반환 값 * 3
}).then(함수(값) {
    반환 값 + 4
})
)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    evaluator::Evaluator eval;
    eval.eval(program.get());

    auto result = eval.environment()->get("결과");
    EXPECT_TRUE(result.isPromise());

    if (result.isPromise())
    {
        auto promise = result.asPromise();
        EXPECT_TRUE(promise->isSettled());
        // 2 * 3 = 6, 6 + 4 = 10
        EXPECT_EQ(promise->value().asInteger(), 10);
    }
}

TEST(AsyncAwaitTest, ShouldReturnStringFromAsyncFunction)
{
    std::string input = R"(
인사 = 비동기 함수() {
    반환 "안녕하세요"
}
결과 = 대기 인사()
)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    evaluator::Evaluator eval;
    eval.eval(program.get());

    auto result = eval.environment()->get("결과");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "안녕하세요");
}

TEST(AsyncAwaitTest, ShouldReturnBooleanFromAsyncFunction)
{
    std::string input = R"(
확인 = 비동기 함수() {
    반환 참
}
결과 = 대기 확인()
)";

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    evaluator::Evaluator eval;
    eval.eval(program.get());

    auto result = eval.environment()->get("결과");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_EQ(result.asBoolean(), true);
}
