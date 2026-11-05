/**
 * @file NoUnusedParameterRule.cpp
 * @brief NoUnusedParameterRule 구현
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "linter/rules/NoUnusedParameterRule.h"
#include "ast/Visitor.h"
#include "linter/Linter.h"
#include "lexer/JosaRecognizer.h"
#include <unordered_map>
#include <unordered_set>

namespace kingsejong {
namespace linter {
namespace rules {

using namespace ast;

/**
 * @brief 함수 본문에서 쓰인 식별자 이름을 모은다
 *
 * 중첩 함수 안도 들어간다. 안쪽 함수가 바깥 매개변수를 쓰는 클로저("반환 함수(x) { 반환 x * n }")에서
 * 바깥 매개변수는 쓰인 것이다. 다만 안쪽 함수가 같은 이름의 매개변수를 다시 선언하면 안쪽의 그 이름은
 * 안쪽 매개변수를 가리키므로 바깥 쪽 사용으로 세지 않는다.
 */
class IdentifierCollector : public ast::Visitor
{
public:
    std::unordered_set<std::string> identifiers;

    void collect(const Statement* stmt)
    {
        ast::walk(const_cast<Statement*>(stmt), *this);
    }

    bool enter(ast::Node& node) override
    {
        if (node.type() == ast::NodeType::IDENTIFIER)
        {
            identifiers.insert(static_cast<const Identifier&>(node).name());
            return true;
        }
        if (node.type() == ast::NodeType::FUNCTION_LITERAL)
        {
            const auto& fn = static_cast<const FunctionLiteral&>(node);
            collectNested(fn.parameters(), fn.body());
            return false;
        }
        if (node.type() == ast::NodeType::ASYNC_FUNCTION_LITERAL)
        {
            const auto& fn = static_cast<const AsyncFunctionLiteral&>(node);
            collectNested(fn.parameters(), fn.body());
            return false;
        }
        return true;
    }

private:
    void collectNested(const std::vector<std::string>& params, const Statement* body)
    {
        if (!body)
        {
            return;
        }
        IdentifierCollector inner;
        inner.collect(body);
        std::unordered_set<std::string> shadowed(params.begin(), params.end());
        for (const auto& name : inner.identifiers)
        {
            if (shadowed.find(name) == shadowed.end())
            {
                identifiers.insert(name);
            }
        }
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

    // 미사용 매개변수 검사 ('_' 로 시작하는 이름은 문법상 만들 수 없으므로 이름으로 의도를 표시하는 관례는 없다)
    for (const std::string& param : expr->parameters())
    {
        // 사용되지 않았으면 경고
        if (collector.identifiers.find(param) == collector.identifiers.end())
        {
            reportIssue(
                "함수 매개변수 '" + param + "'" + lexer::JosaRecognizer::selectOrBoth(param, lexer::JosaRecognizer::JosaType::I_GA) + " 사용되지 않습니다. "
                "다른 함수와 모양을 맞추려고 둔 매개변수라면 .ksjlint.json 에서 no-unused-parameter 를 끌 수 있습니다",
                IssueSeverity::WARNING,
                expr->location().line,
                expr->location().column,
                param
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
