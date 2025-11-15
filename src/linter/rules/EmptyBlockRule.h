#pragma once

/**
 * @file EmptyBlockRule.h
 * @brief 빈 블록 감지 규칙
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/Rule.h"

namespace kingsejong {
namespace linter {
namespace rules {

/**
 * @class EmptyBlockRule
 * @brief 빈 블록 감지 (의미 없는 빈 if, while, 함수 등)
 */
class EmptyBlockRule : public Rule
{
public:
    EmptyBlockRule()
        : Rule("empty-block", "빈 블록 감지")
    {
    }

    void analyze(ast::Program* program) override;

protected:
    void onIfStatement(ast::IfStatement* stmt) override;
    void onWhileStatement(ast::WhileStatement* stmt) override;
    void onRangeForStatement(ast::RangeForStatement* stmt) override;
    void onRepeatStatement(ast::RepeatStatement* stmt) override;
    void onFunctionLiteral(ast::FunctionLiteral* expr) override;
    void onVarDeclaration(ast::VarDeclaration* stmt) override;

private:
    /**
     * @brief 블록이 비어있는지 확인
     */
    bool isEmpty(const ast::BlockStatement* block);
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
