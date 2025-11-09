/**
 * @file Builtin.cpp
 * @brief KingSejong 언어 내장 함수 구현
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include "Builtin.h"
#include <iostream>
#include <stdexcept>

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

            if ((c & 0x80) == 0)
            {
                // 1바이트 문자 (ASCII)
                i += 1;
            }
            else if ((c & 0xE0) == 0xC0)
            {
                // 2바이트 UTF-8
                i += 2;
            }
            else if ((c & 0xF0) == 0xE0)
            {
                // 3바이트 UTF-8 (한글 등)
                i += 3;
            }
            else if ((c & 0xF8) == 0xF0)
            {
                // 4바이트 UTF-8
                i += 4;
            }
            else
            {
                // fallback
                i += 1;
            }
            count++;
        }

        return Value::createInteger(count);
    }

    // 배열은 아직 구현되지 않았으므로 에러 처리
    throw std::runtime_error("길이() 함수는 문자열 또는 배열 타입이어야 합니다");
}

// ============================================================================
// 내장 함수 등록
// ============================================================================

void Builtin::registerAllBuiltins()
{
    registerBuiltin("출력", builtin_출력);
    registerBuiltin("타입", builtin_타입);
    registerBuiltin("길이", builtin_길이);
}

} // namespace evaluator
} // namespace kingsejong
