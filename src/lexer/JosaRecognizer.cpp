/**
 * @file JosaRecognizer.cpp
 * @brief JosaRecognizer 클래스 구현
 * @author KingSejong Team
 * @date 2025-11-06
 */

#include "JosaRecognizer.h"
#include "Token.h"
#include <stdexcept>
#include <unordered_map>

namespace kingsejong {
namespace lexer {

bool JosaRecognizer::isJosa(const std::string& str)
{
    // Token.h의 isJosa 함수 사용
    return lexer::isJosa(str);
}

JosaRecognizer::JosaType JosaRecognizer::getType(const std::string& str)
{
    static const std::unordered_map<std::string, JosaType> josaMap = {
        {"을", JosaType::EUL_REUL},
        {"를", JosaType::EUL_REUL},
        {"이", JosaType::I_GA},
        {"가", JosaType::I_GA},
        {"은", JosaType::EUN_NEUN},
        {"는", JosaType::EUN_NEUN},
        {"의", JosaType::UI},
        {"로", JosaType::RO_EURO},
        {"으로", JosaType::RO_EURO},
        {"에서", JosaType::ESO},
        {"에", JosaType::E}
    };

    auto it = josaMap.find(str);
    if (it != josaMap.end())
    {
        return it->second;
    }

    throw std::invalid_argument("Invalid josa: " + str);
}

int JosaRecognizer::getFinalConsonantIndex(const std::string& ch)
{
    // UTF-8 한글 문자 처리
    // 한글은 3바이트로 인코딩됨
    if (ch.length() < 3)
    {
        return -1;  // 한글이 아님
    }

    // UTF-8에서 유니코드 코드 포인트 추출
    unsigned char b1 = static_cast<unsigned char>(ch[0]);
    unsigned char b2 = static_cast<unsigned char>(ch[1]);
    unsigned char b3 = static_cast<unsigned char>(ch[2]);

    // UTF-8 3바이트 문자 확인 (1110xxxx 10xxxxxx 10xxxxxx)
    if ((b1 & 0xF0) != 0xE0 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80)
    {
        return -1;  // 올바른 UTF-8 3바이트 문자가 아님
    }

    // 유니코드 코드 포인트 계산
    int unicode = ((b1 & 0x0F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);

    // 한글 완성형 범위 확인 (0xAC00 ~ 0xD7A3)
    if (unicode < 0xAC00 || unicode > 0xD7A3)
    {
        return -1;  // 한글 완성형이 아님
    }

    // 종성(받침) 인덱스 계산
    // 한글 유니코드 = 0xAC00 + (초성 * 588) + (중성 * 28) + 종성
    int jongseong = (unicode - 0xAC00) % 28;

    return jongseong;
}

bool JosaRecognizer::hasFinalConsonant(const std::string& word)
{
    if (word.empty())
    {
        return false;
    }

    // 마지막 글자 추출 (UTF-8 한글은 3바이트)
    if (word.length() < 3)
    {
        return false;  // 한글이 아님
    }

    // 마지막 한글 글자 (마지막 3바이트)
    std::string lastChar = word.substr(word.length() - 3, 3);

    int jongseong = getFinalConsonantIndex(lastChar);

    // 종성이 0이 아니면 받침 있음
    return jongseong > 0;
}

bool JosaRecognizer::hasRieulFinal(const std::string& word)
{
    if (word.empty())
    {
        return false;
    }

    // 마지막 글자 추출
    if (word.length() < 3)
    {
        return false;
    }

    std::string lastChar = word.substr(word.length() - 3, 3);
    int jongseong = getFinalConsonantIndex(lastChar);

    // ㄹ 받침은 인덱스 8
    return jongseong == 8;
}

std::string JosaRecognizer::select(const std::string& noun, JosaType type)
{
    bool hasFinal = hasFinalConsonant(noun);

    switch (type)
    {
        case JosaType::EUL_REUL:
            return hasFinal ? "을" : "를";

        case JosaType::I_GA:
            return hasFinal ? "이" : "가";

        case JosaType::EUN_NEUN:
            return hasFinal ? "은" : "는";

        case JosaType::UI:
            return "의";  // 받침 무관

        case JosaType::RO_EURO:
            // ㄹ 받침이거나 받침이 없으면 "로"
            if (hasRieulFinal(noun) || !hasFinal)
            {
                return "로";
            }
            return "으로";

        case JosaType::ESO:
            return "에서";  // 받침 무관

        case JosaType::E:
            return "에";  // 받침 무관

        default:
            throw std::invalid_argument("Unknown JosaType");
    }
}

std::string JosaRecognizer::josaTypeToString(JosaType type)
{
    switch (type)
    {
        case JosaType::EUL_REUL:  return "EUL_REUL";
        case JosaType::I_GA:      return "I_GA";
        case JosaType::EUN_NEUN:  return "EUN_NEUN";
        case JosaType::UI:        return "UI";
        case JosaType::RO_EURO:   return "RO_EURO";
        case JosaType::ESO:       return "ESO";
        case JosaType::E:         return "E";
        default:                  return "UNKNOWN";
    }
}

} // namespace lexer
} // namespace kingsejong
