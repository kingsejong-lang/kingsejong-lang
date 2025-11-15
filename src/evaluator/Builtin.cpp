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
#include <nlohmann/json.hpp>
#include <chrono>
#include <iomanip>
#include <thread>
#include <regex>
#include <random>
#include <algorithm>
#include <functional>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define getcwd _getcwd
    #define chdir _chdir
#else
    #include <unistd.h>
    #include <pwd.h>
    #include <sys/stat.h>
#endif

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
// JSON 처리 함수
// ============================================================================

// JSON → Value 변환 헬퍼 함수
static Value jsonToValue(const nlohmann::json& j)
{
    if (j.is_null()) {
        return Value::createNull();
    }
    else if (j.is_boolean()) {
        return Value::createBoolean(j.get<bool>());
    }
    else if (j.is_number_integer()) {
        return Value::createInteger(j.get<int64_t>());
    }
    else if (j.is_number_float()) {
        return Value::createFloat(j.get<double>());
    }
    else if (j.is_string()) {
        return Value::createString(j.get<std::string>());
    }
    else if (j.is_array()) {
        std::vector<Value> arr;
        for (const auto& item : j) {
            arr.push_back(jsonToValue(item));
        }
        return Value::createArray(arr);
    }
    else if (j.is_object()) {
        // JSON object를 [["key", value], ...] 형태의 2D 배열로 변환
        std::vector<Value> pairs;
        for (auto it = j.begin(); it != j.end(); ++it) {
            std::vector<Value> pair;
            pair.push_back(Value::createString(it.key()));
            pair.push_back(jsonToValue(it.value()));
            pairs.push_back(Value::createArray(pair));
        }
        return Value::createArray(pairs);
    }

    return Value::createNull();
}

// Value → JSON 변환 헬퍼 함수
static nlohmann::json valueToJson(const Value& val)
{
    if (val.isNull()) {
        return nlohmann::json(nullptr);
    }
    else if (val.isBoolean()) {
        return nlohmann::json(val.asBoolean());
    }
    else if (val.isInteger()) {
        return nlohmann::json(val.asInteger());
    }
    else if (val.isFloat()) {
        return nlohmann::json(val.asFloat());
    }
    else if (val.isString()) {
        return nlohmann::json(val.asString());
    }
    else if (val.isArray()) {
        const auto& arr = val.asArray();

        // 빈 배열은 JSON 배열로
        if (arr.empty()) {
            return nlohmann::json::array();
        }

        // [["key", value], ...] 형태면 JSON object로 변환
        bool isObject = true;
        for (const auto& item : arr) {
            if (!item.isArray() || item.asArray().size() != 2) {
                isObject = false;
                break;
            }
            if (!item.asArray()[0].isString()) {
                isObject = false;
                break;
            }
        }

        if (isObject) {
            // JSON object로 변환
            nlohmann::json obj = nlohmann::json::object();
            for (const auto& item : arr) {
                std::string key = item.asArray()[0].asString();
                obj[key] = valueToJson(item.asArray()[1]);
            }
            return obj;
        }
        else {
            // 일반 JSON 배열로 변환
            nlohmann::json json_arr = nlohmann::json::array();
            for (const auto& item : arr) {
                json_arr.push_back(valueToJson(item));
            }
            return json_arr;
        }
    }

    return nlohmann::json(nullptr);
}

// JSON 문자열을 파싱한다
static Value builtin_JSON_파싱(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("JSON_파싱() 함수는 정확히 1개의 인자가 필요합니다");
    }

    if (!args[0].isString()) {
        throw std::runtime_error("JSON_파싱() 함수의 인자는 문자열이어야 합니다");
    }

    try {
        std::string json_str = args[0].asString();
        nlohmann::json j = nlohmann::json::parse(json_str);
        return jsonToValue(j);
    }
    catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("JSON 파싱 오류: " + std::string(e.what()));
    }
}

// 값을 JSON 문자열로 변환한다
static Value builtin_JSON_문자열화(const std::vector<Value>& args)
{
    if (args.size() < 1 || args.size() > 2) {
        throw std::runtime_error("JSON_문자열화() 함수는 1개 또는 2개의 인자가 필요합니다");
    }

    nlohmann::json j = valueToJson(args[0]);

    // 두 번째 인자: 들여쓰기 크기 (선택적)
    int indent = -1;  // -1이면 압축 형식
    if (args.size() == 2) {
        if (!args[1].isInteger()) {
            throw std::runtime_error("JSON_문자열화() 함수의 두 번째 인자는 정수여야 합니다");
        }
        indent = static_cast<int>(args[1].asInteger());
    }

    std::string result = j.dump(indent);
    return Value::createString(result);
}

// JSON 파일을 읽어서 파싱한다
static Value builtin_JSON_파일_읽기(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("JSON_파일_읽기() 함수는 정확히 1개의 인자가 필요합니다");
    }

    if (!args[0].isString()) {
        throw std::runtime_error("JSON_파일_읽기() 함수의 인자는 문자열(파일 경로)이어야 합니다");
    }

    std::string filepath = args[0].asString();

    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("JSON 파일을 열 수 없습니다: " + filepath);
    }

    try {
        nlohmann::json j;
        file >> j;
        file.close();
        return jsonToValue(j);
    }
    catch (const nlohmann::json::parse_error& e) {
        file.close();
        throw std::runtime_error("JSON 파일 파싱 오류: " + std::string(e.what()));
    }
}

