/**
 * @file ConstantConditionRule.cpp
 * @brief ConstantConditionRule 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/rules/ConstantConditionRule.h"
#include "linter/Linter.h"

namespace kingsejong {
namespace linter {
namespace rules {

using namespace ast;

void ConstantConditionRule::analyze(Program* program)
{
    if (!program) return;

    // 모든 문장 순회
    for (const auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }
}

void ConstantConditionRule::onIfStatement(IfStatement* stmt)
{
    if (!stmt) return;

    bool value;
    if (isConstantExpression(stmt->condition(), value))
    {
        if (value)
        {
            reportIssue(
                "조건이 항상 참입니다. else 분기는 실행되지 않습니다",
                IssueSeverity::WARNING,
                stmt->location().line,
                stmt->location().column
            );
        }
        else
        {
            reportIssue(
                "조건이 항상 거짓입니다. then 분기는 실행되지 않습니다",
                IssueSeverity::WARNING,
                stmt->location().line,
                stmt->location().column
            );
        }
    }
}

void ConstantConditionRule::onWhileStatement(WhileStatement* stmt)
{
    if (!stmt) return;

    bool value;
    if (isConstantExpression(stmt->condition(), value))
    {
        if (value)
        {
            reportIssue(
                "조건이 항상 참입니다. 무한 루프가 발생합니다",
                IssueSeverity::WARNING,
                stmt->location().line,
                stmt->location().column
            );
        }
        else
        {
            reportIssue(
                "조건이 항상 거짓입니다. 루프가 실행되지 않습니다",
                IssueSeverity::WARNING,
                stmt->location().line,
                stmt->location().column
            );
        }
    }
}

bool ConstantConditionRule::isConstantExpression(const Expression* expr, bool& value)
{
    if (!expr) return false;

    // Boolean 리터럴
    if (auto boolLit = dynamic_cast<const BooleanLiteral*>(expr))
    {
        value = boolLit->value();
        return true;
    }

    // Integer 리터럴 (0은 거짓, 그 외는 참)
    if (auto intLit = dynamic_cast<const IntegerLiteral*>(expr))
    {
        value = (intLit->value() != 0);
        return true;
    }

    return false;
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
