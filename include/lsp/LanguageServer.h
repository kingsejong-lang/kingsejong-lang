#pragma once

/**
 * @file LanguageServer.h
 * @brief LSP 언어 서버 메인 클래스
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * LSP 프로토콜을 구현하는 언어 서버입니다.
 * JSON-RPC 2.0을 통해 클라이언트와 통신합니다.
 */

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "lsp/DocumentManager.h"
#include "lsp/JsonRpc.h"
#include "lsp/CompletionProvider.h"
#include "lsp/DiagnosticsProvider.h"

namespace kingsejong {
namespace lsp {

/**
 * @class LanguageServer
 * @brief LSP 언어 서버
 *
 * LSP 프로토콜을 구현하는 메인 클래스입니다.
 * 클라이언트 요청을 처리하고 문서를 관리합니다.
 *
 * 지원하는 LSP 메서드:
 * - initialize: 서버 초기화
 * - initialized: 초기화 완료 알림
 * - textDocument/didOpen: 문서 열기
 * - textDocument/didChange: 문서 변경
 * - textDocument/didClose: 문서 닫기
 * - textDocument/completion: 자동 완성
 * - shutdown: 서버 종료 준비
 * - exit: 서버 종료
 *
 * Thread Safety: NOT thread-safe (단일 스레드)
 * Memory: RAII - 자동 메모리 관리
 *
 * @invariant initialized_ == true이면 shutdown 가능
 * @invariant shutdown_ == true이면 exit 가능
 *
 * Example:
 * ```cpp
 * LanguageServer server;
 *
 * // Initialize
 * json initRequest = {
 *     {"jsonrpc", "2.0"},
 *     {"id", 1},
 *     {"method", "initialize"},
 *     {"params", {}}
 * };
 * json response = server.handleRequest(initRequest);
 *
 * // Open document
 * json openNotification = {
 *     {"jsonrpc", "2.0"},
 *     {"method", "textDocument/didOpen"},
 *     {"params", {
 *         {"textDocument", {
 *             {"uri", "file:///test.ksj"},
 *             {"languageId", "kingsejong"},
 *             {"version", 1},
 *             {"text", "변수 x = 10"}
 *         }}
 *     }}
 * };
 * server.handleRequest(openNotification);
 *
 * // Shutdown
 * json shutdownRequest = {
 *     {"jsonrpc", "2.0"},
 *     {"id", 2},
 *     {"method", "shutdown"},
 *     {"params", nullptr}
 * };
 * server.handleRequest(shutdownRequest);
 *
 * // Exit
 * json exitNotification = {
 *     {"jsonrpc", "2.0"},
 *     {"method", "exit"}
 * };
 * server.handleRequest(exitNotification);
 * ```
 */
class LanguageServer {
public:
    /**
     * @brief 생성자
     *
     * Postconditions:
     * - initialized_ == false
     * - shutdown_ == false
     */
    LanguageServer();

    // 복사 금지, 이동 허용
    LanguageServer(const LanguageServer&) = delete;
    LanguageServer& operator=(const LanguageServer&) = delete;
    LanguageServer(LanguageServer&&) noexcept = default;
    LanguageServer& operator=(LanguageServer&&) noexcept = default;
    ~LanguageServer() = default;

    /**
     * @brief 요청 처리
     * @param request JSON-RPC 요청
     * @return JSON-RPC 응답 (notification이면 empty json)
     *
     * 요청 형식:
     * {
     *   "jsonrpc": "2.0",
     *   "id": 1,                 // notification이면 생략
     *   "method": "initialize",
     *   "params": {...}
     * }
     *
     * 응답 형식:
     * {
     *   "jsonrpc": "2.0",
     *   "id": 1,
     *   "result": {...}          // 성공
     * }
     * 또는
     * {
     *   "jsonrpc": "2.0",
     *   "id": 1,
     *   "error": {               // 실패
     *     "code": -32601,
     *     "message": "Method not found"
     *   }
     * }
     *
     * Complexity: O(1) for dispatch, O(n) for document operations
     */
    nlohmann::json handleRequest(const nlohmann::json& request);

    /**
     * @brief 문서 조회
     * @param uri 문서 URI
     * @return Document 포인터 (없으면 nullptr)
     *
     * Complexity: O(log n)
     */
    const DocumentManager::Document* getDocument(const std::string& uri) const;

    /**
     * @brief 종료 상태 확인
     * @return true if shutdown called
     *
     * Complexity: O(1)
     */
    bool isShutdown() const { return shutdown_; }

