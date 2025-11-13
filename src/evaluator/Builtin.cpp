/**
 * @file Builtin.cpp
 * @brief KingSejong 언어 내장 함수 구현
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include "Builtin.h"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace kingsejong {
namespace evaluator {

// Static 멤버 초기화
std::unordered_map<std::string, Value> Builtin::builtins_;

// ============================================================================
// 내장 함수 관리
// ============================================================================

void Builtin::registerBuiltin(const std::string& name, Value::BuiltinFn fn)
{
    builtins_[name] = Value::createBuiltinFunction(fn);
}

Value Builtin::getBuiltin(const std::string& name)
{
    auto it = builtins_.find(name);
    if (it != builtins_.end())
    {
        return it->second;
    }
    return Value::createNull();
}

bool Builtin::hasBuiltin(const std::string& name)
{
    return builtins_.find(name) != builtins_.end();
}

// ============================================================================
// 내장 함수 구현
// ============================================================================

/**
 * @brief 출력(값1, 값2, ...) - 콘솔에 출력
 *
 * 여러 인자를 공백으로 구분하여 출력하고 개행합니다.
 * 모든 타입의 값을 출력할 수 있습니다.
 *
 * @param args 출력할 값들
 * @return null
 */
static Value builtin_출력(const std::vector<Value>& args)
{
    for (size_t i = 0; i < args.size(); ++i)
    {
        if (i > 0) std::cout << " ";

        if (args[i].isString())
        {
            std::cout << args[i].asString();
        }
        else if (args[i].isInteger())
        {
            std::cout << args[i].asInteger();
        }
        else if (args[i].isFloat())
        {
            std::cout << args[i].asFloat();
        }
        else if (args[i].isBoolean())
        {
            std::cout << (args[i].asBoolean() ? "참" : "거짓");
        }
        else if (args[i].isNull())
        {
            std::cout << "없음";
        }
        else if (args[i].isFunction())
        {
            std::cout << "[함수]";
        }
        else if (args[i].isBuiltinFunction())
        {
            std::cout << "[내장함수]";
        }
        else
        {
            std::cout << args[i].toString();
        }
    }
    std::cout << std::endl;

    return Value::createNull();
}

/**
 * @brief 타입(값) - 값의 타입을 문자열로 반환
 *
 * @param args 타입을 확인할 값 (1개)
 * @return 타입 문자열
 * @throws std::runtime_error 인자 개수가 1개가 아닌 경우
 */
static Value builtin_타입(const std::vector<Value>& args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("타입() 함수는 정확히 1개의 인자가 필요합니다");
    }

    const Value& arg = args[0];

    if (arg.isInteger()) return Value::createString("정수");
    if (arg.isFloat()) return Value::createString("실수");
    if (arg.isString()) return Value::createString("문자열");
    if (arg.isBoolean()) return Value::createString("논리");
    if (arg.isNull()) return Value::createString("없음");
    if (arg.isFunction()) return Value::createString("함수");
    if (arg.isBuiltinFunction()) return Value::createString("내장함수");

    return Value::createString("알 수 없음");
}

/**
 * @brief 길이(배열/문자열) - 길이 반환
 *
 * 문자열의 경우 UTF-8 문자 수를 반환합니다.
 * 배열의 경우 요소 개수를 반환합니다.
 *
 * @param args 길이를 확인할 값 (1개)
 * @return 길이 (정수)
 * @throws std::runtime_error 인자 개수가 1개가 아니거나, 문자열/배열이 아닌 경우
 */
static Value builtin_길이(const std::vector<Value>& args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("길이() 함수는 정확히 1개의 인자가 필요합니다");
    }

    const Value& arg = args[0];

    // 문자열의 경우: UTF-8 문자 수 계산
    if (arg.isString())
    {
        const std::string& str = arg.asString();
        int64_t count = 0;

        for (size_t i = 0; i < str.length(); )
        {
            unsigned char c = static_cast<unsigned char>(str[i]);
            size_t charLen = 1;

            if ((c & 0x80) == 0)
            {
                // 1바이트 문자 (ASCII)
                charLen = 1;
            }
            else if ((c & 0xE0) == 0xC0)
            {
                // 2바이트 UTF-8
                charLen = 2;
            }
            else if ((c & 0xF0) == 0xE0)
            {
                // 3바이트 UTF-8 (한글 등)
                charLen = 3;
            }
            else if ((c & 0xF8) == 0xF0)
            {
                // 4바이트 UTF-8
                charLen = 4;
            }

            // 문자열 끝을 넘어가지 않도록 체크
            if (i + charLen > str.length())
            {
                charLen = str.length() - i;
            }

            i += charLen;
            count++;
        }

        return Value::createInteger(count);
    }

    // 배열의 경우: 요소 개수 반환
    if (arg.isArray())
    {
        return Value::createInteger(static_cast<int64_t>(arg.asArray().size()));
    }

    throw std::runtime_error("길이() 함수는 문자열 또는 배열 타입이어야 합니다");
}

