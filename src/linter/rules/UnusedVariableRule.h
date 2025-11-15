#pragma once

/**
 * @file UnusedVariableRule.h
 * @brief 미사용 변수 감지 규칙
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/Rule.h"
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace kingsejong {
namespace linter {
namespace rules {

/**
 * @class UnusedVariableRule
 * @brief 선언되었지만 사용되지 않는 변수 감지
 */
class UnusedVariableRule : public Rule
{
public:
    UnusedVariableRule()
        : Rule("unused-variable", "선언되었지만 사용되지 않는 변수 감지")
    {
    }

    void analyze(ast::Program* program) override;

protected:
    void onVarDeclaration(ast::VarDeclaration* stmt) override;
    void onIdentifier(ast::Identifier* expr) override;

private:
    // 변수 이름 -> 선언 위치
    std::unordered_map<std::string, ast::VarDeclaration*> declaredVars_;

    // 사용된 변수 이름 목록
    std::unordered_set<std::string> usedVars_;

    // 현재 처리 중인 변수 선언 (자기 자신 초기화는 사용으로 간주하지 않음)
    std::string currentDeclaring_;
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
