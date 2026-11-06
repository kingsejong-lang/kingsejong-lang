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
#include "linter/rules/ConstantConditionRule.h"
#include "linter/rules/EmptyBlockRule.h"
#include "linter/rules/NoUnusedParameterRule.h"
#include "linter/rules/NoShadowingRule.h"
#include "linter/rules/NoMagicNumberRule.h"
#include "linter/rules/NamingConventionRule.h"
#include "linter/rules/FunctionComplexityRule.h"
#include "linter/rules/UnusedImportRule.h"
#include "linter/rules/ImportOrderRule.h"
#include <filesystem>
#include <fstream>

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
    ASSERT_TRUE(parser.errors().empty());

    Linter linter;
    linter.addRule(std::make_unique<UnusedVariableRule>());
    linter.analyze(program.get(), "test.ksj");

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

// ============================================================================
// ConstantConditionRule 테스트
// ============================================================================

/**
 * @test 항상 참인 if 조건 감지
 */
TEST(LinterTest, ShouldDetectConstantTrueCondition)
{
    std::string code = R"(
만약 (참) {
    출력(1)
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<ConstantConditionRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_EQ(linter.warningCount(), 1);

    const auto& issues = linter.issues();
    ASSERT_EQ(issues.size(), 1);
    EXPECT_EQ(issues[0].ruleId, "constant-condition");
}

/**
 * @test 항상 거짓인 if 조건 감지
 */
TEST(LinterTest, ShouldDetectConstantFalseCondition)
{
    std::string code = R"(
만약 (거짓) {
    출력(1)
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<ConstantConditionRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_EQ(linter.warningCount(), 1);
}

/**
 * @test 변수 조건은 체크하지 않음 (상수가 아님)
 */
TEST(LinterTest, ShouldNotDetectVariableCondition)
{
    std::string code = R"(
정수 x = 10
만약 (x > 0) {
    출력(1)
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<ConstantConditionRule>());
    linter.analyze(program.get(), "test.ksj");

    // 변수 조건이므로 경고 없음
    EXPECT_EQ(linter.warningCount(), 0);
}

/**
 * @test 정수 0은 거짓으로 간주
 */
TEST(LinterTest, ShouldTreatZeroAsFalse)
{
    std::string code = R"(
만약 (0) {
    출력(1)
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<ConstantConditionRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_EQ(linter.warningCount(), 1);
}

// ============================================================================
// EmptyBlockRule 테스트
// ============================================================================

/**
 * @test 빈 if 블록 감지
 */
TEST(LinterTest, ShouldDetectEmptyIfBlock)
{
    std::string code = R"(
만약 (참) {
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<EmptyBlockRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_EQ(linter.warningCount(), 1);

    const auto& issues = linter.issues();
    ASSERT_EQ(issues.size(), 1);
    EXPECT_EQ(issues[0].ruleId, "empty-block");
}

/**
 * @test 빈 else 블록 감지
 */
TEST(LinterTest, ShouldDetectEmptyElseBlock)
{
    std::string code = R"(
만약 (참) {
    출력(1)
} 아니면 {
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<EmptyBlockRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_EQ(linter.warningCount(), 1);
}

/**
 * @test 빈 함수 본문 감지 (INFO 레벨)
 */
TEST(LinterTest, ShouldDetectEmptyFunction)
{
    std::string code = R"(
정수 f = 함수() {
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<EmptyBlockRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_EQ(linter.issues().size(), 1);
    // 함수는 INFO 레벨이므로 warningCount()에 포함되지 않음
}

/**
 * @test 내용이 있는 블록은 감지하지 않음
 */
TEST(LinterTest, ShouldNotDetectNonEmptyBlock)
{
    std::string code = R"(
만약 (참) {
    출력(1)
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<EmptyBlockRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_EQ(linter.issues().size(), 0);
}
// ============================================================================
// NoUnusedParameterRule 테스트
// ============================================================================

/**
 * @test 미사용 매개변수 감지
 */
TEST(LinterTest, ShouldDetectUnusedParameter)
{
    std::string code = R"(
함수 계산(x, y, z) {
    반환 x + y
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(parser.errors().size(), 0);

    Linter linter;
    linter.addRule(std::make_unique<NoUnusedParameterRule>());
    linter.analyze(program.get(), "test.ksj");

    // z가 미사용 매개변수로 감지되어야 함
    EXPECT_EQ(linter.errorCount(), 0);
    EXPECT_GE(linter.warningCount(), 1);

    const auto& issues = linter.issues();
    bool found = false;
    for (const auto& issue : issues) {
        if (issue.message.find("z") != std::string::npos) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @test 사용된 매개변수는 감지하지 않음
 */
TEST(LinterTest, ShouldNotReportUsedParameter)
{
    std::string code = R"(
함수 계산(x, y) {
    반환 x + y
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NoUnusedParameterRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_EQ(linter.issues().size(), 0);
}

/**
 * @test 언더스코어 매개변수는 무시하고, 미사용 매개변수 안내는 '_' 이름을 권한다
 */
TEST(LinterTest, ShouldIgnoreUnderscorePrefixedParameter)
{
    std::string code = R"(
함수 계산(x, _안씀) {
    반환 x
}
함수 합(a, 안씀) {
    반환 a
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_TRUE(parser.errors().empty());

    Linter linter;
    linter.addRule(std::make_unique<NoUnusedParameterRule>());
    linter.analyze(program.get(), "test.ksj");

    ASSERT_EQ(linter.issues().size(), 1u);
    EXPECT_EQ(linter.issues()[0].subject, "안씀");
    EXPECT_NE(linter.issues()[0].message.find("'_안씀'"), std::string::npos);
}

// ============================================================================
// NoShadowingRule 테스트
// ============================================================================

/**
 * @test 변수 섀도잉 감지
 */
TEST(LinterTest, ShouldDetectVariableShadowing)
{
    std::string code = R"(
정수 x = 10
함수 테스트() {
    정수 x = 20
    반환 x
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(parser.errors().size(), 0);

    Linter linter;
    linter.addRule(std::make_unique<NoShadowingRule>());
    linter.analyze(program.get(), "test.ksj");

    // 내부 x가 외부 x를 가림
    EXPECT_GE(linter.warningCount(), 1);
}

/**
 * @test 블록 스코프 섀도잉 감지
 */
TEST(LinterTest, ShouldDetectBlockScopeShadowing)
{
    std::string code = R"(
정수 x = 10
만약 (참) {
    정수 x = 20
    출력(x)
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NoShadowingRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_GE(linter.warningCount(), 1);
}

/**
 * @test 반복문 변수 섀도잉 감지
 */
TEST(LinterTest, ShouldDetectLoopVariableShadowing)
{
    std::string code = R"(
정수 i = 0
i가 1부터 10까지 {
    출력(i)
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NoShadowingRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_GE(linter.warningCount(), 1);
}

// ============================================================================
// NoMagicNumberRule 테스트
// ============================================================================

/**
 * @test 매직 넘버 감지
 */
TEST(LinterTest, ShouldDetectMagicNumber)
{
    std::string code = R"(
정수 x = 10
y = x + 42
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(parser.errors().size(), 0);

    Linter linter;
    linter.addRule(std::make_unique<NoMagicNumberRule>());
    linter.analyze(program.get(), "test.ksj");

    // 42가 매직 넘버로 감지되어야 함 (10은 변수 초기화라 허용)
    EXPECT_GE(linter.issues().size(), 1);
}

/**
 * @test 허용된 숫자는 감지하지 않음
 */
TEST(LinterTest, ShouldNotReportAllowedNumbers)
{
    std::string code = R"(
정수 x = 5
y = x + 0
z = y * 1
w = z + 2
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NoMagicNumberRule>());
    linter.analyze(program.get(), "test.ksj");

    // 0, 1, 2는 허용된 숫자
    EXPECT_EQ(linter.issues().size(), 0);
}

/**
 * @test 변수 초기화 숫자는 허용
 */
TEST(LinterTest, ShouldAllowNumbersInVariableInitialization)
{
    std::string code = R"(
정수 x = 999
정수 y = 12345
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NoMagicNumberRule>());
    linter.analyze(program.get(), "test.ksj");

    // 변수 초기화의 숫자는 허용
    EXPECT_EQ(linter.issues().size(), 0);
}

/**
 * @test 심각도 오버라이드 (HINT -> ERROR)
 */
TEST(LinterTest, ShouldOverrideSeverityToError)
{
    std::string code = R"(
정수 x = 10
y = x + 42
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    // 설정: no-magic-number를 ERROR로 변경
    std::string config = R"({
        "rules": {
            "no-magic-number": "error"
        }
    })";

    Linter linter;
    linter.addRule(std::make_unique<NoMagicNumberRule>());
    ASSERT_TRUE(linter.loadConfigFromString(config));
    linter.analyze(program.get(), "test.ksj");

    // 매직 넘버가 감지되어야 함
    ASSERT_GE(linter.issues().size(), 1);

    // 심각도가 ERROR로 오버라이드되어야 함 (원래는 HINT)
    bool foundError = false;
    for (const auto& issue : linter.issues())
    {
        if (issue.ruleId == "no-magic-number")
        {
            EXPECT_EQ(issue.severity, IssueSeverity::ERROR);
            foundError = true;
        }
    }
    EXPECT_TRUE(foundError);
}

/**
 * @test 심각도 오버라이드 (WARNING -> INFO)
 */
TEST(LinterTest, ShouldOverrideSeverityToInfo)
{
    std::string code = R"(
정수 x = 5
정수 y = 10
정수 z = 15  # 사용되지 않음
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    // 설정: unused-variable을 INFO로 낮춤
    std::string config = R"({
        "rules": {
            "unused-variable": "info"
        }
    })";

    Linter linter;
    linter.addRule(std::make_unique<UnusedVariableRule>());
    ASSERT_TRUE(linter.loadConfigFromString(config));
    linter.analyze(program.get(), "test.ksj");

    // z가 미사용 변수로 감지되어야 함
    ASSERT_GE(linter.issues().size(), 1);

    // 심각도가 INFO로 오버라이드되어야 함 (원래는 WARNING)
    bool foundInfo = false;
    for (const auto& issue : linter.issues())
    {
        if (issue.ruleId == "unused-variable")
        {
            EXPECT_EQ(issue.severity, IssueSeverity::INFO);
            foundInfo = true;
        }
    }
    EXPECT_TRUE(foundInfo);
}

/**
 * @test 심각도 오버라이드 없으면 기본값 사용
 */
TEST(LinterTest, ShouldUseDefaultSeverityWhenNoOverride)
{
    std::string code = R"(
정수 x = 5
y = x + 42
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    // 설정 없이 실행
    Linter linter;
    linter.addRule(std::make_unique<NoMagicNumberRule>());
    linter.analyze(program.get(), "test.ksj");

    // 매직 넘버가 감지되어야 함
    ASSERT_GE(linter.issues().size(), 1);

    // 심각도가 기본값(HINT)이어야 함
    bool foundHint = false;
    for (const auto& issue : linter.issues())
    {
        if (issue.ruleId == "no-magic-number")
        {
            EXPECT_EQ(issue.severity, IssueSeverity::HINT);
            foundHint = true;
        }
    }
    EXPECT_TRUE(foundHint);
}

// ============================================================================
// NamingConventionRule 테스트
// ============================================================================

/**
 * @test PascalCase 변수명 감지
 */
TEST(LinterTest, ShouldDetectPascalCaseVariable)
{
    std::string code = R"(
정수 MyVariable = 10
정수 x = MyVariable + 1
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NamingConventionRule>());
    linter.analyze(program.get(), "test.ksj");

    // PascalCase는 경고가 나와야 함 (snake_case 또는 camelCase 권장)
    EXPECT_GE(linter.warningCount(), 1);

    const auto& issues = linter.issues();
    bool found = false;
    for (const auto& issue : issues) {
        if (issue.message.find("MyVariable") != std::string::npos) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @test snake_case 변수명은 허용
 */
TEST(LinterTest, ShouldAllowSnakeCaseVariable)
{
    std::string code = R"(
정수 my_variable = 10
출력(my_variable)
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NamingConventionRule>());
    linter.analyze(program.get(), "test.ksj");

    EXPECT_EQ(linter.issues().size(), 0);
}

/**
 * @test 한글 변수명은 허용
 */
TEST(LinterTest, ShouldAllowKoreanVariableName)
{
    std::string code = R"(
정수 변수명 = 10
정수 또다른변수 = 20
출력(변수명 + 또다른변수)
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NamingConventionRule>());
    linter.analyze(program.get(), "test.ksj");

    // 한글 변수명은 허용됨
    EXPECT_EQ(linter.issues().size(), 0);
}

/**
 * @test UPPER_SNAKE_CASE 상수는 허용
 */
TEST(LinterTest, ShouldAllowUpperSnakeCaseConstant)
{
    std::string code = R"(
정수 MAX_VALUE = 100
정수 MIN_VALUE = 0
출력(MAX_VALUE)
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NamingConventionRule>());
    linter.analyze(program.get(), "test.ksj");

    // UPPER_SNAKE_CASE는 상수로 간주하여 허용
    EXPECT_EQ(linter.issues().size(), 0);
}

// ============================================================================
// FunctionComplexityRule 테스트
// ============================================================================

/**
 * @test 복잡도가 낮은 함수는 통과
 */
TEST(LinterTest, ShouldPassSimpleFunction)
{
    std::string code = R"(
함수 간단한함수(x) {
    만약 (x > 0) {
        반환 x
    }
    반환 0
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<FunctionComplexityRule>(10));
    linter.analyze(program.get(), "test.ksj");

    // 복잡도가 낮으므로 경고 없음
    EXPECT_EQ(linter.issues().size(), 0);
}

/**
 * @test 복잡도가 높은 함수는 경고
 */
TEST(LinterTest, ShouldWarnComplexFunction)
{
    std::string code = R"(
함수 복잡한함수(x, y, z) {
    만약 (x > 0) {
        만약 (y > 0) {
            만약 (z > 0) {
                반환 1
            }
        }
    }
    만약 (x < 0) {
        만약 (y < 0) {
            만약 (z < 0) {
                반환 -1
            }
        }
    }
    만약 (x == 0 && y == 0 && z == 0) {
        반환 0
    }
    반환 999
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<FunctionComplexityRule>(5));
    linter.analyze(program.get(), "test.ksj");

    // 복잡도가 높으므로 경고
    EXPECT_GE(linter.warningCount(), 1);
}

/**
 * @test 논리 연산자는 복잡도 증가
 */
TEST(LinterTest, ShouldCountLogicalOperatorComplexity)
{
    std::string code = R"(
함수 검사(x, y, z) {
    만약 (x > 0 && y > 0 && z > 0) {
        반환 참
    }
    반환 거짓
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<FunctionComplexityRule>(3));
    linter.analyze(program.get(), "test.ksj");

    // if + 2개의 && = 복잡도 4 (기본 1 + if 1 + && 2)
    EXPECT_GE(linter.warningCount(), 1);
}

// ============================================================================
// UnusedImportRule 테스트
// ============================================================================

/**
 * @test 사용하지 않는 import 감지
 */
TEST(LinterTest, ShouldDetectUnusedImport)
{
    std::string code = R"(
가져오기 "stdlib/math"
가져오기 "stdlib/json"
정수 x = 절댓값(-10)
출력(x)
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<UnusedImportRule>());
    linter.analyze(program.get(), "test.ksj");

    // json 모듈이 사용되지 않음
    EXPECT_GE(linter.warningCount(), 1);

    const auto& issues = linter.issues();
    bool foundJson = false;
    for (const auto& issue : issues) {
        if (issue.message.find("json") != std::string::npos) {
            foundJson = true;
            break;
        }
    }
    EXPECT_TRUE(foundJson);
}

/**
 * @test 여러 미사용 import 감지
 */
TEST(LinterTest, ShouldDetectMultipleUnusedImports)
{
    std::string code = R"(
가져오기 "stdlib/math"
가져오기 "stdlib/json"
가져오기 "stdlib/time"
가져오기 "stdlib/http"
출력("hello")
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<UnusedImportRule>());
    linter.analyze(program.get(), "test.ksj");

    // 4개 모두 미사용
    EXPECT_EQ(linter.warningCount(), 4);
}

/**
 * @test 라이브러리 모듈의 가져오기는 다시 내보내는 것이므로 보고하지 않는다
 *
 * stdlib/utils 는 array·math·string 을 가져와 그 함수를 그대로 내보낸다. 파일 안에서 쓰지 않아도
 * 지우면 utils 를 가져온 프로그램이 그 이름을 잃는다. 실행 문장이 있는 스크립트는 전처럼 보고한다.
 */
TEST(LinterTest, ShouldNotReportImportsOfLibraryModule)
{
    auto warnings = [](const std::string& code) {
        Lexer lexer(code);
        Parser parser(lexer);
        auto program = parser.parseProgram();
        Linter linter;
        linter.addRule(std::make_unique<UnusedImportRule>());
        linter.analyze(program.get(), "test.ksj");
        return linter.warningCount();
    };

    EXPECT_EQ(warnings("가져오기 \"stdlib/math\"\n정수인가 = 함수(값) {\n    반환 타입(값) == \"정수\"\n}\n"), 0);
    EXPECT_EQ(warnings("가져오기 \"stdlib/math\"\n정수 기본값 = 0\n"), 0);
    // 실행 문장이 하나라도 있으면 스크립트로 본다
    EXPECT_EQ(warnings("가져오기 \"stdlib/json\"\n정수인가 = 함수(값) {\n    반환 참\n}\n출력(정수인가(1))\n"), 1);
    // 정의 없이 가져오기만 있는 파일도 스크립트로 본다
    EXPECT_EQ(warnings("가져오기 \"stdlib/json\"\n"), 1);
}

// ============================================================================
// ImportOrderRule 테스트
// ============================================================================

/**
 * @test stdlib import가 먼저 오는지 검사
 */
TEST(LinterTest, ShouldDetectMisorderedImports)
{
    std::string code = R"(
가져오기 "utils/helper"
가져오기 "stdlib/math"
가져오기 "stdlib/json"
출력("test")
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<ImportOrderRule>());
    linter.analyze(program.get(), "test.ksj");

    // stdlib import가 사용자 import 뒤에 옴
    EXPECT_GE(linter.warningCount(), 1);

    // 진단이 문제가 된 가져오기 문의 실제 위치를 가리킨다:
    // 순서 위반은 3번째 줄의 "stdlib/math", 알파벳 순 위반은 4번째 줄의 "stdlib/json"
    bool misorderLocated = false;
    bool alphabetLocated = false;
    for (const auto& issue : linter.issues())
    {
        EXPECT_EQ(issue.column, 1);
        if (issue.message.find("먼저 선언") != std::string::npos)
        {
            EXPECT_EQ(issue.line, 3);
            misorderLocated = true;
        }
        else if (issue.message.find("알파벳") != std::string::npos)
        {
            EXPECT_EQ(issue.line, 4);
            alphabetLocated = true;
        }
    }
    EXPECT_TRUE(misorderLocated);
    EXPECT_TRUE(alphabetLocated);
}

/**
 * @test 올바른 import 순서는 문제 없음
 */
TEST(LinterTest, ShouldAllowCorrectImportOrder)
{
    std::string code = R"(
가져오기 "stdlib/json"
가져오기 "stdlib/math"
가져오기 "utils/helper"
출력("test")
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<ImportOrderRule>());
    linter.analyze(program.get(), "test.ksj");

    // stdlib가 먼저, 각 그룹 내 알파벳 순서
    EXPECT_EQ(linter.issues().size(), 0);
}

/**
 * @test stdlib import 내부 알파벳 순서 검사
 */
TEST(LinterTest, ShouldDetectUnorderedStdlibImports)
{
    std::string code = R"(
가져오기 "stdlib/time"
가져오기 "stdlib/math"
가져오기 "stdlib/json"
출력("test")
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<ImportOrderRule>());
    linter.analyze(program.get(), "test.ksj");

    // stdlib 그룹 내에서 알파벳 순서가 아님
    EXPECT_GE(linter.warningCount(), 1);
}

/**
 * @test 사용자 import 내부 알파벳 순서 검사
 */
TEST(LinterTest, ShouldDetectUnorderedUserImports)
{
    std::string code = R"(
가져오기 "stdlib/math"
가져오기 "utils/validator"
가져오기 "utils/helper"
출력("test")
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<ImportOrderRule>());
    linter.analyze(program.get(), "test.ksj");

    // 사용자 import 그룹 내에서 알파벳 순서가 아님
    EXPECT_GE(linter.warningCount(), 1);
}

/**
 * @test addDefaultRules 가 저장소의 규칙을 모두 등록한다
 */
TEST(LinterDefaultRulesTest, ShouldRegisterEveryRule)
{
    std::string code = "정수 unused_value = 1\n정수 used = 2\n출력(used)\n";
    kingsejong::lexer::Lexer lexer(code);
    kingsejong::parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    kingsejong::linter::Linter linter;
    linter.addDefaultRules();
    linter.analyze(program.get(), "default.ksj");

    bool foundUnused = false;
    for (const auto& issue : linter.issues())
    {
        if (issue.ruleId == "unused-variable") foundUnused = true;
    }
    EXPECT_TRUE(foundUnused) << "기본 규칙에 unused-variable 이 없다";
}

/**
 * @test josa-agreement: 받침과 맞지 않는 조사는 경고, 맞으면 조용하다
 */
TEST(LinterJosaAgreementTest, ShouldWarnOnMismatchedParticle)
{
    std::string code = "목록 = [3, 1, 2]\n결과 = 목록를 정렬한다\n출력(결과)\n";
    kingsejong::lexer::Lexer lexer(code);
    kingsejong::parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_TRUE(parser.errors().empty()) << parser.errors()[0];

    kingsejong::linter::Linter linter;
    linter.addDefaultRules();
    linter.analyze(program.get(), "josa.ksj");

    bool found = false;
    for (const auto& issue : linter.issues())
    {
        if (issue.ruleId == "josa-agreement")
        {
            found = true;
            EXPECT_NE(issue.message.find("목록을"), std::string::npos) << issue.message;
            EXPECT_EQ(issue.line, 2);
        }
    }
    EXPECT_TRUE(found);
}

TEST(LinterJosaAgreementTest, ShouldStaySilentOnMatchingParticle)
{
    std::string code = "목록 = [3, 1, 2]\n결과 = 목록을 정렬한다\n출력(결과)\n";
    kingsejong::lexer::Lexer lexer(code);
    kingsejong::parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_TRUE(parser.errors().empty());

    kingsejong::linter::Linter linter;
    linter.addDefaultRules();
    linter.analyze(program.get(), "josa.ksj");

    for (const auto& issue : linter.issues())
    {
        EXPECT_NE(issue.ruleId, "josa-agreement") << issue.message;
    }
}

/**
 * @test josa-agreement: 소유격 체인 뒤의 조사는 소유격의 오른쪽 이름을 명사로 본다
 */
TEST(LinterJosaAgreementTest, ShouldCheckParticleAfterPossessiveChain)
{
    std::string code = "사용자 = {\"이름\": \"a\"}\n결과 = 사용자의 이름를 출력한다\n";
    kingsejong::lexer::Lexer lexer(code);
    kingsejong::parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_TRUE(parser.errors().empty()) << parser.errors()[0];

    kingsejong::linter::Linter linter;
    linter.addDefaultRules();
    linter.analyze(program.get(), "josa.ksj");

    bool found = false;
    for (const auto& issue : linter.issues())
    {
        if (issue.ruleId == "josa-agreement")
        {
            found = true;
            EXPECT_NE(issue.message.find("이름을"), std::string::npos) << issue.message;
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @test 모듈 파일을 읽을 수 있으면 그 모듈이 정의한 이름의 사용으로 판단한다
 *
 * 함수 이름 목록으로만 추정하던 때는 목록에 없는 함수(결합, 범위)나 값으로 넘긴 함수
 * (`걸러낸다(소수인가)`)를 쓰는 가져오기도 "사용되지 않습니다" 로 보고했다.
 */
TEST(LinterTest, ShouldUseModuleDefinitionsForUnusedImport)
{
    namespace fs = std::filesystem;
    fs::path dir = fs::temp_directory_path() / "ksj_unused_import_test";
    fs::create_directories(dir);
    // Windows 에서 std::string 경로는 ANSI 코드 페이지로 해석되므로 모듈 파일 이름은 영문으로 둔다
    {
        std::ofstream(dir / "tools.ksj") << "결합하기 = 함수(a, b) { 반환 a + b }\n";
        std::ofstream(dir / "check.ksj") << "짝수인가 = 함수(x) { 반환 x % 2 == 0 }\n";
        std::ofstream(dir / "unused.ksj") << "아무것 = 1\n";
    }

    std::string code =
        "가져오기 \"tools\"\n"
        "가져오기 \"check\"\n"
        "가져오기 \"unused\"\n"
        "출력(결합하기(1, 2))\n"
        "출력([1, 2, 3]을 걸러낸다(짝수인가))\n";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<UnusedImportRule>());
    linter.setModuleResolver([&dir](const std::string& modulePath, const std::string&) {
        fs::path candidate = dir / (modulePath + ".ksj");
        return fs::exists(candidate) ? candidate.string() : std::string();
    });
    linter.analyze(program.get(), (dir / "main.ksj").string());

    ASSERT_EQ(linter.issues().size(), 1u);
    EXPECT_NE(linter.issues()[0].message.find("unused"), std::string::npos);
    EXPECT_EQ(linter.issues()[0].line, 3);  // 가져오기 문 위치를 함께 보고한다

    fs::remove_all(dir);
}

/**
 * @test 클로저가 쓰는 바깥 매개변수는 사용된 것이다
 */
TEST(LinterTest, ShouldTreatParameterUsedByClosureAsUsed)
{
    std::string code =
        "곱하기_만들기 = 함수(n) {\n"
        "    반환 함수(x) { 반환 x * n }\n"
        "}\n"
        "가림 = 함수(n) {\n"
        "    반환 함수(n) { 반환 n }\n"
        "}\n";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Linter linter;
    linter.addRule(std::make_unique<NoUnusedParameterRule>());
    linter.analyze(program.get(), "test.ksj");

    // 안쪽 함수가 같은 이름을 다시 선언한 '가림' 의 n 만 쓰이지 않은 것이다
    ASSERT_EQ(linter.issues().size(), 1u);
    EXPECT_EQ(linter.issues()[0].line, 4);
    EXPECT_EQ(linter.issues()[0].subject, "n");
}

// ============================================================================
// 오탐·표기 회귀 시험 (2026-11-05)
// ============================================================================

namespace
{
std::vector<LinterIssue> lintWith(const std::string& code, std::unique_ptr<Rule> rule)
{
    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty()) << (parser.errors().empty() ? "" : parser.errors()[0]);
    Linter linter;
    linter.addRule(std::move(rule));
    linter.analyze(program.get(), "test.ksj");
    return linter.issues();
}
} // namespace

/**
 * @test 실수의 NaN 검사 x != x 는 알리지 않고, 실수가 될 수 없는 이름과 <, > 는 알린다
 */
TEST(LinterTest, ShouldAllowNaNCheckButFlagNonFloatSelfComparison)
{
    auto issues = lintWith(R"(
NaN인가 = 함수(값) {
    반환 값 != 값
}
y = 0.0 / 0.0
출력(y != y, y == y)
)", std::make_unique<NoSelfComparisonRule>());
    EXPECT_EQ(issues.size(), 0u);

    issues = lintWith(R"(
n = 3
s = "가"
f = 함수(v) { 반환 v < v }
출력(n == n, s != s, f(1))
)", std::make_unique<NoSelfComparisonRule>());
    EXPECT_EQ(issues.size(), 3u);

    // 정수였다가 실수를 받은 이름은 실수일 수 있다
    issues = lintWith("k = 1\nk = 0.5\n출력(k == k)\n", std::make_unique<NoSelfComparisonRule>());
    EXPECT_EQ(issues.size(), 0u);
}

/**
 * @test 반복 (참) 안의 반환은 반복을 끝낸다. 안쪽 반복문의 반환도 끝내지만 안쪽 반복문의 중단은 끝내지 않는다
 */
TEST(LinterTest, ShouldTreatReturnAsLoopExit)
{
    auto issues = lintWith(R"(
f = 함수() {
    반복 (참) {
        반환 1
    }
}
g = 함수() {
    반복 (참) {
        i가 1부터 3까지 {
            만약 (i == 2) { 반환 i }
        }
    }
}
)", std::make_unique<ConstantConditionRule>());
    EXPECT_EQ(issues.size(), 0u);

    issues = lintWith(R"(
반복 (참) {
    i가 1부터 3까지 {
        중단
    }
    h = 함수() { 반환 1 }
}
)", std::make_unique<ConstantConditionRule>());
    ASSERT_EQ(issues.size(), 1u);
    EXPECT_NE(issues[0].message.find("무한 루프"), std::string::npos);
}

/**
 * @test 복잡도 경고는 함수 이름과 위치를 알리고, 안쪽 함수의 분기는 바깥 함수에 더하지 않는다
 */
TEST(LinterTest, ShouldNameFunctionAndIsolateNestedComplexity)
{
    auto issues = lintWith(R"(
바깥 = 함수(x) {
    만약 (x == 1) { 반환 1 }
    안쪽 = 함수(y) {
        만약 (y == 1) { 반환 1 }
        만약 (y == 2) { 반환 2 }
        만약 (y == 3) { 반환 3 }
        반환 0
    }
    반환 안쪽(x)
}
함수 선언형(x) {
    만약 (x == 1 || x == 2) { 반환 1 }
    만약 (x == 3) { 반환 3 }
    반환 0
}
출력([1, 2].지도(함수(v) {
    만약 (v == 1 && 참) { 반환 0 }
    만약 (v == 2) { 반환 1 }
    반환 v
}))
)", std::make_unique<FunctionComplexityRule>(3));
    // 바깥 = 2 (통과), 안쪽 = 4, 선언형 = 4, 익명 = 4
    ASSERT_EQ(issues.size(), 3u);
    EXPECT_NE(issues[0].message.find("'안쪽'"), std::string::npos);
    EXPECT_NE(issues[0].message.find("복잡도가 4"), std::string::npos);
    EXPECT_EQ(issues[0].line, 4);
    EXPECT_EQ(issues[0].subject, "안쪽");
    EXPECT_NE(issues[1].message.find("'선언형'"), std::string::npos);
    EXPECT_EQ(issues[1].line, 12);
    EXPECT_NE(issues[2].message.find("'<익명 함수>'"), std::string::npos);
    EXPECT_GT(issues[2].line, 0);
}

/**
 * @test 매직 넘버 실수는 출력과 같은 모양(0.5, 0.000000 이 아님)이고, 16진수는 쓴 그대로 보인다
 */
TEST(LinterTest, ShouldShowMagicNumberAsWritten)
{
    auto issues = lintWith("출력(0.5 * 3, 0xFF)\n", std::make_unique<NoMagicNumberRule>());
    ASSERT_EQ(issues.size(), 3u);
    EXPECT_NE(issues[0].message.find("매직 넘버 0.5를"), std::string::npos) << issues[0].message;
    EXPECT_NE(issues[2].message.find("매직 넘버 0xFF"), std::string::npos) << issues[2].message;
}
