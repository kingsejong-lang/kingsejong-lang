/**
 * @file NoSelfComparisonRule.cpp
 * @brief NoSelfComparisonRule 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/rules/NoSelfComparisonRule.h"
#include "linter/Linter.h"

namespace kingsejong {
namespace linter {
namespace rules {

using namespace ast;

void NoSelfComparisonRule::analyze(Program* program)
{
    if (!program) return;

    // 모든 문장 순회
    for (const auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }
}

void NoSelfComparisonRule::onBinaryExpression(BinaryExpression* expr)
{
    if (!expr) return;

    // 비교 연산자가 아니면 무시
    if (!isComparisonOperator(expr->op()))
    {
        return;
    }

    // 좌변과 우변이 동일한 식별자인지 확인
    if (isSameIdentifier(expr->left(), expr->right()))
    {
        reportIssue(
            "변수를 자기 자신과 비교하고 있습니다 (항상 같은 결과)",
            IssueSeverity::WARNING,
            expr->location().line,
            expr->location().column
        );
    }
}

void NoSelfComparisonRule::onVarDeclaration(VarDeclaration* stmt)
{
    if (!stmt) return;

    // initializer 방문하여 내부의 BinaryExpression 검사
    if (stmt->initializer())
    {
        visitExpression(const_cast<Expression*>(stmt->initializer()));
    }
}

bool NoSelfComparisonRule::isSameIdentifier(const Expression* left, const Expression* right)
{
    if (!left || !right) return false;

    // 둘 다 Identifier인지 확인
    auto leftId = dynamic_cast<const Identifier*>(left);
    auto rightId = dynamic_cast<const Identifier*>(right);

    if (!leftId || !rightId) return false;

    // 이름이 같은지 확인
    return leftId->name() == rightId->name();
}

bool NoSelfComparisonRule::isComparisonOperator(const std::string& op)
{
    return op == "==" || op == "!=" ||
           op == "<" || op == ">" ||
           op == "<=" || op == ">=";
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
