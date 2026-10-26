/**
 * @file Parser.cpp
 * @brief KingSejong 언어 파서 구현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Parser.h"
#include <cctype>
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
    registerPrefixFn(TokenType::LBRACE, [this]() { return parseDictionaryLiteral(); });  // Phase 7.2
    registerPrefixFn(TokenType::HAMSU, [this]() { return parseFunctionLiteral(); });
    registerPrefixFn(TokenType::BIDONGGI, [this]() { return parseAsyncFunctionLiteral(); });  // Phase 7.3
    // '상위' 는 메서드·생성자 안에서 부모 클래스를 가리키는 이름이다 (실행기가 환경에 묶는다)
    registerPrefixFn(TokenType::SANGWI, [this]() { return parseIdentifier(); });
    registerPrefixFn(TokenType::DAEGI, [this]() { return parseAwaitExpression(); });  // Phase 7.3

    // 타입 키워드도 식별자로 사용 가능 (builtin 함수 이름으로 사용)
    // 예: 정수(3.14), 실수(42), 문자열(123)
    registerPrefixFn(TokenType::JEONGSU, [this]() { return parseIdentifier(); });
    registerPrefixFn(TokenType::SILSU, [this]() { return parseIdentifier(); });
    registerPrefixFn(TokenType::MUNJAYEOL, [this]() { return parseIdentifier(); });
    registerPrefixFn(TokenType::BAEYEOL, [this]() { return parseIdentifier(); });

    // 문장 키워드와 이름이 겹치는 표준 라이브러리 함수: 식 안에서 "마지막(배열, 3)", "반복("안녕", 3)"
    // 처럼 바로 괄호가 따라오면 함수 이름(식별자)으로 읽는다. 문장 첫머리의 "반복 (조건) {" 는 while 이다.
    auto keywordAsCallee = [this]() -> std::unique_ptr<Expression> {
        // "마지막(...)" 호출, "반복 = 함수(...)" 정의 둘 다 이름으로 읽는다
        if (!peekTokenIs(TokenType::LPAREN) && !peekTokenIs(TokenType::ASSIGN))
        {
            noPrefixParseFnError(curToken_.type);
            return nullptr;
        }
        return parseIdentifier();
    };
    registerPrefixFn(TokenType::MAJIMAK, keywordAsCallee);
    registerPrefixFn(TokenType::BANBOKK, keywordAsCallee);
    registerPrefixFn(TokenType::MUNJA, [this]() { return parseIdentifier(); });  // "문자" 는 변수 이름으로 흔히 쓰인다

    // Infix 파싱 함수 등록
    registerInfixFn(TokenType::ASSIGN, [this](auto left) { return parseBinaryExpression(std::move(left)); });
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

    // 클래스 관련 파싱 함수 등록 (Phase 7.1)
    registerPrefixFn(TokenType::JASIN, [this]() { return parseThisExpression(); });
    registerInfixFn(TokenType::DOT, [this](auto left) {
        // 멤버 접근: object.member
        auto startLoc = left->location();
        if (!left)
        {
            return std::unique_ptr<Expression>(nullptr);
        }
        nextToken(); // DOT 다음으로 이동 (멤버 이름)

        if (!isNameToken(curToken_))
        {
            peekError(TokenType::IDENTIFIER);
            return std::unique_ptr<Expression>(nullptr);
        }

        std::string memberName = curToken_.literal;
        auto expr = std::make_unique<MemberAccessExpression>(std::move(left), memberName);
        expr->setLocation(startLoc);
        return std::unique_ptr<Expression>(std::move(expr));
    });
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
    recordError(msg, peekToken_.location);
}

void Parser::recordError(const std::string& message, const ast::SourceLocation& location)
{
    errors_.push_back(message);
    errorDetails_.push_back(ParseError{message, location});
}

void Parser::noPrefixParseFnError(TokenType type)
{
    std::string msg = "'" + tokenTypeToString(type) + "'으(로) 시작하는 표현식을 파싱할 수 없습니다.";
    recordError(msg, curToken_.location);
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
        case TokenType::DOT:
            return Precedence::INDEX;  // 멤버 접근은 INDEX와 같은 높은 우선순위
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
    // 명명 비동기 함수: 비동기 함수 이름(매개변수) { ... } → 이름 = 비동기 함수(매개변수) { ... }
    if (curTokenIs(TokenType::BIDONGGI) && peekTokenIs(TokenType::HAMSU) &&
        peekPeekToken_.type == TokenType::IDENTIFIER)
    {
        auto startLoc = curToken_.location;
        nextToken();  // 비동기 → 함수
        nextToken();  // 함수 → 이름
        std::string functionName = curToken_.literal;

        auto functionLiteral = parseAsyncFunctionRest(startLoc);
        if (!functionLiteral)
        {
            return nullptr;
        }

        auto stmt = std::make_unique<AssignmentStatement>(functionName, std::move(functionLiteral));
        stmt->setDeclarationForm(true);
        stmt->setLocation(startLoc);
        return stmt;
    }

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
        stmt->setDeclarationForm(true);
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

    // 조건 반복문: 반복 (조건) { } 또는 동안 (조건) { }
    if ((curTokenIs(TokenType::BANBOKK) || curTokenIs(TokenType::DONGAN)) && peekTokenIs(TokenType::LPAREN))
    {
        return parseWhileStatement();
    }

    // 예외 처리 문장: 시도 { } 오류 { } 마지막 { }
    if (curTokenIs(TokenType::SIDO))
    {
        return parseTryStatement();
    }

    // 예외 던지기: 던지다 expression
    if (curTokenIs(TokenType::DEONJIDA))
    {
        return parseThrowStatement();
    }

    // 클래스 정의 문장 (Phase 7.1)
    if (curTokenIs(TokenType::KEULLAESU))
    {
        return parseClassStatement();
    }

    // 할당 문장: identifier + "=" (범위 반복문보다 먼저 체크)
    // "반복 = 함수(...)", "문자 = ..." 처럼 키워드와 겹치는 이름도 할당문으로 읽어야
    // 의미 분석기가 그 이름을 변수·함수로 등록한다.
    if ((curTokenIs(TokenType::IDENTIFIER) || isNameToken(curToken_)) && peekTokenIs(TokenType::ASSIGN))
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

    // "i가 길이(경로) - 1부터 0 이상 {" 처럼 시작 식이 길어 LL(4) 로 잡지 못한 범위 반복문:
    // JosaExpression(i, 이/가, RangeExpression) 로 파싱되었으면 범위 반복문으로 다시 조립한다.
    if (auto* josa = node_cast<JosaExpression>(expr.get()))
    {
        auto* loopVar = node_cast<Identifier>(josa->object());
        auto* range = node_cast<RangeExpression>(josa->method());
        bool loopFollows = peekTokenIs(TokenType::LBRACE) || peekTokenIs(TokenType::BANBOKHANDA) ||
                           peekTokenIs(TokenType::BANBOKK);
        if (loopVar && range && josa->josaType() == lexer::JosaRecognizer::JosaType::I_GA && loopFollows)
        {
            std::string varName = loopVar->name();
            bool endInclusive = range->endInclusive();
            std::string endWord = range->endWord();
            auto method = josa->releaseMethod();
            auto* rangeNode = static_cast<RangeExpression*>(method.get());
            auto start = rangeNode->releaseStart();
            auto end = rangeNode->releaseEnd();

            std::string loopWord = parseOptionalLoopWord();
            if (!expectPeek(TokenType::LBRACE))
            {
                return nullptr;
            }
            auto body = parseBlockStatement();
            auto loop = std::make_unique<RangeForStatement>(varName, std::move(start), std::move(end),
                                                            std::move(body), endInclusive);
            loop->setLocation(startLoc);
            loop->setEndWord(endWord);
            loop->setLoopWord(loopWord);
            return loop;
        }
    }

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

    // ASSIGN이 바로 오면 변수명이 누락된 것
    if (curTokenIs(TokenType::ASSIGN))
    {
        std::string msg = "변수 선언에서 변수명이 누락되었습니다. '" + typeName +
                         "' 타입 뒤에 변수명을 입력해주세요. (예: " + typeName + " 변수명 = 값)";
        recordError(msg, curToken_.location);
        return nullptr;
    }

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
    // "만약 (조건) {", "만약 조건 {", "만약 (a) && (b) {" 모두 지원
    nextToken(); // 조건식 시작

    bool hasParentheses = false;
    auto condition = parseCondition(hasParentheses);

    // then 블록 파싱
    if (!expectPeek(TokenType::LBRACE))
    {
        return nullptr;
    }

    auto thenBranch = parseBlockStatement();

    // else 블록 (optional)
    std::unique_ptr<BlockStatement> elseBranch = nullptr;
    bool elseIf = false;

    if (peekTokenIs(TokenType::ANIMYEON))
    {
        nextToken(); // "아니면" 으로 이동

        if (peekTokenIs(TokenType::MANYAK))
        {
            // "아니면 만약 (조건) { }": else-if 사슬. 중첩 if 를 else 블록 하나로 감싼다.
            elseIf = true;
            nextToken(); // "만약" 으로 이동
            auto nested = parseIfStatement();
            if (!nested)
            {
                return nullptr;
            }
            auto block = std::make_unique<BlockStatement>();
            block->setLocation(nested->location());
            block->addStatement(std::move(nested));
            elseBranch = std::move(block);
        }
        else
        {
            if (!expectPeek(TokenType::LBRACE))
            {
                return nullptr;
            }

            elseBranch = parseBlockStatement();
        }
    }

    auto stmt = std::make_unique<IfStatement>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch)
    );
    stmt->setLocation(startLoc);
    stmt->setElseIf(elseIf);
    stmt->setConditionParenthesized(hasParentheses);
    return stmt;
}

std::unique_ptr<WhileStatement> Parser::parseWhileStatement()
{
    auto startLoc = curToken_.location;  // "반복" 또는 "동안" 위치
    std::string keyword = curToken_.literal;

    // 조건은 '(' 로 시작한다. "반복 (a) && (b) {" 처럼 괄호 뒤로 이어질 수 있다
    if (!expectPeek(TokenType::LPAREN))
    {
        return nullptr;
    }

    bool wrapped = false;
    auto condition = parseCondition(wrapped);

    if (!expectPeek(TokenType::LBRACE))
    {
        return nullptr;
    }

    auto body = parseBlockStatement();

    auto stmt = std::make_unique<WhileStatement>(std::move(condition), std::move(body));
    stmt->setLocation(startLoc);
    stmt->setKeyword(keyword);
    stmt->setConditionParenthesized(wrapped);
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

std::unique_ptr<Expression> Parser::parseCondition(bool& wrapped)
{
    // 조건 전체를 감싼 괄호는 문장의 괄호로 보고 식에서는 지운다 (포매터가 문장 쪽에서 쓴다)
    auto condition = parseExpression(Precedence::LOWEST);
    wrapped = condition && condition->parenthesized();
    if (wrapped)
    {
        condition->setParenthesized(false);
    }
    return condition;
}

std::string Parser::parseOptionalLoopWord()
{
    // 범위 반복문 끝의 선택 키워드: "반복한다", "반복", "반복 하라"
    if (peekTokenIs(TokenType::BANBOKHANDA))
    {
        nextToken(); // "반복한다"로 이동
        return curToken_.literal;
    }
    if (peekTokenIs(TokenType::BANBOKK))
    {
        nextToken(); // "반복"으로 이동
        std::string word = curToken_.literal;

        // "하라" 확인 (선택적)
        if (peekTokenIs(TokenType::HARA))
        {
            nextToken(); // "하라"로 이동
            word += " " + curToken_.literal;
        }
        return word;
    }
    return "";
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
        recordError(msg, curToken_.location);
        return nullptr;
    }

    nextToken(); // 범위 종료 키워드로 이동
    TokenType endToken = curToken_.type;
    std::string endWord = curToken_.literal;

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
    std::string loopWord = parseOptionalLoopWord();

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
    stmt->setEndWord(endWord);
    stmt->setLoopWord(loopWord);
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

    // 가져오기 "경로" 또는 가져오기("경로")
    bool parenthesized = peekTokenIs(TokenType::LPAREN);
    if (parenthesized)
    {
        nextToken();
    }

    // 다음 토큰은 문자열이어야 함
    if (!expectPeek(TokenType::STRING))
    {
        return nullptr;
    }

    std::string modulePath = curToken_.literal;

    if (parenthesized && !expectPeek(TokenType::RPAREN))
    {
        return nullptr;
    }

    auto stmt = std::make_unique<ImportStatement>(modulePath);
    stmt->setLocation(startLoc);
    return stmt;
}

/**
 * @brief 던지다 문장 파싱
 *
 * 문법: `던지다 <expression> ;`
 *
 * @example
 * 던지다 "에러 메시지";
 * 던지다 에러_생성("문제 발생");
 */
