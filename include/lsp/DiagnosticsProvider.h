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
namespace ast {
class Program;
} // namespace ast

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
 * - 린터 이슈: 구문 에러가 없을 때 린터 규칙 결과 (setLintEnabled(true) 일 때만,
 *   source "kingsejong-lint", code 는 규칙 이름)
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
        std::string source;         ///< 출처 ("kingsejong" 또는 "kingsejong-lint")
        std::string code;           ///< 진단 코드 (린터 규칙 이름, 없으면 빈 문자열)

        /**
         * @brief 진단 정보 생성자
         * @param sLine 시작 줄
         * @param sChar 시작 컬럼
         * @param eLine 끝 줄
         * @param eChar 끝 컬럼
         * @param sev 심각도
         * @param msg 메시지
         * @param src 출처
         * @param cd 진단 코드
         */
        Diagnostic(int sLine, int sChar, int eLine, int eChar,
                  DiagnosticSeverity sev, const std::string& msg,
                  const std::string& src = "kingsejong",
                  const std::string& cd = "")
            : startLine(sLine)
            , startCharacter(sChar)
            , endLine(eLine)
            , endCharacter(eChar)
            , severity(sev)
            , message(msg)
            , source(src)
            , code(cd)
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
     * 4. 구문 에러가 없고 린터가 켜져 있으면 린터 이슈를 덧붙임
     *
     * Complexity: O(n) where n = document size
     */
    std::vector<Diagnostic> provideDiagnostics(
        const DocumentManager::Document& document
    );

    /**
     * @brief 린터 진단을 켜고 끈다
     * @param enabled true 면 구문 에러가 없는 문서에 린터 규칙을 돌린다
     *
     * 기본은 꺼져 있다. LanguageServer 가 kingsejong.lint.enable 설정(기본 켜짐)에 따라 켠다.
     */
    void setLintEnabled(bool enabled) { lintEnabled_ = enabled; }

    /// 린터 진단이 켜져 있는지
    bool isLintEnabled() const { return lintEnabled_; }

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
        const std::string& content,
        const std::string& uri
    );

    /**
     * @brief 린터 이슈 검사
     * @param program 구문 에러 없이 파싱된 AST
     * @param content 문서 내용 (열 변환용)
     * @param uri 문서 URI (.ksjlint.json 을 찾을 위치)
     * @return 진단 목록
     *
     * 문서 파일이 있는 디렉터리부터 루트까지 올라가며 .ksjlint.json 을 찾고,
     * 없으면 기본 규칙 설정을 쓴다. 설정 파일은 매번 다시 읽는다.
     */
    std::vector<Diagnostic> checkLintIssues(
        ast::Program* program,
        const std::string& content,
        const std::string& uri
    );

    /// 린터 진단 사용 여부
    bool lintEnabled_ = false;
};

} // namespace lsp
} // namespace kingsejong
