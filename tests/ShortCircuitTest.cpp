/**
 * @file ShortCircuitTest.cpp
 * @brief &&·||(그리고·또는) 단축 평가 테스트
 *
 * 왼쪽으로 결과가 정해지면 오른쪽을 평가하지 않는다. 전에는 두 엔진 모두 오른쪽을 늘 평가해서
 * `i < n && 배열[i]` 같은 경계 검사가 인덱스 오류로 끝났다. 결과 값(진리값 규칙의 참/거짓)은 그대로다.
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"

using namespace kingsejong;

namespace {

/// 평가기로 돌리고 전역 변수 '기록' 을 문자열로 돌려준다
std::string runEvaluator(const std::string& source)
{
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty()) << (parser.errors().empty() ? "" : parser.errors()[0]);
    auto env = std::make_shared<evaluator::Environment>();
    // 테스트가 env 를 쥐고 있어 평가기 소멸자가 함수-환경 순환 참조를 끊지 못하므로 직접 비운다 (예외로 끝나도)
    struct ClearOnExit
    {
        std::shared_ptr<evaluator::Environment> env;
        ~ClearOnExit() { env->clear(); }
    } clearOnExit{env};
    {
        evaluator::Evaluator evaluator(env);
        evaluator.evalProgram(program.get());
    }
    return env->get("기록").toString();
}

/// VM 으로 돌리고 전역 변수 '기록' 을 문자열로 돌려준다
std::string runVm(const std::string& source)
{
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty());
    bytecode::Chunk chunk;
    bytecode::Compiler compiler;
    if (!compiler.compile(program.get(), &chunk))
    {
        return "COMPILE_ERROR: " + (compiler.errors().empty() ? std::string() : compiler.errors()[0]);
    }
    bytecode::VM vm;
    vm.run(&chunk);
    return vm.globals()->get("기록").toString();
}

/// 평가기와 VM 이 같은 결과를 내는지 확인하고 그 결과를 돌려준다
std::string runBoth(const std::string& source)
{
    std::string evaluated = runEvaluator(source);
    EXPECT_EQ(runVm(source), evaluated) << source;
    return evaluated;
}

/// 식 하나를 시도 블록 안에서 평가해 결과를, 오류면 "오류" 를 기록한다
std::string tryExpression(const std::string& setup, const std::string& expression)
{
    return runBoth(setup + "\n기록 = \"\"\n시도 {\n    기록 = 문자열(" + expression +
                   ")\n} 오류 (e) {\n    기록 = \"오류\"\n}\n");
}

} // namespace

TEST(ShortCircuitTest, ShouldSkipRightSideWhenLeftDecides)
{
    const std::string setup = "호출 = 0\nf = 함수() { 호출 = 호출 + 1\n 반환 참 }\n";
    EXPECT_EQ(runBoth(setup + "a = 거짓 && f()\nb = 참 || f()\nc = 거짓 그리고 f()\nd = 참 또는 f()\n기록 = 호출\n"), "0");
    EXPECT_EQ(runBoth(setup + "a = 참 && f()\nb = 거짓 || f()\n기록 = 호출\n"), "2");
}

TEST(ShortCircuitTest, ShouldGuardIndexAccess)
{
    const std::string setup = "a = [1, 2]\ni = 5";
    EXPECT_EQ(tryExpression(setup, "i < 길이(a) && a[i] == 1"), "거짓");
    EXPECT_EQ(tryExpression(setup, "i >= 길이(a) || a[i] == 1"), "참");
    // 조건문·반복문·함수 반환값 안에서도 같다
    EXPECT_EQ(runBoth(setup + "\n기록 = 0\n반복 (i < 길이(a) && a[i] > 0) {\n    기록 = 기록 + 1\n}\n"), "0");
    EXPECT_EQ(runBoth(setup + "\n기록 = \"아님\"\n만약 (i < 길이(a) && a[i] == 1) {\n    기록 = \"참\"\n}\n"), "아님");
    EXPECT_EQ(runBoth(setup + "\ng = 함수(j) { 반환 j < 길이(a) && a[j] == 2 }\n기록 = [g(1), g(9)]\n"), "[참, 거짓]");
}

TEST(ShortCircuitTest, ShouldKeepTruthinessResults)
{
    // 결과는 진리값 규칙에 따른 참/거짓이다 (피연산자 값을 그대로 돌려주지 않는다)
    EXPECT_EQ(tryExpression("", "[1 && \"a\", 0 || \"\", \"\" || 2, 없음 && 1, 1 그리고 0, 0 또는 [1]]"),
              "[참, 거짓, 참, 거짓, 거짓, 참]");
    // 오른쪽에서 난 오류는 그대로 전해진다
    EXPECT_EQ(tryExpression("a = []", "참 && a[0] == 1"), "오류");
}
