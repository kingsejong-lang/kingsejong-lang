/**
 * @file JosaDispatchTest.cpp
 * @brief 조사 표현식의 의미: '의' 멤버 접근, 사용자 함수·내장 함수 디스패치
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"

using namespace kingsejong;

namespace {

evaluator::Value run(const std::string& code, const std::string& var)
{
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    if (!parser.errors().empty()) {
        throw std::runtime_error("parse: " + parser.errors()[0]);
    }
    auto env = std::make_shared<evaluator::Environment>();
    evaluator::Value result;
    {
        evaluator::Evaluator ev(env);
        ev.evalProgram(program.get());
        result = env->get(var);
    }
    // 환경을 평가기와 같이 들고 있었으므로 소유자가 직접 순환 참조를 끊는다.
    env->releaseIfUnreachable(1);
    return result;
}

} // namespace

TEST(JosaDispatchTest, UserFunctionReceivesObjectAsFirstArgument)
{
    auto v = run("함수 두배(x) { 반환 x * 2 }\n값 = 21\n결과 = 값을 두배\n", "결과");
    EXPECT_EQ(v.asInteger(), 42);
}

TEST(JosaDispatchTest, UserFunctionWithExtraArguments)
{
    auto v = run("함수 더한다(a, b) { 반환 a + b }\n값 = 40\n결과 = 값을 더한다(2)\n", "결과");
    EXPECT_EQ(v.asInteger(), 42);
}

TEST(JosaDispatchTest, BuiltinFunctionIsDispatched)
{
    auto v = run("문장 = \"안녕\"\n결과 = 문장을 길이\n", "결과");
    EXPECT_EQ(v.asInteger(), 2);
}

TEST(JosaDispatchTest, PossessiveReadsDictionaryKey)
{
    auto v = run("사용자 = {\"이름\": \"세종\"}\n결과 = 사용자의 이름\n", "결과");
    EXPECT_EQ(v.asString(), "세종");
}

TEST(JosaDispatchTest, PossessiveOnMissingKeyThrows)
{
    EXPECT_THROW(run("사용자 = {\"이름\": \"세종\"}\n결과 = 사용자의 나이\n", "결과"), std::exception);
}

TEST(JosaDispatchTest, ArrayMethodsStillWork)
{
    auto v = run("목록 = [3, 1, 2]\n결과 = 목록을 정렬한다\n", "결과");
    ASSERT_TRUE(v.isArray());
    EXPECT_EQ(v.asArray()[0].asInteger(), 1);
}

TEST(JosaDispatchTest, WhileLoopRuns)
{
    auto v = run("i = 0\n합 = 0\n반복 (i < 5) {\n  합 = 합 + i\n  i = i + 1\n}\n", "합");
    EXPECT_EQ(v.asInteger(), 10);
}

TEST(JosaDispatchTest, ElseIfChainParses)
{
    auto v = run("점수 = 85\n등급 = \"\"\n만약 (점수 >= 90) {\n  등급 = \"A\"\n} 아니면 만약 (점수 >= 80) {\n  등급 = \"B\"\n} 아니면 {\n  등급 = \"C\"\n}\n", "등급");
    EXPECT_EQ(v.asString(), "B");
}

TEST(JosaDispatchTest, IndexAssignmentMutatesArrayInPlace)
{
    auto v = run("목록 = [3, 1, 2]\n목록[0] = 9\n사전 = {\"a\": 1}\n사전[\"b\"] = 2\n결과 = 목록[0] + 사전[\"b\"]\n", "결과");
    EXPECT_EQ(v.asInteger(), 11);
}

TEST(JosaDispatchTest, IndexAssignmentOutOfRangeThrows)
{
    EXPECT_THROW(run("목록 = [1]\n목록[5] = 0\n", "목록"), std::exception);
}

TEST(JosaDispatchTest, ArrayConcatenationAndNullLiteral)
{
    auto v = run("합 = [1] + [2, 3]\n결과 = 길이(합)\n", "결과");
    EXPECT_EQ(v.asInteger(), 3);
    auto n = run("값 = 없음\n", "값");
    EXPECT_TRUE(n.isNull());
}
