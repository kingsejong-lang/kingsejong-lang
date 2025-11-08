#pragma once

/**
 * @file Parser.h
 * @brief KingSejong 언어 파서 (Pratt Parsing 알고리즘)
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "lexer/Lexer.h"
#include "lexer/Token.h"
#include "ast/Node.h"
#include "ast/Expression.h"
#include "ast/Statement.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

namespace kingsejong {
namespace parser {

using namespace lexer;
using namespace ast;

/**
 * @enum ParseFeature
 * @brief 표현식 파싱 시 허용할 문법 기능을 제어하는 플래그
 *
 * 비트마스크로 여러 기능을 조합할 수 있습니다.
 * 특정 컨텍스트에서 특정 문법 요소를 금지할 때 사용합니다.
 */
enum class ParseFeature : uint32_t
{
    None       = 0,              ///< 모든 기능 비활성화
    Range      = 1 << 0,         ///< 범위 표현식 (부터...까지) 허용
    // 향후 확장: Assignment = 1 << 1, Lambda = 1 << 2, Pipe = 1 << 3, ...
    All        = 0xFFFFFFFFu     ///< 모든 기능 활성화 (기본값)
};

/// ParseFeature 비트마스크 OR 연산자
inline ParseFeature operator|(ParseFeature a, ParseFeature b)
{
    return static_cast<ParseFeature>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
    );
}

/// ParseFeature 비트마스크 AND 연산자
inline ParseFeature operator&(ParseFeature a, ParseFeature b)
{
    return static_cast<ParseFeature>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b)
    );
}

/// ParseFeature 비트마스크 NOT 연산자
inline ParseFeature operator~(ParseFeature a)
{
    return static_cast<ParseFeature>(~static_cast<uint32_t>(a));
}

/// ParseFeature 플래그 확인 헬퍼
inline bool hasFeature(ParseFeature features, ParseFeature flag)
{
    return (static_cast<uint32_t>(features) & static_cast<uint32_t>(flag)) != 0;
}

/**
 * @class Parser
 * @brief Pratt Parsing 알고리즘을 사용한 파서
 *
 * Top-Down Operator Precedence (Pratt) Parsing을 사용하여
 * 토큰 스트림을 AST로 변환합니다.
 */
class Parser
{
public:
    /**
     * @enum Precedence
     * @brief 연산자 우선순위
     *
     * 낮은 값일수록 낮은 우선순위를 가집니다.
     */
    enum class Precedence
    {
        LOWEST = 0,         ///< 가장 낮은 우선순위
        ASSIGN,             ///< = (할당)
        RANGE,              ///< 부터...까지 (범위 연산자)
        OR,                 ///< || (논리 OR)
        AND,                ///< && (논리 AND)
        EQUALS,             ///< ==, != (동등 비교)
        LESSGREATER,        ///< <, >, <=, >= (크기 비교)
        SUM,                ///< +, - (덧셈, 뺄셈)
        PRODUCT,            ///< *, /, % (곱셈, 나눗셈, 나머지)
        PREFIX,             ///< -X, !X (단항 연산자)
        CALL,               ///< 함수호출()
        INDEX               ///< 배열[인덱스]
    };

    explicit Parser(Lexer& lexer);

    /**
     * @brief 전체 프로그램 파싱
     * @return Program AST 노드
     */
    std::unique_ptr<Program> parseProgram();

    /**
     * @brief 에러 메시지 목록 반환
     * @return 파싱 중 발생한 에러 메시지들
     */
    const std::vector<std::string>& errors() const { return errors_; }

private:
    // 타입 별칭
    using PrefixParseFn = std::function<std::unique_ptr<Expression>()>;
    using InfixParseFn = std::function<std::unique_ptr<Expression>(std::unique_ptr<Expression>)>;

    Lexer& lexer_;
    Token curToken_;
    Token peekToken_;
    std::vector<std::string> errors_;

    // Pratt Parsing 함수 맵
    std::unordered_map<TokenType, PrefixParseFn> prefixParseFns_;
    std::unordered_map<TokenType, InfixParseFn> infixParseFns_;

    // 토큰 관리
    void nextToken();
    bool curTokenIs(TokenType type) const;
    bool peekTokenIs(TokenType type) const;
    bool expectPeek(TokenType type);

    // 에러 처리
    void peekError(TokenType expected);
    void noPrefixParseFnError(TokenType type);

    // 우선순위
    Precedence curPrecedence() const;
    Precedence peekPrecedence() const;
    Precedence tokenPrecedence(TokenType type) const;

    // 문장 파싱
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<ExpressionStatement> parseExpressionStatement();
    std::unique_ptr<VarDeclaration> parseVarDeclaration();
    std::unique_ptr<ReturnStatement> parseReturnStatement();
    std::unique_ptr<IfStatement> parseIfStatement();
    std::unique_ptr<RepeatStatement> parseRepeatStatement();
    std::unique_ptr<RangeForStatement> parseRangeForStatement();
    std::unique_ptr<BlockStatement> parseBlockStatement();

    // 표현식 파싱 (Pratt Parsing 핵심)
    std::unique_ptr<Expression> parseExpression(
        Precedence precedence,
        ParseFeature features = ParseFeature::All
    );

    // Prefix 파싱 함수들
    std::unique_ptr<Expression> parseIdentifier();
    std::unique_ptr<Expression> parseIntegerLiteral();
    std::unique_ptr<Expression> parseFloatLiteral();
    std::unique_ptr<Expression> parseStringLiteral();
    std::unique_ptr<Expression> parseBooleanLiteral();
    std::unique_ptr<Expression> parsePrefixExpression();
    std::unique_ptr<Expression> parseGroupedExpression();
    std::unique_ptr<Expression> parseArrayLiteral();
    std::unique_ptr<Expression> parseFunctionLiteral();

    // Infix 파싱 함수들
    std::unique_ptr<Expression> parseBinaryExpression(std::unique_ptr<Expression> left);
    std::unique_ptr<Expression> parseCallExpression(std::unique_ptr<Expression> function);
    std::unique_ptr<Expression> parseIndexExpression(std::unique_ptr<Expression> left);
    std::unique_ptr<Expression> parseJosaExpression(std::unique_ptr<Expression> left);
    std::unique_ptr<Expression> parseRangeExpression(std::unique_ptr<Expression> left);

    // 헬퍼 함수들
    std::vector<std::unique_ptr<Expression>> parseExpressionList(TokenType endToken);
    std::vector<std::unique_ptr<Statement>> parseStatements(TokenType endToken);

    // 조사 파싱 헬퍼
    bool isJosaToken(TokenType type) const;
    lexer::JosaRecognizer::JosaType tokenToJosaType(TokenType type) const;

    // 범위 파싱 헬퍼
    bool isRangeStartToken(TokenType type) const;
    bool isRangeEndToken(TokenType type) const;

    // 파싱 함수 등록
    void registerPrefixFn(TokenType type, PrefixParseFn fn);
    void registerInfixFn(TokenType type, InfixParseFn fn);
    void registerParseFunctions();
};

} // namespace parser
} // namespace kingsejong
