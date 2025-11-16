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
    NoShadowingRule()
        : Rule("no-shadowing", "외부 스코프 변수를 가리는 섀도잉 감지")
    {
    }

    void analyze(ast::Program* program) override;

protected:
    void onVarDeclaration(ast::VarDeclaration* stmt) override;
    void onFunctionLiteral(ast::FunctionLiteral* expr) override;
    void onBlockStatement(ast::BlockStatement* stmt) override;
    void onRangeForStatement(ast::RangeForStatement* stmt) override;

private:
    // 스코프별 변수 정보 저장
    struct Scope {
        std::unordered_map<std::string, ast::VarDeclaration*> variables;
        std::vector<std::string> parameters;  // 함수 매개변수
    };

    std::vector<Scope> scopeStack_;  // 스코프 스택

    // 현재 스코프에 변수 추가
    void addVariable(const std::string& name, ast::VarDeclaration* decl);

    // 외부 스코프에 동일 이름 변수가 있는지 확인
    ast::VarDeclaration* findInOuterScopes(const std::string& name);

    // 외부 스코프에 동일 이름 매개변수가 있는지 확인
    bool isParameterInOuterScopes(const std::string& name);

    // 새 스코프 시작
    void pushScope();

    // 스코프 종료
    void popScope();
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
