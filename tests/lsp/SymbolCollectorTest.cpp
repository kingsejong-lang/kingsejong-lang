/**
 * @file SymbolCollectorTest.cpp
 * @brief SymbolCollector 테스트
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * AST를 순회하여 SymbolTable을 구축하는 기능 테스트
 */

#include <gtest/gtest.h>
#include "lsp/SymbolCollector.h"
#include "lsp/SymbolTable.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"

using namespace kingsejong::lsp;
using namespace kingsejong::lexer;
using namespace kingsejong::parser;
using namespace kingsejong::ast;

class SymbolCollectorTest : public ::testing::Test
{
protected:
    SymbolTable symbolTable;
    SymbolCollector collector;

    void SetUp() override
    {
        collector = SymbolCollector(symbolTable);
    }

    /**
     * @brief 소스 코드를 파싱하여 AST 생성
     */
    std::unique_ptr<Program> parse(const std::string& source)
    {
        Lexer lexer(source);
        Parser parser(lexer);
        return parser.parseProgram();
    }
};

// ============================================================================
// 변수 선언 수집 테스트
// ============================================================================

TEST_F(SymbolCollectorTest, ShouldCollectVariableDeclaration)
{
    // Arrange
    std::string source = "정수 x = 10";
    auto program = parse(source);

    // Act
    collector.collect(program.get(), "test.ksj");

    // Assert
    auto symbol = symbolTable.findSymbol("x");
    ASSERT_TRUE(symbol.has_value());
    EXPECT_EQ(symbol->name, "x");
    EXPECT_EQ(symbol->kind, SymbolKind::Variable);
    EXPECT_EQ(symbol->typeInfo, "정수");
}

TEST_F(SymbolCollectorTest, ShouldCollectMultipleVariableDeclarations)
{
    // Arrange
    std::string source =
        "정수 x = 10\n"
        "실수 y = 3.14\n"
        "문자열 이름 = \"홍길동\"";
    auto program = parse(source);

    // Act
    collector.collect(program.get(), "test.ksj");

    // Assert
    auto x = symbolTable.findSymbol("x");
    ASSERT_TRUE(x.has_value());
    EXPECT_EQ(x->typeInfo, "정수");

    auto y = symbolTable.findSymbol("y");
    ASSERT_TRUE(y.has_value());
    EXPECT_EQ(y->typeInfo, "실수");

    auto name = symbolTable.findSymbol("이름");
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(name->typeInfo, "문자열");
}

// ============================================================================
// 함수 선언 수집 테스트
// ============================================================================

TEST_F(SymbolCollectorTest, ShouldCollectFunctionDeclaration)
{
    // Arrange
    std::string source =
        "함수 더하기(a, b) {\n"
        "    반환 a + b\n"
        "}";
    auto program = parse(source);

    // Act
    collector.collect(program.get(), "test.ksj");

    // Assert
    auto func = symbolTable.findSymbol("더하기");
    ASSERT_TRUE(func.has_value());
    EXPECT_EQ(func->name, "더하기");
    EXPECT_EQ(func->kind, SymbolKind::Function);
}

TEST_F(SymbolCollectorTest, ShouldCollectFunctionParameters)
{
    // Arrange
    std::string source =
        "함수 더하기(a, b) {\n"
        "    반환 a + b\n"
        "}";
    auto program = parse(source);

    // Act
    collector.collect(program.get(), "test.ksj");

    // Assert
    // 매개변수는 함수 스코프에 있어야 함
    auto paramA = symbolTable.findSymbolInScope("a", "함수:더하기");
    ASSERT_TRUE(paramA.has_value());
    EXPECT_EQ(paramA->kind, SymbolKind::Parameter);

    auto paramB = symbolTable.findSymbolInScope("b", "함수:더하기");
    ASSERT_TRUE(paramB.has_value());
    EXPECT_EQ(paramB->kind, SymbolKind::Parameter);
}

// ============================================================================
// 변수 참조 수집 테스트
// ============================================================================

TEST_F(SymbolCollectorTest, ShouldCollectVariableReferences)
{
    // Arrange
    std::string source =
        "정수 x = 10\n"
        "정수 y = x + 5\n"
        "정수 z = x * 2";
    auto program = parse(source);

    // Act
    collector.collect(program.get(), "test.ksj");

    // Assert
    auto refs = symbolTable.getReferences("x");
    // x는 2번 참조됨 (y = x + 5, z = x * 2)
    EXPECT_GE(refs.size(), 2u);
}

