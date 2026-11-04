#pragma once

/**
 * @file FloatFormat.h
 * @brief 실수를 문자열로 바꾸는 규칙 하나 (Python 의 repr 과 같다)
 *
 * 되읽으면 같은 값이 되는 가장 짧은 자릿수를 쓴다(0.1 + 0.2 → 0.30000000000000004).
 * 정수 값인 실수는 ".0" 을 붙여 정수와 구별한다(3.0). 지수가 -4 이상 16 미만이면 고정 소수점,
 * 그 밖은 지수 표기다(1e+16, 1e-05). 출력·문자열()·문자열 보간이 모두 이 함수를 쓴다.
 */

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <string>
#include <system_error>

namespace kingsejong {
namespace common {

inline std::string formatFloat(double value)
{
    if (std::isnan(value)) return "nan";
    if (std::isinf(value)) return value < 0 ? "-inf" : "inf";
    if (value == 0.0) return std::signbit(value) ? "-0.0" : "0.0";

    // 가장 짧은 되읽기 자릿수를 지수 표기로 받는다: "-1.2345e+05"
    char buffer[64];
    auto result = std::to_chars(buffer, buffer + sizeof(buffer), value, std::chars_format::scientific);
    if (result.ec != std::errc())
    {
        return "nan";  // 64 바이트면 모자라지 않는다
    }
    std::string text(buffer, result.ptr);

    std::string sign;
    size_t pos = 0;
    if (text[0] == '-')
    {
        sign = "-";
        pos = 1;
    }
    size_t ePos = text.find('e');
    std::string digits;
    for (size_t i = pos; i < ePos; ++i)
    {
        if (text[i] != '.') digits += text[i];
    }
    int exponent = std::atoi(text.c_str() + ePos + 1);

    if (exponent >= -4 && exponent < 16)
    {
        std::string out = sign;
        if (exponent >= 0)
        {
            size_t intDigits = static_cast<size_t>(exponent) + 1;
            if (digits.size() <= intDigits)
            {
                out += digits + std::string(intDigits - digits.size(), '0') + ".0";
            }
            else
            {
                out += digits.substr(0, intDigits) + "." + digits.substr(intDigits);
            }
        }
        else
        {
            out += "0." + std::string(static_cast<size_t>(-exponent - 1), '0') + digits;
        }
        return out;
    }

    std::string out = sign + digits.substr(0, 1);
    if (digits.size() > 1)
    {
        out += "." + digits.substr(1);
    }
    out += exponent < 0 ? "e-" : "e+";
    int magnitude = exponent < 0 ? -exponent : exponent;
    if (magnitude < 10) out += "0";
    out += std::to_string(magnitude);
    return out;
}

} // namespace common
} // namespace kingsejong
