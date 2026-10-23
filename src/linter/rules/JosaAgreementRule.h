/**
 * @file JosaAgreementRule.h
 * @brief 조사가 앞 명사의 받침과 맞는지 검사하는 규칙 (josa-agreement)
 *
 * 파서는 을/를, 이/가, 은/는, 로/으로를 한 타입으로 접어서 받아들이므로 "배열를"도
 * 실행된다. 글처럼 읽히는 코드가 목표인 언어에서 이것은 경고로 알려 주어야 한다.
 */
#pragma once

#include "linter/Rule.h"

namespace kingsejong {
namespace linter {
namespace rules {

class JosaAgreementRule : public Rule
{
public:
    JosaAgreementRule()
        : Rule("josa-agreement", "조사와 앞 명사의 받침 일치 검사")
    {
    }

    void analyze(ast::Program* program) override;

protected:
    void onJosaExpression(ast::JosaExpression* expr) override;
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
