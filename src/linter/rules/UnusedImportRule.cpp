/**
 * @file UnusedImportRule.cpp
 * @brief 사용하지 않는 임포트 검출 규칙 구현
 * @author KingSejong Team
 * @date 2025-11-23
 */

#include "UnusedImportRule.h"
#include "linter/Linter.h"
#include "linter/IssueSeverity.h"
#include "lexer/JosaRecognizer.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "ast/Visitor.h"
#include <fstream>
#include <sstream>

namespace kingsejong {
namespace linter {
namespace rules {

namespace {

/// 프로그램 전체에서 쓰인 식별자 이름을 모은다 (호출뿐 아니라 값으로 넘긴 함수 이름도 포함)
class NameCollector : public ast::Visitor
{
public:
    std::unordered_set<std::string>& names;
    explicit NameCollector(std::unordered_set<std::string>& out) : names(out) {}

    bool enter(ast::Node& node) override
    {
        if (node.type() == ast::NodeType::IDENTIFIER)
        {
            names.insert(static_cast<const ast::Identifier&>(node).name());
        }
        return true;
    }
};

} // namespace

void UnusedImportRule::analyze(ast::Program* program)
{
    // 1단계: 모든 import 수집
    importedModules_.clear();
    usedModules_.clear();
    usedNames_.clear();

    // 2단계: AST 순회
    for (auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
        NameCollector collector(usedNames_);
        ast::walk(stmt.get(), collector);
    }

    // 라이브러리 모듈(최상위에 가져오기와 정의만 있는 파일)은 가져온 이름을 그대로 내보낸다
    // (평가기가 모듈 환경 전체를 복사한다). 그 가져오기를 지우면 모듈의 공개 이름이 줄어드므로 보고하지 않는다.
    if (isLibraryModule(program))
    {
        return;
    }

    // 3단계: 사용되지 않은 import 보고
    for (const auto& [moduleName, importInfo] : importedModules_)
    {
        // 모듈 파일을 읽을 수 있으면 그 모듈이 정의한 이름이 하나라도 쓰였는지 본다.
        // 읽지 못하면 함수 이름 목록으로 추정한다 (예전 동작).
        std::unordered_set<std::string> moduleNames;
        std::unordered_set<std::string> visited;
        bool known = collectModuleNames(importInfo.modulePath, linter() ? linter()->filename() : "", moduleNames, visited);
        bool used = false;
        if (known)
        {
            for (const auto& name : moduleNames)
            {
                if (usedNames_.count(name))
                {
                    used = true;
                    break;
                }
            }
        }
        else
        {
            used = usedModules_.find(moduleName) != usedModules_.end();
        }

        if (!used)
        {
            reportIssue(
                "import된 모듈 '" + importInfo.modulePath + "'" +
                    lexer::JosaRecognizer::selectOrBoth(importInfo.modulePath, lexer::JosaRecognizer::JosaType::I_GA) + " 사용되지 않습니다",
                IssueSeverity::WARNING,
                importInfo.line,
                importInfo.column
            );
        }
    }
}

bool UnusedImportRule::isLibraryModule(const ast::Program* program) const
{
    bool hasDefinition = false;
    for (const auto& stmt : program->statements())
    {
        switch (stmt->type())
        {
            case ast::NodeType::IMPORT_STATEMENT:
                break;
            case ast::NodeType::ASSIGNMENT_STATEMENT:
            case ast::NodeType::VAR_DECLARATION:
            case ast::NodeType::CLASS_STATEMENT:
                hasDefinition = true;
                break;
            default:
                return false;
        }
    }
    return hasDefinition;
}

bool UnusedImportRule::collectModuleNames(const std::string& modulePath, const std::string& fromFile,
                                          std::unordered_set<std::string>& names,
                                          std::unordered_set<std::string>& visited) const
{
    if (!linter() || !linter()->moduleResolver())
    {
        return false;
    }
    std::string path = linter()->moduleResolver()(modulePath, fromFile);
    if (path.empty())
    {
        return false;
    }
    if (!visited.insert(path).second)
    {
        return true;
    }

    std::ifstream file(path);
    if (!file.is_open())
    {
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();

    lexer::Lexer lexer(buffer.str(), path);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    if (!program || !parser.errors().empty())
    {
        return false;
    }

    // 의미 분석기(declareModuleSymbols)와 같은 규칙: 최상위 대입·선언·클래스, 그리고 모듈이 다시
    // 가져온 이름 (평가기가 모듈 환경 전체를 복사한다)
    for (const auto& stmt : program->statements())
    {
        switch (stmt->type())
        {
            case ast::NodeType::ASSIGNMENT_STATEMENT:
                names.insert(static_cast<const ast::AssignmentStatement*>(stmt.get())->varName());
                break;
            case ast::NodeType::VAR_DECLARATION:
                names.insert(static_cast<const ast::VarDeclaration*>(stmt.get())->varName());
                break;
            case ast::NodeType::CLASS_STATEMENT:
                names.insert(static_cast<const ast::ClassStatement*>(stmt.get())->className());
                break;
            case ast::NodeType::IMPORT_STATEMENT:
                collectModuleNames(static_cast<const ast::ImportStatement*>(stmt.get())->modulePath(), path, names, visited);
                break;
            default:
                break;
        }
    }
    return true;
}

void UnusedImportRule::onImportStatement(ast::ImportStatement* stmt)
{
    std::string moduleName = extractModuleName(stmt->modulePath());
    importedModules_[moduleName] = ImportInfo{stmt->modulePath(), stmt->location().line, stmt->location().column};
}

void UnusedImportRule::onCallExpression(ast::CallExpression* expr)
{
    // 함수명 추출
    if (auto funcIdent = ast::node_cast<ast::Identifier>(expr->function()))
    {
        std::string functionName = funcIdent->name();
        std::string moduleName = guessModuleFromFunction(functionName);

        if (!moduleName.empty() && importedModules_.find(moduleName) != importedModules_.end())
        {
            usedModules_.insert(moduleName);
        }
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