std::unique_ptr<ThrowStatement> Parser::parseThrowStatement()
{
    // 현재 토큰은 "던지다"
    auto startLoc = curToken_.location;

    // 다음 토큰부터 던질 값 파싱
    nextToken();
    auto value = parseExpression(Precedence::LOWEST);

    if (!value)
    {
        recordError("던지다 문장에서 던질 값을 파싱할 수 없습니다.", curToken_.location);
        return nullptr;
    }

    // 선택적 세미콜론
    if (peekTokenIs(TokenType::SEMICOLON))
    {
        nextToken();
    }

    auto stmt = std::make_unique<ThrowStatement>(std::move(value));
    stmt->setLocation(startLoc);
    return stmt;
}

/**
 * @brief 시도-오류-마지막 블록 파싱
 *
 * 문법: 시도 { ... } [오류 (errorVar) { ... }]* [마지막 { ... }]
 *
 * @example
 * 시도 {
 *     결과 = 10 / 0;
 * } 오류 (e) {
 *     출력(e);
 * } 마지막 {
 *     정리();
 * }
 */
std::unique_ptr<TryStatement> Parser::parseTryStatement()
{
    // 현재 토큰은 "시도"
    auto startLoc = curToken_.location;

    // Try 블록 파싱
    if (!expectPeek(TokenType::LBRACE))
    {
        return nullptr;
    }

    auto tryBlock = parseBlockStatement();
    if (!tryBlock)
    {
        return nullptr;
    }

    // Catch 절 파싱 (0개 이상)
    std::vector<std::unique_ptr<CatchClause>> catchClauses;

    while (peekTokenIs(TokenType::ORYU))
    {
        nextToken(); // "오류"로 이동

        // 에러 변수명 파싱: (errorVarName)
        if (!expectPeek(TokenType::LPAREN))
        {
            return nullptr;
        }

        if (!expectPeek(TokenType::IDENTIFIER))
        {
            return nullptr;
        }

        std::string errorVarName = curToken_.literal;

        if (!expectPeek(TokenType::RPAREN))
        {
            return nullptr;
        }

        // Catch 블록 파싱
        if (!expectPeek(TokenType::LBRACE))
        {
            return nullptr;
        }

        auto catchBody = parseBlockStatement();
        if (!catchBody)
        {
            return nullptr;
        }

        catchClauses.push_back(
            std::make_unique<CatchClause>(errorVarName, std::move(catchBody))
        );
    }

    // Finally 블록 파싱 (선택적)
    std::unique_ptr<BlockStatement> finallyBlock = nullptr;

    if (peekTokenIs(TokenType::MAJIMAK))
    {
        nextToken(); // "마지막"으로 이동

        if (!expectPeek(TokenType::LBRACE))
        {
            return nullptr;
        }

        finallyBlock = parseBlockStatement();
        if (!finallyBlock)
        {
            return nullptr;
        }
    }

    // 최소 하나의 catch 또는 finally가 있어야 함
    if (catchClauses.empty() && !finallyBlock)
    {
        recordError("시도 문장에는 최소 하나의 오류 절 또는 마지막 절이 필요합니다.", curToken_.location);
        return nullptr;
    }

    auto stmt = std::make_unique<TryStatement>(
        std::move(tryBlock),
        std::move(catchClauses),
        std::move(finallyBlock)
    );
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

        // prefix 가 실패했으면 infix 를 이어 갈 수 없다 (null 역참조 방지)
        if (!leftExp)
        {
            return nullptr;
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
        recordError("'" + curToken_.literal + "'을(를) 정수로 변환할 수 없습니다.", curToken_.location);
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
        recordError("'" + curToken_.literal + "'을(를) 실수로 변환할 수 없습니다.", curToken_.location);
        return nullptr;
    }
}

