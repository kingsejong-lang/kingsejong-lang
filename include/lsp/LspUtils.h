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

/**
 * @brief Lexer 의 1-based 바이트 열을 LSP 의 0-based UTF-16 단위로 바꾼다
 *
 * 한글 한 글자는 UTF-8 3바이트, UTF-16 1단위이고 이모지는 4바이트, 2단위다.
 */
int byteColumnToUtf16(const std::string& line, int byteColumn);

/// UTF-8 문자열의 UTF-16 단위 길이 (LSP range 의 end 계산용)
int utf16Length(const std::string& text);

/// byteOffset 이 가리키는 식별자의 시작 바이트 오프셋 (식별자가 아니면 byteOffset 그대로)
size_t wordStartOffset(const std::string& line, size_t byteOffset);

/**
 * @brief file:// URI 를 로컬 파일 경로로 바꾼다
 * @return 파일 경로. file 스킴이 아니면 빈 문자열
 *
 * 퍼센트 인코딩(한글 경로는 %EC%A0%95 처럼 온다)을 풀고, Windows 드라이브 경로
 * (file:///c%3A/dir/a.ksj)는 앞의 '/' 를 떼어 c:/dir/a.ksj 로 돌려준다.
 */
std::string uriToFilePath(const std::string& uri);

} // namespace lsp
} // namespace kingsejong