// 값을 JSON 파일로 저장한다
static Value builtin_JSON_파일_쓰기(const std::vector<Value>& args)
{
    if (args.size() < 2 || args.size() > 3) {
        throw std::runtime_error("JSON_파일_쓰기() 함수는 2개 또는 3개의 인자가 필요합니다");
    }

    if (!args[0].isString()) {
        throw std::runtime_error("JSON_파일_쓰기() 함수의 첫 번째 인자는 문자열(파일 경로)이어야 합니다");
    }

    std::string filepath = args[0].asString();
    nlohmann::json j = valueToJson(args[1]);

    // 세 번째 인자: 들여쓰기 크기 (선택적)
    int indent = 2;  // 기본값: 2칸 들여쓰기
    if (args.size() == 3) {
        if (!args[2].isInteger()) {
            throw std::runtime_error("JSON_파일_쓰기() 함수의 세 번째 인자는 정수여야 합니다");
        }
        indent = static_cast<int>(args[2].asInteger());
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("JSON 파일을 열 수 없습니다: " + filepath);
    }

    file << j.dump(indent);
    file.close();

    return Value::createNull();
}

// ============================================================================
// 시간/날짜 처리 함수
// ============================================================================

// 현재 Unix timestamp를 초 단위로 반환한다
static Value builtin_현재_시간(const std::vector<Value>& args)
{
    if (!args.empty()) {
        throw std::runtime_error("현재_시간() 함수는 인자가 필요하지 않습니다");
    }

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

    return Value::createInteger(seconds);
}

// 현재 날짜를 YYYY-MM-DD 형식으로 반환한다
static Value builtin_현재_날짜(const std::vector<Value>& args)
{
    if (!args.empty()) {
        throw std::runtime_error("현재_날짜() 함수는 인자가 필요하지 않습니다");
    }

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    std::tm tm_now;
    #ifdef _WIN32
        localtime_s(&tm_now, &time_t_now);
    #else
        localtime_r(&time_t_now, &tm_now);
    #endif

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d");

    return Value::createString(oss.str());
}

// 타임스탬프를 지정된 형식으로 변환한다
static Value builtin_시간_포맷(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("시간_포맷() 함수는 정확히 2개의 인자가 필요합니다");
    }

    if (!args[0].isInteger()) {
        throw std::runtime_error("시간_포맷() 함수의 첫 번째 인자는 정수(타임스탬프)여야 합니다");
    }

    if (!args[1].isString()) {
        throw std::runtime_error("시간_포맷() 함수의 두 번째 인자는 문자열(포맷)이어야 합니다");
    }

    int64_t timestamp = args[0].asInteger();
    std::string format = args[1].asString();

    std::time_t time_t_val = static_cast<std::time_t>(timestamp);
    std::tm tm_val;

    #ifdef _WIN32
        localtime_s(&tm_val, &time_t_val);
    #else
        localtime_r(&time_t_val, &tm_val);
    #endif

    std::ostringstream oss;
    oss << std::put_time(&tm_val, format.c_str());

    return Value::createString(oss.str());
}

// 현재 시간을 밀리초 단위 타임스탬프로 반환한다
static Value builtin_타임스탬프(const std::vector<Value>& args)
{
    if (!args.empty()) {
        throw std::runtime_error("타임스탬프() 함수는 인자가 필요하지 않습니다");
    }

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    return Value::createInteger(milliseconds);
}

// 지정된 시간만큼 대기한다 (밀리초)
static Value builtin_슬립(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("슬립() 함수는 정확히 1개의 인자가 필요합니다");
    }

    if (!args[0].isInteger()) {
        throw std::runtime_error("슬립() 함수의 인자는 정수(밀리초)여야 합니다");
    }

    int64_t milliseconds = args[0].asInteger();

    if (milliseconds < 0) {
        throw std::runtime_error("슬립() 함수의 인자는 0 이상이어야 합니다");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));

    return Value::createNull();
}

// 현재 시간을 HH:MM:SS 형식으로 반환한다
static Value builtin_현재_시각(const std::vector<Value>& args)
{
    if (!args.empty()) {
        throw std::runtime_error("현재_시각() 함수는 인자가 필요하지 않습니다");
    }

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    std::tm tm_now;
    #ifdef _WIN32
        localtime_s(&tm_now, &time_t_now);
    #else
        localtime_r(&time_t_now, &tm_now);
    #endif

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%H:%M:%S");

    return Value::createString(oss.str());
}

// 현재 날짜와 시간을 YYYY-MM-DD HH:MM:SS 형식으로 반환한다
static Value builtin_현재_날짜시간(const std::vector<Value>& args)
{
    if (!args.empty()) {
        throw std::runtime_error("현재_날짜시간() 함수는 인자가 필요하지 않습니다");
    }

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    std::tm tm_now;
    #ifdef _WIN32
        localtime_s(&tm_now, &time_t_now);
    #else
        localtime_r(&time_t_now, &tm_now);
    #endif

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");

    return Value::createString(oss.str());
}

