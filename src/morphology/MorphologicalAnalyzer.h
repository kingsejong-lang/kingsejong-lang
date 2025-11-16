#pragma once

/**
 * @file MorphologicalAnalyzer.h
 * @brief 한글 형태소 분석기
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "Dictionary.h"
#include <string>
#include <vector>

namespace kingsejong {
namespace morphology {

/**
 * @struct Morpheme
 * @brief 형태소 정보
 */
struct Morpheme
{
    std::string surface;   ///< 표층형 (원본 단어)
    std::string base;      ///< 기본형 (조사 분리 후)
    std::string josa;      ///< 조사 (분리된 경우)

    Morpheme(const std::string& surf, const std::string& b, const std::string& j = "")
        : surface(surf), base(b), josa(j)
    {
    }
};

/**
 * @class MorphologicalAnalyzer
 * @brief 한글 형태소 분석기
 *
 * 사전 기반으로 조사를 분리하고 형태소를 분석합니다.
 * Lexer의 복잡한 휴리스틱 코드를 대체합니다.
 *
 * 처리 규칙:
 * 1. Builtin 함수/변수는 조사 분리하지 않음
 * 2. 사전에 등록된 명사는 조사 분리하지 않음
 * 3. 언더스코어/숫자 뒤 한글은 조사 분리하지 않음
 * 4. 2글자 명사 (나이, 거리 등)는 조사 분리하지 않음
 */
class MorphologicalAnalyzer
{
private:
    Dictionary dictionary_;  ///< 형태소 사전 (RAII로 자동 관리)

public:
    /**
     * @brief 생성자 - 기본 사전 초기화
     */
    MorphologicalAnalyzer() = default;

    /**
     * @brief 단어를 형태소 분석
     * @param word 분석할 단어
     * @return 형태소 목록
     *
     * @example
     * ```cpp
     * MorphologicalAnalyzer analyzer;
     * auto morphemes = analyzer.analyze("배열을");
     * // morphemes[0] = {surface: "배열을", base: "배열", josa: ""}
     * // morphemes[1] = {surface: "배열을", base: "을", josa: ""}
     * ```
     */
    std::vector<Morpheme> analyze(const std::string& word) const;

private:
    /**
     * @brief 2글자 조사 분리 시도
     * @param word 원본 단어
     * @return 분리 성공 시 형태소 목록, 실패 시 빈 벡터
     */
    std::vector<Morpheme> tryTwoCharJosa(const std::string& word) const;

    /**
     * @brief 1글자 조사 분리 시도
     * @param word 원본 단어
     * @return 분리 성공 시 형태소 목록, 실패 시 빈 벡터
     */
    std::vector<Morpheme> tryOneCharJosa(const std::string& word) const;

    /**
     * @brief 언더스코어/숫자 뒤에 한글이 있는지 확인
     * @param word 확인할 단어
     * @return 언더스코어/숫자 뒤 한글이 있으면 true
     */
    bool hasKoreanAfterUnderscoreOrDigit(const std::string& word) const;

    /**
     * @brief ASCII 식별자 + 조사 분리 시도
     * @param word 원본 단어 (예: "i가", "x를")
     * @return 분리 성공 시 형태소 목록, 실패 시 빈 벡터
     *
     * ASCII 문자로 시작하는 식별자 뒤에 한글 조사가 붙은 경우 분리합니다.
     * 예: "i가" → ["i", "가"], "count를" → ["count", "를"]
     */
    std::vector<Morpheme> tryASCIIWithJosa(const std::string& word) const;
};

} // namespace morphology
} // namespace kingsejong
