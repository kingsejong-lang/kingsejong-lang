/**
 * @file MorphologicalAnalyzer.cpp
 * @brief 한글 형태소 분석기 구현
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "MorphologicalAnalyzer.h"

namespace kingsejong {
namespace morphology {

std::vector<Morpheme> MorphologicalAnalyzer::analyze(const std::string& word) const
{
    // 빈 문자열 처리
    if (word.empty())
    {
        return {Morpheme(word, word, "")};
    }

    // 1. 단어 전체가 조사인 경우
    if (dictionary_.isJosa(word))
    {
        return {Morpheme(word, word, "")};
    }

    // 2. Builtin 함수/변수는 조사 분리하지 않음
    if (dictionary_.isBuiltinFunc(word) || dictionary_.isBuiltinVar(word))
    {
        return {Morpheme(word, word, "")};
    }

    // 3. 사전에 등록된 명사는 조사 분리하지 않음
    if (dictionary_.isNoun(word))
    {
        return {Morpheme(word, word, "")};
    }

    // 4. 언더스코어/숫자 뒤 한글은 조사 분리하지 않음
    if (hasKoreanAfterUnderscoreOrDigit(word))
    {
        return {Morpheme(word, word, "")};
    }

    // 5. 2글자 조사 분리 시도 (6바이트)
    auto result = tryTwoCharJosa(word);
    if (!result.empty())
    {
        return result;
    }

    // 6. 1글자 조사 분리 시도 (3바이트)
    result = tryOneCharJosa(word);
    if (!result.empty())
    {
        return result;
    }

    // 7. ASCII 식별자 + 조사 분리 시도 (예: "i가", "x를")
    result = tryASCIIWithJosa(word);
    if (!result.empty())
    {
        return result;
    }

    // 8. 조사 분리 실패 - 원본 그대로 반환
    return {Morpheme(word, word, "")};
}

std::vector<Morpheme> MorphologicalAnalyzer::tryTwoCharJosa(const std::string& word) const
{
    // 최소 3글자 필요 (기본형 1글자 + 조사 2글자)
    // UTF-8: 1글자 = 3바이트 → 3글자 = 9바이트
    if (word.length() < 9)
    {
        return {};
    }

    // 마지막 6바이트 (2글자) 추출
    std::string lastTwoChars = word.substr(word.length() - 6);

    // 조사인지 확인
    if (!dictionary_.isJosa(lastTwoChars))
    {
        return {};
    }

    // 기본형 추출
    std::string base = word.substr(0, word.length() - 6);

    // 분리 성공
    return {
        Morpheme(word, base, ""),
        Morpheme(word, lastTwoChars, "")
    };
}

std::vector<Morpheme> MorphologicalAnalyzer::tryOneCharJosa(const std::string& word) const
{
    // 최소 2글자 필요 (기본형 1글자 + 조사 1글자)
    // UTF-8: 2글자 = 6바이트
    if (word.length() < 6)
    {
        return {};
    }

    // 마지막 3바이트 (1글자) 추출
    std::string lastChar = word.substr(word.length() - 3);

    // 조사인지 확인
    if (!dictionary_.isJosa(lastChar))
    {
        return {};
    }

    // 기본형 추출
    std::string base = word.substr(0, word.length() - 3);

    // 특수 케이스: 2글자 명사 + "이" 조사는 분리하지 않음
    // 예: "나이", "거리", "자리"
    if (base.length() == 3 && lastChar == "이")
    {
        // base가 등록된 명사인지 확인
        std::string originalWord = base + lastChar;
        if (dictionary_.isNoun(originalWord))
        {
            return {};  // 분리하지 않음
        }
    }

    // Builtin 변수/함수 보호
    // 예: "경로로" -> "경로" + "로" (경로는 builtin 변수)
    std::string originalWord = base + lastChar;
    if (dictionary_.isBuiltinVar(originalWord) || dictionary_.isBuiltinFunc(originalWord))
    {
        return {};  // 분리하지 않음
    }

    // 분리 성공
    return {
        Morpheme(word, base, ""),
        Morpheme(word, lastChar, "")
    };
}

bool MorphologicalAnalyzer::hasKoreanAfterUnderscoreOrDigit(const std::string& word) const
{
    // 역순으로 스캔하여 언더스코어나 숫자를 찾음
    for (size_t i = 0; i < word.length(); i++)
    {
        unsigned char byte = static_cast<unsigned char>(word[i]);

        // ASCII 문자 발견
        if (byte < 0x80)
        {
            // 언더스코어나 숫자이고, 뒤에 문자가 있는 경우
            if ((byte == '_' || (byte >= '0' && byte <= '9')) && i + 1 < word.length())
            {
                // 다음 바이트가 한글 시작인지 확인
                unsigned char nextByte = static_cast<unsigned char>(word[i + 1]);
                if ((nextByte & 0xF0) == 0xE0)  // UTF-8 3바이트 한글
                {
                    return true;
                }
            }
        }
    }

    return false;
}

std::vector<Morpheme> MorphologicalAnalyzer::tryASCIIWithJosa(const std::string& word) const
{
    // 최소 4바이트 필요 (ASCII 1바이트 + 한글 조사 3바이트)
    if (word.length() < 4)
    {
        return {};
    }

    // 마지막 3바이트가 한글인지 확인
    unsigned char lastByte = static_cast<unsigned char>(word[word.length() - 3]);
    if ((lastByte & 0xF0) != 0xE0)  // UTF-8 3바이트 한글 시작 바이트가 아님
    {
        return {};
    }

    // 마지막 3바이트 (1글자) 추출
    std::string lastChar = word.substr(word.length() - 3);

    // 조사인지 확인
    if (!dictionary_.isJosa(lastChar))
    {
        return {};
    }

    // 기본형 추출 (마지막 3바이트 제외)
    std::string base = word.substr(0, word.length() - 3);

    // 기본형이 비어있으면 실패
    if (base.empty())
    {
        return {};
    }

    // 분리 성공
    return {
        Morpheme(word, base, ""),
        Morpheme(word, lastChar, "")
    };
}

} // namespace morphology
} // namespace kingsejong
