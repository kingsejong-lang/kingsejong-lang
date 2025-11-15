/**
 * @file LinterTest.cpp
 * @brief Linter 테스트
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "linter/Linter.h"
#include "linter/rules/UnusedVariableRule.h"
#include "linter/rules/DeadCodeRule.h"
#include "linter/rules/NoSelfComparisonRule.h"

using namespace kingsejong;
using namespace kingsejong::lexer;
using namespace kingsejong::parser;
using namespace kingsejong::linter;
using namespace kingsejong::linter::rules;

// ============================================================================
// UnusedVariableRule 테스트
// ============================================================================

/**
 * @test 미사용 변수 감지
 */
TEST(LinterTest, ShouldDetectUnusedVariable)
{
    std::string code = R"(
정수 x = 10
정수 y = 20
출력(x)
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(parser.errors().size(), 0);

    Linter linter;
    linter.addRule(std::make_unique<UnusedVariableRule>());
    linter.analyze(program.get(), "test.ksj");

    // y가 미사용 변수로 감지되어야 함
    EXPECT_EQ(linter.errorCount(), 0);
    EXPECT_EQ(linter.warningCount(), 1);

    const auto& issues = linter.issues();
    ASSERT_EQ(issues.size(), 1);
    EXPECT_EQ(issues[0].ruleId, "unused-variable");
    EXPECT_TRUE(issues[0].message.find("y") != std::string::npos);
}

/**
 * @test 사용된 변수는 감지하지 않음
 */
TEST(LinterTest, ShouldNotReportUsedVariable)
{
    std::string code = R"(
정수 x = 10
정수 y = 20
출력(x + y)
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<UnusedVariableRule>());
    linter.analyze(program.get(), "test.ksj");

    // 모든 변수가 사용됨
    EXPECT_EQ(linter.issues().size(), 0);
}

/**
 * @test 언더스코어로 시작하는 변수는 무시
 */
TEST(LinterTest, ShouldIgnoreUnderscorePrefixedVariable)
{
    std::string code = R"(
정수 _unused = 10
정수 x = 20
출력(x)
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<UnusedVariableRule>());
    linter.analyze(program.get(), "test.ksj");

    // _unused는 무시됨
    EXPECT_EQ(linter.issues().size(), 0);
}

/**
 * @test 함수 내 지역 변수도 감지
 */
TEST(LinterTest, ShouldDetectUnusedLocalVariable)
{
    std::string code = R"(
정수 x = 함수() {
    정수 local = 10
    정수 unused = 20
    반환 local
}()
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<UnusedVariableRule>());
    linter.analyze(program.get(), "test.ksj");

    // unused와 x 둘 다 미사용 (x는 선언 후 바로 사용되지 않음)
    EXPECT_GE(linter.warningCount(), 1);  // 최소 1개 이상

    const auto& issues = linter.issues();
    ASSERT_GE(issues.size(), 1);

    // unused가 포함되어 있는지 확인
    bool foundUnused = false;
    for (const auto& issue : issues)
    {
        if (issue.message.find("unused") != std::string::npos)
        {
            foundUnused = true;
            break;
        }
    }
    EXPECT_TRUE(foundUnused);
}

/**
 * @test 여러 미사용 변수 감지
 */
TEST(LinterTest, ShouldDetectMultipleUnusedVariables)
{
    std::string code = R"(
정수 a = 1
정수 b = 2
정수 c = 3
정수 d = 4
출력(a + c)
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<UnusedVariableRule>());
    linter.analyze(program.get(), "test.ksj");

    // b와 d가 미사용
    EXPECT_EQ(linter.warningCount(), 2);
}

/**
 * @test 자기 자신 초기화는 사용으로 간주하지 않음
 */
