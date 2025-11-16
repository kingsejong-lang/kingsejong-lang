#pragma once

/**
 * @file Value.h
 * @brief KingSejong 언어 런타임 값 표현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "types/Type.h"
#include <variant>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <functional>

namespace kingsejong {

// Forward declarations
namespace ast {
    class Statement;
}

namespace evaluator {

// Forward declaration
class Environment;

/**
 * @class Function
 * @brief 함수 객체
 *
 * 함수의 매개변수, 본문, 클로저 환경을 저장합니다.
 */
class Function
{
private:
    std::vector<std::string> parameters_;          ///< 매개변수 이름 리스트
    ast::Statement* body_;                         ///< 함수 본문 (BlockStatement)
    std::shared_ptr<Environment> closure_;         ///< 클로저 환경

public:
    /**
     * @brief Function 생성자
     * @param parameters 매개변수 이름 리스트
     * @param body 함수 본문
     * @param closure 클로저 환경
     */
    Function(
        std::vector<std::string> parameters,
        ast::Statement* body,
        std::shared_ptr<Environment> closure
    )
        : parameters_(std::move(parameters))
        , body_(body)
        , closure_(closure)
    {}

    const std::vector<std::string>& parameters() const { return parameters_; }
    ast::Statement* body() const { return body_; }
    std::shared_ptr<Environment> closure() const { return closure_; }
};

/**
 * @class ErrorObject
 * @brief 에러 객체
 *
 * 예외 처리 시스템에서 사용하는 에러 정보를 저장합니다.
 */
class ErrorObject
{
private:
    std::string message_;  ///< 에러 메시지
    std::string type_;     ///< 에러 타입 (예: "RuntimeError", "TypeError")

public:
    /**
     * @brief ErrorObject 생성자
     * @param message 에러 메시지
     * @param type 에러 타입 (기본값: "Error")
     */
    ErrorObject(std::string message, std::string type = "Error")
        : message_(std::move(message))
        , type_(std::move(type))
    {}

    const std::string& message() const { return message_; }
    const std::string& type() const { return type_; }
};

/**
 * @class Value
 * @brief 런타임 값을 표현하는 클래스
 *
 * std::variant를 사용하여 다양한 타입의 값을 저장합니다.
 * 타입 안전성을 보장하며, 타입 체크와 변환 기능을 제공합니다.
 */
class Value
{
public:
    /**
     * @brief 내장 함수 타입
     *
     * 내장 함수는 인자 목록을 받아서 Value를 반환하는 함수입니다.
     */
    using BuiltinFn = std::function<Value(const std::vector<Value>&)>;

    /**
     * @brief 값 데이터 타입
     *
     * std::variant로 다양한 타입을 안전하게 저장합니다.
     */
    using ValueData = std::variant<
        std::monostate,                         // 초기화되지 않은 상태
        int64_t,                                // INTEGER
        double,                                 // FLOAT
        std::string,                            // STRING
        bool,                                   // BOOLEAN
        std::nullptr_t,                         // NULL_TYPE
        std::shared_ptr<Function>,              // FUNCTION
        BuiltinFn,                              // BUILTIN_FUNCTION
        std::shared_ptr<std::vector<Value>>,    // ARRAY
        std::shared_ptr<ErrorObject>            // ERROR
    >;

private:
    types::TypeKind type_;  ///< 값의 타입
    ValueData data_;        ///< 실제 데이터

public:
    /**
     * @brief 기본 생성자
     *
     * 초기화되지 않은 값을 생성합니다.
     */
    Value();

    /**
     * @brief 정수 값 생성
     * @param val 정수 값
     * @return Value 객체
     */
    static Value createInteger(int64_t val);

    /**
     * @brief 실수 값 생성
     * @param val 실수 값
     * @return Value 객체
     */
    static Value createFloat(double val);

    /**
     * @brief 문자열 값 생성
     * @param val 문자열 값
     * @return Value 객체
     */
    static Value createString(const std::string& val);

    /**
     * @brief 불린 값 생성
     * @param val 불린 값
     * @return Value 객체
     */
    static Value createBoolean(bool val);

    /**
     * @brief Null 값 생성
     * @return Value 객체
     */
    static Value createNull();

    /**
     * @brief 함수 값 생성
     * @param func 함수 객체 (shared_ptr)
     * @return Value 객체
     */
    static Value createFunction(std::shared_ptr<Function> func);

    /**
     * @brief 내장 함수 값 생성
     * @param func 내장 함수 (BuiltinFn)
     * @return Value 객체
     */
    static Value createBuiltinFunction(BuiltinFn func);

