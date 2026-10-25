#pragma once

/**
 * @file UTF8Utils.h
 * @brief UTF-8 인코딩 관련 상수 및 유틸리티 함수
 * @author KingSejong Team
 * @date 2025-11-19
 *
 * UTF-8 바이트 패턴 검증 및 문자 길이 계산을 위한 공통 유틸리티입니다.
 * 매직 넘버를 상수화하여 코드 가독성과 유지보수성을 향상시킵니다.
 */

#include <algorithm>
#include <cstddef>
#include <string>

namespace kingsejong {
namespace common {

// ============================================================================
// UTF-8 바이트 패턴 상수
// ============================================================================

/// @name UTF-8 바이트 마스크
/// @{
constexpr unsigned char UTF8_1BYTE_MASK = 0x80;      ///< 1바이트 문자 마스크 (0xxxxxxx)
constexpr unsigned char UTF8_2BYTE_MASK = 0xE0;      ///< 2바이트 문자 마스크 (110xxxxx)
constexpr unsigned char UTF8_3BYTE_MASK = 0xF0;      ///< 3바이트 문자 마스크 (1110xxxx)
constexpr unsigned char UTF8_4BYTE_MASK = 0xF8;      ///< 4바이트 문자 마스크 (11110xxx)
constexpr unsigned char UTF8_CONTINUATION_MASK = 0xC0;  ///< 연속 바이트 마스크 (10xxxxxx)
/// @}

/// @name UTF-8 바이트 패턴
/// @{
constexpr unsigned char UTF8_1BYTE_PATTERN = 0x00;   ///< 1바이트 문자 패턴 (0xxxxxxx)
constexpr unsigned char UTF8_2BYTE_PATTERN = 0xC0;   ///< 2바이트 문자 패턴 (110xxxxx)
constexpr unsigned char UTF8_3BYTE_PATTERN = 0xE0;   ///< 3바이트 문자 패턴 (1110xxxx)
constexpr unsigned char UTF8_4BYTE_PATTERN = 0xF0;   ///< 4바이트 문자 패턴 (11110xxx)
constexpr unsigned char UTF8_CONTINUATION_PATTERN = 0x80;  ///< 연속 바이트 패턴 (10xxxxxx)
/// @}

/// @name 한글 UTF-8 범위
/// @{
constexpr unsigned char KOREAN_UTF8_START = 0xEA;    ///< 한글 UTF-8 시작 (가)
constexpr unsigned char KOREAN_UTF8_END = 0xED;      ///< 한글 UTF-8 끝 (힣)
/// @}

// ============================================================================
// UTF-8 유틸리티 함수
// ============================================================================

/**
 * @brief UTF-8 문자의 바이트 길이를 반환
 * @param firstByte 문자의 첫 번째 바이트
 * @return 문자의 바이트 길이 (1-4), 잘못된 경우 1 반환
 */
inline size_t getUTF8CharLength(unsigned char firstByte)
{
    if ((firstByte & UTF8_1BYTE_MASK) == UTF8_1BYTE_PATTERN)
    {
        return 1;  // ASCII (0xxxxxxx)
    }
    if ((firstByte & UTF8_2BYTE_MASK) == UTF8_2BYTE_PATTERN)
    {
        return 2;  // 2바이트 문자 (110xxxxx)
    }
    if ((firstByte & UTF8_3BYTE_MASK) == UTF8_3BYTE_PATTERN)
    {
        return 3;  // 3바이트 문자 (1110xxxx)
    }
    if ((firstByte & UTF8_4BYTE_MASK) == UTF8_4BYTE_PATTERN)
    {
        return 4;  // 4바이트 문자 (11110xxx)
    }
    return 1;  // 잘못된 바이트는 1로 처리
}

/**
 * @brief 바이트가 UTF-8 문자의 시작인지 확인
 * @param byte 검사할 바이트
 * @return 시작 바이트이면 true
 */
inline bool isUTF8StartByte(unsigned char byte)
{
    // 연속 바이트가 아니면 시작 바이트
    return (byte & UTF8_CONTINUATION_MASK) != UTF8_CONTINUATION_PATTERN;
}

/**
 * @brief 바이트가 UTF-8 연속 바이트인지 확인
 * @param byte 검사할 바이트
 * @return 연속 바이트이면 true
 */
inline bool isUTF8ContinuationByte(unsigned char byte)
{
    return (byte & UTF8_CONTINUATION_MASK) == UTF8_CONTINUATION_PATTERN;
}

/**
 * @brief 바이트가 한글 UTF-8 시작인지 확인
 * @param byte 검사할 바이트
 * @return 한글 시작 바이트이면 true
 */
inline bool isKoreanUTF8Start(unsigned char byte)
{
    return byte >= KOREAN_UTF8_START && byte <= KOREAN_UTF8_END;
}

/**
 * @brief UTF-8 문자열의 문자 개수를 반환 (바이트 수가 아닌 글자 수)
 * @param str UTF-8 문자열
 * @return 문자 개수
 */
inline size_t countUTF8Characters(const std::string& str)
{
    size_t count = 0;
    size_t i = 0;

    while (i < str.length())
    {
        size_t charLen = getUTF8CharLength(static_cast<unsigned char>(str[i]));
        i += charLen;
        ++count;
    }

    return count;
}

/**
 * @brief UTF-8 문자열에서 n번째 문자의 바이트 오프셋을 반환
 * @param str UTF-8 문자열
 * @param charIndex 문자 인덱스 (0-based)
 * @return 바이트 오프셋, 범위를 벗어나면 string::npos 반환
 */
inline size_t getUTF8CharOffset(const std::string& str, size_t charIndex)
{
    size_t offset = 0;
    size_t currentChar = 0;

    while (offset < str.length() && currentChar < charIndex)
    {
        size_t charLen = getUTF8CharLength(static_cast<unsigned char>(str[offset]));
        offset += charLen;
        ++currentChar;
    }

    if (currentChar != charIndex || offset >= str.length())
    {
        return std::string::npos;
    }

    return offset;
}

/**
 * @brief UTF-8 문자열에서 n번째 문자를 추출
 * @param str UTF-8 문자열
 * @param charIndex 문자 인덱스 (0-based)
 * @return 추출된 문자, 범위를 벗어나면 빈 문자열
 */
inline std::string extractUTF8Char(const std::string& str, size_t charIndex)
{
    size_t offset = getUTF8CharOffset(str, charIndex);

    if (offset == std::string::npos)
    {
        return "";
    }

    size_t charLen = getUTF8CharLength(static_cast<unsigned char>(str[offset]));
    return str.substr(offset, charLen);
}

/**
 * @brief 코드 포인트가 터미널에서 두 칸을 차지하는지 확인 (East Asian Wide/Fullwidth)
 * @param cp 유니코드 코드 포인트
 * @return 두 칸 폭이면 true
 *
 * 한글 음절·자모, CJK 문자, 전각 기호, 주요 이모지 범위만 다룬다. 결합 문자처럼
 * 폭이 0인 문자는 따로 구분하지 않고 한 칸으로 센다.
 */
inline bool isWideCodePoint(char32_t cp)
{
    return (cp >= 0x1100 && cp <= 0x115F) ||    // 한글 자모 (초성)
           (cp >= 0x2E80 && cp <= 0x303E) ||    // CJK 부수, 기호와 구두점
           (cp >= 0x3041 && cp <= 0x33FF) ||    // 가나, 호환 자모, CJK 호환
           (cp >= 0x3400 && cp <= 0x4DBF) ||    // CJK 확장 A
           (cp >= 0x4E00 && cp <= 0x9FFF) ||    // CJK 통합 한자
           (cp >= 0xA000 && cp <= 0xA4CF) ||    // 이 문자
           (cp >= 0xA960 && cp <= 0xA97F) ||    // 한글 자모 확장 A
           (cp >= 0xAC00 && cp <= 0xD7A3) ||    // 한글 음절
           (cp >= 0xF900 && cp <= 0xFAFF) ||    // CJK 호환 한자
           (cp >= 0xFE30 && cp <= 0xFE4F) ||    // CJK 호환 형태
           (cp >= 0xFF00 && cp <= 0xFF60) ||    // 전각 ASCII
           (cp >= 0xFFE0 && cp <= 0xFFE6) ||    // 전각 기호
           (cp >= 0x1F300 && cp <= 0x1F64F) ||  // 그림 기호, 이모티콘
           (cp >= 0x1F900 && cp <= 0x1F9FF) ||  // 보충 그림 기호
           (cp >= 0x20000 && cp <= 0x3FFFD);    // CJK 확장 B 이후
}

/**
 * @brief 바이트 열(column) 앞의 텍스트와 같은 화면 폭을 차지하는 공백 문자열을 만든다
 * @param line 소스 한 줄 (UTF-8)
 * @param byteColumn 1부터 시작하는 바이트 열
 * @return 캐럿(^) 앞에 둘 들여쓰기. 탭은 그대로, 넓은 문자는 공백 두 칸, 나머지는 한 칸
 *
 * 렉서가 기록하는 열은 바이트 오프셋이라 한글 뒤에서는 실제 글자 위치보다 크다.
 * 이 함수로 바이트 열을 화면 위치로 바꿔야 오류 표시 캐럿이 해당 글자 아래에 온다.
 *
 * @code
 * caretPadding("출력(값들[5])", 8);  // "     " (출력 4칸 + "(" 1칸)
 * @endcode
 */
inline std::string caretPadding(const std::string& line, int byteColumn)
{
    std::string padding;
    size_t end = byteColumn > 1 ? std::min(static_cast<size_t>(byteColumn - 1), line.size()) : 0;
    size_t i = 0;
    while (i < end)
    {
        auto first = static_cast<unsigned char>(line[i]);
        size_t len = getUTF8CharLength(first);
        if (first == '\t')
        {
            padding += '\t';
            i += 1;
            continue;
        }
        char32_t cp = 0;
        if (len == 1 || i + len > line.size())
        {
            cp = first;
            len = 1;
        }
        else
        {
            // 첫 바이트의 데이터 비트 뒤에 연속 바이트의 하위 6비트를 차례로 붙인다
            cp = first & (0x7F >> len);
            for (size_t k = 1; k < len; ++k)
            {
                cp = (cp << 6) | (static_cast<unsigned char>(line[i + k]) & 0x3F);
            }
        }
        padding.append(isWideCodePoint(cp) ? 2 : 1, ' ');
        i += len;
    }
    return padding;
}

/**
 * @brief 바이트 열을 글자(코드 포인트) 단위 열로 바꾼다
 * @param line 소스 한 줄 (UTF-8)
 * @param byteColumn 1부터 시작하는 바이트 열
 * @return 1부터 시작하는 글자 열. 바이트 열이 줄 끝을 넘으면 넘는 바이트를 한 글자씩 센다
 *
 * 렉서와 LSP 는 바이트 열을 쓰지만, 사람이 읽는 `파일:줄:열` 은 편집기처럼 글자 단위로 보여 준다.
 *
 * @code
 * byteColumnToCharColumn("출력(값들[5])", 8);  // 4 ("출력(" 세 글자 다음)
 * @endcode
 */
inline int byteColumnToCharColumn(const std::string& line, int byteColumn)
{
    if (byteColumn <= 1)
    {
        return byteColumn;
    }
    size_t end = static_cast<size_t>(byteColumn - 1);
    size_t inLine = std::min(end, line.size());
    int chars = 0;
    for (size_t i = 0; i < inLine; ++i)
    {
        if (!isUTF8ContinuationByte(static_cast<unsigned char>(line[i])))
        {
            ++chars;
        }
    }
    return chars + static_cast<int>(end - inLine) + 1;
}

/**
 * @brief 소스 텍스트에서 1부터 세는 line 번째 줄을 꺼낸다 (줄 끝 \r 제외). 없으면 빈 문자열
 */
inline std::string sourceLine(const std::string& source, int line)
{
    size_t start = 0;
    for (int i = 1; i < line; ++i)
    {
        start = source.find('\n', start);
        if (start == std::string::npos)
        {
            return "";
        }
        ++start;
    }
    size_t end = source.find('\n', start);
    std::string text = source.substr(start, end == std::string::npos ? std::string::npos : end - start);
    if (!text.empty() && text.back() == '\r')
    {
        text.pop_back();
    }
    return text;
}

/**
 * @brief 소스 텍스트 기준으로 바이트 열을 글자 열로 바꾼다
 */
inline int charColumnInSource(const std::string& source, int line, int byteColumn)
{
    return byteColumnToCharColumn(sourceLine(source, line), byteColumn);
}

} // namespace common
} // namespace kingsejong