/**
 * @brief 분리(문자열, 구분자) - 문자열을 구분자로 분리하여 배열 반환
 *
 * @param args 문자열, 구분자 (2개)
 * @return 분리된 문자열 배열
 * @throws std::runtime_error 인자 개수가 2개가 아니거나, 둘 다 문자열이 아닌 경우
 */
static Value builtin_분리(const std::vector<Value>& args)
{
    if (args.size() != 2)
    {
        throw std::runtime_error("분리() 함수는 정확히 2개의 인자가 필요합니다");
    }

    if (!args[0].isString() || !args[1].isString())
    {
        throw std::runtime_error("분리() 함수의 인자는 모두 문자열이어야 합니다");
    }

    const std::string& str = args[0].asString();
    const std::string& delimiter = args[1].asString();

    std::vector<Value> result;

    if (delimiter.empty())
    {
        // 구분자가 빈 문자열이면 각 문자를 분리
        for (size_t i = 0; i < str.length(); )
        {
            unsigned char c = static_cast<unsigned char>(str[i]);
            size_t charLen = 1;

            if ((c & 0x80) == 0)
            {
                charLen = 1;  // ASCII
            }
            else if ((c & 0xE0) == 0xC0)
            {
                charLen = 2;  // 2바이트 UTF-8
            }
            else if ((c & 0xF0) == 0xE0)
            {
                charLen = 3;  // 3바이트 UTF-8 (한글 등)
            }
            else if ((c & 0xF8) == 0xF0)
            {
                charLen = 4;  // 4바이트 UTF-8
            }

            // 문자열 끝을 넘어가지 않도록 체크
            if (i + charLen > str.length())
            {
                charLen = str.length() - i;
            }

            result.push_back(Value::createString(str.substr(i, charLen)));
            i += charLen;
        }
    }
    else
    {
        // 구분자로 분리
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (end != std::string::npos)
        {
            result.push_back(Value::createString(str.substr(start, end - start)));
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }

        result.push_back(Value::createString(str.substr(start)));
    }

    return Value::createArray(result);
}

/**
 * @brief 찾기(문자열, 패턴) - 문자열에서 패턴의 위치를 찾아 반환
 *
 * @param args 문자열, 패턴 (2개)
 * @return 찾은 위치 (0부터 시작), 없으면 -1
 * @throws std::runtime_error 인자 개수가 2개가 아니거나, 둘 다 문자열이 아닌 경우
 */
static Value builtin_찾기(const std::vector<Value>& args)
{
    if (args.size() != 2)
    {
        throw std::runtime_error("찾기() 함수는 정확히 2개의 인자가 필요합니다");
    }

    if (!args[0].isString() || !args[1].isString())
    {
        throw std::runtime_error("찾기() 함수의 인자는 모두 문자열이어야 합니다");
    }

    const std::string& str = args[0].asString();
    const std::string& pattern = args[1].asString();

    size_t pos = str.find(pattern);

    if (pos == std::string::npos)
    {
        return Value::createInteger(-1);
    }

    // UTF-8 문자 인덱스로 변환
    int64_t charIndex = 0;
    for (size_t i = 0; i < pos && i < str.length(); )
    {
        unsigned char c = static_cast<unsigned char>(str[i]);
        size_t charLen = 1;

        if ((c & 0x80) == 0)
        {
            charLen = 1;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            charLen = 2;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            charLen = 3;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            charLen = 4;
        }

        // 문자열 끝을 넘어가지 않도록 체크
        if (i + charLen > str.length())
        {
            charLen = str.length() - i;
        }

        i += charLen;
        charIndex++;
    }

    return Value::createInteger(charIndex);
}

