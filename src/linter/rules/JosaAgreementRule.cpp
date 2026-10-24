/**
 * @file JosaAgreementRule.cpp
 * @brief 조사 받침 일치 검사 구현
 */
#include "linter/rules/JosaAgreementRule.h"
#include "linter/Linter.h"
#include "lexer/JosaRecognizer.h"

namespace kingsejong {
namespace linter {
namespace rules {

using namespace ast;

namespace {

/// 마지막 글자가 완성형 한글 음절(U+AC00..U+D7A3)인가. UTF-8 선두 바이트 0xEA..0xED 로 판단한다.
bool endsWithHangulSyllable(const std::string& word)
{
    if (word.size() < 3)
    {
        return false;
    }
    unsigned char lead = static_cast<unsigned char>(word[word.size() - 3]);
    unsigned char second = static_cast<unsigned char>(word[word.size() - 2]);
    if (lead < 0xEA || lead > 0xED)
    {
        return false;
    }
    // U+AC00 = EA B0 80, U+D7A3 = ED 9E A3
    if (lead == 0xEA && second < 0xB0) return false;
    if (lead == 0xED && second > 0x9E) return false;
    return true;
}

} // namespace

void JosaAgreementRule::analyze(Program* program)
{
    if (!program) return;
    for (const auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }
}

void JosaAgreementRule::onJosaExpression(JosaExpression* expr)
{
    if (!expr || expr->particle().empty()) return;

    auto ident = node_cast<Identifier>(expr->object());
    if (!ident) return;  // 식(호출 결과 등) 뒤의 조사는 명사를 알 수 없다

    using JosaType = lexer::JosaRecognizer::JosaType;
    JosaType type = expr->josaType();
    if (type != JosaType::EUL_REUL && type != JosaType::I_GA &&
        type != JosaType::EUN_NEUN && type != JosaType::RO_EURO)
    {
        return;  // 의/에서/에 는 받침과 무관하다
    }

    const std::string& noun = ident->name();
    if (!endsWithHangulSyllable(noun))
    {
        return;  // 영문·숫자로 끝나는 이름은 받침을 판단할 수 없다
    }

    std::string expected = lexer::JosaRecognizer::select(noun, type);
    if (expected != expr->particle())
    {
        reportIssue(
            "조사가 받침과 맞지 않습니다: '" + noun + expr->particle() + "' → '" + noun + expected + "'",
            IssueSeverity::WARNING,
            expr->location().line,
            expr->location().column
        );
    }
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
