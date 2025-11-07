/**
 * @file ValueTest.cpp
 * @brief Value 시스템 테스트
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include <gtest/gtest.h>
#include "evaluator/Value.h"

using namespace kingsejong::evaluator;
using namespace kingsejong::types;

/**
 * @test 정수 값 생성 및 확인
 */
TEST(ValueTest, ShouldCreateIntegerValue)
{
    Value val = Value::createInteger(42);

    EXPECT_TRUE(val.isInteger());
    EXPECT_FALSE(val.isFloat());
    EXPECT_FALSE(val.isString());
    EXPECT_FALSE(val.isBoolean());
    EXPECT_FALSE(val.isNull());

    EXPECT_EQ(val.getType(), TypeKind::INTEGER);
    EXPECT_EQ(val.asInteger(), 42);
    EXPECT_EQ(val.toString(), "42");
}

/**
 * @test 실수 값 생성 및 확인
 */
TEST(ValueTest, ShouldCreateFloatValue)
{
    Value val = Value::createFloat(3.14);

    EXPECT_FALSE(val.isInteger());
    EXPECT_TRUE(val.isFloat());
    EXPECT_FALSE(val.isString());
    EXPECT_FALSE(val.isBoolean());
    EXPECT_FALSE(val.isNull());

    EXPECT_EQ(val.getType(), TypeKind::FLOAT);
    EXPECT_DOUBLE_EQ(val.asFloat(), 3.14);
}

/**
 * @test 문자열 값 생성 및 확인
 */
TEST(ValueTest, ShouldCreateStringValue)
{
    Value val = Value::createString("안녕하세요");

    EXPECT_FALSE(val.isInteger());
    EXPECT_FALSE(val.isFloat());
    EXPECT_TRUE(val.isString());
    EXPECT_FALSE(val.isBoolean());
    EXPECT_FALSE(val.isNull());

    EXPECT_EQ(val.getType(), TypeKind::STRING);
    EXPECT_EQ(val.asString(), "안녕하세요");
    EXPECT_EQ(val.toString(), "안녕하세요");
}

/**
 * @test 불린 값 생성 및 확인 - 참
 */
TEST(ValueTest, ShouldCreateBooleanValueTrue)
{
    Value val = Value::createBoolean(true);

    EXPECT_FALSE(val.isInteger());
    EXPECT_FALSE(val.isFloat());
    EXPECT_FALSE(val.isString());
    EXPECT_TRUE(val.isBoolean());
    EXPECT_FALSE(val.isNull());

    EXPECT_EQ(val.getType(), TypeKind::BOOLEAN);
    EXPECT_TRUE(val.asBoolean());
    EXPECT_EQ(val.toString(), "참");
}

/**
 * @test 불린 값 생성 및 확인 - 거짓
 */
TEST(ValueTest, ShouldCreateBooleanValueFalse)
{
    Value val = Value::createBoolean(false);

    EXPECT_TRUE(val.isBoolean());
    EXPECT_FALSE(val.asBoolean());
    EXPECT_EQ(val.toString(), "거짓");
}

/**
 * @test Null 값 생성 및 확인
 */
TEST(ValueTest, ShouldCreateNullValue)
{
    Value val = Value::createNull();

    EXPECT_FALSE(val.isInteger());
    EXPECT_FALSE(val.isFloat());
    EXPECT_FALSE(val.isString());
    EXPECT_FALSE(val.isBoolean());
    EXPECT_TRUE(val.isNull());

    EXPECT_EQ(val.getType(), TypeKind::NULL_TYPE);
    EXPECT_EQ(val.toString(), "없음");
}

/**
 * @test 기본 생성자는 Null 값 생성
 */
TEST(ValueTest, DefaultConstructorCreatesNull)
{
    Value val;

    EXPECT_TRUE(val.isNull());
    EXPECT_EQ(val.getType(), TypeKind::NULL_TYPE);
}

/**
 * @test 잘못된 타입으로 값 접근 시 예외 발생 - asInteger
 */
TEST(ValueTest, ShouldThrowWhenAccessingIntegerFromNonInteger)
{
    Value val = Value::createString("문자열");

    EXPECT_THROW(val.asInteger(), std::runtime_error);
}

/**
 * @test 잘못된 타입으로 값 접근 시 예외 발생 - asFloat
 */
TEST(ValueTest, ShouldThrowWhenAccessingFloatFromNonFloat)
{
    Value val = Value::createInteger(42);

    EXPECT_THROW(val.asFloat(), std::runtime_error);
}

/**
 * @test 잘못된 타입으로 값 접근 시 예외 발생 - asString
 */
TEST(ValueTest, ShouldThrowWhenAccessingStringFromNonString)
{
    Value val = Value::createBoolean(true);

    EXPECT_THROW(val.asString(), std::runtime_error);
}

