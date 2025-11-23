/**
 * @file FunctionComplexityRule.cpp
 * @brief 함수 복잡도 검사 규칙 구현
 * @author KingSejong Team
 * @date 2025-11-23
 */

#include "FunctionComplexityRule.h"
#include "linter/Linter.h"
#include "linter/IssueSeverity.h"
#include "lexer/Token.h"
#include <sstream>

namespace kingsejong {
namespace linter {
namespace rules {

void FunctionComplexityRule::analyze(ast::Program* program)
{
    // AST 순회하여 모든 함수 검사
    for (auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }
}

void FunctionComplexityRule::onFunctionLiteral(ast::FunctionLiteral* expr)
{
    // 익명 함수도 복잡도 검사
    currentFunctionName_ = "<익명 함수>";
    currentComplexity_ = 0;

    // 함수 본문의 복잡도 계산
    if (expr->body())
    {
        int complexity = calculateComplexity(expr->body());

        // 복잡도가 기준을 초과하면 경고
        if (complexity > maxComplexity_)
        {
            std::ostringstream oss;
            oss << "함수 '" << currentFunctionName_
                << "'의 순환 복잡도가 " << complexity
                << "입니다 (최대 권장: " << maxComplexity_ << "). "
                << "함수를 더 작은 단위로 분해하는 것을 고려하세요.";

            IssueSeverity severity = IssueSeverity::WARNING;
            if (complexity > 20)
            {
                severity = IssueSeverity::ERROR;
                oss << " (매우 복잡함)";
            }

            reportIssue(oss.str(), severity);
        }
    }

    currentFunctionName_.clear();
    currentComplexity_ = 0;
}

void FunctionComplexityRule::onIfStatement(ast::IfStatement* /* stmt */)
{
    // If 문은 복잡도 +1
    currentComplexity_++;
}

void FunctionComplexityRule::onWhileStatement(ast::WhileStatement* /* stmt */)
{
    // While 문은 복잡도 +1
    currentComplexity_++;
}

void FunctionComplexityRule::onRangeForStatement(ast::RangeForStatement* /* stmt */)
{
    // For 문은 복잡도 +1
    currentComplexity_++;
}

void FunctionComplexityRule::onRepeatStatement(ast::RepeatStatement* /* stmt */)
{
    // Repeat 문은 복잡도 +1
    currentComplexity_++;
}

void FunctionComplexityRule::onBinaryExpression(ast::BinaryExpression* expr)
{
    // 논리 연산자 (&&, ||)는 복잡도 +1
    const std::string& op = expr->op();
    if (op == "&&" || op == "||")
    {
        currentComplexity_++;
    }
}

int FunctionComplexityRule::calculateComplexity(ast::Statement* body)
{
    // 초기 복잡도는 1
    currentComplexity_ = 1;

    // AST 순회하여 복잡도 계산
    visitStatement(body);

    return currentComplexity_;
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
