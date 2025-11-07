/**
 * @file TypeTest.cpp
 * @brief Type 시스템 테스트
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include <gtest/gtest.h>
#include "types/Type.h"

using namespace kingsejong::types;

/**
 * @test TypeKind enum 값 테스트
 */
TEST(TypeTest, ShouldHaveCorrectTypeKinds)
{
    // TypeKind enum이 올바르게 정의되어 있는지 확인
    EXPECT_EQ(static_cast<int>(TypeKind::INTEGER), 0);
    EXPECT_NE(TypeKind::INTEGER, TypeKind::FLOAT);
    EXPECT_NE(TypeKind::STRING, TypeKind::BOOLEAN);
}

/**
 * @test 빌트인 정수 타입 생성 및 확인
 */
TEST(TypeTest, ShouldCreateIntegerType)
{
    Type* intType = Type::integerType();

    ASSERT_NE(intType, nullptr);
    EXPECT_EQ(intType->kind(), TypeKind::INTEGER);
    EXPECT_EQ(intType->koreanName(), "정수");
    EXPECT_EQ(intType->englishName(), "INTEGER");
}

/**
 * @test 빌트인 실수 타입 생성 및 확인
 */
TEST(TypeTest, ShouldCreateFloatType)
{
    Type* floatType = Type::floatType();

    ASSERT_NE(floatType, nullptr);
    EXPECT_EQ(floatType->kind(), TypeKind::FLOAT);
    EXPECT_EQ(floatType->koreanName(), "실수");
    EXPECT_EQ(floatType->englishName(), "FLOAT");
}

/**
 * @test 빌트인 문자열 타입 생성 및 확인
 */
TEST(TypeTest, ShouldCreateStringType)
{
    Type* stringType = Type::stringType();

    ASSERT_NE(stringType, nullptr);
    EXPECT_EQ(stringType->kind(), TypeKind::STRING);
    EXPECT_EQ(stringType->koreanName(), "문자열");
    EXPECT_EQ(stringType->englishName(), "STRING");
}

/**
 * @test 빌트인 불린 타입 생성 및 확인
 */
TEST(TypeTest, ShouldCreateBooleanType)
{
    Type* boolType = Type::booleanType();

    ASSERT_NE(boolType, nullptr);
    EXPECT_EQ(boolType->kind(), TypeKind::BOOLEAN);
    EXPECT_EQ(boolType->koreanName(), "참거짓");
    EXPECT_EQ(boolType->englishName(), "BOOLEAN");
}

/**
 * @test 빌트인 Null 타입 생성 및 확인
 */
TEST(TypeTest, ShouldCreateNullType)
{
    Type* nullType = Type::nullType();

    ASSERT_NE(nullType, nullptr);
    EXPECT_EQ(nullType->kind(), TypeKind::NULL_TYPE);
    EXPECT_EQ(nullType->koreanName(), "없음");
    EXPECT_EQ(nullType->englishName(), "NULL");
}

/**
 * @test 빌트인 범위 타입 생성 및 확인
 */
TEST(TypeTest, ShouldCreateRangeType)
{
    Type* rangeType = Type::rangeType();

    ASSERT_NE(rangeType, nullptr);
    EXPECT_EQ(rangeType->kind(), TypeKind::RANGE);
    EXPECT_EQ(rangeType->koreanName(), "범위");
    EXPECT_EQ(rangeType->englishName(), "RANGE");
}

/**
 * @test 한글 이름으로 빌트인 타입 가져오기 - "정수"
 */
TEST(TypeTest, ShouldGetBuiltinTypeByKoreanName_Integer)
{
    Type* type = Type::getBuiltin("정수");

    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->kind(), TypeKind::INTEGER);
    EXPECT_EQ(type->koreanName(), "정수");
}

/**
 * @test 한글 이름으로 빌트인 타입 가져오기 - "실수"
 */
TEST(TypeTest, ShouldGetBuiltinTypeByKoreanName_Float)
{
    Type* type = Type::getBuiltin("실수");

    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->kind(), TypeKind::FLOAT);
    EXPECT_EQ(type->koreanName(), "실수");
}

