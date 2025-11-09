/**
 * @file TypeCheckerTest.cpp
 * @brief 타입 검사 시스템 테스트
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include <gtest/gtest.h>
#include "../src/types/TypeChecker.h"
#include "../src/types/Type.h"
#include "../src/evaluator/Value.h"
#include "../src/error/Error.h"

using namespace kingsejong;
using namespace kingsejong::types;
using namespace kingsejong::evaluator;
using namespace kingsejong::error;

// ============================================================================
// getValueType 테스트
// ============================================================================

TEST(TypeCheckerTest, ShouldGetIntegerType)
{
    Value val = Value::createInteger(42);
    EXPECT_EQ(TypeChecker::getValueType(val), TypeKind::INTEGER);
}

TEST(TypeCheckerTest, ShouldGetFloatType)
{
    Value val = Value::createFloat(3.14);
    EXPECT_EQ(TypeChecker::getValueType(val), TypeKind::FLOAT);
}

TEST(TypeCheckerTest, ShouldGetStringType)
{
    Value val = Value::createString("안녕");
    EXPECT_EQ(TypeChecker::getValueType(val), TypeKind::STRING);
}

TEST(TypeCheckerTest, ShouldGetBooleanType)
{
    Value val = Value::createBoolean(true);
    EXPECT_EQ(TypeChecker::getValueType(val), TypeKind::BOOLEAN);
}

TEST(TypeCheckerTest, ShouldGetNullType)
{
    Value val = Value::createNull();
    EXPECT_EQ(TypeChecker::getValueType(val), TypeKind::NULL_TYPE);
}

TEST(TypeCheckerTest, ShouldGetArrayType)
{
    Value val = Value::createArray({Value::createInteger(1), Value::createInteger(2)});
    EXPECT_EQ(TypeChecker::getValueType(val), TypeKind::ARRAY);
}

// ============================================================================
// checkType 테스트
// ============================================================================

TEST(TypeCheckerTest, ShouldPassTypeCheckForMatchingType)
{
    Value val = Value::createInteger(42);
    Type* expectedType = Type::integerType();

    EXPECT_NO_THROW(TypeChecker::checkType(val, expectedType));
}

TEST(TypeCheckerTest, ShouldThrowForMismatchedType)
{
    Value val = Value::createString("안녕");
    Type* expectedType = Type::integerType();

    EXPECT_THROW({
        TypeChecker::checkType(val, expectedType, "x");
    }, TypeError);
}

TEST(TypeCheckerTest, ShouldIncludeVariableNameInError)
{
    Value val = Value::createFloat(3.14);
    Type* expectedType = Type::stringType();

    try
    {
        TypeChecker::checkType(val, expectedType, "이름");
        FAIL() << "Should have thrown TypeError";
    }
    catch (const TypeError& e)
    {
        std::string msg = e.what();
        EXPECT_TRUE(msg.find("이름") != std::string::npos);
        EXPECT_TRUE(msg.find("문자열") != std::string::npos);
    }
}

// ============================================================================
// checkBinaryOperation 산술 연산 테스트
// ============================================================================

TEST(TypeCheckerTest, ShouldAllowIntegerAddition)
{
    Value left = Value::createInteger(10);
    Value right = Value::createInteger(20);

    EXPECT_NO_THROW(TypeChecker::checkBinaryOperation(left, "+", right));
}

TEST(TypeCheckerTest, ShouldAllowFloatAddition)
{
    Value left = Value::createFloat(10.5);
    Value right = Value::createFloat(20.5);

    EXPECT_NO_THROW(TypeChecker::checkBinaryOperation(left, "+", right));
}

TEST(TypeCheckerTest, ShouldAllowMixedNumberAddition)
{
    Value left = Value::createInteger(10);
    Value right = Value::createFloat(20.5);

    EXPECT_NO_THROW(TypeChecker::checkBinaryOperation(left, "+", right));
}

TEST(TypeCheckerTest, ShouldAllowStringConcatenation)
{
    Value left = Value::createString("안녕");
    Value right = Value::createString("하세요");

    EXPECT_NO_THROW(TypeChecker::checkBinaryOperation(left, "+", right));
}

TEST(TypeCheckerTest, ShouldThrowForInvalidAddition)
{
    Value left = Value::createInteger(10);
    Value right = Value::createString("안녕");

    EXPECT_THROW({
        TypeChecker::checkBinaryOperation(left, "+", right);
    }, TypeError);
}

TEST(TypeCheckerTest, ShouldThrowForStringSubtraction)
{
    Value left = Value::createString("안녕");
    Value right = Value::createString("하세요");

    EXPECT_THROW({
        TypeChecker::checkBinaryOperation(left, "-", right);
    }, TypeError);
}

// ============================================================================
// checkBinaryOperation 비교 연산 테스트
// ============================================================================

TEST(TypeCheckerTest, ShouldAllowIntegerComparison)
{
    Value left = Value::createInteger(10);
    Value right = Value::createInteger(20);

    EXPECT_NO_THROW(TypeChecker::checkBinaryOperation(left, "<", right));
    EXPECT_NO_THROW(TypeChecker::checkBinaryOperation(left, "==", right));
}

TEST(TypeCheckerTest, ShouldAllowMixedNumberComparison)
{
    Value left = Value::createInteger(10);
    Value right = Value::createFloat(20.5);

    EXPECT_NO_THROW(TypeChecker::checkBinaryOperation(left, "<", right));
}

TEST(TypeCheckerTest, ShouldAllowStringComparison)
{
    Value left = Value::createString("a");
    Value right = Value::createString("b");

    EXPECT_NO_THROW(TypeChecker::checkBinaryOperation(left, "<", right));
}

TEST(TypeCheckerTest, ShouldThrowForIncompatibleComparison)
{
    Value left = Value::createInteger(10);
    Value right = Value::createString("안녕");

    EXPECT_THROW({
        TypeChecker::checkBinaryOperation(left, "==", right);
    }, TypeError);
}

// ============================================================================
// checkBinaryOperation 논리 연산 테스트
// ============================================================================

TEST(TypeCheckerTest, ShouldAllowLogicalOperationOnAnyType)
{
    Value left = Value::createInteger(10);
    Value right = Value::createString("안녕");

    // 논리 연산자는 모든 타입에 사용 가능 (truthy/falsy)
    EXPECT_NO_THROW(TypeChecker::checkBinaryOperation(left, "&&", right));
    EXPECT_NO_THROW(TypeChecker::checkBinaryOperation(left, "||", right));
}

// ============================================================================
// checkUnaryOperation 테스트
// ============================================================================

TEST(TypeCheckerTest, ShouldAllowNegationOnInteger)
{
    Value val = Value::createInteger(10);
    EXPECT_NO_THROW(TypeChecker::checkUnaryOperation("-", val));
}

TEST(TypeCheckerTest, ShouldAllowNegationOnFloat)
{
    Value val = Value::createFloat(10.5);
    EXPECT_NO_THROW(TypeChecker::checkUnaryOperation("-", val));
}

TEST(TypeCheckerTest, ShouldThrowForNegationOnString)
{
    Value val = Value::createString("안녕");
    EXPECT_THROW({
        TypeChecker::checkUnaryOperation("-", val);
    }, TypeError);
}

TEST(TypeCheckerTest, ShouldAllowLogicalNotOnAnyType)
{
    Value val = Value::createString("안녕");
    EXPECT_NO_THROW(TypeChecker::checkUnaryOperation("!", val));
}

// ============================================================================
// checkArrayIndex 테스트
// ============================================================================

TEST(TypeCheckerTest, ShouldAllowIntegerArrayIndex)
{
    Value array = Value::createArray({Value::createInteger(1)});
    Value index = Value::createInteger(0);

    EXPECT_NO_THROW(TypeChecker::checkArrayIndex(array, index));
}

TEST(TypeCheckerTest, ShouldThrowForNonArrayIndexing)
{
    Value notArray = Value::createInteger(42);
    Value index = Value::createInteger(0);

    EXPECT_THROW({
        TypeChecker::checkArrayIndex(notArray, index);
    }, TypeError);
}

TEST(TypeCheckerTest, ShouldThrowForNonIntegerIndex)
{
    Value array = Value::createArray({Value::createInteger(1)});
    Value index = Value::createString("0");

    EXPECT_THROW({
        TypeChecker::checkArrayIndex(array, index);
    }, TypeError);
}

// ============================================================================
// checkCallable 테스트
// ============================================================================

TEST(TypeCheckerTest, ShouldAllowFunctionCall)
{
    // 함수 Value 생성 (간단히 빌트인 함수로 테스트)
    Value func = Value::createBuiltinFunction([](const std::vector<Value>&) { return Value::createNull(); });

    EXPECT_NO_THROW(TypeChecker::checkCallable(func));
}

TEST(TypeCheckerTest, ShouldThrowForNonFunctionCall)
{
    Value notFunc = Value::createInteger(42);

    EXPECT_THROW({
        TypeChecker::checkCallable(notFunc);
    }, TypeError);
}

// ============================================================================
// 에러 메시지 테스트
// ============================================================================

TEST(TypeCheckerTest, ShouldProvideDetailedErrorMessage)
{
    Value left = Value::createBoolean(true);
    Value right = Value::createInteger(10);

    try
    {
        TypeChecker::checkBinaryOperation(left, "+", right);
        FAIL() << "Should have thrown TypeError";
    }
    catch (const TypeError& e)
    {
        std::string msg = e.what();
        // 에러 메시지에 연산자와 타입 정보가 포함되어 있는지 확인
        EXPECT_TRUE(msg.find("+") != std::string::npos);
        EXPECT_TRUE(msg.length() > 10);  // 상세한 메시지인지 확인
    }
}
