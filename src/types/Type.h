#pragma once

/**
 * @file Type.h
 * @brief KingSejong 언어 타입 시스템
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include <string>
#include <unordered_map>
#include <memory>

namespace kingsejong {
namespace types {

/**
 * @enum TypeKind
 * @brief 기본 타입 종류
 */
enum class TypeKind
{
    INTEGER,   ///< 정수 (int64_t)
    FLOAT,     ///< 실수 (double)
    STRING,    ///< 문자열 (std::string)
    BOOLEAN,   ///< 참거짓 (bool)
    NULL_TYPE, ///< 없음 (nullptr)
    ARRAY,     ///< 배열
    FUNCTION,  ///< 함수
    RANGE      ///< 범위 (1부터 10까지)
};

/**
 * @class Type
 * @brief 타입 정보를 표현하는 클래스
 *
 * KingSejong 언어의 타입 시스템을 구현합니다.
 * 한글 타입 이름을 지원하며, 기본 타입과 복합 타입을 표현할 수 있습니다.
 */
class Type
{
public:
    /**
     * @brief 생성자
     * @param kind 타입 종류
     * @param koreanName 한글 타입 이름 (선택)
     */
    explicit Type(TypeKind kind, const std::string& koreanName = "");

    /**
     * @brief 타입 종류 반환
     * @return TypeKind
     */
    TypeKind kind() const { return kind_; }

    /**
     * @brief 한글 타입 이름 반환
     * @return 한글 타입 이름 (예: "정수", "문자열")
     */
    const std::string& koreanName() const { return koreanName_; }

    /**
     * @brief 영어 타입 이름 반환
     * @return 영어 타입 이름 (예: "INTEGER", "STRING")
     */
    std::string englishName() const;

    /**
     * @brief 타입을 문자열로 변환
     * @return 타입 문자열 표현
     */
    std::string toString() const;

    /**
     * @brief 한글 타입 이름으로 빌트인 타입 가져오기
     * @param koreanName 한글 타입 이름 ("정수", "실수", "문자열", "참거짓", "없음")
     * @return Type 객체 포인터, 없으면 nullptr
     */
    static Type* getBuiltin(const std::string& koreanName);

    /**
     * @brief 빌트인 정수 타입
     * @return 정수 타입
     */
    static Type* integerType();

    /**
     * @brief 빌트인 실수 타입
     * @return 실수 타입
     */
    static Type* floatType();

    /**
     * @brief 빌트인 문자열 타입
     * @return 문자열 타입
     */
    static Type* stringType();

    /**
     * @brief 빌트인 불린 타입
     * @return 불린 타입
     */
    static Type* booleanType();

    /**
     * @brief 빌트인 Null 타입
     * @return Null 타입
     */
    static Type* nullType();

    /**
     * @brief 빌트인 범위 타입
     * @return 범위 타입
     */
    static Type* rangeType();

    /**
     * @brief 두 타입이 동일한지 확인
     * @param other 비교할 타입
     * @return 동일하면 true
     */
    bool equals(const Type* other) const;

    /**
     * @brief TypeKind를 문자열로 변환
     * @param kind TypeKind
     * @return 문자열 표현
     */
    static std::string typeKindToString(TypeKind kind);

private:
    TypeKind kind_;          ///< 타입 종류
    std::string koreanName_; ///< 한글 타입 이름

    /// 한글 이름 → 빌트인 타입 매핑
    static std::unordered_map<std::string, Type*> builtinTypes_;

    /// 빌트인 타입 초기화
    static void initBuiltinTypes();

    /// 빌트인 타입 인스턴스들
    static Type* integerType_;
    static Type* floatType_;
    static Type* stringType_;
    static Type* booleanType_;
    static Type* nullType_;
    static Type* rangeType_;
};

} // namespace types
} // namespace kingsejong
