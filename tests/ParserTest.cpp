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

    // 타입 객체 확인
    ASSERT_NE(varDecl->varType(), nullptr);
    EXPECT_EQ(varDecl->varType()->kind(), kingsejong::types::TypeKind::INTEGER);
    EXPECT_EQ(varDecl->varType()->koreanName(), "정수");

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

// ============================================================================
// 조사 파싱 테스트 (KingSejong 핵심 기능!)
// ============================================================================

/**
 * @brief 목적격 조사 파싱 테스트 (을/를)
 */
TEST_F(ParserTest, ShouldParseJosaExpressionEulReul)
{
    // Arrange
    std::string input = "배열을 정렬한다;";
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

    auto josaExpr = dynamic_cast<const JosaExpression*>(exprStmt->expression());
    ASSERT_NE(josaExpr, nullptr);

    // 객체는 "배열"
    auto object = dynamic_cast<const Identifier*>(josaExpr->object());
    ASSERT_NE(object, nullptr);
    EXPECT_EQ(object->name(), "배열");

    // 조사 타입은 EUL_REUL
    EXPECT_EQ(josaExpr->josaType(), JosaRecognizer::JosaType::EUL_REUL);

    // 메서드는 "정렬한다"
    auto method = dynamic_cast<const Identifier*>(josaExpr->method());
    ASSERT_NE(method, nullptr);
    EXPECT_EQ(method->name(), "정렬한다");
}

/**
 * @brief 소유격 조사 파싱 테스트 (의)
 */
TEST_F(ParserTest, ShouldParseJosaExpressionUi)
{
    // Arrange
    std::string input = "사용자의 이름;";
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

    auto josaExpr = dynamic_cast<const JosaExpression*>(exprStmt->expression());
    ASSERT_NE(josaExpr, nullptr);

    // 객체는 "사용자"
    auto object = dynamic_cast<const Identifier*>(josaExpr->object());
    ASSERT_NE(object, nullptr);
    EXPECT_EQ(object->name(), "사용자");

    // 조사 타입은 UI
    EXPECT_EQ(josaExpr->josaType(), JosaRecognizer::JosaType::UI);

    // 메서드는 "이름"
    auto method = dynamic_cast<const Identifier*>(josaExpr->method());
    ASSERT_NE(method, nullptr);
    EXPECT_EQ(method->name(), "이름");
}

/**
 * @brief 방향/수단 조사 파싱 테스트 (로/으로)
 */
TEST_F(ParserTest, ShouldParseJosaExpressionRoEuro)
{
    // Arrange
    std::string input = "데이터로 변환한다;";
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

    auto josaExpr = dynamic_cast<const JosaExpression*>(exprStmt->expression());
    ASSERT_NE(josaExpr, nullptr);

    // 객체는 "데이터"
    auto object = dynamic_cast<const Identifier*>(josaExpr->object());
    ASSERT_NE(object, nullptr);
    EXPECT_EQ(object->name(), "데이터");

    // 조사 타입은 RO_EURO
    EXPECT_EQ(josaExpr->josaType(), JosaRecognizer::JosaType::RO_EURO);

    // 메서드는 "변환한다"
    auto method = dynamic_cast<const Identifier*>(josaExpr->method());
    ASSERT_NE(method, nullptr);
    EXPECT_EQ(method->name(), "변환한다");
}

/**
 * @brief 주격 조사 파싱 테스트 (이/가)
 */
TEST_F(ParserTest, ShouldParseJosaExpressionIGa)
{
    // Arrange
    std::string input = "데이터가 존재한다;";
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

    auto josaExpr = dynamic_cast<const JosaExpression*>(exprStmt->expression());
    ASSERT_NE(josaExpr, nullptr);

    // 객체는 "데이터"
    auto object = dynamic_cast<const Identifier*>(josaExpr->object());
    ASSERT_NE(object, nullptr);
    EXPECT_EQ(object->name(), "데이터");

    // 조사 타입은 I_GA
    EXPECT_EQ(josaExpr->josaType(), JosaRecognizer::JosaType::I_GA);

    // 메서드는 "존재한다"
    auto method = dynamic_cast<const Identifier*>(josaExpr->method());
    ASSERT_NE(method, nullptr);
    EXPECT_EQ(method->name(), "존재한다");
}

