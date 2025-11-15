/**
 * @file SemanticSymbolTableTest.cpp
 * @brief SymbolTable 단위 테스트
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include <gtest/gtest.h>
#include "semantic/SymbolTable.h"
#include "types/Type.h"

using namespace kingsejong::semantic;
using namespace kingsejong::types;

// ============================================================================
// Scope 테스트
// ============================================================================

TEST(ScopeTest, ShouldDefineSymbol)
{
    Scope scope;
    Symbol symbol("x", SymbolKind::VARIABLE, Type::getBuiltin("정수"));

    EXPECT_TRUE(scope.define(symbol));
    EXPECT_TRUE(scope.isDefined("x"));
}

TEST(ScopeTest, ShouldNotRedefineSameSymbol)
{
    Scope scope;
    Symbol symbol1("x", SymbolKind::VARIABLE, Type::getBuiltin("정수"));
    Symbol symbol2("x", SymbolKind::VARIABLE, Type::getBuiltin("실수"));

    EXPECT_TRUE(scope.define(symbol1));
    EXPECT_FALSE(scope.define(symbol2));  // 재정의 실패
}

TEST(ScopeTest, ShouldLookupLocalSymbol)
{
    Scope scope;
    Symbol symbol("x", SymbolKind::VARIABLE, Type::getBuiltin("정수"));
    scope.define(symbol);

    Symbol* found = scope.lookupLocal("x");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->name, "x");
    EXPECT_EQ(found->kind, SymbolKind::VARIABLE);
}

TEST(ScopeTest, ShouldReturnNullForUndefinedSymbol)
{
    Scope scope;
    EXPECT_EQ(scope.lookupLocal("undefined"), nullptr);
}

TEST(ScopeTest, ShouldLookupInParentScope)
{
    Scope parent;
    Scope child(&parent);

    Symbol parentSymbol("x", SymbolKind::VARIABLE, Type::getBuiltin("정수"));
    parent.define(parentSymbol);

    // 자식 스코프에서 부모 스코프의 심볼 조회
    Symbol* found = child.lookup("x");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->name, "x");
}

TEST(ScopeTest, ShouldShadowParentSymbol)
{
    Scope parent;
    Scope child(&parent);

    Symbol parentSymbol("x", SymbolKind::VARIABLE, Type::getBuiltin("정수"));
    Symbol childSymbol("x", SymbolKind::VARIABLE, Type::getBuiltin("실수"));

    parent.define(parentSymbol);
    child.define(childSymbol);

    // 자식 스코프에서 조회하면 자식의 심볼이 반환됨 (shadowing)
    Symbol* found = child.lookup("x");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->type, Type::getBuiltin("실수"));
}

// ============================================================================
// SymbolTable 테스트
// ============================================================================

TEST(SemanticSymbolTableTest, ShouldStartWithGlobalScope)
{
    SymbolTable table;
    EXPECT_NE(table.globalScope(), nullptr);
    EXPECT_EQ(table.currentScope(), table.globalScope());
}

TEST(SemanticSymbolTableTest, ShouldDefineGlobalSymbol)
{
    SymbolTable table;
    EXPECT_TRUE(table.define("x", SymbolKind::VARIABLE, Type::getBuiltin("정수")));
    EXPECT_TRUE(table.isDefined("x"));
}

TEST(SemanticSymbolTableTest, ShouldEnterAndExitScope)
{
    SymbolTable table;
    Scope* globalScope = table.currentScope();

    table.enterScope();  // 함수 스코프 진입
    Scope* functionScope = table.currentScope();
    EXPECT_NE(functionScope, globalScope);
    EXPECT_EQ(functionScope->parent(), globalScope);

    table.exitScope();  // 함수 스코프 탈출
    EXPECT_EQ(table.currentScope(), globalScope);
}

TEST(SemanticSymbolTableTest, ShouldDefineSymbolInDifferentScopes)
{
    SymbolTable table;

    // 전역 스코프에 x 정의
    table.define("x", SymbolKind::VARIABLE, Type::getBuiltin("정수"));

    table.enterScope();  // 함수 스코프 진입

    // 함수 스코프에 x 정의 (shadowing)
    EXPECT_TRUE(table.define("x", SymbolKind::VARIABLE, Type::getBuiltin("실수")));

    // 현재 스코프에서 조회하면 함수 스코프의 x
    Symbol* found = table.lookup("x");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->type, Type::getBuiltin("실수"));

    table.exitScope();  // 함수 스코프 탈출

    // 전역 스코프로 돌아왔으므로 전역 스코프의 x
    found = table.lookup("x");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->type, Type::getBuiltin("정수"));
}

TEST(SemanticSymbolTableTest, ShouldDistinguishVariableAndFunction)
{
    SymbolTable table;

    table.define("x", SymbolKind::VARIABLE, Type::getBuiltin("정수"));
    table.define("f", SymbolKind::FUNCTION, Type::getBuiltin("정수"));

    EXPECT_TRUE(table.isVariable("x"));
    EXPECT_FALSE(table.isFunction("x"));

    EXPECT_TRUE(table.isFunction("f"));
    EXPECT_FALSE(table.isVariable("f"));
}

TEST(SemanticSymbolTableTest, ShouldTrackSymbolLocation)
{
    SymbolTable table;
    table.define("x", SymbolKind::VARIABLE, Type::getBuiltin("정수"), true, 10, 5);

    Symbol* found = table.lookup("x");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->line, 10);
    EXPECT_EQ(found->column, 5);
}

TEST(SemanticSymbolTableTest, ShouldClearAllScopes)
{
    SymbolTable table;
    table.define("x", SymbolKind::VARIABLE, Type::getBuiltin("정수"));
    table.enterScope();
    table.define("y", SymbolKind::VARIABLE, Type::getBuiltin("실수"));

    table.clear();

    // 클리어 후 전역 스코프만 존재
    EXPECT_EQ(table.currentScope(), table.globalScope());
    EXPECT_FALSE(table.isDefined("x"));
    EXPECT_FALSE(table.isDefined("y"));
}

TEST(SemanticSymbolTableTest, ShouldHandleNestedScopes)
{
    SymbolTable table;

    // 전역: x
    table.define("x", SymbolKind::VARIABLE, Type::getBuiltin("정수"));

    table.enterScope();  // Level 1
    table.define("y", SymbolKind::VARIABLE, Type::getBuiltin("실수"));

    table.enterScope();  // Level 2
    table.define("z", SymbolKind::VARIABLE, Type::getBuiltin("문자열"));

    // Level 2에서 x, y, z 모두 접근 가능
    EXPECT_TRUE(table.isDefined("x"));
    EXPECT_TRUE(table.isDefined("y"));
    EXPECT_TRUE(table.isDefined("z"));

    table.exitScope();  // Level 1로 복귀

    // Level 1에서 x, y 접근 가능하지만 z는 불가능
    EXPECT_TRUE(table.isDefined("x"));
    EXPECT_TRUE(table.isDefined("y"));
    EXPECT_FALSE(table.isDefined("z"));

    table.exitScope();  // Global로 복귀

    // 전역에서 x만 접근 가능
    EXPECT_TRUE(table.isDefined("x"));
    EXPECT_FALSE(table.isDefined("y"));
    EXPECT_FALSE(table.isDefined("z"));
}

TEST(SemanticSymbolTableTest, ShouldHandleMutableFlag)
{
    SymbolTable table;

    // 변경 가능한 변수
    table.define("x", SymbolKind::VARIABLE, Type::getBuiltin("정수"), true);
    Symbol* x = table.lookup("x");
    ASSERT_NE(x, nullptr);
    EXPECT_TRUE(x->isMutable);

    // 상수 (향후 사용)
    table.define("PI", SymbolKind::VARIABLE, Type::getBuiltin("실수"), false);
    Symbol* pi = table.lookup("PI");
    ASSERT_NE(pi, nullptr);
    EXPECT_FALSE(pi->isMutable);
}
