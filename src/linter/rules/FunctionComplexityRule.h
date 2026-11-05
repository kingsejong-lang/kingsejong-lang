#pragma once

/**
 * @file FunctionComplexityRule.h
 * @brief 함수 복잡도 검사 규칙
 * @author KingSejong Team
 * @date 2025-11-23
 */

#include "linter/Rule.h"
#include <string>

namespace kingsejong {
namespace linter {
namespace rules {

/**
 * @class FunctionComplexityRule
 * @brief 함수의 순환 복잡도(Cyclomatic Complexity) 검사
 *
 * 순환 복잡도 계산:
 * - 기본값: 1
 * - 조건문 (if, while, for 등): +1
 * - 논리 연산자 (&&, ||): +1
 * - catch 절: +1
 *
 * 권장 기준:
 * - 1-10: 간단, 테스트 용이
 * - 11-20: 복잡, 리팩토링 권장
 * - 21+: 매우 복잡, 반드시 리팩토링 필요
 */
class FunctionComplexityRule : public Rule
{
public:
    /// @brief FunctionComplexityRule 생성자
    /// @param maxComplexity 최대 허용 복잡도 (기본값: 10)
    explicit FunctionComplexityRule(int maxComplexity = 10)
        : Rule("function-complexity", "함수 순환 복잡도 검사"),
          maxComplexity_(maxComplexity)
    {
    }

    /**
     * @brief 프로그램 분석 실행
     * @param program 분석할 AST 프로그램
     */
    void analyze(ast::Program* program) override;

private:
    class Walker;  ///< 함수마다 복잡도를 따로 세는 순회

    /// 함수 하나의 복잡도를 다 세었을 때 기준을 넘으면 알린다
    void finishFunction(const std::string& name, int complexity, const ast::SourceLocation& location);

    int maxComplexity_;  ///< 최대 허용 복잡도
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
