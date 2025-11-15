/**
 * @file TypeCheckingTest.cpp
 * @brief 타입 검사 (Type Checking) 테스트
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "semantic/SemanticAnalyzer.h"

using namespace kingsejong::lexer;
using namespace kingsejong::parser;
using namespace kingsejong::semantic;

// ============================================================================
// 기본 타입 추론 테스트
// ============================================================================

TEST(TypeCheckingTest, ShouldInferIntegerType)
{
    std::string code = R"(
정수 x = 42
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 타입이 일치하므로 성공
    EXPECT_EQ(analyzer.errors().size(), 0);
}

TEST(TypeCheckingTest, ShouldInferFloatType)
{
    std::string code = R"(
실수 y = 3.14
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);
    EXPECT_EQ(analyzer.errors().size(), 0);
}

TEST(TypeCheckingTest, ShouldInferStringType)
{
    std::string code = R"(
문자열 s = "안녕"
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);
    EXPECT_EQ(analyzer.errors().size(), 0);
}

TEST(TypeCheckingTest, ShouldInferBooleanType)
{
    std::string code = R"(
논리 b = 참
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);
    EXPECT_EQ(analyzer.errors().size(), 0);
}

// ============================================================================
// 타입 불일치 검사 테스트
// ============================================================================

TEST(TypeCheckingTest, ShouldDetectIntegerToStringMismatch)
{
    std::string code = R"(
문자열 s = 42
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // 타입 불일치로 실패
    EXPECT_GT(analyzer.errors().size(), 0);
}

TEST(TypeCheckingTest, ShouldDetectStringToIntegerMismatch)
{
    std::string code = R"(
정수 x = "안녕"
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);
    EXPECT_GT(analyzer.errors().size(), 0);
}

TEST(TypeCheckingTest, ShouldDetectBooleanToIntegerMismatch)
{
    std::string code = R"(
논리 b = 참
정수 x = b
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // 논리 값을 정수에 할당하면 에러
    EXPECT_GT(analyzer.errors().size(), 0);
}

// ============================================================================
// 이항 연산 타입 추론 테스트
// ============================================================================

TEST(TypeCheckingTest, ShouldInferIntegerFromIntegerAddition)
{
    std::string code = R"(
정수 x = 10 + 20
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 정수 + 정수 = 정수
    EXPECT_EQ(analyzer.errors().size(), 0);
}

TEST(TypeCheckingTest, ShouldInferFloatFromMixedArithmetic)
{
    std::string code = R"(
실수 x = 10 + 3.14
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 정수 + 실수 = 실수
    EXPECT_EQ(analyzer.errors().size(), 0);
}

TEST(TypeCheckingTest, ShouldInferBooleanFromComparison)
{
    std::string code = R"(
논리 result = 10 < 20
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 비교 연산 = 논리
    EXPECT_EQ(analyzer.errors().size(), 0);
}

TEST(TypeCheckingTest, ShouldInferBooleanFromLogicalOperation)
{
    std::string code = R"(
논리 result = 참 && 거짓
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 논리 연산 = 논리
    EXPECT_EQ(analyzer.errors().size(), 0);
}

TEST(TypeCheckingTest, ShouldInferStringFromStringConcatenation)
{
    std::string code = R"(
문자열 result = "안녕" + "하세요"
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 문자열 + 문자열 = 문자열
    EXPECT_EQ(analyzer.errors().size(), 0);
}

// ============================================================================
// 할당문 타입 검사 테스트
// ============================================================================

TEST(TypeCheckingTest, ShouldAllowCorrectTypeAssignment)
{
    std::string code = R"(
정수 x = 10
x = 20
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 정수 변수에 정수 할당 OK
    EXPECT_EQ(analyzer.errors().size(), 0);
}

TEST(TypeCheckingTest, ShouldDetectWrongTypeAssignment)
{
    std::string code = R"(
정수 x = 10
x = "문자열"
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // 정수 변수에 문자열 할당 에러
    EXPECT_GT(analyzer.errors().size(), 0);
}

// ============================================================================
// 제어문 조건 타입 검사 테스트
// ============================================================================

TEST(TypeCheckingTest, ShouldAllowBooleanConditionInIf)
{
    std::string code = R"(
만약 참 {
    정수 x = 10
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 논리 조건 OK
    EXPECT_EQ(analyzer.errors().size(), 0);
}

TEST(TypeCheckingTest, ShouldDetectNonBooleanConditionInIf)
{
    std::string code = R"(
만약 42 {
    정수 x = 10
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // 정수 조건 에러
    EXPECT_GT(analyzer.errors().size(), 0);
}

TEST(TypeCheckingTest, ShouldAllowBooleanConditionInWhile)
{
    std::string code = R"(
10 < 20 동안 {
    정수 x = 10
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 비교 연산 (논리) OK
    EXPECT_EQ(analyzer.errors().size(), 0);
}

// ============================================================================
// 복잡한 표현식 타입 추론 테스트
// ============================================================================

TEST(TypeCheckingTest, ShouldInferTypeFromNestedExpression)
{
    std::string code = R"(
정수 result = (10 + 20) * 3 - 5
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 중첩 산술 연산 = 정수
    EXPECT_EQ(analyzer.errors().size(), 0);
}

TEST(TypeCheckingTest, ShouldInferTypeFromComplexBooleanExpression)
{
    std::string code = R"(
논리 result = (10 < 20) && (30 > 25)
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 복잡한 논리 표현식 OK
    EXPECT_EQ(analyzer.errors().size(), 0);
}
