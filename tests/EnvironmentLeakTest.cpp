/**
 * @file EnvironmentLeakTest.cpp
 * @brief 클로저-환경 순환 참조 해제 테스트
 *
 * 함수는 정의 환경을 shared_ptr 로 강참조하고 환경은 그 함수를 변수로 저장하므로,
 * 이름 붙은 함수를 선언한 스코프는 refcount 만으로는 해제되지 않는다.
 * Evaluator 가 스코프를 떠날 때 Environment::releaseIfUnreachable 로 사슬을 끊는지,
 * 그리고 바깥으로 빠져나간 클로저는 손대지 않는지 Environment::liveCount 로 확인한다.
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "runtime/Environment.h"
#include "evaluator/Builtin.h"

using namespace kingsejong;
using kingsejong::evaluator::Value;

namespace {

// 프로그램을 평가하고 결과와 AST 를 돌려준다. Function 은 AST 를 raw pointer 로
// 참조하므로 환경이 살아 있는 동안 AST 도 살아 있어야 한다.
struct Run
{
    std::unique_ptr<ast::Program> program;
    Value result;
};

Run runProgram(const std::string& source, std::shared_ptr<evaluator::Environment> env)
{
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    Run run;
    run.program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty()) << "파서 에러: " << (parser.errors().empty() ? "" : parser.errors()[0]);
    evaluator::Evaluator evaluator(env);
    run.result = evaluator.evalProgram(run.program.get());
    return run;
}

} // namespace

class EnvironmentLeakTest : public ::testing::Test
{
protected:
    size_t baseline_ = 0;

    void SetUp() override
    {
        evaluator::Builtin::registerAllBuiltins();
        baseline_ = evaluator::Environment::liveCount();
    }
};

// 전역에 선언한 재귀 함수: 전역 환경 <-> 함수 순환. 마지막 핸들이 사라지면 환경도 사라져야 한다.
TEST_F(EnvironmentLeakTest, GlobalRecursiveFunctionIsFreedWithEvaluator)
{
    std::unique_ptr<ast::Program> keepAst;
    {
        auto env = std::make_shared<evaluator::Environment>();
        auto run = runProgram(
            "함수 팩토리얼(n) { 만약 (n <= 1) { 반환 1 } 반환 n * 팩토리얼(n - 1) }\n"
            "팩토리얼(10)",
            env);
        keepAst = std::move(run.program);
        EXPECT_EQ(run.result.asInteger(), 3628800);
        // 평가기는 사라졌지만 env 핸들이 여기 있으므로 비워지지 않았다.
        EXPECT_TRUE(env->exists("팩토리얼"));
        EXPECT_GT(evaluator::Environment::liveCount(), baseline_);
        // 마지막 핸들을 놓기 전에 순환을 끊는다 (Evaluator 소멸자가 하는 일과 같다).
        EXPECT_TRUE(env->releaseIfUnreachable(1));
    }
    EXPECT_EQ(evaluator::Environment::liveCount(), baseline_);
}

// 평가기가 유일한 소유자인 전역 환경은 평가기 소멸자가 끊는다.
TEST_F(EnvironmentLeakTest, EvaluatorDestructorBreaksGlobalCycle)
{
    lexer::Lexer lexer("함수 f() { 반환 1 }\nf()");
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_TRUE(parser.errors().empty());
    {
        evaluator::Evaluator evaluator;
        EXPECT_EQ(evaluator.evalProgram(program.get()).asInteger(), 1);
        EXPECT_EQ(evaluator::Environment::liveCount(), baseline_ + 1);
    }
    EXPECT_EQ(evaluator::Environment::liveCount(), baseline_);
}

// 호출마다 이름 붙은 지역 함수를 만드는 함수를 반복 호출해도 환경이 쌓이지 않아야 한다.
TEST_F(EnvironmentLeakTest, LocalNamedFunctionDoesNotAccumulatePerCall)
{
    auto env = std::make_shared<evaluator::Environment>();
    auto run = runProgram(
        "함수 작업(x) { 함수 두배(y) { 반환 y * 2 } 반환 두배(x) }\n"
        "정수 합 = 0\n"
        "i가 0부터 199까지 반복한다 { 합 = 합 + 작업(i) }\n"
        "합",
        env);
    EXPECT_EQ(run.result.asInteger(), 39800);
    // 전역 환경 하나만 남아 있어야 한다. 호출 200번 분의 환경이 남았다면 실패.
    EXPECT_EQ(evaluator::Environment::liveCount(), baseline_ + 1);
    EXPECT_TRUE(env->releaseIfUnreachable(1));
}

// 자손 환경이 사슬로 이어진 경우: 바깥 호출 환경 A 가 중간 호출 환경 B 의 함수를 변수로 잡고,
// B 는 자기 함수에 잡혀 있다. A 를 떠날 때 A 와 B 가 모두 풀려야 하고, 푸는 순서 때문에
// 해제된 환경을 건드리는 일이 없어야 한다 (ASan 빌드에서 확인).
TEST_F(EnvironmentLeakTest, ChainedDescendantEnvironmentsAreFreedTogether)
{
    auto env = std::make_shared<evaluator::Environment>();
    auto run = runProgram(
        "함수 바깥() {\n"
        "    함수 중간() { 함수 안쪽() { 반환 1 } 반환 안쪽 }\n"
        "    x = 중간()\n"
        "    y = 중간()\n"
        "    반환 x() + y()\n"
        "}\n"
        "정수 합 = 0\n"
        "i가 0부터 99까지 반복한다 { 합 = 합 + 바깥() }\n"
        "합",
        env);
    EXPECT_EQ(run.result.asInteger(), 200);
    EXPECT_EQ(evaluator::Environment::liveCount(), baseline_ + 1);
    EXPECT_TRUE(env->releaseIfUnreachable(1));
    env.reset();
    EXPECT_EQ(evaluator::Environment::liveCount(), baseline_);
}

// 빠져나간 클로저는 환경을 계속 써야 한다. 이름 붙은 내부 함수(재귀)를 반환하는 경우.
TEST_F(EnvironmentLeakTest, EscapedNamedClosureKeepsItsEnvironment)
{
    auto env = std::make_shared<evaluator::Environment>();
    auto run = runProgram(
        "함수 만들기(기준) {\n"
        "    함수 세기(n) { 만약 (n <= 0) { 반환 기준 } 반환 1 + 세기(n - 1) }\n"
        "    반환 세기\n"
        "}\n"
        "세기5 = 만들기(100)\n0",
        env);
    // 만들기 호출 환경은 세기 의 클로저로 살아 있어야 한다 (전역 + 호출 환경).
    EXPECT_EQ(evaluator::Environment::liveCount(), baseline_ + 2);
    auto second = runProgram("세기5(5)", env);
    EXPECT_EQ(second.result.asInteger(), 105);
    // 전역 저장소 → 세기5 → 호출 환경 → 전역 으로 닫힌 사슬은 소유자가 끊는다.
    EXPECT_TRUE(env->releaseIfUnreachable(1));
    env.reset();
    EXPECT_EQ(evaluator::Environment::liveCount(), baseline_);
}

// 익명 클로저가 상태를 갱신하는 카운터: 기존 동작이 그대로여야 한다.
TEST_F(EnvironmentLeakTest, AnonymousCounterClosureStillWorks)
{
    auto env = std::make_shared<evaluator::Environment>();
    auto run = runProgram(
        "함수 카운터() { 정수 c = 0\n 반환 함수() { c = c + 1\n 반환 c } }\n"
        "증가 = 카운터()\n"
        "증가()\n증가()\n"
        "증가()",
        env);
    EXPECT_EQ(run.result.asInteger(), 3);
    EXPECT_TRUE(env->releaseIfUnreachable(1));
    env.reset();
    EXPECT_EQ(evaluator::Environment::liveCount(), baseline_);
}

// 클래스: 메서드 클로저가 전역을 잡고 인스턴스가 클래스 정의를 잡는다. 전부 해제되어야 한다.
TEST_F(EnvironmentLeakTest, ClassWithInstancesIsFreed)
{
    std::unique_ptr<ast::Program> keepAst;
    {
        auto env = std::make_shared<evaluator::Environment>();
        auto run = runProgram(
            "클래스 점 {\n"
            "    비공개 정수 x\n"
            "    생성자(x) { 자신.x = x }\n"
            "    공개 함수 두배() { 반환 자신.x * 2 }\n"
            "}\n"
            "p = 점(21)\n"
            "p.두배()",
            env);
        keepAst = std::move(run.program);
        EXPECT_EQ(run.result.asInteger(), 42);
        EXPECT_TRUE(env->releaseIfUnreachable(1));
    }
    EXPECT_EQ(evaluator::Environment::liveCount(), baseline_);
}

// 환경이 바깥에서도 참조되면 비우지 않는다.
TEST_F(EnvironmentLeakTest, ReleaseRefusesWhenAnotherHandleExists)
{
    auto env = std::make_shared<evaluator::Environment>();
    // 프로그램 결과로 함수가 빠져나가지 않도록 마지막 문장은 정수로 끝낸다.
    auto run = runProgram("함수 f() { 반환 1 }\n0", env);
    auto second = env;
    EXPECT_FALSE(env->releaseIfUnreachable(1));
    EXPECT_TRUE(env->exists("f"));
    EXPECT_TRUE(env->releaseIfUnreachable(2));
    EXPECT_FALSE(env->exists("f"));
}

// 함수가 반환값으로 빠져나갔으면 환경을 비우지 않는다.
TEST_F(EnvironmentLeakTest, ReleaseRefusesWhenFunctionEscaped)
{
    auto env = std::make_shared<evaluator::Environment>();
    auto run = runProgram("함수 f() { 반환 1 }\nf", env);
    ASSERT_TRUE(run.result.isFunction());
    EXPECT_FALSE(env->releaseIfUnreachable(1));
    EXPECT_TRUE(env->exists("f"));
    // 빠져나간 함수를 놓으면 비울 수 있다.
    run.result = Value::createNull();
    EXPECT_TRUE(env->releaseIfUnreachable(1));
}

// 상속과 '상위': 메서드가 부모 클래스 정의를, '상위' 프록시가 원래 인스턴스를 붙잡는다.
// 이 참조가 도달성 그래프에 없으면 바깥에서 붙잡힌 것으로 보여 전역 환경이 해제되지 않는다
TEST_F(EnvironmentLeakTest, InheritanceAndSuperCallsAreFreedWithEvaluator)
{
    lexer::Lexer lexer(
        "클래스 동물 {\n"
        "    공개 문자열 이름\n"
        "    생성자(이름) {\n        자신.이름 = 이름\n    }\n"
        "    공개 함수 소리() {\n        반환 \"...\"\n    }\n"
        "}\n"
        "클래스 개 상속 동물 {\n"
        "    생성자(이름) {\n        상위(이름)\n    }\n"
        "    공개 함수 소리() {\n        반환 \"멍멍 \" + 상위.소리()\n    }\n"
        "}\n"
        "바둑이 = 개(\"바둑이\")\n"
        "바둑이.이름 + \" \" + 바둑이.소리()\n");
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_TRUE(parser.errors().empty());
    {
        evaluator::Evaluator evaluator;
        EXPECT_EQ(evaluator.evalProgram(program.get()).asString(), "바둑이 멍멍 ...");
    }
    EXPECT_EQ(evaluator::Environment::liveCount(), baseline_);
}

// 메서드가 돌려준 클로저가 '상위' 와 '자신' 을 붙잡고 메서드 밖으로 나가도
// 원래 인스턴스를 가리키고(나중에 바뀐 필드가 보인다), 끝나면 환경이 모두 해제된다
TEST_F(EnvironmentLeakTest, ClosureThatCapturesSuperIsFreedWithEvaluator)
{
    lexer::Lexer lexer(
        "클래스 동물 {\n"
        "    공개 문자열 이름\n"
        "    생성자(이름) {\n        자신.이름 = 이름\n    }\n"
        "    공개 함수 소리() {\n        반환 자신.이름\n    }\n"
        "}\n"
        "클래스 개 상속 동물 {\n"
        "    생성자(이름) {\n        상위(이름)\n    }\n"
        "    공개 함수 나중에() {\n"
        "        반환 함수() {\n            반환 \"멍 \" + 상위.소리()\n        }\n"
        "    }\n"
        "}\n"
        "바둑이 = 개(\"바둑이\")\n"
        "나중 = 바둑이.나중에()\n"
        "바둑이.이름 = \"흰둥이\"\n"
        "나중()\n");
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_TRUE(parser.errors().empty());
    {
        evaluator::Evaluator evaluator;
        EXPECT_EQ(evaluator.evalProgram(program.get()).asString(), "멍 흰둥이");
    }
    EXPECT_EQ(evaluator::Environment::liveCount(), baseline_);
}
