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
 * @test 언더스코어 매개변수는 무시
 */
TEST(LinterTest, ShouldIgnoreUnderscorePrefixedParameter)
{
    std::string code = R"(
함수 계산(x, _unused) {
    반환 x
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