/**
 * @brief 바꾸기(문자열, 이전, 이후) - 문자열에서 모든 '이전' 패턴을 '이후'로 치환
 *
 * @param args 문자열, 이전, 이후 (3개)
 * @return 치환된 문자열
 * @throws std::runtime_error 인자 개수가 3개가 아니거나, 모두 문자열이 아닌 경우
 */
static Value builtin_바꾸기(const std::vector<Value>& args)
{
    if (args.size() != 3)
    {
        throw std::runtime_error("바꾸기() 함수는 정확히 3개의 인자가 필요합니다");
    }

    if (!args[0].isString() || !args[1].isString() || !args[2].isString())
    {
        throw std::runtime_error("바꾸기() 함수의 인자는 모두 문자열이어야 합니다");
    }

    std::string result = args[0].asString();
    const std::string& oldStr = args[1].asString();
    const std::string& newStr = args[2].asString();

    if (oldStr.empty())
    {
        return Value::createString(result);
    }

    size_t pos = 0;
    while ((pos = result.find(oldStr, pos)) != std::string::npos)
    {
        result.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }

    return Value::createString(result);
}

/**
 * @brief 대문자(문자열) - 영문자를 대문자로 변환
 *
 * @param args 문자열 (1개)
 * @return 대문자로 변환된 문자열
 * @throws std::runtime_error 인자 개수가 1개가 아니거나, 문자열이 아닌 경우
 */
static Value builtin_대문자(const std::vector<Value>& args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("대문자() 함수는 정확히 1개의 인자가 필요합니다");
    }

    if (!args[0].isString())
    {
        throw std::runtime_error("대문자() 함수의 인자는 문자열이어야 합니다");
    }

    std::string result = args[0].asString();

    for (char& c : result)
    {
        if (c >= 'a' && c <= 'z')
        {
            c = c - 'a' + 'A';
        }
    }

    return Value::createString(result);
}

/**
 * @brief 소문자(문자열) - 영문자를 소문자로 변환
 *
 * @param args 문자열 (1개)
 * @return 소문자로 변환된 문자열
 * @throws std::runtime_error 인자 개수가 1개가 아니거나, 문자열이 아닌 경우
 */
static Value builtin_소문자(const std::vector<Value>& args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("소문자() 함수는 정확히 1개의 인자가 필요합니다");
    }

    if (!args[0].isString())
    {
        throw std::runtime_error("소문자() 함수의 인자는 문자열이어야 합니다");
    }

    std::string result = args[0].asString();

    for (char& c : result)
    {
        if (c >= 'A' && c <= 'Z')
        {
            c = c - 'A' + 'a';
        }
    }

    return Value::createString(result);
}

// ============================================================================
// 타입 변환 함수
// ============================================================================

/**
 * @brief 정수(값) - 값을 정수로 변환
 *
 * @param args 변환할 값 (1개)
 * @return 정수 값
 * @throws std::runtime_error 변환할 수 없는 타입인 경우
 */
static Value builtin_정수(const std::vector<Value>& args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("정수() 함수는 정확히 1개의 인자가 필요합니다");
    }

    const Value& arg = args[0];

    // 이미 정수면 그대로 반환
    if (arg.isInteger())
    {
        return arg;
    }

    // 실수를 정수로 변환
    if (arg.isFloat())
    {
        return Value::createInteger(static_cast<int64_t>(arg.asFloat()));
    }

    // 문자열을 정수로 변환
    if (arg.isString())
    {
        try
        {
            int64_t value = std::stoll(arg.asString());
            return Value::createInteger(value);
        }
        catch (...)
        {
            throw std::runtime_error("문자열을 정수로 변환할 수 없습니다: " + arg.asString());
        }
    }

    // 불린을 정수로 변환 (참=1, 거짓=0)
    if (arg.isBoolean())
    {
        return Value::createInteger(arg.asBoolean() ? 1 : 0);
    }

    throw std::runtime_error("정수() 함수는 정수, 실수, 문자열, 논리 타입만 변환할 수 있습니다");
}

/**
 * @brief 실수(값) - 값을 실수로 변환
 *
 * @param args 변환할 값 (1개)
 * @return 실수 값
 * @throws std::runtime_error 변환할 수 없는 타입인 경우
 */