TEST(LinterTest, ShouldNotCountSelfInitializationAsUsage)
{
    std::string code = R"(
정수 x = 10
정수 y = y + 1
출력(x)
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<UnusedVariableRule>());
    linter.analyze(program.get(), "test.ksj");

    // y만 미사용 (y의 초기화에서 y를 사용하는 것은 카운트하지 않음)
    // x는 출력에서 사용됨
    EXPECT_EQ(linter.warningCount(), 1);

    const auto& issues = linter.issues();
    ASSERT_EQ(issues.size(), 1);
    EXPECT_TRUE(issues[0].message.find("y") != std::string::npos);
}

/**
 * @test Linter clear 테스트
 */
TEST(LinterTest, ShouldClearIssues)
{
    std::string code = "정수 unused = 10";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<UnusedVariableRule>());
    linter.analyze(program.get());

    EXPECT_GT(linter.issues().size(), 0);

    linter.clear();
    EXPECT_EQ(linter.issues().size(), 0);
}

// ============================================================================
// DeadCodeRule 테스트
// ============================================================================

/**
 * @test return 후 도달 불가능한 코드 감지
 */
TEST(LinterTest, ShouldDetectDeadCodeAfterReturn)
{
    std::string code = R"(
정수 x = 함수() {
    반환 10
    정수 y = 20
}()
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<DeadCodeRule>());
    linter.analyze(program.get(), "test.ksj");

    // return 후 코드가 감지되어야 함
    EXPECT_EQ(linter.warningCount(), 1);

    const auto& issues = linter.issues();
    ASSERT_EQ(issues.size(), 1);
    EXPECT_EQ(issues[0].ruleId, "dead-code");
}

/**
 * @test return이 마지막 문장이면 문제 없음
 */
TEST(LinterTest, ShouldNotReportDeadCodeWhenReturnIsLast)
{
    std::string code = R"(
정수 x = 함수() {
    정수 y = 20
    반환 y
}()
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<DeadCodeRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_EQ(linter.issues().size(), 0);
}

/**
 * @test 중첩된 블록의 dead code도 감지
 */
TEST(LinterTest, ShouldDetectDeadCodeInNestedBlock)
{
    std::string code = R"(
정수 x = 함수() {
    만약 (참) {
        반환 10
        정수 unreachable = 20
    }
    반환 0
}()
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<DeadCodeRule>());
    linter.analyze(program.get(), "test.ksj");

    // if 블록 내부의 dead code 감지
    EXPECT_GE(linter.warningCount(), 1);
}

// ============================================================================
// NoSelfComparisonRule 테스트
// ============================================================================

/**
 * @test 자기 자신과의 동등 비교 감지
 */
TEST(LinterTest, ShouldDetectSelfEqualityComparison)
{
    std::string code = R"(
정수 x = 10
참거짓 result = x == x
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NoSelfComparisonRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_EQ(linter.warningCount(), 1);

    const auto& issues = linter.issues();
    ASSERT_EQ(issues.size(), 1);
    EXPECT_EQ(issues[0].ruleId, "no-self-comparison");
}

/**
 * @test 자기 자신과의 크기 비교 감지
 */
TEST(LinterTest, ShouldDetectSelfLessThanComparison)
{
    std::string code = R"(
정수 x = 10
참거짓 result = x < x
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NoSelfComparisonRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_EQ(linter.warningCount(), 1);
}

/**
 * @test 다른 변수와의 비교는 문제 없음
 */
TEST(LinterTest, ShouldNotReportDifferentVariableComparison)
{
    std::string code = R"(
정수 x = 10
정수 y = 20
참거짓 result = x == y
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NoSelfComparisonRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_EQ(linter.issues().size(), 0);
}

/**
 * @test 여러 자기 비교 감지
 */
TEST(LinterTest, ShouldDetectMultipleSelfComparisons)
{
    std::string code = R"(
정수 x = 10
정수 y = 20
참거짓 r1 = x == x
참거짓 r2 = y != y
참거짓 r3 = x < y
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NoSelfComparisonRule>());
    linter.analyze(program.get(), "test.ksj");

    // r1과 r2가 자기 비교 (r3는 정상)
    EXPECT_EQ(linter.warningCount(), 2);
}
