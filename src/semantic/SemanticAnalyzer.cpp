/**
 * @file SemanticAnalyzer.cpp
 * @brief 의미 분석기 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "semantic/SemanticAnalyzer.h"
#include <iostream>

namespace kingsejong {
namespace semantic {

using namespace ast;
using namespace types;

// ============================================================================
// 메인 진입점
// ============================================================================

bool SemanticAnalyzer::analyze(Program* program)
{
    if (!program)
    {
        addError("Program is null");
        return false;
    }

    // Phase 1: Symbol Table 구축
    buildSymbolTable(program);

    // Phase 2: 이름 해석
    resolveNames(program);

    // Phase 3: 타입 검사
    checkTypes(program);

    // Phase 4: 모호성 해결
    resolveAmbiguities(program);

    // 에러가 없으면 성공
    return errors_.empty();
}

// ============================================================================
// Phase 1: Symbol Table 구축
// ============================================================================

void SemanticAnalyzer::buildSymbolTable(Program* program)
{
    for (auto& stmt : program->statements())
    {
        registerSymbolFromStatement(stmt.get());
    }
}

void SemanticAnalyzer::registerSymbolFromStatement(Statement* stmt)
{
    if (!stmt) return;

    // 변수 선언
    if (auto varDecl = dynamic_cast<VarDeclaration*>(stmt))
    {
        registerVariable(varDecl);
    }
    // 할당문 (함수 선언도 할당문으로 변환됨)
    else if (auto assignStmt = dynamic_cast<AssignmentStatement*>(stmt))
    {
        // 함수 리터럴 할당인지 확인 (const_cast 필요)
        if (auto funcLit = dynamic_cast<const FunctionLiteral*>(assignStmt->value()))
        {
            // registerFunction은 const가 아닌 포인터를 받으므로 const_cast 사용
            registerFunction(assignStmt->varName(), const_cast<FunctionLiteral*>(funcLit));
        }
        // 일반 할당문은 Symbol Table에 등록하지 않음 (변수 선언이 아니므로)
    }
    // 기타 문장들은 현재 버전에서는 스킵
    // TODO: BlockStatement, IfStatement, WhileStatement 등 처리
}

void SemanticAnalyzer::registerVariable(VarDeclaration* varDecl)
{
    if (!varDecl) return;

    const std::string& varName = varDecl->varName();
    Type* varType = varDecl->varType();

    // 이미 정의된 변수인지 확인 (현재 스코프에서만)
    if (symbolTable_.currentScope()->lookupLocal(varName))
    {
        addError("Variable '" + varName + "' is already defined in this scope");
        return;
    }

    // Symbol Table에 등록
    symbolTable_.define(varName, SymbolKind::VARIABLE, varType);
}

void SemanticAnalyzer::registerFunction(const std::string& name, FunctionLiteral* funcLit)
{
    if (!funcLit) return;

    // 이미 정의된 함수인지 확인
    if (symbolTable_.currentScope()->lookupLocal(name))
    {
        addError("Function '" + name + "' is already defined in this scope");
        return;
    }

    // 함수 타입 (향후 타입 시스템 확장 시 사용)
    // 현재는 반환 타입을 알 수 없으므로 nullptr
    symbolTable_.define(name, SymbolKind::FUNCTION, nullptr);

    // TODO: 함수 스코프 관리 및 매개변수 등록은 향후 구현
    // 현재는 기본 등록만 수행
}

// ============================================================================
// Phase 2: 이름 해석 (Name Resolution)
// ============================================================================

void SemanticAnalyzer::resolveNames(Program* program)
{
    // TODO: 모든 식별자가 정의된 심볼인지 확인
    // 현재는 skeleton만 구현
    for (auto& stmt : program->statements())
    {
        resolveNamesInStatement(stmt.get());
    }
}

void SemanticAnalyzer::resolveNamesInStatement(Statement* stmt)
{
    // TODO: Statement의 모든 식별자 검증
    // 현재는 skeleton
    if (!stmt) return;

    // ExpressionStatement
    if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt))
    {
        resolveNamesInExpression(exprStmt->expression());
    }
    // VarDeclaration
    else if (auto varDecl = dynamic_cast<VarDeclaration*>(stmt))
    {
        if (varDecl->initializer())
        {
            resolveNamesInExpression(varDecl->initializer());
        }
    }
    // 기타 문장들...
}

void SemanticAnalyzer::resolveNamesInExpression(const Expression* expr)
{
    // TODO: Expression의 모든 식별자 검증
    // 현재는 skeleton
    if (!expr) return;

    // Identifier
    if (auto ident = dynamic_cast<const Identifier*>(expr))
    {
        const std::string& name = ident->name();
        if (!symbolTable_.isDefined(name))
        {
            // 미정의 변수 경고 (현재는 에러로 처리하지 않음 - builtin 함수 등이 있을 수 있음)
            // addError("Undefined variable: " + name);
        }
    }
    // BinaryExpression
    else if (auto binExpr = dynamic_cast<const BinaryExpression*>(expr))
    {
        resolveNamesInExpression(binExpr->left());
        resolveNamesInExpression(binExpr->right());
    }
    // 기타 표현식들...
}

// ============================================================================
// Phase 3: 타입 검사 (Type Checking)
// ============================================================================

void SemanticAnalyzer::checkTypes(Program* program)
{
    // TODO: 타입 검사 구현
    // 현재는 skeleton
    for (auto& stmt : program->statements())
    {
        checkTypesInStatement(stmt.get());
    }
}

void SemanticAnalyzer::checkTypesInStatement(Statement* stmt)
{
    // TODO: Statement의 타입 검사
    // 현재는 skeleton
    if (!stmt) return;

    // VarDeclaration
    if (auto varDecl = dynamic_cast<VarDeclaration*>(stmt))
    {
        if (varDecl->initializer())
        {
            Type* initType = inferType(varDecl->initializer());
            Type* varType = varDecl->varType();

            if (initType && varType && !isTypeCompatible(varType, initType))
            {
                addError("Type mismatch: cannot assign " + initType->koreanName() +
                        " to " + varType->koreanName());
            }
        }
    }
}

Type* SemanticAnalyzer::inferType(const Expression* expr)
{
    // TODO: 타입 추론 구현
    // 현재는 기본만 구현
    if (!expr) return nullptr;

    if (dynamic_cast<const IntegerLiteral*>(expr))
    {
        return Type::getBuiltin("정수");
    }
    else if (dynamic_cast<const FloatLiteral*>(expr))
    {
        return Type::getBuiltin("실수");
    }
    else if (dynamic_cast<const StringLiteral*>(expr))
    {
        return Type::getBuiltin("문자열");
    }
    else if (dynamic_cast<const BooleanLiteral*>(expr))
    {
        return Type::getBuiltin("논리");
    }
    else if (auto ident = dynamic_cast<const Identifier*>(expr))
    {
        Symbol* symbol = symbolTable_.lookup(ident->name());
        return symbol ? symbol->type : nullptr;
    }

    return nullptr;
}

bool SemanticAnalyzer::isTypeCompatible(Type* expected, Type* actual)
{
    if (!expected || !actual) return false;

    // 같은 타입이면 호환
    if (expected == actual) return true;

    // 타입 이름이 같으면 호환
    return expected->koreanName() == actual->koreanName();
}

// ============================================================================
// Phase 4: 모호성 해결
// ============================================================================

void SemanticAnalyzer::resolveAmbiguities(Program* program)
{
    // TODO: 모호성 해결 구현
    // 현재는 skeleton
    for (auto& stmt : program->statements())
    {
        resolveAmbiguitiesInStatement(stmt.get());
    }
}

void SemanticAnalyzer::resolveAmbiguitiesInStatement(Statement* /* stmt */)
{
    // TODO: 모호성 해결
    // 예: JosaExpression이 실제로 RangeForStatement여야 하는지 검증
    // 현재는 skeleton
}

// ============================================================================
// 유틸리티
// ============================================================================

void SemanticAnalyzer::addError(const std::string& message, int line, int column)
{
    errors_.emplace_back(message, line, column);
}

} // namespace semantic
} // namespace kingsejong
