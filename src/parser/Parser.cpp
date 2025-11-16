/**
 * @file Parser.cpp
 * @brief KingSejong 언어 파서 구현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Parser.h"
#include "types/Type.h"
#include <stdexcept>
#include <iostream>

namespace kingsejong {
namespace parser {

Parser::Parser(Lexer& lexer)
    : lexer_(lexer)
{
    // 파싱 함수 등록
    registerParseFunctions();

    // LL(4) lookahead: 첫 다섯 토큰 읽기
    nextToken();  // curToken 초기화
    nextToken();  // peekToken 초기화
    nextToken();  // peekPeekToken 초기화
    nextToken();  // peekPeekPeekToken 초기화
    nextToken();  // peekPeekPeekPeekToken 초기화
}

void Parser::registerParseFunctions()
{
    // Prefix 파싱 함수 등록
    registerPrefixFn(TokenType::IDENTIFIER, [this]() { return parseIdentifier(); });
    registerPrefixFn(TokenType::INTEGER, [this]() { return parseIntegerLiteral(); });
    registerPrefixFn(TokenType::FLOAT, [this]() { return parseFloatLiteral(); });
    registerPrefixFn(TokenType::STRING, [this]() { return parseStringLiteral(); });
    registerPrefixFn(TokenType::CHAM, [this]() { return parseBooleanLiteral(); });
    registerPrefixFn(TokenType::GEOJIT, [this]() { return parseBooleanLiteral(); });
    registerPrefixFn(TokenType::MINUS, [this]() { return parsePrefixExpression(); });
    registerPrefixFn(TokenType::NOT, [this]() { return parsePrefixExpression(); });
    registerPrefixFn(TokenType::LPAREN, [this]() { return parseGroupedExpression(); });
    registerPrefixFn(TokenType::LBRACKET, [this]() { return parseArrayLiteral(); });
    registerPrefixFn(TokenType::HAMSU, [this]() { return parseFunctionLiteral(); });

    // 타입 키워드도 식별자로 사용 가능 (builtin 함수 이름으로 사용)
    // 예: 정수(3.14), 실수(42), 문자열(123)
    registerPrefixFn(TokenType::JEONGSU, [this]() { return parseIdentifier(); });
    registerPrefixFn(TokenType::SILSU, [this]() { return parseIdentifier(); });
    registerPrefixFn(TokenType::MUNJAYEOL, [this]() { return parseIdentifier(); });
    registerPrefixFn(TokenType::BAEYEOL, [this]() { return parseIdentifier(); });

    // Infix 파싱 함수 등록
    registerInfixFn(TokenType::PLUS, [this](auto left) { return parseBinaryExpression(std::move(left)); });
    registerInfixFn(TokenType::MINUS, [this](auto left) { return parseBinaryExpression(std::move(left)); });
    registerInfixFn(TokenType::ASTERISK, [this](auto left) { return parseBinaryExpression(std::move(left)); });
    registerInfixFn(TokenType::SLASH, [this](auto left) { return parseBinaryExpression(std::move(left)); });
    registerInfixFn(TokenType::PERCENT, [this](auto left) { return parseBinaryExpression(std::move(left)); });
    registerInfixFn(TokenType::EQ, [this](auto left) { return parseBinaryExpression(std::move(left)); });
    registerInfixFn(TokenType::NOT_EQ, [this](auto left) { return parseBinaryExpression(std::move(left)); });
    registerInfixFn(TokenType::LT, [this](auto left) { return parseBinaryExpression(std::move(left)); });
    registerInfixFn(TokenType::GT, [this](auto left) { return parseBinaryExpression(std::move(left)); });
    registerInfixFn(TokenType::LE, [this](auto left) { return parseBinaryExpression(std::move(left)); });
    registerInfixFn(TokenType::GE, [this](auto left) { return parseBinaryExpression(std::move(left)); });
    registerInfixFn(TokenType::AND, [this](auto left) { return parseBinaryExpression(std::move(left)); });
    registerInfixFn(TokenType::OR, [this](auto left) { return parseBinaryExpression(std::move(left)); });
    registerInfixFn(TokenType::LPAREN, [this](auto left) { return parseCallExpression(std::move(left)); });
    registerInfixFn(TokenType::LBRACKET, [this](auto left) { return parseIndexExpression(std::move(left)); });

    // 조사 파싱 함수 등록
    registerInfixFn(TokenType::JOSA_EUL, [this](auto left) { return parseJosaExpression(std::move(left)); });
    registerInfixFn(TokenType::JOSA_REUL, [this](auto left) { return parseJosaExpression(std::move(left)); });
    registerInfixFn(TokenType::JOSA_I, [this](auto left) { return parseJosaExpression(std::move(left)); });
    registerInfixFn(TokenType::JOSA_GA, [this](auto left) { return parseJosaExpression(std::move(left)); });
    registerInfixFn(TokenType::JOSA_EUN, [this](auto left) { return parseJosaExpression(std::move(left)); });
    registerInfixFn(TokenType::JOSA_NEUN, [this](auto left) { return parseJosaExpression(std::move(left)); });
    registerInfixFn(TokenType::JOSA_UI, [this](auto left) { return parseJosaExpression(std::move(left)); });
    registerInfixFn(TokenType::JOSA_RO, [this](auto left) { return parseJosaExpression(std::move(left)); });
    registerInfixFn(TokenType::JOSA_EURO, [this](auto left) { return parseJosaExpression(std::move(left)); });
    registerInfixFn(TokenType::JOSA_ESO, [this](auto left) { return parseJosaExpression(std::move(left)); });
    registerInfixFn(TokenType::JOSA_E, [this](auto left) { return parseJosaExpression(std::move(left)); });

    // 패턴 매칭 파싱 함수 등록 (F5.5)
    registerInfixFn(TokenType::E_DAEHAE, [this](auto left) { return parseMatchExpression(std::move(left)); });

    // 범위 파싱 함수 등록
    registerInfixFn(TokenType::BUTEO, [this](auto left) { return parseRangeExpression(std::move(left)); });
    registerInfixFn(TokenType::CHOGA, [this](auto left) { return parseRangeExpression(std::move(left)); });
    registerInfixFn(TokenType::ISANG, [this](auto left) { return parseRangeExpression(std::move(left)); });
}

void Parser::registerPrefixFn(TokenType type, PrefixParseFn fn)
{
    prefixParseFns_[type] = fn;
}

void Parser::registerInfixFn(TokenType type, InfixParseFn fn)
{
    infixParseFns_[type] = fn;
}

// ============================================================================
// 토큰 관리
// ============================================================================

void Parser::nextToken()
{
    curToken_ = peekToken_;
    peekToken_ = peekPeekToken_;
    peekPeekToken_ = peekPeekPeekToken_;
    peekPeekPeekToken_ = peekPeekPeekPeekToken_;
    peekPeekPeekPeekToken_ = lexer_.nextToken();
}

bool Parser::curTokenIs(TokenType type) const
{
    return curToken_.type == type;
}

bool Parser::peekTokenIs(TokenType type) const
{
    return peekToken_.type == type;
}

bool Parser::peek2TokenIs(TokenType type) const
{
    return peekPeekToken_.type == type;
}

bool Parser::peek3TokenIs(TokenType type) const
{
    return peekPeekPeekToken_.type == type;
}

bool Parser::peek4TokenIs(TokenType type) const
{
    return peekPeekPeekPeekToken_.type == type;
}

bool Parser::expectPeek(TokenType type)
{
    if (peekTokenIs(type))
    {
        nextToken();
        return true;
    }

    peekError(type);
    return false;
}

// ============================================================================
// 에러 처리
// ============================================================================

void Parser::peekError(TokenType expected)
{
    std::string msg = "다음 토큰으로 '" + tokenTypeToString(expected) + "'을(를) 예상했지만, '" +
                      tokenTypeToString(peekToken_.type) + "'을(를) 받았습니다.";
    errors_.push_back(msg);
}

void Parser::noPrefixParseFnError(TokenType type)
{
    std::string msg = "'" + tokenTypeToString(type) + "'으(로) 시작하는 표현식을 파싱할 수 없습니다.";
    errors_.push_back(msg);
}

/**
 * @brief Panic Mode Recovery: 에러 후 안전한 지점까지 건너뛰기
 *
 * 파싱 에러가 발생했을 때, 다음 문장의 시작이나 세미콜론 같은
 * 동기화 지점(synchronization point)까지 토큰을 건너뛰어
 * 파싱을 계속할 수 있도록 합니다.
 *
 * 동기화 지점:
 * - 세미콜론 (;)
 * - 문장 시작 키워드 (정수, 실수, 만약, 함수, 반환 등)
 * - 블록 시작/끝 ({ })
 * - EOF
 */