    /**
     * @brief 배열 값 생성
     * @param elements 배열 요소들
     * @return Value 객체
     */
    static Value createArray(const std::vector<Value>& elements);

    /**
     * @brief 에러 값 생성
     * @param message 에러 메시지
     * @param type 에러 타입 (기본값: "Error")
     * @return Value 객체
     */
    static Value createError(const std::string& message, const std::string& type = "Error");

    /**
     * @brief 값의 타입 반환
     * @return TypeKind
     */
    types::TypeKind getType() const { return type_; }

    /**
     * @brief 정수 값인지 확인
     * @return 정수이면 true
     */
    bool isInteger() const { return type_ == types::TypeKind::INTEGER; }

    /**
     * @brief 실수 값인지 확인
     * @return 실수이면 true
     */
    bool isFloat() const { return type_ == types::TypeKind::FLOAT; }

    /**
     * @brief 문자열 값인지 확인
     * @return 문자열이면 true
     */
    bool isString() const { return type_ == types::TypeKind::STRING; }

    /**
     * @brief 불린 값인지 확인
     * @return 불린이면 true
     */
    bool isBoolean() const { return type_ == types::TypeKind::BOOLEAN; }

    /**
     * @brief Null 값인지 확인
     * @return Null이면 true
     */
    bool isNull() const { return type_ == types::TypeKind::NULL_TYPE; }

    /**
     * @brief 함수 값인지 확인
     * @return 함수이면 true
     */
    bool isFunction() const { return type_ == types::TypeKind::FUNCTION; }

    /**
     * @brief 내장 함수 값인지 확인
     * @return 내장 함수이면 true
     */
    bool isBuiltinFunction() const { return type_ == types::TypeKind::BUILTIN_FUNCTION; }

    /**
     * @brief 배열 값인지 확인
     * @return 배열이면 true
     */
    bool isArray() const { return type_ == types::TypeKind::ARRAY; }

    /**
     * @brief 에러 값인지 확인
     * @return 에러이면 true
     */
    bool isError() const { return type_ == types::TypeKind::ERROR; }

    /**
     * @brief 정수 값 반환
     * @return int64_t 값
     * @throws std::runtime_error 정수가 아닌 경우
     */
    int64_t asInteger() const;

    /**
     * @brief 실수 값 반환
     * @return double 값
     * @throws std::runtime_error 실수가 아닌 경우
     */
    double asFloat() const;

    /**
     * @brief 문자열 값 반환
     * @return std::string 값
     * @throws std::runtime_error 문자열이 아닌 경우
     */
    std::string asString() const;

    /**
     * @brief 불린 값 반환
     * @return bool 값
     * @throws std::runtime_error 불린이 아닌 경우
     */
    bool asBoolean() const;

    /**
     * @brief 함수 값 반환
     * @return shared_ptr<Function>
     * @throws std::runtime_error 함수가 아닌 경우
     */
    std::shared_ptr<Function> asFunction() const;

    /**
     * @brief 내장 함수 값 반환
     * @return BuiltinFn
     * @throws std::runtime_error 내장 함수가 아닌 경우
     */
    BuiltinFn asBuiltinFunction() const;

    /**
     * @brief 배열 값 반환
     * @return std::vector<Value> 참조
     * @throws std::runtime_error 배열이 아닌 경우
     */
    std::vector<Value>& asArray();
    const std::vector<Value>& asArray() const;

    /**
     * @brief 에러 값 반환
     * @return shared_ptr<ErrorObject>
     * @throws std::runtime_error 에러가 아닌 경우
     */
    std::shared_ptr<ErrorObject> asError() const;

    /**
     * @brief 값을 문자열로 변환
     * @return 문자열 표현
     */
    std::string toString() const;

    /**
     * @brief 값의 참/거짓 판별
     *
     * KingSejong 언어의 참/거짓 규칙:
     * - 거짓: false, 0, 0.0, "", null
     * - 참: 그 외 모든 값
     *
     * @return 참이면 true
     */
    bool isTruthy() const;

    /**
     * @brief 두 값이 같은지 비교
     * @param other 비교할 값
     * @return 같으면 true
     */
    bool equals(const Value& other) const;

    /**
     * @brief 값 비교 (작다)
     * @param other 비교할 값
     * @return this < other이면 true
     * @throws std::runtime_error 비교할 수 없는 타입인 경우
     */
    bool lessThan(const Value& other) const;

    /**
     * @brief 값 비교 (크다)
     * @param other 비교할 값
     * @return this > other이면 true
     * @throws std::runtime_error 비교할 수 없는 타입인 경우
     */
    bool greaterThan(const Value& other) const;
};

} // namespace evaluator
} // namespace kingsejong