// ============================================================================
// 정규표현식 함수
// ============================================================================

/**
 * @brief 정규표현식_일치(문자열, 패턴) - 전체 문자열이 패턴과 일치하는지 확인
 */
static Value builtin_정규표현식_일치(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("정규표현식_일치(문자열, 패턴): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isString()) {
        throw std::runtime_error("정규표현식_일치(문자열, 패턴): 문자열 타입이어야 합니다");
    }

    try {
        std::regex pattern(args[1].asString());
        bool matches = std::regex_match(args[0].asString(), pattern);
        return Value::createBoolean(matches);
    } catch (const std::regex_error& e) {
        throw std::runtime_error("정규표현식 오류: " + std::string(e.what()));
    }
}

/**
 * @brief 정규표현식_검색(문자열, 패턴) - 패턴이 문자열 내에 존재하는지 확인
 */
static Value builtin_정규표현식_검색(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("정규표현식_검색(문자열, 패턴): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isString()) {
        throw std::runtime_error("정규표현식_검색(문자열, 패턴): 문자열 타입이어야 합니다");
    }

    try {
        std::regex pattern(args[1].asString());
        bool found = std::regex_search(args[0].asString(), pattern);
        return Value::createBoolean(found);
    } catch (const std::regex_error& e) {
        throw std::runtime_error("정규표현식 오류: " + std::string(e.what()));
    }
}

/**
 * @brief 정규표현식_모두_찾기(문자열, 패턴) - 모든 일치 항목을 배열로 반환
 */
static Value builtin_정규표현식_모두_찾기(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("정규표현식_모두_찾기(문자열, 패턴): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isString()) {
        throw std::runtime_error("정규표현식_모두_찾기(문자열, 패턴): 문자열 타입이어야 합니다");
    }

    try {
        std::regex pattern(args[1].asString());
        std::string text = args[0].asString();
        std::vector<Value> matches;

        auto begin = std::sregex_iterator(text.begin(), text.end(), pattern);
        auto end = std::sregex_iterator();

        for (std::sregex_iterator i = begin; i != end; ++i) {
            matches.push_back(Value::createString(i->str()));
        }

        return Value::createArray(matches);
    } catch (const std::regex_error& e) {
        throw std::runtime_error("정규표현식 오류: " + std::string(e.what()));
    }
}

/**
 * @brief 정규표현식_치환(문자열, 패턴, 교체) - 패턴과 일치하는 부분을 교체 문자열로 치환
 */
