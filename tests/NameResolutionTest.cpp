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

/**
 * @test 인덱스 밖의 범위 식은 의미 분석에서 알린다 (평가기·VM 모두 값으로 만들지 못한다)
 */
TEST(NameResolutionTest, ShouldRejectRangeOutsideIndex)
{
    // 다음 줄의 식별자를 step 으로 읽지 않아야 파싱이 끝까지 간다
    Lexer lexer("r = 1부터 3까지\n출력(r)\n");
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_TRUE(parser.errors().empty()) << parser.errors()[0];

    SemanticAnalyzer analyzer;
    EXPECT_FALSE(analyzer.analyze(program.get()));
    ASSERT_FALSE(analyzer.errors().empty());
    EXPECT_NE(analyzer.errors()[0].message.find("배열 인덱스 안"), std::string::npos);

    Lexer okLexer("배열 = [1, 2, 3, 4]\n출력(배열[1부터 2까지], 배열[0부터 4까지 2씩])\n");
    Parser okParser(okLexer);
    auto okProgram = okParser.parseProgram();
    SemanticAnalyzer okAnalyzer;
    EXPECT_TRUE(okAnalyzer.analyze(okProgram.get()));
}

TEST(NameResolutionTest, ShouldRejectSelfAndSuperOutsideClassMembers)
{
    // 메서드·생성자 밖의 자신·상위는 실행 전에 알려 준다 (호출되지 않는 함수 안이라도)
    for (const char* code : {"출력(자신)\n", "함수 f() {\n    반환 자신\n}\n", "출력(상위.소리())\n"})
    {
        Lexer lexer(code);
        Parser parser(lexer);
        auto program = parser.parseProgram();
        ASSERT_TRUE(parser.errors().empty()) << parser.errors()[0];

        SemanticAnalyzer analyzer;
        EXPECT_FALSE(analyzer.analyze(program.get())) << code;
        ASSERT_FALSE(analyzer.errors().empty()) << code;
        EXPECT_NE(analyzer.errors()[0].message.find("메서드나 생성자 안에서만"), std::string::npos)
            << analyzer.errors()[0].message;
    }

    // 메서드·생성자 안, 그 안의 함수 리터럴에서는 그대로 쓸 수 있다
    Lexer okLexer(
        "클래스 동물 {\n"
        "    비공개 문자열 이름\n"
        "    생성자(이름) {\n        자신.이름 = 이름\n    }\n"
        "    공개 함수 소리() {\n        반환 \"...\"\n    }\n"
        "}\n"
        "클래스 개 상속 동물 {\n"
        "    생성자(이름) {\n        상위(이름)\n    }\n"
        "    공개 함수 소리() {\n        f = 함수() {\n            반환 상위.소리() + 자신.이름\n        }\n        반환 f()\n    }\n"
        "}\n");
    Parser okParser(okLexer);
    auto okProgram = okParser.parseProgram();
    ASSERT_TRUE(okParser.errors().empty()) << okParser.errors()[0];
    SemanticAnalyzer okAnalyzer;
    EXPECT_TRUE(okAnalyzer.analyze(okProgram.get()))
        << (okAnalyzer.errors().empty() ? "" : okAnalyzer.errors()[0].message);
}
