/**
 * @file Parser.cpp
 * @brief KingSejong 언어 파서 구현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Parser.h"
#include "types/Type.h"
#include <stdexcept>

namespace kingsejong {
namespace parser {

Parser::Parser(Lexer& lexer)
    : lexer_(lexer)
{
    // 파싱 함수 등록
    registerParseFunctions();

    // 첫 두 토큰 읽기
    nextToken();
    nextToken();
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
    peekToken_ = lexer_.nextToken();
}

bool Parser::curTokenIs(TokenType type) const
{
    return curToken_.type == type;
}

bool Parser::peekTokenIs(TokenType type) const
{
    return peekToken_.type == type;
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
    std::string msg = "expected next token to be " +
                      tokenTypeToString(expected) +
                      ", got " + tokenTypeToString(peekToken_.type);
    errors_.push_back(msg);
}

void Parser::noPrefixParseFnError(TokenType type)
{
    std::string msg = "no prefix parse function for " + tokenTypeToString(type);
    errors_.push_back(msg);
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

    while (!curTokenIs(TokenType::EOF_TOKEN))
    {
        auto stmt = parseStatement();
        if (stmt)
        {
            program->addStatement(std::move(stmt));
        }
        nextToken();
    }

    return program;
}

// ============================================================================
// 문장 파싱
// ============================================================================

std::unique_ptr<Statement> Parser::parseStatement()
{
    // 타입 키워드로 시작하면 변수 선언
    if (curTokenIs(TokenType::JEONGSU) || curTokenIs(TokenType::SILSU) ||
        curTokenIs(TokenType::MUNJAYEOL) || curTokenIs(TokenType::NONLI))
    {
        return parseVarDeclaration();
    }

    // 반환 문장
    if (curTokenIs(TokenType::BANHWAN))
    {
        return parseReturnStatement();
    }

    // 조건 문장
    if (curTokenIs(TokenType::MANYAK))
    {
        return parseIfStatement();
    }

    // 범위 반복문: identifier + "가"/"이" (N번 반복보다 먼저 체크)
    if (curTokenIs(TokenType::IDENTIFIER) &&
        (peekTokenIs(TokenType::JOSA_GA) || peekTokenIs(TokenType::JOSA_I)))
    {
        return parseRangeForStatement();
    }

    // N번 반복문: integer/expression + "번"
    if (peekTokenIs(TokenType::BEON))
    {
        return parseRepeatStatement();
    }

    // 블록 문장
    if (curTokenIs(TokenType::LBRACE))
    {
        return parseBlockStatement();
    }

    // 기본: 표현식 문장
    return parseExpressionStatement();
}

std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement()
{
    auto expr = parseExpression(Precedence::LOWEST);

    // 선택적 세미콜론
    if (peekTokenIs(TokenType::SEMICOLON))
    {
        nextToken();
    }

    return std::make_unique<ExpressionStatement>(std::move(expr));
}

std::unique_ptr<VarDeclaration> Parser::parseVarDeclaration()
{
    std::string typeName = curToken_.literal;

    // 타입 이름으로 Type 객체 조회
    types::Type* varType = types::Type::getBuiltin(typeName);

    // 변수 이름
    if (!expectPeek(TokenType::IDENTIFIER))
    {
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
    }

    // 선택적 세미콜론
    if (peekTokenIs(TokenType::SEMICOLON))
    {
        nextToken();
    }

    return std::make_unique<VarDeclaration>(typeName, varName, std::move(initializer), varType);
}

std::unique_ptr<ReturnStatement> Parser::parseReturnStatement()
{
    nextToken(); // "반환" 건너뛰기

    std::unique_ptr<Expression> returnValue = nullptr;

    if (!curTokenIs(TokenType::SEMICOLON) && !curTokenIs(TokenType::EOF_TOKEN))
    {
        returnValue = parseExpression(Precedence::LOWEST);
    }

    // 선택적 세미콜론
    if (peekTokenIs(TokenType::SEMICOLON))
    {
        nextToken();
    }

    return std::make_unique<ReturnStatement>(std::move(returnValue));
}

std::unique_ptr<IfStatement> Parser::parseIfStatement()
{
    // "만약" 다음 LPAREN 확인
    if (!expectPeek(TokenType::LPAREN))
    {
        return nullptr;
    }

    nextToken(); // 조건식 시작

    auto condition = parseExpression(Precedence::LOWEST);

    if (!expectPeek(TokenType::RPAREN))
    {
        return nullptr;
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

    return std::make_unique<IfStatement>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch)
    );
}

std::unique_ptr<RepeatStatement> Parser::parseRepeatStatement()
{
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

    return std::make_unique<RepeatStatement>(
        std::move(count),
        std::move(body)
    );
}

std::unique_ptr<RangeForStatement> Parser::parseRangeForStatement()
{
    // 범위 for문: "i가 1부터 5까지 반복한다 { ... }"
    // ParseFeature::Range를 비활성화하여 start/end 표현식에서 RangeExpression이 생성되지 않도록 함

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

    // "까지" 명시적으로 확인
    if (!expectPeek(TokenType::KKAJI))
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

    return std::make_unique<RangeForStatement>(
        varName,
        std::move(start),
        std::move(end),
        std::move(body)
    );
}

std::unique_ptr<BlockStatement> Parser::parseBlockStatement()
{
    nextToken(); // { 건너뛰기

    auto statements = parseStatements(TokenType::RBRACE);

    return std::make_unique<BlockStatement>(std::move(statements));
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
    return std::make_unique<Identifier>(curToken_.literal);
}

std::unique_ptr<Expression> Parser::parseIntegerLiteral()
{
    try
    {
        int64_t value = std::stoll(curToken_.literal);
        return std::make_unique<IntegerLiteral>(value);
    }
    catch (const std::exception&)
    {
        errors_.push_back("could not parse " + curToken_.literal + " as integer");
        return nullptr;
    }
}

std::unique_ptr<Expression> Parser::parseFloatLiteral()
{
    try
    {
        double value = std::stod(curToken_.literal);
        return std::make_unique<FloatLiteral>(value);
    }
    catch (const std::exception&)
    {
        errors_.push_back("could not parse " + curToken_.literal + " as float");
        return nullptr;
    }
}

std::unique_ptr<Expression> Parser::parseStringLiteral()
{
    return std::make_unique<StringLiteral>(curToken_.literal);
}

std::unique_ptr<Expression> Parser::parseBooleanLiteral()
{
    bool value = curTokenIs(TokenType::CHAM);
    return std::make_unique<BooleanLiteral>(value);
}

std::unique_ptr<Expression> Parser::parsePrefixExpression()
{
    std::string op = curToken_.literal;
    nextToken();

    auto right = parseExpression(Precedence::PREFIX);
    return std::make_unique<UnaryExpression>(op, std::move(right));
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
    auto elements = parseExpressionList(TokenType::RBRACKET);
    return std::make_unique<ArrayLiteral>(std::move(elements));
}

// ============================================================================
// Infix 파싱 함수들
// ============================================================================

std::unique_ptr<Expression> Parser::parseBinaryExpression(std::unique_ptr<Expression> left)
{
    std::string op = curToken_.literal;
    Precedence precedence = curPrecedence();

    nextToken();
    auto right = parseExpression(precedence);

    return std::make_unique<BinaryExpression>(std::move(left), op, std::move(right));
}

std::unique_ptr<Expression> Parser::parseCallExpression(std::unique_ptr<Expression> function)
{
    auto arguments = parseExpressionList(TokenType::RPAREN);
    return std::make_unique<CallExpression>(std::move(function), std::move(arguments));
}

std::unique_ptr<Expression> Parser::parseIndexExpression(std::unique_ptr<Expression> left)
{
    nextToken(); // [ 건너뛰기

    auto index = parseExpression(Precedence::LOWEST);

    if (!expectPeek(TokenType::RBRACKET))
    {
        return nullptr;
    }

    return std::make_unique<IndexExpression>(std::move(left), std::move(index));
}

std::unique_ptr<Expression> Parser::parseJosaExpression(std::unique_ptr<Expression> left)
{
    // 현재 토큰은 조사 (EUL, REUL, I, GA 등)
    lexer::JosaRecognizer::JosaType josaType = tokenToJosaType(curToken_.type);

    nextToken(); // 조사 다음 토큰으로 이동 (메서드/명사)

    // 메서드/명사 파싱
    auto method = parseExpression(Precedence::LOWEST);

    return std::make_unique<JosaExpression>(std::move(left), josaType, std::move(method));
}

std::unique_ptr<Expression> Parser::parseRangeExpression(std::unique_ptr<Expression> left)
{
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
        std::string msg = "expected range end keyword (까지/미만/이하/이상), got " +
                         tokenTypeToString(peekToken_.type);
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

    return std::make_unique<RangeExpression>(std::move(left), std::move(end),
                                             startInclusive, endInclusive);
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
    // 현재 토큰은 HAMSU ("함수")

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

    return std::make_unique<FunctionLiteral>(std::move(parameters), std::move(body));
}

} // namespace parser
} // namespace kingsejong