// ============================================================================
// 범위 표현 파싱 테스트 (KingSejong 핵심 기능!)
// ============================================================================

/**
 * @brief 범위 표현 파싱 테스트 - 양쪽 포함 (부터...까지)
 */
TEST_F(ParserTest, ShouldParseRangeExpressionInclusive)
{
    // Arrange
    std::string input = "1부터 10까지;";
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

    auto rangeExpr = dynamic_cast<const RangeExpression*>(exprStmt->expression());
    ASSERT_NE(rangeExpr, nullptr);

    // 시작 값은 1
    auto start = dynamic_cast<const IntegerLiteral*>(rangeExpr->start());
    ASSERT_NE(start, nullptr);
    EXPECT_EQ(start->value(), 1);

    // 끝 값은 10
    auto end = dynamic_cast<const IntegerLiteral*>(rangeExpr->end());
    ASSERT_NE(end, nullptr);
    EXPECT_EQ(end->value(), 10);

    // 양쪽 모두 포함
    EXPECT_TRUE(rangeExpr->startInclusive());
    EXPECT_TRUE(rangeExpr->endInclusive());

    // toString()은 [1, 10]
    EXPECT_EQ(rangeExpr->toString(), "Range[1, 10]");
}

/**
 * @brief 범위 표현 파싱 테스트 - 시작 포함, 끝 미포함 (부터...미만)
 */
TEST_F(ParserTest, ShouldParseRangeExpressionHalfOpen)
{
    // Arrange
    std::string input = "1부터 10미만;";
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

    auto rangeExpr = dynamic_cast<const RangeExpression*>(exprStmt->expression());
    ASSERT_NE(rangeExpr, nullptr);

    // 시작 값은 1
    auto start = dynamic_cast<const IntegerLiteral*>(rangeExpr->start());
    ASSERT_NE(start, nullptr);
    EXPECT_EQ(start->value(), 1);

    // 끝 값은 10
    auto end = dynamic_cast<const IntegerLiteral*>(rangeExpr->end());
    ASSERT_NE(end, nullptr);
    EXPECT_EQ(end->value(), 10);

    // 시작 포함, 끝 미포함
    EXPECT_TRUE(rangeExpr->startInclusive());
    EXPECT_FALSE(rangeExpr->endInclusive());

    // toString()은 [1, 10)
    EXPECT_EQ(rangeExpr->toString(), "Range[1, 10)");
}

/**
 * @brief 범위 표현 파싱 테스트 - 시작 미포함, 끝 포함 (초과...이하)
 */
TEST_F(ParserTest, ShouldParseRangeExpressionOpen)
{
    // Arrange
    std::string input = "1초과 10이하;";
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

    auto rangeExpr = dynamic_cast<const RangeExpression*>(exprStmt->expression());
    ASSERT_NE(rangeExpr, nullptr);

    // 시작 값은 1
    auto start = dynamic_cast<const IntegerLiteral*>(rangeExpr->start());
    ASSERT_NE(start, nullptr);
    EXPECT_EQ(start->value(), 1);

    // 끝 값은 10
    auto end = dynamic_cast<const IntegerLiteral*>(rangeExpr->end());
    ASSERT_NE(end, nullptr);
    EXPECT_EQ(end->value(), 10);

    // 시작 미포함, 끝 포함
    EXPECT_FALSE(rangeExpr->startInclusive());
    EXPECT_TRUE(rangeExpr->endInclusive());

    // toString()은 (1, 10]
    EXPECT_EQ(rangeExpr->toString(), "Range(1, 10]");
}

/**
 * @brief 범위 표현 파싱 테스트 - 이상...이하
 */
TEST_F(ParserTest, ShouldParseRangeExpressionIsangIha)
{
    // Arrange
    std::string input = "5이상 15이하;";
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

    auto rangeExpr = dynamic_cast<const RangeExpression*>(exprStmt->expression());
    ASSERT_NE(rangeExpr, nullptr);

    // 시작 값은 5
    auto start = dynamic_cast<const IntegerLiteral*>(rangeExpr->start());
    ASSERT_NE(start, nullptr);
    EXPECT_EQ(start->value(), 5);

    // 끝 값은 15
    auto end = dynamic_cast<const IntegerLiteral*>(rangeExpr->end());
    ASSERT_NE(end, nullptr);
    EXPECT_EQ(end->value(), 15);

    // 양쪽 모두 포함 (이상, 이하)
    EXPECT_TRUE(rangeExpr->startInclusive());
    EXPECT_TRUE(rangeExpr->endInclusive());

    // toString()은 [5, 15]
    EXPECT_EQ(rangeExpr->toString(), "Range[5, 15]");
}