static Value builtin_정규표현식_치환(const std::vector<Value>& args)
{
    if (args.size() != 3) {
        throw std::runtime_error("정규표현식_치환(문자열, 패턴, 교체): 3개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isString() || !args[2].isString()) {
        throw std::runtime_error("정규표현식_치환(문자열, 패턴, 교체): 모두 문자열 타입이어야 합니다");
    }

    try {
        std::regex pattern(args[1].asString());
        std::string result = std::regex_replace(args[0].asString(), pattern, args[2].asString());
        return Value::createString(result);
    } catch (const std::regex_error& e) {
        throw std::runtime_error("정규표현식 오류: " + std::string(e.what()));
    }
}

/**
 * @brief 정규표현식_분리(문자열, 패턴) - 패턴으로 문자열을 분리하여 배열 반환
 */
static Value builtin_정규표현식_분리(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("정규표현식_분리(문자열, 패턴): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isString()) {
        throw std::runtime_error("정규표현식_분리(문자열, 패턴): 문자열 타입이어야 합니다");
    }

    try {
        std::regex pattern(args[1].asString());
        std::string text = args[0].asString();
        std::vector<Value> parts;

        std::sregex_token_iterator iter(text.begin(), text.end(), pattern, -1);
        std::sregex_token_iterator end;

        for (; iter != end; ++iter) {
            parts.push_back(Value::createString(*iter));
        }

        return Value::createArray(parts);
    } catch (const std::regex_error& e) {
        throw std::runtime_error("정규표현식 오류: " + std::string(e.what()));
    }
}

/**
 * @brief 이메일_검증(문자열) - 이메일 형식이 유효한지 확인
 */
static Value builtin_이메일_검증(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("이메일_검증(문자열): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("이메일_검증(문자열): 문자열 타입이어야 합니다");
    }

    // 간단한 이메일 패턴
    std::regex pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    bool valid = std::regex_match(args[0].asString(), pattern);
    return Value::createBoolean(valid);
}

/**
 * @brief URL_검증(문자열) - URL 형식이 유효한지 확인
 */
static Value builtin_URL_검증(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("URL_검증(문자열): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("URL_검증(문자열): 문자열 타입이어야 합니다");
    }

    // 간단한 URL 패턴
    std::regex pattern(R"(^https?://[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}(/.*)?$)");
    bool valid = std::regex_match(args[0].asString(), pattern);
    return Value::createBoolean(valid);
}

/**
 * @brief 전화번호_검증(문자열) - 한국 전화번호 형식이 유효한지 확인
 */
static Value builtin_전화번호_검증(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("전화번호_검증(문자열): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("전화번호_검증(문자열): 문자열 타입이어야 합니다");
    }

    // 한국 전화번호 패턴: 010-XXXX-XXXX, 02-XXX-XXXX, 031-XXX-XXXX 등
    std::regex pattern(R"(^0\d{1,2}-\d{3,4}-\d{4}$)");
    bool valid = std::regex_match(args[0].asString(), pattern);
    return Value::createBoolean(valid);
}

/**
 * @brief 정규표현식_추출(문자열, 패턴) - 첫 번째 일치 항목 추출
 */
static Value builtin_정규표현식_추출(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("정규표현식_추출(문자열, 패턴): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isString()) {
        throw std::runtime_error("정규표현식_추출(문자열, 패턴): 문자열 타입이어야 합니다");
    }

    try {
        std::regex pattern(args[1].asString());
        std::string text = args[0].asString();
        std::smatch match;

        if (std::regex_search(text, match, pattern)) {
            return Value::createString(match.str());
        }

        return Value::createString("");  // 일치하는 항목이 없으면 빈 문자열
    } catch (const std::regex_error& e) {
        throw std::runtime_error("정규표현식 오류: " + std::string(e.what()));
    }
}

/**
 * @brief 정규표현식_개수(문자열, 패턴) - 일치하는 항목의 개수 반환
 */
static Value builtin_정규표현식_개수(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("정규표현식_개수(문자열, 패턴): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isString()) {
        throw std::runtime_error("정규표현식_개수(문자열, 패턴): 문자열 타입이어야 합니다");
    }

    try {
        std::regex pattern(args[1].asString());
        std::string text = args[0].asString();

        auto begin = std::sregex_iterator(text.begin(), text.end(), pattern);
        auto end = std::sregex_iterator();

        int64_t count = std::distance(begin, end);
        return Value::createInteger(count);
    } catch (const std::regex_error& e) {
        throw std::runtime_error("정규표현식 오류: " + std::string(e.what()));
    }
}

// ============================================================================
// 암호화 함수
// ============================================================================

/**
 * @brief Base64_인코딩(문자열) - Base64 인코딩
 */
static Value builtin_Base64_인코딩(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("Base64_인코딩(문자열): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("Base64_인코딩(문자열): 문자열 타입이어야 합니다");
    }

    static const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string input = args[0].asString();
    std::string result;
    int val = 0;
    int valb = -6;

    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }

    if (valb > -6) {
        result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }

    while (result.size() % 4) {
        result.push_back('=');
    }

    return Value::createString(result);
}

/**
 * @brief Base64_디코딩(문자열) - Base64 디코딩
 */
