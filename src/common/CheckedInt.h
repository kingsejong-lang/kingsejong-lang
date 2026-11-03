#pragma once

/**
 * @file CheckedInt.h
 * @brief 넘침을 알려 주는 64비트 정수 연산과, 2의 보수로 감싸 도는 연산
 *
 * 언어의 정수는 int64_t 다. C++ 에서 부호 있는 정수가 넘치면 정의되지 않은 동작이라
 * 컴파일러·플랫폼에 따라 결과가 달라진다. 그래서 기본 산술은 넘침을 확인해 실행 오류로 만들고,
 * 해시처럼 감싸 돌기를 의도한 계산은 부호 없는 정수로 계산해 동작이 정의된 감싸 돌기 함수를 쓴다.
 */

#include <cstdint>
#include <limits>

namespace kingsejong {
namespace common {

/// a + b 를 result 에 넣는다. 넘치면 false (result 는 정해지지 않음).
inline bool checkedAdd(int64_t a, int64_t b, int64_t& result)
{
#if defined(__GNUC__) || defined(__clang__)
    return !__builtin_add_overflow(a, b, &result);
#else
    if ((b > 0 && a > std::numeric_limits<int64_t>::max() - b) ||
        (b < 0 && a < std::numeric_limits<int64_t>::min() - b))
    {
        return false;
    }
    result = a + b;
    return true;
#endif
}

/// a - b 를 result 에 넣는다. 넘치면 false.
inline bool checkedSub(int64_t a, int64_t b, int64_t& result)
{
#if defined(__GNUC__) || defined(__clang__)
    return !__builtin_sub_overflow(a, b, &result);
#else
    if ((b < 0 && a > std::numeric_limits<int64_t>::max() + b) ||
        (b > 0 && a < std::numeric_limits<int64_t>::min() + b))
    {
        return false;
    }
    result = a - b;
    return true;
#endif
}

/// a * b 를 result 에 넣는다. 넘치면 false.
inline bool checkedMul(int64_t a, int64_t b, int64_t& result)
{
#if defined(__GNUC__) || defined(__clang__)
    return !__builtin_mul_overflow(a, b, &result);
#else
    constexpr int64_t kMax = std::numeric_limits<int64_t>::max();
    constexpr int64_t kMin = std::numeric_limits<int64_t>::min();
    if (a > 0 ? (b > 0 ? a > kMax / b : b < kMin / a)
              : (b > 0 ? a < kMin / b : (a != 0 && b < kMax / a)))
    {
        return false;
    }
    result = a * b;
    return true;
#endif
}

/// -a 가 넘치는지 (정수 최솟값만 넘친다). 나눗셈 a / -1, a % -1 도 같은 경우에만 넘친다.
inline bool negationOverflows(int64_t a)
{
    return a == std::numeric_limits<int64_t>::min();
}

// 감싸 도는 연산: 부호 없는 정수의 산술은 2^64 로 나눈 나머지로 정의되어 있고,
// C++20 부터 부호 없는 값을 부호 있는 값으로 바꾸는 것도 2의 보수로 정의되어 있다.
inline int64_t wrappingAdd(int64_t a, int64_t b)
{
    return static_cast<int64_t>(static_cast<uint64_t>(a) + static_cast<uint64_t>(b));
}

inline int64_t wrappingSub(int64_t a, int64_t b)
{
    return static_cast<int64_t>(static_cast<uint64_t>(a) - static_cast<uint64_t>(b));
}

inline int64_t wrappingMul(int64_t a, int64_t b)
{
    return static_cast<int64_t>(static_cast<uint64_t>(a) * static_cast<uint64_t>(b));
}

/// 실수 값이 int64_t 로 정확히 바뀔 수 있는 범위 안인지 (NaN 은 false). 범위 밖 변환은 정의되지 않은 동작이다.
inline bool fitsInInt64(double value)
{
    // 2^63 은 double 로 정확히 표현된다. [-2^63, 2^63) 이 int64_t 범위다.
    return value >= -9223372036854775808.0 && value < 9223372036854775808.0;
}

} // namespace common
} // namespace kingsejong
