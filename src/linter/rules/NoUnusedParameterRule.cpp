/**
 * @file NoUnusedParameterRule.cpp
 * @brief NoUnusedParameterRule 구현
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "linter/rules/NoUnusedParameterRule.h"
#include "linter/Linter.h"
#include <unordered_map>
#include <unordered_set>

namespace kingsejong {
namespace linter {
namespace rules {

using namespace ast;

// Helper: 함수 내에서 사용된 식별자 수집
class IdentifierCollector
{
public:
    std::unordered_set<std::string> identifiers;

    void collect(const Statement* stmt)
    {
        if (!stmt) return;

        // VarDeclaration
        if (auto varDecl = dynamic_cast<const VarDeclaration*>(stmt))
        {
            if (varDecl->initializer())
            {
                collectExpr(varDecl->initializer());
            }
        }
        // AssignmentStatement
        else if (auto assignStmt = dynamic_cast<const AssignmentStatement*>(stmt))
        {
            collectExpr(assignStmt->value());
        }
        // ExpressionStatement
        else if (auto exprStmt = dynamic_cast<const ExpressionStatement*>(stmt))
        {
            collectExpr(exprStmt->expression());
        }
        // ReturnStatement
        else if (auto retStmt = dynamic_cast<const ReturnStatement*>(stmt))
        {
            if (retStmt->returnValue())
            {
                collectExpr(retStmt->returnValue());
            }
        }
        // IfStatement
        else if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt))
        {
            collectExpr(ifStmt->condition());
            if (ifStmt->thenBranch()) collect(ifStmt->thenBranch());
            if (ifStmt->elseBranch()) collect(ifStmt->elseBranch());
        }
        // WhileStatement
        else if (auto whileStmt = dynamic_cast<const WhileStatement*>(stmt))
        {
            collectExpr(whileStmt->condition());
            if (whileStmt->body()) collect(whileStmt->body());
        }
        // BlockStatement
        else if (auto blockStmt = dynamic_cast<const BlockStatement*>(stmt))
        {
            for (const auto& s : blockStmt->statements())
            {
                collect(s.get());
            }
        }
        // RangeForStatement
        else if (auto forStmt = dynamic_cast<const RangeForStatement*>(stmt))
        {
            collectExpr(forStmt->start());
            collectExpr(forStmt->end());
            if (forStmt->body()) collect(forStmt->body());
        }
        // RepeatStatement
        else if (auto repeatStmt = dynamic_cast<const RepeatStatement*>(stmt))
        {
            collectExpr(repeatStmt->count());
            if (repeatStmt->body()) collect(repeatStmt->body());
        }
    }

    void collectExpr(const Expression* expr)
    {
        if (!expr) return;

        // Identifier
        if (auto ident = dynamic_cast<const Identifier*>(expr))
        {
            identifiers.insert(ident->name());
        }
        // BinaryExpression
        else if (auto binExpr = dynamic_cast<const BinaryExpression*>(expr))
        {
            collectExpr(binExpr->left());
            collectExpr(binExpr->right());
        }
        // UnaryExpression
        else if (auto unaryExpr = dynamic_cast<const UnaryExpression*>(expr))
        {
            collectExpr(unaryExpr->operand());
        }
        // CallExpression
        else if (auto callExpr = dynamic_cast<const CallExpression*>(expr))
        {
            collectExpr(callExpr->function());
            for (const auto& arg : callExpr->arguments())
            {
                collectExpr(arg.get());
            }
        }
        // IndexExpression
        else if (auto indexExpr = dynamic_cast<const IndexExpression*>(expr))
        {
            collectExpr(indexExpr->array());
            collectExpr(indexExpr->index());
        }
        // ArrayLiteral
        else if (auto arrayLit = dynamic_cast<const ArrayLiteral*>(expr))
        {
            for (const auto& elem : arrayLit->elements())
            {
                collectExpr(elem.get());
            }
        }
        // FunctionLiteral - 중첩 함수는 무시 (외부 매개변수 사용은 클로저이므로 허용)
    }
};

void NoUnusedParameterRule::analyze(Program* program)
{
    if (!program) return;

    // 초기화
    functionStack_.clear();

    // 모든 문장 순회
    for (const auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }
}

void NoUnusedParameterRule::onFunctionLiteral(FunctionLiteral* expr)
{
    if (!expr) return;

    // 함수 본문에서 사용된 식별자 수집
    IdentifierCollector collector;
    if (expr->body())
    {
        collector.collect(expr->body());
    }

    // 미사용 매개변수 검사
    for (const std::string& param : expr->parameters())
    {
        // 언더스코어로 시작하는 매개변수는 의도적으로 미사용인 것으로 간주
        if (!param.empty() && param[0] == '_')
        {
            continue;
        }

        // 사용되지 않았으면 경고
        if (collector.identifiers.find(param) == collector.identifiers.end())
        {
            reportIssue(
                "함수 매개변수 '" + param + "'이(가) 사용되지 않습니다. "
                "의도적인 경우 '_" + param + "'로 표시하세요",
                IssueSeverity::WARNING,
                expr->location().line,
                expr->location().column
            );
        }
    }
}

void NoUnusedParameterRule::onIdentifier(Identifier* /* expr */)
{
    // 더 이상 필요 없음 - IdentifierCollector가 처리함
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
