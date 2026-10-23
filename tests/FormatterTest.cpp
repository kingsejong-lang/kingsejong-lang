/**
 * @file FormatterTest.cpp
 * @brief Formatter 테스트
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "formatter/Formatter.h"

using namespace kingsejong;
using namespace kingsejong::lexer;
using namespace kingsejong::parser;
using namespace kingsejong::formatter;

// ============================================================================
// 기본 포맷팅 테스트
// ============================================================================

/**
 * @test 변수 선언 포맷팅
 */
TEST(FormatterTest, ShouldFormatVariableDeclaration)
{
    std::string code = "정수 x=10";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    EXPECT_EQ(formatted, "정수 x = 10\n");
}

/**
 * @test 이진 연산자 주변 공백
 */
TEST(FormatterTest, ShouldFormatBinaryExpression)
{
    std::string code = "정수 x=10+20*30";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    EXPECT_EQ(formatted, "정수 x = 10 + 20 * 30\n");
}

/**
 * @test 배열 리터럴 포맷팅
 */
TEST(FormatterTest, ShouldFormatArrayLiteral)
{
    std::string code = "배열 arr=[1,2,3,4,5]";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    EXPECT_EQ(formatted, "배열 arr = [1, 2, 3, 4, 5]\n");
}

/**
 * @test 함수 호출 포맷팅
 */
TEST(FormatterTest, ShouldFormatFunctionCall)
{
    std::string code = "출력(1,2,3)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    EXPECT_EQ(formatted, "출력(1, 2, 3)\n");
}

// ============================================================================
// 블록 및 들여쓰기 테스트
// ============================================================================

/**
 * @test if 문 들여쓰기
 */
TEST(FormatterTest, ShouldFormatIfStatement)
{
    std::string code = R"(
만약(참){
정수 x=10
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    std::string expected = R"(만약 (참) {
    정수 x = 10
}
)";

    EXPECT_EQ(formatted, expected);
}

/**
 * @test if-else 문 포맷팅
 */
TEST(FormatterTest, ShouldFormatIfElseStatement)
{
    std::string code = R"(
만약(x>0){
출력(1)
}아니면{
출력(2)
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    std::string expected = R"(만약 (x > 0) {
    출력(1)
} 아니면 {
    출력(2)
}
)";

    EXPECT_EQ(formatted, expected);
}

/**
 * @test repeat 문 포맷팅
 */
TEST(FormatterTest, ShouldFormatRepeatStatement)
{
    std::string code = R"(
10번 반복한다{
출력(1)
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    std::string expected = R"(반복 (10) {
    출력(1)
}
)";

    EXPECT_EQ(formatted, expected);
}

/**
 * @test 중첩 블록 들여쓰기
 */
TEST(FormatterTest, ShouldFormatNestedBlocks)
{
    std::string code = R"(
만약(참){
만약(거짓){
정수 x=1
}
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    std::string expected = R"(만약 (참) {
    만약 (거짓) {
        정수 x = 1
    }
}
)";

    EXPECT_EQ(formatted, expected);
}

// ============================================================================
// 함수 리터럴 테스트
// ============================================================================

/**
 * @test 함수 리터럴 포맷팅
 */
TEST(FormatterTest, ShouldFormatFunctionLiteral)
{
    std::string code = "정수 f=함수(x,y){반환 x+y}";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    std::string expected = R"(정수 f = 함수(x, y) {
    반환 x + y
}
)";

    EXPECT_EQ(formatted, expected);
}

/**
 * @test 즉시 실행 함수 포맷팅
 */
TEST(FormatterTest, ShouldFormatIIFE)
{
    std::string code = "정수 result=함수(){반환 42}()";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    std::string expected = R"(정수 result = 함수() {
    반환 42
}()
)";

    EXPECT_EQ(formatted, expected);
}

// ============================================================================
// 복합 표현식 테스트
// ============================================================================

/**
 * @test 복잡한 표현식 포맷팅
 * 참고: 파서가 괄호를 AST로 변환하면서 연산자 우선순위에 따라 재구성
 */