std::unique_ptr<Expression> Parser::parseStringLiteral()
{
    auto startLoc = curToken_.location;
    std::string str = curToken_.literal;

    // Phase 7.2: 문자열 보간 확인
    if (str.find("${") == std::string::npos)
    {
        // 일반 문자열 리터럴
        auto node = std::make_unique<StringLiteral>(str);
        node->setLocation(startLoc);
        return node;
    }

    // 문자열 보간 처리
    std::vector<std::string> parts;
    std::vector<std::unique_ptr<Expression>> expressions;

    size_t pos = 0;
    while (pos < str.length())
    {
        // "${" 찾기
        size_t start = str.find("${", pos);
        if (start == std::string::npos)
        {
            // 마지막 부분
            parts.push_back(str.substr(pos));
            break;
        }

        // "${" 앞부분을 part로 추가
        parts.push_back(str.substr(pos, start - pos));

        // "}" 찾기
        size_t end = str.find("}", start + 2);
        if (end == std::string::npos)
        {
            recordError("문자열 보간에서 닫는 '}'을(를) 찾을 수 없습니다.", curToken_.location);
            return nullptr;
        }

        // "${" 와 "}" 사이의 표현식 코드
        std::string exprCode = str.substr(start + 2, end - start - 2);

        // 표현식 파싱 (간단한 경우만 지원: 식별자, 산술 연산)
        // 미니 Lexer + Parser 생성
        lexer::Lexer exprLexer(exprCode, "");
        Parser exprParser(exprLexer);
        auto expr = exprParser.parseExpression(Precedence::LOWEST);

        if (!expr || !exprParser.errors().empty())
        {
            recordError("문자열 보간 표현식 파싱 실패: " + exprCode, curToken_.location);
            return nullptr;
        }

        expressions.push_back(std::move(expr));
        pos = end + 1;
    }

    // InterpolatedString 노드 생성
    auto node = std::make_unique<InterpolatedString>(std::move(parts), std::move(expressions));
    node->setLocation(startLoc);
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

    if (expr)
    {
        expr->setParenthesized(true);
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

std::unique_ptr<Expression> Parser::parseDictionaryLiteral()
{
    auto startLoc = curToken_.location;  // { 토큰 위치 저장

    std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>> pairs;

    // 빈 딕셔너리 체크
    if (peekTokenIs(TokenType::RBRACE))
    {
        nextToken(); // } 건너뛰기
        auto expr = std::make_unique<DictionaryLiteral>(std::move(pairs));
        expr->setLocation(startLoc);
        return expr;
    }

    // 첫 번째 key:value 쌍 파싱
    nextToken(); // 첫 key 시작
    auto key = parseExpression(Precedence::LOWEST);

    if (!expectPeek(TokenType::COLON))
    {
        return nullptr;
    }

    nextToken(); // value 시작
    auto value = parseExpression(Precedence::LOWEST);

    pairs.push_back(std::make_pair(std::move(key), std::move(value)));

    // 나머지 key:value 쌍 파싱
    while (peekTokenIs(TokenType::COMMA))
    {
        nextToken(); // ,
        nextToken(); // 다음 key

        key = parseExpression(Precedence::LOWEST);

        if (!expectPeek(TokenType::COLON))
        {
            return nullptr;
        }

        nextToken(); // value 시작
        value = parseExpression(Precedence::LOWEST);

        pairs.push_back(std::make_pair(std::move(key), std::move(value)));
    }

    if (!expectPeek(TokenType::RBRACE))
    {
        return nullptr;
    }

    auto expr = std::make_unique<DictionaryLiteral>(std::move(pairs));
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
    if (!function)
    {
        return nullptr;
    }
    auto startLoc = function->location();  // 함수 표현식 위치 사용
    auto arguments = parseExpressionList(TokenType::RPAREN);
    auto expr = std::make_unique<CallExpression>(std::move(function), std::move(arguments));
    expr->setLocation(startLoc);
    return expr;
}

std::unique_ptr<Expression> Parser::parseIndexExpression(std::unique_ptr<Expression> left)
{
    if (!left)
    {
        return nullptr;
    }
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
    if (!left)
    {
        return nullptr;
    }
    auto startLoc = left->location();  // 좌측 표현식 위치 사용
    // 현재 토큰은 조사 (EUL, REUL, I, GA 등)
    lexer::JosaRecognizer::JosaType josaType = tokenToJosaType(curToken_.type);
    std::string particle = curToken_.literal;

    nextToken(); // 조사 다음 토큰으로 이동 (메서드/명사)

    // 메서드/명사 파싱. 범위("1부터 10까지")가 올 수 있어 가장 낮은 우선순위로 읽는다.
    auto method = parseExpression(Precedence::LOWEST);
    if (!method)
    {
        return nullptr;
    }

    // 조사는 바로 뒤의 동사 하나에만 붙는다. 그런데 method 는 가장 낮은 우선순위로 읽었으므로
    // "x를 f() + 1" 은 (f() + 1) 로, "x를 f()를 g()" 는 (f()를 g()) 로 들어온다. 그 식의 가장 왼쪽 잎
    // (f())을 꺼내 (x를 f()) 로 감싸고 제자리에 끼워 ((x를 f()) + 1), ((x를 f())를 g()) 로 만든다.
    // 이항식과 조사식을 번갈아 내려가며 잎을 찾고, "값을 문자열로 변환한다" 같은 변환 관용구는 하나의
    // 단위이므로 쪼개지 않는다.
    if (Expression* parent = chainParent(method.get()))
    {
        std::unique_ptr<Expression> leaf;
        if (auto* binary = node_cast<BinaryExpression>(parent))
        {
            leaf = binary->releaseLeft();
        }
        else
        {
            leaf = static_cast<JosaExpression*>(parent)->releaseObject();
        }
        auto josa = std::make_unique<JosaExpression>(std::move(left), josaType, std::move(leaf), particle);
        josa->setLocation(startLoc);
        if (auto* binary = node_cast<BinaryExpression>(parent))
        {
            binary->setLeft(std::move(josa));
        }
        else
        {
            static_cast<JosaExpression*>(parent)->setObject(std::move(josa));
        }
        return method;
    }

    auto expr = std::make_unique<JosaExpression>(std::move(left), josaType, std::move(method), particle);
    expr->setLocation(startLoc);
    return expr;
}

bool Parser::isConversionIdiom(const JosaExpression* josa) const
{
    // "값을 <이름>으로 변환한다/바꾼다": 로/으로 조사식의 객체와 동사가 모두 식별자일 때
    if (josa->josaType() != lexer::JosaRecognizer::JosaType::RO_EURO)
    {
        return false;
    }
    auto* verb = node_cast<Identifier>(josa->method());
    return verb && node_cast<Identifier>(josa->object()) &&
           (verb->name() == "변환한다" || verb->name() == "바꾼다");
}

Expression* Parser::chainParent(Expression* method) const
{
    // method 의 가장 왼쪽 잎을 품은 노드를 돌려준다. 이항식은 왼쪽으로, 조사식은 객체 쪽으로 내려간다.
    // 내려갈 수 없는 노드(식별자, 호출, 범위, 변환 관용구 등)가 method 자체면 nullptr.
    auto descend = [this](Expression* node) -> Expression* {
        if (node->parenthesized())
        {
            return nullptr;  // 괄호로 묶은 식은 한 덩어리다
        }
        if (auto* binary = node_cast<BinaryExpression>(node))
        {
            return binary->leftMutable();
        }
        if (auto* josa = node_cast<JosaExpression>(node))
        {
            return isConversionIdiom(josa) ? nullptr : josa->objectMutable();
        }
        return nullptr;
    };

    Expression* parent = nullptr;
    Expression* node = method;
    while (Expression* child = descend(node))
    {
        parent = node;
        node = child;
    }
    return parent;
}

std::unique_ptr<Expression> Parser::parseRangeExpression(std::unique_ptr<Expression> left)
{
    auto startLoc = left->location();  // 좌측 표현식 (시작값) 위치 사용
    // 복합 연산자: 부터 ... 까지를 한 번에 파싱
    // left는 시작 값, curToken은 범위 시작 키워드 (BUTEO, CHOGA, ISANG)

    TokenType startToken = curToken_.type;
    std::string startWord = curToken_.literal;
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
        recordError(msg, curToken_.location);
        return nullptr;
    }

    // 끝 키워드로 이동
    nextToken();

    // 끝 키워드에 따라 inclusive 결정
    bool endInclusive = true;
    TokenType endToken = curToken_.type;
    std::string endWord = curToken_.literal;

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

    // Phase 7.2: Step 파싱 ("씩" 키워드)
    // 문법: [0부터 10까지 2씩] - step 값이 "씩" 앞에 옴
    std::unique_ptr<Expression> step = nullptr;
    if (peekTokenIs(TokenType::INTEGER) || peekTokenIs(TokenType::IDENTIFIER))
    {
        // step 값 파싱
        nextToken(); // step 값으로 이동
        step = parseExpression(Precedence::SUM, ParseFeature::All & ~ParseFeature::Range);

        if (!step)
        {
            recordError("step 값 파싱에 실패했습니다.", curToken_.location);
            return nullptr;
        }

        // "씩" 키워드 확인
        if (!peekTokenIs(TokenType::SSIK))
        {
            // step 값 없이 다른 토큰이 온 경우 - 에러가 아니라 step이 없는 것으로 처리
            // 하지만 이미 step 값을 파싱했으므로 에러
            recordError("step 값 뒤에 '씩' 키워드가 필요합니다.", curToken_.location);
            return nullptr;
        }

        nextToken(); // "씩" 토큰으로 이동
    }

    auto expr = std::make_unique<RangeExpression>(std::move(left), std::move(end),
                                                   startInclusive, endInclusive, std::move(step));
    expr->setLocation(startLoc);
    expr->setWords(startWord, endWord);
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
    std::vector<std::string> parameterTypes;

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
        std::string name;
        std::string typeName;
        if (!readParameterName(name, &typeName))
        {
            return nullptr;
        }
        parameters.push_back(name);
        parameterTypes.push_back(typeName);

        // 나머지 매개변수들 (COMMA로 구분)
        while (peekTokenIs(TokenType::COMMA))
        {
            nextToken();  // 현재를 COMMA로
            nextToken();  // 다음 매개변수로

            if (!readParameterName(name, &typeName))
            {
                return nullptr;
            }
            parameters.push_back(name);
            parameterTypes.push_back(typeName);
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
    expr->setParameterTypes(std::move(parameterTypes));
    expr->setLocation(startLoc);
    return expr;
}

// ============================================================================
// 비동기 프로그래밍 파싱 (Phase 7.3)
// ============================================================================

/**
 * @brief 비동기 함수 리터럴 파싱
 *
 * 문법: 비동기 함수(매개변수...) { 본문 }
 * 일반 함수와 거의 동일하지만 AsyncFunctionLiteral을 반환합니다.
 *
 * @return AsyncFunctionLiteral 표현식
 */
std::unique_ptr<Expression> Parser::parseAsyncFunctionLiteral()
{
    // 현재 토큰은 "비동기" 키워드
    auto startLoc = curToken_.location;

    // 다음 토큰이 "함수" 키워드인지 확인
    if (!expectPeek(TokenType::HAMSU))
    {
        recordError("'비동기' 다음에는 '함수' 키워드가 와야 합니다", curToken_.location);
        return nullptr;
    }

    return parseAsyncFunctionRest(startLoc);
}

/**
 * @brief "(" 부터 본문까지 읽어 비동기 함수 리터럴을 만든다
 *
 * 현재 토큰은 "(" 바로 앞(`함수` 키워드 또는 명명 선언의 이름)이어야 한다.
 */
std::unique_ptr<Expression> Parser::parseAsyncFunctionRest(const ast::SourceLocation& startLoc)
{
    // 다음 토큰이 LPAREN "(" 인지 확인
    if (!expectPeek(TokenType::LPAREN))
    {
        return nullptr;
    }

    // 매개변수 리스트 파싱
    std::vector<std::string> parameters;
    std::vector<std::string> parameterTypes;

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
        std::string name;
        std::string typeName;
        if (!readParameterName(name, &typeName))
        {
            return nullptr;
        }
        parameters.push_back(name);
        parameterTypes.push_back(typeName);

        // 나머지 매개변수들 (COMMA로 구분)
        while (peekTokenIs(TokenType::COMMA))
        {
            nextToken();  // 현재를 COMMA로
            nextToken();  // 다음 매개변수로

            if (!readParameterName(name, &typeName))
            {
                return nullptr;
            }
            parameters.push_back(name);
            parameterTypes.push_back(typeName);
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

    auto expr = std::make_unique<AsyncFunctionLiteral>(std::move(parameters), std::move(body));
    expr->setParameterTypes(std::move(parameterTypes));
    expr->setLocation(startLoc);
    return expr;
}

/**
 * @brief await 표현식 파싱
 *
 * 문법: `대기 <표현식>`
 * Promise를 반환하는 표현식을 기다립니다.
 *
 * @return AwaitExpression 표현식
 */
std::unique_ptr<Expression> Parser::parseAwaitExpression()
{
    // 현재 토큰은 "대기" 키워드
    auto startLoc = curToken_.location;

    // 다음 표현식 파싱 (PREFIX precedence로 파싱)
    nextToken();

    auto argument = parseExpression(Precedence::PREFIX);

    if (!argument)
    {
        recordError("'대기' 다음에는 표현식이 와야 합니다", curToken_.location);
        return nullptr;
    }

    auto expr = std::make_unique<AwaitExpression>(std::move(argument));
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
        recordError("패턴 매칭에는 '{'가 필요합니다", curToken_.location);
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
        recordError("패턴 매칭에는 '}'가 필요합니다", curToken_.location);
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
        recordError("패턴 파싱 실패", curToken_.location);
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
        recordError("패턴 매칭 케이스에는 '->'가 필요합니다", curToken_.location);
        return MatchCase(std::move(pattern), nullptr, nullptr);
    }

    nextToken(); // 본문 표현식으로 이동

    auto body = parseExpression(Precedence::LOWEST);

    if (!body)
    {
        recordError("패턴 매칭 본문 파싱 실패", curToken_.location);
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

    recordError("알 수 없는 패턴: " + tokenTypeToString(curToken_.type), curToken_.location);
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

// ============================================================================
// 클래스 파싱 (Phase 7.1)
// ============================================================================

/**
 * @brief 클래스 정의 파싱
 *
 * 문법: `클래스 <이름> [상속 <부모클래스>] { ... }`
 *
 * @example
 * 클래스 사람 {
 *     비공개 문자열 이름
 *     공개 정수 나이
 *
 *     생성자(이름, 나이) {
 *         자신.이름 = 이름
 *         자신.나이 = 나이
 *     }
 *
 *     공개 함수 인사하기() {
 *         출력("안녕하세요, " + 자신.이름 + "입니다")
 *     }
 * }
 */
std::unique_ptr<ClassStatement> Parser::parseClassStatement()
{
    auto startLoc = curToken_.location;  // "클래스" 키워드 위치 저장

    // 클래스 이름
    if (!expectPeek(TokenType::IDENTIFIER))
    {
        return nullptr;
    }

    std::string className = curToken_.literal;

    // 상속 처리 (선택적)
    std::string superClass;
    if (peekTokenIs(TokenType::SANGSOK))
    {
        nextToken();  // "상속"으로 이동

        if (!expectPeek(TokenType::IDENTIFIER))
        {
            return nullptr;
        }

        superClass = curToken_.literal;
    }

    // 클래스 본문 시작 "{"
    if (!expectPeek(TokenType::LBRACE))
    {
        return nullptr;
    }

    nextToken();  // "{" 다음으로 이동

    // 클래스 멤버 파싱
    std::vector<std::unique_ptr<FieldDeclaration>> fields;
    std::unique_ptr<ConstructorDeclaration> constructor;
    std::vector<std::unique_ptr<MethodDeclaration>> methods;

    while (!curTokenIs(TokenType::RBRACE) && !curTokenIs(TokenType::EOF_TOKEN))
    {
        // 접근 제한자 확인
        AccessModifier access = AccessModifier::PUBLIC;  // 기본값: 공개

        if (curTokenIs(TokenType::GONGGAE))
        {
            access = AccessModifier::PUBLIC;
            nextToken();  // 접근 제한자 건너뛰기
        }
        else if (curTokenIs(TokenType::BIGONGGAE))
        {
            access = AccessModifier::PRIVATE;
            nextToken();  // 접근 제한자 건너뛰기
        }

        // 생성자인지 확인
        if (curTokenIs(TokenType::SAENGSEONGJA))
        {
            if (constructor)
            {
                recordError("클래스에는 하나의 생성자만 허용됩니다.", curToken_.location);
                return nullptr;
            }

            constructor = parseConstructorDeclaration();
            if (!constructor)
            {
                return nullptr;
            }
        }
        // 메서드인지 확인 (함수 키워드)
        else if (curTokenIs(TokenType::HAMSU))
        {
            auto method = parseMethodDeclaration(access);
            if (!method)
            {
                return nullptr;
            }
            methods.push_back(std::move(method));
        }
        // 필드 선언 (타입 키워드로 시작)
        else if (curTokenIs(TokenType::JEONGSU) || curTokenIs(TokenType::SILSU) ||
                 curTokenIs(TokenType::MUNJAYEOL) || curTokenIs(TokenType::NONLI) ||
                 curTokenIs(TokenType::BAEYEOL))
        {
            auto field = parseFieldDeclaration(access);
            if (!field)
            {
                return nullptr;
            }
            fields.push_back(std::move(field));
        }
        else
        {
            recordError("클래스 멤버로 예상하지 못한 토큰: " + tokenTypeToString(curToken_.type), curToken_.location);
            return nullptr;
        }

        nextToken();  // 다음 멤버로 이동
    }

    if (!curTokenIs(TokenType::RBRACE))
    {
        peekError(TokenType::RBRACE);
        return nullptr;
    }

    auto stmt = std::make_unique<ClassStatement>(
        className,
        std::move(fields),
        std::move(constructor),
        std::move(methods),
        superClass
    );
    stmt->setLocation(startLoc);
    return stmt;
}

/**
 * @brief 필드 선언 파싱
 *
 * 문법: `[공개|비공개] <타입> <이름> [= <초기값>]`
 *
 * @example
 * 비공개 문자열 이름 = "홍길동"
 * 공개 정수 나이
 */
std::unique_ptr<FieldDeclaration> Parser::parseFieldDeclaration(AccessModifier access)
{
    auto startLoc = curToken_.location;  // 타입 키워드 위치 저장

    // 타입 이름
    std::string typeName = curToken_.literal;

    // 필드 이름
    if (!expectPeek(TokenType::IDENTIFIER))
    {
        return nullptr;
    }

    std::string fieldName = curToken_.literal;

    // 초기화 표현식 (선택적)
    std::unique_ptr<Expression> initializer = nullptr;

    if (peekTokenIs(TokenType::ASSIGN))
    {
        nextToken();  // "=" 로 이동
        nextToken();  // 초기값 표현식으로 이동

        initializer = parseExpression(Precedence::LOWEST);

        if (!initializer)
        {
            return nullptr;
        }
    }

    auto field = std::make_unique<FieldDeclaration>(access, typeName, fieldName, std::move(initializer));
    field->setLocation(startLoc);
    return field;
}

/**
 * @brief 메서드 선언 파싱
 *
 * 문법: `[공개|비공개] 함수 <이름>(<매개변수>) { ... }`
 *
 * @example
 * 공개 함수 인사하기() {
 *     출력("안녕하세요")
 * }
 */
bool Parser::isNameToken(const Token& token) const
{
    if (token.type == TokenType::IDENTIFIER)
    {
        return true;
    }
    // 리터럴·구두점·끝 토큰은 이름이 될 수 없다
    switch (token.type)
    {
        case TokenType::STRING:
        case TokenType::INTEGER:
        case TokenType::FLOAT:
        case TokenType::ILLEGAL:
        case TokenType::EOF_TOKEN:
            return false;
        default:
            break;
    }
    if (token.literal.empty())
    {
        return false;
    }
    unsigned char first = static_cast<unsigned char>(token.literal[0]);
    bool hangul = (first & 0xF0) == 0xE0;
    return hangul || std::isalpha(first) || first == '_';
}

std::unique_ptr<MethodDeclaration> Parser::parseMethodDeclaration(AccessModifier access)
{
    auto startLoc = curToken_.location;  // "함수" 키워드 위치 저장

    // 메서드 이름. "가져오기" 처럼 키워드와 겹치는 이름도 메서드 이름으로는 허용한다.
    if (!isNameToken(peekToken_))
    {
        peekError(TokenType::IDENTIFIER);
        return nullptr;
    }
    nextToken();

    std::string methodName = curToken_.literal;

    // 매개변수 리스트 파싱
    if (!expectPeek(TokenType::LPAREN))
    {
        return nullptr;
    }

    auto parameters = parseParameterList();

    // 메서드 본문
    if (!expectPeek(TokenType::LBRACE))
    {
        return nullptr;
    }

    auto body = parseBlockStatement();

    if (!body)
    {
        return nullptr;
    }

    // 반환 타입은 빈 문자열 (나중에 타입 추론으로 결정)
    std::string returnType = "";

    auto method = std::make_unique<MethodDeclaration>(
        access,
        returnType,
        methodName,
        std::move(parameters),
        std::move(body)
    );
    method->setLocation(startLoc);
    return method;
}

/**
 * @brief 생성자 선언 파싱
 *
 * 문법: `생성자(<매개변수>) { ... }`
 *
 * @example
 * 생성자(이름, 나이) {
 *     자신.이름 = 이름
 *     자신.나이 = 나이
 * }
 */
std::unique_ptr<ConstructorDeclaration> Parser::parseConstructorDeclaration()
{
    auto startLoc = curToken_.location;  // "생성자" 키워드 위치 저장

    // 매개변수 리스트
    if (!expectPeek(TokenType::LPAREN))
    {
        return nullptr;
    }

    auto parameters = parseParameterList();

    // 생성자 본문
    if (!expectPeek(TokenType::LBRACE))
    {
        return nullptr;
    }

    auto body = parseBlockStatement();

    if (!body)
    {
        return nullptr;
    }

    auto constructor = std::make_unique<ConstructorDeclaration>(std::move(parameters), std::move(body));
    constructor->setLocation(startLoc);
    return constructor;
}

/**
 * @brief 매개변수 리스트 파싱
 *
 * 문법: ( [매개변수1, 매개변수2, ...] )
 *
 * 현재 구현: 타입 없이 이름만 사용 (나중에 타입 추론)
 * 향후 구현: 타입 명시 지원 (정수 x, 문자열 이름)
 */
bool Parser::readParameterName(std::string& name, std::string* typeName)
{
    if (typeName)
    {
        typeName->clear();
    }
    auto isTypeKeyword = [](TokenType type) {
        return type == TokenType::JEONGSU || type == TokenType::SILSU || type == TokenType::MUNJAYEOL ||
               type == TokenType::NONLI || type == TokenType::BAEYEOL || type == TokenType::MUNJA;
    };

    if (curTokenIs(TokenType::IDENTIFIER))
    {
        name = curToken_.literal;
        return true;
    }
    if (isTypeKeyword(curToken_.type))
    {
        // "배열 arr": 타입이 붙은 매개변수. 타입은 아직 검사하지 않고 이름만 쓴다.
        if (peekTokenIs(TokenType::IDENTIFIER))
        {
            if (typeName)
            {
                *typeName = curToken_.literal;
            }
            nextToken();
            name = curToken_.literal;
            return true;
        }
        // "함수(배열)": 타입 키워드를 매개변수 이름으로 쓴 경우. 식에서도 식별자로 읽히므로 허용한다.
        name = curToken_.literal;
        return true;
    }
    peekError(TokenType::IDENTIFIER);
    return false;
}

std::vector<Parameter> Parser::parseParameterList()
{
    std::vector<Parameter> parameters;

    // 현재 토큰은 LPAREN
    nextToken();  // 첫 매개변수 또는 RPAREN으로 이동

    // 빈 매개변수 리스트
    if (curTokenIs(TokenType::RPAREN))
    {
        return parameters;
    }

    // 첫 번째 매개변수
    std::string name;
    std::string typeName;
    if (!readParameterName(name, &typeName))
    {
        return parameters;
    }

    // 타입은 아직 검사하지 않고 표기만 남긴다
    parameters.emplace_back(typeName, name);

    // 나머지 매개변수들
    while (peekTokenIs(TokenType::COMMA))
    {
        nextToken();  // COMMA로 이동
        nextToken();  // 다음 매개변수로 이동

        if (!readParameterName(name, &typeName))
        {
            return parameters;
        }

        parameters.emplace_back(typeName, name);
    }

    // RPAREN 확인
    if (!expectPeek(TokenType::RPAREN))
    {
        return parameters;
    }

    return parameters;
}

/**
 * @brief this 표현식 파싱 (자신 키워드)
 *
 * 문법: 자신
 *
 * @example
 * 자신.이름
 * 자신.나이 = 20
 */
std::unique_ptr<Expression> Parser::parseThisExpression()
{
    auto expr = std::make_unique<ThisExpression>();
    expr->setLocation(curToken_.location);
    return expr;
}

/**
 * @brief 객체 생성 표현식 파싱
 *
 * 문법: `<클래스이름>(<인자...>)`
 *
 * Note: new 키워드 없이 클래스 이름으로 직접 생성
 *
 * @example
 * 사람 p = 사람("홍길동", 30)
 */
std::unique_ptr<Expression> Parser::parseNewExpression()
{
    auto startLoc = curToken_.location;  // 클래스 이름 위치 저장

    // 클래스 이름
    std::string className = curToken_.literal;

    // 생성자 인자 파싱
    if (!expectPeek(TokenType::LPAREN))
    {
        return nullptr;
    }

    auto arguments = parseExpressionList(TokenType::RPAREN);

    auto expr = std::make_unique<NewExpression>(className, std::move(arguments));
    expr->setLocation(startLoc);
    return expr;
}

} // namespace parser
} // namespace kingsejong