static Value builtin_실수(const std::vector<Value>& args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("실수() 함수는 정확히 1개의 인자가 필요합니다");
    }

    const Value& arg = args[0];

    // 이미 실수면 그대로 반환
    if (arg.isFloat())
    {
        return arg;
    }

    // 정수를 실수로 변환
    if (arg.isInteger())
    {
        return Value::createFloat(static_cast<double>(arg.asInteger()));
    }

    // 문자열을 실수로 변환
    if (arg.isString())
    {
        try
        {
            double value = std::stod(arg.asString());
            return Value::createFloat(value);
        }
        catch (...)
        {
            throw std::runtime_error("문자열을 실수로 변환할 수 없습니다: " + arg.asString());
        }
    }

    // 불린을 실수로 변환 (참=1.0, 거짓=0.0)
    if (arg.isBoolean())
    {
        return Value::createFloat(arg.asBoolean() ? 1.0 : 0.0);
    }

    throw std::runtime_error("실수() 함수는 정수, 실수, 문자열, 논리 타입만 변환할 수 있습니다");
}

// ============================================================================
// 수학 함수
// ============================================================================

/**
 * @brief 반올림(실수) - 실수를 반올림
 *
 * Round half toward zero (절댓값이 작은 쪽으로):
 * - 2.5 → 3 (양수는 올림)
 * - -2.5 → -2 (음수는 내림)
 *
 * @param args 실수 값 (1개)
 * @return 반올림된 정수
 * @throws std::runtime_error 인자가 실수나 정수가 아닌 경우
 */
static Value builtin_반올림(const std::vector<Value>& args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("반올림() 함수는 정확히 1개의 인자가 필요합니다");
    }

    const Value& arg = args[0];

    if (arg.isInteger())
    {
        return arg;
    }

    if (arg.isFloat())
    {
        double value = arg.asFloat();
        // Round half up (toward positive infinity for .5):
        // floor(value + 0.5) works for both positive and negative
        // 2.5 → floor(3.0) = 3
        // -2.5 → floor(-2.0) = -2
        return Value::createInteger(static_cast<int64_t>(std::floor(value + 0.5)));
    }

    throw std::runtime_error("반올림() 함수의 인자는 정수 또는 실수여야 합니다");
}

/**
 * @brief 올림(실수) - 실수를 올림
 *
 * @param args 실수 값 (1개)
 * @return 올림된 정수
 * @throws std::runtime_error 인자가 실수나 정수가 아닌 경우
 */
static Value builtin_올림(const std::vector<Value>& args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("올림() 함수는 정확히 1개의 인자가 필요합니다");
    }

    const Value& arg = args[0];

    if (arg.isInteger())
    {
        return arg;
    }

    if (arg.isFloat())
    {
        return Value::createInteger(static_cast<int64_t>(std::ceil(arg.asFloat())));
    }

    throw std::runtime_error("올림() 함수의 인자는 정수 또는 실수여야 합니다");
}

/**
 * @brief 내림(실수) - 실수를 내림
 *
 * @param args 실수 값 (1개)
 * @return 내림된 정수
 * @throws std::runtime_error 인자가 실수나 정수가 아닌 경우
 */
static Value builtin_내림(const std::vector<Value>& args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("내림() 함수는 정확히 1개의 인자가 필요합니다");
    }

    const Value& arg = args[0];

    if (arg.isInteger())
    {
        return arg;
    }

    if (arg.isFloat())
    {
        return Value::createInteger(static_cast<int64_t>(std::floor(arg.asFloat())));
    }

    throw std::runtime_error("내림() 함수의 인자는 정수 또는 실수여야 합니다");
}

/**
 * @brief 절대값(수) - 절대값 반환
 *
 * @param args 수 값 (1개)
 * @return 절대값
 * @throws std::runtime_error 인자가 수가 아닌 경우
 */
static Value builtin_절대값(const std::vector<Value>& args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("절대값() 함수는 정확히 1개의 인자가 필요합니다");
    }

    const Value& arg = args[0];

    if (arg.isInteger())
    {
        int64_t value = arg.asInteger();
        return Value::createInteger(value < 0 ? -value : value);
    }

    if (arg.isFloat())
    {
        return Value::createFloat(std::abs(arg.asFloat()));
    }

    throw std::runtime_error("절대값() 함수의 인자는 정수 또는 실수여야 합니다");
}