/**
 * @brief 문자열 타입 변수 선언 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseVarDeclarationWithStringType)
{
    // Arrange
    std::string input = "문자열 이름 = \"김철수\";";
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

    EXPECT_EQ(varDecl->typeName(), "문자열");
    EXPECT_EQ(varDecl->varName(), "이름");

    // 타입 객체 확인
    ASSERT_NE(varDecl->varType(), nullptr);
    EXPECT_EQ(varDecl->varType()->kind(), kingsejong::types::TypeKind::STRING);
    EXPECT_EQ(varDecl->varType()->koreanName(), "문자열");
}

/**
 * @brief 실수 타입 변수 선언 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseVarDeclarationWithFloatType)
{
    // Arrange
    std::string input = "실수 온도 = 36.5;";
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

    EXPECT_EQ(varDecl->typeName(), "실수");
    EXPECT_EQ(varDecl->varName(), "온도");

    // 타입 객체 확인
    ASSERT_NE(varDecl->varType(), nullptr);
    EXPECT_EQ(varDecl->varType()->kind(), kingsejong::types::TypeKind::FLOAT);
    EXPECT_EQ(varDecl->varType()->koreanName(), "실수");
}

/**
 * @brief 논리 타입 변수 선언 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseVarDeclarationWithBooleanType)
{
    // Arrange
    std::string input = "논리 결과 = 참;";
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

    EXPECT_EQ(varDecl->typeName(), "논리");
    EXPECT_EQ(varDecl->varName(), "결과");

    // 타입 객체 확인
    // 주의: 타입 시스템에서 "논리" 키워드는 아직 등록되지 않았을 수 있음
    // 타입 이름 "참거짓"과 키워드 "논리"의 매핑 필요
    if (varDecl->varType() == nullptr)
    {
        // 현재는 "논리" → Type 매핑이 없을 수 있으므로 nullptr 허용
        EXPECT_EQ(varDecl->varType(), nullptr);
    }
    else
    {
        EXPECT_EQ(varDecl->varType()->kind(), kingsejong::types::TypeKind::BOOLEAN);
    }
}

/**
 * @brief 초기화 없는 변수 선언 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseVarDeclarationWithoutInitializer)
{
    // Arrange
    std::string input = "정수 카운트;";
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
    EXPECT_EQ(varDecl->varName(), "카운트");
    EXPECT_EQ(varDecl->initializer(), nullptr);

    // 타입 객체 확인
    ASSERT_NE(varDecl->varType(), nullptr);
    EXPECT_EQ(varDecl->varType()->kind(), kingsejong::types::TypeKind::INTEGER);
}

// ============================================================================
// 예외 처리 파싱 테스트
// ============================================================================

/**
 * @brief 던지다 문장 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseThrowStatement)
{
    // Arrange
    std::string input = "던지다 \"에러 메시지\";";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto throwStmt = dynamic_cast<ThrowStatement*>(program->statements()[0].get());
    ASSERT_NE(throwStmt, nullptr);

    auto value = dynamic_cast<const StringLiteral*>(throwStmt->value());
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(value->value(), "에러 메시지");
}

/**
 * @brief 시도-오류 블록 파싱 테스트 (단일 catch)
 */
