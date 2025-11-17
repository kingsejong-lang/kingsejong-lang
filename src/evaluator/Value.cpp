/**
 * @file Value.cpp
 * @brief KingSejong 언어 런타임 값 구현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Value.h"
#include "../error/Error.h"
#include "../types/Type.h"
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

ClassInstance::ClassInstance(std::shared_ptr<ClassDefinition> classDef)
    : classDef_(std::move(classDef))
{
    // 필드 초기화 (모두 null로)
    for (const auto& fieldName : classDef_->fieldNames())
    {
        fields_[fieldName] = Value::createNull();
    }
}

Value ClassInstance::getField(const std::string& fieldName) const
{
    auto it = fields_.find(fieldName);
    if (it != fields_.end())
    {
        return it->second;
    }
    throw std::runtime_error("필드 '" + fieldName + "'이(가) 존재하지 않습니다.");
}

void ClassInstance::setField(const std::string& fieldName, const Value& value)
{
    if (fields_.find(fieldName) != fields_.end())
    {
        fields_[fieldName] = value;
    }
    else
    {
        throw std::runtime_error("필드 '" + fieldName + "'이(가) 존재하지 않습니다.");
    }
}

const std::unordered_map<std::string, Value>& ClassInstance::fields() const
{
    return fields_;
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

Value Value::createArray(const std::vector<Value>& elements)
{
    Value v;
    v.type_ = types::TypeKind::ARRAY;
    v.data_ = std::make_shared<std::vector<Value>>(elements);
    return v;
}

Value Value::createError(const std::string& message, const std::string& type)
{
    Value v;
    v.type_ = types::TypeKind::ERROR;
    v.data_ = std::make_shared<ErrorObject>(message, type);
    return v;
}

Value Value::createClassInstance(std::shared_ptr<ClassInstance> instance)
{
    Value v;
    v.type_ = types::TypeKind::CLASS;
    v.data_ = instance;
    return v;
}

// ============================================================================
// 타입 변환 및 접근
// ============================================================================

int64_t Value::asInteger() const
{
    if (!isInteger())
    {
        throw error::TypeError("값이 정수 타입이 아닙니다. 실제 타입: " + types::Type::typeKindToString(type_));
    }
    return std::get<int64_t>(data_);
}

double Value::asFloat() const
{
    if (!isFloat())
    {
        throw error::TypeError("값이 실수 타입이 아닙니다. 실제 타입: " + types::Type::typeKindToString(type_));
    }
    return std::get<double>(data_);
}

std::string Value::asString() const
{
    if (!isString())
    {
        throw error::TypeError("값이 문자열 타입이 아닙니다. 실제 타입: " + types::Type::typeKindToString(type_));
    }
    return std::get<std::string>(data_);
}

bool Value::asBoolean() const
{
    if (!isBoolean())
    {
        throw error::TypeError("값이 논리 타입이 아닙니다. 실제 타입: " + types::Type::typeKindToString(type_));
    }
    return std::get<bool>(data_);
}

std::shared_ptr<Function> Value::asFunction() const
{
    if (!isFunction())
    {
        throw error::TypeError("값이 함수 타입이 아닙니다. 실제 타입: " + types::Type::typeKindToString(type_));
    }
    return std::get<std::shared_ptr<Function>>(data_);
}

Value::BuiltinFn Value::asBuiltinFunction() const
{
    if (!isBuiltinFunction())
    {
        throw error::TypeError("값이 내장 함수 타입이 아닙니다. 실제 타입: " + types::Type::typeKindToString(type_));
    }
    return std::get<Value::BuiltinFn>(data_);
}

std::vector<Value>& Value::asArray()
{
    if (!isArray())
    {
        throw error::TypeError("값이 배열 타입이 아닙니다. 실제 타입: " + types::Type::typeKindToString(type_));
    }
    return *std::get<std::shared_ptr<std::vector<Value>>>(data_);
}

const std::vector<Value>& Value::asArray() const
{
    if (!isArray())
    {
        throw error::TypeError("값이 배열 타입이 아닙니다. 실제 타입: " + types::Type::typeKindToString(type_));
    }
    return *std::get<std::shared_ptr<std::vector<Value>>>(data_);
}

std::shared_ptr<ErrorObject> Value::asError() const
{
    if (!isError())
    {
        throw std::runtime_error("값이 에러 타입이 아닙니다. 실제 타입: " + types::Type::typeKindToString(type_));
    }
    return std::get<std::shared_ptr<ErrorObject>>(data_);
}

std::shared_ptr<ClassInstance> Value::asClassInstance() const
{
    if (!isClassInstance())
    {
        throw error::TypeError("값이 클래스 인스턴스 타입이 아닙니다. 실제 타입: " + types::Type::typeKindToString(type_));
    }
    return std::get<std::shared_ptr<ClassInstance>>(data_);
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

        case types::TypeKind::ARRAY:
        {
            auto arr = std::get<std::shared_ptr<std::vector<Value>>>(data_);
            std::string result = "[";
            for (size_t i = 0; i < arr->size(); ++i)
            {
                if (i > 0) result += ", ";
                result += (*arr)[i].toString();
            }
            result += "]";
            return result;
        }

        case types::TypeKind::ERROR:
        {
            auto err = std::get<std::shared_ptr<ErrorObject>>(data_);
            return err->type() + ": " + err->message();
        }

        case types::TypeKind::CLASS:
        {
            auto instance = std::get<std::shared_ptr<ClassInstance>>(data_);
            return instance->classDef()->className() + " 인스턴스";
        }

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

        case types::TypeKind::ARRAY:
        {
            auto arr = std::get<std::shared_ptr<std::vector<Value>>>(data_);
            return !arr->empty();
        }

        case types::TypeKind::ERROR:
            return false;  // 에러는 항상 거짓

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

        case types::TypeKind::ERROR:
        {
            auto err1 = std::get<std::shared_ptr<ErrorObject>>(data_);
            auto err2 = std::get<std::shared_ptr<ErrorObject>>(other.data_);
            return err1->message() == err2->message() && err1->type() == err2->type();
        }

        default:
            return false;
    }
}

bool Value::lessThan(const Value& other) const
{
    // 정수와 실수 간 비교 지원
    if (isInteger() && other.isFloat())
    {
        return static_cast<double>(asInteger()) < other.asFloat();
    }
    if (isFloat() && other.isInteger())
    {
        return asFloat() < static_cast<double>(other.asInteger());
    }

    // 타입이 같아야 비교 가능
    if (type_ != other.type_)
    {
        throw error::TypeError("서로 다른 타입의 값을 비교할 수 없습니다: " +
                               types::Type::typeKindToString(type_) + " < " +
                               types::Type::typeKindToString(other.type_));
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
            throw error::TypeError("이 타입은 크기 비교를 지원하지 않습니다: " +
                                   types::Type::typeKindToString(type_));
    }
}

bool Value::greaterThan(const Value& other) const
{
    // 정수와 실수 간 비교 지원
    if (isInteger() && other.isFloat())
    {
        return static_cast<double>(asInteger()) > other.asFloat();
    }
    if (isFloat() && other.isInteger())
    {
        return asFloat() > static_cast<double>(other.asInteger());
    }

    // 타입이 같아야 비교 가능
    if (type_ != other.type_)
    {
        throw error::TypeError("서로 다른 타입의 값을 비교할 수 없습니다: " +
                               types::Type::typeKindToString(type_) + " > " +
                               types::Type::typeKindToString(other.type_));
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
            throw error::TypeError("이 타입은 크기 비교를 지원하지 않습니다: " +
                                   types::Type::typeKindToString(type_));
    }
}

} // namespace evaluator
} // namespace kingsejong
