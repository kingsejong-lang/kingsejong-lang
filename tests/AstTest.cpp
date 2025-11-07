#include <gtest/gtest.h>
#include "ast/Node.h"
#include "ast/Expression.h"
#include "ast/Statement.h"

/**
 * @file AstTest.cpp
 * @brief AST 노드 테스트
 */

using namespace kingsejong::ast;
using namespace kingsejong::lexer;

/**
 * @brief AST 테스트 픽스처
 */
class AstTest : public ::testing::Test
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
};

// ============================================================================
// 기본 리터럴 테스트
// ============================================================================

/**
 * @brief Identifier 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateIdentifier)
{
    // Arrange & Act
    Identifier ident("변수명");

    // Assert
    EXPECT_EQ(ident.type(), NodeType::IDENTIFIER);
    EXPECT_EQ(ident.toString(), "변수명");
    EXPECT_EQ(ident.name(), "변수명");
    EXPECT_EQ(ident.typeName(), "IDENTIFIER");
}

/**
 * @brief IntegerLiteral 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateIntegerLiteral)
{
    // Arrange & Act
    IntegerLiteral intLit(42);

    // Assert
    EXPECT_EQ(intLit.type(), NodeType::INTEGER_LITERAL);
    EXPECT_EQ(intLit.toString(), "42");
    EXPECT_EQ(intLit.value(), 42);
}

/**
 * @brief FloatLiteral 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateFloatLiteral)
{
    // Arrange & Act
    FloatLiteral floatLit(3.14);

    // Assert
    EXPECT_EQ(floatLit.type(), NodeType::FLOAT_LITERAL);
    EXPECT_DOUBLE_EQ(floatLit.value(), 3.14);
    // toString은 부동소수점 표현이므로 정확한 비교는 하지 않음
}

/**
 * @brief StringLiteral 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateStringLiteral)
{
    // Arrange & Act
    StringLiteral strLit("안녕하세요");

    // Assert
    EXPECT_EQ(strLit.type(), NodeType::STRING_LITERAL);
    EXPECT_EQ(strLit.toString(), "\"안녕하세요\"");
    EXPECT_EQ(strLit.value(), "안녕하세요");
}

/**
 * @brief BooleanLiteral 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateBooleanLiteral)
{
    // Arrange & Act
    BooleanLiteral trueLit(true);
    BooleanLiteral falseLit(false);

    // Assert
    EXPECT_EQ(trueLit.type(), NodeType::BOOLEAN_LITERAL);
    EXPECT_EQ(trueLit.toString(), "참");
    EXPECT_TRUE(trueLit.value());

    EXPECT_EQ(falseLit.toString(), "거짓");
    EXPECT_FALSE(falseLit.value());
}

// ============================================================================
// 연산 표현식 테스트
// ============================================================================

/**
 * @brief BinaryExpression 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateBinaryExpression)
{
    // Arrange
    auto left = std::make_unique<IntegerLiteral>(5);
    auto right = std::make_unique<IntegerLiteral>(3);

    // Act
    BinaryExpression binExpr(std::move(left), "+", std::move(right));

    // Assert
    EXPECT_EQ(binExpr.type(), NodeType::BINARY_EXPRESSION);
    EXPECT_EQ(binExpr.toString(), "(5 + 3)");
    EXPECT_EQ(binExpr.op(), "+");
    EXPECT_NE(binExpr.left(), nullptr);
    EXPECT_NE(binExpr.right(), nullptr);
}

/**
 * @brief UnaryExpression 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateUnaryExpression)
{
    // Arrange
    auto operand = std::make_unique<IntegerLiteral>(5);

    // Act
    UnaryExpression unaryExpr("-", std::move(operand));

    // Assert
    EXPECT_EQ(unaryExpr.type(), NodeType::UNARY_EXPRESSION);
    EXPECT_EQ(unaryExpr.toString(), "(-5)");
    EXPECT_EQ(unaryExpr.op(), "-");
}

// ============================================================================
// KingSejong 특화 표현식 테스트
// ============================================================================

/**
 * @brief JosaExpression 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateJosaExpression)
{
    // Arrange
    auto object = std::make_unique<Identifier>("배열");
    auto method = std::make_unique<Identifier>("정렬한다");

    // Act
    JosaExpression josaExpr(
        std::move(object),
        JosaRecognizer::JosaType::EUL_REUL,
        std::move(method)
    );

    // Assert
    EXPECT_EQ(josaExpr.type(), NodeType::JOSA_EXPRESSION);
    EXPECT_EQ(josaExpr.josaType(), JosaRecognizer::JosaType::EUL_REUL);
    EXPECT_NE(josaExpr.object(), nullptr);
    EXPECT_NE(josaExpr.method(), nullptr);

    // toString은 "JosaExpr(배열 EUL_REUL 정렬한다)" 형식
    std::string result = josaExpr.toString();
    EXPECT_NE(result.find("배열"), std::string::npos);
    EXPECT_NE(result.find("정렬한다"), std::string::npos);
}

/**
 * @brief RangeExpression 노드 생성 및 toString 테스트 (부터 까지)
 */
