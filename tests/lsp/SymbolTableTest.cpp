/**
 * @file SymbolTableTest.cpp
 * @brief SymbolTable 테스트
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * LSP 고급 기능을 위한 심볼 테이블 테스트
 */

#include <gtest/gtest.h>
#include "lsp/SymbolTable.h"

using namespace kingsejong::lsp;

class SymbolTableTest : public ::testing::Test
{
protected:
    SymbolTable symbolTable;
};

// ============================================================================
// 기본 심볼 추가 및 조회 테스트
// ============================================================================

TEST_F(SymbolTableTest, ShouldAddVariableSymbol)
{
    // Arrange & Act
    Symbol symbol;
    symbol.name = "x";
    symbol.kind = SymbolKind::Variable;
    symbol.definitionLocation = {1, 5, "test.ksj"};
    symbol.typeInfo = "정수";

    symbolTable.addSymbol(symbol);

    // Assert
    auto found = symbolTable.findSymbol("x");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "x");
    EXPECT_EQ(found->kind, SymbolKind::Variable);
    EXPECT_EQ(found->definitionLocation.line, 1);
    EXPECT_EQ(found->definitionLocation.column, 5);
}

TEST_F(SymbolTableTest, ShouldAddFunctionSymbol)
{
    // Arrange & Act
    Symbol symbol;
    symbol.name = "더하기";
    symbol.kind = SymbolKind::Function;
    symbol.definitionLocation = {3, 3, "test.ksj"};
    symbol.typeInfo = "함수";

    symbolTable.addSymbol(symbol);

    // Assert
    auto found = symbolTable.findSymbol("더하기");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "더하기");
    EXPECT_EQ(found->kind, SymbolKind::Function);
}

TEST_F(SymbolTableTest, ShouldReturnNulloptForUnknownSymbol)
{
    // Act
    auto found = symbolTable.findSymbol("unknown");

    // Assert
    EXPECT_FALSE(found.has_value());
}

// ============================================================================
// 참조 추가 및 조회 테스트
// ============================================================================

TEST_F(SymbolTableTest, ShouldAddReferences)
{
    // Arrange
    Symbol symbol;
    symbol.name = "x";
    symbol.kind = SymbolKind::Variable;
    symbol.definitionLocation = {1, 5, "test.ksj"};

    symbolTable.addSymbol(symbol);

    // Act - 참조 추가
    symbolTable.addReference("x", {5, 10, "test.ksj"});
    symbolTable.addReference("x", {8, 15, "test.ksj"});

    // Assert
    auto refs = symbolTable.getReferences("x");
    ASSERT_EQ(refs.size(), 2u);
    EXPECT_EQ(refs[0].line, 5);
    EXPECT_EQ(refs[0].column, 10);
    EXPECT_EQ(refs[1].line, 8);
    EXPECT_EQ(refs[1].column, 15);
}

TEST_F(SymbolTableTest, ShouldReturnEmptyReferencesForUnknownSymbol)
{
    // Act
    auto refs = symbolTable.getReferences("unknown");

    // Assert
    EXPECT_TRUE(refs.empty());
}

// ============================================================================
// 위치 기반 심볼 조회 테스트 (Go to Definition용)
// ============================================================================

TEST_F(SymbolTableTest, ShouldFindSymbolAtLocation)
{
    // Arrange
    Symbol symbol;
    symbol.name = "x";
    symbol.kind = SymbolKind::Variable;
    symbol.definitionLocation = {1, 5, "test.ksj"};

    symbolTable.addSymbol(symbol);
    symbolTable.addReference("x", {5, 10, "test.ksj"});

    // Act - 참조 위치에서 심볼 찾기
    auto found = symbolTable.findSymbolAtLocation({5, 10, "test.ksj"});

    // Assert
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "x");
    EXPECT_EQ(found->definitionLocation.line, 1);
}

TEST_F(SymbolTableTest, ShouldFindSymbolAtDefinitionLocation)
{
    // Arrange
    Symbol symbol;
    symbol.name = "y";
    symbol.kind = SymbolKind::Variable;
    symbol.definitionLocation = {2, 5, "test.ksj"};

    symbolTable.addSymbol(symbol);

    // Act - 정의 위치에서 심볼 찾기
    auto found = symbolTable.findSymbolAtLocation({2, 5, "test.ksj"});

    // Assert
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "y");
}

TEST_F(SymbolTableTest, ShouldReturnNulloptForUnknownLocation)
{
    // Act
    auto found = symbolTable.findSymbolAtLocation({99, 99, "test.ksj"});

    // Assert
    EXPECT_FALSE(found.has_value());
}

// ============================================================================
// 스코프 관리 테스트
// ============================================================================

