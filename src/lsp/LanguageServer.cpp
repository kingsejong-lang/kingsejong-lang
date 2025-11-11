/**
 * @file LanguageServer.cpp
 * @brief LSP 언어 서버 구현
 * @author KingSejong Team
 * @date 2025-11-12
 */

#include "lsp/LanguageServer.h"
#include <stdexcept>

namespace kingsejong {
namespace lsp {

LanguageServer::LanguageServer()
    : initialized_(false)
    , shutdown_(false)
{
    // DiagnosticsProvider는 향후 사용 예정
    (void)diagnosticsProvider_;
}

nlohmann::json LanguageServer::handleRequest(const nlohmann::json& request)
{
    try {
        // jsonrpc 버전 확인
        if (!request.contains("jsonrpc") || request["jsonrpc"] != "2.0") {
            return jsonRpc_.createError(
                request.value("id", -1),
                JsonRpc::InvalidRequest,
                "Invalid jsonrpc version"
            );
        }

        // method 확인
        if (!request.contains("method") || !request["method"].is_string()) {
            return jsonRpc_.createError(
                request.value("id", -1),
                JsonRpc::InvalidRequest,
                "Missing or invalid method"
            );
        }

        std::string method = request["method"];
        nlohmann::json params = request.value("params", nlohmann::json::object());

        // Notification인지 Request인지 확인
        bool isNotification = !request.contains("id");

        // Method 디스패치
        if (method == "initialize") {
            if (isNotification) {
                return jsonRpc_.createError(
                    -1,
                    JsonRpc::InvalidRequest,
                    "initialize must be a request, not a notification"
                );
            }
            nlohmann::json result = handleInitialize(params);
            return jsonRpc_.createResponse(request["id"], result);
        }
        else if (method == "initialized") {
            handleInitialized(params);
            return nlohmann::json();  // notification - no response
        }
        else if (method == "textDocument/didOpen") {
            try {
                handleTextDocumentDidOpen(params);
                return nlohmann::json();  // notification - no response
            } catch (const std::exception& e) {
                if (!isNotification) {
                    return jsonRpc_.createError(
                        request["id"],
                        JsonRpc::InvalidParams,
                        e.what()
                    );
                }
                return nlohmann::json();
            }
        }
        else if (method == "textDocument/didChange") {
            try {
                handleTextDocumentDidChange(params);
                return nlohmann::json();  // notification - no response
            } catch (const std::exception& e) {
                if (!isNotification) {
                    return jsonRpc_.createError(
                        request["id"],
                        JsonRpc::InvalidParams,
                        e.what()
                    );
                }
                return nlohmann::json();
            }
        }
        else if (method == "textDocument/didClose") {
            handleTextDocumentDidClose(params);
            return nlohmann::json();  // notification - no response
        }
        else if (method == "textDocument/completion") {
            if (isNotification) {
                return jsonRpc_.createError(
                    -1,
                    JsonRpc::InvalidRequest,
                    "completion must be a request, not a notification"
                );
            }
            try {
                nlohmann::json result = handleTextDocumentCompletion(params);
                return jsonRpc_.createResponse(request["id"], result);
            } catch (const std::exception& e) {
                return jsonRpc_.createError(
                    request["id"],
                    JsonRpc::InvalidParams,
                    e.what()
                );
            }
        }
        else if (method == "shutdown") {
            if (isNotification) {
                return jsonRpc_.createError(
                    -1,
                    JsonRpc::InvalidRequest,
                    "shutdown must be a request, not a notification"
                );
            }
            nlohmann::json result = handleShutdown(params);
            return jsonRpc_.createResponse(request["id"], result);
        }
        else if (method == "exit") {
            handleExit(params);
            return nlohmann::json();  // notification - no response
        }
        else {
            // Unknown method
            if (!isNotification) {
                return jsonRpc_.createError(
                    request["id"],
                    JsonRpc::MethodNotFound,
                    "Method not found: " + method
                );
            }
            return nlohmann::json();
        }
    }
    catch (const std::exception& e) {
        // Internal error
        return jsonRpc_.createError(
            request.value("id", -1),
            JsonRpc::InternalError,
            std::string("Internal error: ") + e.what()
        );
    }
}

const DocumentManager::Document* LanguageServer::getDocument(const std::string& uri) const
{
    return documentManager_.getDocument(uri);
}

nlohmann::json LanguageServer::handleInitialize(const nlohmann::json& params)
{
    (void)params;  // 현재는 사용하지 않음

    // 서버 capabilities 반환
    nlohmann::json capabilities = {
        {"textDocumentSync", 1},  // Full sync
        {"completionProvider", {
            {"triggerCharacters", nlohmann::json::array()}
        }}
    };

    initialized_ = true;

    return {
        {"capabilities", capabilities},
        {"serverInfo", {
            {"name", "kingsejong-lsp"},
            {"version", "0.1.0"}
        }}
    };
}

void LanguageServer::handleInitialized(const nlohmann::json& params)
{
    // initialized notification - 아무 작업 없음
    (void)params;
}

void LanguageServer::handleTextDocumentDidOpen(const nlohmann::json& params)
{
    // params 검증
    if (!params.contains("textDocument")) {
        throw std::runtime_error("Missing textDocument parameter");
    }

    auto textDocument = params["textDocument"];

    if (!textDocument.contains("uri") || !textDocument["uri"].is_string()) {
        throw std::runtime_error("Missing or invalid uri");
    }
    if (!textDocument.contains("text") || !textDocument["text"].is_string()) {
        throw std::runtime_error("Missing or invalid text");
    }
    if (!textDocument.contains("version") || !textDocument["version"].is_number()) {
        throw std::runtime_error("Missing or invalid version");
    }

    std::string uri = textDocument["uri"];
    std::string text = textDocument["text"];
    int version = textDocument["version"];

    // 문서 열기
    documentManager_.openDocument(uri, text, version);
}

void LanguageServer::handleTextDocumentDidChange(const nlohmann::json& params)
{
    // params 검증
    if (!params.contains("textDocument")) {
        throw std::runtime_error("Missing textDocument parameter");
    }
    if (!params.contains("contentChanges")) {
        throw std::runtime_error("Missing contentChanges parameter");
    }

    auto textDocument = params["textDocument"];
    auto contentChanges = params["contentChanges"];

    if (!textDocument.contains("uri") || !textDocument["uri"].is_string()) {
        throw std::runtime_error("Missing or invalid uri");
    }
    if (!textDocument.contains("version") || !textDocument["version"].is_number()) {
        throw std::runtime_error("Missing or invalid version");
    }
    if (!contentChanges.is_array() || contentChanges.empty()) {
        throw std::runtime_error("Invalid contentChanges");
    }

    std::string uri = textDocument["uri"];
    int version = textDocument["version"];

    // Full sync: contentChanges 배열의 마지막 요소 사용
    auto lastChange = contentChanges.back();
    if (!lastChange.contains("text") || !lastChange["text"].is_string()) {
        throw std::runtime_error("Invalid change text");
    }

    std::string newText = lastChange["text"];

    // 문서 업데이트
    documentManager_.updateDocument(uri, newText, version);
}

void LanguageServer::handleTextDocumentDidClose(const nlohmann::json& params)
{
    // params 검증
    if (!params.contains("textDocument")) {
        throw std::runtime_error("Missing textDocument parameter");
    }

    auto textDocument = params["textDocument"];

    if (!textDocument.contains("uri") || !textDocument["uri"].is_string()) {
        throw std::runtime_error("Missing or invalid uri");
    }

    std::string uri = textDocument["uri"];

    // 문서 닫기
    documentManager_.closeDocument(uri);
}

nlohmann::json LanguageServer::handleTextDocumentCompletion(const nlohmann::json& params)
{
    // params 검증
    if (!params.contains("textDocument")) {
        throw std::runtime_error("Missing textDocument parameter");
    }
    if (!params.contains("position")) {
        throw std::runtime_error("Missing position parameter");
    }

    auto textDocument = params["textDocument"];
    auto position = params["position"];

    if (!textDocument.contains("uri") || !textDocument["uri"].is_string()) {
        throw std::runtime_error("Missing or invalid uri");
    }
    if (!position.contains("line") || !position["line"].is_number()) {
        throw std::runtime_error("Missing or invalid line");
    }
    if (!position.contains("character") || !position["character"].is_number()) {
        throw std::runtime_error("Missing or invalid character");
    }

    std::string uri = textDocument["uri"];
    int line = position["line"];
    int character = position["character"];

    // 문서 조회
    const auto* doc = documentManager_.getDocument(uri);
    if (!doc) {
        throw std::runtime_error("Document not found: " + uri);
    }

    // 완성 항목 생성
    auto items = completionProvider_.provideCompletions(*doc, line, character);

    // JSON 변환
    nlohmann::json completionItems = nlohmann::json::array();
    for (const auto& item : items) {
        nlohmann::json jsonItem = {
            {"label", item.label},
            {"kind", static_cast<int>(item.kind)}
        };

        if (!item.detail.empty()) {
            jsonItem["detail"] = item.detail;
        }
        if (!item.documentation.empty()) {
            jsonItem["documentation"] = item.documentation;
        }

        completionItems.push_back(jsonItem);
    }

    return {{"items", completionItems}};
}

nlohmann::json LanguageServer::handleShutdown(const nlohmann::json& params)
{
    (void)params;

    shutdown_ = true;

    // null 반환
    return nullptr;
}

void LanguageServer::handleExit(const nlohmann::json& params)
{
    (void)params;

    // exit notification - 실제 종료는 상위 레벨에서 처리
}

nlohmann::json LanguageServer::createPublishDiagnosticsNotification(
    const std::string& uri,
    const std::vector<DiagnosticsProvider::Diagnostic>& diagnostics)
{
    // Diagnostics 배열 생성
    nlohmann::json diagnosticsArray = nlohmann::json::array();

    for (const auto& diag : diagnostics) {
        nlohmann::json jsonDiag = {
            {"range", {
                {"start", {
                    {"line", diag.startLine},
                    {"character", diag.startCharacter}
                }},
                {"end", {
                    {"line", diag.endLine},
                    {"character", diag.endCharacter}
                }}
            }},
            {"severity", static_cast<int>(diag.severity)},
            {"source", diag.source},
            {"message", diag.message}
        };

        diagnosticsArray.push_back(jsonDiag);
    }

    // publishDiagnostics 알림 생성
    return {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/publishDiagnostics"},
        {"params", {
            {"uri", uri},
            {"diagnostics", diagnosticsArray}
        }}
    };
}

} // namespace lsp
} // namespace kingsejong
