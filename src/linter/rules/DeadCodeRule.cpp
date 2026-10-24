/**
 * @file DeadCodeRule.cpp
 * @brief DeadCodeRule 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/rules/DeadCodeRule.h"
#include "linter/Linter.h"

namespace kingsejong {
namespace linter {
namespace rules {

using namespace ast;

void DeadCodeRule::analyze(Program* program)
{
    if (!program) return;

    // 프로그램 레벨의 문장들도 BlockStatement처럼 검사
    bool foundReturn = false;
    size_t returnIndex = 0;

    const auto& statements = program->statements();
    for (size_t i = 0; i < statements.size(); ++i)
    {
        if (node_cast<ReturnStatement>(statements[i].get()))
        {
            foundReturn = true;
            returnIndex = i;
            break;
        }
    }

    if (foundReturn && returnIndex + 1 < statements.size())
    {
        auto* deadStmt = statements[returnIndex + 1].get();
        reportIssue(
            "return 문 이후의 코드는 실행되지 않습니다",
            IssueSeverity::WARNING,
            deadStmt->location().line,
            deadStmt->location().column
        );
    }

    // 모든 문장 순회하여 중첩된 블록 검사
    for (const auto& stmt : statements)
    {
        visitStatement(stmt.get());
    }
}

void DeadCodeRule::onBlockStatement(BlockStatement* block)
{
    if (!block) return;

    // 이 블록 내부의 dead code 검사
    checkBlockForDeadCode(block);
}

void DeadCodeRule::checkBlockForDeadCode(BlockStatement* block)
{
    if (!block) return;

    const auto& statements = block->statements();
    bool foundReturn = false;
    size_t returnIndex = 0;

    // return 문 찾기
    for (size_t i = 0; i < statements.size(); ++i)
    {
        if (node_cast<ReturnStatement>(statements[i].get()))
        {
            foundReturn = true;
            returnIndex = i;
            break;
        }
    }

    // return 문 이후에 문장이 있으면 경고
    if (foundReturn && returnIndex + 1 < statements.size())
    {
        auto* deadStmt = statements[returnIndex + 1].get();
        reportIssue(
            "return 문 이후의 코드는 실행되지 않습니다",
            IssueSeverity::WARNING,
            deadStmt->location().line,
            deadStmt->location().column
        );
    }

    // 중첩된 블록도 검사
    for (const auto& stmt : statements)
    {
        if (auto ifStmt = node_cast<IfStatement>(stmt.get()))
        {
            if (ifStmt->thenBranch())
            {
                checkBlockForDeadCode(const_cast<BlockStatement*>(
                    node_cast<BlockStatement>(ifStmt->thenBranch())
                ));
            }
            if (ifStmt->elseBranch())
            {
                checkBlockForDeadCode(const_cast<BlockStatement*>(
                    node_cast<BlockStatement>(ifStmt->elseBranch())
                ));
            }
        }
        else if (auto whileStmt = node_cast<WhileStatement>(stmt.get()))
        {
            if (whileStmt->body())
            {
                checkBlockForDeadCode(const_cast<BlockStatement*>(
                    node_cast<BlockStatement>(whileStmt->body())
                ));
            }
        }
        else if (auto forStmt = node_cast<RangeForStatement>(stmt.get()))
        {
            if (forStmt->body())
            {
                checkBlockForDeadCode(const_cast<BlockStatement*>(
                    node_cast<BlockStatement>(forStmt->body())
                ));
            }
        }
        else if (auto repeatStmt = node_cast<RepeatStatement>(stmt.get()))
        {
            if (repeatStmt->body())
            {
                checkBlockForDeadCode(const_cast<BlockStatement*>(
                    node_cast<BlockStatement>(repeatStmt->body())
                ));
            }
        }
        else if (auto nestedBlock = node_cast<BlockStatement>(stmt.get()))
        {
            checkBlockForDeadCode(nestedBlock);
        }
    }
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