    /**
     * @brief 초기화 상태 확인
     * @return true if initialized
     *
     * Complexity: O(1)
     */
    bool isInitialized() const { return initialized_; }

private:
    /**
     * @brief initialize 핸들러
     * @param params 초기화 파라미터
     * @return 서버 capabilities
     *
     * Postconditions:
     * - initialized_ == true
     *
     * Complexity: O(1)
     */
    nlohmann::json handleInitialize(const nlohmann::json& params);

    /**
     * @brief initialized 핸들러 (notification)
     * @param params 빈 객체
     *
     * Complexity: O(1)
     */
    void handleInitialized(const nlohmann::json& params);

    /**
     * @brief textDocument/didOpen 핸들러
     * @param params 문서 정보
     *
     * params 형식:
     * {
     *   "textDocument": {
     *     "uri": "file:///test.ksj",
     *     "languageId": "kingsejong",
     *     "version": 1,
     *     "text": "변수 x = 10"
     *   }
     * }
     *
     * Preconditions:
     * - initialized_ == true
     * - params["textDocument"] 존재
     *
     * Postconditions:
     * - getDocument(uri) != nullptr
     *
     * Complexity: O(log n)
     */
    void handleTextDocumentDidOpen(const nlohmann::json& params);

    /**
     * @brief textDocument/didChange 핸들러
     * @param params 변경 내용
     *
     * params 형식:
     * {
     *   "textDocument": {
     *     "uri": "file:///test.ksj",
     *     "version": 2
     *   },
     *   "contentChanges": [
     *     {"text": "변수 x = 20"}
     *   ]
     * }
     *
     * Preconditions:
     * - getDocument(uri) != nullptr
     *
     * Postconditions:
     * - getDocument(uri)->version == new version
     * - getDocument(uri)->content == new content
     *
     * Complexity: O(log n)
     */
    void handleTextDocumentDidChange(const nlohmann::json& params);

    /**
     * @brief textDocument/didClose 핸들러
     * @param params 문서 정보
     *
     * params 형식:
     * {
     *   "textDocument": {
     *     "uri": "file:///test.ksj"
     *   }
     * }
     *
     * Postconditions:
     * - getDocument(uri) == nullptr
     *
     * Complexity: O(log n)
     */
    void handleTextDocumentDidClose(const nlohmann::json& params);

    /**
     * @brief textDocument/completion 핸들러
     * @param params 완성 요청 파라미터
     * @return 완성 항목 목록
     *
     * params 형식:
     * {
     *   "textDocument": {
     *     "uri": "file:///test.ksj"
     *   },
     *   "position": {
     *     "line": 5,
     *     "character": 10
     *   }
     * }
     *
     * 반환 형식:
     * {
     *   "items": [
     *     {
     *       "label": "변수",
     *       "kind": 14,
     *       "detail": "변수 선언",
     *       "documentation": "..."
     *     },
     *     ...
     *   ]
     * }
     *
     * Preconditions:
     * - getDocument(uri) != nullptr
     *
     * Complexity: O(n) where n = document size
     */
    nlohmann::json handleTextDocumentCompletion(const nlohmann::json& params);

    /**
     * @brief shutdown 핸들러
     * @param params 빈 객체
     * @return null
     *
     * Postconditions:
     * - shutdown_ == true
     *
     * Complexity: O(1)
     */
    nlohmann::json handleShutdown(const nlohmann::json& params);

    /**
     * @brief exit 핸들러 (notification)
     * @param params 빈 객체
     *
     * Preconditions:
     * - shutdown_ == true (권장)
     *
     * Complexity: O(1)
     */
    void handleExit(const nlohmann::json& params);

    /**
     * @brief 진단 알림 발행
     * @param uri 문서 URI
     * @param diagnostics 진단 목록
     * @return publishDiagnostics 알림 JSON
     *
     * textDocument/publishDiagnostics 알림을 생성합니다.
     * 이 알림은 클라이언트에게 에러/경고를 전달합니다.
     *
     * Complexity: O(n) where n = diagnostics size
     */
    nlohmann::json createPublishDiagnosticsNotification(
        const std::string& uri,
        const std::vector<DiagnosticsProvider::Diagnostic>& diagnostics
    );

    /// 문서 관리자
    DocumentManager documentManager_;

    /// JSON-RPC 핸들러
    JsonRpc jsonRpc_;

    /// 자동 완성 제공자
    CompletionProvider completionProvider_;

    /// 진단 제공자
    DiagnosticsProvider diagnosticsProvider_;

    /// 초기화 상태
    bool initialized_ = false;

    /// 종료 상태
    bool shutdown_ = false;
};

} // namespace lsp
} // namespace kingsejong
