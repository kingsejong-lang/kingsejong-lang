#include <gtest/gtest.h>
#include "parser/Parser.h"
#include "lexer/Lexer.h"

/**
 * @file ParserTest.cpp
 * @brief Parser 테스트
 */

using namespace kingsejong::parser;
using namespace kingsejong::lexer;
using namespace kingsejong::ast;

/**
 * @brief Parser 테스트 픽스처
 */
class ParserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 각 테스트 전 실행
    }

    void TearDown() override
    {
        // 각 테스트 후 실행
    }

    // 헬퍼 함수: 파서 에러 체크
    void checkParserErrors(const Parser& parser)
    {
        const auto& errors = parser.errors();
        if (errors.empty())
        {
            return;
        }

        std::string errorMsg = "parser has " + std::to_string(errors.size()) + " errors:\n";
        for (const auto& err : errors)
        {
            errorMsg += "  " + err + "\n";
        }

        FAIL() << errorMsg;
    }
};

// ============================================================================
// 리터럴 파싱 테스트
// ============================================================================

/**
 * @brief 정수 리터럴 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseIntegerLiteral)
{
    // Arrange
    std::string input = "42;";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements()[0].get());
    ASSERT_NE(exprStmt, nullptr);

    auto intLit = dynamic_cast<const IntegerLiteral*>(exprStmt->expression());
    ASSERT_NE(intLit, nullptr);
    EXPECT_EQ(intLit->value(), 42);
}

/**
 * @brief 식별자 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseIdentifier)
{
    // Arrange
    std::string input = "변수명;";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements()[0].get());
    ASSERT_NE(exprStmt, nullptr);

    auto ident = dynamic_cast<const Identifier*>(exprStmt->expression());
    ASSERT_NE(ident, nullptr);
    EXPECT_EQ(ident->name(), "변수명");
}

/**
 * @brief 문자열 리터럴 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseStringLiteral)
{
    // Arrange
    std::string input = "\"안녕하세요\";";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements()[0].get());
    ASSERT_NE(exprStmt, nullptr);

    auto strLit = dynamic_cast<const StringLiteral*>(exprStmt->expression());
    ASSERT_NE(strLit, nullptr);
    EXPECT_EQ(strLit->value(), "안녕하세요");
}

/**
 * @brief 불리언 리터럴 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseBooleanLiteral)
{
    // Arrange
    std::string input1 = "참;";
    std::string input2 = "거짓;";

    Lexer lexer1(input1);
    Parser parser1(lexer1);
    auto program1 = parser1.parseProgram();

    Lexer lexer2(input2);
    Parser parser2(lexer2);
    auto program2 = parser2.parseProgram();

    // Assert
    checkParserErrors(parser1);
    checkParserErrors(parser2);

    auto exprStmt1 = dynamic_cast<ExpressionStatement*>(program1->statements()[0].get());
    auto boolLit1 = dynamic_cast<const BooleanLiteral*>(exprStmt1->expression());
    EXPECT_TRUE(boolLit1->value());

    auto exprStmt2 = dynamic_cast<ExpressionStatement*>(program2->statements()[0].get());
    auto boolLit2 = dynamic_cast<const BooleanLiteral*>(exprStmt2->expression());
    EXPECT_FALSE(boolLit2->value());
}

// ============================================================================
// 연산자 파싱 테스트
// ============================================================================

/**
 * @brief 이항 연산 파싱 테스트 (덧셈)
 */
TEST_F(ParserTest, ShouldParseBinaryExpressionAddition)
{
    // Arrange
    std::string input = "5 + 3;";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements()[0].get());
    auto binExpr = dynamic_cast<const BinaryExpression*>(exprStmt->expression());
    ASSERT_NE(binExpr, nullptr);

    EXPECT_EQ(binExpr->op(), "+");

    auto left = dynamic_cast<const IntegerLiteral*>(binExpr->left());
    ASSERT_NE(left, nullptr);
    EXPECT_EQ(left->value(), 5);

    auto right = dynamic_cast<const IntegerLiteral*>(binExpr->right());
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->value(), 3);
}

/**
 * @brief 이항 연산 파싱 테스트 (곱셈)
 */
TEST_F(ParserTest, ShouldParseBinaryExpressionMultiplication)
{
    // Arrange
    std::string input = "2 * 3;";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements()[0].get());
    auto binExpr = dynamic_cast<const BinaryExpression*>(exprStmt->expression());

    EXPECT_EQ(binExpr->op(), "*");
}

/**
 * @brief 연산자 우선순위 테스트
 */
TEST_F(ParserTest, ShouldRespectOperatorPrecedence)
{
    // Arrange
    std::string input = "1 + 2 * 3;";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);

    // (1 + (2 * 3)) 형태로 파싱되어야 함
    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements()[0].get());
    auto binExpr = dynamic_cast<const BinaryExpression*>(exprStmt->expression());
    ASSERT_NE(binExpr, nullptr);

    EXPECT_EQ(binExpr->op(), "+");

    // 오른쪽은 2 * 3 이어야 함
    auto rightBinExpr = dynamic_cast<const BinaryExpression*>(binExpr->right());
    ASSERT_NE(rightBinExpr, nullptr);
    EXPECT_EQ(rightBinExpr->op(), "*");
}

