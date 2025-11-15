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

void SemanticAnalyzer::registerSymbolFromStatement(const Statement* stmt)
{
    if (!stmt) return;

    // 변수 선언
    if (auto varDecl = dynamic_cast<const VarDeclaration*>(stmt))
    {
        registerVariable(const_cast<VarDeclaration*>(varDecl));
    }
    // 할당문 (함수 선언도 할당문으로 변환됨)
    else if (auto assignStmt = dynamic_cast<const AssignmentStatement*>(stmt))
    {
        // 함수 리터럴 할당인지 확인
        if (auto funcLit = dynamic_cast<const FunctionLiteral*>(assignStmt->value()))
        {
            registerFunction(assignStmt->varName(), const_cast<FunctionLiteral*>(funcLit));
        }
    }
    // BlockStatement: 블록 내 모든 문장 등록
    else if (auto blockStmt = dynamic_cast<const BlockStatement*>(stmt))
    {
        for (const auto& s : blockStmt->statements())
        {
            registerSymbolFromStatement(s.get());
        }
    }
    // IfStatement: then과 else 블록 등록
    else if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt))
    {
        if (ifStmt->thenBranch())
        {
            registerSymbolFromStatement(ifStmt->thenBranch());
        }
        if (ifStmt->elseBranch())
        {
            registerSymbolFromStatement(ifStmt->elseBranch());
        }
    }
    // WhileStatement: body 등록
    else if (auto whileStmt = dynamic_cast<const WhileStatement*>(stmt))
    {
        if (whileStmt->body())
        {
            registerSymbolFromStatement(whileStmt->body());
        }
    }
}

void SemanticAnalyzer::registerVariable(VarDeclaration* varDecl)
{
    if (!varDecl) return;

    const std::string& varName = varDecl->varName();
    Type* varType = varDecl->varType();

    // 타입이 명시되지 않은 경우 초기화 값에서 타입 추론
    if (!varType && varDecl->initializer())
    {
        varType = inferType(varDecl->initializer());
    }

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
    if (auto exprStmt = dynamic_cast<const ExpressionStatement*>(stmt))
    {
        resolveNamesInExpression(exprStmt->expression());
    }
    // VarDeclaration
    else if (auto varDecl = dynamic_cast<const VarDeclaration*>(stmt))
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

void SemanticAnalyzer::checkTypesInStatement(const Statement* stmt)
{
    if (!stmt) return;

    // VarDeclaration: 변수 선언 시 초기화 값의 타입 검사
    if (auto varDecl = dynamic_cast<const VarDeclaration*>(stmt))
    {
        if (varDecl->initializer())
        {
            Type* initType = inferType(varDecl->initializer());
            Type* varType = varDecl->varType();

            if (initType && varType && !isTypeCompatible(varType, initType))
            {
                addError("타입 불일치: " + varType->koreanName() + " 변수에 " +
                        initType->koreanName() + " 값을 할당할 수 없습니다");
            }
        }
    }

    // AssignmentStatement: 할당문의 타입 검사
    else if (auto assignStmt = dynamic_cast<const AssignmentStatement*>(stmt))
    {
        const std::string& varName = assignStmt->varName();
        Symbol* symbol = symbolTable_.lookup(varName);

        if (!symbol)
        {
            addError("정의되지 않은 변수: " + varName);
            return;
        }

        Type* valueType = inferType(assignStmt->value());
        Type* varType = symbol->type;

        if (valueType && varType && !isTypeCompatible(varType, valueType))
        {
            addError("타입 불일치: " + varType->koreanName() + " 변수 '" + varName +
                    "'에 " + valueType->koreanName() + " 값을 할당할 수 없습니다");
        }
    }

    // ExpressionStatement: 표현식 문장의 타입 검사
    else if (auto exprStmt = dynamic_cast<const ExpressionStatement*>(stmt))
    {
        // 표현식 자체의 타입을 검사 (부작용 확인용)
        inferType(exprStmt->expression());
    }

    // ReturnStatement: 반환문의 타입 검사
    else if (auto retStmt = dynamic_cast<const ReturnStatement*>(stmt))
    {
        if (retStmt->returnValue())
        {
            // TODO: 현재 함수의 반환 타입과 비교
            // 함수 컨텍스트 추적 필요
            inferType(retStmt->returnValue());
        }
    }

    // IfStatement: if문의 조건 타입 검사
    else if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt))
    {
        Type* condType = inferType(ifStmt->condition());

        if (condType && condType->koreanName() != "논리")
        {
            addError("if문의 조건은 논리 타입이어야 합니다 (현재: " +
                    condType->koreanName() + ")");
        }

        // then과 else 블록의 타입 검사
        if (ifStmt->thenBranch())
        {
            checkTypesInStatement(ifStmt->thenBranch());
        }
        if (ifStmt->elseBranch())
        {
            checkTypesInStatement(ifStmt->elseBranch());
        }
    }

    // BlockStatement: 블록 내 모든 문장 검사
    else if (auto blockStmt = dynamic_cast<const BlockStatement*>(stmt))
    {
        for (const auto& s : blockStmt->statements())
        {
            checkTypesInStatement(s.get());
        }
    }

    // WhileStatement: while문의 조건 타입 검사
    else if (auto whileStmt = dynamic_cast<const WhileStatement*>(stmt))
    {
        Type* condType = inferType(whileStmt->condition());

        if (condType && condType->koreanName() != "논리")
        {
            addError("while문의 조건은 논리 타입이어야 합니다 (현재: " +
                    condType->koreanName() + ")");
        }

        if (whileStmt->body())
        {
            checkTypesInStatement(whileStmt->body());
        }
    }
}