/**
 * @test 한글 이름으로 빌트인 타입 가져오기 - "문자열"
 */
TEST(TypeTest, ShouldGetBuiltinTypeByKoreanName_String)
{
    Type* type = Type::getBuiltin("문자열");

    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->kind(), TypeKind::STRING);
    EXPECT_EQ(type->koreanName(), "문자열");
}

/**
 * @test 한글 이름으로 빌트인 타입 가져오기 - "참거짓"
 */
TEST(TypeTest, ShouldGetBuiltinTypeByKoreanName_Boolean)
{
    Type* type = Type::getBuiltin("참거짓");

    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->kind(), TypeKind::BOOLEAN);
    EXPECT_EQ(type->koreanName(), "참거짓");
}

/**
 * @test 한글 이름으로 빌트인 타입 가져오기 - "불린" (별칭)
 */
TEST(TypeTest, ShouldGetBuiltinTypeByKoreanName_BooleanAlias)
{
    Type* type = Type::getBuiltin("불린");

    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->kind(), TypeKind::BOOLEAN);
}

/**
 * @test 한글 이름으로 빌트인 타입 가져오기 - "없음"
 */
TEST(TypeTest, ShouldGetBuiltinTypeByKoreanName_Null)
{
    Type* type = Type::getBuiltin("없음");

    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->kind(), TypeKind::NULL_TYPE);
    EXPECT_EQ(type->koreanName(), "없음");
}

/**
 * @test 존재하지 않는 타입 이름으로 조회 시 nullptr 반환
 */
TEST(TypeTest, ShouldReturnNullForInvalidKoreanName)
{
    Type* type = Type::getBuiltin("존재하지않는타입");

    EXPECT_EQ(type, nullptr);
}

/**
 * @test 타입 동일성 비교
 */
TEST(TypeTest, ShouldCompareTypesForEquality)
{
    Type* int1 = Type::integerType();
    Type* int2 = Type::integerType();
    Type* float1 = Type::floatType();

    EXPECT_TRUE(int1->equals(int2));
    EXPECT_TRUE(int2->equals(int1));
    EXPECT_FALSE(int1->equals(float1));
    EXPECT_FALSE(float1->equals(int1));
    EXPECT_FALSE(int1->equals(nullptr));
}

/**
 * @test TypeKind를 문자열로 변환
 */
TEST(TypeTest, ShouldConvertTypeKindToString)
{
    EXPECT_EQ(Type::typeKindToString(TypeKind::INTEGER), "INTEGER");
    EXPECT_EQ(Type::typeKindToString(TypeKind::FLOAT), "FLOAT");
    EXPECT_EQ(Type::typeKindToString(TypeKind::STRING), "STRING");
    EXPECT_EQ(Type::typeKindToString(TypeKind::BOOLEAN), "BOOLEAN");
    EXPECT_EQ(Type::typeKindToString(TypeKind::NULL_TYPE), "NULL");
    EXPECT_EQ(Type::typeKindToString(TypeKind::ARRAY), "ARRAY");
    EXPECT_EQ(Type::typeKindToString(TypeKind::FUNCTION), "FUNCTION");
    EXPECT_EQ(Type::typeKindToString(TypeKind::RANGE), "RANGE");
}

/**
 * @test toString() 메서드 - 한글 이름 포함
 */
TEST(TypeTest, ShouldConvertTypeToString)
{
    Type* intType = Type::integerType();
    std::string str = intType->toString();

    EXPECT_TRUE(str.find("정수") != std::string::npos);
    EXPECT_TRUE(str.find("INTEGER") != std::string::npos);
}

/**
 * @test 싱글톤 패턴 - 동일한 타입은 같은 인스턴스 반환
 */
TEST(TypeTest, ShouldReturnSameInstanceForBuiltinTypes)
{
    Type* int1 = Type::integerType();
    Type* int2 = Type::integerType();
    Type* int3 = Type::getBuiltin("정수");

    // 모두 같은 인스턴스를 가리켜야 함
    EXPECT_EQ(int1, int2);
    EXPECT_EQ(int2, int3);
}
