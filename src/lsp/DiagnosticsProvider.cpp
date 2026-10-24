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

namespace kingsejong {
namespace lsp {

std::vector<DiagnosticsProvider::Diagnostic> DiagnosticsProvider::provideDiagnostics(
    const DocumentManager::Document& document)
{
    return checkSyntaxErrors(document.content);
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

} // namespace

std::vector<DiagnosticsProvider::Diagnostic> DiagnosticsProvider::checkSyntaxErrors(
    const std::string& content)
{
    std::vector<Diagnostic> diagnostics;

    try {
        // Lexer로 파싱 시도
        lexer::Lexer lexer(content);
        parser::Parser parser(lexer);

        // 전체 프로그램 파싱
        try {
            auto program = parser.parseProgram();
            (void)program;  // 사용하지 않음
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

} // namespace lsp
} // namespace kingsejong
