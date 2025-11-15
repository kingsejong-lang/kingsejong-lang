/**
 * @file EmptyBlockRule.cpp
 * @brief EmptyBlockRule 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/rules/EmptyBlockRule.h"
#include "linter/Linter.h"

namespace kingsejong {
namespace linter {
namespace rules {

using namespace ast;

void EmptyBlockRule::analyze(Program* program)
{
    if (!program) return;

    // 모든 문장 순회
    for (const auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }
}

void EmptyBlockRule::onIfStatement(IfStatement* stmt)
{
    if (!stmt) return;

    // then 분기가 비어있는지 확인
    if (auto thenBlock = dynamic_cast<const BlockStatement*>(stmt->thenBranch()))
    {
        if (isEmpty(thenBlock))
        {
            reportIssue(
                "빈 if 블록입니다. 불필요한 코드를 제거하세요",
                IssueSeverity::WARNING,
                stmt->location().line,
                stmt->location().column
            );
        }
    }

    // else 분기가 비어있는지 확인
    if (stmt->elseBranch())
    {
        if (auto elseBlock = dynamic_cast<const BlockStatement*>(stmt->elseBranch()))
        {
            if (isEmpty(elseBlock))
            {
                reportIssue(
                    "빈 else 블록입니다. 불필요한 코드를 제거하세요",
                    IssueSeverity::WARNING,
                    stmt->location().line,
                    stmt->location().column
                );
            }
        }
    }
}

void EmptyBlockRule::onWhileStatement(WhileStatement* stmt)
{
    if (!stmt) return;

    if (auto bodyBlock = dynamic_cast<const BlockStatement*>(stmt->body()))
    {
        if (isEmpty(bodyBlock))
        {
            reportIssue(
                "빈 while 블록입니다. 불필요한 코드를 제거하세요",
                IssueSeverity::WARNING,
                stmt->location().line,
                stmt->location().column
            );
        }
    }
}

void EmptyBlockRule::onRangeForStatement(RangeForStatement* stmt)
{
    if (!stmt) return;

    if (auto bodyBlock = dynamic_cast<const BlockStatement*>(stmt->body()))
    {
        if (isEmpty(bodyBlock))
        {
            reportIssue(
                "빈 for 블록입니다. 불필요한 코드를 제거하세요",
                IssueSeverity::WARNING,
                stmt->location().line,
                stmt->location().column
            );
        }
    }
}

void EmptyBlockRule::onRepeatStatement(RepeatStatement* stmt)
{
    if (!stmt) return;

    if (auto bodyBlock = dynamic_cast<const BlockStatement*>(stmt->body()))
    {
        if (isEmpty(bodyBlock))
        {
            reportIssue(
                "빈 repeat 블록입니다. 불필요한 코드를 제거하세요",
                IssueSeverity::WARNING,
                stmt->location().line,
                stmt->location().column
            );
        }
    }
}

void EmptyBlockRule::onFunctionLiteral(FunctionLiteral* expr)
{
    if (!expr) return;

    if (auto bodyBlock = dynamic_cast<const BlockStatement*>(expr->body()))
    {
        if (isEmpty(bodyBlock))
        {
            reportIssue(
                "빈 함수 본문입니다. 함수 구현을 추가하세요",
                IssueSeverity::INFO,  // 함수는 INFO 레벨 (의도적일 수 있음)
                expr->location().line,
                expr->location().column
            );
        }
    }
}

void EmptyBlockRule::onVarDeclaration(VarDeclaration* stmt)
{
    if (!stmt) return;

    // initializer 방문하여 내부의 FunctionLiteral 검사
    if (stmt->initializer())
    {
        visitExpression(const_cast<Expression*>(stmt->initializer()));
    }
}

bool EmptyBlockRule::isEmpty(const BlockStatement* block)
{
    if (!block) return true;
    return block->statements().empty();
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
