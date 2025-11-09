#pragma once

/**
 * @file TypeChecker.h
 * @brief KingSejong 언어 타입 검사 시스템
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include "Type.h"
#include "../evaluator/Value.h"
#include "../error/Error.h"
#include <string>

namespace kingsejong {
namespace types {

/**
 * @class TypeChecker
 * @brief 타입 검사 및 추론을 수행하는 클래스
 *
 * Value의 타입을 검사하고, 연산의 타입 호환성을 확인합니다.
 */
class TypeChecker
{
public:
    /**
     * @brief Value에서 TypeKind 추출
     * @param value 검사할 값
     * @return TypeKind
     */
    static TypeKind getValueType(const evaluator::Value& value)
    {
        if (value.isInteger()) return TypeKind::INTEGER;
        if (value.isFloat()) return TypeKind::FLOAT;
        if (value.isString()) return TypeKind::STRING;
        if (value.isBoolean()) return TypeKind::BOOLEAN;
        if (value.isNull()) return TypeKind::NULL_TYPE;
        if (value.isArray()) return TypeKind::ARRAY;
        if (value.isFunction()) return TypeKind::FUNCTION;
        if (value.isBuiltinFunction()) return TypeKind::BUILTIN_FUNCTION;

        throw error::TypeError("알 수 없는 타입입니다");
    }

    /**
     * @brief Value의 타입이 예상 타입과 일치하는지 검사
     * @param value 검사할 값
     * @param expectedType 예상 타입
     * @param varName 변수 이름 (에러 메시지용)
     * @throws TypeError 타입이 일치하지 않으면
     */
    static void checkType(const evaluator::Value& value, const Type* expectedType, const std::string& varName = "")
    {
        TypeKind valueType = getValueType(value);
        TypeKind expectedKind = expectedType->kind();

        if (valueType != expectedKind)
        {
            std::string message;
            if (!varName.empty())
            {
                message = "변수 '" + varName + "'의 타입이 일치하지 않습니다: " +
                          "예상 " + expectedType->koreanName() + ", " +
                          "실제 " + Type::typeKindToString(valueType);
            }
            else
            {
                message = "타입이 일치하지 않습니다: 예상 " + expectedType->koreanName() + ", " +
                          "실제 " + Type::typeKindToString(valueType);
            }
            throw error::TypeError(message);
        }
    }

    /**
     * @brief 이항 연산자의 타입 호환성 검사
     * @param left 왼쪽 피연산자
     * @param op 연산자
     * @param right 오른쪽 피연산자
     * @throws TypeError 타입이 호환되지 않으면
     */
    static void checkBinaryOperation(const evaluator::Value& left, const std::string& op, const evaluator::Value& right)
    {
        TypeKind leftType = getValueType(left);
        TypeKind rightType = getValueType(right);

        // 산술 연산자 (+, -, *, /, %)
        if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%")
        {
            // 정수 + 정수 = OK
            if (leftType == TypeKind::INTEGER && rightType == TypeKind::INTEGER)
            {
                return;
            }

            // 실수 연산 (정수와 실수 혼합 가능)
            if ((leftType == TypeKind::INTEGER || leftType == TypeKind::FLOAT) &&
                (rightType == TypeKind::INTEGER || rightType == TypeKind::FLOAT))
            {
                return;
            }

            // 문자열 연결 (+ 연산자만)
            if (op == "+" && leftType == TypeKind::STRING && rightType == TypeKind::STRING)
            {
                return;
            }

            // 타입 오류
            throw error::TypeError(
                "연산자 '" + op + "'는 " +
                Type::typeKindToString(leftType) + "와 " +
                Type::typeKindToString(rightType) + " 타입에 사용할 수 없습니다"
            );
        }

        // 비교 연산자 (==, !=, <, >, <=, >=)
        if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
        {
            // 같은 타입끼리만 비교 가능 (단, 정수와 실수는 예외)
            if (leftType == rightType)
            {
                return;
            }

            // 정수와 실수 비교는 허용
            if ((leftType == TypeKind::INTEGER || leftType == TypeKind::FLOAT) &&
                (rightType == TypeKind::INTEGER || rightType == TypeKind::FLOAT))
            {
                return;
            }

            // 타입 오류
            throw error::TypeError(
                "연산자 '" + op + "'는 " +
                Type::typeKindToString(leftType) + "와 " +
                Type::typeKindToString(rightType) + " 타입을 비교할 수 없습니다"
            );
        }

        // 논리 연산자 (&&, ||)
        if (op == "&&" || op == "||")
        {
            // 논리 연산자는 모든 타입에 사용 가능 (truthy/falsy 변환)
            return;
        }
    }

    /**
     * @brief 단항 연산자의 타입 호환성 검사
     * @param op 연산자
     * @param operand 피연산자
     * @throws TypeError 타입이 호환되지 않으면
     */
    static void checkUnaryOperation(const std::string& op, const evaluator::Value& operand)
    {
        TypeKind operandType = getValueType(operand);

        if (op == "-")
        {
            // 음수 연산은 숫자에만 적용 가능
            if (operandType != TypeKind::INTEGER && operandType != TypeKind::FLOAT)
            {
                throw error::TypeError(
                    "단항 연산자 '-'는 " + Type::typeKindToString(operandType) + " 타입에 사용할 수 없습니다"
                );
            }
        }
        else if (op == "!")
        {
            // 논리 부정은 모든 타입에 사용 가능 (truthy/falsy 변환)
            return;
        }
    }

    /**
     * @brief 배열 인덱스 타입 검사
     * @param array 배열
     * @param index 인덱스
     * @throws TypeError 타입이 호환되지 않으면
     */
    static void checkArrayIndex(const evaluator::Value& array, const evaluator::Value& index)
    {
        TypeKind arrayType = getValueType(array);
        TypeKind indexType = getValueType(index);

        if (arrayType != TypeKind::ARRAY)
        {
            throw error::TypeError(
                "인덱스 접근은 배열에만 가능합니다: " + Type::typeKindToString(arrayType)
            );
        }

        if (indexType != TypeKind::INTEGER)
        {
            throw error::TypeError(
                "배열 인덱스는 정수여야 합니다: " + Type::typeKindToString(indexType)
            );
        }
    }

    /**
     * @brief 함수 호출 가능 여부 검사
     * @param func 함수 값
     * @throws TypeError 함수가 아니면
     */
    static void checkCallable(const evaluator::Value& func)
    {
        TypeKind funcType = getValueType(func);

        if (funcType != TypeKind::FUNCTION && funcType != TypeKind::BUILTIN_FUNCTION)
        {
            throw error::TypeError(
                "함수가 아닌 값을 호출할 수 없습니다: " + Type::typeKindToString(funcType)
            );
        }
    }
};

} // namespace types
} // namespace kingsejong