/**
 * @brief 단항 연산 파싱 테스트
 */
TEST_F(ParserTest, ShouldParsePrefixExpression)
{
    // Arrange
    std::string input = "-5;";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements()[0].get());
    auto prefixExpr = dynamic_cast<const UnaryExpression*>(exprStmt->expression());
    ASSERT_NE(prefixExpr, nullptr);

    EXPECT_EQ(prefixExpr->op(), "-");

    auto operand = dynamic_cast<const IntegerLiteral*>(prefixExpr->operand());
    ASSERT_NE(operand, nullptr);
    EXPECT_EQ(operand->value(), 5);
}

/**
 * @brief 괄호 표현식 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseGroupedExpression)
{
    // Arrange
    std::string input = "(1 + 2) * 3;";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements()[0].get());
    auto binExpr = dynamic_cast<const BinaryExpression*>(exprStmt->expression());
    ASSERT_NE(binExpr, nullptr);

    EXPECT_EQ(binExpr->op(), "*");

    // 왼쪽은 (1 + 2) 이어야 함
    auto leftBinExpr = dynamic_cast<const BinaryExpression*>(binExpr->left());
    ASSERT_NE(leftBinExpr, nullptr);
    EXPECT_EQ(leftBinExpr->op(), "+");
}

// ============================================================================
// 문장 파싱 테스트
// ============================================================================

/**
 * @brief 변수 선언 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseVarDeclaration)
{
    // Arrange
    std::string input = "정수 x = 10;";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto varDecl = dynamic_cast<VarDeclaration*>(program->statements()[0].get());
    ASSERT_NE(varDecl, nullptr);

    EXPECT_EQ(varDecl->typeName(), "정수");
    EXPECT_EQ(varDecl->varName(), "x");

    auto initializer = dynamic_cast<const IntegerLiteral*>(varDecl->initializer());
    ASSERT_NE(initializer, nullptr);
    EXPECT_EQ(initializer->value(), 10);
}

/**
 * @brief 반환 문장 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseReturnStatement)
{
    // Arrange
    std::string input = "반환 42;";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto returnStmt = dynamic_cast<ReturnStatement*>(program->statements()[0].get());
    ASSERT_NE(returnStmt, nullptr);

    auto returnValue = dynamic_cast<const IntegerLiteral*>(returnStmt->returnValue());
    ASSERT_NE(returnValue, nullptr);
    EXPECT_EQ(returnValue->value(), 42);
}

// ============================================================================
// 복합 표현식 테스트
// ============================================================================

/**
 * @brief 함수 호출 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseCallExpression)
{
    // Arrange
    std::string input = "출력(42);";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements()[0].get());
    auto callExpr = dynamic_cast<const CallExpression*>(exprStmt->expression());
    ASSERT_NE(callExpr, nullptr);

    auto function = dynamic_cast<const Identifier*>(callExpr->function());
    ASSERT_NE(function, nullptr);
    EXPECT_EQ(function->name(), "출력");

    ASSERT_EQ(callExpr->arguments().size(), 1);
}

/**
 * @brief 배열 리터럴 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseArrayLiteral)
{
    // Arrange
    std::string input = "[1, 2, 3];";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements()[0].get());
    auto arrayLit = dynamic_cast<const ArrayLiteral*>(exprStmt->expression());
    ASSERT_NE(arrayLit, nullptr);

    EXPECT_EQ(arrayLit->elements().size(), 3);
}

/**
 * @brief 인덱스 접근 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseIndexExpression)
{
    // Arrange
    std::string input = "배열[0];";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements()[0].get());
    auto indexExpr = dynamic_cast<const IndexExpression*>(exprStmt->expression());
    ASSERT_NE(indexExpr, nullptr);

    auto array = dynamic_cast<const Identifier*>(indexExpr->array());
    ASSERT_NE(array, nullptr);
    EXPECT_EQ(array->name(), "배열");

    auto index = dynamic_cast<const IntegerLiteral*>(indexExpr->index());
    ASSERT_NE(index, nullptr);
    EXPECT_EQ(index->value(), 0);
}

/**
 * @brief 복잡한 표현식 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseComplexExpression)
{
    // Arrange
    std::string input = "정수 결과 = (1 + 2) * 3;";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto varDecl = dynamic_cast<VarDeclaration*>(program->statements()[0].get());
    ASSERT_NE(varDecl, nullptr);
    EXPECT_EQ(varDecl->varName(), "결과");

    // 초기화 값은 (1 + 2) * 3
    auto binExpr = dynamic_cast<const BinaryExpression*>(varDecl->initializer());
    ASSERT_NE(binExpr, nullptr);
    EXPECT_EQ(binExpr->op(), "*");
}
