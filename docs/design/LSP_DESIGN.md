# LSP (Language Server Protocol) 설계 문서

> **작성일**: 2025-11-12
> **작성자**: KingSejong Team
> **버전**: 1.0
> **Phase**: 5.3.1 - 기본 LSP 서버

---

## 1. 개요

### 1.1 목표
KingSejong 언어를 위한 LSP (Language Server Protocol) 서버를 구현하여 VS Code 등 에디터에서 다음 기능을 제공합니다:

- 🎨 **구문 강조** (Syntax Highlighting)
- ✨ **자동 완성** (Auto-completion)
- 🔍 **에러 진단** (Diagnostics)
- 📝 **호버 정보** (Hover - Week 5-6)
- 🔗 **정의로 이동** (Go to Definition - Week 5-6)

### 1.2 범위

**Week 3-4 (F5.3.1 기본 LSP)**:
- LSP 서버 기본 구조 (JSON-RPC 통신)
- 구문 강조 (TextMate 문법)
- 키워드 자동 완성
- 변수명 자동 완성
- 실시간 에러 진단

**Week 5-6 (F5.3.2 고급 LSP)**:
- 정의로 이동
- 참조 찾기
- 이름 변경
- 호버 정보

### 1.3 참고 자료
- [LSP 공식 스펙](https://microsoft.github.io/language-server-protocol/)
- [VS Code 확장 프로그램 가이드](https://code.visualstudio.com/api)

---

## 2. 아키텍처

### 2.1 전체 구조

```
┌─────────────────────────────────────────────────────────────┐
│                        VS Code                              │
│  ┌─────────────────────────────────────────────────────┐   │
│  │         KingSejong Extension (TypeScript)           │   │
│  │  - 구문 강조 (TextMate 문법)                         │   │
│  │  - LSP 클라이언트                                    │   │
│  └──────────────────┬──────────────────────────────────┘   │
└─────────────────────┼──────────────────────────────────────┘
                      │ JSON-RPC (stdio)
                      │
┌─────────────────────▼──────────────────────────────────────┐
│              LSP Server (C++)                               │
│  ┌──────────────────────────────────────────────────────┐  │
│  │            LanguageServer                            │  │
│  │  - initialize()                                      │  │
│  │  - textDocument/didOpen                              │  │
│  │  - textDocument/didChange                            │  │
│  │  - textDocument/completion                           │  │
│  │  - textDocument/publishDiagnostics                   │  │
│  └────────┬──────────────────┬──────────────────────────┘  │
│           │                  │                             │
│  ┌────────▼────────┐  ┌──────▼────────────┐               │
│  │ CompletionProvider│  │DiagnosticsProvider│               │
│  │  - 키워드 완성   │  │  - 구문 에러      │               │
│  │  - 변수명 완성   │  │  - 타입 에러      │               │
│  └─────────────────┘  └───────────────────┘               │
│           │                  │                             │
│  ┌────────▼──────────────────▼──────────────────────────┐  │
│  │          기존 KingSejong 컴포넌트                     │  │
│  │  - Lexer (토큰화)                                    │  │
│  │  - Parser (구문 분석)                                │  │
│  │  - TypeChecker (타입 검사)                           │  │
│  │  - Environment (변수 추적)                           │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 통신 프로토콜

**JSON-RPC 2.0** over stdin/stdout

#### 메시지 포맷
```
Content-Length: 123\r\n
\r\n
{"jsonrpc":"2.0","id":1,"method":"initialize","params":{...}}
```

---

## 3. 주요 컴포넌트

### 3.1 LanguageServer (메인 클래스)

```cpp
/**
 * @class LanguageServer
 * @brief LSP 서버 메인 클래스
 *
 * JSON-RPC 요청을 받아 처리하고 응답을 반환합니다.
 *
 * Thread Safety: NOT thread-safe (단일 스레드)
 * Memory: RAII - 모든 리소스 자동 관리
 */
class LanguageServer {
public:
    LanguageServer();
    ~LanguageServer() = default;

    /**
     * @brief LSP 서버 시작 (메인 루프)
     *
     * stdin에서 JSON-RPC 요청을 읽고 stdout으로 응답 전송
     *
     * @throws std::runtime_error if initialization fails
     */
    void run();

private:
    /**
     * @brief 요청 처리
     * @param request JSON-RPC 요청
     * @return JSON-RPC 응답
     */
    nlohmann::json handleRequest(const nlohmann::json& request);

    // LSP 메서드 핸들러
    nlohmann::json handleInitialize(const nlohmann::json& params);
    nlohmann::json handleTextDocumentDidOpen(const nlohmann::json& params);
    nlohmann::json handleTextDocumentDidChange(const nlohmann::json& params);
    nlohmann::json handleTextDocumentCompletion(const nlohmann::json& params);
    nlohmann::json handleShutdown();

    // 컴포넌트
    std::unique_ptr<DocumentManager> documentManager_;
    std::unique_ptr<CompletionProvider> completionProvider_;
    std::unique_ptr<DiagnosticsProvider> diagnosticsProvider_;

    bool initialized_;
};
```

### 3.2 DocumentManager (문서 관리)

```cpp
/**
 * @class DocumentManager
 * @brief 열린 문서들을 메모리에 관리
 *
 * LSP 클라이언트에서 열거나 수정한 문서를 추적합니다.
 */
class DocumentManager {
public:
    /**
     * @struct Document
     * @brief 문서 정보
     */
    struct Document {
        std::string uri;           // file:///path/to/file.ksj
        std::string content;       // 문서 내용
        int version;              // 문서 버전 (변경 추적용)
    };

    /**
     * @brief 문서 열기
     * @param uri 문서 URI
     * @param content 초기 내용
     * @param version 초기 버전
     */
    void openDocument(const std::string& uri,
                     const std::string& content,
                     int version);

    /**
     * @brief 문서 수정
     * @param uri 문서 URI
     * @param content 새 내용
     * @param version 새 버전
     */
    void updateDocument(const std::string& uri,
                       const std::string& content,
                       int version);

    /**
     * @brief 문서 닫기
     * @param uri 문서 URI
     */
    void closeDocument(const std::string& uri);

    /**
     * @brief 문서 조회
     * @param uri 문서 URI
     * @return Document 포인터 (없으면 nullptr)
     */
    const Document* getDocument(const std::string& uri) const;

private:
    std::map<std::string, Document> documents_;
};
```

### 3.3 CompletionProvider (자동 완성)

```cpp
/**
 * @class CompletionProvider
 * @brief 자동 완성 제안 제공
 *
 * 커서 위치에 따라 적절한 자동 완성 항목을 제안합니다.
 */
class CompletionProvider {
public:
    /**
     * @enum CompletionItemKind
     * @brief 완성 항목 종류 (LSP 표준)
     */
    enum class CompletionItemKind {
        Keyword = 14,      // 키워드 (변수, 함수, 만약 등)
        Variable = 6,      // 변수명
        Function = 3       // 함수명
    };

    /**
     * @struct CompletionItem
     * @brief 완성 항목
     */
    struct CompletionItem {
        std::string label;          // 표시 텍스트
        CompletionItemKind kind;    // 종류
        std::string detail;         // 상세 설명 (optional)
        std::string documentation;  // 문서 (optional)
    };

    /**
     * @brief 자동 완성 항목 제공
     * @param document 문서
     * @param line 줄 번호 (0부터 시작)
     * @param character 컬럼 번호 (0부터 시작)
     * @return 완성 항목 목록
     */
    std::vector<CompletionItem> provideCompletions(
        const DocumentManager::Document& document,
        int line,
        int character
    );

private:
    /**
     * @brief 키워드 완성 항목
     */
    std::vector<CompletionItem> getKeywordCompletions();

    /**
     * @brief 변수명 완성 항목
     * @param document 문서 (변수 추출용)
     */
    std::vector<CompletionItem> getVariableCompletions(
        const DocumentManager::Document& document
    );

    // 캐시된 키워드 목록
    static const std::vector<std::string> KEYWORDS;
};
```

### 3.4 DiagnosticsProvider (에러 진단)

```cpp
/**
 * @class DiagnosticsProvider
 * @brief 실시간 에러 진단 제공
 *
 * 문서를 파싱하여 구문 에러, 타입 에러 등을 찾아 반환합니다.
 */
class DiagnosticsProvider {
public:
    /**
     * @enum DiagnosticSeverity
     * @brief 진단 심각도 (LSP 표준)
     */
    enum class DiagnosticSeverity {
        Error = 1,        // 에러
        Warning = 2,      // 경고
        Information = 3,  // 정보
        Hint = 4         // 힌트
    };

    /**
     * @struct Diagnostic
     * @brief 진단 정보
     */
    struct Diagnostic {
        int startLine;               // 시작 줄 (0부터)
        int startCharacter;          // 시작 컬럼 (0부터)
        int endLine;                // 끝 줄
        int endCharacter;           // 끝 컬럼
        DiagnosticSeverity severity;// 심각도
        std::string message;        // 에러 메시지
        std::string source;         // 출처 ("kingsejong")
    };

    /**
     * @brief 문서 진단
     * @param document 문서
     * @return 진단 목록
     */
    std::vector<Diagnostic> provideDiagnostics(
        const DocumentManager::Document& document
    );

private:
    /**
     * @brief 구문 에러 검사
     */
    std::vector<Diagnostic> checkSyntaxErrors(
        const std::string& content
    );

    /**
     * @brief 에러 위치 변환 (Token → LSP Range)
     */
    void convertErrorLocation(
        const error::SourceLocation& loc,
        Diagnostic& diagnostic
    );
};
```

---

## 4. LSP 메서드 구현

### 4.1 initialize

클라이언트가 서버 초기화 요청

**요청**:
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "initialize",
  "params": {
    "processId": 1234,
    "rootUri": "file:///path/to/workspace",
    "capabilities": { ... }
  }
}
```

**응답**:
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "capabilities": {
      "textDocumentSync": 1,
      "completionProvider": {
        "triggerCharacters": ["."]
      },
      "diagnosticProvider": {
        "interFileDependencies": false,
        "workspaceDiagnostics": false
      }
    }
  }
}
```

### 4.2 textDocument/didOpen

문서 열림 알림

**알림** (응답 없음):
```json
{
  "jsonrpc": "2.0",
  "method": "textDocument/didOpen",
  "params": {
    "textDocument": {
      "uri": "file:///test.ksj",
      "languageId": "kingsejong",
      "version": 1,
      "text": "변수 x = 10\n출력(x)"
    }
  }
}
```

**서버 동작**:
1. 문서 저장 (DocumentManager)
2. 에러 진단 실행 (DiagnosticsProvider)
3. 진단 결과 발행 (textDocument/publishDiagnostics)

### 4.3 textDocument/didChange

문서 변경 알림

**알림** (응답 없음):
```json
{
  "jsonrpc": "2.0",
  "method": "textDocument/didChange",
  "params": {
    "textDocument": {
      "uri": "file:///test.ksj",
      "version": 2
    },
    "contentChanges": [
      {
        "text": "변수 x = 20\n출력(x)"
      }
    ]
  }
}
```

### 4.4 textDocument/completion

자동 완성 요청

**요청**:
```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "method": "textDocument/completion",
  "params": {
    "textDocument": { "uri": "file:///test.ksj" },
    "position": { "line": 0, "character": 3 }
  }
}
```

**응답**:
```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "result": {
    "items": [
      {
        "label": "변수",
        "kind": 14,
        "detail": "변수 선언 키워드"
      },
      {
        "label": "변환",
        "kind": 14,
        "detail": "타입 변환 함수"
      }
    ]
  }
}
```

### 4.5 textDocument/publishDiagnostics

에러 진단 발행 (서버 → 클라이언트)

**알림** (서버에서 전송):
```json
{
  "jsonrpc": "2.0",
  "method": "textDocument/publishDiagnostics",
  "params": {
    "uri": "file:///test.ksj",
    "diagnostics": [
      {
        "range": {
          "start": { "line": 0, "character": 5 },
          "end": { "line": 0, "character": 6 }
        },
        "severity": 1,
        "message": "정의되지 않은 변수 'y'",
        "source": "kingsejong"
      }
    ]
  }
}
```

---

## 5. VS Code 확장 프로그램

### 5.1 파일 구조

```
vscode-extension/
├── package.json          # 확장 프로그램 메타데이터
├── syntaxes/
│   └── kingsejong.tmLanguage.json  # TextMate 문법
├── src/
│   └── extension.ts      # 확장 프로그램 엔트리
└── client/
    └── src/
        └── extension.ts  # LSP 클라이언트
