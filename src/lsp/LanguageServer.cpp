/**
 * @file LanguageServer.cpp
 * @brief LSP 언어 서버 구현
 * @author KingSejong Team
 * @date 2025-11-12
 */

#include "lsp/LanguageServer.h"
#include "lsp/SymbolCollector.h"
#include "lsp/LspUtils.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include <stdexcept>
#include <sstream>
#include <cctype>

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
        else if (method == "textDocument/definition") {
            if (isNotification) {
                return jsonRpc_.createError(
                    -1,
                    JsonRpc::InvalidRequest,
                    "definition must be a request, not a notification"
                );
            }
            try {
                nlohmann::json result = handleTextDocumentDefinition(params);
                return jsonRpc_.createResponse(request["id"], result);
            } catch (const std::exception& e) {
                return jsonRpc_.createError(
                    request["id"],
                    JsonRpc::InvalidParams,
                    e.what()
                );
            }
        }
        else if (method == "textDocument/hover") {
            if (isNotification) {
                return jsonRpc_.createError(
                    -1,
                    JsonRpc::InvalidRequest,
                    "hover must be a request, not a notification"
                );
            }
            try {
                nlohmann::json result = handleTextDocumentHover(params);
                return jsonRpc_.createResponse(request["id"], result);
            } catch (const std::exception& e) {
                return jsonRpc_.createError(
                    request["id"],
                    JsonRpc::InvalidParams,
                    e.what()
                );
            }
        }
        else if (method == "textDocument/references") {
            if (isNotification) {
                return jsonRpc_.createError(
                    -1,
                    JsonRpc::InvalidRequest,
                    "references must be a request, not a notification"
                );
            }
            try {
                nlohmann::json result = handleTextDocumentReferences(params);
                return jsonRpc_.createResponse(request["id"], result);
            } catch (const std::exception& e) {
                return jsonRpc_.createError(
                    request["id"],
                    JsonRpc::InvalidParams,
                    e.what()
                );
            }
        }
        else if (method == "textDocument/rename") {
            if (isNotification) {
                return jsonRpc_.createError(
                    -1,
                    JsonRpc::InvalidRequest,
                    "rename must be a request, not a notification"
                );
            }
            try {
                nlohmann::json result = handleTextDocumentRename(params);
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
        }},
        {"definitionProvider", true},      // Go to Definition (F12)
        {"hoverProvider", true},           // Hover Information
        {"referencesProvider", true},      // Find References (Shift+F12)
        {"renameProvider", true}           // Rename Symbol (F2)
    };

    initialized_ = true;

    return {
        {"capabilities", capabilities},
        {"serverInfo", {
            {"name", "kingsejong-lsp"},
            {"version", "0.3.2"}
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

    // 심볼 테이블 업데이트
    try {
        lexer::Lexer lexer(text);
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();

        if (program) {
            symbolTable_.clear();  // 기존 심볼 제거
            SymbolCollector collector(symbolTable_);
            collector.collect(program.get(), uri);
        }
    } catch (const std::exception& e) {
        // 파싱 에러는 무시 (진단에서 처리됨)
    }
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

    // 심볼 테이블 업데이트
    try {
        lexer::Lexer lexer(newText);
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();

        if (program) {
            symbolTable_.clear();  // 기존 심볼 제거
            SymbolCollector collector(symbolTable_);
            collector.collect(program.get(), uri);
        }
    } catch (const std::exception& e) {
        // 파싱 에러는 무시 (진단에서 처리됨)
    }
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

nlohmann::json LanguageServer::handleTextDocumentDefinition(const nlohmann::json& params)
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

    // 문서 가져오기
    const auto* doc = getDocument(uri);
    if (!doc) {
        throw std::runtime_error("Document not found: " + uri);
    }

    // 커서 위치에서 심볼 이름 추출
    const std::string& content = doc->content;

    // 줄 가져오기
    auto lines = splitLines(content);
    if (line < 0 || line >= static_cast<int>(lines.size())) {
        return nullptr;  // 유효하지 않은 줄
    }

    const std::string& targetLine = lines[line];

    // LSP character position을 바이트 오프셋으로 변환
    size_t byteOffset = characterToByteOffset(targetLine, character);
    if (byteOffset >= targetLine.length()) {
        return nullptr;
    }

    // 바이트 오프셋 위치에서 단어 추출
    std::string symbolName = extractWordAtOffset(targetLine, byteOffset);
    if (symbolName.empty()) {
        return nullptr;
    }

    // 심볼 찾기 (스코프 고려)
    // 1. 커서 위치가 속한 함수 찾기
    std::string currentScope = "global";

    // 모든 함수 심볼을 가져와서 줄 번호로 판단
    auto allFunctions = symbolTable_.getAllSymbolsByKind(SymbolKind::Function);

    // 현재 줄이 어느 함수 안에 있는지 확인
    for (const auto& func : allFunctions) {
        int funcDefLine = func.definitionLocation.line - 1;  // 0-based로 변환

        // 함수 정의보다 뒤에 있고, 함수가 끝나기 전인지 확인
        if (line > funcDefLine) {
            // 함수의 끝을 찾기 (간단한 휴리스틱: 닫는 중괄호)
            bool insideFunc = false;
            int braceCount = 0;

            for (int i = funcDefLine; i <= line && i < static_cast<int>(lines.size()); ++i) {
                const std::string& checkLine = lines[i];

                // 여는 중괄호 카운트
                for (char c : checkLine) {
                    if (c == '{') {
                        braceCount++;
                        if (braceCount == 1 && i == funcDefLine) {
                            insideFunc = true;
                        }
                    }
                    else if (c == '}') {
                        braceCount--;
                        if (braceCount == 0 && insideFunc) {
                            // 함수가 끝남
                            insideFunc = false;
                        }
                    }
                }

                if (i == line && insideFunc && braceCount > 0) {
                    currentScope = "함수:" + func.name;
                    break;
                }
            }

            if (currentScope != "global") {
                break;
            }
        }
    }

    // 현재 스코프에서 먼저 찾기
    if (currentScope != "global") {
        auto symbol = symbolTable_.findSymbolInScope(symbolName, currentScope);
        if (symbol.has_value()) {
            // 정의 위치 반환
            return {
                {"uri", symbol->definitionLocation.uri},
                {"range", {
                    {"start", {
                        {"line", symbol->definitionLocation.line - 1},
                        {"character", symbol->definitionLocation.column - 1}
                    }},
                    {"end", {
                        {"line", symbol->definitionLocation.line - 1},
                        {"character", symbol->definitionLocation.column - 1 + static_cast<int>(symbol->name.length())}
                    }}
                }}
            };
        }
    }

    // 전역 스코프에서 찾기
    auto symbol = symbolTable_.findSymbol(symbolName);

    if (!symbol.has_value()) {
        // 심볼을 찾지 못함 - null 반환
        return nullptr;
    }

    // 정의 위치 반환 (LSP Location 형식)
    // SymbolTable은 1-based이므로 -1
    return {
        {"uri", symbol->definitionLocation.uri},
        {"range", {
            {"start", {
                {"line", symbol->definitionLocation.line - 1},
                {"character", symbol->definitionLocation.column - 1}
            }},
            {"end", {
                {"line", symbol->definitionLocation.line - 1},
                {"character", symbol->definitionLocation.column - 1 + static_cast<int>(symbol->name.length())}
            }}
        }}
    };
}

nlohmann::json LanguageServer::handleTextDocumentHover(const nlohmann::json& params)
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

    // 문서 가져오기
    const auto* doc = getDocument(uri);
    if (!doc) {
        throw std::runtime_error("Document not found: " + uri);
    }

    // 커서 위치에서 심볼 이름 추출
    const std::string& content = doc->content;

    // 줄 가져오기
    auto lines = splitLines(content);
    if (line < 0 || line >= static_cast<int>(lines.size())) {
        return nullptr;  // 유효하지 않은 줄
    }

    const std::string& targetLine = lines[line];

    // LSP character position을 바이트 오프셋으로 변환
    size_t byteOffset = characterToByteOffset(targetLine, character);
    if (byteOffset >= targetLine.length()) {
        return nullptr;
    }

    // 바이트 오프셋 위치에서 단어 추출
    std::string symbolName = extractWordAtOffset(targetLine, byteOffset);
    if (symbolName.empty()) {
        return nullptr;
    }

    // 스코프 감지 (definition과 동일한 로직)
    std::string currentScope = "global";
    auto allFunctions = symbolTable_.getAllSymbolsByKind(SymbolKind::Function);

    for (const auto& func : allFunctions) {
        int funcDefLine = func.definitionLocation.line - 1;

        if (line > funcDefLine) {
            bool insideFunc = false;
            int braceCount = 0;

            for (int i = funcDefLine; i <= line && i < static_cast<int>(lines.size()); ++i) {
                const std::string& checkLine = lines[i];

                for (char c : checkLine) {
                    if (c == '{') {
                        braceCount++;
                        if (braceCount == 1 && i == funcDefLine) {
                            insideFunc = true;
                        }
                    }
                    else if (c == '}') {
                        braceCount--;
                        if (braceCount == 0 && insideFunc) {
                            insideFunc = false;
                        }
                    }
                }

                if (i == line && insideFunc && braceCount > 0) {
                    currentScope = "함수:" + func.name;
                    break;
                }
            }

            if (currentScope != "global") {
                break;
            }
        }
    }

    // 현재 스코프에서 먼저 찾기
    std::optional<Symbol> symbol;
    if (currentScope != "global") {
        symbol = symbolTable_.findSymbolInScope(symbolName, currentScope);
    }

    // 로컬에서 못 찾으면 전역에서 찾기
    if (!symbol.has_value()) {
        symbol = symbolTable_.findSymbol(symbolName);
    }

    if (!symbol.has_value()) {
        // 심볼을 찾지 못함 - null 반환
        return nullptr;
    }

    // Hover 정보 생성
    std::string hoverText;

    switch (symbol->kind) {
        case SymbolKind::Variable:
            // "정수 x" 형식
            hoverText = symbol->typeInfo + " " + symbol->name;
            break;

        case SymbolKind::Function:
            // "함수 더하기(a, b)" 형식
            // 매개변수 목록 찾기
            {
                std::vector<std::string> params;
                auto paramSymbols = symbolTable_.getAllSymbolsByKind(SymbolKind::Parameter);

                for (const auto& param : paramSymbols) {
                    if (param.scope == "함수:" + symbol->name) {
                        params.push_back(param.name);
                    }
                }

                hoverText = "함수 " + symbol->name + "(";
                for (size_t i = 0; i < params.size(); ++i) {
                    if (i > 0) hoverText += ", ";
                    hoverText += params[i];
                }
                hoverText += ")";
            }
            break;

        case SymbolKind::Parameter:
            // "매개변수 a" 형식
            hoverText = "매개변수 " + symbol->name;
            break;

        default:
            hoverText = symbol->name;
            break;
    }

    // LSP Hover 형식으로 반환
    return {
        {"contents", {
            {"kind", "markdown"},
            {"value", hoverText}
        }}
    };
}

nlohmann::json LanguageServer::handleTextDocumentReferences(const nlohmann::json& params)
{
    // params 검증
    if (!params.contains("textDocument")) {
        throw std::runtime_error("Missing textDocument parameter");
    }
    if (!params.contains("position")) {
        throw std::runtime_error("Missing position parameter");
    }
    if (!params.contains("context")) {
        throw std::runtime_error("Missing context parameter");
    }

    auto textDocument = params["textDocument"];
    auto position = params["position"];
    auto context = params["context"];

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
    bool includeDeclaration = context.contains("includeDeclaration") &&
                               context["includeDeclaration"].is_boolean() &&
                               context["includeDeclaration"].get<bool>();

    // 문서 가져오기
    const auto* doc = getDocument(uri);
    if (!doc) {
        throw std::runtime_error("Document not found: " + uri);
    }

    // 커서 위치에서 심볼 이름 추출
    const std::string& content = doc->content;

    // 줄 가져오기
    auto lines = splitLines(content);
    if (line < 0 || line >= static_cast<int>(lines.size())) {
        return nlohmann::json::array();  // 유효하지 않은 줄 - 빈 배열 반환
    }

    const std::string& targetLine = lines[line];

    // LSP character position을 바이트 오프셋으로 변환
    size_t byteOffset = characterToByteOffset(targetLine, character);
    if (byteOffset >= targetLine.length()) {
        return nlohmann::json::array();
    }

    // 바이트 오프셋 위치에서 단어 추출
    std::string symbolName = extractWordAtOffset(targetLine, byteOffset);
    if (symbolName.empty()) {
        return nlohmann::json::array();
    }

    // 스코프 감지 (definition/hover와 동일한 로직)
    std::string currentScope = "global";
    auto allFunctions = symbolTable_.getAllSymbolsByKind(SymbolKind::Function);

    for (const auto& func : allFunctions) {
        int funcDefLine = func.definitionLocation.line - 1;

        if (line > funcDefLine) {
            bool insideFunc = false;
            int braceCount = 0;

            for (int i = funcDefLine; i <= line && i < static_cast<int>(lines.size()); ++i) {
                const std::string& checkLine = lines[i];

                for (char c : checkLine) {
                    if (c == '{') {
                        braceCount++;
                        if (braceCount == 1 && i == funcDefLine) {
                            insideFunc = true;
                        }
                    }
                    else if (c == '}') {
                        braceCount--;
                        if (braceCount == 0 && insideFunc) {
                            insideFunc = false;
                        }
                    }
                }

                if (i == line && insideFunc && braceCount > 0) {
                    currentScope = "함수:" + func.name;
                    break;
                }
            }

            if (currentScope != "global") {
                break;
            }
        }
    }

    // 현재 스코프에서 먼저 찾기
    std::optional<Symbol> symbol;
    if (currentScope != "global") {
        symbol = symbolTable_.findSymbolInScope(symbolName, currentScope);
    }

    // 로컬에서 못 찾으면 전역에서 찾기
    if (!symbol.has_value()) {
        symbol = symbolTable_.findSymbol(symbolName);
    }

    if (!symbol.has_value()) {
        // 심볼을 찾지 못함 - 빈 배열 반환
        return nlohmann::json::array();
    }

    // 참조 목록 가져오기
    auto references = symbolTable_.getReferences(symbolName);

    // 스코프 필터링: 현재 심볼과 같은 스코프의 참조만
    nlohmann::json result = nlohmann::json::array();

    // includeDeclaration이 true면 정의 위치도 추가
    if (includeDeclaration) {
        result.push_back({
            {"uri", symbol->definitionLocation.uri},
            {"range", {
                {"start", {
                    {"line", symbol->definitionLocation.line - 1},
                    {"character", symbol->definitionLocation.column - 1}
                }},
                {"end", {
                    {"line", symbol->definitionLocation.line - 1},
                    {"character", symbol->definitionLocation.column - 1 + static_cast<int>(symbol->name.length())}
                }}
            }}
        });
    }

    // 참조 추가 (스코프 필터링)
    for (const auto& ref : references) {
        // 참조가 현재 심볼과 같은 스코프에 있는지 확인
        // 참조의 줄 번호로 스코프 판단
        int refLine = ref.line - 1;  // 0-based
        std::string refScope = "global";

        // 참조가 어느 함수 안에 있는지 확인
        for (const auto& func : allFunctions) {
            int funcDefLine = func.definitionLocation.line - 1;

            if (refLine > funcDefLine) {
                bool insideFunc = false;
                int braceCount = 0;

                for (int i = funcDefLine; i <= refLine && i < static_cast<int>(lines.size()); ++i) {
                    const std::string& checkLine = lines[i];

                    for (char c : checkLine) {
                        if (c == '{') {
                            braceCount++;
                            if (braceCount == 1 && i == funcDefLine) {
                                insideFunc = true;
                            }
                        }
                        else if (c == '}') {
                            braceCount--;
                            if (braceCount == 0 && insideFunc) {
                                insideFunc = false;
                            }
                        }
                    }

                    if (i == refLine && insideFunc && braceCount > 0) {
                        refScope = "함수:" + func.name;
                        break;
                    }
                }

                if (refScope != "global") {
                    break;
                }
            }
        }

        // 같은 스코프의 참조만 추가
        if (refScope == symbol->scope) {
            result.push_back({
                {"uri", ref.uri},
                {"range", {
                    {"start", {
                        {"line", ref.line - 1},
                        {"character", ref.column - 1}
                    }},
                    {"end", {
                        {"line", ref.line - 1},
                        {"character", ref.column - 1 + static_cast<int>(symbolName.length())}
                    }}
                }}
            });
        }
    }

    return result;
}

nlohmann::json LanguageServer::handleTextDocumentRename(const nlohmann::json& params)
{
    // params 검증
    if (!params.contains("textDocument")) {
        throw std::runtime_error("Missing textDocument parameter");
    }
    if (!params.contains("position")) {
        throw std::runtime_error("Missing position parameter");
    }
    if (!params.contains("newName")) {
        throw std::runtime_error("Missing newName parameter");
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
    if (!params["newName"].is_string()) {
        throw std::runtime_error("Invalid newName");
    }

    std::string uri = textDocument["uri"];
    int line = position["line"];
    int character = position["character"];
    std::string newName = params["newName"];

    // 빈 이름 거부
    if (newName.empty()) {
        return nullptr;
    }

    // 문서 가져오기
    const auto* doc = getDocument(uri);
    if (!doc) {
        throw std::runtime_error("Document not found: " + uri);
    }

    // 커서 위치에서 심볼 이름 추출
    const std::string& content = doc->content;

    // 줄 가져오기
    auto lines = splitLines(content);
    if (line < 0 || line >= static_cast<int>(lines.size())) {
        return nullptr;  // 유효하지 않은 줄
    }

    const std::string& targetLine = lines[line];

    // LSP character position을 바이트 오프셋으로 변환
    size_t byteOffset = characterToByteOffset(targetLine, character);
    if (byteOffset >= targetLine.length()) {
        return nullptr;
    }

    // 바이트 오프셋 위치에서 단어 추출
    std::string symbolName = extractWordAtOffset(targetLine, byteOffset);
    if (symbolName.empty()) {
        return nullptr;
    }

    // 스코프 감지 (references와 동일한 로직)
    std::string currentScope = "global";
    auto allFunctions = symbolTable_.getAllSymbolsByKind(SymbolKind::Function);

    for (const auto& func : allFunctions) {
        int funcDefLine = func.definitionLocation.line - 1;

        if (line > funcDefLine) {
            bool insideFunc = false;
            int braceCount = 0;

            for (int i = funcDefLine; i <= line && i < static_cast<int>(lines.size()); ++i) {
                const std::string& checkLine = lines[i];

                for (char c : checkLine) {
                    if (c == '{') {
                        braceCount++;
                        if (braceCount == 1 && i == funcDefLine) {
                            insideFunc = true;
                        }
                    }
                    else if (c == '}') {
                        braceCount--;
                        if (braceCount == 0 && insideFunc) {
                            insideFunc = false;
                        }
                    }
                }

                if (i == line && insideFunc && braceCount > 0) {
                    currentScope = "함수:" + func.name;
                    break;
                }
            }

            if (currentScope != "global") {
                break;
            }
        }
    }

    // 현재 스코프에서 먼저 찾기
    std::optional<Symbol> symbol;
    if (currentScope != "global") {
        symbol = symbolTable_.findSymbolInScope(symbolName, currentScope);
    }

    // 로컬에서 못 찾으면 전역에서 찾기
    if (!symbol.has_value()) {
        symbol = symbolTable_.findSymbol(symbolName);
    }

    if (!symbol.has_value()) {
        // 심볼을 찾지 못함 - null 반환
        return nullptr;
    }

    // 참조 목록 가져오기
    auto references = symbolTable_.getReferences(symbolName);

    // WorkspaceEdit 생성
    nlohmann::json edits = nlohmann::json::array();

    // 1. 정의 위치 편집 추가
    edits.push_back({
        {"range", {
            {"start", {
                {"line", symbol->definitionLocation.line - 1},
                {"character", symbol->definitionLocation.column - 1}
            }},
            {"end", {
                {"line", symbol->definitionLocation.line - 1},
                {"character", symbol->definitionLocation.column - 1 + static_cast<int>(symbol->name.length())}
            }}
        }},
        {"newText", newName}
    });

    // 2. 참조 위치 편집 추가 (스코프 필터링)
    for (const auto& ref : references) {
        // 참조가 현재 심볼과 같은 스코프에 있는지 확인
        int refLine = ref.line - 1;  // 0-based
        std::string refScope = "global";

        // 참조가 어느 함수 안에 있는지 확인
        for (const auto& func : allFunctions) {
            int funcDefLine = func.definitionLocation.line - 1;

            if (refLine > funcDefLine) {
                bool insideFunc = false;
                int braceCount = 0;

                for (int i = funcDefLine; i <= refLine && i < static_cast<int>(lines.size()); ++i) {
                    const std::string& checkLine = lines[i];

                    for (char c : checkLine) {
                        if (c == '{') {
                            braceCount++;
                            if (braceCount == 1 && i == funcDefLine) {
                                insideFunc = true;
                            }
                        }
                        else if (c == '}') {
                            braceCount--;
                            if (braceCount == 0 && insideFunc) {
                                insideFunc = false;
                            }
                        }
                    }

                    if (i == refLine && insideFunc && braceCount > 0) {
                        refScope = "함수:" + func.name;
                        break;
                    }
                }

                if (refScope != "global") {
                    break;
                }
            }
        }

        // 같은 스코프의 참조만 추가
        if (refScope == symbol->scope) {
            edits.push_back({
                {"range", {
                    {"start", {
                        {"line", ref.line - 1},
                        {"character", ref.column - 1}
                    }},
                    {"end", {
                        {"line", ref.line - 1},
                        {"character", ref.column - 1 + static_cast<int>(symbolName.length())}
                    }}
                }},
                {"newText", newName}
            });
        }
    }

    // WorkspaceEdit 반환
    return {
        {"changes", {
            {uri, edits}
        }}
    };
}

} // namespace lsp
} // namespace kingsejong
