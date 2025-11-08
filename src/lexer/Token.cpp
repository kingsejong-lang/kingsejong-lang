/**
 * @file Token.cpp
 * @brief Token 관련 함수 구현
 * @author KingSejong Team
 * @date 2025-11-06
 */

#include "Token.h"

namespace kingsejong {
namespace lexer {

/**
 * @brief 한글 키워드 매핑 테이블
 *
 * 한글 키워드 문자열을 TokenType으로 매핑하는 해시 테이블입니다.
 */
static const std::unordered_map<std::string, TokenType> keywords = {
    // 조사 - 목적격
    {"을", TokenType::JOSA_EUL},
    {"를", TokenType::JOSA_REUL},

    // 조사 - 주격
    {"이", TokenType::JOSA_I},
    {"가", TokenType::JOSA_GA},

    // 조사 - 보조사
    {"은", TokenType::JOSA_EUN},
    {"는", TokenType::JOSA_NEUN},

    // 조사 - 소유격
    {"의", TokenType::JOSA_UI},

    // 조사 - 방향/수단
    {"로", TokenType::JOSA_RO},
    {"으로", TokenType::JOSA_EURO},

    // 조사 - 위치
    {"에서", TokenType::JOSA_ESO},
    {"에", TokenType::JOSA_E},

    // 범위 키워드
    {"부터", TokenType::BUTEO},
    {"까지", TokenType::KKAJI},
    {"미만", TokenType::MIMAN},
    {"초과", TokenType::CHOGA},
    {"이하", TokenType::IHA},
    {"이상", TokenType::ISANG},

    // 반복 키워드
    {"번", TokenType::BEON},
    {"반복한다", TokenType::BANBOKHANDA},
    {"반복", TokenType::BANBOKK},
    {"각각", TokenType::GAKGAK},

    // 제어문 키워드
    {"만약", TokenType::MANYAK},
    {"아니면", TokenType::ANIMYEON},
    {"동안", TokenType::DONGAN},

    // 함수 키워드
    {"함수", TokenType::HAMSU},
    {"반환", TokenType::BANHWAN},

    // 타입 키워드
    {"정수", TokenType::JEONGSU},
    {"실수", TokenType::SILSU},
    {"문자", TokenType::MUNJA},
    {"문자열", TokenType::MUNJAYEOL},
    {"논리", TokenType::NONLI},

    // 불리언 리터럴
    {"참", TokenType::CHAM},
    {"거짓", TokenType::GEOJIT},

    // 메서드 체이닝
    {"하고", TokenType::HAGO},
    {"하라", TokenType::HARA}
};

std::string tokenTypeToString(TokenType type)
{
    switch (type)
    {
        case TokenType::ILLEGAL:      return "ILLEGAL";
        case TokenType::EOF_TOKEN:    return "EOF";
        case TokenType::IDENTIFIER:   return "IDENTIFIER";
        case TokenType::INTEGER:      return "INTEGER";
        case TokenType::FLOAT:        return "FLOAT";
        case TokenType::STRING:       return "STRING";

        case TokenType::ASSIGN:       return "ASSIGN";
        case TokenType::PLUS:         return "PLUS";
        case TokenType::MINUS:        return "MINUS";
        case TokenType::ASTERISK:     return "ASTERISK";
        case TokenType::SLASH:        return "SLASH";
        case TokenType::PERCENT:      return "PERCENT";

        case TokenType::EQ:           return "EQ";
        case TokenType::NOT_EQ:       return "NOT_EQ";
        case TokenType::LT:           return "LT";
        case TokenType::GT:           return "GT";
        case TokenType::LE:           return "LE";
        case TokenType::GE:           return "GE";

        case TokenType::AND:          return "AND";
        case TokenType::OR:           return "OR";
        case TokenType::NOT:          return "NOT";

        case TokenType::JOSA_EUL:     return "JOSA_EUL";
        case TokenType::JOSA_REUL:    return "JOSA_REUL";
        case TokenType::JOSA_I:       return "JOSA_I";
        case TokenType::JOSA_GA:      return "JOSA_GA";
        case TokenType::JOSA_EUN:     return "JOSA_EUN";
        case TokenType::JOSA_NEUN:    return "JOSA_NEUN";
        case TokenType::JOSA_UI:      return "JOSA_UI";
        case TokenType::JOSA_RO:      return "JOSA_RO";
        case TokenType::JOSA_EURO:    return "JOSA_EURO";
        case TokenType::JOSA_ESO:     return "JOSA_ESO";
        case TokenType::JOSA_E:       return "JOSA_E";

        case TokenType::BUTEO:        return "BUTEO";
        case TokenType::KKAJI:        return "KKAJI";
        case TokenType::MIMAN:        return "MIMAN";
        case TokenType::CHOGA:        return "CHOGA";
        case TokenType::IHA:          return "IHA";
        case TokenType::ISANG:        return "ISANG";

        case TokenType::BEON:         return "BEON";
        case TokenType::BANBOKHANDA:  return "BANBOKHANDA";
        case TokenType::BANBOKK:      return "BANBOKK";
        case TokenType::GAKGAK:       return "GAKGAK";

        case TokenType::MANYAK:       return "MANYAK";
        case TokenType::ANIMYEON:     return "ANIMYEON";
        case TokenType::ANIMYEON_MANYAK: return "ANIMYEON_MANYAK";
        case TokenType::DONGAN:       return "DONGAN";

        case TokenType::HAMSU:        return "HAMSU";
        case TokenType::BANHWAN:      return "BANHWAN";

        case TokenType::JEONGSU:      return "JEONGSU";
        case TokenType::SILSU:        return "SILSU";
        case TokenType::MUNJA:        return "MUNJA";
        case TokenType::MUNJAYEOL:    return "MUNJAYEOL";
        case TokenType::NONLI:        return "NONLI";

        case TokenType::CHAM:         return "CHAM";
        case TokenType::GEOJIT:       return "GEOJIT";

        case TokenType::LPAREN:       return "LPAREN";
        case TokenType::RPAREN:       return "RPAREN";
        case TokenType::LBRACE:       return "LBRACE";
        case TokenType::RBRACE:       return "RBRACE";
        case TokenType::LBRACKET:     return "LBRACKET";
        case TokenType::RBRACKET:     return "RBRACKET";

        case TokenType::COMMA:        return "COMMA";
        case TokenType::SEMICOLON:    return "SEMICOLON";
        case TokenType::COLON:        return "COLON";

        case TokenType::HAGO:         return "HAGO";
        case TokenType::HARA:         return "HARA";

        default:                      return "UNKNOWN";
    }
}

TokenType lookupKeyword(const std::string& literal)
{
    auto it = keywords.find(literal);
    if (it != keywords.end())
    {
        return it->second;
    }
    return TokenType::IDENTIFIER;
}

bool isJosa(const std::string& literal)
{
    TokenType type = lookupKeyword(literal);
    return type == TokenType::JOSA_EUL ||
           type == TokenType::JOSA_REUL ||
           type == TokenType::JOSA_I ||
           type == TokenType::JOSA_GA ||
           type == TokenType::JOSA_EUN ||
           type == TokenType::JOSA_NEUN ||
           type == TokenType::JOSA_UI ||
           type == TokenType::JOSA_RO ||
           type == TokenType::JOSA_EURO ||
           type == TokenType::JOSA_ESO ||
           type == TokenType::JOSA_E;
}

bool isRangeKeyword(const std::string& literal)
{
    TokenType type = lookupKeyword(literal);
    return type == TokenType::BUTEO ||
           type == TokenType::KKAJI ||
           type == TokenType::MIMAN ||
           type == TokenType::CHOGA ||
           type == TokenType::IHA ||
           type == TokenType::ISANG;
}

} // namespace lexer
} // namespace kingsejong
