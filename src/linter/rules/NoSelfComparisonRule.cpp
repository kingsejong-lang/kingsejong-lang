/**
 * @file NoSelfComparisonRule.cpp
 * @brief NoSelfComparisonRule 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/rules/NoSelfComparisonRule.h"
#include "linter/Linter.h"
#include "ast/Visitor.h"

namespace kingsejong {
namespace linter {
namespace rules {

using namespace ast;

namespace
{
/// 이름마다 대입·선언된 값이 실수가 될 수 있는지 모은다
class FloatTracker : public ast::Visitor
{
public:
    std::unordered_set<std::string> neverFloat;
    std::unordered_set<std::string> maybeFloat;

    bool enter(ast::Node& node) override
    {
        if (node.type() == NodeType::VAR_DECLARATION)
        {
            auto& decl = static_cast<VarDeclaration&>(node);
            const std::string& type = decl.typeName();
            bool typed = type == "정수" || type == "문자열" || type == "논리" || type == "참거짓" || type == "문자";
            record(decl.varName(), typed || isNonFloatLiteral(decl.initializer()));
        }
        else if (node.type() == NodeType::ASSIGNMENT_STATEMENT)
        {
            auto& stmt = static_cast<AssignmentStatement&>(node);
            record(stmt.varName(), isNonFloatLiteral(stmt.value()));
        }
        return true;
    }

private:
    static bool isNonFloatLiteral(const Expression* value)
    {
        if (!value) return false;
        switch (value->type())
        {
            case NodeType::INTEGER_LITERAL:
            case NodeType::STRING_LITERAL:
            case NodeType::BOOLEAN_LITERAL:
                return true;
            default:
                return false;
        }
    }

    void record(const std::string& name, bool nonFloat)
    {
        if (nonFloat && !maybeFloat.count(name))
        {
            neverFloat.insert(name);
        }
        else if (!nonFloat)
        {
            maybeFloat.insert(name);
            neverFloat.erase(name);
        }
    }
};
} // namespace

void NoSelfComparisonRule::analyze(Program* program)
{
    if (!program) return;

    // 먼저 실수가 될 수 없는 이름을 모은다 (범위는 구분하지 않으므로 같은 이름이 실수를 받은 적이 있으면 제외)
    FloatTracker tracker;
    for (const auto& stmt : program->statements())
    {
        ast::walk(stmt.get(), tracker);
    }
    neverFloat_ = std::move(tracker.neverFloat);

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
    if (!isSameIdentifier(expr->left(), expr->right()))
    {
        return;
    }

    // <, > 는 값과 상관없이 늘 거짓이다. ==, !=, <=, >= 는 값이 NaN 이면 결과가 달라진다:
    // x != x 는 실수가 NaN 인지 확인하는 관용구이므로 실수가 될 수 없는 이름에서만 알린다
    bool strict = expr->op() == "<" || expr->op() == ">";
    const std::string& name = node_cast<Identifier>(expr->left())->name();
    if (strict || neverFloat_.count(name))
    {
        reportIssue(
            "변수를 자기 자신과 비교하고 있습니다 (항상 같은 결과)",
            IssueSeverity::WARNING,
            expr->location().line,
            expr->location().column
        );
    }
}

bool NoSelfComparisonRule::isSameIdentifier(const Expression* left, const Expression* right)
{
    if (!left || !right) return false;

    // 둘 다 Identifier인지 확인
    auto leftId = node_cast<Identifier>(left);
    auto rightId = node_cast<Identifier>(right);

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