/**
 * @test 잘못된 타입으로 값 접근 시 예외 발생 - asBoolean
 */
TEST(ValueTest, ShouldThrowWhenAccessingBooleanFromNonBoolean)
{
    Value val = Value::createNull();

    EXPECT_THROW(val.asBoolean(), std::runtime_error);
}

/**
 * @test 정수 값의 참/거짓 판별
 */
TEST(ValueTest, IntegerTruthiness)
{
    Value zero = Value::createInteger(0);
    Value nonZero = Value::createInteger(42);
    Value negative = Value::createInteger(-10);

    EXPECT_FALSE(zero.isTruthy());
    EXPECT_TRUE(nonZero.isTruthy());
    EXPECT_TRUE(negative.isTruthy());
}

/**
 * @test 실수 값의 참/거짓 판별
 */
TEST(ValueTest, FloatTruthiness)
{
    Value zero = Value::createFloat(0.0);
    Value nonZero = Value::createFloat(3.14);
    Value negative = Value::createFloat(-2.5);

    EXPECT_FALSE(zero.isTruthy());
    EXPECT_TRUE(nonZero.isTruthy());
    EXPECT_TRUE(negative.isTruthy());
}

/**
 * @test 문자열 값의 참/거짓 판별
 */
TEST(ValueTest, StringTruthiness)
{
    Value empty = Value::createString("");
    Value nonEmpty = Value::createString("안녕");

    EXPECT_FALSE(empty.isTruthy());
    EXPECT_TRUE(nonEmpty.isTruthy());
}

/**
 * @test 불린 값의 참/거짓 판별
 */
TEST(ValueTest, BooleanTruthiness)
{
    Value trueVal = Value::createBoolean(true);
    Value falseVal = Value::createBoolean(false);

    EXPECT_TRUE(trueVal.isTruthy());
    EXPECT_FALSE(falseVal.isTruthy());
}

/**
 * @test Null 값의 참/거짓 판별
 */
TEST(ValueTest, NullTruthiness)
{
    Value nullVal = Value::createNull();

    EXPECT_FALSE(nullVal.isTruthy());
}

/**
 * @test 정수 값 동등 비교
 */
TEST(ValueTest, IntegerEquality)
{
    Value val1 = Value::createInteger(42);
    Value val2 = Value::createInteger(42);
    Value val3 = Value::createInteger(10);

    EXPECT_TRUE(val1.equals(val2));
    EXPECT_FALSE(val1.equals(val3));
}

/**
 * @test 실수 값 동등 비교
 */
TEST(ValueTest, FloatEquality)
{
    Value val1 = Value::createFloat(3.14);
    Value val2 = Value::createFloat(3.14);
    Value val3 = Value::createFloat(2.71);

    EXPECT_TRUE(val1.equals(val2));
    EXPECT_FALSE(val1.equals(val3));
}

/**
 * @test 문자열 값 동등 비교
 */
TEST(ValueTest, StringEquality)
{
    Value val1 = Value::createString("안녕");
    Value val2 = Value::createString("안녕");
    Value val3 = Value::createString("하이");

    EXPECT_TRUE(val1.equals(val2));
    EXPECT_FALSE(val1.equals(val3));
}

/**
 * @test 불린 값 동등 비교
 */
TEST(ValueTest, BooleanEquality)
{
    Value val1 = Value::createBoolean(true);
    Value val2 = Value::createBoolean(true);
    Value val3 = Value::createBoolean(false);

    EXPECT_TRUE(val1.equals(val2));
    EXPECT_FALSE(val1.equals(val3));
}

/**
 * @test Null 값 동등 비교
 */
TEST(ValueTest, NullEquality)
{
    Value val1 = Value::createNull();
    Value val2 = Value::createNull();

    EXPECT_TRUE(val1.equals(val2));
}

/**
 * @test 다른 타입 간 동등 비교는 false
 */
TEST(ValueTest, DifferentTypesAreNotEqual)
{
    Value intVal = Value::createInteger(42);
    Value floatVal = Value::createFloat(42.0);
    Value stringVal = Value::createString("42");

    EXPECT_FALSE(intVal.equals(floatVal));
    EXPECT_FALSE(intVal.equals(stringVal));
    EXPECT_FALSE(floatVal.equals(stringVal));
}

/**
 * @test 정수 값 작다 비교
 */
TEST(ValueTest, IntegerLessThan)
{
    Value val1 = Value::createInteger(10);
    Value val2 = Value::createInteger(20);

    EXPECT_TRUE(val1.lessThan(val2));
    EXPECT_FALSE(val2.lessThan(val1));
    EXPECT_FALSE(val1.lessThan(val1));
}

/**
 * @test 실수 값 작다 비교
 */
