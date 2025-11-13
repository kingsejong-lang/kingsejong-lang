/**
 * @file DebuggerReplTest.cpp
 * @brief Debugger REPL 테스트
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * TDD 방식: 테스트를 먼저 작성하고 구현합니다.
 */

#include <gtest/gtest.h>
#include "debugger/Debugger.h"
#include "evaluator/Environment.h"
#include "evaluator/Value.h"
#include "ast/Node.h"
#include <sstream>

using namespace kingsejong::debugger;
using namespace kingsejong::evaluator;
using namespace kingsejong::ast;

/**
 * @class DebuggerReplTest
 * @brief Debugger REPL 테스트 픽스처
 */
class DebuggerReplTest : public ::testing::Test {
protected:
    void SetUp() override {
        debugger = std::make_unique<Debugger>();
        env = std::make_shared<Environment>();
        program = std::make_unique<Program>();
    }

    std::unique_ptr<Debugger> debugger;
    std::shared_ptr<Environment> env;
    std::unique_ptr<Program> program;
};

// ============================================================================
// 기본 REPL 테스트
// ============================================================================

TEST_F(DebuggerReplTest, ShouldHandleQuitCommand) {
    // Arrange
    std::istringstream input("quit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("KingSejong Debugger"), std::string::npos);
    EXPECT_NE(result.find("디버거를 종료합니다"), std::string::npos);
}

TEST_F(DebuggerReplTest, ShouldHandleHelpCommand) {
    // Arrange
    std::istringstream input("help\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("명령어"), std::string::npos);
    EXPECT_NE(result.find("break"), std::string::npos);
    EXPECT_NE(result.find("step"), std::string::npos);
}

TEST_F(DebuggerReplTest, ShouldHandleEmptyInput) {
    // Arrange
    std::istringstream input("\n\n\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert - 오류 없이 종료
    SUCCEED();
}

// ============================================================================
// 브레이크포인트 명령어 테스트
// ============================================================================

TEST_F(DebuggerReplTest, ShouldHandleBreakCommand) {
    // Arrange
    std::istringstream input("break test.ksj:10\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("브레이크포인트 설정"), std::string::npos);
    EXPECT_NE(result.find("test.ksj:10"), std::string::npos);
}

TEST_F(DebuggerReplTest, ShouldHandleConditionalBreakpoint) {
    // Arrange
    std::istringstream input("break test.ksj:10 x > 5\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("조건부 브레이크포인트"), std::string::npos);
    EXPECT_NE(result.find("x > 5"), std::string::npos);
}

TEST_F(DebuggerReplTest, ShouldHandleDeleteAllBreakpoints) {
    // Arrange
    std::istringstream input("break test.ksj:10\ndelete all\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("모든 브레이크포인트를 삭제"), std::string::npos);
}

// ============================================================================
// 실행 제어 명령어 테스트
// ============================================================================

TEST_F(DebuggerReplTest, ShouldHandleStepCommand) {
    // Arrange
    std::istringstream input("step\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("단계 실행"), std::string::npos);
    EXPECT_EQ(debugger->getState(), DebuggerState::STEPPING);
}

TEST_F(DebuggerReplTest, ShouldHandleNextCommand) {
    // Arrange
    std::istringstream input("next\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("다음 줄"), std::string::npos);
    EXPECT_EQ(debugger->getState(), DebuggerState::STEPPING_OVER);
}

TEST_F(DebuggerReplTest, ShouldHandleContinueCommand) {
    // Arrange
    std::istringstream input("continue\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("계속 실행"), std::string::npos);
    EXPECT_EQ(debugger->getState(), DebuggerState::RUNNING);
}

// ============================================================================
// 검사 명령어 테스트
// ============================================================================

TEST_F(DebuggerReplTest, ShouldHandlePrintCommand) {
    // Arrange
    env->set("x", Value::createInteger(42));
    std::istringstream input("print x\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("x = 42"), std::string::npos);
}

TEST_F(DebuggerReplTest, ShouldHandlePrintExpression) {
    // Arrange
    env->set("x", Value::createInteger(10));
    env->set("y", Value::createInteger(20));
    std::istringstream input("print x + y\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("30"), std::string::npos);
}

TEST_F(DebuggerReplTest, ShouldHandleBacktraceCommand) {
    // Arrange
    std::istringstream input("backtrace\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("호출 스택"), std::string::npos);
}

TEST_F(DebuggerReplTest, ShouldHandleWatchCommand) {
    // Arrange
    env->set("x", Value::createInteger(10));
    std::istringstream input("watch x\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("와치포인트 설정"), std::string::npos);
}

TEST_F(DebuggerReplTest, ShouldHandleUnwatchCommand) {
    // Arrange
    env->set("x", Value::createInteger(10));
    std::istringstream input("watch x\nunwatch x\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("와치포인트 제거"), std::string::npos);
}

// ============================================================================
// 별칭 테스트
// ============================================================================

TEST_F(DebuggerReplTest, ShouldHandleAliases) {
    // Arrange
    std::istringstream input("b test.ksj:10\ns\nn\nc\nh\nq\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("브레이크포인트"), std::string::npos);
    EXPECT_NE(result.find("단계 실행"), std::string::npos);
    EXPECT_NE(result.find("다음 줄"), std::string::npos);
    EXPECT_NE(result.find("계속 실행"), std::string::npos);
    EXPECT_NE(result.find("명령어"), std::string::npos);
}

// ============================================================================
// 에러 처리 테스트
// ============================================================================

TEST_F(DebuggerReplTest, ShouldHandleUnknownCommand) {
    // Arrange
    std::istringstream input("invalid_command\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("알 수 없는 명령어"), std::string::npos);
}

TEST_F(DebuggerReplTest, ShouldHandleInvalidBreakFormat) {
    // Arrange
    std::istringstream input("break invalid_format\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("잘못된 형식"), std::string::npos);
}

TEST_F(DebuggerReplTest, ShouldHandlePrintWithoutArgs) {
    // Arrange
    std::istringstream input("print\nquit\n");
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("사용법"), std::string::npos);
}

// ============================================================================
// 통합 시나리오 테스트
// ============================================================================

TEST_F(DebuggerReplTest, ShouldHandleCompleteSession) {
    // Arrange
    env->set("x", Value::createInteger(10));
    env->set("y", Value::createInteger(20));

    std::istringstream input(
        "break test.ksj:10\n"
        "watch x\n"
        "print x + y\n"
        "step\n"
        "backtrace\n"
        "help\n"
        "quit\n"
    );
    std::ostringstream output;

    // Act
    debugger->repl(program.get(), env, input, output);

    // Assert
    std::string result = output.str();
    EXPECT_NE(result.find("브레이크포인트 설정"), std::string::npos);
    EXPECT_NE(result.find("와치포인트 설정"), std::string::npos);
    EXPECT_NE(result.find("30"), std::string::npos);
    EXPECT_NE(result.find("단계 실행"), std::string::npos);
    EXPECT_NE(result.find("호출 스택"), std::string::npos);
    EXPECT_NE(result.find("명령어"), std::string::npos);
    EXPECT_NE(result.find("디버거를 종료"), std::string::npos);
}
