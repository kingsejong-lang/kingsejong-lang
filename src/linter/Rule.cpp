/**
 * @file Rule.cpp
 * @brief Rule 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/Rule.h"
#include "ast/Visitor.h"
#include "linter/Linter.h"

namespace kingsejong {
namespace linter {

using namespace ast;

void Rule::reportIssue(
    const std::string& message,
    IssueSeverity severity,
    int line,
    int column,
    const std::string& subject
)
{
    if (linter_)
    {
        // 설정 파일에서 오버라이드된 심각도가 있으면 사용
        auto overrideSeverity = linter_->getRuleSeverity(ruleId_);
        IssueSeverity finalSeverity = overrideSeverity.has_value() ? overrideSeverity.value() : severity;

        linter_->addIssue(LinterIssue{message, finalSeverity, ruleId_, line, column, linter_->filename_, subject});
    }
}

/// 공용 순회 위에서 규칙의 on* 훅을 부른다. 자식 열거는 ast::walk 가 맡는다.
class HookDispatcher : public ast::Visitor
{
public:
    explicit HookDispatcher(Rule& rule) : rule_(rule) {}

    bool enter(ast::Node& node) override
    {
        switch (node.type())
        {
            case ast::NodeType::VAR_DECLARATION:        rule_.onVarDeclaration(static_cast<VarDeclaration*>(&node)); break;
            case ast::NodeType::ASSIGNMENT_STATEMENT:   rule_.onAssignmentStatement(static_cast<AssignmentStatement*>(&node)); break;
            case ast::NodeType::EXPRESSION_STATEMENT:   rule_.onExpressionStatement(static_cast<ExpressionStatement*>(&node)); break;
            case ast::NodeType::RETURN_STATEMENT:       rule_.onReturnStatement(static_cast<ReturnStatement*>(&node)); break;
            case ast::NodeType::IF_STATEMENT:           rule_.onIfStatement(static_cast<IfStatement*>(&node)); break;
            case ast::NodeType::WHILE_STATEMENT:        rule_.onWhileStatement(static_cast<WhileStatement*>(&node)); break;
            case ast::NodeType::BLOCK_STATEMENT:        rule_.onBlockStatement(static_cast<BlockStatement*>(&node)); break;
            case ast::NodeType::RANGE_FOR_STATEMENT:    rule_.onRangeForStatement(static_cast<RangeForStatement*>(&node)); break;
            case ast::NodeType::REPEAT_STATEMENT:       rule_.onRepeatStatement(static_cast<RepeatStatement*>(&node)); break;
            case ast::NodeType::IMPORT_STATEMENT:       rule_.onImportStatement(static_cast<ImportStatement*>(&node)); break;
            case ast::NodeType::IDENTIFIER:             rule_.onIdentifier(static_cast<Identifier*>(&node)); break;
            case ast::NodeType::INTEGER_LITERAL:        rule_.onIntegerLiteral(static_cast<IntegerLiteral*>(&node)); break;
            case ast::NodeType::FLOAT_LITERAL:          rule_.onFloatLiteral(static_cast<FloatLiteral*>(&node)); break;
            case ast::NodeType::STRING_LITERAL:         rule_.onStringLiteral(static_cast<StringLiteral*>(&node)); break;
            case ast::NodeType::BOOLEAN_LITERAL:        rule_.onBooleanLiteral(static_cast<BooleanLiteral*>(&node)); break;
            case ast::NodeType::ARRAY_LITERAL:          rule_.onArrayLiteral(static_cast<ArrayLiteral*>(&node)); break;
            case ast::NodeType::BINARY_EXPRESSION:      rule_.onBinaryExpression(static_cast<BinaryExpression*>(&node)); break;
            case ast::NodeType::UNARY_EXPRESSION:       rule_.onUnaryExpression(static_cast<UnaryExpression*>(&node)); break;
            case ast::NodeType::CALL_EXPRESSION:        rule_.onCallExpression(static_cast<CallExpression*>(&node)); break;
            case ast::NodeType::INDEX_EXPRESSION:       rule_.onIndexExpression(static_cast<IndexExpression*>(&node)); break;
            case ast::NodeType::FUNCTION_LITERAL:       rule_.onFunctionLiteral(static_cast<FunctionLiteral*>(&node)); break;
            case ast::NodeType::JOSA_EXPRESSION:        rule_.onJosaExpression(static_cast<JosaExpression*>(&node)); break;
            default: break;
        }
        return true;
    }

    void leave(ast::Node& node) override
    {
        switch (node.type())
        {
            case ast::NodeType::VAR_DECLARATION: rule_.onVarDeclarationEnd(static_cast<VarDeclaration*>(&node)); break;
            case ast::NodeType::ARRAY_LITERAL:   rule_.onArrayLiteralEnd(static_cast<ArrayLiteral*>(&node)); break;
            default: break;
        }
    }

private:
    Rule& rule_;
};

void Rule::visitStatement(Statement* stmt)
{
    HookDispatcher dispatcher(*this);
    ast::walk(stmt, dispatcher);
}

void Rule::visitExpression(Expression* expr)
{
    HookDispatcher dispatcher(*this);
    ast::walk(expr, dispatcher);
}

} // namespace linter
} // namespace kingsejong
