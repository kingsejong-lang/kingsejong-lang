/**
 * @file Type.cpp
 * @brief KingSejong 언어 타입 시스템 구현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Type.h"

namespace kingsejong {
namespace types {

// Static 멤버 초기화
std::unordered_map<std::string, Type*> Type::builtinTypes_;
Type* Type::integerType_ = nullptr;
Type* Type::floatType_ = nullptr;
Type* Type::stringType_ = nullptr;
Type* Type::booleanType_ = nullptr;
Type* Type::nullType_ = nullptr;
Type* Type::rangeType_ = nullptr;

// ============================================================================
// 생성자 및 기본 메서드
// ============================================================================

Type::Type(TypeKind kind, const std::string& koreanName)
    : kind_(kind), koreanName_(koreanName)
{
}

std::string Type::englishName() const
{
    return typeKindToString(kind_);
}

std::string Type::toString() const
{
    if (!koreanName_.empty())
    {
        return koreanName_ + " (" + englishName() + ")";
    }
    return englishName();
}

bool Type::equals(const Type* other) const
{
    if (other == nullptr)
    {
        return false;
    }
    return kind_ == other->kind_;
}

// ============================================================================
// 빌트인 타입 시스템
// ============================================================================

void Type::initBuiltinTypes()
{
    if (integerType_ != nullptr)
    {
        return; // 이미 초기화됨
    }

    // 빌트인 타입은 프로세스가 끝날 때까지 살아야 하므로 정적 객체로 둔다.
    // (heap 에 new 로 만들고 지우지 않으면 LeakSanitizer 가 누수로 보고한다.)
    static Type integerType(TypeKind::INTEGER, "정수");
    static Type floatType(TypeKind::FLOAT, "실수");
    static Type stringType(TypeKind::STRING, "문자열");
    static Type booleanType(TypeKind::BOOLEAN, "논리");
    static Type nullType(TypeKind::NULL_TYPE, "없음");
    static Type rangeType(TypeKind::RANGE, "범위");
    static Type arrayType(TypeKind::ARRAY, "배열");
    static Type dictionaryType(TypeKind::DICTIONARY, "딕셔너리");

    integerType_ = &integerType;
    floatType_ = &floatType;
    stringType_ = &stringType;
    booleanType_ = &booleanType;
    nullType_ = &nullType;
    rangeType_ = &rangeType;

    // 한글 이름 매핑
    builtinTypes_["정수"] = integerType_;
    builtinTypes_["실수"] = floatType_;
    builtinTypes_["문자열"] = stringType_;
    builtinTypes_["논리"] = booleanType_;
    builtinTypes_["참거짓"] = booleanType_;  // 별칭
    builtinTypes_["불린"] = booleanType_;  // 별칭
    builtinTypes_["없음"] = nullType_;
    builtinTypes_["범위"] = rangeType_;
    builtinTypes_["배열"] = &arrayType;
    builtinTypes_["딕셔너리"] = &dictionaryType;  // (Phase 7.2)
}

Type* Type::getBuiltin(const std::string& koreanName)
{
    initBuiltinTypes();

    auto it = builtinTypes_.find(koreanName);
    if (it != builtinTypes_.end())
    {
        return it->second;
    }
    return nullptr;
}

Type* Type::integerType()
{
    initBuiltinTypes();
    return integerType_;
}

Type* Type::floatType()
{
    initBuiltinTypes();
    return floatType_;
}

Type* Type::stringType()
{
    initBuiltinTypes();
    return stringType_;
}

Type* Type::booleanType()
{
    initBuiltinTypes();
    return booleanType_;
}

Type* Type::nullType()
{
    initBuiltinTypes();
    return nullType_;
}

Type* Type::rangeType()
{
    initBuiltinTypes();
    return rangeType_;
}

// ============================================================================
// 유틸리티
// ============================================================================

std::string Type::typeKindToString(TypeKind kind)
{
    switch (kind)
    {
        case TypeKind::INTEGER:
            return "INTEGER";
        case TypeKind::FLOAT:
            return "FLOAT";
        case TypeKind::STRING:
            return "STRING";
        case TypeKind::BOOLEAN:
            return "BOOLEAN";
        case TypeKind::NULL_TYPE:
            return "NULL";
        case TypeKind::ARRAY:
            return "ARRAY";
        case TypeKind::DICTIONARY:
            return "DICTIONARY";
        case TypeKind::FUNCTION:
            return "FUNCTION";
        case TypeKind::BUILTIN_FUNCTION:
            return "BUILTIN_FUNCTION";
        case TypeKind::RANGE:
            return "RANGE";
        case TypeKind::ERROR:
            return "ERROR";
        case TypeKind::CLASS:
            return "CLASS";
        case TypeKind::PROMISE:
            return "PROMISE";
        default:
            return "UNKNOWN";
    }
}

} // namespace types
} // namespace kingsejong
