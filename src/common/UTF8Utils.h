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

} // namespace common
} // namespace kingsejong
