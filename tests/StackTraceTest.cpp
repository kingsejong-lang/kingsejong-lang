/**
 * @file StackTraceTest.cpp
 * @brief 잡히지 않은 실행 에러에 붙는 호출 스택 테스트
 *
 * 평가기는 에러가 함수 호출 경계를 거슬러 올라갈 때마다 프레임(함수 이름, 그 함수 안에서 실행 중이던
 * 문장의 위치)을 안쪽부터 더하고, 파일 최상위에서 "<최상위>" 프레임을 마지막으로 더한다.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "error/Error.h"
#include "error/ErrorReporter.h"
#include <sstream>
#include <utility>
#include <vector>

using namespace kingsejong;
using ::testing::HasSubstr;
using ::testing::Not;

namespace {

using Frames = std::vector<std::pair<std::string, int>>;

/// 프로그램(파일 이름 test.ksj)을 평가해 잡히지 않은 KingSejongError 를 돌려준다. 에러가 안 나면 테스트 실패
error::KingSejongError runExpectingError(const std::string& source,
                                         std::shared_ptr<evaluator::Environment> env = nullptr)
{
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(source, "test.ksj");
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty()) << (parser.errors().empty() ? "" : parser.errors()[0]);

    evaluator::Evaluator eval(env ? env : std::make_shared<evaluator::Environment>());
    try
    {
        eval.evalProgram(program.get());
    }
    catch (const error::KingSejongError& e)
    {
        return e;
    }
    ADD_FAILURE() << "에러가 나지 않았습니다";
    return error::KingSejongError(error::ErrorType::RUNTIME_ERROR, "(없음)");
}

/// (함수 이름, 줄) 쌍으로 줄인 호출 스택
Frames framesOf(const error::KingSejongError& e)
{
    Frames out;
    for (const auto& frame : e.stackTrace())
    {
        out.emplace_back(frame.function, frame.location.line);
    }
    return out;
}

const char* const kNestedSource =
R"(함수 나누기(a, b) {
    반환 a / b
}
함수 계산(x) {
    반환 나누기(x, 0)
}
계산(10)
)";

} // namespace

TEST(StackTraceTest, ShouldRecordNestedFunctionCallsInnermostFirst)
{
    auto e = runExpectingError(kNestedSource);
    EXPECT_EQ(framesOf(e), (Frames{{"나누기", 2}, {"계산", 5}, {"<최상위>", 7}}));
    ASSERT_FALSE(e.stackTrace().empty());
    EXPECT_EQ(e.stackTrace()[0].location.filename, "test.ksj");
}

TEST(StackTraceTest, ShouldRecordFramesForErrorsThrownByBuiltins)
{
    auto e = runExpectingError(R"(함수 변환(글) {
    반환 정수(글)
}
변환("abc")
)");
    EXPECT_THAT(e.what(), HasSubstr("문자열을 정수로 변환할 수 없습니다"));
    EXPECT_EQ(framesOf(e), (Frames{{"변환", 2}, {"<최상위>", 4}}));
}

TEST(StackTraceTest, ShouldRecordFramesForUncaughtThrowStatement)
{
    auto e = runExpectingError(R"(함수 실패() {
    던지다 "문제 발생"
}
함수 작업() {
    실패()
}
작업()
)");
    EXPECT_THAT(e.what(), HasSubstr("문제 발생"));
    EXPECT_EQ(framesOf(e), (Frames{{"실패", 2}, {"작업", 5}, {"<최상위>", 7}}));
}

TEST(StackTraceTest, ShouldNameFunctionValueByCalledIdentifier)
{
    auto e = runExpectingError(R"(f = 함수(x) {
    반환 x / 0
}
f(1)
)");
    EXPECT_EQ(framesOf(e), (Frames{{"f", 2}, {"<최상위>", 4}}));
}

TEST(StackTraceTest, ShouldNameMethodFramesWithClassName)
{
    auto e = runExpectingError(R"(클래스 계산기 {
    공개 함수 나누기(a, b) {
        반환 a / b
    }
}
계 = 계산기()
계.나누기(1, 0)
)");
    EXPECT_EQ(framesOf(e), (Frames{{"계산기.나누기", 3}, {"<최상위>", 7}}));
}

TEST(StackTraceTest, ShouldNameConstructorFrames)
{
    auto e = runExpectingError(R"(클래스 포인트 {
    비공개 정수 x
    생성자(값) {
        자신.x = 1 / 값
    }
}
p = 포인트(0)
)");
    EXPECT_EQ(framesOf(e), (Frames{{"포인트.생성자", 4}, {"<최상위>", 7}}));
}

TEST(StackTraceTest, ShouldRestoreEnvironmentWhenMethodThrowsIntoTryBlock)
{
    // 메서드가 던진 에러를 호출한 쪽의 시도 블록이 잡으면, 이후 문장은 원래 환경에서 실행돼야 한다.
    // 환경은 평가기만 쥔다 (클래스·메서드가 만드는 함수-환경 순환은 평가기 소멸자가 끊는다)
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(R"(클래스 계산기 {
    공개 함수 나누기(a, b) {
        반환 a / b
    }
}
계 = 계산기()
시도 {
    계.나누기(1, 0)
} 오류 (e) {
    무시 = e
}
결과 = 1
)", "test.ksj");
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_TRUE(parser.errors().empty());

    evaluator::Evaluator eval;
    const evaluator::Environment* globalEnv = eval.environment().get();
    EXPECT_NO_THROW(eval.evalProgram(program.get()));
    EXPECT_EQ(eval.environment().get(), globalEnv);
    auto result = eval.environment()->get("결과");
    ASSERT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 1);
}

TEST(StackTraceTest, ShouldHaveOnlyTopLevelFrameOutsideFunctions)
{
    auto e = runExpectingError("x = 1\ny = x / 0\n");
    EXPECT_EQ(framesOf(e), (Frames{{"<최상위>", 2}}));
}

TEST(StackTraceTest, ShouldRecordCallSiteLineAfterInnerCallReturned)
{
    // 안쪽 호출이 정상으로 돌아온 뒤 같은 함수의 다음 문장에서 난 에러는 그 문장의 줄을 가리켜야 한다
    auto e = runExpectingError(R"(함수 괜찮음() {
    반환 1
}
함수 작업() {
    a = 괜찮음()
    반환 a / 0
}
작업()
)");
    EXPECT_EQ(framesOf(e), (Frames{{"작업", 6}, {"<최상위>", 8}}));
}

TEST(StackTraceTest, ShouldPointAtLoopLineWhenConditionFailsAfterBodyRan)
{
    // 본문이 한 번 돈 뒤 조건식에서 난 에러는 본문의 마지막 문장이 아니라 반복문의 줄을 가리켜야 한다
    auto e = runExpectingError(R"(x = 2
반복 (10 / x > 0) {
    y = 1
    x = x - 1
}
)");
    EXPECT_THAT(e.what(), HasSubstr("0으로 나눌 수 없습니다"));
    EXPECT_EQ(framesOf(e), (Frames{{"<최상위>", 2}}));
}

TEST(StackTraceTest, ShouldPointAtLoopLineWhenConditionCallThrowsAfterBodyRan)
{
    // 조건식이 부른 함수 안에서 난 에러도 바깥 프레임은 반복문의 줄이다
    auto e = runExpectingError(R"(함수 검사(n) {
    반환 10 / n
}
x = 1
반복 (검사(x) > 0) {
    x = x - 1
}
)");
    EXPECT_EQ(framesOf(e), (Frames{{"검사", 2}, {"<최상위>", 5}}));
}

TEST(StackTraceTest, ShouldReportStackTraceWithInnermostLocation)
{
    auto e = runExpectingError(kNestedSource);

    error::ErrorReporter reporter;
    reporter.setColorEnabled(false);
    reporter.registerSource("test.ksj", kNestedSource);
    std::ostringstream out;
    reporter.report(e, out);

    EXPECT_THAT(out.str(), HasSubstr("오류 위치: test.ksj:2"));
    EXPECT_THAT(out.str(), HasSubstr("반환 a / b"));
    EXPECT_THAT(out.str(), HasSubstr("0으로 나눌 수 없습니다"));
    EXPECT_THAT(out.str(), HasSubstr("호출 스택 (가장 안쪽부터):\n  나누기 (test.ksj:2)\n  계산 (test.ksj:5)\n  <최상위> (test.ksj:7)\n"));
}

TEST(StackTraceTest, ShouldNotReportStackTraceForTopLevelError)
{
    auto e = runExpectingError("x = 1 / 0\n");

    error::ErrorReporter reporter;
    reporter.setColorEnabled(false);
    std::ostringstream out;
    reporter.report(e, out);

    EXPECT_THAT(out.str(), HasSubstr("오류 위치: test.ksj:1"));
    EXPECT_THAT(out.str(), Not(HasSubstr("호출 스택")));
}
