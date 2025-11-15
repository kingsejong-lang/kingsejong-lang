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
    : input(input), position(0), readPosition(0), ch('\0'), currentLine(1), currentColumn(1)
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

    // identifier 전체가 조사인 경우 분리하지 않음
    // 예: "으로", "에서" 등이 단독으로 입력된 경우
    if (isJosa(identifier))
    {
        return identifier;
    }

    // 조사/키워드 접미사 분리 처리
    // 한글은 3바이트이므로 마지막 3바이트 또는 6바이트 확인

    // 먼저 2글자 조사/키워드 확인 (6바이트) - "에서", "으로", "하고" 등
    if (identifier.length() >= 9)  // 최소 3글자 (남은 부분 1글자 + 접미사 2글자)
    {
        std::string lastTwoChars = identifier.substr(identifier.length() - 6);
        TokenType suffixType = lookupKeyword(lastTwoChars);

        // 조사이거나 분리 가능한 키워드인 경우
        if (isJosa(lastTwoChars) ||
            suffixType == TokenType::HAGO ||
            suffixType == TokenType::HARA ||
            suffixType == TokenType::KKAJI ||
            suffixType == TokenType::BUTEO ||
            suffixType == TokenType::BANBOKK)
        {
            // 접미사를 분리
            identifier = identifier.substr(0, identifier.length() - 6);
            position -= 6;
            readPosition = position + 1;
            if (position < input.length())
            {
                ch = input[position];
            }
            return identifier;
        }
    }

    // 1글자 조사/키워드 확인 (3바이트)
    if (identifier.length() >= 4)  // 최소 1글자 + 접미사
    {
        std::string lastChar = identifier.substr(identifier.length() - 3);
        TokenType suffixType = lookupKeyword(lastChar);

        if (isJosa(lastChar) || suffixType == TokenType::BEON)
        {
            std::string base = identifier.substr(0, identifier.length() - 3);

            // 조사 분리 전 base의 마지막 문자 확인
            // 조사 바로 앞에 언더스코어나 숫자가 있으면 조사가 아님
            // 예: "원의_넓이"에서 "_넓"이 있으면 "이"는 조사가 아님
            // base에서 역순으로 스캔하여 언더스코어나 숫자를 찾음
            if (base.length() >= 4)  // 최소한 "_X이" 형태
            {
                // base를 역순으로 스캔
                size_t pos = base.length();
                while (pos > 0)
                {
                    unsigned char byte = static_cast<unsigned char>(base[pos - 1]);
                    if (byte < 0x80)  // ASCII 문자 발견
                    {
                        // 언더스코어나 숫자 뒤에 한글이 바로 오는 경우
                        // 예: "_넓이"에서 "_" 뒤의 "넓이"는 조사 분리하지 않음
                        if (byte == '_' || (byte >= '0' && byte <= '9'))
                        {
                            return identifier;  // 조사 분리하지 않음
                        }
                        break;  // ASCII 문자 발견 시 중단
                    }
                    pos--;
                }
            }

            // 특수 케이스: "나이" 같은 일반 명사는 분리하지 않음
            // 2글자 한글 단어 중 마지막 글자가 "이"인 경우는 대부분 명사
            if (base.length() == 3 && lastChar == "이" && suffixType != TokenType::BEON)
            {
                // "나이", "거리", "자리" 등은 분리하지 않음
                // 추후 사전 기반 형태소 분석으로 개선 필요
                return identifier;
            }

            // Builtin 함수/변수명 보호: "경로", "절대경로" 등
            if (lastChar == "로")
            {
                // "경로", "절대경로" 등 builtin 관련 명사
                if (identifier == "경로" || identifier == "절대경로")
                {
                    return identifier;
                }
            }

            // Builtin 함수명 보호: "디렉토리인가", "파일인가", "존재하는가" 등
            if (lastChar == "가")
            {
                // "~인가" 패턴 (boolean 질문형 함수)
                // "인" = 3바이트
                if (base.length() >= 3 && base.substr(base.length() - 3) == "인")
                {
                    return identifier;
                }
                // "~존재하는가" 패턴
                // "존재하는" = 4글자 × 3바이트 = 12바이트
                if (base.length() >= 12 && base.substr(base.length() - 12) == "존재하는")
                {
                    return identifier;
                }
            }

            // 조사 또는 키워드를 분리
            identifier = base;
            position -= 3;
            readPosition = position + 1;
            if (position < input.length())
            {
                ch = input[position];
            }
        }
    }

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
            token = Token(TokenType::SLASH, std::string(1, ch));
            readChar();
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

    // 토큰에 위치 정보 설정
    token.location.update(tokenLine, tokenColumn);

    return token;
}

} // namespace lexer
} // namespace kingsejong
