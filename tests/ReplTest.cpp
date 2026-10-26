/**
 * @file ReplTest.cpp
 * @brief REPL 과 이름 변환 함수 테스트
 *
 * REPL 은 std::cin / std::cout / std::cerr 를 쓰므로 스트림 버퍼를 바꿔 끼워 대화를 흉내 낸다.
 */

#include <gtest/gtest.h>
#include "repl/Repl.h"
#include "lexer/Token.h"
#include "ast/Node.h"
#include "bytecode/OpCode.h"
#include <iostream>
#include <sstream>

using namespace kingsejong;

namespace {

/// 입력을 넣고 REPL 을 끝까지(EOF) 돌린 뒤 표준 출력과 표준 에러를 돌려준다
struct ReplSession
{
    std::string out;
    std::string err;
};

ReplSession runRepl(const std::string& input)
{
    std::istringstream in(input);
    std::ostringstream out;
    std::ostringstream err;
    auto* oldIn = std::cin.rdbuf(in.rdbuf());
    auto* oldOut = std::cout.rdbuf(out.rdbuf());
    auto* oldErr = std::cerr.rdbuf(err.rdbuf());
    {
        repl::Repl repl;
        repl.start();
    }
    std::cin.rdbuf(oldIn);
    std::cout.rdbuf(oldOut);
    std::cerr.rdbuf(oldErr);
    return {out.str(), err.str()};
}

} // namespace

TEST(ReplTest, ShouldEvaluateInputAndKeepVariablesAcrossLines)
{
    auto session = runRepl("x = 20\n출력(x + 22)\n");
    EXPECT_NE(session.out.find("대화형 실행 환경"), std::string::npos);
    EXPECT_NE(session.out.find("42"), std::string::npos) << session.out;
    EXPECT_NE(session.out.find("안녕히 가세요"), std::string::npos);
}

TEST(ReplTest, ShouldCallFunctionsAndClassesDefinedOnEarlierLines)
{
    // 함수·클래스 값은 정의한 입력의 구문 트리를 가리킨다. 예전에는 입력마다 트리를 해제해서
    // 다음 줄에서 부르면 해제된 메모리를 읽고 죽었다
    auto session = runRepl(
        "두배 = 함수(n) { 반환 n * 2 }\n"
        "클래스 점 {\n    공개 정수 x\n    생성자(x) {\n        자신.x = x\n    }\n}\n"
        "출력(두배(21))\n"
        "출력(점(5).x)\n");
    EXPECT_NE(session.out.find("42"), std::string::npos) << session.out;
    EXPECT_NE(session.out.find("5\n"), std::string::npos) << session.out;
}

TEST(ReplTest, ShouldReadMultiLineInputUntilBracketsClose)
{
    // 괄호가 닫힐 때까지 이어 읽고, 그 사이에는 "... " 프롬프트를 띄운다
    auto session = runRepl("두배 = 함수(n) {\n    반환 n * 2\n}\n출력(두배(21))\n");
    EXPECT_NE(session.out.find("... "), std::string::npos);
    EXPECT_NE(session.out.find("42"), std::string::npos) << session.out;
}

TEST(ReplTest, ShouldIgnoreBracketsInsideStrings)
{
    auto session = runRepl("출력(\"{ 열린 괄호\")\n");
    EXPECT_NE(session.out.find("{ 열린 괄호"), std::string::npos) << session.out;
}

TEST(ReplTest, ShouldHandleCommands)
{
    auto session = runRepl(".help\ny = 7\n.vars\n.clear\n.변수\n");
    EXPECT_NE(session.out.find("REPL 명령어"), std::string::npos);
    EXPECT_NE(session.out.find("y = 7"), std::string::npos) << session.out;
    EXPECT_NE(session.out.find("모든 변수가 초기화되었습니다"), std::string::npos);
    EXPECT_NE(session.out.find("정의된 변수가 없습니다"), std::string::npos);
}

TEST(ReplTest, ShouldReportErrorsAndKeepRunning)
{
    // 구문 오류와 실행 오류를 보고한 뒤에도 다음 입력을 계속 받는다
    auto session = runRepl("출력(1 +)\n출력(없는값)\n출력(1 / 0)\n출력(\"계속\")\n");
    EXPECT_NE(session.out.find("계속"), std::string::npos) << session.out;
    EXPECT_NE(session.err.find("구문 오류"), std::string::npos) << session.err;
    EXPECT_NE(session.err.find("정의되지 않은 변수"), std::string::npos) << session.err;
    EXPECT_NE(session.err.find("0으로 나눌 수 없습니다"), std::string::npos) << session.err;
}

TEST(NameConversionTest, ShouldNameEveryTokenType)
{
    // 새 토큰을 추가하고 이름표를 빠뜨리면 디버그 출력과 오류 메시지에 UNKNOWN 이 찍힌다
    for (int i = static_cast<int>(lexer::TokenType::ILLEGAL); i <= static_cast<int>(lexer::TokenType::DAEGI); ++i)
    {
        auto name = lexer::tokenTypeToString(static_cast<lexer::TokenType>(i));
        EXPECT_NE(name, "UNKNOWN") << "TokenType " << i;
        EXPECT_FALSE(name.empty());
    }
}

TEST(NameConversionTest, ShouldNameEveryNodeType)
{
    for (int i = static_cast<int>(ast::NodeType::PROGRAM); i <= static_cast<int>(ast::NodeType::NEW_EXPRESSION); ++i)
    {
        auto name = ast::nodeTypeToString(static_cast<ast::NodeType>(i));
        EXPECT_FALSE(name.empty()) << "NodeType " << i;
        EXPECT_EQ(name.find("UNKNOWN"), std::string::npos) << "NodeType " << i;
    }
}

TEST(NameConversionTest, ShouldNameEveryOpCode)
{
    for (int i = static_cast<int>(bytecode::OpCode::LOAD_CONST); i <= static_cast<int>(bytecode::OpCode::HALT); ++i)
    {
        auto name = bytecode::opCodeToString(static_cast<bytecode::OpCode>(i));
        EXPECT_FALSE(name.empty()) << "OpCode " << i;
        EXPECT_EQ(name.find("UNKNOWN"), std::string::npos) << "OpCode " << i;
    }
}
