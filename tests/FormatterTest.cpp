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
using namespace kingsejong::ast;

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

    std::string expected = R"(10번 반복한다 {
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

    // 호출되는 함수 리터럴은 괄호로 감싼다 (다시 읽어도 같은 구문 트리)
    std::string expected = R"(정수 result = (함수() {
    반환 42
})()
)";

    EXPECT_EQ(formatted, expected);
}

// ============================================================================
// 복합 표현식 테스트
// ============================================================================

/**
 * @test 복잡한 표현식 포맷팅: 원래 괄호를 남긴다 (예전에는 괄호를 버려 (1+2)*(3-4) 가 1+2*3-4 로 바뀌었다)
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

    EXPECT_EQ(formatted, "정수 x = (1 + 2) * (3 - 4) / 5\n");
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
    // 파서가 만들지 않는 ForStatement 를 직접 넣어 미지원 분기를 확인한다
    Program program;
    std::vector<std::unique_ptr<Statement>> body;
    program.addStatement(std::make_unique<ForStatement>(
        "x", std::make_unique<Identifier>("목록"), std::make_unique<BlockStatement>(std::move(body))));

    Formatter formatter;
    formatter.format(&program);

    EXPECT_TRUE(formatter.hasUnsupportedNodes());
    ASSERT_FALSE(formatter.unsupportedNodes().empty());
    EXPECT_NE(formatter.unsupportedNodes()[0].find("FOR"), std::string::npos)
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

// ============================================================================
// 원본 소스를 참고하는 포맷팅과 검증
// ============================================================================

namespace {

/// 원본을 포맷하고, 결과가 검증을 통과하는지와 한 번 더 포맷해도 같은지 확인한다
std::string formatChecked(const std::string& code)
{
    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty());

    Formatter formatter;
    std::string formatted = formatter.format(program.get(), code);
    EXPECT_FALSE(formatter.hasUnsupportedNodes());

    std::string reason;
    EXPECT_TRUE(Formatter::verify(program.get(), code, formatted, reason)) << reason << "\n" << formatted;

    Lexer lexer2(formatted);
    Parser parser2(lexer2);
    auto program2 = parser2.parseProgram();
    Formatter formatter2;
    EXPECT_EQ(formatter2.format(program2.get(), formatted), formatted) << "두 번 포맷한 결과가 다르다";
    return formatted;
}

} // namespace

TEST(FormatterSourceTest, ShouldKeepCommentsAndBlankLines)
{
    std::string code =
        "# 머리 주석\n"
        "x = 1  # 끝 주석\n"
        "\n"
        "\n"
        "# 함수 설명\n"
        "두배 = 함수(n) {\n"
        "    # 안쪽\n"
        "    반환 n * 2\n"
        "    # 블록 끝\n"
        "}\n"
        "# 파일 끝\n";
    std::string expected =
        "# 머리 주석\n"
        "x = 1  # 끝 주석\n"
        "\n"
        "# 함수 설명\n"
        "두배 = 함수(n) {\n"
        "    # 안쪽\n"
        "    반환 n * 2\n"
        "    # 블록 끝\n"
        "}\n"
        "# 파일 끝\n";
    EXPECT_EQ(formatChecked(code), expected);
}

TEST(FormatterSourceTest, ShouldWriteRealSyntaxForLoopsAndElseIf)
{
    std::string code =
        "i가 1부터 3까지 {\n출력(i)\n}\n"
        "j가 0부터 3 미만 {\n출력(j)\n}\n"
        "3번 반복한다 {\n출력(1)\n}\n"
        "만약 (x > 1) {\n출력(1)\n} 아니면 만약 (x > 0) {\n출력(2)\n} 아니면 {\n출력(3)\n}\n";
    std::string expected =
        "i가 1부터 3까지 {\n    출력(i)\n}\n"
        "j가 0부터 3 미만 {\n    출력(j)\n}\n"
        "3번 반복한다 {\n    출력(1)\n}\n"
        "만약 (x > 1) {\n    출력(1)\n} 아니면 만약 (x > 0) {\n    출력(2)\n} 아니면 {\n    출력(3)\n}\n";
    EXPECT_EQ(formatChecked(code), expected);
}

TEST(FormatterSourceTest, ShouldKeepTheKeywordsTheAuthorChose)
{
    // 같은 뜻의 다른 표기(괄호 없는 조건, 동안, 이하·이상, 선택 키워드 반복)를 바꾸지 않는다
    std::string code =
        "만약 x > 0 {\n출력(1)\n} 아니면 만약 (x < 0) {\n출력(2)\n}\n"
        "동안 (x > 0) {\nx = x - 1\n}\n"
        "i가 1부터 3 이하 반복 {\n출력(i)\n}\n"
        "j가 0부터 2까지 반복 하라 {\n출력(j)\n}\n"
        "k가 0부터 2 미만 반복한다 {\n출력(k)\n}\n"
        "출력(배열[1 이상 3 이하], 배열[0초과 2미만])\n";
    std::string expected =
        "만약 x > 0 {\n    출력(1)\n} 아니면 만약 (x < 0) {\n    출력(2)\n}\n"
        "동안 (x > 0) {\n    x = x - 1\n}\n"
        "i가 1부터 3 이하 반복 {\n    출력(i)\n}\n"
        "j가 0부터 2까지 반복 하라 {\n    출력(j)\n}\n"
        "k가 0부터 2 미만 반복한다 {\n    출력(k)\n}\n"
        "출력(배열[1 이상 3 이하], 배열[0초과 2미만])\n";
    EXPECT_EQ(formatChecked(code), expected);
}

TEST(FormatterSourceTest, ShouldKeepConditionsThatStartWithParentheses)
{
    std::string code =
        "만약 (a) && (b) {\n출력(1)\n}\n"
        "반복 (x > 0) || (y > 0) {\nx = x - 1\n}\n";
    std::string expected =
        "만약 (a) && (b) {\n    출력(1)\n}\n"
        "반복 (x > 0) || (y > 0) {\n    x = x - 1\n}\n";
    EXPECT_EQ(formatChecked(code), expected);
}

TEST(FormatterSourceTest, ShouldNotAddParenthesesAroundJosaOperands)
{
    // 조사식이 바깥 연산자의 오른쪽 식을 끌어가지 않으므로 괄호를 더하지 않는다
    std::string code = "출력(\"[\" + i를 문자열로 변환한다 + \"]\")\n출력(10 - x를 정수로 변환한다 - 3)\n";
    EXPECT_EQ(formatChecked(code), code);
}

TEST(FormatterSourceTest, ShouldFormatPreviouslyUnsupportedSyntax)
{
    std::string code =
        "가져오기 \"stdlib/math\"\n"
        "클래스 개 상속 동물 {\n"
        "    비공개 문자열 이름\n"
        "    생성자(이름) {\n"
        "        자신.이름 = 이름\n"
        "    }\n"
        "    공개 함수 소리() {\n"
        "        반환 \"멍멍 ${자신.이름}\"\n"
        "    }\n"
        "}\n"
        "시도 {\n"
        "    던지다 \"x\"\n"
        "} 오류 (e) {\n"
        "    출력(e)\n"
        "} 마지막 {\n"
        "    출력(\"끝\")\n"
        "}\n"
        "사전 = {\"a\": 1, \"b\": [1, 2]}\n"
        "정렬됨 = [3, 1, 2]를 정렬한다\n"
        "일부 = 목록[2부터 5까지]\n"
        "결과 = 값 에 대해 {\n"
        "    0 -> \"영\"\n"
        "    n when n > 0 -> \"양수\"\n"
        "    _ -> \"음수\"\n"
        "}\n"
        "비동기 함수 가져와() {\n"
        "    반환 대기 다른것()\n"
        "}\n";
    EXPECT_EQ(formatChecked(code), code);
}

TEST(FormatterSourceTest, ShouldKeepMultilineLiterals)
{
    std::string code =
        "목록 = [\n"
        "    \"가\",\n"
        "    # 둘째\n"
        "    \"나\"\n"
        "]\n";
    EXPECT_EQ(formatChecked(code), code);
}

TEST(FormatterSourceTest, ShouldRejectOutputThatChangesTheProgram)
{
    std::string code = "x = (1 + 2) * 3  # 9\n";
    Lexer lexer(code);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    std::string reason;
    EXPECT_FALSE(Formatter::verify(program.get(), code, "x = 1 + 2 * 3  # 9\n", reason));
    EXPECT_FALSE(Formatter::verify(program.get(), code, "x = (1 + 2) * 3\n", reason));
    EXPECT_FALSE(Formatter::verify(program.get(), code, "x = (1 + 2 *\n", reason));
    EXPECT_TRUE(Formatter::verify(program.get(), code, "x = (1 + 2) * 3 # 9\n", reason)) << reason;
}

TEST(FormatterSourceTest, ShouldFindCommentsOutsideStrings)
{
    auto comments = Formatter::extractComments("a = \"#아님\"  # 맞음\n# 줄\n");
    ASSERT_EQ(comments.size(), 2u);
    EXPECT_EQ(comments[0].text, "# 맞음");
    EXPECT_TRUE(comments[0].trailing);
    EXPECT_EQ(comments[1].line, 2);
    EXPECT_FALSE(comments[1].trailing);
}

TEST(FormatterSourceTest, ShouldKeepSlashCommentsAndOneLineBlocks)
{
    // '//' 도 렉서가 주석으로 건너뛰므로 지우면 안 된다. 한 줄 블록은 한 줄로 둔다
    std::string code =
        "// 머리\n"
        "# 둘째\n"
        "\n"
        "짝수 = 숫자들을 걸러낸다(함수(x) { 반환 x % 2 == 0 })  // 끝\n"
        "빈것 = 함수() {}\n";
    EXPECT_EQ(formatChecked(code), code);

    auto comments = Formatter::extractComments(code);
    ASSERT_EQ(comments.size(), 3u);
    EXPECT_EQ(comments[0].text, "// 머리");
    EXPECT_EQ(comments[2].text, "// 끝");
}
