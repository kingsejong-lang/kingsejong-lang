/**
 * @file LoopScopeTest.cpp
 * @brief 반복문은 회차마다 새 환경에서 본문을 돈다
 *
 * 반복 변수와 본문에서 처음 만든 변수는 회차마다 따로 생기므로, 본문에서 만든 클로저는 그 회차의 값을 잡는다.
 * 바깥에 이미 있는 변수에 대입하면 바깥 변수가 바뀐다. 평가기와 VM 이 같은 결과를 내는지 본다.
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

/// 평가기로 돌리고 프로그램의 마지막 식 값을 문자열로 돌려준다. 실행 오류는 "오류" 로 돌려준다
/// 환경을 테스트가 쥐지 않아야 평가기 소멸자가 바깥으로 빠져나간 클로저의 순환 참조까지 끊는다
std::string runEvaluator(const std::string& source)
{
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty()) << (parser.errors().empty() ? "" : parser.errors()[0]);
    try
    {
        evaluator::Evaluator evaluator(std::make_shared<evaluator::Environment>());
        return evaluator.evalProgram(program.get()).toString();
    }
    catch (const std::exception&)
    {
        return "오류";
    }
}

/// VM 으로 돌리고 전역 변수 '기록' 을 문자열로 돌려준다. 실행 오류는 "오류" 로 돌려준다
std::string runVm(const std::string& source)
{
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty());
    bytecode::Chunk chunk;
    bytecode::Compiler compiler;
    EXPECT_TRUE(compiler.compile(program.get(), &chunk));
    bytecode::VM vm;
    if (vm.run(&chunk) == bytecode::VMResult::RUNTIME_ERROR)
    {
        return "오류";
    }
    return vm.globals()->get("기록").toString();
}

/// 프로그램은 마지막 줄에서 '기록' 을 식으로 둔다. 평가기와 VM 이 같은지 확인하고 결과를 돌려준다
std::string runBoth(const std::string& source)
{
    std::string evaluated = runEvaluator(source + "\n기록\n");
    EXPECT_EQ(runVm(source + "\n기록\n"), evaluated) << source;
    return evaluated;
}

} // namespace

TEST(LoopScopeTest, RangeLoopClosuresCaptureEachIteration)
{
    EXPECT_EQ(runBoth(
        "함수들 = []\n"
        "i가 1부터 3까지 {\n"
        "    f = 함수() { 반환 i }\n"
        "    함수들 = 추가(함수들, f)\n"
        "}\n"
        "기록 = [함수들[0](), 함수들[1](), 함수들[2]()]\n"), "[1, 2, 3]");
}

TEST(LoopScopeTest, BodyVariablesAreFreshEachIteration)
{
    // 본문에서 처음 만든 변수도 회차마다 따로 생긴다 (반복 조건 반복문)
    EXPECT_EQ(runBoth(
        "함수들 = []\n"
        "j = 0\n"
        "반복 (j < 3) {\n"
        "    k = j * 10\n"
        "    g = 함수() { 반환 k }\n"
        "    함수들 = 추가(함수들, g)\n"
        "    j = j + 1\n"
        "}\n"
        "기록 = [함수들[0](), 함수들[1](), 함수들[2]()]\n"), "[0, 10, 20]");
    // N번 반복
    EXPECT_EQ(runBoth(
        "함수들 = []\n"
        "n = 0\n"
        "3번 반복한다 {\n"
        "    n = n + 1\n"
        "    m = n\n"
        "    h = 함수() { 반환 m }\n"
        "    함수들 = 추가(함수들, h)\n"
        "}\n"
        "기록 = [함수들[0](), 함수들[1](), 함수들[2]()]\n"), "[1, 2, 3]");
}

TEST(LoopScopeTest, OuterVariablesAreStillUpdated)
{
    // 바깥에 이미 있는 변수에 대입하면 바깥 변수가 바뀐다 (누적 패턴)
    EXPECT_EQ(runBoth("합 = 0\ni가 1부터 10까지 { 합 = 합 + i }\n기록 = 합\n"), "55");
    EXPECT_EQ(runBoth("최근 = 0\ni가 1부터 3까지 { 최근 = i }\n기록 = 최근\n"), "3");
}

TEST(LoopScopeTest, LoopVariablesDoNotOutliveTheLoop)
{
    // 반복 변수와 본문에서 처음 만든 변수는 반복문 뒤에 없다 (의미 분석기의 범위 규칙과 같다)
    EXPECT_EQ(runBoth("기록 = 0\ni가 1부터 3까지 { 임시 = i }\n기록 = i\n"), "오류");
    EXPECT_EQ(runBoth("기록 = 0\ni가 1부터 3까지 { 임시 = i }\n기록 = 임시\n"), "오류");
}

TEST(LoopScopeTest, BreakAndContinueCloseTheIterationScope)
{
    // 중단·계속 뒤에도 스코프가 맞아야 반복문 뒤에 만든 변수가 전역에 남는다
    EXPECT_EQ(runBoth(
        "i가 1부터 10까지 {\n"
        "    만약 (i == 2) { 계속 }\n"
        "    만약 (i == 4) { 중단 }\n"
        "}\n"
        "기록 = \"뒤\"\n"), "뒤");
    EXPECT_EQ(runBoth(
        "합 = 0\n"
        "i가 1부터 3까지 {\n"
        "    j가 1부터 3까지 {\n"
        "        만약 (j == 2) { 중단 }\n"
        "        합 = 합 + i * 10 + j\n"
        "    }\n"
        "}\n"
        "기록 = 합\n"), "63");
    EXPECT_EQ(runBoth(
        "합 = 0\n"
        "i가 1부터 3까지 {\n"
        "    시도 {\n"
        "        만약 (i == 2) { 계속 }\n"
        "        합 = 합 + i\n"
        "    } 오류 (e) {\n"
        "        합 = -1\n"
        "    }\n"
        "}\n"
        "기록 = 합\n"), "4");
}

TEST(LoopScopeTest, ErrorInsideLoopIsCaughtOutside)
{
    // 반복문 밖의 시도 블록이 잡은 뒤에도 전역 환경으로 돌아와 있어야 한다
    EXPECT_EQ(runBoth(
        "시도 {\n"
        "    i가 1부터 3까지 {\n"
        "        만약 (i == 2) { 던지다 \"멈춤\" }\n"
        "    }\n"
        "} 오류 (e) {\n"
        "    잡음 = e\n"
        "}\n"
        "뒤 = \"전역\"\n"
        "기록 = 뒤\n"), "전역");
}
