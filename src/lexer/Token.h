#pragma once

/**
 * @file Token.h
 * @brief KingSejong 언어의 토큰 정의
 * @author KingSejong Team
 * @date 2025-11-06
 */

#include <string>
#include <unordered_map>
#include "ast/SourceLocation.h"

namespace kingsejong {
namespace lexer {

/**
 * @enum TokenType
 * @brief 토큰의 타입을 나타내는 열거형
 *
 * KingSejong 언어의 모든 토큰 타입을 정의합니다.
 * 한글 조사, 범위 키워드, 반복 키워드 등을 포함합니다.
 */
enum class TokenType
{
    // 기본 토큰
    ILLEGAL,      ///< 잘못된 토큰
    EOF_TOKEN,    ///< 파일 끝
    IDENTIFIER,   ///< 식별자 (변수명, 함수명 등)
    INTEGER,      ///< 정수 리터럴
    FLOAT,        ///< 실수 리터럴
    STRING,       ///< 문자열 리터럴

    // 연산자
    ASSIGN,       ///< =
    PLUS,         ///< +
    MINUS,        ///< -
    ASTERISK,     ///< *
    SLASH,        ///< /
    PERCENT,      ///< %

    // 비교 연산자
    EQ,           ///< ==
    NOT_EQ,       ///< !=
    LT,           ///< <
    GT,           ///< >
    LE,           ///< <=
    GE,           ///< >=

    // 논리 연산자
    AND,          ///< &&
    OR,           ///< ||
    NOT,          ///< !

    // 조사 (Josa) - 목적격
    JOSA_EUL,     ///< 을 (받침 있을 때)
    JOSA_REUL,    ///< 를 (받침 없을 때)

    // 조사 - 주격
    JOSA_I,       ///< 이 (받침 있을 때)
    JOSA_GA,      ///< 가 (받침 없을 때)

    // 조사 - 보조사
    JOSA_EUN,     ///< 은 (받침 있을 때)
    JOSA_NEUN,    ///< 는 (받침 없을 때)

    // 조사 - 소유격
    JOSA_UI,      ///< 의

    // 조사 - 방향/수단
    JOSA_RO,      ///< 로 (받침 없거나 ㄹ받침)
    JOSA_EURO,    ///< 으로 (받침 있을 때, ㄹ 제외)

    // 조사 - 위치
    JOSA_ESO,     ///< 에서
    JOSA_E,       ///< 에

    // 범위 키워드
    BUTEO,        ///< 부터
    KKAJI,        ///< 까지
    MIMAN,        ///< 미만
    CHOGA,        ///< 초과
    IHA,          ///< 이하
    ISANG,        ///< 이상
    SSIK,         ///< 씩 (step)

    // 반복 키워드
    BEON,         ///< 번
    BANBOKK,      ///< 반복
    BANBOKHANDA,  ///< 반복한다
    GAKGAK,       ///< 각각

    // 제어문 키워드
    MANYAK,       ///< 만약 (if)
    ANIMYEON,     ///< 아니면 (else)
    ANIMYEON_MANYAK, ///< 아니면 만약 (else if)

    // 반복문 키워드
    DONGAN,       ///< 동안 (while)

    // 함수 키워드
    HAMSU,        ///< 함수
    BANHWAN,      ///< 반환 (return)

    // 모듈 키워드
    GAJYEOOGI,    ///< 가져오기 (import)

    // 타입 키워드
    JEONGSU,      ///< 정수 (integer)
    SILSU,        ///< 실수 (float)
    MUNJA,        ///< 문자 (char)
    MUNJAYEOL,    ///< 문자열 (string)
    NONLI,        ///< 논리 (boolean)
    BAEYEOL,      ///< 배열 (array)

    // 불리언 리터럴
    CHAM,         ///< 참 (true)
    GEOJIT,       ///< 거짓 (false)

    // 괄호
    LPAREN,       ///< (
    RPAREN,       ///< )
    LBRACE,       ///< {
    RBRACE,       ///< }
    LBRACKET,     ///< [
    RBRACKET,     ///< ]

    // 구분자
    COMMA,        ///< ,
    SEMICOLON,    ///< ;
    COLON,        ///< :
    DOT,          ///< . (Phase 7.1: 멤버 접근)

    // 메서드 체이닝 연산자
    HAGO,         ///< 하고 (method chaining)
    HARA,         ///< 하라 (execute)

    // 패턴 매칭 (F5.5)
    ARROW,        ///< -> (패턴 -> 결과)
    E_DAEHAE,     ///< 에 대해 (match 키워드)
    WHEN,         ///< when (가드 조건)
    UNDERSCORE,   ///< _ (와일드카드 패턴)

    // 예외 처리 키워드 (P0 Phase 2)
    SIDO,         ///< 시도 (try)
    ORYU,         ///< 오류 (catch)
    MAJIMAK,      ///< 마지막 (finally)
    DEONJIDA,     ///< 던지다 (throw)

    // 클래스 관련 키워드 (Phase 7.1)
    KEULLAESU,    ///< 클래스 (class)
    SAENGSEONGJA, ///< 생성자 (constructor)
    GONGGAE,      ///< 공개 (public)
    BIGONGGAE,    ///< 비공개 (private)
    JASIN,        ///< 자신 (this)
    SANGSOK,      ///< 상속 (extends)
    SANGWI,       ///< 상위 (super)

    // 비동기 프로그래밍 키워드 (Phase 7.3)
    BIDONGGI,     ///< 비동기 (async)
    DAEGI         ///< 대기 (await)
};

/**
 * @struct Token
 * @brief 토큰 구조체
 *
 * 토큰의 타입과 리터럴 값, 위치 정보를 저장합니다.
 */
struct Token
{
    TokenType type;                ///< 토큰 타입
    std::string literal;           ///< 토큰의 실제 문자열 값
    ast::SourceLocation location;  ///< 소스 코드 위치 정보

    /**
     * @brief Token 생성자
     * @param t 토큰 타입
     * @param lit 토큰 리터럴
     * @param l 줄 번호
     * @param c 열 번호
     */
    Token(TokenType t = TokenType::ILLEGAL, const std::string& lit = "", int l = 1, int c = 1)
        : type(t), literal(lit), location(l, c)
    {
    }

    /**
     * @brief 파일명을 포함한 Token 생성자
     * @param t 토큰 타입
     * @param lit 토큰 리터럴
     * @param loc 소스 위치
     */
    Token(TokenType t, const std::string& lit, const ast::SourceLocation& loc)
        : type(t), literal(lit), location(loc)
    {
    }
};

/**
 * @brief TokenType을 문자열로 변환
 * @param type 토큰 타입
 * @return 토큰 타입의 문자열 표현
 */
std::string tokenTypeToString(TokenType type);

/**
 * @brief 한글 키워드를 TokenType으로 변환
 * @param literal 한글 키워드 문자열
 * @return 해당하는 TokenType (키워드가 아니면 IDENTIFIER)
 */
TokenType lookupKeyword(const std::string& literal);

/**
 * @brief 한글 조사인지 확인
 * @param literal 문자열
 * @return 조사이면 true, 아니면 false
 */
bool isJosa(const std::string& literal);

/**
 * @brief 한글 범위 키워드인지 확인
 * @param literal 문자열
 * @return 범위 키워드이면 true, 아니면 false
 */
bool isRangeKeyword(const std::string& literal);

} // namespace lexer
} // namespace kingsejong