static Value builtin_Base64_디코딩(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("Base64_디코딩(문자열): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("Base64_디코딩(문자열): 문자열 타입이어야 합니다");
    }

    static const unsigned char base64_table[256] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };

    std::string input = args[0].asString();
    std::string result;
    int val = 0;
    int valb = -8;

    for (unsigned char c : input) {
        if (base64_table[c] == 64) break;
        val = (val << 6) + base64_table[c];
        valb += 6;
        if (valb >= 0) {
            result.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    return Value::createString(result);
}

/**
 * @brief 문자열_해시(문자열) - 간단한 해시 생성
 */
static Value builtin_문자열_해시(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("문자열_해시(문자열): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("문자열_해시(문자열): 문자열 타입이어야 합니다");
    }

    std::string input = args[0].asString();
    std::hash<std::string> hasher;
    size_t hash_value = hasher(input);

    std::ostringstream oss;
    oss << std::hex << hash_value;
    return Value::createString(oss.str());
}

/**
 * @brief 파일_해시(경로) - 파일 해시
 */
static Value builtin_파일_해시(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("파일_해시(경로): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("파일_해시(경로): 문자열 타입이어야 합니다");
    }

    std::string path = args[0].asString();
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("파일을 열 수 없습니다: " + path);
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string content = oss.str();

    std::hash<std::string> hasher;
    size_t hash_value = hasher(content);

    std::ostringstream result;
    result << std::hex << hash_value;
    return Value::createString(result.str());
}

/**
 * @brief 해시_비교(문자열1, 문자열2) - 해시 비교
 */
static Value builtin_해시_비교(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("해시_비교(문자열1, 문자열2): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isString()) {
        throw std::runtime_error("해시_비교(문자열1, 문자열2): 문자열 타입이어야 합니다");
    }

    std::hash<std::string> hasher;
    size_t hash1 = hasher(args[0].asString());
    size_t hash2 = hasher(args[1].asString());

    return Value::createBoolean(hash1 == hash2);
}

/**
 * @brief 체크섬(문자열) - 간단한 체크섬 계산
 */
static Value builtin_체크섬(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("체크섬(문자열): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("체크섬(문자열): 문자열 타입이어야 합니다");
    }

    std::string input = args[0].asString();
    uint32_t checksum = 0;

    for (unsigned char c : input) {
        checksum = (checksum << 1) | (checksum >> 31);
        checksum += c;
    }

    return Value::createInteger(static_cast<int64_t>(checksum));
}

/**
 * @brief XOR_암호화(문자열, 키) - XOR 암호화
 */
static Value builtin_XOR_암호화(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("XOR_암호화(문자열, 키): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isString()) {
        throw std::runtime_error("XOR_암호화(문자열, 키): 문자열 타입이어야 합니다");
    }

    std::string text = args[0].asString();
    std::string key = args[1].asString();

    if (key.empty()) {
        throw std::runtime_error("키는 비어있을 수 없습니다");
    }

    std::string result;
    size_t key_idx = 0;

    for (char c : text) {
        result.push_back(c ^ key[key_idx]);
        key_idx = (key_idx + 1) % key.length();
    }

    return Value::createString(result);
}

/**
 * @brief XOR_복호화(문자열, 키) - XOR 복호화 (암호화와 동일)
 */
static Value builtin_XOR_복호화(const std::vector<Value>& args)
{
    return builtin_XOR_암호화(args);  // XOR은 대칭
}

/**
 * @brief 시저_암호화(문자열, 이동) - Caesar cipher 암호화
 */
static Value builtin_시저_암호화(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("시저_암호화(문자열, 이동): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isInteger()) {
        throw std::runtime_error("시저_암호화(문자열, 이동): 문자열과 정수 타입이어야 합니다");
    }

    std::string text = args[0].asString();
    int64_t shift = args[1].asInteger();
    std::string result;

    for (char c : text) {
        if (c >= 'a' && c <= 'z') {
            result.push_back('a' + (c - 'a' + shift + 26) % 26);
        } else if (c >= 'A' && c <= 'Z') {
            result.push_back('A' + (c - 'A' + shift + 26) % 26);
        } else {
            result.push_back(c);
        }
    }

    return Value::createString(result);
}

/**
 * @brief 시저_복호화(문자열, 이동) - Caesar cipher 복호화
 */
static Value builtin_시저_복호화(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("시저_복호화(문자열, 이동): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isInteger()) {
        throw std::runtime_error("시저_복호화(문자열, 이동): 문자열과 정수 타입이어야 합니다");
    }

    // 복호화는 -shift로 암호화
    std::vector<Value> decrypt_args = args;
    decrypt_args[1] = Value::createInteger(-args[1].asInteger());
    return builtin_시저_암호화(decrypt_args);
}

/**
 * @brief 랜덤_문자열(길이) - 랜덤 문자열 생성
 */
static Value builtin_랜덤_문자열(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("랜덤_문자열(길이): 1개의 인자가 필요합니다");
    }
    if (!args[0].isInteger()) {
        throw std::runtime_error("랜덤_문자열(길이): 정수 타입이어야 합니다");
    }

    int64_t length = args[0].asInteger();
    if (length < 0) {
        throw std::runtime_error("길이는 0 이상이어야 합니다");
    }

    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);

    std::string result;
    result.reserve(length);

    for (int64_t i = 0; i < length; ++i) {
        result.push_back(alphanum[dis(gen)]);
    }

    return Value::createString(result);
}

/**
 * @brief 랜덤_숫자(최소, 최대) - 랜덤 정수 생성
 */
static Value builtin_랜덤_숫자(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("랜덤_숫자(최소, 최대): 2개의 인자가 필요합니다");
    }
    if (!args[0].isInteger() || !args[1].isInteger()) {
        throw std::runtime_error("랜덤_숫자(최소, 최대): 정수 타입이어야 합니다");
    }

    int64_t min = args[0].asInteger();
    int64_t max = args[1].asInteger();

    if (min > max) {
        throw std::runtime_error("최소값은 최대값보다 작거나 같아야 합니다");
    }

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int64_t> dis(min, max);

    return Value::createInteger(dis(gen));
}

// ============================================================================
// OS 및 파일 시스템 함수
// ============================================================================

/**
 * @brief 환경변수_읽기(이름) - 환경 변수 읽기
 */
static Value builtin_환경변수_읽기(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("환경변수_읽기(이름): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("환경변수_읽기(이름): 문자열 타입이어야 합니다");
    }

    std::string name = args[0].asString();
    const char* value = std::getenv(name.c_str());

    if (value == nullptr) {
        return Value::createString("");
    }

    return Value::createString(value);
}

/**
 * @brief 환경변수_쓰기(이름, 값) - 환경 변수 설정
 */
static Value builtin_환경변수_쓰기(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("환경변수_쓰기(이름, 값): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isString()) {
        throw std::runtime_error("환경변수_쓰기(이름, 값): 문자열 타입이어야 합니다");
    }

    std::string name = args[0].asString();
    std::string value = args[1].asString();

#ifdef _WIN32
    _putenv_s(name.c_str(), value.c_str());
#else
    setenv(name.c_str(), value.c_str(), 1);
#endif

    return Value::createBoolean(true);
}

/**
 * @brief 환경변수_존재하는가(이름) - 환경 변수 존재 여부
 */
