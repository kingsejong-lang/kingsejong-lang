/**
 * @file ConstantConditionRule.cpp
 * @brief ConstantConditionRule 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/rules/ConstantConditionRule.h"
#include "linter/Linter.h"
#include "ast/Visitor.h"

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

namespace
{
/// 이 반복문을 끝내는 '중단' 이 본문에 있는지. 안쪽 반복문과 함수 안의 중단은 이 반복문을 끝내지 않는다
class BreakFinder : public ast::Visitor
{
public:
    bool found = false;

    bool enter(ast::Node& node) override
    {
        switch (node.type())
        {
            case NodeType::BREAK_STATEMENT:
                found = true;
                return false;
            case NodeType::WHILE_STATEMENT:
            case NodeType::REPEAT_STATEMENT:
            case NodeType::RANGE_FOR_STATEMENT:
            case NodeType::FUNCTION_LITERAL:
            case NodeType::ASYNC_FUNCTION_LITERAL:
                return false;
            default:
                return !found;
        }
    }
};

bool bodyBreaksOut(const Statement* body)
{
    if (!body) return false;
    BreakFinder finder;
    ast::walk(const_cast<Statement*>(body), finder);
    return finder.found;
}
} // namespace

void ConstantConditionRule::onWhileStatement(WhileStatement* stmt)
{
    if (!stmt) return;

    bool value;
    if (isConstantExpression(stmt->condition(), value))
    {
        if (value)
        {
            // 반복 (참) { ... 중단 } 은 의도한 무한 루프가 아니다
            if (!bodyBreaksOut(stmt->body()))
            {
                reportIssue(
                    "조건이 항상 참입니다. 무한 루프가 발생합니다",
                    IssueSeverity::WARNING,
                    stmt->location().line,
                    stmt->location().column
                );
            }
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
    if (auto boolLit = node_cast<BooleanLiteral>(expr))
    {
        value = boolLit->value();
        return true;
    }

    // Integer 리터럴 (0은 거짓, 그 외는 참)
    if (auto intLit = node_cast<IntegerLiteral>(expr))
    {
        value = (intLit->value() != 0);
        return true;
    }

    return false;
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
