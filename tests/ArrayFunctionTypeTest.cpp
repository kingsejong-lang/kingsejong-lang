/**
 * @file ArrayFunctionTypeTest.cpp
 * @brief 배열과 함수 타입 검사 테스트
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
// 배열 타입 검사 테스트
// ============================================================================

/**
 * @test 배열 리터럴 타입 추론
 */
TEST(ArrayFunctionTypeTest, ShouldInferArrayType)
{
    std::string code = R"(
배열 arr = [1, 2, 3]
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

/**
 * @test 배열 요소 타입 불일치 감지
 */
TEST(ArrayFunctionTypeTest, ShouldDetectArrayElementTypeMismatch)
{
    std::string code = R"(
배열 arr = [1, "문자열", 3]
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // 요소 타입 불일치
    EXPECT_GT(analyzer.errors().size(), 0);

    // 에러 메시지 확인
    bool foundError = false;
    for (const auto& err : analyzer.errors())
    {
        if (err.message.find("배열 요소") != std::string::npos &&
            err.message.find("일치하지") != std::string::npos)
        {
            foundError = true;
            break;
        }
    }
    EXPECT_TRUE(foundError);
}

/**
 * @test 빈 배열 허용
 */
TEST(ArrayFunctionTypeTest, ShouldAllowEmptyArray)
{
    std::string code = R"(
배열 arr = []
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
// 배열 인덱스 접근 타입 검사 테스트
// ============================================================================

/**
 * @test 배열 인덱스 접근 허용
 */
TEST(ArrayFunctionTypeTest, ShouldAllowArrayIndexAccess)
{
    std::string code = R"(
배열 arr = [1, 2, 3]
정수 x = 0
정수 val = arr[x]
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

/**
 * @test 배열이 아닌 타입에 인덱스 접근 시도 감지
 */
TEST(ArrayFunctionTypeTest, ShouldDetectIndexAccessOnNonArray)
{
    std::string code = R"(
정수 x = 10
정수 val = x[0]
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // 정수에 인덱스 접근 불가
    EXPECT_GT(analyzer.errors().size(), 0);

    // 에러 메시지 확인
    bool foundError = false;
    for (const auto& err : analyzer.errors())
    {
        if (err.message.find("인덱스 접근") != std::string::npos)
        {
            foundError = true;
            break;
        }
    }
    EXPECT_TRUE(foundError);
}

/**
 * @test 문자열 인덱스 접근 허용
 */
TEST(ArrayFunctionTypeTest, ShouldAllowStringIndexAccess)
{
    std::string code = R"(
문자열 s = "안녕"
문자열 ch = s[0]
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

/**
 * @test 배열 인덱스가 정수가 아닐 때 감지
 */
TEST(ArrayFunctionTypeTest, ShouldDetectNonIntegerArrayIndex)
{
    std::string code = R"(
배열 arr = [1, 2, 3]
문자열 idx = "0"
정수 val = arr[idx]
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);  // 인덱스가 문자열
    EXPECT_GT(analyzer.errors().size(), 0);

    // 에러 메시지 확인
    bool foundError = false;
    for (const auto& err : analyzer.errors())
    {
        if (err.message.find("인덱스") != std::string::npos &&
            err.message.find("정수") != std::string::npos)
        {
            foundError = true;
            break;
        }
    }
    EXPECT_TRUE(foundError);
}

// ============================================================================
// 함수 호출 타입 검사 테스트
// ============================================================================

/**
 * @test Builtin 함수 반환 타입 추론 - 길이
 */
TEST(ArrayFunctionTypeTest, ShouldInferBuiltinFunctionReturnType_Length)
{
    std::string code = R"(
정수 len = 길이("안녕")
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 길이는 정수를 반환
    EXPECT_EQ(analyzer.errors().size(), 0);
}

/**
 * @test Builtin 함수 반환 타입 추론 - 타입
 */
TEST(ArrayFunctionTypeTest, ShouldInferBuiltinFunctionReturnType_Type)
{
    std::string code = R"(
문자열 typeName = 타입(10)
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 타입은 문자열을 반환
    EXPECT_EQ(analyzer.errors().size(), 0);
}

/**
 * @test Builtin 함수 반환 타입 추론 - 정수
 */
TEST(ArrayFunctionTypeTest, ShouldInferBuiltinFunctionReturnType_Integer)
{
    std::string code = R"(
정수 num = 정수("123")
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 정수는 정수를 반환
    EXPECT_EQ(analyzer.errors().size(), 0);
}

/**
 * @test Builtin 함수 반환 타입 추론 - 실수
 */
TEST(ArrayFunctionTypeTest, ShouldInferBuiltinFunctionReturnType_Float)
{
    std::string code = R"(
실수 num = 실수("3.14")
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 실수는 실수를 반환
    EXPECT_EQ(analyzer.errors().size(), 0);
}

/**
 * @test Builtin 함수 반환 타입 추론 - 문자열
 */
TEST(ArrayFunctionTypeTest, ShouldInferBuiltinFunctionReturnType_String)
{
    std::string code = R"(
문자열 str = 문자열(123)
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 문자열은 문자열을 반환
    EXPECT_EQ(analyzer.errors().size(), 0);
}

/**
 * @test 반환 타입을 알 수 없는 builtin 함수도 허용
 */
TEST(ArrayFunctionTypeTest, ShouldAllowBuiltinFunctionWithUnknownReturnType)
{
    std::string code = R"(
출력("Hello")
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);  // 반환 타입 모르지만 허용
    EXPECT_EQ(analyzer.errors().size(), 0);
}

// ============================================================================
// 복합 타입 검사 테스트
// ============================================================================

/**
 * @test 배열 요소를 변수로 초기화
 */
TEST(ArrayFunctionTypeTest, ShouldAllowArrayWithVariableElements)
{
    std::string code = R"(
정수 x = 10
정수 y = 20
배열 arr = [x, y, 30]
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

/**
 * @test 중첩 배열 타입 검사
 */
TEST(ArrayFunctionTypeTest, ShouldAllowNestedArrays)
{
    std::string code = R"(
배열 inner = [1, 2]
배열 outer = [inner, inner]
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

/**
 * @test 함수 호출 결과를 배열에 저장
 */
TEST(ArrayFunctionTypeTest, ShouldAllowFunctionCallResultInArray)
{
    std::string code = R"(
배열 arr = [길이("Hello"), 정수("10"), 20]
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
