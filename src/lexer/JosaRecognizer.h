#pragma once

/**
 * @file JosaRecognizer.h
 * @brief 한글 조사 인식 및 선택 기능
 * @author KingSejong Team
 * @date 2025-11-06
 */

#include <string>

namespace kingsejong {
namespace lexer {

/**
 * @class JosaRecognizer
 * @brief 한글 단어의 받침을 분석하여 적절한 조사를 선택하는 클래스
 *
 * 한국어의 조사는 앞 명사의 받침 유무에 따라 형태가 달라집니다.
 * 이 클래스는 명사의 마지막 글자를 분석하여 받침 유무를 판단하고,
 * 적절한 조사를 자동으로 선택합니다.
 */
class JosaRecognizer
{
public:
    /**
     * @enum JosaType
     * @brief 조사의 종류
     */
    enum class JosaType
    {
        EUL_REUL,   ///< 을/를 - 목적격 조사
        I_GA,       ///< 이/가 - 주격 조사
        EUN_NEUN,   ///< 은/는 - 보조사
        UI,         ///< 의 - 소유격 조사 (받침 무관)
        RO_EURO,    ///< 로/으로 - 방향/수단 조사
        ESO,        ///< 에서 - 출처 조사 (받침 무관)
        E           ///< 에 - 위치/대상 조사 (받침 무관)
    };

    /**
     * @brief 문자열이 한글 조사인지 확인
     * @param str 확인할 문자열
     * @return 조사이면 true, 아니면 false
     */
    static bool isJosa(const std::string& str);

    /**
     * @brief 조사 문자열을 JosaType으로 변환
     * @param str 조사 문자열 ("을", "를", "이", "가" 등)
     * @return 해당하는 JosaType
     * @throws std::invalid_argument 유효하지 않은 조사인 경우
     */
    static JosaType getType(const std::string& str);

    /**
     * @brief 한글 단어의 마지막 글자에 받침이 있는지 확인
     * @param word 확인할 한글 단어 (UTF-8 인코딩)
     * @return 받침이 있으면 true, 없으면 false
     *
     * @details
     * 한글의 유니코드 구조를 이용하여 받침을 판단합니다.
     * - 한글 완성형: 0xAC00 ~ 0xD7A3
     * - 종성(받침) 계산: (유니코드 - 0xAC00) % 28
     * - 0이면 받침 없음, 0이 아니면 받침 있음
     *
     * 예외:
     * - "ㄹ" 받침은 "로/으로" 선택 시 특별 처리
     */
    static bool hasFinalConsonant(const std::string& word);

    /**
     * @brief 명사에 맞는 조사를 선택
     * @param noun 명사 (한글 단어)
     * @param type 조사 타입
     * @return 선택된 조사 문자열
     *
     * @details
     * 받침 유무에 따라 적절한 조사를 선택합니다:
     * - EUL_REUL: 받침 있음 → "을", 받침 없음 → "를"
     * - I_GA: 받침 있음 → "이", 받침 없음 → "가"
     * - EUN_NEUN: 받침 있음 → "은", 받침 없음 → "는"
     * - UI: "의" (받침 무관)
     * - RO_EURO: 받침 없음 또는 ㄹ받침 → "로", 그 외 → "으로"
     * - ESO: "에서" (받침 무관)
     * - E: "에" (받침 무관)
     *
     * @example
     * \code
     * JosaRecognizer::select("사과", JosaType::EUL_REUL);  // "를"
     * JosaRecognizer::select("책", JosaType::EUL_REUL);    // "을"
     * JosaRecognizer::select("물", JosaType::RO_EURO);     // "로" (ㄹ받침)
     * JosaRecognizer::select("집", JosaType::RO_EURO);     // "으로"
     * \endcode
     */
    static std::string select(const std::string& noun, JosaType type);

    /**
     * @brief JosaType을 문자열로 변환
     * @param type 조사 타입
     * @return 조사 타입의 문자열 표현
     */
    static std::string josaTypeToString(JosaType type);

private:
    /**
     * @brief 한글 글자의 종성(받침) 인덱스를 반환
     * @param ch 한글 UTF-8 문자열 (한 글자)
     * @return 종성 인덱스 (0: 받침 없음, 1-27: 받침 있음, -1: 한글이 아님)
     *
     * @details
     * 한글 종성 목록 (28개):
     * 0: (없음)
     * 1: ㄱ, 2: ㄲ, 3: ㄳ, 4: ㄴ, 5: ㄵ, 6: ㄶ, 7: ㄷ, 8: ㄹ,
     * 9: ㄺ, 10: ㄻ, 11: ㄼ, 12: ㄽ, 13: ㄾ, 14: ㄿ, 15: ㅀ, 16: ㅁ,
     * 17: ㅂ, 18: ㅄ, 19: ㅅ, 20: ㅆ, 21: ㅇ, 22: ㅈ, 23: ㅊ, 24: ㅋ,
     * 25: ㅌ, 26: ㅍ, 27: ㅎ
     */
    static int getFinalConsonantIndex(const std::string& ch);

    /**
     * @brief ㄹ 받침인지 확인
     * @param word 확인할 한글 단어
     * @return ㄹ 받침이면 true, 아니면 false
     */
    static bool hasRieulFinal(const std::string& word);
};

} // namespace lexer
} // namespace kingsejong