TEST_F(SymbolTableTest, ShouldHandleGlobalScope)
{
    // Arrange & Act
    Symbol global;
    global.name = "globalVar";
    global.kind = SymbolKind::Variable;
    global.definitionLocation = {1, 1, "test.ksj"};
    global.scope = "global";

    symbolTable.addSymbol(global);

    // Assert
    auto found = symbolTable.findSymbolInScope("globalVar", "global");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->scope, "global");
}

TEST_F(SymbolTableTest, ShouldHandleFunctionScope)
{
    // Arrange
    Symbol localVar;
    localVar.name = "localVar";
    localVar.kind = SymbolKind::Variable;
    localVar.definitionLocation = {5, 5, "test.ksj"};
    localVar.scope = "함수:더하기";

    symbolTable.addSymbol(localVar);

    // Act
    auto found = symbolTable.findSymbolInScope("localVar", "함수:더하기");

    // Assert
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->scope, "함수:더하기");
}

TEST_F(SymbolTableTest, ShouldNotFindSymbolInWrongScope)
{
    // Arrange
    Symbol localVar;
    localVar.name = "x";
    localVar.kind = SymbolKind::Variable;
    localVar.definitionLocation = {5, 5, "test.ksj"};
    localVar.scope = "함수:더하기";

    symbolTable.addSymbol(localVar);

    // Act - 다른 스코프에서 찾기
    auto found = symbolTable.findSymbolInScope("x", "함수:곱하기");

    // Assert
    EXPECT_FALSE(found.has_value());
}

// ============================================================================
// 복잡한 시나리오 테스트
// ============================================================================

TEST_F(SymbolTableTest, ShouldHandleMultipleSymbolsWithSameName)
{
    // Arrange - 같은 이름이지만 다른 스코프
    Symbol global;
    global.name = "x";
    global.kind = SymbolKind::Variable;
    global.definitionLocation = {1, 5, "test.ksj"};
    global.scope = "global";

    Symbol local;
    local.name = "x";
    local.kind = SymbolKind::Variable;
    local.definitionLocation = {10, 5, "test.ksj"};
    local.scope = "함수:test";

    symbolTable.addSymbol(global);
    symbolTable.addSymbol(local);

    // Act
    auto globalX = symbolTable.findSymbolInScope("x", "global");
    auto localX = symbolTable.findSymbolInScope("x", "함수:test");

    // Assert
    ASSERT_TRUE(globalX.has_value());
    ASSERT_TRUE(localX.has_value());
    EXPECT_EQ(globalX->definitionLocation.line, 1);
    EXPECT_EQ(localX->definitionLocation.line, 10);
}

TEST_F(SymbolTableTest, ShouldClearSymbols)
{
    // Arrange
    Symbol symbol;
    symbol.name = "x";
    symbol.kind = SymbolKind::Variable;
    symbol.definitionLocation = {1, 5, "test.ksj"};

    symbolTable.addSymbol(symbol);

    // Act
    symbolTable.clear();

    // Assert
    auto found = symbolTable.findSymbol("x");
    EXPECT_FALSE(found.has_value());
}

// ============================================================================
// 심볼 종류별 필터링 테스트
// ============================================================================

TEST_F(SymbolTableTest, ShouldGetAllVariables)
{
    // Arrange
    Symbol var1;
    var1.name = "x";
    var1.kind = SymbolKind::Variable;
    var1.definitionLocation = {1, 5, "test.ksj"};

    Symbol var2;
    var2.name = "y";
    var2.kind = SymbolKind::Variable;
    var2.definitionLocation = {2, 5, "test.ksj"};

    Symbol func;
    func.name = "더하기";
    func.kind = SymbolKind::Function;
    func.definitionLocation = {5, 3, "test.ksj"};

    symbolTable.addSymbol(var1);
    symbolTable.addSymbol(var2);
    symbolTable.addSymbol(func);

    // Act
    auto variables = symbolTable.getAllSymbolsByKind(SymbolKind::Variable);

    // Assert
    EXPECT_EQ(variables.size(), 2u);
}

TEST_F(SymbolTableTest, ShouldGetAllFunctions)
{
    // Arrange
    Symbol func1;
    func1.name = "더하기";
    func1.kind = SymbolKind::Function;
    func1.definitionLocation = {1, 1, "test.ksj"};

    Symbol func2;
    func2.name = "빼기";
    func2.kind = SymbolKind::Function;
    func2.definitionLocation = {5, 1, "test.ksj"};

    symbolTable.addSymbol(func1);
    symbolTable.addSymbol(func2);

    // Act
    auto functions = symbolTable.getAllSymbolsByKind(SymbolKind::Function);

    // Assert
    EXPECT_EQ(functions.size(), 2u);
}