void Parser::synchronize()
{
    while (!curTokenIs(TokenType::EOF_TOKEN))
    {
        // 세미콜론을 만나면 다음 토큰으로 이동하고 중단
        if (curTokenIs(TokenType::SEMICOLON))
        {
            nextToken();
            return;
        }

        // 다음 문장의 시작으로 보이는 키워드들
        // 이 키워드들을 만나면 현재 위치에서 중단 (토큰 소비 X)
        switch (peekToken_.type)
        {
            // 타입 키워드 (변수 선언 시작)
            case TokenType::JEONGSU:    // 정수
            case TokenType::SILSU:      // 실수
            case TokenType::MUNJAYEOL:  // 문자열
            case TokenType::NONLI:      // 논리

            // 제어문 키워드
            case TokenType::MANYAK:     // 만약 (if)
            case TokenType::DONGAN:     // 동안 (while)

            // 함수 키워드
            case TokenType::HAMSU:      // 함수
            case TokenType::BANHWAN:    // 반환 (return)

            // 모듈 키워드
            case TokenType::GAJYEOOGI:  // 가져오기 (import)

            // 블록 끝
            case TokenType::RBRACE:     // }
                nextToken();  // peekToken을 curToken으로 만들고 중단
                return;

            default:
                break;
        }

        nextToken();
    }
}

// ============================================================================
// 우선순위
// ============================================================================

Parser::Precedence Parser::tokenPrecedence(TokenType type) const
{
    switch (type)
    {
        case TokenType::ASSIGN:
            return Precedence::ASSIGN;
        case TokenType::OR:
            return Precedence::OR;
        case TokenType::AND:
            return Precedence::AND;
        case TokenType::EQ:
        case TokenType::NOT_EQ:
            return Precedence::EQUALS;
        case TokenType::LT:
        case TokenType::GT:
        case TokenType::LE:
        case TokenType::GE:
            return Precedence::LESSGREATER;
        case TokenType::PLUS:
        case TokenType::MINUS:
            return Precedence::SUM;
        case TokenType::ASTERISK:
        case TokenType::SLASH:
        case TokenType::PERCENT:
            return Precedence::PRODUCT;
        case TokenType::LPAREN:
            return Precedence::CALL;
        case TokenType::LBRACKET:
            return Precedence::INDEX;
        // 조사 토큰들 - CALL과 같은 우선순위
        case TokenType::JOSA_EUL:
        case TokenType::JOSA_REUL:
        case TokenType::JOSA_I:
        case TokenType::JOSA_GA:
        case TokenType::JOSA_EUN:
        case TokenType::JOSA_NEUN:
        case TokenType::JOSA_UI:
        case TokenType::JOSA_RO:
        case TokenType::JOSA_EURO:
        case TokenType::JOSA_ESO:
        case TokenType::JOSA_E:
        case TokenType::E_DAEHAE:
            return Precedence::CALL;
        // 범위 토큰들 - RANGE 우선순위 (ASSIGN보다 높고 OR보다 낮음)
        case TokenType::BUTEO:
        case TokenType::CHOGA:
        case TokenType::ISANG:
            return Precedence::RANGE;
        default:
            return Precedence::LOWEST;
    }
}

Parser::Precedence Parser::curPrecedence() const
{
    return tokenPrecedence(curToken_.type);
}

Parser::Precedence Parser::peekPrecedence() const
{
    return tokenPrecedence(peekToken_.type);
}

// ============================================================================
// 프로그램 파싱
// ============================================================================

std::unique_ptr<Program> Parser::parseProgram()
{
    auto program = std::make_unique<Program>();
    // 프로그램 시작 위치 (첫 토큰의 위치 정보 사용, 파일명 포함)
    program->setLocation(curToken_.location);

    while (!curTokenIs(TokenType::EOF_TOKEN))
    {
        auto stmt = parseStatement();
        if (stmt)
        {
            program->addStatement(std::move(stmt));
        }
        else
        {
            // Error Recovery: 파싱 실패 시 다음 문장 시작까지 건너뛰기
            // 이렇게 하면 여러 에러를 한 번에 수집할 수 있음
            synchronize();
        }
        nextToken();
    }

    return program;
}

