#pragma once

/**
 * @file NamingConventionRule.h
 * @brief 네이밍 컨벤션 규칙
 * @author KingSejong Team
 * @date 2025-11-23
 */

#include "linter/Rule.h"
#include <string>
#include <regex>

namespace kingsejong {
namespace linter {
namespace rules {

/**
 * @class NamingConventionRule
 * @brief 변수, 함수, 클래스 네이밍 컨벤션 검사
 *
 * 규칙:
 * - 변수명: snake_case 권장 (한글 허용)
 * - 함수명: snake_case 권장 (한글 허용)
 * - 클래스명: PascalCase 권장 (한글 허용)
 * - 상수명: UPPER_SNAKE_CASE 권장
 */
class NamingConventionRule : public Rule
{
public:
    /// @brief NamingConventionRule 생성자
    NamingConventionRule()
        : Rule("naming-convention", "변수, 함수, 클래스 네이밍 컨벤션 검사")
    {
    }

    /**
     * @brief 프로그램 분석 실행
     * @param program 분석할 AST 프로그램
     */
    void analyze(ast::Program* program) override;

protected:
    /// @brief 변수 선언 검사
    void onVarDeclaration(ast::VarDeclaration* stmt) override;

private:
    /**
     * @brief 이름이 한글을 포함하는지 검사
     */
    bool containsKorean(const std::string& name) const;

    /**
     * @brief snake_case 형식인지 검사
     */
    bool isSnakeCase(const std::string& name) const;

    /**
     * @brief PascalCase 형식인지 검사
     */
    bool isPascalCase(const std::string& name) const;

    /**
     * @brief UPPER_SNAKE_CASE 형식인지 검사
     */
    bool isUpperSnakeCase(const std::string& name) const;

    /**
     * @brief camelCase 형식인지 검사
     */
    bool isCamelCase(const std::string& name) const;

    /**
     * @brief 상수 변수인지 검사 (추후 타입 정보 활용 가능)
     */
    bool isConstant(const std::string& name) const;
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
