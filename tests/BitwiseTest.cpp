/**
 * @file BitwiseTest.cpp
 * @brief 비트 연산자(&, |, ^, ~, <<, >>)와 16진수·2진수·지수 리터럴 테스트
 *
 * 비트 연산은 정수에만 쓰고, 비교보다 강하게 묶인다(x & 1 == 0 은 (x & 1) == 0).
 * 평가기와 VM 이 같은 결과를 내는지, 포매터가 작성자의 진법 표기를 지키는지 본다.
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "semantic/SemanticAnalyzer.h"
#include "formatter/Formatter.h"
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

std::vector<std::string> parseErrors(const std::string& source)
{
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    parser.parseProgram();
    return parser.errors();
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

std::string format(const std::string& source)
{
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty());
    formatter::Formatter formatter;
    return formatter.format(program.get());
}

std::vector<lexer::TokenType> tokenTypes(const std::string& source)
{
    lexer::Lexer lexer(source);
    std::vector<lexer::TokenType> types;
    for (auto token = lexer.nextToken(); token.type != lexer::TokenType::EOF_TOKEN; token = lexer.nextToken())
    {
        types.push_back(token.type);
    }
    return types;
}

} // namespace

TEST(BitwiseLexerTest, ShouldTokenizeBitwiseOperators)
{
    using lexer::TokenType;
    EXPECT_EQ(tokenTypes("a & b | c ^ ~d << 1 >> 2"),
              (std::vector<TokenType>{TokenType::IDENTIFIER, TokenType::BIT_AND, TokenType::IDENTIFIER,
                                      TokenType::BIT_OR, TokenType::IDENTIFIER, TokenType::BIT_XOR,
                                      TokenType::BIT_NOT, TokenType::IDENTIFIER, TokenType::SHIFT_LEFT,
                                      TokenType::INTEGER, TokenType::SHIFT_RIGHT, TokenType::INTEGER}));
    // 두 글자 논리·비교 연산자는 그대로다
    EXPECT_EQ(tokenTypes("a && b || c <= d >= e"),
              (std::vector<TokenType>{TokenType::IDENTIFIER, TokenType::AND, TokenType::IDENTIFIER, TokenType::OR,
                                      TokenType::IDENTIFIER, TokenType::LE, TokenType::IDENTIFIER, TokenType::GE,
                                      TokenType::IDENTIFIER}));
}

TEST(BitwiseLexerTest, ShouldTokenizeRadixAndExponentLiterals)
{
    using lexer::TokenType;
    lexer::Lexer lexer("0xFF 0b101 1e10 2.5E-3 0x 1e");
    auto hex = lexer.nextToken();
    EXPECT_EQ(hex.type, TokenType::INTEGER);
    EXPECT_EQ(hex.literal, "0xFF");
    auto bin = lexer.nextToken();
    EXPECT_EQ(bin.type, TokenType::INTEGER);
    EXPECT_EQ(bin.literal, "0b101");
    auto exp = lexer.nextToken();
    EXPECT_EQ(exp.type, TokenType::FLOAT);
    EXPECT_EQ(exp.literal, "1e10");
    auto negExp = lexer.nextToken();
    EXPECT_EQ(negExp.type, TokenType::FLOAT);
    EXPECT_EQ(negExp.literal, "2.5E-3");
    // 자릿수가 없는 접두사·지수는 전처럼 숫자와 식별자로 나뉜다
    EXPECT_EQ(lexer.nextToken().literal, "0");
    EXPECT_EQ(lexer.nextToken().literal, "x");
    EXPECT_EQ(lexer.nextToken().literal, "1");
    EXPECT_EQ(lexer.nextToken().literal, "e");
}

TEST(BitwiseTest, ShouldComputeBitwiseOperations)
{
    EXPECT_EQ(tryExpression("", "5 & 3"), "1");
    EXPECT_EQ(tryExpression("", "5 | 3"), "7");
    EXPECT_EQ(tryExpression("", "5 ^ 3"), "6");
    EXPECT_EQ(tryExpression("", "~5"), "-6");
    EXPECT_EQ(tryExpression("", "1 << 4"), "16");
    EXPECT_EQ(tryExpression("", "-16 >> 2"), "-4");  // 부호를 유지한다
    EXPECT_EQ(tryExpression("x = 12", "x & 0xF"), "12");
}

TEST(BitwiseTest, ShouldDropBitsShiftedOutWithoutOverflowError)
{
    EXPECT_EQ(tryExpression("", "1 << 63"), "-9223372036854775808");
    EXPECT_EQ(tryExpression("", "3 << 63"), "-9223372036854775808");
    EXPECT_EQ(tryExpression("", "-1 << 1"), "-2");
}

TEST(BitwiseTest, ShouldRejectShiftCountOutOfRange)
{
    EXPECT_EQ(tryExpression("", "1 << 64"), "오류");
    EXPECT_EQ(tryExpression("n = -1", "1 >> n"), "오류");
    EXPECT_EQ(tryExpression("", "1 << 0"), "1");
    EXPECT_EQ(tryExpression("", "-1 >> 63"), "-1");
}

TEST(BitwiseTest, ShouldRejectNonIntegerOperands)
{
    EXPECT_EQ(tryExpression("x = 1.5", "x & 1"), "오류");
    EXPECT_EQ(tryExpression("x = \"a\"", "x | 1"), "오류");
    EXPECT_EQ(tryExpression("x = 참", "~x"), "오류");
}

TEST(BitwiseTest, ShouldBindTighterThanComparisonAndLooserThanArithmetic)
{
    EXPECT_EQ(tryExpression("x = 6", "x & 1 == 0"), "참");      // (x & 1) == 0
    EXPECT_EQ(tryExpression("", "1 + 2 << 1"), "6");            // (1 + 2) << 1
    EXPECT_EQ(tryExpression("", "6 | 1 & 2"), "6");             // 6 | (1 & 2)
    EXPECT_EQ(tryExpression("", "1 | 6 ^ 3"), "5");             // 1 | (6 ^ 3)
    EXPECT_EQ(tryExpression("", "~0 & 0xFF"), "255");           // (~0) & 0xFF
    EXPECT_EQ(tryExpression("", "1 << 2 < 5"), "참");           // (1 << 2) < 5
}

TEST(BitwiseTest, ShouldWorkInRangeLoopBounds)
{
    EXPECT_EQ(runBoth("기록 = 0\ni가 0부터 1 << 3 미만 {\n    기록 = 기록 + 1\n}\n"), "8");
}

TEST(BitwiseLiteralTest, ShouldEvaluateRadixAndExponentLiterals)
{
    EXPECT_EQ(tryExpression("", "0xFF"), "255");
    EXPECT_EQ(tryExpression("", "0x1f"), "31");
    EXPECT_EQ(tryExpression("", "0b101"), "5");
    EXPECT_EQ(tryExpression("", "1e3 == 1000.0"), "참");
    EXPECT_EQ(tryExpression("", "2.5e-3 == 0.0025"), "참");
    EXPECT_EQ(tryExpression("", "타입(1e3)"), "실수");
}

TEST(BitwiseLiteralTest, ShouldTreatRadixLiteralsAsSixtyFourBitPatterns)
{
    EXPECT_EQ(tryExpression("", "0xFFFFFFFFFFFFFFFF"), "-1");
    EXPECT_EQ(tryExpression("", "0x8000000000000000"), "-9223372036854775808");
    EXPECT_EQ(tryExpression("", "0x7FFFFFFFFFFFFFFF"), "9223372036854775807");
}

TEST(BitwiseLiteralTest, ShouldRejectLiteralsBeyondSixtyFourBits)
{
    EXPECT_FALSE(parseErrors("x = 0x10000000000000000\n").empty());
    EXPECT_FALSE(parseErrors("x = 9223372036854775808\n").empty());  // 10진수는 정수 범위 안이어야 한다
}

TEST(BitwiseTest, ShouldComputeFnvHash)
{
    // FNV-1a 64비트 해시 ("a" → 0xaf63dc4c8601ec8c)
    std::string source =
        "해시 = 0xcbf29ce484222325\n"
        "s = \"a\"\n"
        "i가 0부터 길이(s) 미만 {\n"
        "    해시 = 감싸곱하기(해시 ^ 97, 0x100000001b3)\n"
        "}\n"
        "기록 = 해시 == 0xaf63dc4c8601ec8c\n";
    EXPECT_EQ(runBoth(source), "참");
}

TEST(BitwiseTest, ShouldReportNonIntegerOperandsInSemanticAnalysis)
{
    EXPECT_FALSE(semanticErrors("x = 1.5 & 1\n").empty());
    EXPECT_FALSE(semanticErrors("x = ~\"a\"\n").empty());
    EXPECT_TRUE(semanticErrors("x = 5 & 3 | ~1 << 2\n").empty());
    // 비트 연산 결과는 정수이므로 조건 자리에서 막는다
    EXPECT_FALSE(semanticErrors("만약 (5 & 1) { 출력(1) }\n").empty());
}

TEST(BitwiseFormatterTest, ShouldKeepRadixSpellingAndPrecedence)
{
    EXPECT_EQ(format("x = 0xFF&0b1010|~3^1<<2\n"), "x = 0xFF & 0b1010 | ~3 ^ 1 << 2\n");
    EXPECT_EQ(format("y = 1e10 + 2.5E-3\n"), "y = 1e10 + 2.5E-3\n");
    EXPECT_EQ(format("z = (x | 1) & 2\n"), "z = (x | 1) & 2\n");
}
