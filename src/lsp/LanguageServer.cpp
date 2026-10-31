/**
 * @file LanguageServer.cpp
 * @brief LSP 언어 서버 구현
 * @author KingSejong Team
 * @date 2025-11-12
 */

#include "lsp/LanguageServer.h"
#include "lsp/SymbolCollector.h"
#include "version/Version.h"
#include "lsp/LspUtils.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include <stdexcept>
#include <optional>
#include <sstream>
#include <cctype>
#include <filesystem>

namespace kingsejong {
namespace lsp {

LanguageServer::LanguageServer()
    : initialized_(false)
    , shutdown_(false)
{
    // 린터 진단은 기본으로 켠다 (initializationOptions·설정 변경으로 끌 수 있다)
    diagnosticsProvider_.setLintEnabled(true);
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
        else if (method == "workspace/didChangeConfiguration") {
            handleDidChangeConfiguration(params);
            return nlohmann::json();  // notification - no response
        }
        else if (method == "workspace/didChangeWatchedFiles") {
            handleDidChangeWatchedFiles(params);
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
    // initializationOptions: { "lint": { "enable": bool } }
    if (params.contains("initializationOptions") && params["initializationOptions"].is_object()) {
        applyLintSettings(params["initializationOptions"]);
    }

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
            {"version", version::VERSION}
        }}
    };
}

void LanguageServer::handleInitialized(const nlohmann::json& params)
{
    // initialized notification - 아무 작업 없음
    (void)params;
}

void LanguageServer::handleDidChangeConfiguration(const nlohmann::json& params)
{
    // settings: { "kingsejong": { "lint": { "enable": bool }, ... } }
    if (!params.contains("settings") || !params["settings"].is_object()) {
        return;
    }
    const auto& settings = params["settings"];
    if (!settings.contains("kingsejong") || !settings["kingsejong"].is_object()) {
        return;
    }
    if (!applyLintSettings(settings["kingsejong"])) {
        return;
    }
    // 바뀐 설정으로 열린 문서의 진단을 다시 보낸다 (끄면 린터 밑줄이 지워진다)
    for (const auto& [uri, document] : documentManager_.getAllDocuments()) {
        (void)document;
        queueDiagnostics(uri);
    }
}

void LanguageServer::handleDidChangeWatchedFiles(const nlohmann::json& params)
{
    // changes: [{ "uri": "...", "type": 1|2|3 }] 중 .ksjlint.json 이 있으면 린터 설정이 바뀐 것이다
    if (!diagnosticsProvider_.isLintEnabled() || !params.contains("changes") || !params["changes"].is_array()) {
        return;
    }
    bool lintConfigChanged = false;
    for (const auto& change : params["changes"]) {
        if (!change.contains("uri") || !change["uri"].is_string()) continue;
        // URI 를 경로로 풀어 파일 이름만 비교한다 (Windows 의 `\` 구분자도 같이 다룬다)
        const std::string path = uriToFilePath(change["uri"].get<std::string>());
        if (!path.empty() && std::filesystem::path(path).filename() == ".ksjlint.json") {
            lintConfigChanged = true;
            break;
        }
    }
    if (!lintConfigChanged) {
        return;
    }
    // 설정 파일은 진단할 때마다 다시 읽으므로, 열린 문서의 진단만 다시 보내면 된다
    for (const auto& [uri, document] : documentManager_.getAllDocuments()) {
        (void)document;
        queueDiagnostics(uri);
    }
}

