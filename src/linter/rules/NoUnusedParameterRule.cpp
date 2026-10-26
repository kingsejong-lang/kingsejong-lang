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

// Helper: 함수 내에서 사용된 식별자 수집
/**
 * @brief 함수 본문에서 쓰인 식별자 이름을 모은다
 *
 * 중첩 함수 안은 들어가지 않는다 (예전 동작 유지: 안쪽 함수가 바깥 매개변수를 쓰는지는
 * 클로저 문제라 이 규칙이 다루지 않는다).
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
        }
        return node.type() != ast::NodeType::FUNCTION_LITERAL &&
               node.type() != ast::NodeType::ASYNC_FUNCTION_LITERAL;
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
                "함수 매개변수 '" + param + "'" + lexer::JosaRecognizer::selectOrBoth(param, lexer::JosaRecognizer::JosaType::I_GA) + " 사용되지 않습니다. "
                "의도적인 경우 '_" + param + "'" + lexer::JosaRecognizer::selectOrBoth(param, lexer::JosaRecognizer::JosaType::RO_EURO) + " 표시하세요",
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
