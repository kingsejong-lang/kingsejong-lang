/**
 * @file SemanticAnalyzer.cpp
 * @brief 의미 분석기 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "semantic/SemanticAnalyzer.h"
#include <iostream>

namespace kingsejong {
namespace semantic {

using namespace ast;
using namespace types;

// ============================================================================
// Builtin 함수 초기화
// ============================================================================

void SemanticAnalyzer::initBuiltinFunctions()
{
    // 기본 I/O
    builtins_.insert("출력");
    builtins_.insert("입력");

    // 타입 및 변환
    builtins_.insert("타입");
    builtins_.insert("정수");
    builtins_.insert("실수");
    builtins_.insert("문자열");

    // 문자열 함수
    builtins_.insert("길이");
    builtins_.insert("분리");
    builtins_.insert("찾기");
    builtins_.insert("바꾸기");
    builtins_.insert("대문자");
    builtins_.insert("소문자");

    // 수학 함수
    builtins_.insert("반올림");
    builtins_.insert("올림");
    builtins_.insert("내림");
    builtins_.insert("절대값");
    builtins_.insert("제곱근");
    builtins_.insert("제곱");

    // 파일 I/O
    builtins_.insert("파일_읽기");
    builtins_.insert("파일_쓰기");
    builtins_.insert("파일_추가");
    builtins_.insert("파일_존재");
    builtins_.insert("줄별_읽기");

    // 배열 함수
    builtins_.insert("추가");
    builtins_.insert("삭제");
    builtins_.insert("정렬");
    builtins_.insert("뒤집기");

    // 환경변수
    builtins_.insert("환경변수_읽기");
    builtins_.insert("환경변수_쓰기");
    builtins_.insert("환경변수_존재하는가");
    builtins_.insert("환경변수_삭제");

    // 디렉토리
    builtins_.insert("현재_디렉토리");
    builtins_.insert("디렉토리_변경");
    builtins_.insert("디렉토리_생성");
    builtins_.insert("디렉토리_삭제");
    builtins_.insert("디렉토리_목록");
    builtins_.insert("디렉토리_나열");
    builtins_.insert("디렉토리인가");
    builtins_.insert("임시_디렉토리");

    // 파일 시스템
    builtins_.insert("파일_존재하는가");
    builtins_.insert("파일_삭제");
    builtins_.insert("파일_복사");
    builtins_.insert("파일_이동");
    builtins_.insert("파일_크기");
    builtins_.insert("파일인가");

    // 경로
    builtins_.insert("경로_결합");
    builtins_.insert("파일명_추출");
    builtins_.insert("확장자_추출");
    builtins_.insert("절대경로");
    builtins_.insert("경로_존재하는가");

    // 시스템 정보
    builtins_.insert("OS_이름");
    builtins_.insert("사용자_이름");
    builtins_.insert("호스트_이름");
    builtins_.insert("프로세스_종료");

    // JSON 처리
    builtins_.insert("JSON_파싱");
    builtins_.insert("JSON_문자열화");
    builtins_.insert("JSON_파일_읽기");
    builtins_.insert("JSON_파일_쓰기");

    // 시간/날짜
    builtins_.insert("현재_시간");
    builtins_.insert("현재_날짜");
    builtins_.insert("시간_포맷");
    builtins_.insert("타임스탬프");
    builtins_.insert("슬립");
    builtins_.insert("현재_시각");
    builtins_.insert("현재_날짜시간");

    // 정규표현식
    builtins_.insert("정규표현식_일치");
    builtins_.insert("정규표현식_검색");
    builtins_.insert("정규표현식_모두_찾기");
    builtins_.insert("정규표현식_치환");
    builtins_.insert("정규표현식_분리");
    builtins_.insert("이메일_검증");
    builtins_.insert("URL_검증");
    builtins_.insert("전화번호_검증");
    builtins_.insert("정규표현식_추출");
    builtins_.insert("정규표현식_개수");

    // 암호화
    builtins_.insert("Base64_인코딩");
    builtins_.insert("Base64_디코딩");
    builtins_.insert("문자열_해시");
    builtins_.insert("파일_해시");
    builtins_.insert("해시_비교");
    builtins_.insert("체크섬");
    builtins_.insert("XOR_암호화");
    builtins_.insert("XOR_복호화");
    builtins_.insert("시저_암호화");
    builtins_.insert("시저_복호화");
    builtins_.insert("랜덤_문자열");
    builtins_.insert("랜덤_숫자");

    // HTTP
    builtins_.insert("HTTP_GET");
    builtins_.insert("HTTP_POST");
    builtins_.insert("HTTP_요청");

    // SQLite 데이터베이스
    builtins_.insert("DB_열기");
    builtins_.insert("DB_닫기");
    builtins_.insert("DB_실행");
    builtins_.insert("DB_쿼리");
    builtins_.insert("DB_마지막_ID");
    builtins_.insert("DB_영향받은_행수");

    // 테스트 프레임워크
    builtins_.insert("assert");
    builtins_.insert("assert_같음");
    builtins_.insert("assert_다름");
    builtins_.insert("assert_참");
    builtins_.insert("assert_거짓");
}

bool SemanticAnalyzer::isBuiltinFunction(const std::string& name) const
{
    return builtins_.find(name) != builtins_.end();
}

// ============================================================================
// 메인 진입점
// ============================================================================

bool SemanticAnalyzer::analyze(Program* program)
{
    if (!program)
    {
        addError("Program is null");
        return false;
    }

    // 파일명 저장 (에러 메시지에 사용)
    filename_ = program->location().filename;

    // Phase 1+2: Symbol Table 구축 및 이름 해석 (통합)
    analyzeAndResolve(program);

    // Phase 3: 타입 검사
    checkTypes(program);

    // Phase 4: 모호성 해결
    resolveAmbiguities(program);

    // 에러가 없으면 성공
    return errors_.empty();
}

// ============================================================================
// Phase 1+2: Symbol Table 구축 및 이름 해석 (통합)
// ============================================================================

void SemanticAnalyzer::analyzeAndResolve(Program* program)
{
    if (!program) return;

    // 프로그램의 각 문장을 순회하며 분석
    for (auto& stmt : program->statements())
    {
        analyzeAndResolveStatement(stmt.get());
    }
}

void SemanticAnalyzer::analyzeAndResolveStatement(const Statement* stmt)
{
    if (!stmt) return;

    // 변수 선언: 현재 스코프에 등록
    if (auto varDecl = dynamic_cast<const VarDeclaration*>(stmt))
    {
        // 변수를 현재 스코프에 등록
        symbolTable_.define(varDecl->varName(), SymbolKind::VARIABLE, varDecl->varType());

        // 초기화 표현식이 있으면 이름 해석
        if (varDecl->initializer())
        {
            analyzeAndResolveExpression(varDecl->initializer());
        }
    }
    // 할당문 (함수 선언도 포함)
    else if (auto assignStmt = dynamic_cast<const AssignmentStatement*>(stmt))
    {
        // 함수 리터럴 할당인지 확인
        if (auto funcLit = dynamic_cast<const FunctionLiteral*>(assignStmt->value()))
        {
            // 함수 이름을 현재 스코프에 등록
            symbolTable_.define(assignStmt->varName(), SymbolKind::FUNCTION,
                              nullptr);

            // 함수 body를 새로운 스코프에서 분석
            symbolTable_.enterScope();

            // 매개변수를 함수 스코프에 등록
            for (const auto& param : funcLit->parameters())
            {
                symbolTable_.define(param, SymbolKind::VARIABLE,
                                  nullptr);  // 타입 추론 미지원
            }

            // 함수 body 분석
            if (funcLit->body())
            {
                analyzeAndResolveStatement(funcLit->body());
            }

            symbolTable_.exitScope();
        }
        else
        {
            // 일반 할당문: 변수가 정의되어 있지 않으면 현재 스코프에 등록 (동적 타이핑)
            if (!symbolTable_.lookup(assignStmt->varName()))
            {
                symbolTable_.define(assignStmt->varName(), SymbolKind::VARIABLE, nullptr);
            }

            // 할당 값 표현식 분석
            if (assignStmt->value())
            {
                analyzeAndResolveExpression(assignStmt->value());
            }
        }
    }
    // 블록문: 새로운 스코프 생성
    else if (auto blockStmt = dynamic_cast<const BlockStatement*>(stmt))
    {
        symbolTable_.enterScope();

        for (const auto& s : blockStmt->statements())
        {
            analyzeAndResolveStatement(s.get());
        }

        symbolTable_.exitScope();
    }
    // if문: then/else 각각 새로운 스코프
    else if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt))
    {
        // 조건 표현식 분석
        if (ifStmt->condition())
        {
            analyzeAndResolveExpression(ifStmt->condition());
        }

        // then 블록
        if (ifStmt->thenBranch())
        {
            symbolTable_.enterScope();
            analyzeAndResolveStatement(ifStmt->thenBranch());
            symbolTable_.exitScope();
        }

        // else 블록
        if (ifStmt->elseBranch())
        {
            symbolTable_.enterScope();
            analyzeAndResolveStatement(ifStmt->elseBranch());
            symbolTable_.exitScope();
        }
    }
    // while문: body를 새로운 스코프에서 분석
    else if (auto whileStmt = dynamic_cast<const WhileStatement*>(stmt))
    {
        // 조건 표현식 분석
        if (whileStmt->condition())
        {
            analyzeAndResolveExpression(whileStmt->condition());
        }

        // body
        if (whileStmt->body())
        {
            symbolTable_.enterScope();
            analyzeAndResolveStatement(whileStmt->body());
            symbolTable_.exitScope();
        }
    }
    // 범위 for문: 루프 변수를 블록 스코프에 등록
    else if (auto forStmt = dynamic_cast<const RangeForStatement*>(stmt))
    {
        symbolTable_.enterScope();

        // 루프 변수 등록
        symbolTable_.define(forStmt->varName(), SymbolKind::VARIABLE,
                          types::Type::integerType());

        // start, end 표현식 분석
        if (forStmt->start())
        {
            analyzeAndResolveExpression(forStmt->start());
        }
        if (forStmt->end())
        {
            analyzeAndResolveExpression(forStmt->end());
        }

        // body 분석
        if (forStmt->body())
        {
            analyzeAndResolveStatement(forStmt->body());
        }

        symbolTable_.exitScope();
    }
    // repeat문: body를 새로운 스코프에서 분석
    else if (auto repeatStmt = dynamic_cast<const RepeatStatement*>(stmt))
    {
        // 반복 횟수 표현식 분석
        if (repeatStmt->count())
        {
            analyzeAndResolveExpression(repeatStmt->count());
        }

        // body
        if (repeatStmt->body())
        {
            symbolTable_.enterScope();
            analyzeAndResolveStatement(repeatStmt->body());
            symbolTable_.exitScope();
        }
    }
    // return문
    else if (auto returnStmt = dynamic_cast<const ReturnStatement*>(stmt))
    {
        if (returnStmt->returnValue())
        {
            analyzeAndResolveExpression(returnStmt->returnValue());
        }
    }
    // 표현식문
    else if (auto exprStmt = dynamic_cast<const ExpressionStatement*>(stmt))
    {
        if (exprStmt->expression())
        {
            analyzeAndResolveExpression(exprStmt->expression());
        }
    }
    // import문 (스킵)
    else if (dynamic_cast<const ImportStatement*>(stmt))
    {
        // TODO: 모듈 import 처리
    }
    // Phase 7.1: 클래스 정의문
    else if (auto classStmt = dynamic_cast<const ClassStatement*>(stmt))
    {
        // 클래스 이름을 현재 스코프에 등록
        symbolTable_.define(classStmt->className(), SymbolKind::CLASS, nullptr);

        // 필드 검증 (중복 필드 이름 확인)
        std::unordered_set<std::string> fieldNames;
        for (const auto& field : classStmt->fields())
        {
            if (fieldNames.count(field->fieldName()))
            {
                addError("중복된 필드 이름: " + field->fieldName(),
                        stmt->location().line, stmt->location().column);
            }
            fieldNames.insert(field->fieldName());
        }

        // 메서드 검증 (중복 메서드 이름 확인)
        std::unordered_set<std::string> methodNames;
        for (const auto& method : classStmt->methods())
        {
            if (methodNames.count(method->methodName()))
            {
                addError("중복된 메서드 이름: " + method->methodName(),
                        stmt->location().line, stmt->location().column);
            }
            methodNames.insert(method->methodName());

            // 메서드 본문 분석 (새 스코프에서)
            symbolTable_.enterScope();

            // 매개변수 등록
            for (const auto& param : method->parameters())
            {
                symbolTable_.define(param.name, SymbolKind::VARIABLE, nullptr);
            }

            // 메서드 본문 분석
            if (method->body())
            {
                analyzeAndResolveStatement(method->body());
            }

            symbolTable_.exitScope();
        }

        // 생성자 분석 (있으면)
        if (classStmt->constructor())
        {
            auto ctor = classStmt->constructor();

            symbolTable_.enterScope();

            // 생성자 매개변수 등록
            for (const auto& param : ctor->parameters())
            {
                symbolTable_.define(param.name, SymbolKind::VARIABLE, nullptr);
            }

            // 생성자 본문 분석
            if (ctor->body())
            {
                analyzeAndResolveStatement(ctor->body());
            }

            symbolTable_.exitScope();
        }
    }
}

void SemanticAnalyzer::analyzeAndResolveExpression(const Expression* expr)
{
    if (!expr) return;

    // 식별자: 정의되어 있는지 확인
    if (auto ident = dynamic_cast<const Identifier*>(expr))
    {
        if (!symbolTable_.lookup(ident->name()) &&
            !isBuiltinFunction(ident->name()))
        {
            addError("정의되지 않은 식별자: " + ident->name(),
                    expr->location().line, expr->location().column);
        }
    }
    // 이항 표현식: 양쪽 피연산자 분석
    else if (auto binary = dynamic_cast<const BinaryExpression*>(expr))
    {
        analyzeAndResolveExpression(binary->left());
        analyzeAndResolveExpression(binary->right());
    }
    // 단항 표현식: 피연산자 분석
    else if (auto unary = dynamic_cast<const UnaryExpression*>(expr))
    {
        analyzeAndResolveExpression(unary->operand());
    }
    // 함수 호출: 함수명과 인자들 분석
    else if (auto call = dynamic_cast<const CallExpression*>(expr))
    {
        analyzeAndResolveExpression(call->function());

        for (const auto& arg : call->arguments())
        {
            analyzeAndResolveExpression(arg.get());
        }
    }
    // 조사 표현식: 객체 분석
    else if (auto josa = dynamic_cast<const JosaExpression*>(expr))
    {
        analyzeAndResolveExpression(josa->object());
    }
    // 인덱스 표현식: 배열과 인덱스 분석
    else if (auto index = dynamic_cast<const IndexExpression*>(expr))
    {
        analyzeAndResolveExpression(index->array());
        analyzeAndResolveExpression(index->index());
    }
    // 배열 리터럴: 각 요소 분석
    else if (auto arrayLit = dynamic_cast<const ArrayLiteral*>(expr))
    {
        for (const auto& elem : arrayLit->elements())
        {
            analyzeAndResolveExpression(elem.get());
        }
    }
    // Range 표현식: start, end 분석
    else if (auto range = dynamic_cast<const RangeExpression*>(expr))
    {
        analyzeAndResolveExpression(range->start());
        analyzeAndResolveExpression(range->end());
    }
    // Phase 7.1: 객체 생성 표현식
    else if (auto newExpr = dynamic_cast<const NewExpression*>(expr))
    {
        // 클래스 이름이 정의되어 있는지 확인
        if (!symbolTable_.lookup(newExpr->className()))
        {
            addError("정의되지 않은 클래스: " + newExpr->className(),
                    expr->location().line, expr->location().column);
        }

        // 생성자 인자들 분석
        for (const auto& arg : newExpr->arguments())
        {
            analyzeAndResolveExpression(arg.get());
        }
    }
    // Phase 7.1: 멤버 접근 표현식
    else if (auto memberAccess = dynamic_cast<const MemberAccessExpression*>(expr))
    {
        // 객체 표현식 분석
        analyzeAndResolveExpression(memberAccess->object());
        // 멤버 이름은 런타임에 검증 (클래스 정의를 알아야 함)
    }
    // Phase 7.1: this 표현식
    else if (dynamic_cast<const ThisExpression*>(expr))
    {
        // this는 메서드나 생성자 내부에서만 사용 가능
        // 현재는 스코프 체크 없이 허용 (TODO: 향후 메서드/생성자 스코프 추적 추가)
    }
    // 리터럴 (정수, 실수, 문자열, 불린): 아무것도 안 함
    // 함수 리터럴: 별도 처리 필요 없음 (AssignmentStatement에서 처리)
}

// ============================================================================
// Phase 1: Symbol Table 구축 (레거시)
// ============================================================================

void SemanticAnalyzer::buildSymbolTable(Program* program)
{
    for (auto& stmt : program->statements())
    {
        registerSymbolFromStatement(stmt.get());
    }
}

void SemanticAnalyzer::registerSymbolFromStatement(const Statement* stmt)
{
    if (!stmt) return;

    // 변수 선언
    if (auto varDecl = dynamic_cast<const VarDeclaration*>(stmt))
    {
        registerVariable(const_cast<VarDeclaration*>(varDecl));
    }
    // 할당문 (함수 선언도 할당문으로 변환됨)
    else if (auto assignStmt = dynamic_cast<const AssignmentStatement*>(stmt))
    {
        // 함수 리터럴 할당인지 확인
        if (auto funcLit = dynamic_cast<const FunctionLiteral*>(assignStmt->value()))
        {
            registerFunction(assignStmt->varName(), const_cast<FunctionLiteral*>(funcLit));
        }
    }
    // BlockStatement: 블록 내 모든 문장 등록 (Scope는 resolveNames에서 처리)
    else if (auto blockStmt = dynamic_cast<const BlockStatement*>(stmt))
    {
        for (const auto& s : blockStmt->statements())
        {
            registerSymbolFromStatement(s.get());
        }
    }
    // IfStatement: then과 else 블록 등록 (Scope는 resolveNames에서 처리)
    else if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt))
    {
        if (ifStmt->thenBranch())
        {
            registerSymbolFromStatement(ifStmt->thenBranch());
        }
        if (ifStmt->elseBranch())
        {
            registerSymbolFromStatement(ifStmt->elseBranch());
        }
    }
    // WhileStatement: body 등록 (Scope는 resolveNames에서 처리)
    else if (auto whileStmt = dynamic_cast<const WhileStatement*>(stmt))
    {
        if (whileStmt->body())
        {
            registerSymbolFromStatement(whileStmt->body());
        }
    }
    // RangeForStatement: body 등록 (루프 변수와 Scope는 resolveNames에서 처리)
    else if (auto forStmt = dynamic_cast<const RangeForStatement*>(stmt))
    {
        if (forStmt->body())
        {
            registerSymbolFromStatement(forStmt->body());
        }
    }
    // RepeatStatement: 반복 본문 등록 (Scope는 resolveNames에서 처리)
    else if (auto repeatStmt = dynamic_cast<const RepeatStatement*>(stmt))
    {
        if (repeatStmt->body())
        {
            registerSymbolFromStatement(repeatStmt->body());
        }
    }
}

void SemanticAnalyzer::registerVariable(VarDeclaration* varDecl)
{
    if (!varDecl) return;

    const std::string& varName = varDecl->varName();
    Type* varType = varDecl->varType();

    // 타입이 명시되지 않은 경우 초기화 값에서 타입 추론
    if (!varType && varDecl->initializer())
    {
        varType = inferType(varDecl->initializer());
    }

    // 이미 정의된 변수인지 확인 (현재 스코프에서만)
    if (symbolTable_.currentScope()->lookupLocal(varName))
    {
        addError("Variable '" + varName + "' is already defined in this scope");
        return;
    }

    // Symbol Table에 등록
    symbolTable_.define(varName, SymbolKind::VARIABLE, varType);
}

void SemanticAnalyzer::registerFunction(const std::string& name, FunctionLiteral* funcLit)
{
    if (!funcLit) return;

    // 이미 정의된 함수인지 확인
    if (symbolTable_.currentScope()->lookupLocal(name))
    {
        addError("Function '" + name + "' is already defined in this scope");
        return;
    }

    // 함수 타입 (향후 타입 시스템 확장 시 사용)
    // 현재는 반환 타입을 알 수 없으므로 nullptr
    symbolTable_.define(name, SymbolKind::FUNCTION, nullptr);

    // 함수 매개변수와 본문은 Phase 2 (resolveNames)에서 처리
    // 여기서는 함수 이름만 등록
}

// ============================================================================
// Phase 2: 이름 해석 (Name Resolution)
// ============================================================================

void SemanticAnalyzer::resolveNames(Program* program)
{
    // TODO: 모든 식별자가 정의된 심볼인지 확인
    // 현재는 skeleton만 구현
    for (auto& stmt : program->statements())
    {
        resolveNamesInStatement(stmt.get());
    }
}

void SemanticAnalyzer::resolveNamesInStatement(const Statement* stmt)
{
    if (!stmt) return;

    // VarDeclaration: 초기화 값 검증
    if (auto varDecl = dynamic_cast<const VarDeclaration*>(stmt))
    {
        if (varDecl->initializer())
        {
            resolveNamesInExpression(varDecl->initializer());
        }
    }

    // AssignmentStatement: 할당 값 검증 (변수명은 이미 정의되어 있어야 함)
    else if (auto assignStmt = dynamic_cast<const AssignmentStatement*>(stmt))
    {
        const std::string& varName = assignStmt->varName();

        // 할당 대상 변수가 정의되어 있는지 확인 (타입 검사 단계에서도 확인하지만 여기서도 체크)
        if (!symbolTable_.isDefined(varName))
        {
            addError("정의되지 않은 변수에 할당 시도: " + varName);
        }

        // 할당 값 검증
        resolveNamesInExpression(assignStmt->value());
    }

    // ExpressionStatement: 표현식 검증
    else if (auto exprStmt = dynamic_cast<const ExpressionStatement*>(stmt))
    {
        resolveNamesInExpression(exprStmt->expression());
    }

    // ReturnStatement: 반환 값 검증
    else if (auto retStmt = dynamic_cast<const ReturnStatement*>(stmt))
    {
        if (retStmt->returnValue())
        {
            resolveNamesInExpression(retStmt->returnValue());
        }
    }

    // IfStatement: 조건과 블록들 검증
    else if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt))
    {
        resolveNamesInExpression(ifStmt->condition());

        if (ifStmt->thenBranch())
        {
            symbolTable_.enterScope();  // then 블록 스코프 진입
            resolveNamesInStatement(ifStmt->thenBranch());
            symbolTable_.exitScope();   // then 블록 스코프 탈출
        }

        if (ifStmt->elseBranch())
        {
            symbolTable_.enterScope();  // else 블록 스코프 진입
            resolveNamesInStatement(ifStmt->elseBranch());
            symbolTable_.exitScope();   // else 블록 스코프 탈출
        }
    }

    // WhileStatement: 조건과 본문 검증
    else if (auto whileStmt = dynamic_cast<const WhileStatement*>(stmt))
    {
        resolveNamesInExpression(whileStmt->condition());

        if (whileStmt->body())
        {
            symbolTable_.enterScope();  // while 본문 스코프 진입
            resolveNamesInStatement(whileStmt->body());
            symbolTable_.exitScope();   // while 본문 스코프 탈출
        }
    }

    // BlockStatement: 모든 문장 검증
    else if (auto blockStmt = dynamic_cast<const BlockStatement*>(stmt))
    {
        symbolTable_.enterScope();  // 블록 스코프 진입

        for (const auto& s : blockStmt->statements())
        {
            resolveNamesInStatement(s.get());
        }

        symbolTable_.exitScope();  // 블록 스코프 탈출
    }

    // RangeForStatement: 범위와 본문 검증
    else if (auto forStmt = dynamic_cast<const RangeForStatement*>(stmt))
    {
        resolveNamesInExpression(forStmt->start());
        resolveNamesInExpression(forStmt->end());

        if (forStmt->body())
        {
            symbolTable_.enterScope();  // for 본문 스코프 진입

            // 루프 변수를 스코프에 등록 (타입은 범위의 타입과 동일하게 정수로 가정)
            symbolTable_.define(forStmt->varName(), SymbolKind::VARIABLE, types::Type::integerType());

            resolveNamesInStatement(forStmt->body());
            symbolTable_.exitScope();   // for 본문 스코프 탈출
        }
    }

    // RepeatStatement: 횟수와 본문 검증
    else if (auto repeatStmt = dynamic_cast<const RepeatStatement*>(stmt))
    {
        resolveNamesInExpression(repeatStmt->count());

        if (repeatStmt->body())
        {
            symbolTable_.enterScope();  // repeat 본문 스코프 진입
            resolveNamesInStatement(repeatStmt->body());
            symbolTable_.exitScope();   // repeat 본문 스코프 탈출
        }
    }
}

void SemanticAnalyzer::resolveNamesInExpression(const Expression* expr)
{
    if (!expr) return;

    // Identifier: 변수나 함수가 정의되었는지 확인
    if (auto ident = dynamic_cast<const Identifier*>(expr))
    {
        const std::string& name = ident->name();

        // builtin 함수나 정의된 심볼이 아니면 에러
        if (!isBuiltinFunction(name) && !symbolTable_.isDefined(name))
        {
            const auto& loc = ident->location();
            addError("정의되지 않은 식별자: " + name, loc.line, loc.column);
        }
    }

    // BinaryExpression: 좌변과 우변 검증
    else if (auto binExpr = dynamic_cast<const BinaryExpression*>(expr))
    {
        resolveNamesInExpression(binExpr->left());
        resolveNamesInExpression(binExpr->right());
    }

    // UnaryExpression: 피연산자 검증
    else if (auto unaryExpr = dynamic_cast<const UnaryExpression*>(expr))
    {
        resolveNamesInExpression(unaryExpr->operand());
    }

    // CallExpression: 함수와 인자들 검증
    else if (auto callExpr = dynamic_cast<const CallExpression*>(expr))
    {
        resolveNamesInExpression(callExpr->function());
        for (const auto& arg : callExpr->arguments())
        {
            resolveNamesInExpression(arg.get());
        }
    }

    // IndexExpression: 배열과 인덱스 검증
    else if (auto indexExpr = dynamic_cast<const IndexExpression*>(expr))
    {
        resolveNamesInExpression(indexExpr->array());
        resolveNamesInExpression(indexExpr->index());
    }

    // ArrayLiteral: 모든 요소 검증
    else if (auto arrayLit = dynamic_cast<const ArrayLiteral*>(expr))
    {
        for (const auto& elem : arrayLit->elements())
        {
            resolveNamesInExpression(elem.get());
        }
    }

    // FunctionLiteral: 함수 본문 검증 (매개변수는 함수 스코프에서 처리)
    else if (auto funcLit = dynamic_cast<const FunctionLiteral*>(expr))
    {
        symbolTable_.enterScope();  // 함수 스코프 진입

        // 매개변수들을 함수 스코프에 등록
        for (const auto& param : funcLit->parameters())
        {
            // 매개변수 타입은 현재 알 수 없으므로 nullptr
            symbolTable_.define(param, SymbolKind::VARIABLE, nullptr);
        }

        // 함수 본문 검증
        if (funcLit->body())
        {
            resolveNamesInStatement(funcLit->body());
        }

        symbolTable_.exitScope();  // 함수 스코프 탈출
    }

    // JosaExpression: 대상 표현식과 메서드 검증
    else if (auto josaExpr = dynamic_cast<const JosaExpression*>(expr))
    {
        resolveNamesInExpression(josaExpr->object());
        resolveNamesInExpression(josaExpr->method());
    }

    // MatchExpression: 패턴 매칭 표현식 검증
    else if (auto matchExpr = dynamic_cast<const MatchExpression*>(expr))
    {
        // 매칭 대상 표현식 검증
        resolveNamesInExpression(matchExpr->value());

        // 각 케이스의 결과 표현식 검증
        for (const auto& matchCase : matchExpr->cases())
        {
            // 가드 조건 검증
            if (matchCase.guard())
            {
                resolveNamesInExpression(matchCase.guard());
            }

            // 결과 표현식 검증
            if (matchCase.body())
            {
                resolveNamesInExpression(matchCase.body());
            }
        }
    }

    // 리터럴들은 검증 불필요 (IntegerLiteral, FloatLiteral, StringLiteral, BooleanLiteral)
}

// ============================================================================
// Phase 3: 타입 검사 (Type Checking)
// ============================================================================

void SemanticAnalyzer::checkTypes(Program* program)
{
    // TODO: 타입 검사 구현
    // 현재는 skeleton
    for (auto& stmt : program->statements())
    {
        checkTypesInStatement(stmt.get());
    }
}

void SemanticAnalyzer::checkTypesInStatement(const Statement* stmt)
{
    if (!stmt) return;

    // VarDeclaration: 변수 선언 시 초기화 값의 타입 검사
    if (auto varDecl = dynamic_cast<const VarDeclaration*>(stmt))
    {
        if (varDecl->initializer())
        {
            Type* initType = inferType(varDecl->initializer());
            Type* varType = varDecl->varType();

            if (initType && varType && !isTypeCompatible(varType, initType))
            {
                addError("타입 불일치: " + varType->koreanName() + " 변수에 " +
                        initType->koreanName() + " 값을 할당할 수 없습니다");
            }
        }
    }

    // AssignmentStatement: 할당문의 타입 검사
    else if (auto assignStmt = dynamic_cast<const AssignmentStatement*>(stmt))
    {
        const std::string& varName = assignStmt->varName();
        Symbol* symbol = symbolTable_.lookup(varName);

        if (!symbol)
        {
            addError("정의되지 않은 변수: " + varName);
            return;
        }

        Type* valueType = inferType(assignStmt->value());
        Type* varType = symbol->type;

        if (valueType && varType && !isTypeCompatible(varType, valueType))
        {
            addError("타입 불일치: " + varType->koreanName() + " 변수 '" + varName +
                    "'에 " + valueType->koreanName() + " 값을 할당할 수 없습니다");
        }
    }

    // ExpressionStatement: 표현식 문장의 타입 검사
    else if (auto exprStmt = dynamic_cast<const ExpressionStatement*>(stmt))
    {
        // 표현식 자체의 타입을 검사 (부작용 확인용)
        inferType(exprStmt->expression());
    }

    // ReturnStatement: 반환문의 타입 검사
    else if (auto retStmt = dynamic_cast<const ReturnStatement*>(stmt))
    {
        if (retStmt->returnValue())
        {
            // TODO: 현재 함수의 반환 타입과 비교
            // 함수 컨텍스트 추적 필요
            inferType(retStmt->returnValue());
        }
    }

    // IfStatement: if문의 조건 타입 검사
    else if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt))
    {
        Type* condType = inferType(ifStmt->condition());

        if (condType && condType->koreanName() != "논리")
        {
            addError("if문의 조건은 논리 타입이어야 합니다 (현재: " +
                    condType->koreanName() + ")");
        }

        // then과 else 블록의 타입 검사
        if (ifStmt->thenBranch())
        {
            checkTypesInStatement(ifStmt->thenBranch());
        }
        if (ifStmt->elseBranch())
        {
            checkTypesInStatement(ifStmt->elseBranch());
        }
    }

    // BlockStatement: 블록 내 모든 문장 검사
    else if (auto blockStmt = dynamic_cast<const BlockStatement*>(stmt))
    {
        for (const auto& s : blockStmt->statements())
        {
            checkTypesInStatement(s.get());
        }
    }

    // WhileStatement: while문의 조건 타입 검사
    else if (auto whileStmt = dynamic_cast<const WhileStatement*>(stmt))
    {
        Type* condType = inferType(whileStmt->condition());

        if (condType && condType->koreanName() != "논리")
        {
            addError("while문의 조건은 논리 타입이어야 합니다 (현재: " +
                    condType->koreanName() + ")");
        }

        if (whileStmt->body())
        {
            checkTypesInStatement(whileStmt->body());
        }
    }
}

Type* SemanticAnalyzer::inferType(const Expression* expr)
{
    if (!expr) return nullptr;

    // 리터럴 타입 추론
    if (dynamic_cast<const IntegerLiteral*>(expr))
    {
        return Type::getBuiltin("정수");
    }
    else if (dynamic_cast<const FloatLiteral*>(expr))
    {
        return Type::getBuiltin("실수");
    }
    else if (dynamic_cast<const StringLiteral*>(expr))
    {
        return Type::getBuiltin("문자열");
    }
    else if (dynamic_cast<const BooleanLiteral*>(expr))
    {
        return Type::getBuiltin("논리");
    }

    // 식별자 타입 추론
    else if (auto ident = dynamic_cast<const Identifier*>(expr))
    {
        Symbol* symbol = symbolTable_.lookup(ident->name());
        return symbol ? symbol->type : nullptr;
    }

    // 이항 연산 타입 추론
    else if (auto binExpr = dynamic_cast<const BinaryExpression*>(expr))
    {
        Type* leftType = inferType(binExpr->left());
        Type* rightType = inferType(binExpr->right());

        if (!leftType || !rightType) return nullptr;

        const std::string& op = binExpr->op();

        // 산술 연산자 (+, -, *, /, %)
        if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%")
        {
            // 정수 + 정수 = 정수
            if (leftType->koreanName() == "정수" && rightType->koreanName() == "정수")
            {
                return Type::getBuiltin("정수");
            }
            // 실수가 하나라도 있으면 실수
            else if (leftType->koreanName() == "실수" || rightType->koreanName() == "실수")
            {
                return Type::getBuiltin("실수");
            }
            // 문자열 + 문자열 = 문자열 (연결)
            else if (op == "+" && leftType->koreanName() == "문자열" && rightType->koreanName() == "문자열")
            {
                return Type::getBuiltin("문자열");
            }
        }

        // 비교 연산자 (<, >, <=, >=, ==, !=)
        else if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=")
        {
            return Type::getBuiltin("논리");
        }

        // 논리 연산자 (&&, ||)
        else if (op == "&&" || op == "||")
        {
            return Type::getBuiltin("논리");
        }
    }

    // 단항 연산 타입 추론
    else if (auto unaryExpr = dynamic_cast<const UnaryExpression*>(expr))
    {
        Type* operandType = inferType(unaryExpr->operand());

        if (!operandType) return nullptr;

        const std::string& op = unaryExpr->op();

        // 부정 연산자 (!)
        if (op == "!")
        {
            return Type::getBuiltin("논리");
        }
        // 부호 연산자 (-, +)
        else if (op == "-" || op == "+")
        {
            return operandType;  // 같은 타입 반환
        }
    }

    // 배열 리터럴 타입 추론
    else if (auto arrayLit = dynamic_cast<const ArrayLiteral*>(expr))
    {
        // 배열 요소들의 타입 일관성 검사
        if (!arrayLit->elements().empty())
        {
            Type* firstType = inferType(arrayLit->elements()[0].get());

            // 모든 요소가 같은 타입인지 확인
            for (size_t i = 1; i < arrayLit->elements().size(); i++)
            {
                Type* elemType = inferType(arrayLit->elements()[i].get());
                if (firstType && elemType &&
                    firstType->koreanName() != elemType->koreanName())
                {
                    addError("배열 요소의 타입이 일치하지 않습니다: " +
                            firstType->koreanName() + " vs " + elemType->koreanName());
                }
            }
        }

        return Type::getBuiltin("배열");
    }

    // 배열 인덱스 접근 타입 추론
    else if (auto indexExpr = dynamic_cast<const IndexExpression*>(expr))
    {
        Type* arrayType = inferType(indexExpr->array());
        Type* indexType = inferType(indexExpr->index());

        // 배열이 실제로 배열 타입인지 확인
        if (arrayType && arrayType->koreanName() != "배열" &&
            arrayType->koreanName() != "문자열")  // 문자열도 인덱스 접근 가능
        {
            addError("인덱스 접근은 배열 또는 문자열에만 사용할 수 있습니다 (현재: " +
                    arrayType->koreanName() + ")");
        }

        // 인덱스가 정수 타입인지 확인
        if (indexType && indexType->koreanName() != "정수")
        {
            addError("배열 인덱스는 정수여야 합니다 (현재: " +
                    indexType->koreanName() + ")");
        }

        // 문자열 인덱스 접근은 문자열 반환
        if (arrayType && arrayType->koreanName() == "문자열")
        {
            return Type::getBuiltin("문자열");
        }

        // 배열 요소 타입은 현재 추론 불가 (나중에 제네릭 타입으로 확장)
        // 일단 nullptr 반환
        return nullptr;
    }

    // 함수 호출 타입 추론
    else if (auto callExpr = dynamic_cast<const CallExpression*>(expr))
    {
        // 함수가 정의되어 있는지 확인
        if (auto funcIdent = dynamic_cast<const Identifier*>(callExpr->function()))
        {
            const std::string& funcName = funcIdent->name();

            // Phase 7.1: 클래스 인스턴스화인지 확인
            if (symbolTable_.isClass(funcName))
            {
                // 클래스 인스턴스화 - 타입 검사는 생략 (생성자 인자 검증은 나중에)
                return nullptr;  // 클래스 인스턴스 타입은 아직 정의되지 않음
            }

            // builtin 함수가 아니고 정의되지 않았으면 에러 (Name Resolution에서 이미 체크했지만)
            if (!isBuiltinFunction(funcName) && !symbolTable_.isDefined(funcName))
            {
                addError("정의되지 않은 함수 또는 클래스: " + funcName);
            }

            // builtin 함수들의 반환 타입 (알려진 것들만)
            if (funcName == "길이")
            {
                return Type::getBuiltin("정수");
            }
            else if (funcName == "정수")
            {
                return Type::getBuiltin("정수");
            }
            else if (funcName == "실수")
            {
                return Type::getBuiltin("실수");
            }
            else if (funcName == "문자열")
            {
                return Type::getBuiltin("문자열");
            }
            else if (funcName == "타입")
            {
                return Type::getBuiltin("문자열");
            }
            // 그 외 builtin 함수나 사용자 정의 함수는 반환 타입 알 수 없음
        }

        // 함수 반환 타입을 알 수 없는 경우
        return nullptr;
    }

    return nullptr;
}

bool SemanticAnalyzer::isTypeCompatible(Type* expected, Type* actual)
{
    if (!expected || !actual) return false;

    // 같은 타입이면 호환
    if (expected == actual) return true;

    // 타입 이름이 같으면 호환
    return expected->koreanName() == actual->koreanName();
}

// ============================================================================
// Phase 4: 모호성 해결
// ============================================================================

void SemanticAnalyzer::resolveAmbiguities(Program* program)
{
    // TODO: 모호성 해결 구현
    // 현재는 skeleton
    for (auto& stmt : program->statements())
    {
        resolveAmbiguitiesInStatement(stmt.get());
    }
}

void SemanticAnalyzer::resolveAmbiguitiesInStatement(Statement* /* stmt */)
{
    // TODO: 모호성 해결
    // 예: JosaExpression이 실제로 RangeForStatement여야 하는지 검증
    // 현재는 skeleton
}

// ============================================================================
// 유틸리티
// ============================================================================

void SemanticAnalyzer::addError(const std::string& message, int line, int column)
{
    errors_.emplace_back(message, line, column, filename_);
}

} // namespace semantic
} // namespace kingsejong
