#pragma once

/**
 * @file ConstantConditionRule.h
 * @brief 상수 조건 감지 규칙
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/Rule.h"

namespace kingsejong {
namespace linter {
namespace rules {

/**
 * @class ConstantConditionRule
 * @brief 항상 참 또는 거짓인 조건문 감지 (if (참), while (거짓) 등)
 */
class ConstantConditionRule : public Rule
{
public:
    ConstantConditionRule()
        : Rule("constant-condition", "항상 참 또는 거짓인 조건 감지")
    {
    }

    void analyze(ast::Program* program) override;

protected:
    void onIfStatement(ast::IfStatement* stmt) override;
    void onWhileStatement(ast::WhileStatement* stmt) override;

private:
    /**
     * @brief 표현식이 상수인지 확인
     */
    bool isConstantExpression(const ast::Expression* expr, bool& value);
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