// ============================================================================
// 문장 파싱
// ============================================================================

/**
 * @brief 범위 for문인지 확인하는 헬퍼 함수 (LL(4) lookahead 기반)
 *
 * 휴리스틱을 사용하지 않고 실제 토큰 패턴을 확인합니다.
 *
 * @return true if 범위 for문 패턴 감지
 *
 * @details
 * 토큰 패턴:
 *   IDENTIFIER / 가(이) / <simple-expr> / 부터(초과/이상) / ...
 *   cur         peek     peek2          peek3
 *
 * <simple-expr>는 INTEGER, FLOAT, IDENTIFIER 등 단순 표현식만 허용
 * 복잡한 표현식 (1+2)는 괄호로 시작하므로 peek2가 LPAREN
 */
bool Parser::isRangeForPattern() const
{
    // 패턴: IDENTIFIER + 가/이 + <expression> + <range-keyword>
    // LL(4) lookahead를 사용하여 범위 반복문 패턴을 감지합니다.
    //
    // 예시:
    //   i가 1부터 10까지      → 부터가 peek3에 위치 (1토큰 표현식)
    //   i가 -2부터 10까지     → 부터가 peek4에 위치 (2토큰 표현식: MINUS + INTEGER)
    //   i가 "문자열"부터 ... → 부터가 peek3에 위치 (STRING도 허용, 평가 시 에러)

    // Step 1: 현재 토큰이 IDENTIFIER인지 확인
    if (!curTokenIs(TokenType::IDENTIFIER))
        return false;

    // Step 2: 다음 토큰이 조사 "가" 또는 "이"인지 확인
    if (!peekTokenIs(TokenType::JOSA_GA) && !peekTokenIs(TokenType::JOSA_I))
        return false;

    // Step 3: peek3 또는 peek4에 범위 키워드가 있는지 확인
    // peek2의 타입을 제한하지 않음 (모든 표현식 허용)
    // - peek3에 범위 키워드: 1토큰 표현식 (숫자, 변수, 문자열 등)
    // - peek4에 범위 키워드: 2토큰 표현식 (전위 연산자 + 피연산자)
    return isRangeStartToken(peekPeekPeekToken_.type) ||
           isRangeStartToken(peekPeekPeekPeekToken_.type);
}

std::unique_ptr<Statement> Parser::parseStatement()
{
    // 명명 함수: 함수 이름(매개변수) { ... }
    // 이것을 할당문으로 변환: 이름 = 함수(매개변수) { ... }
    if (curTokenIs(TokenType::HAMSU) && peekTokenIs(TokenType::IDENTIFIER))
    {
        auto startLoc = curToken_.location;  // HAMSU 토큰 위치 저장
        nextToken();  // HAMSU를 건너뜀 → curToken = 함수 이름
        std::string functionName = curToken_.literal;

        // parseFunctionLiteral은 내부적으로 expectPeek(LPAREN)을 호출하므로
        // 현재 위치에서 호출하면 됨 (curToken = 함수 이름, peekToken = LPAREN)
        auto functionLiteral = parseFunctionLiteral();
        if (!functionLiteral)
        {
            return nullptr;
        }

        // 할당문으로 변환
        // NOTE: Symbol Table 등록은 SemanticAnalyzer가 담당
        auto stmt = std::make_unique<AssignmentStatement>(functionName, std::move(functionLiteral));
        stmt->setLocation(startLoc);
        return stmt;
    }

    // 타입 키워드로 시작하면 변수 선언
    // 단, 타입 키워드 뒤에 LPAREN이 오면 함수 호출이므로 표현식으로 처리
    // 또한 LBRACKET이 오면 인덱스 접근이므로 표현식으로 처리
    // Josa가 오면 조사 표현식이므로 표현식으로 처리
    // 예: 정수(3.14), 실수(42)는 타입 변환 함수 호출
    // 예: 배열[0]은 변수 "배열"의 인덱스 접근
    // 예: 배열을 정렬한다는 조사 표현식
    if ((curTokenIs(TokenType::JEONGSU) || curTokenIs(TokenType::SILSU) ||
         curTokenIs(TokenType::MUNJAYEOL) || curTokenIs(TokenType::NONLI) ||
         curTokenIs(TokenType::BAEYEOL)) &&
        !peekTokenIs(TokenType::LPAREN) && !peekTokenIs(TokenType::LBRACKET) &&
        !isJosaToken(peekToken_.type))
    {
        return parseVarDeclaration();
    }

    // 반환 문장
    if (curTokenIs(TokenType::BANHWAN))
    {
        return parseReturnStatement();
    }

    // 모듈 가져오기
    if (curTokenIs(TokenType::GAJYEOOGI))
    {
        return parseImportStatement();
    }

    // 조건 문장
    if (curTokenIs(TokenType::MANYAK))
    {
        return parseIfStatement();
    }

    // 할당 문장: identifier + "=" (범위 반복문보다 먼저 체크)
    if (curTokenIs(TokenType::IDENTIFIER) && peekTokenIs(TokenType::ASSIGN))
    {
        return parseAssignmentStatement();
    }

    // 범위 반복문: LL(4) lookahead로 패턴 확인 (휴리스틱 완전 제거!)
    // 패턴: IDENTIFIER + 가/이 + <simple-expr> + 부터/초과/이상
    if (isRangeForPattern())
    {
        return parseRangeForStatement();
    }

    // 블록 문장
    if (curTokenIs(TokenType::LBRACE))
    {
        return parseBlockStatement();
    }

    // 기본: 표현식 문장 또는 N번 반복문
    // 표현식을 먼저 파싱한 후 BEON 토큰을 확인
    auto startLoc = curToken_.location;  // 표현식 시작 위치 저장
    auto expr = parseExpression(Precedence::LOWEST);

    // 표현식 파싱 후 다음 토큰이 BEON이면 RepeatStatement
    if (peekTokenIs(TokenType::BEON))
    {
        nextToken(); // BEON으로 이동

        // "반복한다" 확인
        if (!peekTokenIs(TokenType::BANBOKHANDA))
        {
            peekError(TokenType::BANBOKHANDA);
            return nullptr;
        }
        nextToken(); // BANBOKHANDA로 이동

        // 본문 블록
        if (!expectPeek(TokenType::LBRACE))
        {
            return nullptr;
        }

        auto body = parseBlockStatement();
        auto stmt = std::make_unique<RepeatStatement>(std::move(expr), std::move(body));
        stmt->setLocation(startLoc);
        return stmt;
    }

    // 일반 표현식 문장
    if (peekTokenIs(TokenType::SEMICOLON))
    {
        nextToken();
    }

    auto stmt = std::make_unique<ExpressionStatement>(std::move(expr));
    stmt->setLocation(startLoc);
    return stmt;
}