Type* SemanticAnalyzer::inferType(const Expression* expr)
{
    if (!expr) return nullptr;

    // 리터럴 타입 추론
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

    // 식별자 타입 추론
    else if (auto ident = dynamic_cast<const Identifier*>(expr))
    {
        Symbol* symbol = symbolTable_.lookup(ident->name());
        return symbol ? symbol->type : nullptr;
    }

    // 이항 연산 타입 추론
    else if (auto binExpr = dynamic_cast<const BinaryExpression*>(expr))
    {
        Type* leftType = inferType(binExpr->left());
        Type* rightType = inferType(binExpr->right());

        if (!leftType || !rightType) return nullptr;

        const std::string& op = binExpr->op();

        // 산술 연산자 (+, -, *, /, %)
        if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%")
        {
            // 정수 + 정수 = 정수
            if (leftType->koreanName() == "정수" && rightType->koreanName() == "정수")
            {
                return Type::getBuiltin("정수");
            }
            // 실수가 하나라도 있으면 실수
            else if (leftType->koreanName() == "실수" || rightType->koreanName() == "실수")
            {
                return Type::getBuiltin("실수");
            }
            // 문자열 + 문자열 = 문자열 (연결)
            else if (op == "+" && leftType->koreanName() == "문자열" && rightType->koreanName() == "문자열")
            {
                return Type::getBuiltin("문자열");
            }
        }

        // 비교 연산자 (<, >, <=, >=, ==, !=)
        else if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=")
        {
            return Type::getBuiltin("논리");
        }

        // 논리 연산자 (&&, ||)
        else if (op == "&&" || op == "||")
        {
            return Type::getBuiltin("논리");
        }
    }

    // 단항 연산 타입 추론
    else if (auto unaryExpr = dynamic_cast<const UnaryExpression*>(expr))
    {
        Type* operandType = inferType(unaryExpr->operand());

        if (!operandType) return nullptr;

        const std::string& op = unaryExpr->op();

        // 부정 연산자 (!)
        if (op == "!")
        {
            return Type::getBuiltin("논리");
        }
        // 부호 연산자 (-, +)
        else if (op == "-" || op == "+")
        {
            return operandType;  // 같은 타입 반환
        }
    }

    // 배열 리터럴 타입 추론
    else if (dynamic_cast<const ArrayLiteral*>(expr))
    {
        // TODO: 배열 타입 시스템 구현 후 처리
        // 현재는 기본 타입만 지원하므로 nullptr 반환
        return nullptr;
    }

    // 배열 인덱스 접근 타입 추론
    else if (dynamic_cast<const IndexExpression*>(expr))
    {
        // TODO: 배열의 요소 타입 반환
        // 현재는 기본 타입만 지원하므로 nullptr 반환
        return nullptr;
    }

    // 함수 호출 타입 추론
    else if (dynamic_cast<const CallExpression*>(expr))
    {
        // TODO: 함수의 반환 타입 반환
        // 현재는 함수 타입 시스템이 없으므로 nullptr 반환
        return nullptr;
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