```

### 5.2 package.json

```json
{
  "name": "kingsejong",
  "displayName": "KingSejong Language Support",
  "description": "Language support for KingSejong",
  "version": "0.1.0",
  "engines": {
    "vscode": "^1.80.0"
  },
  "categories": ["Programming Languages"],
  "activationEvents": ["onLanguage:kingsejong"],
  "main": "./out/extension.js",
  "contributes": {
    "languages": [{
      "id": "kingsejong",
      "aliases": ["KingSejong", "kingsejong"],
      "extensions": [".ksj"],
      "configuration": "./language-configuration.json"
    }],
    "grammars": [{
      "language": "kingsejong",
      "scopeName": "source.kingsejong",
      "path": "./syntaxes/kingsejong.tmLanguage.json"
    }],
    "configuration": {
      "title": "KingSejong",
      "properties": {
        "kingsejong.lspServerPath": {
          "type": "string",
          "default": "kingsejong-lsp",
          "description": "LSP 서버 실행 파일 경로"
        }
      }
    }
  }
}
```

### 5.3 TextMate 문법 (구문 강조)

`syntaxes/kingsejong.tmLanguage.json`:
```json
{
  "name": "KingSejong",
  "scopeName": "source.kingsejong",
  "patterns": [
    { "include": "#keywords" },
    { "include": "#strings" },
    { "include": "#numbers" },
    { "include": "#comments" }
  ],
  "repository": {
    "keywords": {
      "patterns": [{
        "name": "keyword.control.kingsejong",
        "match": "\\b(변수|함수|반환|만약|아니면|반복|출력)\\b"
      }]
    },
    "strings": {
      "patterns": [{
        "name": "string.quoted.double.kingsejong",
        "begin": "\"",
        "end": "\"",
        "patterns": [{ "include": "#string-escape" }]
      }]
    },
    "numbers": {
      "patterns": [{
        "name": "constant.numeric.kingsejong",
        "match": "\\b\\d+(\\.\\d+)?\\b"
      }]
    },
    "comments": {
      "patterns": [{
        "name": "comment.line.number-sign.kingsejong",
        "match": "#.*$"
      }]
    }
  }
}
```

### 5.4 LSP 클라이언트

`client/src/extension.ts`:
```typescript
import * as vscode from 'vscode';
import { LanguageClient, LanguageClientOptions, ServerOptions } from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: vscode.ExtensionContext) {
    const serverPath = vscode.workspace.getConfiguration('kingsejong').get('lspServerPath') || 'kingsejong-lsp';

    const serverOptions: ServerOptions = {
        command: serverPath,
        args: []
    };

    const clientOptions: LanguageClientOptions = {
        documentSelector: [{ scheme: 'file', language: 'kingsejong' }],
        synchronize: {
            fileEvents: vscode.workspace.createFileSystemWatcher('**/*.ksj')
        }
    };

    client = new LanguageClient(
        'kingsejongLanguageServer',
        'KingSejong Language Server',
        serverOptions,
        clientOptions
    );

    client.start();
}

