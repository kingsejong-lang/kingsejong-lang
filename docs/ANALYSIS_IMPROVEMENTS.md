# 개선 방안 및 로드맵

**문서**: ANALYSIS_IMPROVEMENTS.md
**작성일**: 2025-11-15
**목적**: 아키텍처 개선 계획 및 실행 로드맵

---

## 목차

1. [우선순위별 개선안](#1-우선순위별-개선안)
2. [Semantic Analyzer 설계](#2-semantic-analyzer-설계)
3. [리팩토링 로드맵](#3-리팩토링-로드맵)
4. [즉시 실행 가능한 조치](#4-즉시-실행-가능한-조치)
5. [장기 비전](#5-장기-비전)

---

## 1. 우선순위별 개선안

### 1.1 긴급 (P0) - 구조적 결함 수정

#### P0-1: Semantic Analyzer 도입 ⭐⭐⭐⭐⭐

**목표**: Parser에서 의미론적 판단 분리

**현재 문제**:
```cpp
// Parser.cpp
static bool isLikelyLoopVariable(const std::string& str) {
    // 휴리스틱으로 루프 변수 판단
}
```

**개선안**:
```cpp
// src/semantic/SemanticAnalyzer.h (신규)
class SemanticAnalyzer {
private:
    SymbolTable symbolTable_;
    TypeChecker typeChecker_;
    std::vector<std::string> errors_;

public:
    void analyze(Program* program);

private:
    void resolveSymbols(Node* node);
    void checkTypes(Node* node);
    void resolveAmbiguity(Statement* stmt);
};
```

**구현 예시**:
```cpp
void SemanticAnalyzer::analyze(Program* program) {
    // Phase 1: Symbol Table 구축
    for (auto* stmt : program->statements()) {
        if (auto varDecl = dynamic_cast<VarDeclaration*>(stmt)) {
            symbolTable_.define(
                varDecl->name(),
                SymbolKind::VARIABLE,
                varDecl->type()
            );
        }
        else if (auto funcDecl = dynamic_cast<FunctionDeclaration*>(stmt)) {
            symbolTable_.define(
                funcDecl->name(),
                SymbolKind::FUNCTION,
                funcDecl->returnType()
            );
        }
    }

    // Phase 2: 의미론적 검증
    for (auto* stmt : program->statements()) {
        resolveSymbols(stmt);
        checkTypes(stmt);
    }

    // Phase 3: 모호성 해결
    for (auto* stmt : program->statements()) {
        resolveAmbiguity(stmt);
    }
}

void SemanticAnalyzer::resolveAmbiguity(Statement* stmt) {
    // "i가" 패턴 → 루프문 vs 조사 표현식
    if (auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt)) {
        if (auto josaExpr = dynamic_cast<JosaExpression*>(exprStmt->expression())) {
            std::string name = josaExpr->identifier();
            std::string josa = josaExpr->josa();

            if (josa == "가" || josa == "이") {
                // Symbol Table에서 확인
                if (symbolTable_.isDefined(name)) {
                    // 이미 정의된 변수 → 조사 표현식
                    // 그대로 유지
                } else {
                    // 미정의 → 루프 변수 가능성
                    // 다음 토큰 확인 (부터, 까지)
                    // → RangeForStatement로 변환
                }
            }
        }
    }
}
```

**우선순위**: **최우선** (모든 문제의 근본 원인)

**예상 공수**: 2주

**효과**:
- ✅ 휴리스틱 제거 (`isLikelyLoopVariable`)
- ✅ 타입 안전성 향상
- ✅ Parser 단순화

---

#### P0-2: Symbol Table 구현 ⭐⭐⭐⭐⭐

**목표**: 변수/함수 정확히 추적

**구현**:
```cpp
// src/semantic/SymbolTable.h (신규)
enum class SymbolKind {
    VARIABLE,
    FUNCTION,
    TYPE,
    MODULE
};

struct Symbol {
    std::string name;
    SymbolKind kind;
    Type* type;
    Scope* scope;
    bool isMutable;  // 상수 vs 변수
};

class Scope {
private:
    Scope* parent_;
    std::unordered_map<std::string, Symbol> symbols_;

public:
    Scope(Scope* parent = nullptr) : parent_(parent) {}

    bool define(const Symbol& symbol) {
        if (symbols_.count(symbol.name)) {
            return false;  // 이미 정의됨
        }
        symbols_[symbol.name] = symbol;
        return true;
    }

    Symbol* lookup(const std::string& name) {
        if (symbols_.count(name)) {
            return &symbols_[name];
        }
        if (parent_) {
            return parent_->lookup(name);  // 부모 스코프에서 찾기
        }
        return nullptr;  // 미정의
    }

    bool isDefined(const std::string& name) {
        return lookup(name) != nullptr;
    }
};

class SymbolTable {
private:
    Scope* currentScope_;
    std::vector<std::unique_ptr<Scope>> scopes_;

public:
    SymbolTable() {
        enterScope();  // 전역 스코프
    }

    void enterScope() {
        auto scope = std::make_unique<Scope>(currentScope_);
        currentScope_ = scope.get();
        scopes_.push_back(std::move(scope));
    }

    void exitScope() {
        if (currentScope_->parent()) {
            currentScope_ = currentScope_->parent();
        }
    }

    bool define(const std::string& name, SymbolKind kind, Type* type) {
        return currentScope_->define(Symbol{name, kind, type, currentScope_, true});
    }

    Symbol* lookup(const std::string& name) {
        return currentScope_->lookup(name);
    }

    bool isDefined(const std::string& name) {
        return currentScope_->isDefined(name);
    }
};
```

**사용 예시**:
```cpp
// Semantic Analyzer에서
symbolTable_.enterScope();  // 함수 스코프 진입

for (auto* param : funcDecl->parameters()) {
    symbolTable_.define(param->name(), SymbolKind::VARIABLE, param->type());
}

for (auto* stmt : funcDecl->body()->statements()) {
    analyze(stmt);
}

symbolTable_.exitScope();  // 함수 스코프 탈출
```

**우선순위**: **최우선**

**예상 공수**: 1주

**효과**:
- ✅ 변수 재정의 검사
- ✅ 미정의 변수 검출
- ✅ 스코프 관리

---

#### P0-3: 위치 정보 추가 ⭐⭐⭐⭐

**목표**: 에러 메시지에 line, column 표시

**구현**:
```cpp
// src/ast/SourceLocation.h (신규)
struct SourceLocation {
    int line;
    int column;
    int length;
    std::string filename;

    SourceLocation(int line = 0, int column = 0, int length = 0, std::string filename = "")
        : line(line), column(column), length(length), filename(std::move(filename)) {}

    std::string toString() const {
        return filename + ":" + std::to_string(line) + ":" + std::to_string(column);
    }
};

// src/lexer/Token.h 수정
struct Token {
    TokenType type;
    std::string literal;
    SourceLocation location;  // 추가

    Token(TokenType type, std::string literal, SourceLocation location)
        : type(type), literal(std::move(literal)), location(location) {}
};

// src/ast/Node.h 수정
class Node {
protected:
    SourceLocation location_;

public:
    const SourceLocation& location() const { return location_; }
    void setLocation(const SourceLocation& loc) { location_ = loc; }
};

// Lexer에서 위치 추적
class Lexer {
private:
    int currentLine_;
    int currentColumn_;

    void readChar() {
        if (ch == '\n') {
            currentLine_++;
            currentColumn_ = 0;
        } else {
            currentColumn_++;
        }
        // ...
    }

    Token nextToken() {
        // ...
        SourceLocation loc(currentLine_, currentColumn_, literal.length());
        return Token(type, literal, loc);
    }
};

// Parser에서 AST에 위치 정보 설정
auto expr = std::make_unique<IntegerLiteral>(value);
expr->setLocation(curToken_.location);
```

**우선순위**: **긴급**

**예상 공수**: 3일

**효과**:
- ✅ 에러 메시지 개선
- ✅ IDE 통합 개선 (go-to-definition)
- ✅ 디버깅 용이

---

#### P0-4: Error Recovery 구현 ⭐⭐⭐⭐

**목표**: 여러 에러 한 번에 보고

**구현**:
```cpp
// Parser.h
class Parser {
private:
    std::vector<std::string> errors_;

    void synchronize();  // Panic Mode Recovery
};

// Parser.cpp
void Parser::synchronize() {
    // 동기화 토큰까지 건너뛰기
    while (!curTokenIs(TokenType::SEMICOLON) &&
           !curTokenIs(TokenType::EOF_TOKEN)) {
        // 블록 끝에서도 복구
        if (curTokenIs(TokenType::RBRACE)) {
            nextToken();
            return;
        }
        nextToken();
    }

    if (curTokenIs(TokenType::SEMICOLON)) {
        nextToken();
    }
}

std::unique_ptr<Expression> Parser::parseExpression(Precedence prec) {
    auto left = callPrefixFunction();

    if (!left) {
        // 에러 기록
        errors_.push_back("Failed to parse expression at " +
                         curToken_.location.toString());
        synchronize();  // 복구 시도
        return nullptr; // 계속 파싱
    }

    // ...
}

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();

    while (!curTokenIs(TokenType::EOF_TOKEN)) {
        auto stmt = parseStatement();
        if (stmt) {
            program->addStatement(std::move(stmt));
        }
        // stmt == nullptr이어도 계속 파싱 (에러는 이미 기록됨)
        nextToken();
    }

    return program;
}
```

**우선순위**: **긴급**

**예상 공수**: 1주

**효과**:
- ✅ 모든 에러 한 번에 보고
- ✅ 사용자 경험 향상
- ✅ 개발 속도 향상

---

### 1.2 중요 (P1) - 품질 개선

#### P1-1: 형태소 분석기 분리 ⭐⭐⭐⭐

**목표**: 조사 분리 정확도 향상

**현재 문제**:
- Lexer.cpp:162-253 (92줄) 휴리스틱
- "나이", "거리" 같은 2글자 명사 보호 불완전

**개선안**:
```cpp
// src/morphology/MorphologicalAnalyzer.h (신규)
struct Morpheme {
    std::string surface;   // 표층형 ("배열을")
    std::string base;      // 기본형 ("배열")
    std::string josa;      // 조사 ("을")
    MorphemeType type;     // NOUN, JOSA, VERB 등
};

class Dictionary {
private:
    std::unordered_set<std::string> nouns_;
    std::unordered_set<std::string> verbs_;
    std::unordered_set<std::string> josas_;

public:
    void loadFromFile(const std::string& filename);
    bool isNoun(const std::string& word) const;
    bool isVerb(const std::string& word) const;
    bool isJosa(const std::string& word) const;
};

class MorphologicalAnalyzer {
private:
    Dictionary dictionary_;

public:
    MorphologicalAnalyzer(const std::string& dictPath) {
        dictionary_.loadFromFile(dictPath);
    }

    std::vector<Morpheme> analyze(const std::string& word) {
        // 사전 확인
        if (dictionary_.isNoun(word)) {
            return {Morpheme{word, word, "", MorphemeType::NOUN}};
        }

        // 조사 분리 시도 (2글자 조사)
        if (word.length() >= 9) {  // 3글자 이상
            std::string last6 = word.substr(word.length() - 6);
            if (dictionary_.isJosa(last6)) {
                std::string base = word.substr(0, word.length() - 6);
                if (dictionary_.isNoun(base) || dictionary_.isVerb(base)) {
                    return {
                        Morpheme{word, base, last6, MorphemeType::NOUN},
                        Morpheme{word, last6, "", MorphemeType::JOSA}
                    };
                }
            }
        }

        // 1글자 조사
        if (word.length() >= 4) {
            std::string last3 = word.substr(word.length() - 3);
            if (dictionary_.isJosa(last3)) {
                std::string base = word.substr(0, word.length() - 3);
                if (dictionary_.isNoun(base) || dictionary_.isVerb(base)) {
                    return {
                        Morpheme{word, base, last3, MorphemeType::NOUN},
                        Morpheme{word, last3, "", MorphemeType::JOSA}
                    };
                }
            }
        }

        // 분리 불가 → 원형 반환
        return {Morpheme{word, word, "", MorphemeType::UNKNOWN}};
    }
};

// Lexer에서 사용
class Lexer {
private:
    MorphologicalAnalyzer morphAnalyzer_;

    std::string readIdentifier() {
        std::string identifier = readWhileLetterOrDigit();

        auto morphemes = morphAnalyzer_.analyze(identifier);
        if (morphemes.size() > 1) {
            // 조사 분리됨 → position 조정
            std::string base = morphemes[0].base;
            int josaBytes = morphemes[1].surface.length();
            position -= josaBytes;
            readPosition = position + 1;
            ch = input[position];
            return base;
        }

        return identifier;
    }
};
```

**사전 파일 예시**:
```
# nouns.txt
나이
거리
자리
배열
정수
문자열
...

# verbs.txt
정렬한다
출력한다
추가한다
...

# josas.txt
가
이
을
를
에
에서
으로
로
...
```

**우선순위**: **중요**

**예상 공수**: 2주

**효과**:
- ✅ 조사 분리 정확도 95% → 99%+
- ✅ 휴리스틱 제거
- ✅ 사전 추가로 지속 개선 가능

---

#### P1-2: Lookahead 확장 (LL(1) → LL(2)) ⭐⭐⭐

**목표**: 복잡한 문법 처리 가능

**구현**:
```cpp
// Parser.h
class Parser {
private:
    Token curToken_;
    Token peekToken_;
    Token peekPeekToken_;  // 추가

    void nextToken() {
        curToken_ = peekToken_;
        peekToken_ = peekPeekToken_;
        peekPeekToken_ = lexer_.nextToken();
    }

    bool peek2TokenIs(TokenType type) const {
        return peekPeekToken_.type == type;
    }
};

// 사용 예시
if (curTokenIs(TokenType::HAMSU) &&         // "함수"
    peekTokenIs(TokenType::IDENTIFIER) &&   // "이름"
    peek2TokenIs(TokenType::LPAREN)) {      // "("
    return parseFunctionDeclaration();
}
```

**우선순위**: **중요**

**예상 공수**: 3일

**효과**:
- ✅ 복잡한 문법 처리 가능
- ✅ 미래 확장성 향상

---

#### P1-3: 문법 개선 (모호성 제거) ⭐⭐⭐

**목표**: 휴리스틱 없이 파싱 가능한 문법

**현재 (모호함)**:
```javascript
i가 1부터 10까지 반복한다 { ... }
데이터가 존재한다
// "가" 토큰만으로는 구분 불가
```

**개선안 1: 명시적 키워드**:
```javascript
반복 (i가 1부터 10까지) { ... }
데이터가 존재한다
// "반복" 키워드로 시작 → 명확
```

**개선안 2: 다른 조사 사용**:
```javascript
i를 1부터 10까지 반복한다 { ... }
// "를" (목적격) → 주격 "가"와 구분
```

**우선순위**: **중요** (문법 변경은 신중)

**예상 공수**: 1주 (설계 + 토론)

**효과**:
- ✅ 모호성 제거
- ✅ 휴리스틱 불필요
- ⚠️ 기존 코드 마이그레이션 필요

---

### 1.3 개선 (P2) - 성능 및 기능

#### P2-1: GC 구현 (Mark-and-Sweep) ⭐⭐⭐

**목표**: 순환 참조 메모리 누수 방지

**구현**:
```cpp
// src/memory/GC.h (신규)
class Object {
protected:
    bool marked_;  // GC 마크

public:
    virtual ~Object() = default;
    virtual std::vector<Object*> references() = 0;  // 참조하는 객체들

    void mark() { marked_ = true; }
    void unmark() { marked_ = false; }
    bool isMarked() const { return marked_; }
};

class GarbageCollector {
private:
    std::vector<Object*> objects_;

public:
    template<typename T, typename... Args>
    T* allocate(Args&&... args) {
        T* obj = new T(std::forward<Args>(args)...);
        objects_.push_back(obj);
        return obj;
    }

    void collect(const std::vector<Object*>& roots) {
        // Mark Phase
        for (Object* root : roots) {
            mark(root);
        }

        // Sweep Phase
        for (auto it = objects_.begin(); it != objects_.end(); ) {
            if (!(*it)->isMarked()) {
                delete *it;
                it = objects_.erase(it);
            } else {
                (*it)->unmark();
                ++it;
            }
        }
    }

private:
    void mark(Object* obj) {
        if (!obj || obj->isMarked()) return;

        obj->mark();

        for (Object* ref : obj->references()) {
            mark(ref);
        }
    }
};
```

**우선순위**: **개선**

**예상 공수**: 2주

**효과**:
- ✅ 순환 참조 해결
- ✅ 메모리 누수 방지
- ⚠️ GC 일시 정지 (STW)

---

#### P2-2: JIT 컴파일러 (LLVM 백엔드) ⭐⭐⭐⭐⭐

**목표**: 10-100배 성능 향상

**구현** (POC):
```cpp
// src/jit/LLVMBackend.h (신규)
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

class LLVMBackend {
private:
    llvm::LLVMContext context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

public:
    LLVMBackend(const std::string& moduleName)
        : module_(std::make_unique<llvm::Module>(moduleName, context_)),
          builder_(std::make_unique<llvm::IRBuilder<>>(context_)) {}

    llvm::Function* compileFunctionDeclaration(FunctionDeclaration* funcDecl) {
        // LLVM 함수 타입 생성
        std::vector<llvm::Type*> paramTypes;
        for (auto* param : funcDecl->parameters()) {
            paramTypes.push_back(convertType(param->type()));
        }

        llvm::Type* returnType = convertType(funcDecl->returnType());
        llvm::FunctionType* funcType = llvm::FunctionType::get(
            returnType, paramTypes, false
        );

        // LLVM 함수 생성
        llvm::Function* llvmFunc = llvm::Function::Create(
            funcType, llvm::Function::ExternalLinkage,
            funcDecl->name(), module_.get()
        );

        // Entry 블록 생성
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(
            context_, "entry", llvmFunc
        );
        builder_->SetInsertPoint(entry);

        // 함수 body 컴파일
        for (auto* stmt : funcDecl->body()->statements()) {
            compileStatement(stmt);
        }

        return llvmFunc;
    }

    llvm::Value* compileStatement(Statement* stmt) {
        if (auto returnStmt = dynamic_cast<ReturnStatement*>(stmt)) {
            llvm::Value* retVal = compileExpression(returnStmt->value());
            return builder_->CreateRet(retVal);
        }
        // ...
    }

    llvm::Value* compileExpression(Expression* expr) {
        if (auto intLit = dynamic_cast<IntegerLiteral*>(expr)) {
            return llvm::ConstantInt::get(context_, llvm::APInt(64, intLit->value()));
        }
        else if (auto binExpr = dynamic_cast<BinaryExpression*>(expr)) {
            llvm::Value* left = compileExpression(binExpr->left());
            llvm::Value* right = compileExpression(binExpr->right());

            if (binExpr->op() == "+") {
                return builder_->CreateAdd(left, right, "addtmp");
            }
            else if (binExpr->op() == "*") {
                return builder_->CreateMul(left, right, "multmp");
            }
            // ...
        }
        // ...
    }

    llvm::Type* convertType(Type* type) {
        if (type->name() == "정수") {
            return llvm::Type::getInt64Ty(context_);
        }
        // ...
    }
};
```

**우선순위**: **개선** (장기 목표)

**예상 공수**: 2-3개월

**효과**:
- ✅ 10-100배 성능 향상
- ✅ 네이티브 코드 생성
- ⚠️ 복잡도 대폭 증가

---

## 2. Semantic Analyzer 설계

### 2.1 아키텍처

```
       ┌───────────────────────────────────┐
       │    Semantic Analyzer              │
       ├───────────────────────────────────┤
       │                                   │
       │  1. Symbol Table 구축             │
       │     - 변수/함수 등록               │
       │     - 스코프 관리                  │
       │                                   │
       │  2. 이름 해석 (Name Resolution)   │
       │     - 변수 참조 검증               │
       │     - 미정의 변수 검출              │
       │                                   │
       │  3. 타입 검사 (Type Checking)     │
       │     - 타입 일치성 검증             │
       │     - 타입 추론                    │
       │                                   │
       │  4. 모호성 해결                    │
       │     - Statement 구분               │
       │     - 연산자 오버로딩 해결          │
       │                                   │
       └───────────────────────────────────┘
```

### 2.2 주요 클래스

```cpp
class SemanticAnalyzer {
private:
    SymbolTable symbolTable_;
    TypeChecker typeChecker_;
    std::vector<SemanticError> errors_;

public:
    void analyze(Program* program);

private:
    // Phase 1: Symbol Table
    void buildSymbolTable(Program* program);
    void registerSymbol(Node* node);

    // Phase 2: Name Resolution
    void resolveNames(Program* program);
    void resolveIdentifier(Identifier* ident);

    // Phase 3: Type Checking
    void checkTypes(Program* program);
    Type* inferType(Expression* expr);
    void checkTypeCompatibility(Type* expected, Type* actual);

    // Phase 4: Ambiguity Resolution
    void resolveAmbiguities(Program* program);
    void resolveStatementAmbiguity(Statement* stmt);
};
```

### 2.3 실행 흐름

```cpp
// main.cpp
int main() {
    // 1. Lexing
    Lexer lexer(sourceCode);

    // 2. Parsing
    Parser parser(lexer);
    auto program = parser.parseProgram();

    if (!parser.errors().empty()) {
        // 파싱 에러 출력
        return 1;
    }

    // 3. Semantic Analysis (신규!)
    SemanticAnalyzer analyzer;
    analyzer.analyze(program.get());

    if (!analyzer.errors().empty()) {
        // 의미론적 에러 출력
        return 1;
    }

    // 4. Compilation/Evaluation
    Compiler compiler;
    auto bytecode = compiler.compile(program.get());

    VM vm;
    vm.run(bytecode);

    return 0;
}
```

---

## 3. 리팩토링 로드맵

### 3.1 Phase 1: 기반 강화 (2-3주)

**Week 1**: Semantic Analyzer 골격
```
- [ ] SemanticAnalyzer 클래스 생성
- [ ] SymbolTable 기본 구현
- [ ] Symbol, Scope 클래스
- [ ] 통합 테스트 작성
```

**Week 2**: Symbol Table 및 타입 검사
```
- [ ] 변수/함수 심볼 등록
- [ ] 스코프 관리 구현
- [ ] 타입 추론 기본 구현
- [ ] 미정의 변수 검출
```

**Week 3**: Error Handling 개선
```
- [ ] SourceLocation 추가
- [ ] Token에 위치 정보
- [ ] AST에 위치 정보
- [ ] Error Recovery 구현
```

### 3.2 Phase 2: 품질 개선 (2주)

**Week 4**: 형태소 분석기 분리
```
- [ ] MorphologicalAnalyzer 설계
- [ ] Dictionary 클래스
- [ ] 사전 파일 작성 (nouns, verbs, josas)
- [ ] Lexer 리팩토링 (휴리스틱 제거)
```

**Week 5**: 문법 모호성 해결
```
- [ ] Lookahead 확장 (LL(2))
- [ ] 문법 개선 검토 및 토론
- [ ] 회귀 테스트 추가
```

### 3.3 Phase 3: 성능 최적화 (장기, 2-6개월)

**Month 2-3**: 최적화 패스 강화
```
- [ ] Bytecode 최적화 (Peephole)
- [ ] Constant Folding 강화
- [ ] Dead Code Elimination
- [ ] 벤치마크 구축
```

**Month 4-6**: JIT 컴파일러 연구
```
- [ ] LLVM 백엔드 POC
- [ ] 간단한 함수 JIT 컴파일
- [ ] 성능 비교 분석
- [ ] 프로덕션 적용 검토
```

---

## 4. 즉시 실행 가능한 조치

### 4.1 KNOWN_ISSUES 검증 (30분)

```bash
# DISABLED 테스트 활성화
cd tests
# EvaluatorTest.cpp에서 DISABLED_ 제거
TEST_F(EvaluatorTest, ShouldEvaluateComplexExpression) {
    // (a + b) * c 테스트
}

# 테스트 실행
./build/bin/kingsejong_tests --gtest_filter="EvaluatorTest.ShouldEvaluateComplexExpression"
```

### 4.2 Symbol Table 프로토타입 (1일)

```cpp
// src/semantic/SymbolTable.h (신규)
#pragma once
#include <string>
#include <unordered_map>

enum class SymbolKind {
    VARIABLE,
    FUNCTION
};

class SymbolTable {
private:
    std::unordered_map<std::string, SymbolKind> symbols_;

public:
    bool isVariable(const std::string& name) const {
        auto it = symbols_.find(name);
        return it != symbols_.end() && it->second == SymbolKind::VARIABLE;
    }

    bool isFunction(const std::string& name) const {
        auto it = symbols_.find(name);
        return it != symbols_.end() && it->second == SymbolKind::FUNCTION;
    }

    void define(const std::string& name, SymbolKind kind) {
        symbols_[name] = kind;
    }
};

// Parser에서 즉시 활용
static SymbolTable globalSymbolTable;

static bool isLikelyLoopVariable(const std::string& str) {
    // 휴리스틱 대신 Symbol Table 확인
    return !globalSymbolTable.isVariable(str) && !globalSymbolTable.isFunction(str);
}
```

### 4.3 위치 정보 간단 추가 (1일)

```cpp
// Token 수정
struct Token {
    TokenType type;
    std::string literal;
    int line;     // 추가
    int column;   // 추가
};

// Lexer 수정
class Lexer {
private:
    int currentLine_ = 1;
    int currentColumn_ = 1;

    void readChar() {
        if (ch == '\n') {
            currentLine_++;
            currentColumn_ = 1;
        } else {
            currentColumn_++;
        }
        // ...
    }
};
```

---

## 5. 장기 비전

### 5.1 목표 (1년)

**Ruby MRI 수준 달성**:
- ✅ Semantic Analyzer 완비
- ✅ GC 구현
- ✅ 에러 처리 우수
- ✅ 성능: Ruby MRI 80% 수준

### 5.2 목표 (2-3년)

**CPython 수준 접근**:
- ✅ JIT 컴파일러 (LLVM)
- ✅ 고급 최적화
- ✅ 성능: CPython 수준
- ✅ 풍부한 표준 라이브러리

### 5.3 장기 비전 (5년+)

**세계적 DSL 언어**:
- 한국어 프로그래밍 표준
- 교육 현장 채택
- 오픈소스 커뮤니티
- 프로덕션 적용 사례

---

## 6. 결론

### 6.1 핵심 메시지

> **Semantic Analyzer 도입이 모든 문제의 해결책입니다.**
>
> 이 하나의 개선으로:
> - ✅ 휴리스틱 제거
> - ✅ 버그 감소
> - ✅ 타입 안전성 향상
> - ✅ 확장성 증가

### 6.2 우선순위 (다시 한번 강조)

1. **🔴 P0 (긴급)**: Semantic Analyzer, Symbol Table, 위치 정보, Error Recovery
2. **🟡 P1 (중요)**: 형태소 분석기, Lookahead, 문법 개선
3. **🟢 P2 (개선)**: GC, JIT, 최적화

### 6.3 첫 걸음

**오늘 시작할 수 있는 것**:
1. KNOWN_ISSUES 검증 (30분)
2. Symbol Table 프로토타입 (1일)
3. 위치 정보 간단 추가 (1일)

**내일부터**:
1. Semantic Analyzer 설계 (1주)
2. Symbol Table 완전 구현 (1주)
3. Error Recovery 구현 (1주)

**이번 달 목표**:
- Semantic Analyzer Phase 1 완료
- 모든 P0 항목 시작

---

**문서 끝**

**이전 문서**: [ANALYSIS_BUGS.md](./ANALYSIS_BUGS.md)
**메인 문서**: [ARCHITECTURE_ANALYSIS.md](./ARCHITECTURE_ANALYSIS.md)

---

**모든 분석 문서 작성 완료!**

**문서 목록**:
1. ARCHITECTURE_ANALYSIS.md - 전체 개요
2. ANALYSIS_LEXER.md - Lexer 상세 분석
3. ANALYSIS_PARSER.md - Parser 상세 분석
4. ANALYSIS_AST_EXECUTION.md - AST & 실행 엔진
5. ANALYSIS_COMPARISON.md - 타 인터프리터 비교
6. ANALYSIS_BUGS.md - 버그 근본 원인 분석
7. ANALYSIS_IMPROVEMENTS.md - 개선 방안 및 로드맵 (본 문서)