TEST_F(AstTest, ShouldCreateRangeExpressionInclusive)
{
    // Arrange
    auto start = std::make_unique<IntegerLiteral>(1);
    auto end = std::make_unique<IntegerLiteral>(10);

    // Act - "1부터 10까지" → [1, 10]
    RangeExpression rangeExpr(
        std::move(start),
        std::move(end),
        true,  // startInclusive
        true   // endInclusive
    );

    // Assert
    EXPECT_EQ(rangeExpr.type(), NodeType::RANGE_EXPRESSION);
    EXPECT_EQ(rangeExpr.toString(), "Range[1, 10]");
    EXPECT_TRUE(rangeExpr.startInclusive());
    EXPECT_TRUE(rangeExpr.endInclusive());
}

/**
 * @brief RangeExpression 노드 생성 및 toString 테스트 (부터 미만)
 */
TEST_F(AstTest, ShouldCreateRangeExpressionHalfOpen)
{
    // Arrange
    auto start = std::make_unique<IntegerLiteral>(1);
    auto end = std::make_unique<IntegerLiteral>(10);

    // Act - "1부터 10미만" → [1, 10)
    RangeExpression rangeExpr(
        std::move(start),
        std::move(end),
        true,  // startInclusive
        false  // endInclusive
    );

    // Assert
    EXPECT_EQ(rangeExpr.toString(), "Range[1, 10)");
    EXPECT_TRUE(rangeExpr.startInclusive());
    EXPECT_FALSE(rangeExpr.endInclusive());
}

/**
 * @brief RangeExpression 노드 생성 및 toString 테스트 (초과 이하)
 */
TEST_F(AstTest, ShouldCreateRangeExpressionOpen)
{
    // Arrange
    auto start = std::make_unique<IntegerLiteral>(1);
    auto end = std::make_unique<IntegerLiteral>(10);

    // Act - "1초과 10이하" → (1, 10]
    RangeExpression rangeExpr(
        std::move(start),
        std::move(end),
        false,  // startInclusive
        true    // endInclusive
    );

    // Assert
    EXPECT_EQ(rangeExpr.toString(), "Range(1, 10]");
    EXPECT_FALSE(rangeExpr.startInclusive());
    EXPECT_TRUE(rangeExpr.endInclusive());
}

// ============================================================================
// 함수 및 배열 표현식 테스트
// ============================================================================

