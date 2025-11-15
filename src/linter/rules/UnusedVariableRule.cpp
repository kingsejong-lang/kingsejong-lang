/**
 * @file UnusedVariableRule.cpp
 * @brief UnusedVariableRule 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/rules/UnusedVariableRule.h"
#include "linter/Linter.h"

namespace kingsejong {
namespace linter {
namespace rules {

using namespace ast;

void UnusedVariableRule::analyze(Program* program)
{
    if (!program) return;

    // 초기화
    declaredVars_.clear();
    usedVars_.clear();
    currentDeclaring_.clear();

    // 모든 문장 순회
    for (const auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }

    // 미사용 변수 보고
    for (const auto& pair : declaredVars_)
    {
        const std::string& varName = pair.first;
        VarDeclaration* decl = pair.second;

        // 사용되지 않았고, 언더스코어로 시작하지 않으면 경고
        if (usedVars_.find(varName) == usedVars_.end() &&
            !varName.empty() && varName[0] != '_')
        {
            reportIssue(
                "변수 '" + varName + "'이(가) 선언되었지만 사용되지 않습니다",
                IssueSeverity::WARNING,
                decl->location().line,
                decl->location().column
            );
        }
    }
}

void UnusedVariableRule::onVarDeclaration(VarDeclaration* stmt)
{
    if (!stmt) return;

    const std::string& varName = stmt->varName();

    // 변수 선언 기록
    declaredVars_[varName] = stmt;

    // 초기화 값 처리 시 자기 자신을 사용으로 간주하지 않음
    currentDeclaring_ = varName;
    if (stmt->initializer())
    {
        visitExpression(const_cast<ast::Expression*>(stmt->initializer()));
    }
    currentDeclaring_.clear();
}

void UnusedVariableRule::onIdentifier(Identifier* expr)
{
    if (!expr) return;

    const std::string& name = expr->name();

    // 현재 선언 중인 변수가 아니면 사용으로 간주
    if (name != currentDeclaring_)
    {
        usedVars_.insert(name);
    }
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