std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement()
{
    auto startLoc = curToken_.location;  // 표현식 시작 위치 저장
    auto expr = parseExpression(Precedence::LOWEST);

    // 선택적 세미콜론 또는 ASI (Automatic Semicolon Insertion)
    if (peekTokenIs(TokenType::SEMICOLON))
    {
        nextToken();
    }
    // ASI: 줄이 바뀌면 자동으로 세미콜론 삽입
    else if (curToken_.location.line < peekToken_.location.line)
    {
        // 세미콜론 없어도 OK (줄이 바뀜)
    }

    auto stmt = std::make_unique<ExpressionStatement>(std::move(expr));
    stmt->setLocation(startLoc);
    return stmt;
}

std::unique_ptr<VarDeclaration> Parser::parseVarDeclaration()
{
    auto startLoc = curToken_.location;  // 타입 키워드 위치 저장
    std::string typeName = curToken_.literal;

    // 타입 이름으로 Type 객체 조회
    types::Type* varType = types::Type::getBuiltin(typeName);

    // 변수 이름 (IDENTIFIER 또는 타입 키워드도 변수명으로 허용)
    nextToken();
    if (!curTokenIs(TokenType::IDENTIFIER) &&
        !curTokenIs(TokenType::JEONGSU) &&
        !curTokenIs(TokenType::SILSU) &&
        !curTokenIs(TokenType::MUNJAYEOL) &&
        !curTokenIs(TokenType::NONLI) &&
        !curTokenIs(TokenType::BAEYEOL))
    {
        peekError(TokenType::IDENTIFIER);
        return nullptr;
    }

    std::string varName = curToken_.literal;

    // 초기화 (optional)
    std::unique_ptr<Expression> initializer = nullptr;

    if (peekTokenIs(TokenType::ASSIGN))
    {
        nextToken(); // =
        nextToken(); // 표현식 시작

        initializer = parseExpression(Precedence::LOWEST);

        // Error Recovery: ASSIGN 뒤에 표현식이 필수
        if (!initializer)
        {
            // 에러는 이미 parseExpression()에서 기록됨
            return nullptr;  // 파싱 실패
        }
    }

    // 선택적 세미콜론 또는 ASI (Automatic Semicolon Insertion)
    if (peekTokenIs(TokenType::SEMICOLON))
    {
        nextToken();
    }
    // ASI: 줄이 바뀌면 자동으로 세미콜론 삽입
    else if (curToken_.location.line < peekToken_.location.line)
    {
        // 세미콜론 없어도 OK (줄이 바뀜)
    }

    // NOTE: Symbol Table 등록은 SemanticAnalyzer가 담당
    auto stmt = std::make_unique<VarDeclaration>(typeName, varName, std::move(initializer), varType);
    stmt->setLocation(startLoc);
    return stmt;
}

std::unique_ptr<AssignmentStatement> Parser::parseAssignmentStatement()
{
    // 현재 토큰: 변수 이름 (IDENTIFIER)
    auto startLoc = curToken_.location;  // 변수 이름 위치 저장
    std::string varName = curToken_.literal;

    // "=" 확인
    if (!expectPeek(TokenType::ASSIGN))
    {
        return nullptr;
    }

    nextToken(); // 값 표현식 시작

    auto value = parseExpression(Precedence::LOWEST);

    // Error Recovery: ASSIGN 뒤에 표현식이 필수
    if (!value)
    {
        // 에러는 이미 parseExpression()에서 기록됨
        return nullptr;  // 파싱 실패
    }

    // 선택적 세미콜론 또는 ASI (Automatic Semicolon Insertion)
    if (peekTokenIs(TokenType::SEMICOLON))
    {
        nextToken();
    }
    // ASI: 줄이 바뀌면 자동으로 세미콜론 삽입
    else if (curToken_.location.line < peekToken_.location.line)
    {
        // 세미콜론 없어도 OK (줄이 바뀜)
    }

    auto stmt = std::make_unique<AssignmentStatement>(varName, std::move(value));
    stmt->setLocation(startLoc);
    return stmt;
}

std::unique_ptr<ReturnStatement> Parser::parseReturnStatement()
{
    auto startLoc = curToken_.location;  // "반환" 키워드 위치 저장
    nextToken(); // "반환" 건너뛰기

    std::unique_ptr<Expression> returnValue = nullptr;

    if (!curTokenIs(TokenType::SEMICOLON) && !curTokenIs(TokenType::EOF_TOKEN))
    {
        returnValue = parseExpression(Precedence::LOWEST);
    }

    // 선택적 세미콜론 또는 ASI (Automatic Semicolon Insertion)
    if (peekTokenIs(TokenType::SEMICOLON))
    {
        nextToken();
    }
    // ASI: 줄이 바뀌면 자동으로 세미콜론 삽입
    else if (curToken_.location.line < peekToken_.location.line)
    {
        // 세미콜론 없어도 OK (줄이 바뀜)
    }

    auto stmt = std::make_unique<ReturnStatement>(std::move(returnValue));
    stmt->setLocation(startLoc);
    return stmt;
}

