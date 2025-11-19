/**
 * @file LspUtils.cpp
 * @brief LSP 유틸리티 함수 구현
 * @author KingSejong Team
 * @date 2025-11-13
 */

#include "lsp/LspUtils.h"
#include "common/UTF8Utils.h"
#include <sstream>
#include <cctype>

namespace kingsejong {
namespace lsp {

std::vector<std::string> splitLines(const std::string& text)
{
    std::vector<std::string> lines;
    std::istringstream iss(text);
    std::string line;

    while (std::getline(iss, line)) {
        lines.push_back(line);
    }

    return lines;
}

size_t characterToByteOffset(const std::string& line, int character)
{
    if (character < 0) {
        return 0;
    }

    size_t byteOffset = 0;
    int utf16Position = 0;

    while (byteOffset < line.length() && utf16Position < character) {
        unsigned char c = line[byteOffset];

        // UTF-8 바이트 수와 UTF-16 code units 계산
        if ((c & common::UTF8_1BYTE_MASK) == common::UTF8_1BYTE_PATTERN) {
            // ASCII: 1 바이트 = 1 UTF-16 code unit
            byteOffset += 1;
            utf16Position += 1;
        }
        else if ((c & common::UTF8_2BYTE_MASK) == common::UTF8_2BYTE_PATTERN) {
            // 2바이트 UTF-8 = 1 UTF-16 code unit
            byteOffset += 2;
            utf16Position += 1;
        }
        else if ((c & common::UTF8_3BYTE_MASK) == common::UTF8_3BYTE_PATTERN) {
            // 3바이트 UTF-8 = 1 UTF-16 code unit (한글 포함)
            byteOffset += 3;
            utf16Position += 1;
        }
        else if ((c & common::UTF8_4BYTE_MASK) == common::UTF8_4BYTE_PATTERN) {
            // 4바이트 UTF-8 = 2 UTF-16 code units (surrogate pair)
            byteOffset += 4;
            utf16Position += 2;
        }
        else {
            // Invalid UTF-8, skip 1 byte
            byteOffset += 1;
        }
    }

    return byteOffset;
}

size_t lspCharacterToByteOffset(const std::string& text, int line, int character)
{
    auto lines = splitLines(text);

    if (line < 0 || line >= static_cast<int>(lines.size())) {
        return std::string::npos;
    }

    return characterToByteOffset(lines[line], character);
}

std::string extractWordAtOffset(const std::string& line, size_t byteOffset)
{
    if (byteOffset >= line.length()) {
        return "";
    }

    // 단어의 시작 찾기 (뒤로 이동)
    size_t start = byteOffset;

    while (start > 0) {
        unsigned char c = line[start - 1];

        // ASCII 식별자 문자
        if (std::isalnum(c) || c == '_') {
            start--;
            continue;
        }

        // UTF-8 multi-byte 문자 (한글 등)
        if (c & common::UTF8_1BYTE_MASK) {
            // UTF-8 시작 바이트 찾기
            size_t temp = start - 1;
            while (temp > 0 && common::isUTF8ContinuationByte(line[temp])) {
                temp--;
            }

            // 유효한 UTF-8 시작인지 확인
            unsigned char firstByte = line[temp];
            if ((firstByte & common::UTF8_2BYTE_MASK) == common::UTF8_2BYTE_PATTERN ||
                (firstByte & common::UTF8_3BYTE_MASK) == common::UTF8_3BYTE_PATTERN ||
                (firstByte & common::UTF8_4BYTE_MASK) == common::UTF8_4BYTE_PATTERN) {
                start = temp;
                continue;
            }
        }

        // 식별자가 아닌 문자
        break;
    }

    // 단어의 끝 찾기 (앞으로 이동)
    size_t end = byteOffset;

    while (end < line.length()) {
        unsigned char c = line[end];

        // ASCII 식별자 문자
        if (std::isalnum(c) || c == '_') {
            end++;
            continue;
        }

        // UTF-8 multi-byte 문자
        if ((c & common::UTF8_1BYTE_MASK) != common::UTF8_1BYTE_PATTERN) {
            // UTF-8 문자 전체 크기 계산
            size_t charSize = common::getUTF8CharLength(c);
            end += charSize;
            continue;
        }

        // 식별자가 아닌 문자
        break;
    }

    if (start == end) {
        return "";
    }

    return line.substr(start, end - start);
}

} // namespace lsp
} // namespace kingsejong
