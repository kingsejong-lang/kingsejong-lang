/**
 * @file CommandParserTest.cpp
 * @brief CommandParser 테스트
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * TDD 방식: 테스트를 먼저 작성하고 구현합니다.
 */

#include <gtest/gtest.h>
#include "debugger/CommandParser.h"

using namespace kingsejong::debugger;

/**
 * @class CommandParserTest
 * @brief CommandParser 테스트 픽스처
 */
class CommandParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser = std::make_unique<CommandParser>();
    }

    std::unique_ptr<CommandParser> parser;
};

// ============================================================================
// 기본 명령어 테스트
// ============================================================================

TEST_F(CommandParserTest, ShouldParseBreakCommand) {
    Command cmd = parser->parse("break main.ksj:10");

    EXPECT_EQ(cmd.type, CommandType::BREAK);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "main.ksj:10");
}

TEST_F(CommandParserTest, ShouldParseBreakCommandWithCondition) {
    Command cmd = parser->parse("break main.ksj:10 x > 5");

    EXPECT_EQ(cmd.type, CommandType::BREAK);
    ASSERT_EQ(cmd.args.size(), 2u);
    EXPECT_EQ(cmd.args[0], "main.ksj:10");
    EXPECT_EQ(cmd.args[1], "x > 5");
}

TEST_F(CommandParserTest, ShouldParseDeleteCommand) {
    Command cmd = parser->parse("delete 1");

    EXPECT_EQ(cmd.type, CommandType::DELETE);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "1");
}

TEST_F(CommandParserTest, ShouldParseRunCommand) {
    Command cmd = parser->parse("run");

    EXPECT_EQ(cmd.type, CommandType::RUN);
    EXPECT_EQ(cmd.args.size(), 0u);
}

TEST_F(CommandParserTest, ShouldParseContinueCommand) {
    Command cmd = parser->parse("continue");

    EXPECT_EQ(cmd.type, CommandType::CONTINUE);
    EXPECT_EQ(cmd.args.size(), 0u);
}

TEST_F(CommandParserTest, ShouldParseStepCommand) {
    Command cmd = parser->parse("step");

    EXPECT_EQ(cmd.type, CommandType::STEP);
    EXPECT_EQ(cmd.args.size(), 0u);
}

TEST_F(CommandParserTest, ShouldParseNextCommand) {
    Command cmd = parser->parse("next");

    EXPECT_EQ(cmd.type, CommandType::NEXT);
    EXPECT_EQ(cmd.args.size(), 0u);
}

TEST_F(CommandParserTest, ShouldParsePrintCommand) {
    Command cmd = parser->parse("print x");

    EXPECT_EQ(cmd.type, CommandType::PRINT);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "x");
}

TEST_F(CommandParserTest, ShouldParseBacktraceCommand) {
    Command cmd = parser->parse("backtrace");

    EXPECT_EQ(cmd.type, CommandType::BACKTRACE);
    EXPECT_EQ(cmd.args.size(), 0u);
}

TEST_F(CommandParserTest, ShouldParseListCommand) {
    Command cmd = parser->parse("list");

    EXPECT_EQ(cmd.type, CommandType::LIST);
    EXPECT_EQ(cmd.args.size(), 0u);
}

TEST_F(CommandParserTest, ShouldParseListCommandWithLineNumber) {
    Command cmd = parser->parse("list 10");

    EXPECT_EQ(cmd.type, CommandType::LIST);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "10");
}

TEST_F(CommandParserTest, ShouldParseWatchCommand) {
    Command cmd = parser->parse("watch x");

    EXPECT_EQ(cmd.type, CommandType::WATCH);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "x");
}

TEST_F(CommandParserTest, ShouldParseUnwatchCommand) {
    Command cmd = parser->parse("unwatch x");

    EXPECT_EQ(cmd.type, CommandType::UNWATCH);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "x");
}