std::unique_ptr<IfStatement> Parser::parseIfStatement()
{
    auto startLoc = curToken_.location;  // "만약" 키워드 위치 저장

    // 조건식 파싱: 괄호는 선택사항
    // "만약 (조건) {" 또는 "만약 조건 {" 모두 지원
    bool hasParentheses = peekTokenIs(TokenType::LPAREN);

    if (hasParentheses)
    {
        nextToken(); // '(' 로 이동
    }

    nextToken(); // 조건식 시작

    auto condition = parseExpression(Precedence::LOWEST);

    if (hasParentheses)
    {
        if (!expectPeek(TokenType::RPAREN))
        {
            return nullptr;
        }
    }

    // then 블록 파싱
    if (!expectPeek(TokenType::LBRACE))
    {
        return nullptr;
    }

    auto thenBranch = parseBlockStatement();

    // else 블록 (optional)
    std::unique_ptr<BlockStatement> elseBranch = nullptr;

    if (peekTokenIs(TokenType::ANIMYEON))
    {
        nextToken(); // "아니면" 으로 이동

        if (!expectPeek(TokenType::LBRACE))
        {
            return nullptr;
        }

        elseBranch = parseBlockStatement();
    }

    auto stmt = std::make_unique<IfStatement>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch)
    );
    stmt->setLocation(startLoc);
    return stmt;
}

std::unique_ptr<RepeatStatement> Parser::parseRepeatStatement()
{
    auto startLoc = curToken_.location;  // 반복 횟수 표현식 시작 위치 저장

    // 반복 횟수 파싱
    auto count = parseExpression(Precedence::LOWEST);

    // "번" 확인
    if (!expectPeek(TokenType::BEON))
    {
        return nullptr;
    }

    // "반복한다" 또는 "반복" 확인 (선택적)
    if (peekTokenIs(TokenType::BANBOKHANDA))
    {
        nextToken(); // "반복한다"로 이동
    }
    else if (peekTokenIs(TokenType::BANBOKK))
    {
        nextToken(); // "반복"으로 이동

        // "하라" 확인 (선택적)
        if (peekTokenIs(TokenType::HARA))
        {
            nextToken(); // "하라"로 이동
        }
    }

    // 본문 블록
    if (!expectPeek(TokenType::LBRACE))
    {
        return nullptr;
    }

    auto body = parseBlockStatement();

    auto stmt = std::make_unique<RepeatStatement>(
        std::move(count),
        std::move(body)
    );
    stmt->setLocation(startLoc);
    return stmt;
}

std::unique_ptr<RangeForStatement> Parser::parseRangeForStatement()
{
    // 범위 for문: "i가 1부터 5까지 반복한다 { ... }"
    // ParseFeature::Range를 비활성화하여 start/end 표현식에서 RangeExpression이 생성되지 않도록 함

    auto startLoc = curToken_.location;  // 루프 변수 위치 저장

    // 변수 이름 저장
    std::string varName = curToken_.literal;

    // "가" 또는 "이" 확인
    if (!expectPeek(TokenType::JOSA_GA) && !expectPeek(TokenType::JOSA_I))
    {
        return nullptr;
    }

    // 시작 값 파싱 (Range 기능 비활성화)
    nextToken(); // 시작 표현식으로 이동
    auto start = parseExpression(Precedence::LOWEST, ParseFeature::All & ~ParseFeature::Range);

    // "부터" 명시적으로 확인
    if (!expectPeek(TokenType::BUTEO))
    {
        return nullptr;
    }

    // 끝 값 파싱 (Range 기능 비활성화)
    nextToken(); // 끝 표현식으로 이동
    auto end = parseExpression(Precedence::LOWEST, ParseFeature::All & ~ParseFeature::Range);

    // 범위 종료 키워드 확인 (까지/미만/이하/이상)
    if (!peekTokenIs(TokenType::KKAJI) &&
        !peekTokenIs(TokenType::MIMAN) &&
        !peekTokenIs(TokenType::IHA) &&
        !peekTokenIs(TokenType::ISANG))
    {
        std::string msg = "범위 종료 키워드 (까지/미만/이하/이상)를 예상했지만, '" +
                         tokenTypeToString(peekToken_.type) + "'을(를) 받았습니다.";
        errors_.push_back(msg);
        return nullptr;
    }

    nextToken(); // 범위 종료 키워드로 이동
    TokenType endToken = curToken_.type;

    // 끝 키워드에 따라 inclusive 결정
    bool endInclusive = true;
    if (endToken == TokenType::MIMAN)  // 미만
    {
        endInclusive = false;
    }
    else if (endToken == TokenType::IHA)  // 이하
    {
        endInclusive = true;
    }
    else if (endToken == TokenType::KKAJI)  // 까지
    {
        endInclusive = true;
    }
    else if (endToken == TokenType::ISANG)  // 이상
    {
        endInclusive = true;
    }

    // "반복한다" 또는 "반복" 확인 (선택적)
    if (peekTokenIs(TokenType::BANBOKHANDA))
    {
        nextToken(); // "반복한다"로 이동
    }
    else if (peekTokenIs(TokenType::BANBOKK))
    {
        nextToken(); // "반복"으로 이동

        // "하라" 확인 (선택적)
        if (peekTokenIs(TokenType::HARA))
        {
            nextToken(); // "하라"로 이동
        }
    }

    // 본문 블록
    if (!expectPeek(TokenType::LBRACE))
    {
        return nullptr;
    }

    auto body = parseBlockStatement();

    auto stmt = std::make_unique<RangeForStatement>(
        varName,
        std::move(start),
        std::move(end),
        std::move(body),
        endInclusive
    );
    stmt->setLocation(startLoc);
    return stmt;
}

std::unique_ptr<BlockStatement> Parser::parseBlockStatement()
{
    auto startLoc = curToken_.location;  // { 토큰 위치 저장
    nextToken(); // { 건너뛰기

    auto statements = parseStatements(TokenType::RBRACE);

    auto stmt = std::make_unique<BlockStatement>(std::move(statements));
    stmt->setLocation(startLoc);
    return stmt;
}

std::unique_ptr<ImportStatement> Parser::parseImportStatement()
{
    // 현재 토큰은 "가져오기"
    auto startLoc = curToken_.location;  // "가져오기" 키워드 위치 저장

    // 다음 토큰은 문자열이어야 함
    if (!expectPeek(TokenType::STRING))
    {
        return nullptr;
    }

    std::string modulePath = curToken_.literal;

    auto stmt = std::make_unique<ImportStatement>(modulePath);
    stmt->setLocation(startLoc);
    return stmt;
}