TEST(ValueTest, FloatLessThan)
{
    Value val1 = Value::createFloat(3.14);
    Value val2 = Value::createFloat(2.71);

    EXPECT_FALSE(val1.lessThan(val2));
    EXPECT_TRUE(val2.lessThan(val1));
}

/**
 * @test 문자열 값 작다 비교 (사전순)
 */
TEST(ValueTest, StringLessThan)
{
    Value val1 = Value::createString("apple");
    Value val2 = Value::createString("banana");

    EXPECT_TRUE(val1.lessThan(val2));
    EXPECT_FALSE(val2.lessThan(val1));
}

/**
 * @test 정수 값 크다 비교
 */
TEST(ValueTest, IntegerGreaterThan)
{
    Value val1 = Value::createInteger(20);
    Value val2 = Value::createInteger(10);

    EXPECT_TRUE(val1.greaterThan(val2));
    EXPECT_FALSE(val2.greaterThan(val1));
}

/**
 * @test 실수 값 크다 비교
 */
TEST(ValueTest, FloatGreaterThan)
{
    Value val1 = Value::createFloat(3.14);
    Value val2 = Value::createFloat(2.71);

    EXPECT_TRUE(val1.greaterThan(val2));
    EXPECT_FALSE(val2.greaterThan(val1));
}

/**
 * @test 문자열 값 크다 비교 (사전순)
 */
TEST(ValueTest, StringGreaterThan)
{
    Value val1 = Value::createString("banana");
    Value val2 = Value::createString("apple");

    EXPECT_TRUE(val1.greaterThan(val2));
    EXPECT_FALSE(val2.greaterThan(val1));
}

/**
 * @test 다른 타입 간 비교는 예외 발생 - lessThan
 */
TEST(ValueTest, ShouldThrowWhenComparingDifferentTypesLessThan)
{
    Value intVal = Value::createInteger(10);
    Value floatVal = Value::createFloat(10.0);

    EXPECT_THROW(intVal.lessThan(floatVal), std::runtime_error);
}

/**
 * @test 다른 타입 간 비교는 예외 발생 - greaterThan
 */
TEST(ValueTest, ShouldThrowWhenComparingDifferentTypesGreaterThan)
{
    Value intVal = Value::createInteger(10);
    Value stringVal = Value::createString("10");

    EXPECT_THROW(intVal.greaterThan(stringVal), std::runtime_error);
}

/**
 * @test 비교 불가능한 타입 비교 시 예외 발생 - 불린
 */
TEST(ValueTest, ShouldThrowWhenComparingBooleans)
{
    Value val1 = Value::createBoolean(true);
    Value val2 = Value::createBoolean(false);

    EXPECT_THROW(val1.lessThan(val2), std::runtime_error);
    EXPECT_THROW(val1.greaterThan(val2), std::runtime_error);
}

/**
 * @test 비교 불가능한 타입 비교 시 예외 발생 - Null
 */
TEST(ValueTest, ShouldThrowWhenComparingNulls)
{
    Value val1 = Value::createNull();
    Value val2 = Value::createNull();

    EXPECT_THROW(val1.lessThan(val2), std::runtime_error);
    EXPECT_THROW(val1.greaterThan(val2), std::runtime_error);
}

/**
 * @test 음수 정수 값
 */
TEST(ValueTest, NegativeInteger)
{
    Value val = Value::createInteger(-42);

    EXPECT_TRUE(val.isInteger());
    EXPECT_EQ(val.asInteger(), -42);
    EXPECT_EQ(val.toString(), "-42");
}

/**
 * @test 음수 실수 값
 */
TEST(ValueTest, NegativeFloat)
{
    Value val = Value::createFloat(-3.14);

    EXPECT_TRUE(val.isFloat());
    EXPECT_DOUBLE_EQ(val.asFloat(), -3.14);
}

/**
 * @test 빈 문자열
 */
TEST(ValueTest, EmptyString)
{
    Value val = Value::createString("");

    EXPECT_TRUE(val.isString());
    EXPECT_EQ(val.asString(), "");
    EXPECT_EQ(val.toString(), "");
    EXPECT_FALSE(val.isTruthy());
}

/**
 * @test 큰 정수 값
 */
TEST(ValueTest, LargeInteger)
{
    Value val = Value::createInteger(9223372036854775807LL);  // INT64_MAX

    EXPECT_TRUE(val.isInteger());
    EXPECT_EQ(val.asInteger(), 9223372036854775807LL);
}

/**
 * @test 작은 정수 값
 */
TEST(ValueTest, SmallInteger)
{
    Value val = Value::createInteger(-9223372036854775807LL - 1);  // INT64_MIN

    EXPECT_TRUE(val.isInteger());
    EXPECT_EQ(val.asInteger(), -9223372036854775807LL - 1);
}
