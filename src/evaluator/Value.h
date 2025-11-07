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

namespace kingsejong {
namespace evaluator {

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
     * @brief 값 데이터 타입
     *
     * std::variant로 다양한 타입을 안전하게 저장합니다.
     */
    using ValueData = std::variant<
        std::monostate,    // 초기화되지 않은 상태
        int64_t,           // INTEGER
        double,            // FLOAT
        std::string,       // STRING
        bool,              // BOOLEAN
        std::nullptr_t     // NULL_TYPE
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
