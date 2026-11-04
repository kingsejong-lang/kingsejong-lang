/**
 * @file ContainerOpsTest.cpp
 * @brief 길이(딕셔너리)와 비교 함수를 받는 정렬한다 테스트
 *
 * 정렬한다(비교 함수) 는 qsort 처럼 비교 함수가 음수를 돌려주면 앞 요소를 앞에 둔다. 안정 정렬이다.
 * 평가기와 VM 이 같은 결과를 내는지 본다.
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "semantic/SemanticAnalyzer.h"
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

std::vector<std::string> semanticErrors(const std::string& source)
{
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    semantic::SemanticAnalyzer analyzer;
    analyzer.analyze(program.get());
    std::vector<std::string> messages;
    for (const auto& e : analyzer.errors())
    {
        messages.push_back(e.message);
    }
    return messages;
}

} // namespace

TEST(ContainerOpsTest, ShouldCountDictionaryKeys)
{
    EXPECT_EQ(tryExpression("", "길이({\"a\": 1, \"b\": 2})"), "2");
    EXPECT_EQ(tryExpression("", "길이({})"), "0");
    EXPECT_EQ(tryExpression("d = {\"a\": 1}\nd[\"b\"] = 2", "길이(d)"), "2");
    // 문자열·배열은 그대로, 그 밖의 타입은 오류다
    EXPECT_EQ(tryExpression("", "길이(\"안녕\")"), "2");
    EXPECT_EQ(tryExpression("", "길이([1, 2, 3])"), "3");
    EXPECT_EQ(tryExpression("n = 5", "길이(n)"), "오류");
}

TEST(ContainerOpsTest, ShouldSortWithComparator)
{
    const std::string setup = "a = [3, 1, 2]";
    EXPECT_EQ(tryExpression(setup, "a를 정렬한다(함수(x, y) { 반환 y - x })"), "[3, 2, 1]");
    EXPECT_EQ(tryExpression(setup, "a를 정렬한다(함수(x, y) { 반환 x - y })"), "[1, 2, 3]");
    // 원본 배열은 바뀌지 않는다
    EXPECT_EQ(runBoth(setup + "\nb = a를 정렬한다(함수(x, y) { 반환 y - x })\n기록 = a\n"), "[3, 1, 2]");
    // 비교 함수 없는 정렬은 전과 같다
    EXPECT_EQ(tryExpression(setup, "a를 정렬한다"), "[1, 2, 3]");
}

TEST(ContainerOpsTest, ShouldMatchDefaultOrderOnLargerArrays)
{
    // 길이가 2의 거듭제곱이 아닌 배열(병합 정렬의 끝 조각)도 기본 정렬과 같은 결과를 낸다
    std::string source =
        "a = []\n"
        "i가 0부터 37 미만 {\n"
        "    a = a + [(i * 17) % 37]\n"
        "}\n"
        "기록 = (a를 정렬한다(함수(x, y) { 반환 x - y })) == (a를 정렬한다)\n";
    EXPECT_EQ(runBoth(source), "참");
}

TEST(ContainerOpsTest, ShouldKeepOrderOfEqualElements)
{
    const std::string setup = "사람들 = [[\"철수\", 30], [\"영희\", 25], [\"민수\", 30], [\"지수\", 25]]";
    EXPECT_EQ(tryExpression(setup, "사람들을 정렬한다(함수(p, q) { 반환 p[1] - q[1] })"),
              "[[영희, 25], [지수, 25], [철수, 30], [민수, 30]]");
}

TEST(ContainerOpsTest, ShouldAcceptFloatComparisonResults)
{
    EXPECT_EQ(tryExpression("a = [2.5, 1.5, 2.25]", "a를 정렬한다(함수(x, y) { 반환 x - y })"), "[1.5, 2.25, 2.5]");
}

TEST(ContainerOpsTest, ShouldRejectBadComparators)
{
    const std::string setup = "a = [3, 1, 2]";
    EXPECT_EQ(tryExpression(setup, "a를 정렬한다(함수(x) { 반환 x })"), "오류");               // 매개변수 수
    EXPECT_EQ(tryExpression(setup, "a를 정렬한다(함수(x, y) { 반환 x < y })"), "오류");       // 수가 아닌 결과
    EXPECT_EQ(tryExpression(setup, "a를 정렬한다(5)"), "오류");                               // 함수가 아님
    EXPECT_EQ(tryExpression(setup, "a를 정렬한다(함수(x, y) { 반환 x - y }, 1)"), "오류");    // 인자 수
}

TEST(ContainerOpsTest, ShouldPropagateComparatorErrors)
{
    std::string source =
        "기록 = \"\"\n"
        "a = [1, 2]\n"
        "시도 {\n"
        "    b = a를 정렬한다(함수(x, y) { 반환 x / 0 })\n"
        "} 오류 (e) {\n"
        "    기록 = \"잡음\"\n"
        "}\n";
    EXPECT_EQ(runBoth(source), "잡음");
}

TEST(ContainerOpsTest, ShouldNotLoseElementsWithInconsistentComparator)
{
    // 늘 -1 을 돌려주는 비교 함수: 순서는 정해지지 않지만 요소를 잃거나 배열 밖을 읽지 않는다.
    // (std::sort 계열에 넘기면 MSVC 디버그 STL 이 invalid comparator 로 프로세스를 끝냈다)
    std::string source =
        "a = []\n"
        "i가 0부터 100 미만 {\n"
        "    a = a + [i]\n"
        "}\n"
        "b = a를 정렬한다(함수(x, y) { 반환 -1 })\n"
        "합 = 0\n"
        "i가 0부터 길이(b) 미만 {\n"
        "    합 = 합 + b[i]\n"
        "}\n"
        "기록 = [길이(b), 합]\n";
    EXPECT_EQ(runBoth(source), "[100, 4950]");  // 요소가 그대로 다 있다
}

TEST(ContainerOpsTest, ShouldPassSemanticAnalysis)
{
    EXPECT_TRUE(semanticErrors("d = {\"a\": 1}\n출력(길이(d))\na = [3, 1]\n출력(a를 정렬한다(함수(x, y) { 반환 x - y }))\n").empty());
}