TEST_F(CommandParserTest, ShouldParseHelpCommand) {
    Command cmd = parser->parse("help");

    EXPECT_EQ(cmd.type, CommandType::HELP);
    EXPECT_EQ(cmd.args.size(), 0u);
}

TEST_F(CommandParserTest, ShouldParseQuitCommand) {
    Command cmd = parser->parse("quit");

    EXPECT_EQ(cmd.type, CommandType::QUIT);
    EXPECT_EQ(cmd.args.size(), 0u);
}

// ============================================================================
// 별칭(Alias) 테스트
// ============================================================================

TEST_F(CommandParserTest, ShouldParseBreakAlias) {
    Command cmd = parser->parse("b main.ksj:10");

    EXPECT_EQ(cmd.type, CommandType::BREAK);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "main.ksj:10");
}

TEST_F(CommandParserTest, ShouldParseDeleteAlias) {
    Command cmd = parser->parse("d 1");

    EXPECT_EQ(cmd.type, CommandType::DELETE);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "1");
}

TEST_F(CommandParserTest, ShouldParseRunAlias) {
    Command cmd = parser->parse("r");

    EXPECT_EQ(cmd.type, CommandType::RUN);
}

TEST_F(CommandParserTest, ShouldParseContinueAlias) {
    Command cmd = parser->parse("c");

    EXPECT_EQ(cmd.type, CommandType::CONTINUE);
}

TEST_F(CommandParserTest, ShouldParseStepAlias) {
    Command cmd = parser->parse("s");

    EXPECT_EQ(cmd.type, CommandType::STEP);
}

TEST_F(CommandParserTest, ShouldParseNextAlias) {
    Command cmd = parser->parse("n");

    EXPECT_EQ(cmd.type, CommandType::NEXT);
}

TEST_F(CommandParserTest, ShouldParsePrintAlias) {
    Command cmd = parser->parse("p x");

    EXPECT_EQ(cmd.type, CommandType::PRINT);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "x");
}

TEST_F(CommandParserTest, ShouldParseBacktraceAlias) {
    Command cmd = parser->parse("bt");

    EXPECT_EQ(cmd.type, CommandType::BACKTRACE);
}

TEST_F(CommandParserTest, ShouldParseListAlias) {
    Command cmd = parser->parse("l");

    EXPECT_EQ(cmd.type, CommandType::LIST);
}

TEST_F(CommandParserTest, ShouldParseWatchAlias) {
    Command cmd = parser->parse("w x");

    EXPECT_EQ(cmd.type, CommandType::WATCH);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "x");
}

TEST_F(CommandParserTest, ShouldParseUnwatchAlias) {
    Command cmd = parser->parse("uw x");

    EXPECT_EQ(cmd.type, CommandType::UNWATCH);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "x");
}

TEST_F(CommandParserTest, ShouldParseHelpAlias) {
    Command cmd = parser->parse("h");

    EXPECT_EQ(cmd.type, CommandType::HELP);
}

TEST_F(CommandParserTest, ShouldParseQuitAlias) {
    Command cmd = parser->parse("q");

    EXPECT_EQ(cmd.type, CommandType::QUIT);
}

// ============================================================================
// 공백 처리 테스트
// ============================================================================

TEST_F(CommandParserTest, ShouldHandleLeadingWhitespace) {
    Command cmd = parser->parse("   step");

    EXPECT_EQ(cmd.type, CommandType::STEP);
}

TEST_F(CommandParserTest, ShouldHandleTrailingWhitespace) {
    Command cmd = parser->parse("step   ");

    EXPECT_EQ(cmd.type, CommandType::STEP);
}

TEST_F(CommandParserTest, ShouldHandleMultipleSpaces) {
    Command cmd = parser->parse("print    x");

    EXPECT_EQ(cmd.type, CommandType::PRINT);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "x");
}

