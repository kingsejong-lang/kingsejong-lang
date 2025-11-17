/**
 * @file Lexer.cpp
 * @brief Lexer 클래스 구현
 * @author KingSejong Team
 * @date 2025-11-06
 */

#include "Lexer.h"
#include "JosaRecognizer.h"
#include <cctype>

namespace kingsejong {
namespace lexer {

Lexer::Lexer(const std::string& input)
    : input(input), filename(""), position(0), readPosition(0), ch('\0'), currentLine(1), currentColumn(1)
{
    // 첫 번째 문자 읽기
    readChar();
}

Lexer::Lexer(const std::string& input, const std::string& filename)
    : input(input), filename(filename), position(0), readPosition(0), ch('\0'), currentLine(1), currentColumn(1)
{
    // 첫 번째 문자 읽기
    readChar();
}

void Lexer::readChar()
{
    if (readPosition >= input.length())
    {
        ch = '\0';  // EOF
    }
    else
    {
        ch = input[readPosition];
    }

    // 위치 정보 업데이트
    if (ch == '\n')
    {
        currentLine++;
        currentColumn = 1;
    }
    else if (ch != '\0')
    {
        currentColumn++;
    }

    position = readPosition;
    readPosition++;
}

char Lexer::peekChar() const
{
    if (readPosition >= input.length())
    {
        return '\0';
    }
    return input[readPosition];
}

void Lexer::skipWhitespace()
{
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
    {
        readChar();
    }
}

void Lexer::skipComment()
{
    // # 문자부터 줄 끝까지 건너뛰기
    if (ch == '#')
    {
        while (ch != '\n' && ch != '\0')
        {
            readChar();
        }
        // \n은 skipWhitespace()에서 처리되도록 남겨둠
    }
}

bool Lexer::isLetter(char c) const
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_' ||
           isKoreanStart(static_cast<unsigned char>(c));
}

bool Lexer::isDigit(char c) const
{
    return c >= '0' && c <= '9';
}

bool Lexer::isKoreanStart(unsigned char c) const
{
    // UTF-8 한글 범위: 0xEA-0xED (대부분의 한글)
    // 더 넓은 범위: 0xE0-0xEF (모든 3바이트 UTF-8)
    return (c & 0xF0) == 0xE0;
}

bool Lexer::isUTF8ContinuationByte(unsigned char c) const
{
    // UTF-8 연속 바이트: 10xxxxxx
    return (c & 0xC0) == 0x80;
}

std::string Lexer::readUTF8Char()
{
    std::string result;
    unsigned char first = static_cast<unsigned char>(ch);

    if ((first & 0x80) == 0)
    {
        // 1바이트 문자 (ASCII)
        result += ch;
        readChar();
    }
    else if ((first & 0xE0) == 0xC0)
    {
        // 2바이트 문자
        result += ch;
        readChar();
        if (isUTF8ContinuationByte(static_cast<unsigned char>(ch)))
        {
            result += ch;
            readChar();
        }
    }
    else if ((first & 0xF0) == 0xE0)
    {
        // 3바이트 문자 (한글)
        result += ch;
        readChar();
        if (isUTF8ContinuationByte(static_cast<unsigned char>(ch)))
        {
            result += ch;
            readChar();
            if (isUTF8ContinuationByte(static_cast<unsigned char>(ch)))
            {
                result += ch;
                readChar();
            }
        }
    }
    else if ((first & 0xF8) == 0xF0)
    {
        // 4바이트 문자
        result += ch;
        readChar();
        for (int i = 0; i < 3; i++)
        {
            if (isUTF8ContinuationByte(static_cast<unsigned char>(ch)))
            {
                result += ch;
                readChar();
            }
        }
    }

    return result;
}

std::string Lexer::readIdentifier()
{
    std::string identifier;

    // 1. 한글/영문/숫자로 구성된 identifier 읽기
    while (isLetter(ch) || isDigit(ch))
    {
        if (isKoreanStart(static_cast<unsigned char>(ch)))
        {
            // UTF-8 한글 문자 읽기
            std::string utf8Char = readUTF8Char();
            identifier += utf8Char;
        }
        else
        {
            // ASCII 문자
            identifier += ch;
            readChar();
        }
    }

    // 2. 형태소 분석기로 조사 분리
    auto morphemes = morphAnalyzer_.analyze(identifier);

    // 3. 조사가 분리된 경우 처리
    if (morphemes.size() > 1)
    {
        // 조사가 분리됨 → 기본형만 반환하고 position 조정
        std::string base = morphemes[0].base;
        std::string josa = morphemes[1].base;

        // position을 조사 길이만큼 되돌림 (조사는 다음에 다시 읽힘)
        size_t josaBytes = josa.length();
        position -= josaBytes;
        readPosition = position + 1;
        if (position < input.length())
        {
            ch = input[position];
        }

        return base;
    }

    // 4. 조사가 분리되지 않은 경우 - 원본 그대로 반환
    return identifier;
}

std::string Lexer::readNumber()
{
    std::string number;

    // 정수 부분
    while (isDigit(ch))
    {
        number += ch;
        readChar();
    }

    // 실수 부분 (선택적)
    if (ch == '.' && isDigit(peekChar()))
    {
        number += ch;
        readChar();

        while (isDigit(ch))
        {
            number += ch;
            readChar();
        }
    }

    return number;
}

std::string Lexer::readString(char quote)
{
    std::string str;
    readChar();  // 시작 따옴표 건너뛰기

    while (ch != quote && ch != '\0')
    {
        if (ch == '\\')
        {
            // 이스케이프 시퀀스 처리
            readChar();
            switch (ch)
            {
                case 'n':  str += '\n'; break;
                case 't':  str += '\t'; break;
                case 'r':  str += '\r'; break;
                case '\\': str += '\\'; break;
                case '"':  str += '"';  break;
                case '\'': str += '\''; break;
                default:   str += ch;   break;
            }
            readChar();
        }
        else if (isKoreanStart(static_cast<unsigned char>(ch)))
        {
            // UTF-8 한글 문자
            std::string utf8Char = readUTF8Char();
            str += utf8Char;
        }
        else
        {
            str += ch;
            readChar();
        }
    }

    if (ch == quote)
    {
        readChar();  // 종료 따옴표 건너뛰기
    }

    return str;
}

Token Lexer::makeTwoCharToken(char ch1, char ch2, TokenType type1, TokenType type2)
{
    if (peekChar() == ch2)
    {
        char currentCh = ch;
        readChar();
        std::string literal;
        literal += currentCh;
        literal += ch;
        readChar();
        return Token(type2, literal);
    }
    else
    {
        std::string literal;
        literal += ch1;
        readChar();
        return Token(type1, literal);
    }
}

Token Lexer::nextToken()
{
    // 공백과 주석을 모두 건너뛰기
    while (true)
    {
        skipWhitespace();
        if (ch == '#')
        {
            skipComment();
        }
        else
        {
            break;
        }
    }

    // 토큰 시작 위치 저장
    int tokenLine = currentLine;
    int tokenColumn = currentColumn;

    Token token;

    switch (ch)
    {
        case '=':
            token = makeTwoCharToken('=', '=', TokenType::ASSIGN, TokenType::EQ);
            break;

        case '!':
            token = makeTwoCharToken('!', '=', TokenType::NOT, TokenType::NOT_EQ);
            break;

        case '<':
            token = makeTwoCharToken('<', '=', TokenType::LT, TokenType::LE);
            break;

        case '>':
            token = makeTwoCharToken('>', '=', TokenType::GT, TokenType::GE);
            break;

        case '&':
            if (peekChar() == '&')
            {
                readChar();
                readChar();
                token = Token(TokenType::AND, "&&");
            }
            else
            {
                token = Token(TokenType::ILLEGAL, std::string(1, ch));
                readChar();
            }
            break;

        case '|':
            if (peekChar() == '|')
            {
                readChar();
                readChar();
                token = Token(TokenType::OR, "||");
            }
            else
            {
                token = Token(TokenType::ILLEGAL, std::string(1, ch));
                readChar();
            }
            break;

        case '+':
            token = Token(TokenType::PLUS, std::string(1, ch));
            readChar();
            break;

        case '-':
            if (peekChar() == '>')
            {
                char prevCh = ch;
                readChar();
                token = Token(TokenType::ARROW, std::string(1, prevCh) + std::string(1, ch));
                readChar();
            }
            else
            {
                token = Token(TokenType::MINUS, std::string(1, ch));
                readChar();
            }
            break;

        case '*':
            token = Token(TokenType::ASTERISK, std::string(1, ch));
            readChar();
            break;

        case '/':
            // 주석 체크
            if (peekChar() == '/')
            {
                // // 주석 처리 (한 줄 주석)
                readChar(); // 첫 번째 /
                readChar(); // 두 번째 /
                while (ch != '\n' && ch != '\0')
                {
                    readChar();
                }
                // 주석 스킵 후 다음 토큰 반환
                return nextToken();
            }
            else if (peekChar() == '*')
            {
                // /* */ 주석 처리 (멀티라인 주석)
                readChar(); // /
                readChar(); // *

                // */를 찾을 때까지 스킵
                while (ch != '\0')
                {
                    if (ch == '*' && peekChar() == '/')
                    {
                        readChar(); // *
                        readChar(); // /
                        break;
                    }
                    readChar();
                }

                // 주석 스킵 후 다음 토큰 반환
                return nextToken();
            }
            else
            {
                // 나눗셈 연산자
                token = Token(TokenType::SLASH, std::string(1, ch));
                readChar();
            }
            break;

        case '%':
            token = Token(TokenType::PERCENT, std::string(1, ch));
            readChar();
            break;

        case '(':
            token = Token(TokenType::LPAREN, std::string(1, ch));
            readChar();
            break;

        case ')':
            token = Token(TokenType::RPAREN, std::string(1, ch));
            readChar();
            break;

        case '{':
            token = Token(TokenType::LBRACE, std::string(1, ch));
            readChar();
            break;

        case '}':
            token = Token(TokenType::RBRACE, std::string(1, ch));
            readChar();
            break;

        case '[':
            token = Token(TokenType::LBRACKET, std::string(1, ch));
            readChar();
            break;

        case ']':
            token = Token(TokenType::RBRACKET, std::string(1, ch));
            readChar();
            break;

        case ',':
            token = Token(TokenType::COMMA, std::string(1, ch));
            readChar();
            break;

        case ';':
            token = Token(TokenType::SEMICOLON, std::string(1, ch));
            readChar();
            break;

        case ':':
            token = Token(TokenType::COLON, std::string(1, ch));
            readChar();
            break;

        case '.':
            token = Token(TokenType::DOT, std::string(1, ch));
            readChar();
            break;

        case '_':
            token = Token(TokenType::UNDERSCORE, std::string(1, ch));
            readChar();
            break;

        case '"':
        case '\'':
            {
                char quote = ch;
                std::string str = readString(quote);
                token = Token(TokenType::STRING, str);
            }
            break;

        case '\0':
            token = Token(TokenType::EOF_TOKEN, "");
            break;

        default:
            if (isLetter(ch))
            {
                std::string identifier = readIdentifier();

                // "에 대해" 특별 처리 (lookupKeyword 전에 체크)
                if (identifier == "에" && ch == ' ')
                {
                    // 공백 건너뛰기
                    size_t savedPos = position;
                    size_t savedReadPos = readPosition;
                    char savedCh = ch;

                    skipWhitespace();
                    std::string next = readIdentifier();

                    if (next == "대해")
                    {
                        token = Token(TokenType::E_DAEHAE, "에 대해");
                    }
                    else
                    {
                        // 롤백
                        position = savedPos;
                        readPosition = savedReadPos;
                        ch = savedCh;
                        // 조사 "에"로 처리
                        TokenType type = lookupKeyword(identifier);
                        token = Token(type, identifier);
                    }
                }
                else
                {
                    TokenType type = lookupKeyword(identifier);
                    token = Token(type, identifier);
                }
            }
            else if (isDigit(ch))
            {
                std::string number = readNumber();

                // 실수인지 정수인지 판단
                TokenType type = TokenType::INTEGER;
                if (number.find('.') != std::string::npos)
                {
                    type = TokenType::FLOAT;
                }

                token = Token(type, number);
            }
            else
            {
                token = Token(TokenType::ILLEGAL, std::string(1, ch));
                readChar();
            }
            break;
    }

    // 토큰에 위치 정보 설정 (파일명 포함)
    token.location.update(filename, tokenLine, tokenColumn);

    return token;
}

} // namespace lexer
} // namespace kingsejong