/**
 * @brief CallExpression 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateCallExpression)
{
    // Arrange
    auto function = std::make_unique<Identifier>("출력");
    std::vector<std::unique_ptr<Expression>> args;
    args.push_back(std::make_unique<StringLiteral>("안녕"));

    // Act
    CallExpression callExpr(std::move(function), std::move(args));

    // Assert
    EXPECT_EQ(callExpr.type(), NodeType::CALL_EXPRESSION);
    EXPECT_EQ(callExpr.toString(), "출력(\"안녕\")");
    EXPECT_EQ(callExpr.arguments().size(), 1);
}

/**
 * @brief ArrayLiteral 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateArrayLiteral)
{
    // Arrange
    std::vector<std::unique_ptr<Expression>> elements;
    elements.push_back(std::make_unique<IntegerLiteral>(1));
    elements.push_back(std::make_unique<IntegerLiteral>(2));
    elements.push_back(std::make_unique<IntegerLiteral>(3));

    // Act
    ArrayLiteral arrayLit(std::move(elements));

    // Assert
    EXPECT_EQ(arrayLit.type(), NodeType::ARRAY_LITERAL);
    EXPECT_EQ(arrayLit.toString(), "[1, 2, 3]");
    EXPECT_EQ(arrayLit.elements().size(), 3);
}

/**
 * @brief IndexExpression 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateIndexExpression)
{
    // Arrange
    auto array = std::make_unique<Identifier>("배열");
    auto index = std::make_unique<IntegerLiteral>(0);

    // Act
    IndexExpression indexExpr(std::move(array), std::move(index));

    // Assert
    EXPECT_EQ(indexExpr.type(), NodeType::INDEX_EXPRESSION);
    EXPECT_EQ(indexExpr.toString(), "배열[0]");
}

// ============================================================================
// 문장 노드 테스트
// ============================================================================

/**
 * @brief ExpressionStatement 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateExpressionStatement)
{
    // Arrange
    auto expr = std::make_unique<IntegerLiteral>(42);

    // Act
    ExpressionStatement exprStmt(std::move(expr));

    // Assert
    EXPECT_EQ(exprStmt.type(), NodeType::EXPRESSION_STATEMENT);
    EXPECT_EQ(exprStmt.toString(), "42;");
}

/**
 * @brief VarDeclaration 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateVarDeclaration)
{
    // Arrange
    auto initializer = std::make_unique<IntegerLiteral>(10);

    // Act
    VarDeclaration varDecl("정수", "x", std::move(initializer));

    // Assert
    EXPECT_EQ(varDecl.type(), NodeType::VAR_DECLARATION);
    EXPECT_EQ(varDecl.toString(), "정수 x = 10;");
    EXPECT_EQ(varDecl.typeName(), "정수");
    EXPECT_EQ(varDecl.varName(), "x");
}

/**
 * @brief ReturnStatement 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateReturnStatement)
{
    // Arrange
    auto returnValue = std::make_unique<IntegerLiteral>(42);

    // Act
    ReturnStatement returnStmt(std::move(returnValue));

    // Assert
    EXPECT_EQ(returnStmt.type(), NodeType::RETURN_STATEMENT);
    EXPECT_EQ(returnStmt.toString(), "반환 42;");
}

/**
 * @brief BlockStatement 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateBlockStatement)
{
    // Arrange
    std::vector<std::unique_ptr<Statement>> statements;

    auto expr = std::make_unique<IntegerLiteral>(1);
    statements.push_back(std::make_unique<ExpressionStatement>(std::move(expr)));

    // Act
    BlockStatement blockStmt(std::move(statements));

    // Assert
    EXPECT_EQ(blockStmt.type(), NodeType::BLOCK_STATEMENT);
    EXPECT_EQ(blockStmt.statements().size(), 1);

    std::string result = blockStmt.toString();
    EXPECT_NE(result.find("{"), std::string::npos);
    EXPECT_NE(result.find("}"), std::string::npos);
}

/**
 * @brief Program 노드 생성 및 toString 테스트
 */
TEST_F(AstTest, ShouldCreateProgram)
{
    // Arrange
    std::vector<std::unique_ptr<Statement>> statements;

    auto expr = std::make_unique<IntegerLiteral>(42);
    statements.push_back(std::make_unique<ExpressionStatement>(std::move(expr)));

    // Act
    Program program(std::move(statements));

    // Assert
    EXPECT_EQ(program.type(), NodeType::PROGRAM);
    EXPECT_EQ(program.statements().size(), 1);

    std::string result = program.toString();
    EXPECT_NE(result.find("Program"), std::string::npos);
}

/**
 * @brief nodeTypeToString 함수 테스트
 */
TEST_F(AstTest, ShouldConvertNodeTypeToString)
{
    // Arrange & Act & Assert
    EXPECT_EQ(nodeTypeToString(NodeType::IDENTIFIER), "IDENTIFIER");
    EXPECT_EQ(nodeTypeToString(NodeType::INTEGER_LITERAL), "INTEGER_LITERAL");
    EXPECT_EQ(nodeTypeToString(NodeType::JOSA_EXPRESSION), "JOSA_EXPRESSION");
    EXPECT_EQ(nodeTypeToString(NodeType::RANGE_EXPRESSION), "RANGE_EXPRESSION");
    EXPECT_EQ(nodeTypeToString(NodeType::PROGRAM), "PROGRAM");
}

/**
 * @brief 복잡한 AST 트리 생성 테스트
 */
TEST_F(AstTest, ShouldCreateComplexAstTree)
{
    // Arrange - "배열을 정렬하고" 표현식 체인
    auto array = std::make_unique<Identifier>("배열");
    auto sortMethod = std::make_unique<Identifier>("정렬하고");

    // Act
    JosaExpression josaExpr(
        std::move(array),
        JosaRecognizer::JosaType::EUL_REUL,
        std::move(sortMethod)
    );

    // Assert
    EXPECT_EQ(josaExpr.type(), NodeType::JOSA_EXPRESSION);
    std::string result = josaExpr.toString();
    EXPECT_NE(result.find("배열"), std::string::npos);
    EXPECT_NE(result.find("정렬하고"), std::string::npos);
}