static Value builtin_환경변수_존재하는가(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("환경변수_존재하는가(이름): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("환경변수_존재하는가(이름): 문자열 타입이어야 합니다");
    }

    std::string name = args[0].asString();
    const char* value = std::getenv(name.c_str());

    return Value::createBoolean(value != nullptr);
}

/**
 * @brief 환경변수_삭제(이름) - 환경 변수 삭제
 */
static Value builtin_환경변수_삭제(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("환경변수_삭제(이름): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("환경변수_삭제(이름): 문자열 타입이어야 합니다");
    }

    std::string name = args[0].asString();

#ifdef _WIN32
    _putenv_s(name.c_str(), "");
#else
    unsetenv(name.c_str());
#endif

    return Value::createBoolean(true);
}

/**
 * @brief 디렉토리_변경(경로) - 작업 디렉토리 변경
 */
static Value builtin_디렉토리_변경(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("디렉토리_변경(경로): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("디렉토리_변경(경로): 문자열 타입이어야 합니다");
    }

    std::string path = args[0].asString();

    try {
        std::filesystem::current_path(path);
        return Value::createBoolean(true);
    } catch (const std::exception& e) {
        throw std::runtime_error("디렉토리를 변경할 수 없습니다: " + std::string(e.what()));
    }
}

/**
 * @brief 디렉토리_나열(경로) - 디렉토리 내용 나열
 */
static Value builtin_디렉토리_나열(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("디렉토리_나열(경로): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("디렉토리_나열(경로): 문자열 타입이어야 합니다");
    }

    std::string path = args[0].asString();

    try {
        std::vector<Value> entries;
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            entries.push_back(Value::createString(entry.path().filename().string()));
        }
        return Value::createArray(entries);
    } catch (const std::exception& e) {
        throw std::runtime_error("디렉토리를 나열할 수 없습니다: " + std::string(e.what()));
    }
}

/**
 * @brief 디렉토리인가(경로) - 디렉토리 여부 확인
 */
static Value builtin_디렉토리인가(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("디렉토리인가(경로): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("디렉토리인가(경로): 문자열 타입이어야 합니다");
    }

    std::string path = args[0].asString();

    try {
        return Value::createBoolean(std::filesystem::is_directory(path));
    } catch (const std::exception&) {
        return Value::createBoolean(false);
    }
}

/**
 * @brief 임시_디렉토리() - 임시 디렉토리 경로
 */
static Value builtin_임시_디렉토리(const std::vector<Value>& args)
{
    if (args.size() != 0) {
        throw std::runtime_error("임시_디렉토리(): 인자가 필요하지 않습니다");
    }

    try {
        auto path = std::filesystem::temp_directory_path();
        return Value::createString(path.string());
    } catch (const std::exception& e) {
        throw std::runtime_error("임시 디렉토리를 읽을 수 없습니다: " + std::string(e.what()));
    }
}

/**
 * @brief 파일_존재하는가(경로) - 파일 존재 여부
 */
static Value builtin_파일_존재하는가(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("파일_존재하는가(경로): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("파일_존재하는가(경로): 문자열 타입이어야 합니다");
    }

    std::string path = args[0].asString();

    try {
        return Value::createBoolean(std::filesystem::exists(path));
    } catch (const std::exception&) {
        return Value::createBoolean(false);
    }
}

/**
 * @brief 파일_복사(출발, 목적) - 파일 복사
 */
static Value builtin_파일_복사(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("파일_복사(출발, 목적): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isString()) {
        throw std::runtime_error("파일_복사(출발, 목적): 문자열 타입이어야 합니다");
    }

    std::string from = args[0].asString();
    std::string to = args[1].asString();

    try {
        std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing);
        return Value::createBoolean(true);
    } catch (const std::exception& e) {
        throw std::runtime_error("파일을 복사할 수 없습니다: " + std::string(e.what()));
    }
}

/**
 * @brief 파일_이동(출발, 목적) - 파일 이동
 */
static Value builtin_파일_이동(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("파일_이동(출발, 목적): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isString()) {
        throw std::runtime_error("파일_이동(출발, 목적): 문자열 타입이어야 합니다");
    }

    std::string from = args[0].asString();
    std::string to = args[1].asString();

    try {
        std::filesystem::rename(from, to);
        return Value::createBoolean(true);
    } catch (const std::exception& e) {
        throw std::runtime_error("파일을 이동할 수 없습니다: " + std::string(e.what()));
    }
}

/**
 * @brief 파일_크기(경로) - 파일 크기 (바이트)
 */
static Value builtin_파일_크기(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("파일_크기(경로): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("파일_크기(경로): 문자열 타입이어야 합니다");
    }

    std::string path = args[0].asString();

    try {
        auto size = std::filesystem::file_size(path);
        return Value::createInteger(static_cast<int64_t>(size));
    } catch (const std::exception& e) {
        throw std::runtime_error("파일 크기를 읽을 수 없습니다: " + std::string(e.what()));
    }
}

/**
 * @brief 파일인가(경로) - 일반 파일 여부
 */
