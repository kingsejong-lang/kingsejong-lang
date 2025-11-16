/**
 * @file NoShadowingRule.cpp
 * @brief NoShadowingRule 구현
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "linter/rules/NoShadowingRule.h"
#include "linter/Linter.h"

namespace kingsejong {
namespace linter {
namespace rules {

using namespace ast;

void NoShadowingRule::analyze(Program* program)
{
    if (!program) return;

    // 초기화
    scopeStack_.clear();

    // 전역 스코프 시작
    pushScope();

    // 모든 문장 순회
    for (const auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }

    // 전역 스코프 종료
    popScope();
}

void NoShadowingRule::onVarDeclaration(VarDeclaration* stmt)
{
    if (!stmt) return;

    const std::string& varName = stmt->varName();

    // 외부 스코프에서 동일한 이름의 변수 찾기
    auto* outerDecl = findInOuterScopes(varName);
    if (outerDecl)
    {
        reportIssue(
            "변수 '" + varName + "'이(가) 외부 스코프의 변수를 가립니다 "
            "(외부 정의: " + std::to_string(outerDecl->location().line) + "줄)",
            IssueSeverity::WARNING,
            stmt->location().line,
            stmt->location().column
        );
    }

    // 외부 스코프의 매개변수를 가리는지 확인
    if (isParameterInOuterScopes(varName))
    {
        reportIssue(
            "변수 '" + varName + "'이(가) 외부 함수의 매개변수를 가립니다",
            IssueSeverity::WARNING,
            stmt->location().line,
            stmt->location().column
        );
    }

    // 현재 스코프에 변수 추가
    addVariable(varName, stmt);

    // 초기화 표현식 순회
    if (stmt->initializer())
    {
        visitExpression(const_cast<Expression*>(stmt->initializer()));
    }
}

void NoShadowingRule::onFunctionLiteral(FunctionLiteral* expr)
{
    if (!expr) return;

    // 새 스코프 시작
    pushScope();

    // 매개변수를 현재 스코프에 추가
    for (const std::string& param : expr->parameters())
    {
        scopeStack_.back().parameters.push_back(param);
    }

    // 함수 본문 순회
    if (expr->body())
    {
        visitStatement(expr->body());
    }

    // 스코프 종료
    popScope();
}

void NoShadowingRule::onBlockStatement(BlockStatement* stmt)
{
    if (!stmt) return;

    // 블록 스코프 시작
    pushScope();

    // 블록 내 문장 순회
    for (const auto& s : stmt->statements())
    {
        visitStatement(s.get());
    }

    // 스코프 종료
    popScope();
}

void NoShadowingRule::onRangeForStatement(RangeForStatement* stmt)
{
    if (!stmt) return;

    // 범위 표현식 먼저 순회 (반복 변수 스코프 밖)
    visitExpression(const_cast<Expression*>(stmt->start()));
    visitExpression(const_cast<Expression*>(stmt->end()));

    // 반복문 스코프 시작
    pushScope();

    // 반복 변수를 매개변수처럼 처리
    const std::string& loopVar = stmt->varName();
    scopeStack_.back().parameters.push_back(loopVar);

    // 외부 스코프에서 동일한 이름 검사
    auto* outerDecl = findInOuterScopes(loopVar);
    if (outerDecl)
    {
        reportIssue(
            "반복 변수 '" + loopVar + "'이(가) 외부 스코프의 변수를 가립니다 "
            "(외부 정의: " + std::to_string(outerDecl->location().line) + "줄)",
            IssueSeverity::WARNING,
            stmt->location().line,
            stmt->location().column
        );
    }

    // 반복문 본문 순회
    if (stmt->body())
    {
        visitStatement(const_cast<BlockStatement*>(stmt->body()));
    }

    // 스코프 종료
    popScope();
}

void NoShadowingRule::addVariable(const std::string& name, VarDeclaration* decl)
{
    if (!scopeStack_.empty())
    {
        scopeStack_.back().variables[name] = decl;
    }
}

VarDeclaration* NoShadowingRule::findInOuterScopes(const std::string& name)
{
    // 현재 스코프를 제외한 외부 스코프들에서 검색
    for (int i = static_cast<int>(scopeStack_.size()) - 2; i >= 0; i--)
    {
        const Scope& scope = scopeStack_[i];
        auto it = scope.variables.find(name);
        if (it != scope.variables.end())
        {
            return it->second;
        }
    }
    return nullptr;
}

bool NoShadowingRule::isParameterInOuterScopes(const std::string& name)
{
    // 현재 스코프를 제외한 외부 스코프들의 매개변수 검색
    for (int i = static_cast<int>(scopeStack_.size()) - 2; i >= 0; i--)
    {
        const Scope& scope = scopeStack_[i];
        for (const std::string& param : scope.parameters)
        {
            if (param == name)
            {
                return true;
            }
        }
    }
    return false;
}

void NoShadowingRule::pushScope()
{
    scopeStack_.push_back(Scope());
}

void NoShadowingRule::popScope()
{
    if (!scopeStack_.empty())
    {
        scopeStack_.pop_back();
    }
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
