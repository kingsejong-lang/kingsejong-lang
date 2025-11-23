/**
 * @file UnusedImportRule.cpp
 * @brief 사용하지 않는 임포트 검출 규칙 구현
 * @author KingSejong Team
 * @date 2025-11-23
 */

#include "UnusedImportRule.h"
#include "linter/Linter.h"
#include "linter/IssueSeverity.h"

namespace kingsejong {
namespace linter {
namespace rules {

void UnusedImportRule::analyze(ast::Program* program)
{
    // 1단계: 모든 import 수집
    importedModules_.clear();
    usedModules_.clear();

    // 2단계: AST 순회
    for (auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }

    // 3단계: 사용되지 않은 import 보고
    for (const auto& [moduleName, importInfo] : importedModules_)
    {
        if (usedModules_.find(moduleName) == usedModules_.end())
        {
            reportIssue(
                "import된 모듈 '" + importInfo.modulePath + "'이(가) 사용되지 않습니다",
                IssueSeverity::WARNING,
                importInfo.line,
                importInfo.column
            );
        }
    }
}

void UnusedImportRule::onImportStatement(ast::ImportStatement* stmt)
{
    std::string moduleName = extractModuleName(stmt->modulePath());
    importedModules_[moduleName] = ImportInfo{stmt->modulePath(), 0, 0};
}

void UnusedImportRule::onCallExpression(ast::CallExpression* expr)
{
    // 함수명 추출
    if (auto funcIdent = dynamic_cast<const ast::Identifier*>(expr->function()))
    {
        std::string functionName = funcIdent->name();
        std::string moduleName = guessModuleFromFunction(functionName);

        if (!moduleName.empty() && importedModules_.find(moduleName) != importedModules_.end())
        {
            usedModules_.insert(moduleName);
        }
    }

    // 하위 표현식 순회 (기본 동작)
    visitExpression(const_cast<ast::Expression*>(expr->function()));
    for (const auto& arg : expr->arguments())
    {
        visitExpression(const_cast<ast::Expression*>(arg.get()));
    }
}

std::string UnusedImportRule::extractModuleName(const std::string& modulePath) const
{
    // "stdlib/math" -> "math"
    // "utils/helper" -> "helper"
    size_t lastSlash = modulePath.find_last_of('/');
    if (lastSlash != std::string::npos)
    {
        return modulePath.substr(lastSlash + 1);
    }
    return modulePath;
}

std::string UnusedImportRule::guessModuleFromFunction(const std::string& functionName) const
{
    // stdlib 함수 이름 기반 모듈 추정 (간단한 휴리스틱)
    // 추후 더 정교한 symbol table 기반으로 개선 가능

    // math 모듈 함수들
    if (functionName == "절댓값" || functionName == "최댓값" || functionName == "최솟값" ||
        functionName == "거듭제곱" || functionName == "제곱근" || functionName == "계승" ||
        functionName == "최대공약수" || functionName == "최소공배수" || functionName == "소수인가")
    {
        return "math";
    }

    // json 모듈 함수들
    if (functionName == "JSON_파싱" || functionName == "JSON_문자열화" ||
        functionName == "JSON_파일_읽기" || functionName == "JSON_파일_쓰기")
    {
        return "json";
    }

    // time 모듈 함수들
    if (functionName == "현재_타임스탬프" || functionName == "날짜_포맷팅" ||
        functionName == "슬립" || functionName == "밀리초_슬립")
    {
        return "time";
    }

    // io 모듈 함수들
    if (functionName == "파일_읽기" || functionName == "파일_쓰기" ||
        functionName == "파일_존재" || functionName == "파일_삭제")
    {
        return "io";
    }

    // http 모듈 함수들
    if (functionName == "HTTP_GET" || functionName == "HTTP_POST" ||
        functionName == "HTTP_PUT" || functionName == "HTTP_DELETE" ||
        functionName == "HTTP_요청")
    {
        return "http";
    }

    // db 모듈 함수들
    if (functionName == "DB_연결" || functionName == "DB_실행" ||
        functionName == "DB_조회" || functionName == "DB_닫기")
    {
        return "db";
    }

    // regex 모듈 함수들
    if (functionName == "정규식_매치" || functionName == "정규식_찾기" ||
        functionName == "정규식_대체")
    {
        return "regex";
    }

    // crypto 모듈 함수들
    if (functionName == "SHA256" || functionName == "Base64_인코딩" ||
        functionName == "Base64_디코딩")
    {
        return "crypto";
    }

    // collections 모듈 함수들
    if (functionName == "리스트_생성" || functionName == "맵_생성" ||
        functionName == "집합_생성")
    {
        return "collections";
    }

    return "";  // 모듈 추정 실패
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
