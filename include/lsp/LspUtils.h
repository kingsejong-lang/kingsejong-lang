#pragma once

/**
 * @file LspUtils.h
 * @brief LSP 유틸리티 함수
 * @author KingSejong Team
 * @date 2025-11-13
 *
 * LSP 프로토콜 관련 유틸리티 함수들
 */

#include <string>
#include <vector>

namespace kingsejong {
namespace lsp {

/**
 * @brief LSP character position을 바이트 오프셋으로 변환
 * @param text UTF-8 텍스트
 * @param line 줄 번호 (0-based)
 * @param character LSP character position (UTF-16 code units, 0-based)
 * @return 바이트 오프셋 (해당 줄 시작부터)
 *
 * LSP는 character를 UTF-16 code units으로 세지만,
 * C++ std::string은 바이트 단위입니다.
 *
 * 예: "정수 x"
 * - UTF-16: 정(1) 수(1) 공백(1) x(1) = position 3이 'x'
 * - UTF-8 바이트: 정(3) 수(3) 공백(1) x(1) = offset 7이 'x'
 */
size_t lspCharacterToByteOffset(const std::string& text, int line, int character);

/**
 * @brief 텍스트를 줄 단위로 분리
 * @param text UTF-8 텍스트
 * @return 줄 목록
 */
std::vector<std::string> splitLines(const std::string& text);

/**
 * @brief 줄에서 character position의 바이트 오프셋 계산
 * @param line UTF-8 텍스트 줄
 * @param character LSP character position (UTF-16 code units, 0-based)
 * @return 바이트 오프셋
 *
 * UTF-16 code units을 세어 바이트 오프셋으로 변환합니다.
 */
size_t characterToByteOffset(const std::string& line, int character);

/**
 * @brief 바이트 오프셋에서 단어 추출
 * @param line UTF-8 텍스트 줄
 * @param byteOffset 바이트 오프셋
 * @return 단어 (식별자)
 *
 * byteOffset 위치의 단어 (식별자)를 추출합니다.
 * 한글, 영문, 숫자, 언더스코어를 포함합니다.
 */
std::string extractWordAtOffset(const std::string& line, size_t byteOffset);

} // namespace lsp
} // namespace kingsejong