static Value builtin_파일인가(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("파일인가(경로): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("파일인가(경로): 문자열 타입이어야 합니다");
    }

    std::string path = args[0].asString();

    try {
        return Value::createBoolean(std::filesystem::is_regular_file(path));
    } catch (const std::exception&) {
        return Value::createBoolean(false);
    }
}

/**
 * @brief 경로_결합(경로1, 경로2) - 경로 결합
 */
static Value builtin_경로_결합(const std::vector<Value>& args)
{
    if (args.size() != 2) {
        throw std::runtime_error("경로_결합(경로1, 경로2): 2개의 인자가 필요합니다");
    }
    if (!args[0].isString() || !args[1].isString()) {
        throw std::runtime_error("경로_결합(경로1, 경로2): 문자열 타입이어야 합니다");
    }

    std::filesystem::path path1 = args[0].asString();
    std::filesystem::path path2 = args[1].asString();

    auto combined = path1 / path2;
    return Value::createString(combined.string());
}

/**
 * @brief 파일명_추출(경로) - 파일명 추출
 */
static Value builtin_파일명_추출(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("파일명_추출(경로): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("파일명_추출(경로): 문자열 타입이어야 합니다");
    }

    std::filesystem::path path = args[0].asString();
    return Value::createString(path.filename().string());
}

/**
 * @brief 확장자_추출(경로) - 확장자 추출
 */
static Value builtin_확장자_추출(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("확장자_추출(경로): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("확장자_추출(경로): 문자열 타입이어야 합니다");
    }

    std::filesystem::path path = args[0].asString();
    return Value::createString(path.extension().string());
}

/**
 * @brief 절대경로(경로) - 절대 경로 변환
 */
static Value builtin_절대경로(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("절대경로(경로): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("절대경로(경로): 문자열 타입이어야 합니다");
    }

    std::filesystem::path path = args[0].asString();

    try {
        auto abs = std::filesystem::absolute(path);
        return Value::createString(abs.string());
    } catch (const std::exception& e) {
        throw std::runtime_error("절대 경로를 계산할 수 없습니다: " + std::string(e.what()));
    }
}

/**
 * @brief 경로_존재하는가(경로) - 경로 존재 여부
 */
static Value builtin_경로_존재하는가(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("경로_존재하는가(경로): 1개의 인자가 필요합니다");
    }
    if (!args[0].isString()) {
        throw std::runtime_error("경로_존재하는가(경로): 문자열 타입이어야 합니다");
    }

    std::string path = args[0].asString();

    try {
        return Value::createBoolean(std::filesystem::exists(path));
    } catch (const std::exception&) {
        return Value::createBoolean(false);
    }
}

/**
 * @brief OS_이름() - 운영체제 이름
 */
static Value builtin_OS_이름(const std::vector<Value>& args)
{
    if (args.size() != 0) {
        throw std::runtime_error("OS_이름(): 인자가 필요하지 않습니다");
    }

#ifdef _WIN32
    return Value::createString("Windows");
#elif __APPLE__
    return Value::createString("macOS");
#elif __linux__
    return Value::createString("Linux");
#else
    return Value::createString("Unknown");
#endif
}

/**
 * @brief 사용자_이름() - 현재 사용자 이름
 */
static Value builtin_사용자_이름(const std::vector<Value>& args)
{
    if (args.size() != 0) {
        throw std::runtime_error("사용자_이름(): 인자가 필요하지 않습니다");
    }

#ifdef _WIN32
    char username[256];
    DWORD username_len = sizeof(username);
    if (GetUserNameA(username, &username_len)) {
        return Value::createString(username);
    }
    return Value::createString("unknown");
#else
    const char* username = getenv("USER");
    if (username) {
        return Value::createString(username);
    }

    struct passwd* pwd = getpwuid(getuid());
    if (pwd) {
        return Value::createString(pwd->pw_name);
    }

    return Value::createString("unknown");
#endif
}

/**
 * @brief 호스트_이름() - 호스트 이름
 */
static Value builtin_호스트_이름(const std::vector<Value>& args)
{
    if (args.size() != 0) {
        throw std::runtime_error("호스트_이름(): 인자가 필요하지 않습니다");
    }

#ifdef _WIN32
    char hostname[256];
    DWORD hostname_len = sizeof(hostname);
    if (GetComputerNameA(hostname, &hostname_len)) {
        return Value::createString(hostname);
    }
    return Value::createString("unknown");
#else
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return Value::createString(hostname);
    }
    return Value::createString("unknown");
#endif
}

/**
 * @brief 프로세스_종료(코드) - 프로세스 종료
 */
