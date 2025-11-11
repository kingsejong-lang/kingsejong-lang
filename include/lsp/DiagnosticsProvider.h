#pragma once

/**
 * @file DiagnosticsProvider.h
 * @brief LSP 진단 제공자
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * 문서를 파싱하여 구문 에러, 타입 에러 등을 찾아 반환합니다.
 */

#include <string>
#include <vector>
#include "lsp/DocumentManager.h"

namespace kingsejong {
namespace lsp {

/**
 * @class DiagnosticsProvider
 * @brief 실시간 에러 진단 제공자
 *
 * KingSejong 언어 코드를 파싱하여 에러를 찾아냅니다.
 * Lexer와 Parser를 사용하여 구문 에러를 검출하고,
 * LSP Diagnostic 형식으로 변환합니다.
 *
 * 제공하는 진단:
 * - Lexer 에러: 잘못된 토큰
 * - Parser 에러: 구문 에러
 * - 에러 위치: line, character (0-indexed)
 * - 심각도: Error, Warning, Information, Hint
 *
 * Thread Safety: NOT thread-safe (단일 스레드)
 * Memory: RAII - 자동 메모리 관리
 *
 * Example:
 * ```cpp
 * DiagnosticsProvider provider;
 * DocumentManager::Document doc("file:///test.ksj", "변수 x @", 1);
 *
 * auto diagnostics = provider.provideDiagnostics(doc);
 * for (const auto& diag : diagnostics) {
 *     std::cout << "Error at line " << diag.startLine << ": "
 *               << diag.message << std::endl;
 * }
 * ```
 */
class DiagnosticsProvider {
public:
    /**
     * @enum DiagnosticSeverity
     * @brief 진단 심각도 (LSP 표준)
     *
     * LSP 표준 DiagnosticSeverity 값들:
     * - Error = 1
     * - Warning = 2
     * - Information = 3
     * - Hint = 4
     */
    enum class DiagnosticSeverity {
        Error = 1,        ///< 에러
        Warning = 2,      ///< 경고
        Information = 3,  ///< 정보
        Hint = 4         ///< 힌트
    };

    /**
     * @struct Diagnostic
     * @brief 진단 정보
     *
     * LSP Diagnostic 구조체입니다.
     * Range는 startLine/startCharacter ~ endLine/endCharacter로 표현됩니다.
     */
    struct Diagnostic {
        int startLine;               ///< 시작 줄 (0부터)
        int startCharacter;          ///< 시작 컬럼 (0부터)
        int endLine;                ///< 끝 줄 (0부터)
        int endCharacter;           ///< 끝 컬럼 (0부터)
        DiagnosticSeverity severity;///< 심각도
        std::string message;        ///< 에러 메시지
        std::string source;         ///< 출처 ("kingsejong")

        /**
         * @brief 진단 정보 생성자
         * @param sLine 시작 줄
         * @param sChar 시작 컬럼
         * @param eLine 끝 줄
         * @param eChar 끝 컬럼
         * @param sev 심각도
         * @param msg 메시지
         * @param src 출처
         */
        Diagnostic(int sLine, int sChar, int eLine, int eChar,
                  DiagnosticSeverity sev, const std::string& msg,
                  const std::string& src = "kingsejong")
            : startLine(sLine)
            , startCharacter(sChar)
            , endLine(eLine)
            , endCharacter(eChar)
            , severity(sev)
            , message(msg)
            , source(src)
        {}
    };

    /**
     * @brief 생성자
     *
     * Postconditions:
     * - DiagnosticsProvider 초기화됨
     */
    DiagnosticsProvider() = default;

    // 복사 금지, 이동 허용
    DiagnosticsProvider(const DiagnosticsProvider&) = delete;
    DiagnosticsProvider& operator=(const DiagnosticsProvider&) = delete;
    DiagnosticsProvider(DiagnosticsProvider&&) noexcept = default;
    DiagnosticsProvider& operator=(DiagnosticsProvider&&) noexcept = default;
    ~DiagnosticsProvider() = default;

    /**
     * @brief 문서 진단
     * @param document 문서
     * @return 진단 목록
     *
     * 문서를 Lexer와 Parser로 파싱하여 에러를 수집합니다.
     * 에러가 없으면 빈 벡터를 반환합니다.
     *
     * 처리 과정:
     * 1. Lexer로 토큰화 (렉서 에러 수집)
     * 2. Parser로 파싱 (파서 에러 수집)
     * 3. 에러를 LSP Diagnostic 형식으로 변환
     *
     * Complexity: O(n) where n = document size
     */
    std::vector<Diagnostic> provideDiagnostics(
        const DocumentManager::Document& document
    );

private:
    /**
     * @brief 구문 에러 검사
     * @param content 문서 내용
     * @return 진단 목록
     *
     * Lexer와 Parser를 사용하여 구문 에러를 검출합니다.
     *
     * Complexity: O(n)
     */
    std::vector<Diagnostic> checkSyntaxErrors(
        const std::string& content
    );
};

} // namespace lsp
} // namespace kingsejong