/**
 * @brief 제곱근(수) - 제곱근 반환
 *
 * @param args 수 값 (1개)
 * @return 제곱근 (실수)
 * @throws std::runtime_error 인자가 수가 아니거나 음수인 경우
 */
static Value builtin_제곱근(const std::vector<Value>& args)
{
    if (args.size() != 1)
    {
        throw std::runtime_error("제곱근() 함수는 정확히 1개의 인자가 필요합니다");
    }

    const Value& arg = args[0];
    double value = 0.0;

    if (arg.isInteger())
    {
        value = static_cast<double>(arg.asInteger());
    }
    else if (arg.isFloat())
    {
        value = arg.asFloat();
    }
    else
    {
        throw std::runtime_error("제곱근() 함수의 인자는 정수 또는 실수여야 합니다");
    }

    if (value < 0.0)
    {
        throw std::runtime_error("제곱근() 함수의 인자는 음수가 아니어야 합니다");
    }

    return Value::createFloat(std::sqrt(value));
}

/**
 * @brief 제곱(수, 지수) - 거듭제곱 반환
 *
 * @param args 밑, 지수 (2개)
 * @return 거듭제곱 결과
 * @throws std::runtime_error 인자가 수가 아닌 경우
 */
static Value builtin_제곱(const std::vector<Value>& args)
{
    if (args.size() != 2)
    {
        throw std::runtime_error("제곱() 함수는 정확히 2개의 인자가 필요합니다");
    }

    double base = 0.0;
    double exponent = 0.0;

    if (args[0].isInteger())
    {
        base = static_cast<double>(args[0].asInteger());
    }
    else if (args[0].isFloat())
    {
        base = args[0].asFloat();
    }
    else
    {
        throw std::runtime_error("제곱() 함수의 첫 번째 인자는 정수 또는 실수여야 합니다");
    }

    if (args[1].isInteger())
    {
        exponent = static_cast<double>(args[1].asInteger());
    }
    else if (args[1].isFloat())
    {
        exponent = args[1].asFloat();
    }
    else
    {
        throw std::runtime_error("제곱() 함수의 두 번째 인자는 정수 또는 실수여야 합니다");
    }

    double result = std::pow(base, exponent);

    // 결과가 정수인지 확인 (지수가 양의 정수이고 밑이 정수인 경우)
    if (args[0].isInteger() && args[1].isInteger() && args[1].asInteger() >= 0)
    {
        return Value::createInteger(static_cast<int64_t>(result));
    }

    return Value::createFloat(result);
}

// ============================================================================
// 파일 I/O 함수
// ============================================================================

/**
 * @brief 파일_읽기(경로) - 파일의 전체 내용을 읽어 문자열로 반환
 *
 * @param args[0] 파일 경로 (문자열)
 * @return 파일 내용 (문자열) 또는 에러
 */
static Value builtin_파일_읽기(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("파일_읽기() 함수는 정확히 1개의 인자가 필요합니다");
    }

    if (!args[0].isString()) {
        throw std::runtime_error("파일_읽기() 함수의 인자는 문자열(파일 경로)이어야 합니다");
    }

    std::string filepath = args[0].asString();
    std::ifstream file(filepath);

    if (!file.is_open()) {
        throw std::runtime_error("파일을 열 수 없습니다: " + filepath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return Value::createString(buffer.str());
}

/**
 * @brief 파일_쓰기(경로, 내용) - 파일에 내용을 씁니다 (덮어쓰기)
 *
 * @param args[0] 파일 경로 (문자열)
 * @param args[1] 파일에 쓸 내용 (문자열)
 * @return null
 */
static Value builtin_파일_쓰기(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("파일_쓰기() 함수는 정확히 2개의 인자가 필요합니다");
    }

    if (!args[0].isString()) {
        throw std::runtime_error("파일_쓰기() 함수의 첫 번째 인자는 문자열(파일 경로)이어야 합니다");
    }

    if (!args[1].isString()) {
        throw std::runtime_error("파일_쓰기() 함수의 두 번째 인자는 문자열(내용)이어야 합니다");
    }

    std::string filepath = args[0].asString();
    std::string content = args[1].asString();

    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("파일을 열 수 없습니다: " + filepath);
    }

    file << content;
    file.close();

    return Value::createNull();
}

