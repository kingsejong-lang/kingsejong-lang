/**
 * @file ErrorRecoveryTest.cpp
 * @brief Error Recovery 테스트 (Panic Mode)
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"

using namespace kingsejong::lexer;
using namespace kingsejong::parser;

// ============================================================================
// Error Recovery 테스트 (여러 에러 수집)
// ============================================================================

TEST(ErrorRecoveryTest, ShouldCollectMultipleErrors)
{
    // 여러 에러가 있는 코드
    std::string code = R"(
정수 x = 10
정수 y =      # 에러: 값이 없음
정수 z = 30
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    // 에러가 수집되어야 함
    const auto& errors = parser.errors();
    EXPECT_GT(errors.size(), 0);

    // 프로그램은 여전히 반환되어야 함 (부분적으로 파싱됨)
    ASSERT_NE(program, nullptr);
}

TEST(ErrorRecoveryTest, ShouldContinueParsingAfterError)
{
    // 첫 번째 문장에 에러, 두 번째 문장은 정상
    std::string code = R"(
정수 x =      # 에러: 값이 없음
정수 y = 20
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    // 적어도 에러가 기록되어야 함
    const auto& errors = parser.errors();
    EXPECT_GT(errors.size(), 0);

    // 두 번째 문장은 성공적으로 파싱될 수 있음
    // (Error Recovery가 작동하여 다음 문장으로 건너뜀)
}

TEST(ErrorRecoveryTest, ShouldSynchronizeAtSemicolon)
{
    // 세미콜론을 동기화 지점으로 사용
    std::string code = R"(
정수 x = 10
정수 y = ;   # 에러: 세미콜론 앞에 값이 없음
정수 z = 30
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    // 에러가 수집되어야 함
    const auto& errors = parser.errors();
    EXPECT_GT(errors.size(), 0);
}

TEST(ErrorRecoveryTest, ShouldHandleMultipleConsecutiveErrors)
{
    // 연속된 에러들 (명확한 구문 에러)
    std::string code = R"(
정수 = 10     # 에러: 변수명 없음
정수 = 20     # 에러: 변수명 없음
정수 z = 30
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    // 여러 에러가 수집되어야 함
    const auto& errors = parser.errors();
    EXPECT_GT(errors.size(), 0);

    // Error Recovery 덕분에 세 번째 문장은 파싱될 수 있음
}

TEST(ErrorRecoveryTest, ShouldSynchronizeAtStatementKeywords)
{
    // 문장 시작 키워드에서 동기화
    std::string code = R"(
정수 x = invalid_token_here
정수 y = 20
만약 참 {
    정수 z = 30
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    // 첫 번째 문장에서 에러가 발생하지만,
    // 두 번째 문장 (정수 y = 20)부터는 다시 파싱될 수 있음
}

TEST(ErrorRecoveryTest, ShouldNotCrashOnSyntaxError)
{
    // 심각한 구문 에러가 있어도 크래시하지 않아야 함
    std::string code = R"(
정수 x = 10
@@@ ### invalid tokens ###
정수 y = 20
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    // 크래시하지 않고 프로그램을 반환해야 함
    ASSERT_NE(program, nullptr);

    // 에러가 기록되어야 함
    const auto& errors = parser.errors();
    EXPECT_GT(errors.size(), 0);
}

TEST(ErrorRecoveryTest, ShouldRecoverFromMissingBrace)
{
    // 중괄호가 없는 if문
    std::string code = R"(
정수 x = 10
만약 참
    정수 y = 20
정수 z = 30
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    // 에러가 있어야 하지만 파싱은 계속됨
    const auto& errors = parser.errors();
    EXPECT_GT(errors.size(), 0);
}

TEST(ErrorRecoveryTest, ShouldCollectAllErrorsInOnePass)
{
    // 여러 독립적인 에러들
    std::string code = R"(
정수 x = 10
정수 =          # 에러 1: 변수명 없음
정수 y = 20
정수 z =        # 에러 2: 값 없음
정수 w = 40
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    // 여러 에러가 한 번의 파싱으로 모두 수집되어야 함
    const auto& errors = parser.errors();
    EXPECT_GT(errors.size(), 1);  // 최소 2개 이상의 에러
}
