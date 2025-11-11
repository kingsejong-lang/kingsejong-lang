/**
 * @file DiagnosticsProvider.cpp
 * @brief LSP 진단 제공자 구현
 * @author KingSejong Team
 * @date 2025-11-12
 */

#include "lsp/DiagnosticsProvider.h"
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

        // Parser 에러 수집
        const auto& parseErrors = parser.errors();
        for (const auto& errorMsg : parseErrors) {
            // 에러 메시지만 있고 위치 정보는 없음
            // 기본 위치 (0, 0)에 에러 표시
            diagnostics.emplace_back(
                0, 0, 0, 1,
                DiagnosticSeverity::Error,
                errorMsg,
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