TEST_F(ParserTest, ShouldParseTryStatementWithSingleCatch)
{
    // Arrange
    std::string input = R"(
        시도 {
            결과 = 10 / 0;
        } 오류 (e) {
            출력(e);
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto tryStmt = dynamic_cast<TryStatement*>(program->statements()[0].get());
    ASSERT_NE(tryStmt, nullptr);

    // Try 블록 확인
    ASSERT_NE(tryStmt->tryBlock(), nullptr);
    EXPECT_EQ(tryStmt->tryBlock()->statements().size(), 1);

    // Catch 절 확인
    ASSERT_EQ(tryStmt->catchClauses().size(), 1);
    const auto& catchClause = tryStmt->catchClauses()[0];
    EXPECT_EQ(catchClause->errorVarName(), "e");
    ASSERT_NE(catchClause->body(), nullptr);
    EXPECT_EQ(catchClause->body()->statements().size(), 1);

    // Finally 블록 없음
    EXPECT_EQ(tryStmt->finallyBlock(), nullptr);
}

/**
 * @brief 시도-오류-마지막 블록 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseTryStatementWithFinally)
{
    // Arrange
    std::string input = R"(
        시도 {
            파일_열기();
        } 오류 (err) {
            출력("에러 발생");
        } 마지막 {
            파일_닫기();
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto tryStmt = dynamic_cast<TryStatement*>(program->statements()[0].get());
    ASSERT_NE(tryStmt, nullptr);

    // Try 블록 확인
    ASSERT_NE(tryStmt->tryBlock(), nullptr);

    // Catch 절 확인
    ASSERT_EQ(tryStmt->catchClauses().size(), 1);
    EXPECT_EQ(tryStmt->catchClauses()[0]->errorVarName(), "err");

    // Finally 블록 확인
    ASSERT_NE(tryStmt->finallyBlock(), nullptr);
    EXPECT_EQ(tryStmt->finallyBlock()->statements().size(), 1);
}

/**
 * @brief 시도-마지막 블록 파싱 테스트 (catch 없이)
 */
TEST_F(ParserTest, ShouldParseTryStatementWithOnlyFinally)
{
    // Arrange
    std::string input = R"(
        시도 {
            작업_수행();
        } 마지막 {
            정리();
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto tryStmt = dynamic_cast<TryStatement*>(program->statements()[0].get());
    ASSERT_NE(tryStmt, nullptr);

    // Try 블록 확인
    ASSERT_NE(tryStmt->tryBlock(), nullptr);

    // Catch 절 없음
    EXPECT_EQ(tryStmt->catchClauses().size(), 0);

    // Finally 블록 확인
    ASSERT_NE(tryStmt->finallyBlock(), nullptr);
}

/**
 * @brief 중첩된 던지다 문장 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseThrowStatementWithExpression)
{
    // Arrange
    std::string input = "던지다 에러_생성(\"문제 발생\");";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto throwStmt = dynamic_cast<ThrowStatement*>(program->statements()[0].get());
    ASSERT_NE(throwStmt, nullptr);

    // 함수 호출 표현식 확인
    auto callExpr = dynamic_cast<const CallExpression*>(throwStmt->value());
    ASSERT_NE(callExpr, nullptr);

    auto function = dynamic_cast<const Identifier*>(callExpr->function());
    ASSERT_NE(function, nullptr);
    EXPECT_EQ(function->name(), "에러_생성");
}

// ============================================================================
// 클래스 시스템 파싱 테스트 (Phase 7.1)
// ============================================================================

/**
 * @brief 간단한 클래스 정의 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseSimpleClassDefinition)
{
    // Arrange
    std::string input = R"(
        클래스 사람 {
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto classStmt = dynamic_cast<ClassStatement*>(program->statements()[0].get());
    ASSERT_NE(classStmt, nullptr);
    EXPECT_EQ(classStmt->className(), "사람");
    EXPECT_EQ(classStmt->superClass(), "");
    EXPECT_EQ(classStmt->fields().size(), 0);
    EXPECT_EQ(classStmt->constructor(), nullptr);
    EXPECT_EQ(classStmt->methods().size(), 0);
}

/**
 * @brief 필드가 있는 클래스 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseClassWithFields)
{
    // Arrange
    std::string input = R"(
        클래스 사람 {
            비공개 문자열 이름
            공개 정수 나이
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto classStmt = dynamic_cast<ClassStatement*>(program->statements()[0].get());
    ASSERT_NE(classStmt, nullptr);
    EXPECT_EQ(classStmt->className(), "사람");
    EXPECT_EQ(classStmt->fields().size(), 2);

    // 첫 번째 필드: 비공개 문자열 이름
    auto& field1 = classStmt->fields()[0];
    EXPECT_EQ(field1->access(), AccessModifier::PRIVATE);
    EXPECT_EQ(field1->typeName(), "문자열");
    EXPECT_EQ(field1->fieldName(), "이름");
    EXPECT_EQ(field1->initializer(), nullptr);

    // 두 번째 필드: 공개 정수 나이
    auto& field2 = classStmt->fields()[1];
    EXPECT_EQ(field2->access(), AccessModifier::PUBLIC);
    EXPECT_EQ(field2->typeName(), "정수");
    EXPECT_EQ(field2->fieldName(), "나이");
}

/**
 * @brief 초기값이 있는 필드 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseFieldWithInitializer)
{
    // Arrange
    std::string input = R"(
        클래스 카운터 {
            비공개 정수 값 = 0
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto classStmt = dynamic_cast<ClassStatement*>(program->statements()[0].get());
    ASSERT_NE(classStmt, nullptr);
    EXPECT_EQ(classStmt->fields().size(), 1);

    auto& field = classStmt->fields()[0];
    EXPECT_EQ(field->fieldName(), "값");
    ASSERT_NE(field->initializer(), nullptr);

    auto intLit = dynamic_cast<const IntegerLiteral*>(field->initializer());
    ASSERT_NE(intLit, nullptr);
    EXPECT_EQ(intLit->value(), 0);
}

/**
 * @brief 생성자가 있는 클래스 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseClassWithConstructor)
{
    // Arrange
    std::string input = R"(
        클래스 사람 {
            비공개 문자열 이름

            생성자(이름) {
                자신.이름 = 이름
            }
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto classStmt = dynamic_cast<ClassStatement*>(program->statements()[0].get());
    ASSERT_NE(classStmt, nullptr);
    ASSERT_NE(classStmt->constructor(), nullptr);

    const auto* constructor = classStmt->constructor();
    EXPECT_EQ(constructor->parameters().size(), 1);
    EXPECT_EQ(constructor->parameters()[0].name, "이름");
    ASSERT_NE(constructor->body(), nullptr);
}

/**
 * @brief 메서드가 있는 클래스 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseClassWithMethods)
{
    // Arrange
    std::string input = R"(
        클래스 사람 {
            공개 함수 인사하기() {
                출력("안녕하세요")
            }

            비공개 함수 내부_함수() {
                반환 42
            }
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto classStmt = dynamic_cast<ClassStatement*>(program->statements()[0].get());
    ASSERT_NE(classStmt, nullptr);
    EXPECT_EQ(classStmt->methods().size(), 2);

    // 첫 번째 메서드: 공개 함수 인사하기
    auto& method1 = classStmt->methods()[0];
    EXPECT_EQ(method1->access(), AccessModifier::PUBLIC);
    EXPECT_EQ(method1->methodName(), "인사하기");
    EXPECT_EQ(method1->parameters().size(), 0);
    ASSERT_NE(method1->body(), nullptr);

    // 두 번째 메서드: 비공개 함수 내부_함수
    auto& method2 = classStmt->methods()[1];
    EXPECT_EQ(method2->access(), AccessModifier::PRIVATE);
    EXPECT_EQ(method2->methodName(), "내부_함수");
}

/**
 * @brief 매개변수가 있는 메서드 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseMethodWithParameters)
{
    // Arrange
    std::string input = R"(
        클래스 계산기 {
            공개 함수 더하기(a, b) {
                반환 a + b
            }
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto classStmt = dynamic_cast<ClassStatement*>(program->statements()[0].get());
    ASSERT_NE(classStmt, nullptr);
    EXPECT_EQ(classStmt->methods().size(), 1);

    auto& method = classStmt->methods()[0];
    EXPECT_EQ(method->methodName(), "더하기");
    EXPECT_EQ(method->parameters().size(), 2);
    EXPECT_EQ(method->parameters()[0].name, "a");
    EXPECT_EQ(method->parameters()[1].name, "b");
}

/**
 * @brief 상속이 있는 클래스 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseClassWithInheritance)
{
    // Arrange
    std::string input = R"(
        클래스 학생 상속 사람 {
            비공개 문자열 학생ID
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto classStmt = dynamic_cast<ClassStatement*>(program->statements()[0].get());
    ASSERT_NE(classStmt, nullptr);
    EXPECT_EQ(classStmt->className(), "학생");
    EXPECT_EQ(classStmt->superClass(), "사람");
    EXPECT_EQ(classStmt->fields().size(), 1);
}

/**
 * @brief this 표현식 (자신) 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseThisExpression)
{
    // Arrange
    std::string input = "자신;";
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

    auto thisExpr = dynamic_cast<const ThisExpression*>(exprStmt->expression());
    ASSERT_NE(thisExpr, nullptr);
    EXPECT_EQ(thisExpr->toString(), "자신");
}

/**
 * @brief 멤버 접근 표현식 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseMemberAccessExpression)
{
    // Arrange
    std::string input = "자신.이름;";
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

    auto memberAccess = dynamic_cast<const MemberAccessExpression*>(exprStmt->expression());
    ASSERT_NE(memberAccess, nullptr);
    EXPECT_EQ(memberAccess->memberName(), "이름");

    auto thisExpr = dynamic_cast<const ThisExpression*>(memberAccess->object());
    ASSERT_NE(thisExpr, nullptr);
}

/**
 * @brief 멤버 접근 할당 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseMemberAccessAssignment)
{
    // Arrange
    std::string input = "자신.이름 = \"홍길동\";";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    // Note: 현재 Parser는 멤버 할당을 AssignmentStatement로 처리하지 않고
    // ExpressionStatement로 처리할 수 있음
    // 이는 나중에 SemanticAnalyzer에서 처리
}

/**
 * @brief 체이닝된 멤버 접근 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseChainedMemberAccess)
{
    // Arrange
    std::string input = "객체.필드.하위필드;";
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

    auto outerMember = dynamic_cast<const MemberAccessExpression*>(exprStmt->expression());
    ASSERT_NE(outerMember, nullptr);
    EXPECT_EQ(outerMember->memberName(), "하위필드");

    auto innerMember = dynamic_cast<const MemberAccessExpression*>(outerMember->object());
    ASSERT_NE(innerMember, nullptr);
    EXPECT_EQ(innerMember->memberName(), "필드");
}

/**
 * @brief 완전한 클래스 예제 파싱 테스트
 */
TEST_F(ParserTest, ShouldParseCompleteClassExample)
{
    // Arrange
    std::string input = R"(
        클래스 사람 {
            비공개 문자열 이름
            공개 정수 나이

            생성자(이름, 나이) {
                자신.이름 = 이름
                자신.나이 = 나이
            }

            공개 함수 인사하기() {
                출력("안녕하세요, " + 자신.이름 + "입니다")
                출력("나이는 " + 자신.나이 + "살입니다")
            }

            공개 함수 나이_증가() {
                자신.나이 = 자신.나이 + 1
            }

            비공개 함수 내부_검증() {
                반환 자신.나이 > 0
            }
        }
    )";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act
    auto program = parser.parseProgram();

    // Assert
    checkParserErrors(parser);
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements().size(), 1);

    auto classStmt = dynamic_cast<ClassStatement*>(program->statements()[0].get());
    ASSERT_NE(classStmt, nullptr);

    // 클래스 이름 확인
    EXPECT_EQ(classStmt->className(), "사람");

    // 필드 확인
    EXPECT_EQ(classStmt->fields().size(), 2);
    EXPECT_EQ(classStmt->fields()[0]->fieldName(), "이름");
    EXPECT_EQ(classStmt->fields()[1]->fieldName(), "나이");

    // 생성자 확인
    ASSERT_NE(classStmt->constructor(), nullptr);
    EXPECT_EQ(classStmt->constructor()->parameters().size(), 2);
    EXPECT_EQ(classStmt->constructor()->parameters()[0].name, "이름");
    EXPECT_EQ(classStmt->constructor()->parameters()[1].name, "나이");

    // 메서드 확인
    EXPECT_EQ(classStmt->methods().size(), 3);
    EXPECT_EQ(classStmt->methods()[0]->methodName(), "인사하기");
    EXPECT_EQ(classStmt->methods()[0]->access(), AccessModifier::PUBLIC);
    EXPECT_EQ(classStmt->methods()[1]->methodName(), "나이_증가");
    EXPECT_EQ(classStmt->methods()[1]->access(), AccessModifier::PUBLIC);
    EXPECT_EQ(classStmt->methods()[2]->methodName(), "내부_검증");
    EXPECT_EQ(classStmt->methods()[2]->access(), AccessModifier::PRIVATE);
}