std::vector<std::unique_ptr<Statement>> Parser::parseStatements(TokenType endToken)
{
    std::vector<std::unique_ptr<Statement>> statements;

    while (!curTokenIs(endToken) && !curTokenIs(TokenType::EOF_TOKEN))
    {
        auto stmt = parseStatement();
        if (stmt)
        {
            statements.push_back(std::move(stmt));
        }

        // parseStatement()가 이미 세미콜론까지 소비했는지 확인
        // 세미콜론이 있었다면 curToken은 세미콜론, 없었다면 표현식 마지막
        // BUG FIX: 원래 조건은 peekToken이 endToken일 때 nextToken()을 호출하지 않아 무한 루프 발생
        // 수정: 항상 nextToken() 호출하여 curToken을 진행시킴
        nextToken();
    }

    return statements;
}

// ============================================================================
// 표현식 파싱 (Pratt Parsing 핵심)
// ============================================================================

std::unique_ptr<Expression> Parser::parseExpression(Precedence precedence, ParseFeature features)
{
    // Prefix 파싱 함수 찾기
    auto prefixIt = prefixParseFns_.find(curToken_.type);
    if (prefixIt == prefixParseFns_.end())
    {
        noPrefixParseFnError(curToken_.type);
        return nullptr;
    }

    auto leftExp = prefixIt->second();

    // Infix 파싱 (우선순위 기반)
    while (!peekTokenIs(TokenType::SEMICOLON) && precedence < peekPrecedence())
    {
        // ASI: 줄이 바뀌면 infix 파싱 중단 (표현식 끝으로 간주)
        // 예: "정수 c = 2" 다음에 "(a + b)"가 다음 줄에 있으면 함수 호출이 아님
        if (curToken_.location.line < peekToken_.location.line)
        {
            break;
        }

        // Range 기능이 비활성화되어 있으면 Range 연산자 건너뛰기
        if (!hasFeature(features, ParseFeature::Range))
        {
            if (peekTokenIs(TokenType::BUTEO) ||
                peekTokenIs(TokenType::CHOGA) ||
                peekTokenIs(TokenType::ISANG))
            {
                // Range 연산자를 infix로 처리하지 않고 반환
                return leftExp;
            }
        }

        auto infixIt = infixParseFns_.find(peekToken_.type);
        if (infixIt == infixParseFns_.end())
        {
            return leftExp;
        }

        nextToken();
        leftExp = infixIt->second(std::move(leftExp));
    }

    return leftExp;
}

// ============================================================================
// Prefix 파싱 함수들
// ============================================================================

std::unique_ptr<Expression> Parser::parseIdentifier()
{
    auto node = std::make_unique<Identifier>(curToken_.literal);
    node->setLocation(curToken_.location);
    return node;
}

std::unique_ptr<Expression> Parser::parseIntegerLiteral()
{
    try
    {
        int64_t value = std::stoll(curToken_.literal);
        auto node = std::make_unique<IntegerLiteral>(value);
        node->setLocation(curToken_.location);
        return node;
    }
    catch (const std::exception&)
    {
        errors_.push_back("'" + curToken_.literal + "'을(를) 정수로 변환할 수 없습니다.");
        return nullptr;
    }
}

std::unique_ptr<Expression> Parser::parseFloatLiteral()
{
    try
    {
        double value = std::stod(curToken_.literal);
        auto node = std::make_unique<FloatLiteral>(value);
        node->setLocation(curToken_.location);
        return node;
    }
    catch (const std::exception&)
    {
        errors_.push_back("'" + curToken_.literal + "'을(를) 실수로 변환할 수 없습니다.");
        return nullptr;
    }
}

std::unique_ptr<Expression> Parser::parseStringLiteral()
{
    auto node = std::make_unique<StringLiteral>(curToken_.literal);
    node->setLocation(curToken_.location);
    return node;
}

std::unique_ptr<Expression> Parser::parseBooleanLiteral()
{
    bool value = curTokenIs(TokenType::CHAM);
    auto node = std::make_unique<BooleanLiteral>(value);
    node->setLocation(curToken_.location);
    return node;
}

std::unique_ptr<Expression> Parser::parsePrefixExpression()
{
    auto startLoc = curToken_.location;  // 연산자 위치 저장
    std::string op = curToken_.literal;
    nextToken();

    auto right = parseExpression(Precedence::PREFIX);
    auto expr = std::make_unique<UnaryExpression>(op, std::move(right));
    expr->setLocation(startLoc);
    return expr;
}

