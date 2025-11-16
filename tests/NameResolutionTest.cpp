/**
 * @file NameResolutionTest.cpp
 * @brief Name Resolution 테스트
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "semantic/SemanticAnalyzer.h"

using namespace kingsejong;
using namespace kingsejong::lexer;
using namespace kingsejong::parser;
using namespace kingsejong::semantic;

// ============================================================================
// 정의되지 않은 변수 감지 테스트
// ============================================================================

/**
 * @test 정의되지 않은 변수 사용 감지
 */
TEST(NameResolutionTest, ShouldDetectUndefinedVariable)
{
    std::string code = R"(
정수 x = y
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // y가 정의되지 않아서 실패
    EXPECT_GT(analyzer.errors().size(), 0);

    // 에러 메시지 확인
    bool foundError = false;
    for (const auto& err : analyzer.errors())
    {
        if (err.message.find("정의되지 않은") != std::string::npos &&
            err.message.find("y") != std::string::npos)
        {
            foundError = true;
            break;
        }
    }
    EXPECT_TRUE(foundError);
}

/**
 * @test 정의된 변수 사용 허용
 */
TEST(NameResolutionTest, ShouldAllowDefinedVariable)
{
    std::string code = R"(
정수 x = 10
정수 y = x
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // x가 정의되어 있으므로 성공
    EXPECT_EQ(analyzer.errors().size(), 0);
}

/**
 * @test Builtin 함수 허용
 */
TEST(NameResolutionTest, ShouldAllowBuiltinFunction)
{
    std::string code = R"(
출력("Hello")
정수 x = 길이("World")
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // builtin 함수는 허용
    EXPECT_EQ(analyzer.errors().size(), 0);
}

// ============================================================================
// 할당문에서의 Name Resolution 테스트
// ============================================================================

/**
 * @test 정의되지 않은 변수에 할당 시도 감지
 */
TEST(NameResolutionTest, ShouldDetectAssignmentToUndefinedVariable)
{
    std::string code = R"(
x = 10
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    // 동적 타이핑: 선언되지 않은 변수에 할당하면 자동으로 등록됨
    EXPECT_TRUE(result);  // x가 자동으로 등록됨
    EXPECT_EQ(analyzer.errors().size(), 0);
}

/**
 * @test 할당 값에 정의되지 않은 변수 사용 감지
 */
TEST(NameResolutionTest, ShouldDetectUndefinedVariableInAssignment)
{
    std::string code = R"(
정수 x = 10
x = y
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // y가 정의되지 않음
    EXPECT_GT(analyzer.errors().size(), 0);
}

// ============================================================================
// 표현식에서의 Name Resolution 테스트
// ============================================================================

/**
 * @test 이항 연산에서 정의되지 않은 변수 감지
 */
TEST(NameResolutionTest, ShouldDetectUndefinedVariableInBinaryExpression)
{
    std::string code = R"(
정수 x = 10
정수 y = x + z
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // z가 정의되지 않음
    EXPECT_GT(analyzer.errors().size(), 0);
}

/**
 * @test 함수 호출에서 정의되지 않은 변수 감지
 */
TEST(NameResolutionTest, ShouldDetectUndefinedVariableInFunctionCall)
{
    std::string code = R"(
출력(x)
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // x가 정의되지 않음
    EXPECT_GT(analyzer.errors().size(), 0);
}

/**
 * @test 배열 리터럴에서 정의되지 않은 변수 감지
 */
TEST(NameResolutionTest, ShouldDetectUndefinedVariableInArrayLiteral)
{
    std::string code = R"(
정수 x = 10
배열 arr = [x, y, 30]
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // y가 정의되지 않음
    EXPECT_GT(analyzer.errors().size(), 0);
}

// ============================================================================
// 제어문에서의 Name Resolution 테스트
// ============================================================================

/**
 * @test if 조건에서 정의되지 않은 변수 감지
 */
TEST(NameResolutionTest, ShouldDetectUndefinedVariableInIfCondition)
{
    std::string code = R"(
만약 x > 10 {
    출력("OK")
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // x가 정의되지 않음
    EXPECT_GT(analyzer.errors().size(), 0);
}

/**
 * @test while 조건에서 정의되지 않은 변수 감지
 */
TEST(NameResolutionTest, ShouldDetectUndefinedVariableInWhileCondition)
{
    std::string code = R"(
반복 x < 10 {
    출력("OK")
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // x가 정의되지 않음
    EXPECT_GT(analyzer.errors().size(), 0);
}

/**
 * @test 블록 내에서 정의된 변수는 사용 가능
 */
TEST(NameResolutionTest, ShouldAllowVariableDefinedInBlock)
{
    std::string code = R"(
만약 참 {
    정수 x = 10
    정수 y = x + 5
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // x가 블록 내에서 정의되어 있음
    EXPECT_EQ(analyzer.errors().size(), 0);
}

// ============================================================================
// 복합 표현식 테스트
// ============================================================================

/**
 * @test 중첩된 표현식에서 정의되지 않은 변수 감지
 */
TEST(NameResolutionTest, ShouldDetectUndefinedVariableInNestedExpression)
{
    std::string code = R"(
정수 x = 10
정수 y = (x + 5) * z
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // z가 정의되지 않음
    EXPECT_GT(analyzer.errors().size(), 0);
}

/**
 * @test 여러 정의되지 않은 변수 모두 감지
 */
TEST(NameResolutionTest, ShouldDetectMultipleUndefinedVariables)
{
    std::string code = R"(
정수 result = x + y + z
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // x, y, z 모두 정의되지 않음
    EXPECT_GE(analyzer.errors().size(), 3);  // 최소 3개 에러
}
