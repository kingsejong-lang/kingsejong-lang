/**
 * @file BuiltinTest.cpp
 * @brief 내장 함수 테스트
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include <gtest/gtest.h>
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "parser/Parser.h"
#include "lexer/Lexer.h"
#include <sstream>

using namespace kingsejong;

// ============================================================================
// 헬퍼 함수 (anonymous namespace to avoid duplicate symbol)
// ============================================================================

namespace {

/**
 * @brief 코드를 파싱하고 평가하는 헬퍼 함수
 * @param input 입력 코드
 * @return 평가 결과 Value
 */
static evaluator::Value evalInput(const std::string& input)
{
    // 내장 함수 등록 (각 테스트마다 안전하게 호출)
    evaluator::Builtin::registerAllBuiltins();

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_EQ(parser.errors().size(), 0) << "파서 에러: "
        << (parser.errors().empty() ? "" : parser.errors()[0]);

    evaluator::Evaluator evaluator;
    return evaluator.evalProgram(program.get());
}

} // anonymous namespace

// ============================================================================
// 출력() 함수 테스트
// ============================================================================

TEST(BuiltinTest, ShouldPrintInteger)
{
    // stdout을 캡처하기 위한 버퍼 설정
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    evalInput("출력(42)");

    std::cout.rdbuf(old);  // 원래 상태로 복원
    EXPECT_EQ(buffer.str(), "42\n");
}

TEST(BuiltinTest, ShouldPrintString)
{
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    evalInput("출력(\"안녕하세요\")");

    std::cout.rdbuf(old);
    EXPECT_EQ(buffer.str(), "안녕하세요\n");
}

TEST(BuiltinTest, ShouldPrintMultipleValues)
{
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    evalInput("출력(1, 2, 3)");

    std::cout.rdbuf(old);
    EXPECT_EQ(buffer.str(), "1 2 3\n");
}

TEST(BuiltinTest, ShouldPrintMixedTypes)
{
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    evalInput("출력(\"나이:\", 25, \"세\")");

    std::cout.rdbuf(old);
    EXPECT_EQ(buffer.str(), "나이: 25 세\n");
}

TEST(BuiltinTest, ShouldPrintBoolean)
{
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    evalInput("출력(참, 거짓)");

    std::cout.rdbuf(old);
    EXPECT_EQ(buffer.str(), "참 거짓\n");
}

TEST(BuiltinTest, ShouldPrintNull)
{
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    // 변수 선언시 초기화하지 않으면 null
    evalInput("정수 x\n출력(x)");

    std::cout.rdbuf(old);
    EXPECT_EQ(buffer.str(), "없음\n");
}

// ============================================================================
// 타입() 함수 테스트
// ============================================================================

TEST(BuiltinTest, ShouldReturnTypeInteger)
{
    auto result = evalInput("타입(42)");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "정수");
}

TEST(BuiltinTest, ShouldReturnTypeFloat)
{
    auto result = evalInput("타입(3.14)");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "실수");
}

TEST(BuiltinTest, ShouldReturnTypeString)
{
    auto result = evalInput("타입(\"안녕\")");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "문자열");
}

TEST(BuiltinTest, ShouldReturnTypeBoolean)
{
    auto result = evalInput("타입(참)");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "논리");
}

TEST(BuiltinTest, ShouldReturnTypeNull)
{
    auto result = evalInput("정수 x\n타입(x)");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "없음");
}

TEST(BuiltinTest, ShouldReturnTypeFunction)
{
    auto result = evalInput("타입(함수(x) { x })");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "함수");
}

TEST(BuiltinTest, ShouldReturnTypeBuiltinFunction)
{
    auto result = evalInput("타입(출력)");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "내장함수");
}

TEST(BuiltinTest, ShouldThrowOnTypeArgumentCountMismatch)
{
    EXPECT_THROW(evalInput("타입(1, 2)"), std::runtime_error);
}

// ============================================================================
// 길이() 함수 테스트
// ============================================================================

TEST(BuiltinTest, ShouldReturnStringLength)
{
    auto result = evalInput("길이(\"안녕하세요\")");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 5);  // 5글자
}

TEST(BuiltinTest, ShouldReturnEmptyStringLength)
{
    auto result = evalInput("길이(\"\")");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 0);
}

TEST(BuiltinTest, ShouldReturnMixedStringLength)
{
    auto result = evalInput("길이(\"Hello세계\")");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 7);  // Hello(5) + 세계(2) = 7
}

TEST(BuiltinTest, ShouldThrowOnLengthArgumentCountMismatch)
{
    EXPECT_THROW(evalInput("길이(\"a\", \"b\")"), std::runtime_error);
}

TEST(BuiltinTest, ShouldThrowOnLengthWithInvalidType)
{
    EXPECT_THROW(evalInput("길이(42)"), std::runtime_error);
}

// ============================================================================
// 통합 테스트
// ============================================================================

TEST(BuiltinTest, ShouldUseBuiltinFunctionsInExpression)
{
    auto result = evalInput("타입(\"안녕\") == \"문자열\"");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(BuiltinTest, ShouldUseBuiltinFunctionsWithVariables)
{
    auto result = evalInput("문자열 msg = \"Hello\"\n길이(msg)");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 5);
}

TEST(BuiltinTest, ShouldCallBuiltinFromUserFunction)
{
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    evalInput(R"(
        정수 인사 = 함수(이름) {
            출력("안녕하세요,", 이름, "님!")
        };
        인사("철수")
    )");

    std::cout.rdbuf(old);
    EXPECT_EQ(buffer.str(), "안녕하세요, 철수 님!\n");
}