bool LanguageServer::applyLintSettings(const nlohmann::json& options)
{
    if (!options.contains("lint") || !options["lint"].is_object()) {
        return false;
    }
    const auto& lint = options["lint"];
    if (!lint.contains("enable") || !lint["enable"].is_boolean()) {
        return false;
    }
    bool enabled = lint["enable"].get<bool>();
    if (enabled == diagnosticsProvider_.isLintEnabled()) {
        return false;
    }
    diagnosticsProvider_.setLintEnabled(enabled);
    return true;
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

    // 구문 진단을 클라이언트에 보낸다
    queueDiagnostics(uri);

    // 심볼 테이블 업데이트
    try {
        lexer::Lexer lexer(text);
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();

        if (program) {
            symbolTable_.clear();  // 기존 심볼 제거
            SymbolCollector collector(symbolTable_);
            collector.collect(program.get(), uri, text);
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

    // 구문 진단을 클라이언트에 보낸다
    queueDiagnostics(uri);

    // 심볼 테이블 업데이트
    try {
        lexer::Lexer lexer(newText);
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();

        if (program) {
            symbolTable_.clear();  // 기존 심볼 제거
            SymbolCollector collector(symbolTable_);
            collector.collect(program.get(), uri, newText);
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

std::vector<nlohmann::json> LanguageServer::takePendingNotifications()
{
    std::vector<nlohmann::json> out;
    out.swap(pendingNotifications_);
    return out;
}

void LanguageServer::queueDiagnostics(const std::string& uri)
{
    const auto* doc = documentManager_.getDocument(uri);
    if (!doc) {
        return;
    }
    // 진단이 비어 있어도 보내야 이전 에러 밑줄이 지워진다
    auto diagnostics = diagnosticsProvider_.provideDiagnostics(*doc);
    pendingNotifications_.push_back(createPublishDiagnosticsNotification(uri, diagnostics));
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
        if (!diag.code.empty()) {
            jsonDiag["code"] = diag.code;  // 린터 규칙 이름
        }

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


namespace {

/// LSP 위치(0-based 줄, UTF-16 문자)에 있는 식별자가 가리키는 심볼.
/// 식별자의 시작 위치를 심볼 테이블의 정의·참조 위치와 맞춰 찾으므로, 같은 이름이 여러
/// 스코프에 있어도 커서가 놓인 바로 그 심볼이 나온다. 위치로 못 찾으면 이름으로 찾는다.
std::optional<Symbol> symbolAtPosition(const SymbolTable& table, const std::string& uri,
                                       const std::string& content, int line, int character)
{
    auto lines = splitLines(content);
    if (line < 0 || line >= static_cast<int>(lines.size())) {
        return std::nullopt;
    }
    const std::string& text = lines[static_cast<size_t>(line)];
    size_t byteOffset = characterToByteOffset(text, character);
    if (byteOffset >= text.length()) {
        return std::nullopt;
    }
    std::string word = extractWordAtOffset(text, byteOffset);
    if (word.empty()) {
        return std::nullopt;
    }
    size_t start = wordStartOffset(text, byteOffset);
    Location at(line + 1, byteColumnToUtf16(text, static_cast<int>(start) + 1) + 1, uri);
    if (auto found = table.findSymbolAtLocation(at)) {
        return found;
    }
    return table.findSymbol(word);
}

nlohmann::json rangeOf(const Location& location, const std::string& name)
{
    return {
        {"start", {{"line", location.line - 1}, {"character", location.column - 1}}},
        {"end", {{"line", location.line - 1}, {"character", location.column - 1 + utf16Length(name)}}}
    };
}

} // namespace

nlohmann::json LanguageServer::handleTextDocumentDefinition(const nlohmann::json& params)
{
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
    const auto* doc = getDocument(uri);
    if (!doc) {
        throw std::runtime_error("Document not found: " + uri);
    }

    auto symbol = symbolAtPosition(symbolTable_, uri, doc->content, position["line"], position["character"]);
    if (!symbol.has_value()) {
        return nullptr;
    }
    return {
        {"uri", symbol->definitionLocation.uri},
        {"range", rangeOf(symbol->definitionLocation, symbol->name)}
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
    bool includeDeclaration = context.contains("includeDeclaration") &&
                              context["includeDeclaration"].is_boolean() &&
                              context["includeDeclaration"].get<bool>();
    const auto* doc = getDocument(uri);
    if (!doc) {
        throw std::runtime_error("Document not found: " + uri);
    }

    auto symbol = symbolAtPosition(symbolTable_, uri, doc->content, position["line"], position["character"]);
    if (!symbol.has_value()) {
        return nlohmann::json::array();
    }

    // 참조는 수집할 때 이미 스코프별로 나뉘어 붙어 있다. 그대로 돌려주면 된다.
    nlohmann::json result = nlohmann::json::array();
    if (includeDeclaration) {
        result.push_back({
            {"uri", symbol->definitionLocation.uri},
            {"range", rangeOf(symbol->definitionLocation, symbol->name)}
        });
    }
    for (const auto& ref : symbol->references) {
        result.push_back({{"uri", ref.uri}, {"range", rangeOf(ref, symbol->name)}});
    }
    return result;
}

nlohmann::json LanguageServer::handleTextDocumentRename(const nlohmann::json& params)
{
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
        throw std::runtime_error("Missing or invalid newName");
    }

    std::string uri = textDocument["uri"];
    std::string newName = params["newName"];
    if (newName.empty()) {
        return nullptr;
    }
    const auto* doc = getDocument(uri);
    if (!doc) {
        throw std::runtime_error("Document not found: " + uri);
    }

    auto symbol = symbolAtPosition(symbolTable_, uri, doc->content, position["line"], position["character"]);
    if (!symbol.has_value()) {
        return nullptr;
    }

    // 정의와 참조를 모두 바꾼다. 다른 스코프의 같은 이름은 다른 심볼이라 건드리지 않는다.
    nlohmann::json edits = nlohmann::json::array();
    edits.push_back({{"range", rangeOf(symbol->definitionLocation, symbol->name)}, {"newText", newName}});
    for (const auto& ref : symbol->references) {
        edits.push_back({{"range", rangeOf(ref, symbol->name)}, {"newText", newName}});
    }
    return {{"changes", {{uri, edits}}}};
}

} // namespace lsp
} // namespace kingsejong
