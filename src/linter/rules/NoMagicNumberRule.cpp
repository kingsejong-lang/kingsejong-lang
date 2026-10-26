/**
 * @file NoMagicNumberRule.cpp
 * @brief NoMagicNumberRule 구현
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "linter/rules/NoMagicNumberRule.h"
#include "linter/Linter.h"
#include "lexer/JosaRecognizer.h"

namespace kingsejong {
namespace linter {
namespace rules {

using namespace ast;

void NoMagicNumberRule::analyze(Program* program)
{
    if (!program) return;

    // 초기화
    inVarInitializer_ = false;
    inArrayLiteral_ = false;

    // 모든 문장 순회
    for (const auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }
}

void NoMagicNumberRule::onIntegerLiteral(IntegerLiteral* expr)
{
    if (!expr) return;

    int64_t value = expr->value();

    // 허용되는 숫자인지 확인
    if (allowedNumbers_.find(value) != allowedNumbers_.end())
    {
        return;  // 허용된 숫자는 검사하지 않음
    }

    // 변수 초기화나 배열 리터럴에서는 허용
    if (inVarInitializer_ || inArrayLiteral_)
    {
        return;
    }

    // 매직 넘버 경고
    std::string text = std::to_string(value);
    reportIssue(
        "매직 넘버 " + text + lexer::JosaRecognizer::selectOrBoth(text, lexer::JosaRecognizer::JosaType::EUL_REUL) +
        " 사용합니다. "
        "상수로 정의하여 의미를 명확히 하세요",
        IssueSeverity::HINT,
        expr->location().line,
        expr->location().column
    );
}

void NoMagicNumberRule::onFloatLiteral(FloatLiteral* expr)
{
    if (!expr) return;

    // 변수 초기화나 배열 리터럴에서는 허용
    if (inVarInitializer_ || inArrayLiteral_)
    {
        return;
    }

    // 실수 매직 넘버 경고
    std::string text = std::to_string(expr->value());
    reportIssue(
        "매직 넘버 " + text + lexer::JosaRecognizer::selectOrBoth(text, lexer::JosaRecognizer::JosaType::EUL_REUL) +
        " 사용합니다. "
        "상수로 정의하여 의미를 명확히 하세요",
        IssueSeverity::HINT,
        expr->location().line,
        expr->location().column
    );
}

void NoMagicNumberRule::onVarDeclaration(VarDeclaration* stmt)
{
    if (!stmt) return;

    // 초기식은 순회가 이어서 걷는다. 그동안 플래그를 켜 둔다.
    inVarInitializer_ = true;
}

void NoMagicNumberRule::onVarDeclarationEnd(VarDeclaration*)
{
    inVarInitializer_ = false;
}

void NoMagicNumberRule::onArrayLiteral(ArrayLiteral* expr)
{
    if (!expr) return;

    // 요소는 순회가 이어서 걷는다. 중첩 배열을 위해 깊이를 센다.
    ++arrayDepth_;
    inArrayLiteral_ = true;
}

void NoMagicNumberRule::onArrayLiteralEnd(ArrayLiteral*)
{
    --arrayDepth_;
    inArrayLiteral_ = arrayDepth_ > 0;
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