static Value builtin_프로세스_종료(const std::vector<Value>& args)
{
    if (args.size() != 1) {
        throw std::runtime_error("프로세스_종료(코드): 1개의 인자가 필요합니다");
    }
    if (!args[0].isInteger()) {
        throw std::runtime_error("프로세스_종료(코드): 정수 타입이어야 합니다");
    }

    int code = static_cast<int>(args[0].asInteger());
    std::exit(code);

    // Never reached
    return Value::createNull();
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

    // ========== 파일 I/O ==========
    registerBuiltin("파일_읽기", builtin_파일_읽기);
    registerBuiltin("파일_쓰기", builtin_파일_쓰기);
    registerBuiltin("파일_추가", builtin_파일_추가);
    registerBuiltin("파일_존재", builtin_파일_존재);
    registerBuiltin("줄별_읽기", builtin_줄별_읽기);

    // ========== 환경변수 ==========
    registerBuiltin("환경변수_읽기", builtin_환경변수_읽기);
    registerBuiltin("환경변수_쓰기", builtin_환경변수_쓰기);
    registerBuiltin("환경변수_존재하는가", builtin_환경변수_존재하는가);
    registerBuiltin("환경변수_삭제", builtin_환경변수_삭제);

    // ========== 디렉토리 ==========
    registerBuiltin("현재_디렉토리", builtin_현재_디렉토리);
    registerBuiltin("디렉토리_변경", builtin_디렉토리_변경);
    registerBuiltin("디렉토리_생성", builtin_디렉토리_생성);
    registerBuiltin("디렉토리_삭제", builtin_디렉토리_삭제);
    registerBuiltin("디렉토리_목록", builtin_디렉토리_목록);
    registerBuiltin("디렉토리_나열", builtin_디렉토리_나열);
    registerBuiltin("디렉토리인가", builtin_디렉토리인가);
    registerBuiltin("임시_디렉토리", builtin_임시_디렉토리);

    // ========== 파일 시스템 ==========
    registerBuiltin("파일_존재하는가", builtin_파일_존재하는가);
    registerBuiltin("파일_삭제", builtin_파일_삭제);
    registerBuiltin("파일_복사", builtin_파일_복사);
    registerBuiltin("파일_이동", builtin_파일_이동);
    registerBuiltin("파일_크기", builtin_파일_크기);
    registerBuiltin("파일인가", builtin_파일인가);

    // ========== 경로 ==========
    registerBuiltin("경로_결합", builtin_경로_결합);
    registerBuiltin("파일명_추출", builtin_파일명_추출);
    registerBuiltin("확장자_추출", builtin_확장자_추출);
    registerBuiltin("절대경로", builtin_절대경로);
    registerBuiltin("경로_존재하는가", builtin_경로_존재하는가);

    // ========== 시스템 정보 ==========
    registerBuiltin("OS_이름", builtin_OS_이름);
    registerBuiltin("사용자_이름", builtin_사용자_이름);
    registerBuiltin("호스트_이름", builtin_호스트_이름);
    registerBuiltin("프로세스_종료", builtin_프로세스_종료);

    // ========== JSON 처리 ==========
    registerBuiltin("JSON_파싱", builtin_JSON_파싱);
    registerBuiltin("JSON_문자열화", builtin_JSON_문자열화);
    registerBuiltin("JSON_파일_읽기", builtin_JSON_파일_읽기);
    registerBuiltin("JSON_파일_쓰기", builtin_JSON_파일_쓰기);

    // ========== 시간/날짜 ==========
    registerBuiltin("현재_시간", builtin_현재_시간);
    registerBuiltin("현재_날짜", builtin_현재_날짜);
    registerBuiltin("시간_포맷", builtin_시간_포맷);
    registerBuiltin("타임스탬프", builtin_타임스탬프);
    registerBuiltin("슬립", builtin_슬립);
    registerBuiltin("현재_시각", builtin_현재_시각);
    registerBuiltin("현재_날짜시간", builtin_현재_날짜시간);

    // ========== 정규표현식 ==========
    registerBuiltin("정규표현식_일치", builtin_정규표현식_일치);
    registerBuiltin("정규표현식_검색", builtin_정규표현식_검색);
    registerBuiltin("정규표현식_모두_찾기", builtin_정규표현식_모두_찾기);
    registerBuiltin("정규표현식_치환", builtin_정규표현식_치환);
    registerBuiltin("정규표현식_분리", builtin_정규표현식_분리);
    registerBuiltin("이메일_검증", builtin_이메일_검증);
    registerBuiltin("URL_검증", builtin_URL_검증);
    registerBuiltin("전화번호_검증", builtin_전화번호_검증);
    registerBuiltin("정규표현식_추출", builtin_정규표현식_추출);
    registerBuiltin("정규표현식_개수", builtin_정규표현식_개수);

    // ========== 암호화 ==========
    registerBuiltin("Base64_인코딩", builtin_Base64_인코딩);
    registerBuiltin("Base64_디코딩", builtin_Base64_디코딩);
    registerBuiltin("문자열_해시", builtin_문자열_해시);
    registerBuiltin("파일_해시", builtin_파일_해시);
    registerBuiltin("해시_비교", builtin_해시_비교);
    registerBuiltin("체크섬", builtin_체크섬);
    registerBuiltin("XOR_암호화", builtin_XOR_암호화);
    registerBuiltin("XOR_복호화", builtin_XOR_복호화);
    registerBuiltin("시저_암호화", builtin_시저_암호화);
    registerBuiltin("시저_복호화", builtin_시저_복호화);
    registerBuiltin("랜덤_문자열", builtin_랜덤_문자열);
    registerBuiltin("랜덤_숫자", builtin_랜덤_숫자);
}

} // namespace evaluator
} // namespace kingsejong
