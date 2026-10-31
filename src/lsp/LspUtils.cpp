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

namespace {

size_t utf8CharLength(unsigned char c)
{
    return (c < 0x80) ? 1 : (c >> 5) == 0x6 ? 2 : (c >> 4) == 0xE ? 3 : (c >> 3) == 0x1E ? 4 : 1;
}

} // namespace

int byteColumnToUtf16(const std::string& line, int byteColumn)
{
    size_t bytes = byteColumn > 0 ? static_cast<size_t>(byteColumn - 1) : 0;
    if (bytes > line.size()) bytes = line.size();
    int units = 0;
    for (size_t i = 0; i < bytes;) {
        size_t len = utf8CharLength(static_cast<unsigned char>(line[i]));
        units += (len == 4) ? 2 : 1;
        i += len;
    }
    return units;
}

int utf16Length(const std::string& text)
{
    int units = 0;
    for (size_t i = 0; i < text.size();) {
        size_t len = utf8CharLength(static_cast<unsigned char>(text[i]));
        units += (len == 4) ? 2 : 1;
        i += len;
    }
    return units;
}

size_t wordStartOffset(const std::string& line, size_t byteOffset)
{
    if (byteOffset > line.length()) {
        return byteOffset;
    }
    size_t start = byteOffset;
    while (start > 0) {
        unsigned char c = line[start - 1];
        if (std::isalnum(c) || c == '_') {
            start--;
            continue;
        }
        if (c & common::UTF8_1BYTE_MASK) {
            size_t temp = start - 1;
            while (temp > 0 && common::isUTF8ContinuationByte(line[temp])) {
                temp--;
            }
            unsigned char firstByte = line[temp];
            if ((firstByte & common::UTF8_2BYTE_MASK) == common::UTF8_2BYTE_PATTERN ||
                (firstByte & common::UTF8_3BYTE_MASK) == common::UTF8_3BYTE_PATTERN ||
                (firstByte & common::UTF8_4BYTE_MASK) == common::UTF8_4BYTE_PATTERN) {
                start = temp;
                continue;
            }
        }
        break;
    }
    return start;
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

std::string uriToFilePath(const std::string& uri)
{
    const std::string scheme = "file://";
    if (uri.compare(0, scheme.size(), scheme) != 0) {
        return "";
    }

    // authority(호스트)는 비어 있다고 보고 경로 부분만 쓴다
    std::string encoded = uri.substr(scheme.size());
    std::string path;
    path.reserve(encoded.size());
    for (size_t i = 0; i < encoded.size(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.size() &&
            std::isxdigit(static_cast<unsigned char>(encoded[i + 1])) &&
            std::isxdigit(static_cast<unsigned char>(encoded[i + 2]))) {
            path.push_back(static_cast<char>(std::stoi(encoded.substr(i + 1, 2), nullptr, 16)));
            i += 2;
        } else {
            path.push_back(encoded[i]);
        }
    }

    // Windows: /c:/dir -> c:/dir
    if (path.size() >= 3 && path[0] == '/' &&
        std::isalpha(static_cast<unsigned char>(path[1])) && path[2] == ':') {
        path.erase(0, 1);
    }
    return path;
}

} // namespace lsp
} // namespace kingsejong
