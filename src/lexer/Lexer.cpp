/**
 * @file Lexer.cpp
 * @brief Lexer 클래스 구현
 * @author KingSejong Team
 * @date 2025-11-06
 */

#include "Lexer.h"
#include "JosaRecognizer.h"
#include "common/UTF8Utils.h"
#include <cctype>

namespace kingsejong {
namespace lexer {

Lexer::Lexer(const std::string& input)
    : input(input), filename(""), position(0), readPosition(0), ch('\0'), currentLine(1), currentColumn(0)
{
    // 첫 번째 문자 읽기
    readChar();
}

Lexer::Lexer(const std::string& input, const std::string& filename)
    : input(input), filename(filename), position(0), readPosition(0), ch('\0'), currentLine(1), currentColumn(0)
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
    // readChar 는 읽은 글자의 열을 가리키도록 증가시키므로, 줄 시작은 0 에서 출발해야
    // 첫 글자가 1 이 된다 (1 에서 출발하면 모든 열이 하나씩 밀린다).
    if (ch == '\n')
    {
        currentLine++;
        currentColumn = 0;
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
    return (c & common::UTF8_3BYTE_MASK) == common::UTF8_3BYTE_PATTERN;
}

bool Lexer::isUTF8ContinuationByte(unsigned char c) const
{
    // UTF-8 연속 바이트: 10xxxxxx
    return common::isUTF8ContinuationByte(c);
}

std::string Lexer::readUTF8Char()
{
    std::string result;
    unsigned char first = static_cast<unsigned char>(ch);

    if ((first & common::UTF8_1BYTE_MASK) == common::UTF8_1BYTE_PATTERN)
    {
        // 1바이트 문자 (ASCII)
        result += ch;
        readChar();
    }
    else if ((first & common::UTF8_2BYTE_MASK) == common::UTF8_2BYTE_PATTERN)
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
    else if ((first & common::UTF8_3BYTE_MASK) == common::UTF8_3BYTE_PATTERN)
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
    else if ((first & common::UTF8_4BYTE_MASK) == common::UTF8_4BYTE_PATTERN)
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

    // 1.5 단어 전체가 키워드면 쪼개지 않는다 ("또는" 의 "는", "마지막" 의 "막" 같은 오분리 방지)
    if (lookupKeyword(identifier) != TokenType::IDENTIFIER)
    {
        return identifier;
    }

    // 2. 형태소 분석기로 조사 분리 후보를 얻는다
    std::string base;
    std::string josa;
    auto morphemes = morphAnalyzer_.analyze(identifier);
    if (morphemes.size() > 1)
    {
        base = morphemes[0].base;
        josa = morphemes[1].base;
    }
    else if (identifier.find_first_of("_0123456789") != std::string::npos)
    {
        // 분석기는 숫자·밑줄 뒤의 한글을 쪼개지 않는다 ("값1이" 는 변수 이름일 수 있다).
        // 그러나 "결과1을 변환한다" 처럼 뒤에 메서드가 오면 조사다. 토큰이 있는 조사만 본다.
        static const char* const kParticles[] = {"에서", "으로", "을", "를", "이", "가", "은", "는", "의", "에", "로"};
        for (const char* particle : kParticles)
        {
            std::string p(particle);
            if (identifier.size() > p.size() &&
                identifier.compare(identifier.size() - p.size(), p.size(), p) == 0)
            {
                base = identifier.substr(0, identifier.size() - p.size());
                josa = p;
                break;
            }
        }
    }

    // 3. 조사로 볼지 결정한다. 어간 사전이 없으므로 접미사만으로는 "마을"과 "마+을"을 구분할 수
    //    없다. 그래서 두 가지를 더 본다.
    //    (a) 토큰이 있는 조사만 떼어 낸다. 과/와/도/만 처럼 파서가 모르는 조사를 떼면 식별자가 둘로
    //        갈라져 구문 오류가 된다 ("사과", "짝수만").
    //    (b) 조사 뒤에 메서드(한글·영문 식별자)나 범위의 숫자가 이어질 때만 떼어 낸다. "마을 = 1",
    //        "출력(소수인가)", "제곱차이 = x" 처럼 뒤에 연산자·괄호·줄 끝이 오면 식별자 전체다.
    TokenType josaType = josa.empty() ? TokenType::IDENTIFIER : lookupKeyword(josa);
    bool isRangeMarker = josaType == TokenType::BUTEO || josaType == TokenType::KKAJI ||
                         josaType == TokenType::MIMAN || josaType == TokenType::CHOGA ||
                         josaType == TokenType::IHA || josaType == TokenType::ISANG ||
                         josaType == TokenType::SSIK || josaType == TokenType::BEON;
    // 부터/까지/미만/이상/씩/번 은 조사가 아니라 범위·횟수 표시라서 뒤에 무엇이 와도 떼어 낸다
    // ("지수까지 {", "n번 반복한다")
    if (!josa.empty() && josaType != TokenType::IDENTIFIER && (isRangeMarker || josaMethodMayFollow()))
    {
        // position을 조사 길이만큼 되돌림 (조사는 다음에 다시 읽힘)
        size_t josaBytes = josa.length();
        position -= josaBytes;
        readPosition = position + 1;
        // 열도 함께 되돌린다. 그대로 두면 이 줄의 뒤쪽 토큰 열이 조사 길이만큼 밀린다 ("시작부터 끝까지 {")
        currentColumn -= static_cast<int>(josaBytes);
        if (position < input.length())
        {
            ch = input[position];
        }
        return base;
    }

    // 4. 조사가 분리되지 않은 경우 - 원본 그대로 반환
    return identifier;
}

bool Lexer::josaMethodMayFollow() const
{
    // 현재 position 은 식별자 바로 뒤다. 같은 줄의 공백만 건너뛰고 다음 글자를 본다.
    size_t i = position;
    while (i < input.length() && (input[i] == ' ' || input[i] == '\t'))
    {
        ++i;
    }
    if (i >= input.length())
    {
        return false;
    }
    unsigned char c = static_cast<unsigned char>(input[i]);
    bool hangul = (c & common::UTF8_3BYTE_MASK) == common::UTF8_3BYTE_PATTERN;
    bool letter = std::isalpha(c) || c == '_';
    if (hangul || letter)
    {
        // 다음 단어를 읽어 키워드인지 본다. "최소길이 미만", "리스트를 반복한다" 처럼 범위·반복
        // 키워드가 오면 메서드가 아니므로 앞 단어는 식별자 전체다.
        std::string word;
        size_t j = i;
        while (j < input.length())
        {
            unsigned char d = static_cast<unsigned char>(input[j]);
            if ((d & common::UTF8_3BYTE_MASK) == common::UTF8_3BYTE_PATTERN && j + 2 < input.length())
            {
                word.append(input, j, 3);
                j += 3;
            }
            else if (std::isalpha(d) || std::isdigit(d) || d == '_')
            {
                word.push_back(static_cast<char>(d));
                ++j;
            }
            else
            {
                break;
            }
        }
        return lookupKeyword(word) == TokenType::IDENTIFIER;
    }
    bool digit = std::isdigit(c) != 0;  // 범위의 숫자: i가 1부터
    bool quote = c == '"' || c == '\'';
    // 범위 반복의 음수("i가 -5부터")는 '-' 바로 뒤에 숫자가 온다. "전체길이 - 1" 의 뺄셈은 아니다.
    bool negative = c == '-' && i + 1 < input.length() && std::isdigit(static_cast<unsigned char>(input[i + 1]));
    return digit || quote || negative;
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
                    int savedLine = currentLine;
                    int savedColumn = currentColumn;

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
                        currentLine = savedLine;
                        currentColumn = savedColumn;
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
