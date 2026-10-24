/**
 * @file AstVisitorTest.cpp
 * @brief ast::walk / ast::Visitor 테스트
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "ast/Visitor.h"
#include <vector>
#include <string>

using namespace kingsejong;

namespace {

std::unique_ptr<ast::Program> parse(const std::string& source)
{
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty()) << (parser.errors().empty() ? "" : parser.errors()[0]);
    return program;
}

// 들어가고 나가는 순서를 기록한다
struct Recorder : ast::Visitor
{
    std::vector<std::string> events;
    bool enter(ast::Node& node) override { events.push_back("+" + node.typeName()); return true; }
    void leave(ast::Node& node) override { events.push_back("-" + node.typeName()); }
};

struct IdentifierCounter : ast::Visitor
{
    int count = 0;
    bool skipFunctions = false;
    bool enter(ast::Node& node) override
    {
        if (node.type() == ast::NodeType::IDENTIFIER) ++count;
        return !(skipFunctions && node.type() == ast::NodeType::FUNCTION_LITERAL);
    }
};

} // namespace

TEST(AstVisitorTest, VisitsChildrenDepthFirstWithEnterAndLeave)
{
    auto program = parse("정수 x = 1 + 2");
    Recorder rec;
    ast::walk(program.get(), rec);
    std::vector<std::string> expected = {
        "+PROGRAM", "+VAR_DECLARATION", "+BINARY_EXPRESSION",
        "+INTEGER_LITERAL", "-INTEGER_LITERAL", "+INTEGER_LITERAL", "-INTEGER_LITERAL",
        "-BINARY_EXPRESSION", "-VAR_DECLARATION", "-PROGRAM"
    };
    EXPECT_EQ(rec.events, expected);
}

TEST(AstVisitorTest, ReachesIdentifiersInsideEveryStatementKind)
{
    auto program = parse(
        "정수 a = 1\n"
        "정수 끝 = 3\n"
        "만약 (a > 0) { 출력(a) } 아니면 { 출력(a) }\n"
        "i가 1부터 끝까지 { 출력(i) }\n"
        "반복 (a < 3) { a = a + 1 }\n"
        "함수 f(p) { 반환 p + a }\n"
        "시도 { 출력(f(a)) } 오류 (e) { 출력(e) }\n"
        "목록 = [a, a]\n");
    IdentifierCounter counter;
    ast::walk(program.get(), counter);
    // 식별자 노드만 센다 (선언·할당의 이름과 매개변수는 노드가 아니다):
    // 만약: a, 출력, a, 출력, a (5) / 범위: 끝, 출력, i (3) / 반복: a, a (2)
    // 함수 본문: p, a (2) / 시도: 출력, f, a, 출력, e (5) / 배열: a, a (2)
    EXPECT_EQ(counter.count, 19);
}

TEST(AstVisitorTest, ReturningFalseFromEnterSkipsTheSubtree)
{
    auto program = parse("함수 f(p) { 반환 p + q }\n출력(r)");
    IdentifierCounter all;
    ast::walk(program.get(), all);
    IdentifierCounter outer;
    outer.skipFunctions = true;
    ast::walk(program.get(), outer);
    EXPECT_EQ(all.count, 4);    // p, q, 출력, r
    EXPECT_EQ(outer.count, 2);  // 출력, r
}

TEST(AstVisitorTest, WalkingNullDoesNothing)
{
    Recorder rec;
    ast::walk(nullptr, rec);
    ast::walkChildren(nullptr, rec);
    EXPECT_TRUE(rec.events.empty());
}

// node_cast: 종류가 맞으면 포인터, 다르거나 nullptr 이면 nullptr
TEST(NodeCastTest, ShouldCastOnlyMatchingNodeType)
{
    auto program = parse("정수 끝 = 1 + 2");
    ASSERT_EQ(program->statements().size(), 1u);
    const ast::Statement* stmt = program->statements()[0].get();

    auto* decl = ast::node_cast<ast::VarDeclaration>(stmt);
    ASSERT_NE(decl, nullptr);
    EXPECT_EQ(decl->varName(), "끝");
    EXPECT_EQ(ast::node_cast<ast::AssignmentStatement>(stmt), nullptr);

    auto* binary = ast::node_cast<ast::BinaryExpression>(decl->initializer());
    ASSERT_NE(binary, nullptr);
    EXPECT_NE(ast::node_cast<ast::IntegerLiteral>(binary->left()), nullptr);
    EXPECT_EQ(ast::node_cast<ast::Identifier>(binary->left()), nullptr);

    const ast::Node* none = nullptr;
    EXPECT_EQ(ast::node_cast<ast::Identifier>(none), nullptr);
}