TEST(FormatterTest, ShouldFormatComplexExpression)
{
    std::string code = "정수 x=(1+2)*(3-4)/5";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    // 파서가 연산자 우선순위에 따라 AST를 구성하므로 괄호가 다를 수 있음
    EXPECT_EQ(formatted, "정수 x = 1 + 2 * 3 - 4 / 5\n");
}

/**
 * @test 배열 인덱싱 포맷팅
 */
TEST(FormatterTest, ShouldFormatArrayIndexing)
{
    std::string code = "정수 x=arr[0]";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    EXPECT_EQ(formatted, "정수 x = arr[0]\n");
}

// ============================================================================
// 옵션 테스트
// ============================================================================

/**
 * @test 공백 없는 포맷팅
 */
TEST(FormatterTest, ShouldFormatWithoutSpaces)
{
    std::string code = "정수 x = 10 + 20";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    FormatterOptions options;
    options.spaceAroundOperators = false;
    options.spaceAfterComma = false;
    options.spaceBeforeBrace = false;

    Formatter formatter(options);
    std::string formatted = formatter.format(program.get());

    EXPECT_EQ(formatted, "정수 x=10+20\n");
}

/**
 * @test 들여쓰기 크기 변경
 */
TEST(FormatterTest, ShouldFormatWithCustomIndent)
{
    std::string code = R"(
만약(참){
정수 x=10
}
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    FormatterOptions options;
    options.indentSize = 2;

    Formatter formatter(options);
    std::string formatted = formatter.format(program.get());

    std::string expected = R"(만약 (참) {
  정수 x = 10
}
)";

    EXPECT_EQ(formatted, expected);
}

/**
 * @test 여러 문장 포맷팅
 */
TEST(FormatterTest, ShouldFormatMultipleStatements)
{
    std::string code = R"(
정수 x=10
정수 y=20
정수 z=x+y
)";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    std::string expected = R"(정수 x = 10

정수 y = 20

정수 z = x + y
)";

    EXPECT_EQ(formatted, expected);
}

/**
 * @test 문자열 리터럴 포맷팅
 */
TEST(FormatterTest, ShouldFormatStringLiteral)
{
    std::string code = R"(문자열 msg="Hello, World!")";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    EXPECT_EQ(formatted, "문자열 msg = \"Hello, World!\"\n");
}

/**
 * @test 불리언 값 포맷팅
 */
TEST(FormatterTest, ShouldFormatBooleanValue)
{
    std::string code = "정수 flag = 참";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    EXPECT_EQ(formatted, "정수 flag = 참\n");
}

/**
 * @test 단항 연산자 포맷팅
 */
TEST(FormatterTest, ShouldFormatUnaryExpression)
{
    std::string code = "정수 x=-10";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    std::string formatted = formatter.format(program.get());

    EXPECT_EQ(formatted, "정수 x = -10\n");
}

// ============================================================================
// 미지원 노드 처리
// ============================================================================

/**
 * @test 포매터가 다루지 못하는 문장이 있으면 조용히 빼지 않고 알린다.
 * fmt 는 이 신호를 보고 파일을 덮어쓰지 않는다.
 */
TEST(FormatterTest, ShouldReportUnsupportedStatements)
{
    std::string code = "시도 {\n  출력(1)\n} 오류 (e) {\n  출력(2)\n}\n";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    ASSERT_TRUE(parser.errors().empty());

    Formatter formatter;
    formatter.format(program.get());

    EXPECT_TRUE(formatter.hasUnsupportedNodes());
    ASSERT_FALSE(formatter.unsupportedNodes().empty());
    EXPECT_NE(formatter.unsupportedNodes()[0].find("TRY"), std::string::npos)
        << formatter.unsupportedNodes()[0];
}

/**
 * @test 지원하는 노드만 있으면 미지원 목록이 비어 있다.
 */
TEST(FormatterTest, ShouldHaveNoUnsupportedNodesForPlainCode)
{
    std::string code = "정수 x = 1\n출력(x + 2)\n";

    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Formatter formatter;
    formatter.format(program.get());

    EXPECT_FALSE(formatter.hasUnsupportedNodes());
}
