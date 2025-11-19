#pragma once

/**
 * @file NoUnusedParameterRule.h
 * @brief 미사용 함수 매개변수 감지 규칙
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "linter/Rule.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

namespace kingsejong {
namespace linter {
namespace rules {

/**
 * @class NoUnusedParameterRule
 * @brief 함수 매개변수가 선언되었지만 사용되지 않는 경우 감지
 */
class NoUnusedParameterRule : public Rule
{
public:
    /// @brief NoUnusedParameterRule 생성자
    NoUnusedParameterRule()
        : Rule("no-unused-parameter", "사용되지 않는 함수 매개변수 감지")
    {
    }

    /**
     * @brief 프로그램 분석 실행
     * @param program 분석할 AST 프로그램
     */
    void analyze(ast::Program* program) override;

protected:
    /// @brief 함수 리터럴 처리 (매개변수 수집)
    void onFunctionLiteral(ast::FunctionLiteral* expr) override;

    /// @brief 식별자 사용 추적
    void onIdentifier(ast::Identifier* expr) override;

private:
    /// @brief 함수별 매개변수 사용 정보
    struct FunctionContext {
        std::vector<std::string> parameters;  ///< 매개변수 목록
        std::unordered_set<std::string> usedParams;  ///< 사용된 매개변수
        ast::FunctionLiteral* funcLit;  ///< 함수 리터럴 포인터
    };

    std::vector<FunctionContext> functionStack_;  ///< 중첩 함수 스택
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
