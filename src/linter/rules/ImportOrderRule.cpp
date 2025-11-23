/**
 * @file ImportOrderRule.cpp
 * @brief 임포트 순서 정리 규칙 구현
 * @author KingSejong Team
 * @date 2025-11-23
 */

#include "ImportOrderRule.h"
#include "linter/Linter.h"
#include "linter/IssueSeverity.h"
#include <algorithm>

namespace kingsejong {
namespace linter {
namespace rules {

void ImportOrderRule::analyze(ast::Program* program)
{
    // 1단계: 모든 import 수집
    imports_.clear();

    // 2단계: AST 순회하여 import 문 수집
    for (auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }

    // 3단계: import 순서 검사
    checkImportOrder();
}

void ImportOrderRule::onImportStatement(ast::ImportStatement* stmt)
{
    std::string modulePath = stmt->modulePath();
    bool isStdlib = isStdlibImport(modulePath);

    imports_.push_back(ImportInfo{
        modulePath,
        0,  // TODO: 실제 줄 번호가 AST에 추가되면 사용
        0,  // TODO: 실제 열 번호가 AST에 추가되면 사용
        isStdlib
    });
}

bool ImportOrderRule::isStdlibImport(const std::string& modulePath) const
{
    // "stdlib/"로 시작하면 stdlib 임포트
    return modulePath.find("stdlib/") == 0;
}

void ImportOrderRule::checkImportOrder()
{
    if (imports_.empty())
    {
        return;
    }

    // stdlib 임포트와 사용자 임포트 분리
    std::vector<ImportInfo> stdlibImports;
    std::vector<ImportInfo> userImports;

    for (const auto& import : imports_)
    {
        if (import.isStdlib)
        {
            stdlibImports.push_back(import);
        }
        else
        {
            userImports.push_back(import);
        }
    }

    // 검사 1: stdlib 임포트가 사용자 임포트보다 먼저 와야 함
    bool foundUser = false;
    for (const auto& import : imports_)
    {
        if (!import.isStdlib)
        {
            foundUser = true;
        }
        else if (foundUser)
        {
            // stdlib 임포트가 사용자 임포트 뒤에 등장
            reportIssue(
                "stdlib 임포트는 사용자 정의 임포트보다 먼저 선언되어야 합니다. "
                "임포트 ''" + import.modulePath + "''의 위치를 확인하세요.",
                IssueSeverity::WARNING,
                import.line,
                import.column
            );
            break;
        }
    }

    // 검사 2: stdlib 임포트 그룹 내 알파벳 순 검사
    if (!stdlibImports.empty())
    {
        for (size_t i = 1; i < stdlibImports.size(); ++i)
        {
            if (stdlibImports[i].modulePath < stdlibImports[i - 1].modulePath)
            {
                reportIssue(
                    "stdlib 임포트가 알파벳 순으로 정렬되지 않았습니다. "
                    "''" + stdlibImports[i].modulePath + "''는 ''" +
                    stdlibImports[i - 1].modulePath + "'' 앞에 와야 합니다.",
                    IssueSeverity::WARNING,
                    stdlibImports[i].line,
                    stdlibImports[i].column
                );
            }
        }
    }

    // 검사 3: 사용자 임포트 그룹 내 알파벳 순 검사
    if (!userImports.empty())
    {
        for (size_t i = 1; i < userImports.size(); ++i)
        {
            if (userImports[i].modulePath < userImports[i - 1].modulePath)
            {
                reportIssue(
                    "사용자 정의 임포트가 알파벳 순으로 정렬되지 않았습니다. "
                    "''" + userImports[i].modulePath + "''는 ''" +
                    userImports[i - 1].modulePath + "'' 앞에 와야 합니다.",
                    IssueSeverity::WARNING,
                    userImports[i].line,
                    userImports[i].column
                );
            }
        }
    }
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
