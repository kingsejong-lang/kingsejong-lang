#pragma once

/**
 * @file DeadCodeRule.h
 * @brief 도달 불가능한 코드 감지 규칙
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/Rule.h"

namespace kingsejong {
namespace linter {
namespace rules {

/**
 * @class DeadCodeRule
 * @brief return 문 이후 도달할 수 없는 코드 감지
 */
class DeadCodeRule : public Rule
{
public:
    DeadCodeRule()
        : Rule("dead-code", "도달할 수 없는 코드 감지")
    {
    }

    void analyze(ast::Program* program) override;

protected:
    void onBlockStatement(ast::BlockStatement* stmt) override;
    void onVarDeclaration(ast::VarDeclaration* stmt) override;

private:
    /**
     * @brief 블록 내에서 return 문 이후의 문장들을 감지
     */
    void checkBlockForDeadCode(ast::BlockStatement* block);
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
