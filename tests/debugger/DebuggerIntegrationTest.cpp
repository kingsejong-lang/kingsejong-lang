/**
 * @file DebuggerIntegrationTest.cpp
 * @brief 디버거가 평가기의 관찰자로 붙어 실제 실행을 멈추는지 확인
 */

#include <gtest/gtest.h>
#include <sstream>
#include "debugger/Debugger.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"

using namespace kingsejong;

namespace {

std::string runWithDebugger(const std::string& code, int breakLine, const std::string& commands)
{
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(code, "dbg.ksj");
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    if (!parser.errors().empty()) {
        return "PARSE ERROR: " + parser.errors()[0];
    }

    std::istringstream input(commands);
    std::ostringstream output;

    debugger::Debugger dbg;
    dbg.setStreams(input, output);
    dbg.getBreakpoints().add(error::SourceLocation("dbg.ksj", breakLine, 1));
    dbg.continueExecution();  // run 과 같은 상태

    auto env = std::make_shared<evaluator::Environment>();
    evaluator::Evaluator ev(env);
    ev.setObserver(&dbg);
    try {
        ev.evalProgram(program.get());
    } catch (const debugger::DebuggerExit&) {
        output << "[exit]";
    }
    return output.str();
}

} // namespace

TEST(DebuggerIntegrationTest, StopsAtBreakpointAndResumes)
{
    std::string code = "정수 x = 1\n정수 y = 2\n정수 z = 3\n";
    std::string out = runWithDebugger(code, 2, "continue\n");

    EXPECT_NE(out.find("중단: dbg.ksj:2"), std::string::npos) << out;
    EXPECT_EQ(out.find("중단: dbg.ksj:3"), std::string::npos) << out;
}

TEST(DebuggerIntegrationTest, StepStopsAtNextStatement)
{
    std::string code = "정수 x = 1\n정수 y = 2\n정수 z = 3\n";
    std::string out = runWithDebugger(code, 1, "step\ncontinue\n");

    EXPECT_NE(out.find("중단: dbg.ksj:1"), std::string::npos) << out;
    EXPECT_NE(out.find("중단: dbg.ksj:2"), std::string::npos) << out;
    EXPECT_EQ(out.find("중단: dbg.ksj:3"), std::string::npos) << out;
}

TEST(DebuggerIntegrationTest, BacktraceShowsCurrentFunction)
{
    std::string code = "함수 f() {\n  정수 inner = 1\n  반환 inner\n}\n정수 r = f()\n";
    std::string out = runWithDebugger(code, 2, "backtrace\ncontinue\n");

    EXPECT_NE(out.find("중단: dbg.ksj:2"), std::string::npos) << out;
    EXPECT_NE(out.find("#0 f"), std::string::npos) << out;
}

TEST(DebuggerIntegrationTest, PrintEvaluatesInPausedEnvironment)
{
    std::string code = "정수 x = 41\n정수 y = x + 1\n정수 z = 0\n";
    std::string out = runWithDebugger(code, 3, "print y\ncontinue\n");

    EXPECT_NE(out.find("y = 42"), std::string::npos) << out;
}

TEST(DebuggerIntegrationTest, QuitStopsExecution)
{
    std::string code = "정수 x = 1\n정수 y = 2\n";
    std::string out = runWithDebugger(code, 1, "quit\n");

    EXPECT_NE(out.find("[exit]"), std::string::npos) << out;
    EXPECT_EQ(out.find("중단: dbg.ksj:2"), std::string::npos) << out;
}