/**
 * @brief 파일_추가(경로, 내용) - 파일에 내용을 추가합니다 (append)
 *
 * @param args[0] 파일 경로 (문자열)
 * @param args[1] 파일에 추가할 내용 (문자열)
 * @return null
 */
static Value builtin_파일_추가(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("파일_추가() 함수는 정확히 2개의 인자가 필요합니다");
    }

    if (!args[0].isString()) {
        throw std::runtime_error("파일_추가() 함수의 첫 번째 인자는 문자열(파일 경로)이어야 합니다");
    }

    if (!args[1].isString()) {
        throw std::runtime_error("파일_추가() 함수의 두 번째 인자는 문자열(내용)이어야 합니다");
    }

    std::string filepath = args[0].asString();
    std::string content = args[1].asString();

    std::ofstream file(filepath, std::ios::app);
    if (!file.is_open()) {
        throw std::runtime_error("파일을 열 수 없습니다: " + filepath);
    }

    file << content;
    file.close();

    return Value::createNull();
}

/**
 * @brief 파일_존재(경로) - 파일이 존재하는지 확인
 *
 * @param args[0] 파일 경로 (문자열)
 * @return 존재 여부 (불리언)
 */
static Value builtin_파일_존재(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("파일_존재() 함수는 정확히 1개의 인자가 필요합니다");
    }

    if (!args[0].isString()) {
        throw std::runtime_error("파일_존재() 함수의 인자는 문자열(파일 경로)이어야 합니다");
    }

    std::string filepath = args[0].asString();
    bool exists = std::filesystem::exists(filepath);

    return Value::createBoolean(exists);
}

/**
 * @brief 줄별_읽기(경로) - 파일을 줄별로 읽어 배열로 반환
 *
 * @param args[0] 파일 경로 (문자열)
 * @return 줄들의 배열
 */
static Value builtin_줄별_읽기(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("줄별_읽기() 함수는 정확히 1개의 인자가 필요합니다");
    }

    if (!args[0].isString()) {
        throw std::runtime_error("줄별_읽기() 함수의 인자는 문자열(파일 경로)이어야 합니다");
    }

    std::string filepath = args[0].asString();
    std::ifstream file(filepath);

    if (!file.is_open()) {
        throw std::runtime_error("파일을 열 수 없습니다: " + filepath);
    }

    std::vector<Value> lines;
    std::string line;

    while (std::getline(file, line)) {
        lines.push_back(Value::createString(line));
    }

    file.close();

    return Value::createArray(lines);
}

/**
 * @brief 파일_삭제(경로) - 파일을 삭제
 *
 * @param args[0] 파일 경로 (문자열)
 * @return null
 */
static Value builtin_파일_삭제(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("파일_삭제() 함수는 정확히 1개의 인자가 필요합니다");
    }

    if (!args[0].isString()) {
        throw std::runtime_error("파일_삭제() 함수의 인자는 문자열(파일 경로)이어야 합니다");
    }

    std::string filepath = args[0].asString();

    if (!std::filesystem::exists(filepath)) {
        throw std::runtime_error("파일이 존재하지 않습니다: " + filepath);
    }

    if (!std::filesystem::remove(filepath)) {
        throw std::runtime_error("파일을 삭제할 수 없습니다: " + filepath);
    }

    return Value::createNull();
}

/**
 * @brief 디렉토리_생성(경로) - 디렉토리를 생성 (중첩 디렉토리 지원)
 *
 * @param args[0] 디렉토리 경로 (문자열)
 * @return null
 */
static Value builtin_디렉토리_생성(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("디렉토리_생성() 함수는 정확히 1개의 인자가 필요합니다");
    }

    if (!args[0].isString()) {
        throw std::runtime_error("디렉토리_생성() 함수의 인자는 문자열(디렉토리 경로)이어야 합니다");
    }

    std::string dirpath = args[0].asString();

    if (!std::filesystem::create_directories(dirpath)) {
        // create_directories는 이미 존재하면 false 반환
        if (!std::filesystem::exists(dirpath)) {
            throw std::runtime_error("디렉토리를 생성할 수 없습니다: " + dirpath);
        }
    }

    return Value::createNull();
}

