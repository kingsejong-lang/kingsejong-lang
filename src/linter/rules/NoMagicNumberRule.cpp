/**
 * @file NoMagicNumberRule.cpp
 * @brief NoMagicNumberRule 구현
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "linter/rules/NoMagicNumberRule.h"
#include "linter/Linter.h"

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
    reportIssue(
        "매직 넘버 " + std::to_string(value) + "을(를) 사용합니다. "
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
    reportIssue(
        "매직 넘버 " + std::to_string(expr->value()) + "을(를) 사용합니다. "
        "상수로 정의하여 의미를 명확히 하세요",
        IssueSeverity::HINT,
        expr->location().line,
        expr->location().column
    );
}

void NoMagicNumberRule::onVarDeclaration(VarDeclaration* stmt)
{
    if (!stmt) return;

    // 변수 초기화 플래그 설정
    inVarInitializer_ = true;

    // 초기화 표현식 순회
    if (stmt->initializer())
    {
        visitExpression(const_cast<Expression*>(stmt->initializer()));
    }

    // 플래그 해제
    inVarInitializer_ = false;
}

void NoMagicNumberRule::onArrayLiteral(ArrayLiteral* expr)
{
    if (!expr) return;

    // 배열 리터럴 플래그 설정
    bool wasInArray = inArrayLiteral_;
    inArrayLiteral_ = true;

    // 배열 요소 순회
    for (const auto& elem : expr->elements())
    {
        visitExpression(elem.get());
    }

    // 플래그 복원
    inArrayLiteral_ = wasInArray;
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