TEST_F(SymbolCollectorTest, ShouldCollectFunctionCallReferences)
{
    // Arrange
    std::string source =
        "함수 더하기(a, b) {\n"
        "    반환 a + b\n"
        "}\n"
        "정수 결과 = 더하기(10, 20)";
    auto program = parse(source);

    // Act
    collector.collect(program.get(), "test.ksj");

    // Assert
    auto refs = symbolTable.getReferences("더하기");
    // 더하기는 1번 호출됨
    EXPECT_GE(refs.size(), 1u);
}

// ============================================================================
// 스코프 처리 테스트
// ============================================================================

TEST_F(SymbolCollectorTest, ShouldHandleGlobalAndLocalScope)
{
    // Arrange
    std::string source =
        "정수 x = 10\n"
        "함수 테스트() {\n"
        "    정수 x = 20\n"  // 로컬 변수 x
        "}";
    auto program = parse(source);

    // Act
    collector.collect(program.get(), "test.ksj");

    // Assert
    // 전역 x
    auto globalX = symbolTable.findSymbolInScope("x", "global");
    ASSERT_TRUE(globalX.has_value());

    // 로컬 x (함수 스코프)
    auto localX = symbolTable.findSymbolInScope("x", "함수:테스트");
    ASSERT_TRUE(localX.has_value());

    // 두 x는 다른 위치에 정의됨
    EXPECT_NE(globalX->definitionLocation.line, localX->definitionLocation.line);
}

// ============================================================================
// 복잡한 코드 테스트
// ============================================================================

TEST_F(SymbolCollectorTest, ShouldHandleComplexCode)
{
    // Arrange
    std::string source =
        "정수 전역변수 = 100\n"
        "\n"
        "함수 계산(a, b) {\n"
        "    정수 결과 = a + b\n"
        "    반환 결과\n"
        "}\n"
        "\n"
        "함수 main() {\n"
        "    정수 x = 10\n"
        "    정수 y = 20\n"
        "    정수 합계 = 계산(x, y)\n"
        "}";
    auto program = parse(source);

    // Act
    collector.collect(program.get(), "test.ksj");

    // Assert
    // 전역 변수
    auto 전역변수 = symbolTable.findSymbol("전역변수");
    ASSERT_TRUE(전역변수.has_value());
    EXPECT_EQ(전역변수->scope, "global");

    // 함수들
    auto 계산 = symbolTable.findSymbol("계산");
    ASSERT_TRUE(계산.has_value());
    EXPECT_EQ(계산->kind, SymbolKind::Function);

    auto main = symbolTable.findSymbol("main");
    ASSERT_TRUE(main.has_value());
    EXPECT_EQ(main->kind, SymbolKind::Function);

    // 로컬 변수들
    auto x = symbolTable.findSymbolInScope("x", "함수:main");
    ASSERT_TRUE(x.has_value());

    auto y = symbolTable.findSymbolInScope("y", "함수:main");
    ASSERT_TRUE(y.has_value());

    auto 합계 = symbolTable.findSymbolInScope("합계", "함수:main");
    ASSERT_TRUE(합계.has_value());

    // 함수 호출 참조
    auto 계산Refs = symbolTable.getReferences("계산");
    EXPECT_GE(계산Refs.size(), 1u);
}

// ============================================================================
// 위치 정보 테스트
// ============================================================================

TEST_F(SymbolCollectorTest, ShouldRecordCorrectLineNumbers)
{
    // Arrange
    std::string source =
        "정수 x = 10\n"        // line 1
        "정수 y = 20\n"        // line 2
        "정수 z = x + y";      // line 3
    auto program = parse(source);

    // Act
    collector.collect(program.get(), "test.ksj");

    // Assert
    auto x = symbolTable.findSymbol("x");
    ASSERT_TRUE(x.has_value());
    EXPECT_EQ(x->definitionLocation.line, 1);

    auto y = symbolTable.findSymbol("y");
    ASSERT_TRUE(y.has_value());
    EXPECT_EQ(y->definitionLocation.line, 2);

    auto z = symbolTable.findSymbol("z");
    ASSERT_TRUE(z.has_value());
    EXPECT_EQ(z->definitionLocation.line, 3);
}

// ============================================================================
// 빈 프로그램 테스트
// ============================================================================

TEST_F(SymbolCollectorTest, ShouldHandleEmptyProgram)
{
    // Arrange
    std::string source = "";
    auto program = parse(source);

    // Act & Assert - 예외 없이 처리되어야 함
    EXPECT_NO_THROW(collector.collect(program.get(), "test.ksj"));
    EXPECT_EQ(symbolTable.size(), 0u);
}
