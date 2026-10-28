/**
 * @file DiagnosticsProvider.cpp
 * @brief LSP 진단 제공자 구현
 * @author KingSejong Team
 * @date 2025-11-12
 */

#include "lsp/DiagnosticsProvider.h"
#include "lsp/LspUtils.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "error/ErrorReporter.h"
#include "linter/Linter.h"
#include <filesystem>

namespace kingsejong {
namespace lsp {

std::vector<DiagnosticsProvider::Diagnostic> DiagnosticsProvider::provideDiagnostics(
    const DocumentManager::Document& document)
{
    return checkSyntaxErrors(document.content, document.uri);
}

namespace {

/// content 의 lineIndex(0부터) 번째 줄 텍스트
std::string lineText(const std::string& content, int lineIndex)
{
    size_t start = 0;
    for (int i = 0; i < lineIndex; ++i) {
        size_t nl = content.find('\n', start);
        if (nl == std::string::npos) return "";
        start = nl + 1;
    }
    size_t end = content.find('\n', start);
    return content.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

/// filePath 의 디렉터리부터 루트까지 올라가며 찾은 .ksjlint.json (없으면 빈 문자열)
std::string findLintConfig(const std::string& filePath)
{
    if (filePath.empty()) return "";

    std::error_code ec;
    std::filesystem::path dir = std::filesystem::path(filePath).parent_path();
    while (!dir.empty()) {
        std::filesystem::path candidate = dir / ".ksjlint.json";
        if (std::filesystem::is_regular_file(candidate, ec)) {
            return candidate.string();
        }
        if (dir == dir.root_path() || dir.parent_path() == dir) break;
        dir = dir.parent_path();
    }
    return "";
}

DiagnosticsProvider::DiagnosticSeverity toDiagnosticSeverity(linter::IssueSeverity severity)
{
    switch (severity) {
        case linter::IssueSeverity::ERROR:   return DiagnosticsProvider::DiagnosticSeverity::Error;
        case linter::IssueSeverity::WARNING: return DiagnosticsProvider::DiagnosticSeverity::Warning;
        case linter::IssueSeverity::INFO:    return DiagnosticsProvider::DiagnosticSeverity::Information;
        case linter::IssueSeverity::HINT:    return DiagnosticsProvider::DiagnosticSeverity::Hint;
    }
    return DiagnosticsProvider::DiagnosticSeverity::Warning;
}

} // namespace

std::vector<DiagnosticsProvider::Diagnostic> DiagnosticsProvider::checkSyntaxErrors(
    const std::string& content,
    const std::string& uri)
{
    std::vector<Diagnostic> diagnostics;

    try {
        // Lexer로 파싱 시도
        lexer::Lexer lexer(content);
        parser::Parser parser(lexer);

        // 전체 프로그램 파싱
        std::unique_ptr<ast::Program> program;
        try {
            program = parser.parseProgram();
        } catch (const std::exception& e) {
            // 파싱 중 예외 발생
            diagnostics.emplace_back(
                0, 0, 0, 1,
                DiagnosticSeverity::Error,
                std::string("Parse error: ") + e.what(),
                "kingsejong"
            );
        }

        // Parser 에러 수집 (파서가 기록한 토큰 위치를 LSP 좌표로 바꾼다)
        for (const auto& err : parser.errorDetails()) {
            int line = err.location.line > 0 ? err.location.line - 1 : 0;
            int character = byteColumnToUtf16(lineText(content, line), err.location.column);
            diagnostics.emplace_back(
                line, character, line, character + 1,
                DiagnosticSeverity::Error,
                err.message,
                "kingsejong"
            );
        }

        // 구문 에러가 없을 때만 린터를 돌린다 (깨진 AST 위의 린터 결과는 엉뚱한 경고가 된다)
        if (lintEnabled_ && diagnostics.empty() && program) {
            auto lintDiagnostics = checkLintIssues(program.get(), content, uri);
            diagnostics.insert(diagnostics.end(), lintDiagnostics.begin(), lintDiagnostics.end());
        }

    } catch (const std::exception& e) {
        // Lexer 예외 발생 시
        diagnostics.emplace_back(
            0, 0, 0, 1,
            DiagnosticSeverity::Error,
            std::string("Lexer error: ") + e.what(),
            "kingsejong"
        );
    }

    return diagnostics;
}

std::vector<DiagnosticsProvider::Diagnostic> DiagnosticsProvider::checkLintIssues(
    ast::Program* program,
    const std::string& content,
    const std::string& uri)
{
    std::vector<Diagnostic> diagnostics;

    try {
        const std::string filePath = uriToFilePath(uri);

        linter::Linter linter;
        linter.addDefaultRules();
        // 설정 파일이 없거나 읽지 못하면 기본 설정(모든 규칙 켜짐)으로 돈다
        const std::string configPath = findLintConfig(filePath);
        if (!configPath.empty()) {
            linter.loadConfig(configPath);
        }

        linter.analyze(program, filePath);

        for (const auto& issue : linter.issues()) {
            // 린터 위치는 파서 에러와 같은 1-based 줄·바이트 열이다
            int line = issue.line > 0 ? issue.line - 1 : 0;
            std::string text = lineText(content, line);
            int character = byteColumnToUtf16(text, issue.column);

            // 그 위치의 식별자(키워드·이름)까지 밑줄을 긋고, 식별자가 아니면 한 글자
            int endCharacter = character + 1;
            size_t byteOffset = issue.column > 0 ? static_cast<size_t>(issue.column - 1) : 0;
            std::string word = extractWordAtOffset(text, byteOffset);
            if (!word.empty() && wordStartOffset(text, byteOffset) == byteOffset) {
                endCharacter = character + utf16Length(word);
            }

            diagnostics.emplace_back(
                line, character, line, endCharacter,
                toDiagnosticSeverity(issue.severity),
                issue.message,
                "kingsejong-lint",
                issue.ruleId
            );
        }
    } catch (const std::exception&) {
        // 린터 실패가 구문 진단까지 막지 않도록 린터 결과만 버린다
        return {};
    }

    return diagnostics;
}

} // namespace lsp
} // namespace kingsejong
