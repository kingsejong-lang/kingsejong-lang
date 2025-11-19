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
    /// @brief UnusedVariableRule 생성자
    UnusedVariableRule()
        : Rule("unused-variable", "선언되었지만 사용되지 않는 변수 감지")
    {
    }

    /**
     * @brief 프로그램 분석 실행
     * @param program 분석할 AST 프로그램
     */
    void analyze(ast::Program* program) override;

protected:
    /// @brief 변수 선언 추적
    void onVarDeclaration(ast::VarDeclaration* stmt) override;

    /// @brief 식별자 사용 추적
    void onIdentifier(ast::Identifier* expr) override;

private:
    std::unordered_map<std::string, ast::VarDeclaration*> declaredVars_;  ///< 변수 이름 → 선언 위치
    std::unordered_set<std::string> usedVars_;  ///< 사용된 변수 이름 목록
    std::string currentDeclaring_;  ///< 현재 선언 중인 변수 (자기 참조 방지)
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
