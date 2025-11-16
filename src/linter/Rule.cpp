/**
 * @file Rule.cpp
 * @brief Rule 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/Rule.h"
#include "linter/Linter.h"

namespace kingsejong {
namespace linter {

using namespace ast;

void Rule::reportIssue(
    const std::string& message,
    IssueSeverity severity,
    int line,
    int column
)
{
    if (linter_)
    {
        // 설정 파일에서 오버라이드된 심각도가 있으면 사용
        auto overrideSeverity = linter_->getRuleSeverity(ruleId_);
        IssueSeverity finalSeverity = overrideSeverity.has_value() ? overrideSeverity.value() : severity;

        linter_->addIssue(LinterIssue{message, finalSeverity, ruleId_, line, column, linter_->filename_});
    }
}

void Rule::visitStatement(Statement* stmt)
{
    if (!stmt) return;

    // VarDeclaration
    if (auto varDecl = dynamic_cast<VarDeclaration*>(stmt))
    {
        onVarDeclaration(varDecl);
        // Note: initializer is visited inside onVarDeclaration, don't visit again
    }
    // AssignmentStatement
    else if (auto assignStmt = dynamic_cast<AssignmentStatement*>(stmt))
    {
        onAssignmentStatement(assignStmt);
        visitExpression(const_cast<Expression*>(assignStmt->value()));
    }
    // ExpressionStatement
    else if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt))
    {
        onExpressionStatement(exprStmt);
        visitExpression(const_cast<Expression*>(exprStmt->expression()));
    }
    // ReturnStatement
    else if (auto retStmt = dynamic_cast<ReturnStatement*>(stmt))
    {
        onReturnStatement(retStmt);
        if (retStmt->returnValue())
        {
            visitExpression(const_cast<Expression*>(retStmt->returnValue()));
        }
    }
    // IfStatement
    else if (auto ifStmt = dynamic_cast<IfStatement*>(stmt))
    {
        onIfStatement(ifStmt);
        visitExpression(const_cast<Expression*>(ifStmt->condition()));
        if (ifStmt->thenBranch())
        {
            visitStatement(const_cast<BlockStatement*>(ifStmt->thenBranch()));
        }
        if (ifStmt->elseBranch())
        {
            visitStatement(const_cast<BlockStatement*>(ifStmt->elseBranch()));
        }
    }
    // WhileStatement
    else if (auto whileStmt = dynamic_cast<WhileStatement*>(stmt))
    {
        onWhileStatement(whileStmt);
        visitExpression(const_cast<Expression*>(whileStmt->condition()));
        if (whileStmt->body())
        {
            visitStatement(const_cast<BlockStatement*>(whileStmt->body()));
        }
    }
    // BlockStatement
    else if (auto blockStmt = dynamic_cast<BlockStatement*>(stmt))
    {
        onBlockStatement(blockStmt);
        for (const auto& s : blockStmt->statements())
        {
            visitStatement(s.get());
        }
    }
    // RangeForStatement
    else if (auto forStmt = dynamic_cast<RangeForStatement*>(stmt))
    {
        onRangeForStatement(forStmt);
        visitExpression(const_cast<Expression*>(forStmt->start()));
        visitExpression(const_cast<Expression*>(forStmt->end()));
        if (forStmt->body())
        {
            visitStatement(const_cast<BlockStatement*>(forStmt->body()));
        }
    }
    // RepeatStatement
    else if (auto repeatStmt = dynamic_cast<RepeatStatement*>(stmt))
    {
        onRepeatStatement(repeatStmt);
        visitExpression(const_cast<Expression*>(repeatStmt->count()));
        if (repeatStmt->body())
        {
            visitStatement(const_cast<BlockStatement*>(repeatStmt->body()));
        }
    }
}

void Rule::visitExpression(Expression* expr)
{
    if (!expr) return;

    // Identifier
    if (auto ident = dynamic_cast<Identifier*>(expr))
    {
        onIdentifier(ident);
    }
    // IntegerLiteral
    else if (auto intLit = dynamic_cast<IntegerLiteral*>(expr))
    {
        onIntegerLiteral(intLit);
    }
    // FloatLiteral
    else if (auto floatLit = dynamic_cast<FloatLiteral*>(expr))
    {
        onFloatLiteral(floatLit);
    }
    // StringLiteral
    else if (auto strLit = dynamic_cast<StringLiteral*>(expr))
    {
        onStringLiteral(strLit);
    }
    // BooleanLiteral
    else if (auto boolLit = dynamic_cast<BooleanLiteral*>(expr))
    {
        onBooleanLiteral(boolLit);
    }
    // ArrayLiteral
    else if (auto arrayLit = dynamic_cast<ArrayLiteral*>(expr))
    {
        onArrayLiteral(arrayLit);
        for (const auto& elem : arrayLit->elements())
        {
            visitExpression(elem.get());
        }
    }
    // BinaryExpression
    else if (auto binExpr = dynamic_cast<BinaryExpression*>(expr))
    {
        onBinaryExpression(binExpr);
        visitExpression(const_cast<Expression*>(binExpr->left()));
        visitExpression(const_cast<Expression*>(binExpr->right()));
    }
    // UnaryExpression
    else if (auto unaryExpr = dynamic_cast<UnaryExpression*>(expr))
    {
        onUnaryExpression(unaryExpr);
        visitExpression(const_cast<Expression*>(unaryExpr->operand()));
    }
    // CallExpression
    else if (auto callExpr = dynamic_cast<CallExpression*>(expr))
    {
        onCallExpression(callExpr);
        visitExpression(const_cast<Expression*>(callExpr->function()));
        for (const auto& arg : callExpr->arguments())
        {
            visitExpression(arg.get());
        }
    }
    // IndexExpression
    else if (auto indexExpr = dynamic_cast<IndexExpression*>(expr))
    {
        onIndexExpression(indexExpr);
        visitExpression(const_cast<Expression*>(indexExpr->array()));
        visitExpression(const_cast<Expression*>(indexExpr->index()));
    }
    // FunctionLiteral
    else if (auto funcLit = dynamic_cast<FunctionLiteral*>(expr))
    {
        onFunctionLiteral(funcLit);
        if (funcLit->body())
        {
            // body()는 Statement*를 반환하므로 그대로 사용
            visitStatement(funcLit->body());
        }
    }
}

} // namespace linter
} // namespace kingsejong