std::unique_ptr<Expression> Parser::parseGroupedExpression()
{
    nextToken(); // ( 건너뛰기

    auto expr = parseExpression(Precedence::LOWEST);

    if (!expectPeek(TokenType::RPAREN))
    {
        return nullptr;
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseArrayLiteral()
{
    auto startLoc = curToken_.location;  // [ 토큰 위치 저장
    auto elements = parseExpressionList(TokenType::RBRACKET);
    auto expr = std::make_unique<ArrayLiteral>(std::move(elements));
    expr->setLocation(startLoc);
    return expr;
}

// ============================================================================
// Infix 파싱 함수들
// ============================================================================

std::unique_ptr<Expression> Parser::parseBinaryExpression(std::unique_ptr<Expression> left)
{
    auto startLoc = left->location();  // 좌측 피연산자 위치 사용
    std::string op = curToken_.literal;
    Precedence precedence = curPrecedence();

    nextToken();
    auto right = parseExpression(precedence);

    auto expr = std::make_unique<BinaryExpression>(std::move(left), op, std::move(right));
    expr->setLocation(startLoc);
    return expr;
}

std::unique_ptr<Expression> Parser::parseCallExpression(std::unique_ptr<Expression> function)
{
    auto startLoc = function->location();  // 함수 표현식 위치 사용
    auto arguments = parseExpressionList(TokenType::RPAREN);
    auto expr = std::make_unique<CallExpression>(std::move(function), std::move(arguments));
    expr->setLocation(startLoc);
    return expr;
}

std::unique_ptr<Expression> Parser::parseIndexExpression(std::unique_ptr<Expression> left)
{
    auto startLoc = left->location();  // 좌측 표현식 위치 사용
    nextToken(); // [ 건너뛰기

    auto index = parseExpression(Precedence::LOWEST);

    if (!expectPeek(TokenType::RBRACKET))
    {
        return nullptr;
    }

    auto expr = std::make_unique<IndexExpression>(std::move(left), std::move(index));
    expr->setLocation(startLoc);
    return expr;
}

std::unique_ptr<Expression> Parser::parseJosaExpression(std::unique_ptr<Expression> left)
{
    auto startLoc = left->location();  // 좌측 표현식 위치 사용
    // 현재 토큰은 조사 (EUL, REUL, I, GA 등)
    lexer::JosaRecognizer::JosaType josaType = tokenToJosaType(curToken_.type);

    nextToken(); // 조사 다음 토큰으로 이동 (메서드/명사)

    // 메서드/명사 파싱
    auto method = parseExpression(Precedence::LOWEST);

    auto expr = std::make_unique<JosaExpression>(std::move(left), josaType, std::move(method));
    expr->setLocation(startLoc);
    return expr;
}

std::unique_ptr<Expression> Parser::parseRangeExpression(std::unique_ptr<Expression> left)
{
    auto startLoc = left->location();  // 좌측 표현식 (시작값) 위치 사용
    // 복합 연산자: 부터 ... 까지를 한 번에 파싱
    // left는 시작 값, curToken은 범위 시작 키워드 (BUTEO, CHOGA, ISANG)

    TokenType startToken = curToken_.type;
    bool startInclusive = true;

    // 시작 키워드에 따라 inclusive 결정
    if (startToken == TokenType::CHOGA)  // 초과
    {
        startInclusive = false;
    }
    else if (startToken == TokenType::ISANG)  // 이상
    {
        startInclusive = true;
    }
    else if (startToken == TokenType::BUTEO)  // 부터
    {
        startInclusive = true;
    }

    // 끝 값 파싱 (Range 연산자를 infix로 처리하지 않음)
    nextToken(); // 끝 값 시작
    auto end = parseExpression(Precedence::SUM, ParseFeature::All & ~ParseFeature::Range);

    // 끝 키워드 확인 (KKAJI, MIMAN, IHA, ISANG 중 하나여야 함)
    if (!peekTokenIs(TokenType::KKAJI) &&
        !peekTokenIs(TokenType::MIMAN) &&
        !peekTokenIs(TokenType::IHA) &&
        !peekTokenIs(TokenType::ISANG))
    {
        std::string msg = "범위 종료 키워드 (까지/미만/이하/이상)를 예상했지만, '" +
                         tokenTypeToString(peekToken_.type) + "'을(를) 받았습니다.";
        errors_.push_back(msg);
        return nullptr;
    }

    // 끝 키워드로 이동
    nextToken();

    // 끝 키워드에 따라 inclusive 결정
    bool endInclusive = true;
    TokenType endToken = curToken_.type;

    if (endToken == TokenType::MIMAN)  // 미만
    {
        endInclusive = false;
    }
    else if (endToken == TokenType::IHA)  // 이하
    {
        endInclusive = true;
    }
    else if (endToken == TokenType::KKAJI)  // 까지
    {
        endInclusive = true;
    }
    else if (endToken == TokenType::ISANG)  // 이상
    {
        endInclusive = true;
    }

    auto expr = std::make_unique<RangeExpression>(std::move(left), std::move(end),
                                                   startInclusive, endInclusive);
    expr->setLocation(startLoc);
    return expr;
}

// ============================================================================
// 헬퍼 함수들
// ============================================================================

std::vector<std::unique_ptr<Expression>> Parser::parseExpressionList(TokenType endToken)
{
    std::vector<std::unique_ptr<Expression>> list;

    if (peekTokenIs(endToken))
    {
        nextToken(); // endToken 건너뛰기
        return list;
    }

    nextToken(); // 첫 표현식 시작
    list.push_back(parseExpression(Precedence::LOWEST));

    while (peekTokenIs(TokenType::COMMA))
    {
        nextToken(); // ,
        nextToken(); // 다음 표현식

        list.push_back(parseExpression(Precedence::LOWEST));
    }

    if (!expectPeek(endToken))
    {
        return {};
    }

    return list;
}

// ============================================================================
// 조사 파싱 헬퍼 함수들
// ============================================================================

bool Parser::isJosaToken(TokenType type) const
{
    switch (type)
    {
        case TokenType::JOSA_EUL:
        case TokenType::JOSA_REUL:
        case TokenType::JOSA_I:
        case TokenType::JOSA_GA:
        case TokenType::JOSA_EUN:
        case TokenType::JOSA_NEUN:
        case TokenType::JOSA_UI:
        case TokenType::JOSA_RO:
        case TokenType::JOSA_EURO:
        case TokenType::JOSA_ESO:
        case TokenType::JOSA_E:
            return true;
        default:
            return false;
    }
}

lexer::JosaRecognizer::JosaType Parser::tokenToJosaType(TokenType type) const
{
    using JosaType = lexer::JosaRecognizer::JosaType;

    switch (type)
    {
        case TokenType::JOSA_EUL:
        case TokenType::JOSA_REUL:
            return JosaType::EUL_REUL;

        case TokenType::JOSA_I:
        case TokenType::JOSA_GA:
            return JosaType::I_GA;

        case TokenType::JOSA_EUN:
        case TokenType::JOSA_NEUN:
            return JosaType::EUN_NEUN;

        case TokenType::JOSA_UI:
            return JosaType::UI;

        case TokenType::JOSA_RO:
        case TokenType::JOSA_EURO:
            return JosaType::RO_EURO;

        case TokenType::JOSA_ESO:
            return JosaType::ESO;

        case TokenType::JOSA_E:
            return JosaType::E;

        default:
            // 이 경우는 발생하지 않아야 함
            return JosaType::EUL_REUL;
    }
}

// ============================================================================
// 범위 파싱 헬퍼 함수들
// ============================================================================

bool Parser::isRangeStartToken(TokenType type) const
{
    switch (type)
    {
        case TokenType::BUTEO:   // 부터
        case TokenType::CHOGA:   // 초과
        case TokenType::ISANG:   // 이상
            return true;
        default:
            return false;
    }
}

bool Parser::isRangeEndToken(TokenType type) const
{
    switch (type)
    {
        case TokenType::KKAJI:   // 까지
        case TokenType::MIMAN:   // 미만
        case TokenType::IHA:     // 이하
        case TokenType::ISANG:   // 이상
            return true;
        default:
            return false;
    }
}

std::unique_ptr<Expression> Parser::parseFunctionLiteral()
{
    // 현재 토큰은 함수 이름 (IDENTIFIER) 또는 "함수" 키워드
    auto startLoc = curToken_.location;  // 함수 정의 시작 위치 저장

    // 다음 토큰이 LPAREN "(" 인지 확인
    if (!expectPeek(TokenType::LPAREN))
    {
        return nullptr;
    }

    // 매개변수 리스트 파싱
    std::vector<std::string> parameters;

    // 다음 토큰으로 이동
    nextToken();

    // 빈 매개변수 리스트 체크
    if (curTokenIs(TokenType::RPAREN))
    {
        // 매개변수 없음, 다음으로 진행
        nextToken();  // RPAREN을 넘어감
    }
    else
    {
        // 첫 번째 매개변수
        if (!curTokenIs(TokenType::IDENTIFIER))
        {
            peekError(TokenType::IDENTIFIER);
            return nullptr;
        }
        parameters.push_back(curToken_.literal);

        // 나머지 매개변수들 (COMMA로 구분)
        while (peekTokenIs(TokenType::COMMA))
        {
            nextToken();  // 현재를 COMMA로
            nextToken();  // 다음 매개변수로

            if (!curTokenIs(TokenType::IDENTIFIER))
            {
                peekError(TokenType::IDENTIFIER);
                return nullptr;
            }
            parameters.push_back(curToken_.literal);
        }

        // RPAREN ")" 확인
        if (!expectPeek(TokenType::RPAREN))
        {
            return nullptr;
        }

        nextToken();  // RPAREN을 넘어감
    }

    // 함수 본문 파싱 (BlockStatement)
    auto body = parseBlockStatement();

    if (!body)
    {
        return nullptr;
    }

    auto expr = std::make_unique<FunctionLiteral>(std::move(parameters), std::move(body));
    expr->setLocation(startLoc);
    return expr;
}

// ============================================================================
// 패턴 매칭 파싱 (F5.5)
// ============================================================================

std::unique_ptr<Expression> Parser::parseMatchExpression(std::unique_ptr<Expression> left)
{
    auto startLoc = left->location();  // 좌측 표현식 위치 사용
    nextToken(); // '{' 로 이동

    if (!curTokenIs(TokenType::LBRACE))
    {
        errors_.push_back("패턴 매칭에는 '{'가 필요합니다");
        return nullptr;
    }

    nextToken(); // 첫 번째 패턴으로 이동

    std::vector<MatchCase> cases;

    while (curToken_.type != TokenType::RBRACE && curToken_.type != TokenType::EOF_TOKEN)
    {
        MatchCase matchCase = parseMatchCase();
        cases.push_back(std::move(matchCase));

        if (peekTokenIs(TokenType::RBRACE))
        {
            nextToken();
        }
        else
        {
            nextToken();
        }
    }

    if (!curTokenIs(TokenType::RBRACE))
    {
        errors_.push_back("패턴 매칭에는 '}'가 필요합니다");
        return nullptr;
    }

    // Don't advance past '}' - leave curToken at '}'
    auto expr = std::make_unique<MatchExpression>(std::move(left), std::move(cases));
    expr->setLocation(startLoc);
    return expr;
}

MatchCase Parser::parseMatchCase()
{
    auto pattern = parsePattern();
    if (!pattern)
    {
        errors_.push_back("패턴 파싱 실패");
        return MatchCase(nullptr, nullptr, nullptr);
    }

    std::unique_ptr<Expression> guard = nullptr;
    if (peekTokenIs(TokenType::WHEN))
    {
        nextToken();
        nextToken();
        guard = parseExpression(Precedence::LOWEST);
    }

    if (!expectPeek(TokenType::ARROW))
    {
        errors_.push_back("패턴 매칭 케이스에는 '->'가 필요합니다");
        return MatchCase(std::move(pattern), nullptr, nullptr);
    }

    nextToken(); // 본문 표현식으로 이동

    auto body = parseExpression(Precedence::LOWEST);

    if (!body)
    {
        errors_.push_back("패턴 매칭 본문 파싱 실패");
        return MatchCase(std::move(pattern), nullptr, nullptr);
    }

    // **중요**: MatchCase 생성자는 (pattern, body, guard) 순서!
    return MatchCase(std::move(pattern), std::move(body), std::move(guard));
}

std::unique_ptr<Pattern> Parser::parsePattern()
{
    auto startLoc = curToken_.location;  // 패턴 시작 위치 저장

    if (curTokenIs(TokenType::UNDERSCORE))
    {
        auto pattern = std::make_unique<WildcardPattern>();
        pattern->setLocation(startLoc);
        return pattern;
    }

    if (curTokenIs(TokenType::LBRACKET))
    {
        return parseArrayPattern();
    }

    if (curTokenIs(TokenType::INTEGER) || curTokenIs(TokenType::FLOAT) ||
        curTokenIs(TokenType::STRING) || curTokenIs(TokenType::CHAM) ||
        curTokenIs(TokenType::GEOJIT))
    {
        auto value = parseExpression(Precedence::LOWEST);
        auto pattern = std::make_unique<LiteralPattern>(std::move(value));
        pattern->setLocation(startLoc);
        return pattern;
    }

    if (curTokenIs(TokenType::IDENTIFIER))
    {
        std::string name = curToken_.literal;
        auto pattern = std::make_unique<BindingPattern>(name);
        pattern->setLocation(startLoc);
        return pattern;
    }

    errors_.push_back("알 수 없는 패턴: " + tokenTypeToString(curToken_.type));
    return nullptr;
}

std::unique_ptr<Pattern> Parser::parseArrayPattern()
{
    auto startLoc = curToken_.location;  // [ 토큰 위치 저장
    std::vector<std::unique_ptr<Pattern>> elements;
    std::string rest;

    nextToken();

    while (!curTokenIs(TokenType::RBRACKET) && !curTokenIs(TokenType::EOF_TOKEN))
    {
        auto pattern = parsePattern();
        if (pattern)
        {
            elements.push_back(std::move(pattern));
        }

        if (peekTokenIs(TokenType::COMMA))
        {
            nextToken();
            nextToken();
        }
        else if (peekTokenIs(TokenType::RBRACKET))
        {
            nextToken();
        }
    }

    auto pattern = std::make_unique<ArrayPattern>(std::move(elements), rest);
    pattern->setLocation(startLoc);
    return pattern;
}

} // namespace parser
} // namespace kingsejong
