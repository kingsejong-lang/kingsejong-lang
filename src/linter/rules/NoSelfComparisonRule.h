#pragma once

/**
 * @file NoSelfComparisonRule.h
 * @brief 자기 자신과의 비교 감지 규칙
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/Rule.h"
#include <unordered_set>

namespace kingsejong {
namespace linter {
namespace rules {

/**
 * @class NoSelfComparisonRule
 * @brief 자기 자신과 비교하는 의미 없는 코드 감지 (x == x, y < y 등)
 */
class NoSelfComparisonRule : public Rule
{
public:
    NoSelfComparisonRule()
        : Rule("no-self-comparison", "자기 자신과의 비교 감지")
    {
    }

    void analyze(ast::Program* program) override;

protected:
    void onBinaryExpression(ast::BinaryExpression* expr) override;

private:
    /**
     * @brief 두 표현식이 동일한 식별자인지 확인
     */
    bool isSameIdentifier(const ast::Expression* left, const ast::Expression* right);

    /**
     * @brief 연산자가 비교 연산자인지 확인
     */
    bool isComparisonOperator(const std::string& op);

    /// 실수가 될 수 없는 이름 (정수·문자열·논리 선언, 또는 정수·문자열·논리 리터럴만 대입). 이 이름만 ==, != 등을 알린다
    std::unordered_set<std::string> neverFloat_;
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
