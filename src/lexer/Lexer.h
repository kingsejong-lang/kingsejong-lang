#pragma once

/**
 * @file Lexer.h
 * @brief KingSejong 언어의 어휘 분석기 (Lexical Analyzer)
 * @author KingSejong Team
 * @date 2025-11-06
 */

#include "Token.h"
#include <string>
#include <cstddef>

namespace kingsejong {
namespace lexer {

/**
 * @class Lexer
 * @brief 소스 코드를 토큰으로 분해하는 어휘 분석기
 *
 * UTF-8 인코딩된 한글 소스 코드를 읽어서 토큰 스트림으로 변환합니다.
 * 한글 키워드, 조사, 식별자를 올바르게 인식합니다.
 */
class Lexer
{
public:
    /**
     * @brief Lexer 생성자
     * @param input 분석할 소스 코드 문자열 (UTF-8 인코딩)
     */
    explicit Lexer(const std::string& input);

    /**
     * @brief 다음 토큰을 반환
     * @return Token 객체
     *
     * 입력 문자열에서 다음 토큰을 읽어서 반환합니다.
     * 파일 끝에 도달하면 EOF_TOKEN을 반환합니다.
     */
    Token nextToken();

private:
    std::string input;          ///< 입력 소스 코드
    size_t position;            ///< 현재 읽는 위치
    size_t readPosition;        ///< 다음 읽을 위치
    char ch;                    ///< 현재 문자

    /**
     * @brief 다음 문자를 읽어서 ch에 저장
     *
     * position과 readPosition을 증가시키고,
     * 다음 문자를 ch에 저장합니다.
     */
    void readChar();

    /**
     * @brief 다음 문자를 미리 보기 (읽지 않고)
     * @return 다음 문자 (파일 끝이면 '\0')
     */
    char peekChar() const;

    /**
     * @brief 공백 문자를 건너뜀
     *
     * 스페이스, 탭, 개행 문자 등을 건너뜁니다.
     */
    void skipWhitespace();

    /**
     * @brief 식별자 또는 한글 키워드를 읽음
     * @return 식별자/키워드 문자열
     *
     * 한글, 영문, 숫자, 언더스코어로 구성된 식별자를 읽습니다.
     * 한글은 UTF-8로 인코딩되어 있으므로 멀티바이트 처리를 합니다.
     */
    std::string readIdentifier();

    /**
     * @brief 숫자를 읽음
     * @return 숫자 문자열
     *
     * 정수와 실수를 모두 지원합니다.
     * 예: 123, 3.14, -42
     */
    std::string readNumber();

    /**
     * @brief 문자열 리터럴을 읽음
     * @param quote 시작 따옴표 문자 (' 또는 ")
     * @return 문자열 내용 (따옴표 제외)
     *
     * 이스케이프 시퀀스를 처리합니다: \n, \t, \", \\
     */
    std::string readString(char quote);

    /**
     * @brief 현재 문자가 문자(한글/영문/언더스코어)인지 확인
     * @param c 확인할 문자
     * @return 문자이면 true, 아니면 false
     *
     * UTF-8 한글의 시작 바이트를 감지합니다.
     */
    bool isLetter(char c) const;

    /**
     * @brief 현재 문자가 숫자인지 확인
     * @param c 확인할 문자
     * @return 숫자이면 true, 아니면 false
     */
    bool isDigit(char c) const;

    /**
     * @brief 현재 문자가 UTF-8 한글의 시작 바이트인지 확인
     * @param c 확인할 문자
     * @return 한글 시작 바이트이면 true, 아니면 false
     *
     * UTF-8 한글은 3바이트로 인코딩되며, 첫 바이트는 0xE0-0xEF 범위입니다.
     */
    bool isKoreanStart(unsigned char c) const;

    /**
     * @brief UTF-8 문자 하나를 읽음
     * @return UTF-8 문자열 (1-4바이트)
     *
     * 현재 위치에서 UTF-8 문자 하나를 완전히 읽어서 반환합니다.
     * 한글은 3바이트, 영문/숫자는 1바이트입니다.
     */
    std::string readUTF8Char();

    /**
     * @brief 현재 문자가 UTF-8 멀티바이트 문자의 연속 바이트인지 확인
     * @param c 확인할 문자
     * @return 연속 바이트이면 true, 아니면 false
     *
     * UTF-8 연속 바이트는 10xxxxxx 패턴입니다.
     */
    bool isUTF8ContinuationByte(unsigned char c) const;

    /**
     * @brief 두 문자 연산자를 만들기
     * @param ch1 첫 번째 문자
     * @param ch2 두 번째 문자
     * @param type1 첫 번째 문자만 있을 때의 토큰 타입
     * @param type2 두 문자가 모두 있을 때의 토큰 타입
     * @return Token 객체
     *
     * 예: '=' 다음 '='가 오면 '==' (EQ), 아니면 '=' (ASSIGN)
     */
    Token makeTwoCharToken(char ch1, char ch2, TokenType type1, TokenType type2);
};

} // namespace lexer
} // namespace kingsejong
