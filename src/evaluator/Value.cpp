/**
 * @file Value.cpp
 * @brief KingSejong 언어 런타임 값 구현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Value.h"
#include <sstream>
#include <cmath>

namespace kingsejong {
namespace evaluator {

// ============================================================================
// 생성자
// ============================================================================

Value::Value()
    : type_(types::TypeKind::NULL_TYPE)
    , data_(nullptr)
{
}

// ============================================================================
// 정적 생성 함수
// ============================================================================

Value Value::createInteger(int64_t val)
{
    Value v;
    v.type_ = types::TypeKind::INTEGER;
    v.data_ = val;
    return v;
}

Value Value::createFloat(double val)
{
    Value v;
    v.type_ = types::TypeKind::FLOAT;
    v.data_ = val;
    return v;
}

Value Value::createString(const std::string& val)
{
    Value v;
    v.type_ = types::TypeKind::STRING;
    v.data_ = val;
    return v;
}

Value Value::createBoolean(bool val)
{
    Value v;
    v.type_ = types::TypeKind::BOOLEAN;
    v.data_ = val;
    return v;
}

Value Value::createNull()
{
    Value v;
    v.type_ = types::TypeKind::NULL_TYPE;
    v.data_ = nullptr;
    return v;
}

Value Value::createFunction(std::shared_ptr<Function> func)
{
    Value v;
    v.type_ = types::TypeKind::FUNCTION;
    v.data_ = func;
    return v;
}

Value Value::createBuiltinFunction(Value::BuiltinFn func)
{
    Value v;
    v.type_ = types::TypeKind::BUILTIN_FUNCTION;
    v.data_ = func;
    return v;
}

// ============================================================================
// 타입 변환 및 접근
// ============================================================================

int64_t Value::asInteger() const
{
    if (!isInteger())
    {
        throw std::runtime_error("Value is not an integer");
    }
    return std::get<int64_t>(data_);
}

double Value::asFloat() const
{
    if (!isFloat())
    {
        throw std::runtime_error("Value is not a float");
    }
    return std::get<double>(data_);
}

std::string Value::asString() const
{
    if (!isString())
    {
        throw std::runtime_error("Value is not a string");
    }
    return std::get<std::string>(data_);
}

bool Value::asBoolean() const
{
    if (!isBoolean())
    {
        throw std::runtime_error("Value is not a boolean");
    }
    return std::get<bool>(data_);
}

std::shared_ptr<Function> Value::asFunction() const
{
    if (!isFunction())
    {
        throw std::runtime_error("Value is not a function");
    }
    return std::get<std::shared_ptr<Function>>(data_);
}

Value::BuiltinFn Value::asBuiltinFunction() const
{
    if (!isBuiltinFunction())
    {
        throw std::runtime_error("Value is not a builtin function");
    }
    return std::get<Value::BuiltinFn>(data_);
}

// ============================================================================
// 문자열 변환
// ============================================================================

std::string Value::toString() const
{
    switch (type_)
    {
        case types::TypeKind::INTEGER:
            return std::to_string(std::get<int64_t>(data_));

        case types::TypeKind::FLOAT:
        {
            std::ostringstream oss;
            oss << std::get<double>(data_);
            return oss.str();
        }

        case types::TypeKind::STRING:
            return std::get<std::string>(data_);

        case types::TypeKind::BOOLEAN:
            return std::get<bool>(data_) ? "참" : "거짓";

        case types::TypeKind::NULL_TYPE:
            return "없음";

        case types::TypeKind::FUNCTION:
        {
            auto func = std::get<std::shared_ptr<Function>>(data_);
            return "함수(" + std::to_string(func->parameters().size()) + " 매개변수)";
        }

        case types::TypeKind::BUILTIN_FUNCTION:
            return "[내장함수]";

        default:
            return "<unknown value>";
    }
}

// ============================================================================
// 참/거짓 판별
// ============================================================================

bool Value::isTruthy() const
{
    switch (type_)
    {
        case types::TypeKind::BOOLEAN:
            return std::get<bool>(data_);

        case types::TypeKind::NULL_TYPE:
            return false;

        case types::TypeKind::INTEGER:
            return std::get<int64_t>(data_) != 0;

        case types::TypeKind::FLOAT:
            return std::get<double>(data_) != 0.0;

        case types::TypeKind::STRING:
            return !std::get<std::string>(data_).empty();

        default:
            return true;
    }
}

// ============================================================================
// 값 비교
// ============================================================================

bool Value::equals(const Value& other) const
{
    // 타입이 다르면 false
    if (type_ != other.type_)
    {
        return false;
    }

    switch (type_)
    {
        case types::TypeKind::INTEGER:
            return std::get<int64_t>(data_) == std::get<int64_t>(other.data_);

        case types::TypeKind::FLOAT:
        {
            // 부동소수점 비교 시 epsilon 사용
            double diff = std::abs(std::get<double>(data_) - std::get<double>(other.data_));
            return diff < 1e-9;
        }

        case types::TypeKind::STRING:
            return std::get<std::string>(data_) == std::get<std::string>(other.data_);

        case types::TypeKind::BOOLEAN:
            return std::get<bool>(data_) == std::get<bool>(other.data_);

        case types::TypeKind::NULL_TYPE:
            return true;  // null == null

        default:
            return false;
    }
}

bool Value::lessThan(const Value& other) const
{
    // 타입이 같아야 비교 가능
    if (type_ != other.type_)
    {
        throw std::runtime_error("Cannot compare values of different types");
    }

    switch (type_)
    {
        case types::TypeKind::INTEGER:
            return std::get<int64_t>(data_) < std::get<int64_t>(other.data_);

        case types::TypeKind::FLOAT:
            return std::get<double>(data_) < std::get<double>(other.data_);

        case types::TypeKind::STRING:
            return std::get<std::string>(data_) < std::get<std::string>(other.data_);

        default:
            throw std::runtime_error("Type does not support less-than comparison");
    }
}

bool Value::greaterThan(const Value& other) const
{
    // 타입이 같아야 비교 가능
    if (type_ != other.type_)
    {
        throw std::runtime_error("Cannot compare values of different types");
    }

    switch (type_)
    {
        case types::TypeKind::INTEGER:
            return std::get<int64_t>(data_) > std::get<int64_t>(other.data_);

        case types::TypeKind::FLOAT:
            return std::get<double>(data_) > std::get<double>(other.data_);

        case types::TypeKind::STRING:
            return std::get<std::string>(data_) > std::get<std::string>(other.data_);

        default:
            throw std::runtime_error("Type does not support greater-than comparison");
    }
}

} // namespace evaluator
} // namespace kingsejong
