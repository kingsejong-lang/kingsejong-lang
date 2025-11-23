/**
 * @file NamingConventionRule.cpp
 * @brief 네이밍 컨벤션 규칙 구현
 * @author KingSejong Team
 * @date 2025-11-23
 */

#include "NamingConventionRule.h"
#include "linter/Linter.h"
#include "linter/IssueSeverity.h"
#include <cctype>
#include <algorithm>

namespace kingsejong {
namespace linter {
namespace rules {

void NamingConventionRule::analyze(ast::Program* program)
{
    // AST 순회하여 모든 선언 검사
    for (auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }
}

void NamingConventionRule::onVarDeclaration(ast::VarDeclaration* stmt)
{
    const std::string& name = stmt->varName();

    // 한글 이름은 스킵 (한글 네이밍은 자유롭게 허용)
    if (containsKorean(name))
    {
        return;
    }

    // 상수 변수는 UPPER_SNAKE_CASE 권장
    if (isConstant(name))
    {
        if (!isUpperSnakeCase(name))
        {
            reportIssue(
                "상수 변수 '" + name + "'는 UPPER_SNAKE_CASE를 권장합니다 (예: MY_CONSTANT)",
                IssueSeverity::WARNING
            );
        }
        return;
    }

    // 일반 변수는 snake_case 또는 camelCase 권장
    if (!isSnakeCase(name) && !isCamelCase(name))
    {
        reportIssue(
            "변수명 '" + name + "'는 snake_case 또는 camelCase를 권장합니다 (예: my_variable, myVariable)",
            IssueSeverity::WARNING
        );
    }
}

bool NamingConventionRule::containsKorean(const std::string& name) const
{
    // UTF-8 한글 범위: 0xAC00 ~ 0xD7A3
    for (size_t i = 0; i < name.length(); )
    {
        unsigned char c = static_cast<unsigned char>(name[i]);

        // UTF-8 3바이트 문자 (한글)
        if ((c & 0xF0) == 0xE0 && i + 2 < name.length())
        {
            unsigned char c1 = static_cast<unsigned char>(name[i + 1]);
            unsigned char c2 = static_cast<unsigned char>(name[i + 2]);

            // 한글 범위 체크
            uint32_t codepoint = ((c & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
            if (codepoint >= 0xAC00 && codepoint <= 0xD7A3)
            {
                return true;
            }
            i += 3;
        }
        else if ((c & 0x80) == 0)
        {
            // ASCII 문자
            i++;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            // UTF-8 2바이트
            i += 2;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            // UTF-8 4바이트
            i += 4;
        }
        else
        {
            i++;
        }
    }

    return false;
}

bool NamingConventionRule::isSnakeCase(const std::string& name) const
{
    if (name.empty())
        return false;

    // 첫 문자는 소문자 또는 언더스코어
    if (!std::islower(static_cast<unsigned char>(name[0])) && name[0] != '_')
        return false;

    // 나머지는 소문자, 숫자, 언더스코어만 허용
    for (char c : name)
    {
        if (!std::islower(static_cast<unsigned char>(c)) &&
            !std::isdigit(static_cast<unsigned char>(c)) &&
            c != '_')
        {
            return false;
        }
    }

    // 연속된 언더스코어는 비권장 (하지만 허용)
    return true;
}

bool NamingConventionRule::isPascalCase(const std::string& name) const
{
    if (name.empty())
        return false;

    // 첫 문자는 대문자
    if (!std::isupper(static_cast<unsigned char>(name[0])))
        return false;

    // 나머지는 알파벳과 숫자만 허용 (언더스코어 없음)
    for (char c : name)
    {
        if (!std::isalnum(static_cast<unsigned char>(c)))
            return false;
    }

    return true;
}

bool NamingConventionRule::isUpperSnakeCase(const std::string& name) const
{
    if (name.empty())
        return false;

    // 모두 대문자, 숫자, 언더스코어만 허용
    for (char c : name)
    {
        if (!std::isupper(static_cast<unsigned char>(c)) &&
            !std::isdigit(static_cast<unsigned char>(c)) &&
            c != '_')
        {
            return false;
        }
    }

    // 적어도 하나의 대문자가 있어야 함
    return std::any_of(name.begin(), name.end(), [](char c) {
        return std::isupper(static_cast<unsigned char>(c));
    });
}

bool NamingConventionRule::isCamelCase(const std::string& name) const
{
    if (name.empty())
        return false;

    // 첫 문자는 소문자
    if (!std::islower(static_cast<unsigned char>(name[0])))
        return false;

    // 나머지는 알파벳과 숫자만 허용 (언더스코어 없음)
    // 적어도 하나의 대문자가 있어야 camelCase
    bool hasUpperCase = false;
    for (size_t i = 1; i < name.length(); i++)
    {
        char c = name[i];
        if (!std::isalnum(static_cast<unsigned char>(c)))
            return false;
        if (std::isupper(static_cast<unsigned char>(c)))
            hasUpperCase = true;
    }

    return hasUpperCase;
}

bool NamingConventionRule::isConstant(const std::string& name) const
{
    // 상수 판별 휴리스틱:
    // 1. 모두 대문자로 시작
    // 2. UPPER_SNAKE_CASE 형식
    if (name.empty())
        return false;

    // 모든 문자가 대문자, 숫자, 언더스코어인 경우 상수로 간주
    return isUpperSnakeCase(name);
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
