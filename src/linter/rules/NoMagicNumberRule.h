#pragma once

/**
 * @file NoMagicNumberRule.h
 * @brief 매직 넘버 감지 규칙
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "linter/Rule.h"
#include <unordered_set>

namespace kingsejong {
namespace linter {
namespace rules {

/**
 * @class NoMagicNumberRule
 * @brief 코드에 하드코딩된 매직 넘버 감지
 *
 * 매직 넘버: 코드에 직접 작성된 숫자 리터럴로, 의미가 명확하지 않아 가독성을 해침
 * 예외: 0, 1, -1, 2 등 일반적으로 사용되는 숫자는 허용
 */
class NoMagicNumberRule : public Rule
{
public:
    NoMagicNumberRule()
        : Rule("no-magic-number", "하드코딩된 매직 넘버 감지")
    {
        // 일반적으로 허용되는 숫자들
        allowedNumbers_.insert(0);
        allowedNumbers_.insert(1);
        allowedNumbers_.insert(-1);
        allowedNumbers_.insert(2);
    }

    void analyze(ast::Program* program) override;

protected:
    void onIntegerLiteral(ast::IntegerLiteral* expr) override;
    void onFloatLiteral(ast::FloatLiteral* expr) override;
    void onVarDeclaration(ast::VarDeclaration* stmt) override;
    void onArrayLiteral(ast::ArrayLiteral* expr) override;

private:
    std::unordered_set<int64_t> allowedNumbers_;  // 허용되는 숫자 목록
    bool inVarInitializer_ = false;  // 변수 초기화 중인지
    bool inArrayLiteral_ = false;    // 배열 리터럴 내부인지
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
