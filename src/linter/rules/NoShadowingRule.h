#pragma once

/**
 * @file NoShadowingRule.h
 * @brief 변수 섀도잉 감지 규칙
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "linter/Rule.h"
#include <unordered_map>
#include <vector>
#include <string>

namespace kingsejong {
namespace linter {
namespace rules {

/**
 * @class NoShadowingRule
 * @brief 외부 스코프의 변수를 같은 이름으로 가리는 변수 섀도잉 감지
 */
class NoShadowingRule : public Rule
{
public:
    /// @brief NoShadowingRule 생성자
    NoShadowingRule()
        : Rule("no-shadowing", "외부 스코프 변수를 가리는 섀도잉 감지")
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

    /// @brief 함수 리터럴 처리 (새 스코프 생성)
    void onFunctionLiteral(ast::FunctionLiteral* expr) override;

    /// @brief 블록문 처리 (새 스코프 생성)
    void onBlockStatement(ast::BlockStatement* stmt) override;

    /// @brief 범위 for문 처리
    void onRangeForStatement(ast::RangeForStatement* stmt) override;

private:
    /// @brief 스코프별 변수 정보
    struct Scope {
        std::unordered_map<std::string, ast::VarDeclaration*> variables;  ///< 변수 맵
        std::vector<std::string> parameters;  ///< 함수 매개변수
    };

    std::vector<Scope> scopeStack_;  ///< 스코프 스택

    /// @brief 현재 스코프에 변수 추가
    void addVariable(const std::string& name, ast::VarDeclaration* decl);

    /// @brief 외부 스코프에서 동일 이름 변수 검색
    ast::VarDeclaration* findInOuterScopes(const std::string& name);

    /// @brief 외부 스코프에 동일 이름 매개변수 존재 확인
    bool isParameterInOuterScopes(const std::string& name);

    /// @brief 새 스코프 시작
    void pushScope();

    /// @brief 현재 스코프 종료
    void popScope();
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