/**
 * @brief 디렉토리_삭제(경로) - 디렉토리를 삭제 (하위 파일/디렉토리 모두 삭제)
 *
 * @param args[0] 디렉토리 경로 (문자열)
 * @return null
 */
static Value builtin_디렉토리_삭제(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("디렉토리_삭제() 함수는 정확히 1개의 인자가 필요합니다");
    }

    if (!args[0].isString()) {
        throw std::runtime_error("디렉토리_삭제() 함수의 인자는 문자열(디렉토리 경로)이어야 합니다");
    }

    std::string dirpath = args[0].asString();

    if (!std::filesystem::exists(dirpath)) {
        throw std::runtime_error("디렉토리가 존재하지 않습니다: " + dirpath);
    }

    std::filesystem::remove_all(dirpath);

    return Value::createNull();
}

/**
 * @brief 디렉토리_목록(경로) - 디렉토리 내 파일/디렉토리 목록을 배열로 반환
 *
 * @param args[0] 디렉토리 경로 (문자열)
 * @return 파일/디렉토리 이름 배열
 */
static Value builtin_디렉토리_목록(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("디렉토리_목록() 함수는 정확히 1개의 인자가 필요합니다");
    }

    if (!args[0].isString()) {
        throw std::runtime_error("디렉토리_목록() 함수의 인자는 문자열(디렉토리 경로)이어야 합니다");
    }

    std::string dirpath = args[0].asString();

    if (!std::filesystem::exists(dirpath)) {
        throw std::runtime_error("디렉토리가 존재하지 않습니다: " + dirpath);
    }

    if (!std::filesystem::is_directory(dirpath)) {
        throw std::runtime_error("경로가 디렉토리가 아닙니다: " + dirpath);
    }

    std::vector<Value> entries;

    for (const auto& entry : std::filesystem::directory_iterator(dirpath)) {
        entries.push_back(Value::createString(entry.path().filename().string()));
    }

    return Value::createArray(entries);
}

/**
 * @brief 현재_디렉토리() - 현재 작업 디렉토리 경로를 반환
 *
 * @return 현재 작업 디렉토리 경로 (문자열)
 */
static Value builtin_현재_디렉토리(const std::vector<Value>& args)
{
    if (args.size() != 0) {
        throw std::runtime_error("현재_디렉토리() 함수는 인자가 필요하지 않습니다");
    }

    std::string current_path = std::filesystem::current_path().string();

    return Value::createString(current_path);
}

// ============================================================================
// 내장 함수 등록
// ============================================================================

void Builtin::registerAllBuiltins()
{
    // 기본 함수
    registerBuiltin("출력", builtin_출력);
    registerBuiltin("타입", builtin_타입);
    registerBuiltin("길이", builtin_길이);

    // 문자열 함수
    registerBuiltin("분리", builtin_분리);
    registerBuiltin("찾기", builtin_찾기);
    registerBuiltin("바꾸기", builtin_바꾸기);
    registerBuiltin("대문자", builtin_대문자);
    registerBuiltin("소문자", builtin_소문자);

    // 타입 변환 함수
    registerBuiltin("정수", builtin_정수);
    registerBuiltin("실수", builtin_실수);

    // 수학 함수
    registerBuiltin("반올림", builtin_반올림);
    registerBuiltin("올림", builtin_올림);
    registerBuiltin("내림", builtin_내림);
    registerBuiltin("절대값", builtin_절대값);
    registerBuiltin("제곱근", builtin_제곱근);
    registerBuiltin("제곱", builtin_제곱);

    // 파일 I/O 함수
    registerBuiltin("파일_읽기", builtin_파일_읽기);
    registerBuiltin("파일_쓰기", builtin_파일_쓰기);
    registerBuiltin("파일_추가", builtin_파일_추가);
    registerBuiltin("파일_존재", builtin_파일_존재);
    registerBuiltin("줄별_읽기", builtin_줄별_읽기);
    registerBuiltin("파일_삭제", builtin_파일_삭제);
    registerBuiltin("디렉토리_생성", builtin_디렉토리_생성);
    registerBuiltin("디렉토리_삭제", builtin_디렉토리_삭제);
    registerBuiltin("디렉토리_목록", builtin_디렉토리_목록);
    registerBuiltin("현재_디렉토리", builtin_현재_디렉토리);
}

} // namespace evaluator
} // namespace kingsejong