TEST_F(CommandParserTest, ShouldHandleTabs) {
    Command cmd = parser->parse("print\tx");

    EXPECT_EQ(cmd.type, CommandType::PRINT);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "x");
}

// ============================================================================
// 복잡한 인자 테스트
// ============================================================================

TEST_F(CommandParserTest, ShouldParsePrintWithExpression) {
    Command cmd = parser->parse("print x + y");

    EXPECT_EQ(cmd.type, CommandType::PRINT);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "x + y");
}

TEST_F(CommandParserTest, ShouldParseBreakWithComplexCondition) {
    Command cmd = parser->parse("break main.ksj:10 x > 5 그리고 y < 10");

    EXPECT_EQ(cmd.type, CommandType::BREAK);
    ASSERT_EQ(cmd.args.size(), 2u);
    EXPECT_EQ(cmd.args[0], "main.ksj:10");
    EXPECT_EQ(cmd.args[1], "x > 5 그리고 y < 10");
}

// ============================================================================
// 대소문자 테스트
// ============================================================================

TEST_F(CommandParserTest, ShouldBeCaseInsensitive) {
    Command cmd1 = parser->parse("BREAK main.ksj:10");
    Command cmd2 = parser->parse("Break main.ksj:10");
    Command cmd3 = parser->parse("break main.ksj:10");

    EXPECT_EQ(cmd1.type, CommandType::BREAK);
    EXPECT_EQ(cmd2.type, CommandType::BREAK);
    EXPECT_EQ(cmd3.type, CommandType::BREAK);
}

// ============================================================================
// 에러 처리 테스트
// ============================================================================

TEST_F(CommandParserTest, ShouldThrowOnEmptyInput) {
    EXPECT_THROW(parser->parse(""), std::invalid_argument);
}

TEST_F(CommandParserTest, ShouldThrowOnWhitespaceOnlyInput) {
    EXPECT_THROW(parser->parse("   "), std::invalid_argument);
}

TEST_F(CommandParserTest, ShouldReturnUnknownForInvalidCommand) {
    Command cmd = parser->parse("invalid_command");

    EXPECT_EQ(cmd.type, CommandType::UNKNOWN);
}

// ============================================================================
// 엣지 케이스 테스트
// ============================================================================

TEST_F(CommandParserTest, ShouldHandleCommandWithNoArgs) {
    Command cmd = parser->parse("run");

    EXPECT_EQ(cmd.type, CommandType::RUN);
    EXPECT_TRUE(cmd.args.empty());
}

TEST_F(CommandParserTest, ShouldHandleSingleCharacterCommand) {
    Command cmd = parser->parse("s");

    EXPECT_EQ(cmd.type, CommandType::STEP);
}

TEST_F(CommandParserTest, ShouldHandleCommandWithSpecialCharacters) {
    Command cmd = parser->parse("print x->y");

    EXPECT_EQ(cmd.type, CommandType::PRINT);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "x->y");
}

TEST_F(CommandParserTest, ShouldHandleKoreanVariableNames) {
    Command cmd = parser->parse("print 변수명");

    EXPECT_EQ(cmd.type, CommandType::PRINT);
    ASSERT_EQ(cmd.args.size(), 1u);
    EXPECT_EQ(cmd.args[0], "변수명");
}

// ============================================================================
// RAII 및 메모리 안전성 테스트
// ============================================================================

TEST_F(CommandParserTest, ShouldNotLeakOnRepeatedParsing) {
    // AddressSanitizer로 메모리 누수 검증
    for (int i = 0; i < 1000; i++) {
        Command cmd = parser->parse("step");
        EXPECT_EQ(cmd.type, CommandType::STEP);
    }
}

TEST_F(CommandParserTest, ShouldHandleMoveSemantics) {
    auto parser2 = std::make_unique<CommandParser>();
    *parser2 = std::move(*parser);

    Command cmd = parser2->parse("step");
    EXPECT_EQ(cmd.type, CommandType::STEP);
}