export function deactivate(): Thenable<void> | undefined {
    if (!client) {
        return undefined;
    }
    return client.stop();
}
```

---

## 6. 구현 계획 (TDD)

### Week 3-4: F5.3.1 기본 LSP

#### Day 1-2: JSON-RPC 통신 및 DocumentManager

**테스트** (tests/lsp/JsonRpcTest.cpp):
```cpp
TEST(JsonRpcTest, ShouldParseRequest) {
    std::string input = R"({"jsonrpc":"2.0","id":1,"method":"initialize"})";
    auto request = JsonRpc::parse(input);
    EXPECT_EQ(request["id"], 1);
    EXPECT_EQ(request["method"], "initialize");
}
```

**구현**:
- [ ] JSON 파싱 (nlohmann/json 라이브러리)
- [ ] Content-Length 헤더 처리
- [ ] DocumentManager 클래스

#### Day 3-4: LanguageServer 기본 구조

**테스트** (tests/lsp/LanguageServerTest.cpp):
```cpp
TEST(LanguageServerTest, ShouldHandleInitialize) {
    LanguageServer server;
    auto response = server.handleInitialize({});
    EXPECT_TRUE(response["result"]["capabilities"].contains("completionProvider"));
}
```

**구현**:
- [ ] LanguageServer 클래스
- [ ] initialize 핸들러
- [ ] textDocument/didOpen 핸들러
- [ ] textDocument/didChange 핸들러

#### Day 5-7: CompletionProvider

**테스트** (tests/lsp/CompletionProviderTest.cpp):
```cpp
TEST(CompletionProviderTest, ShouldProvideKeywordCompletions) {
    CompletionProvider provider;
    auto items = provider.getKeywordCompletions();
    EXPECT_GT(items.size(), 0);
    EXPECT_TRUE(containsLabel(items, "변수"));
}
```

**구현**:
- [ ] CompletionProvider 클래스
- [ ] 키워드 자동 완성
- [ ] 변수명 추출 및 자동 완성

#### Day 8-10: DiagnosticsProvider

**테스트** (tests/lsp/DiagnosticsProviderTest.cpp):
```cpp
TEST(DiagnosticsProviderTest, ShouldDetectSyntaxError) {
    DiagnosticsProvider provider;
    std::string code = "변수 x = ";  // 문법 에러
    auto diagnostics = provider.provideDiagnostics(code);
    EXPECT_GT(diagnostics.size(), 0);
    EXPECT_EQ(diagnostics[0].severity, DiagnosticSeverity::Error);
}
```

**구현**:
- [ ] DiagnosticsProvider 클래스
- [ ] Lexer/Parser 통합
- [ ] 에러 위치 변환

#### Day 11-14: VS Code 확장 프로그램

**작업**:
- [ ] package.json 작성
- [ ] TextMate 문법 작성
- [ ] LSP 클라이언트 구현
- [ ] 로컬 테스트

---

## 7. 테스트 전략

### 7.1 단위 테스트
- JsonRpc 파싱
- DocumentManager 문서 관리
- CompletionProvider 완성 항목
- DiagnosticsProvider 에러 검출

### 7.2 통합 테스트
- LSP 서버 전체 메시지 흐름
- 실제 KingSejong 코드 진단

### 7.3 수동 테스트
- VS Code에서 확장 프로그램 설치
- 구문 강조 확인
- 자동 완성 동작 확인
- 에러 표시 확인

---

## 8. 의존성

### 8.1 라이브러리
- **nlohmann/json**: JSON 파싱 (헤더 온리)
- **기존 KingSejong 컴포넌트**: Lexer, Parser, TypeChecker

### 8.2 VS Code 확장
- **vscode**: VS Code API
- **vscode-languageclient**: LSP 클라이언트

---

## 9. 성능 고려사항

### 9.1 최적화 전략
- 문서 변경 시 증분 파싱 (전체 재파싱 피함)
- 자동 완성 캐싱 (키워드는 정적)
- 진단은 비동기 (타이핑 방해 안 함)

### 9.2 제약사항
- 대용량 파일 (10,000줄 이상): 지원하지 않음
- 멀티 파일 분석: Week 5-6에 추가

---

## 10. 완료 조건

### F5.3.1 기본 LSP
- [ ] LSP 서버 빌드 성공 (`kingsejong-lsp`)
- [ ] VS Code 확장 프로그램 설치 가능
- [ ] .ksj 파일 열면 구문 강조 동작
- [ ] 키워드 자동 완성 동작 (변수, 함수, 만약 등)
- [ ] 구문 에러 빨간 밑줄 표시
- [ ] 30+ 단위 테스트 통과
- [ ] 통합 테스트 통과

---

## 11. 참고: LSP 스펙 요약

### 11.1 필수 구현 메서드
- [x] initialize
- [x] initialized
- [x] shutdown
- [x] exit
- [x] textDocument/didOpen
- [x] textDocument/didChange
- [x] textDocument/completion
- [ ] textDocument/publishDiagnostics

### 11.2 선택 구현 메서드 (Week 5-6)
- [ ] textDocument/hover
- [ ] textDocument/definition
- [ ] textDocument/references
- [ ] textDocument/rename

---

**다음 단계**: JSON-RPC 통신 및 DocumentManager 구현 시작
