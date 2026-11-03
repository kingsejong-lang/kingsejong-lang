/**
 * @file StackGuard.cpp
 * @brief 현재 스레드의 남은 네이티브 스택 확인 구현
 */

#include "StackGuard.h"

#include <algorithm>
#include <cstdint>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#elif defined(__EMSCRIPTEN__)
#include <emscripten/stack.h>
#elif defined(__APPLE__)
#include <pthread.h>
#elif defined(__linux__)
#include <pthread.h>
#endif

namespace kingsejong {
namespace common {

namespace {

struct StackRange
{
    std::uintptr_t low = 0;   ///< 스택이 자라 갈 수 있는 끝 (낮은 주소). 0 이면 알 수 없음
    std::size_t margin = 0;   ///< 이만큼 남으면 다 찬 것으로 본다
};

/// 내장 함수(정규식, 오류 메시지 포맷 등)가 재귀 없이 한 번에 쓰는 스택을 남겨 둔다
std::size_t marginFor(std::size_t size)
{
    constexpr std::size_t kMin = 32 * 1024;
    constexpr std::size_t kMax = 256 * 1024;
    return std::clamp(size / 8, kMin, kMax);
}

StackRange queryStackRange()
{
    StackRange range;
#if defined(_WIN32)
    ULONG_PTR low = 0;
    ULONG_PTR high = 0;
    GetCurrentThreadStackLimits(&low, &high);
    range.low = static_cast<std::uintptr_t>(low);
    range.margin = marginFor(static_cast<std::size_t>(high - low));
#elif defined(__EMSCRIPTEN__)
    range.low = static_cast<std::uintptr_t>(emscripten_stack_get_end());
    range.margin = marginFor(static_cast<std::size_t>(emscripten_stack_get_base() - emscripten_stack_get_end()));
#elif defined(__APPLE__)
    pthread_t self = pthread_self();
    auto high = reinterpret_cast<std::uintptr_t>(pthread_get_stackaddr_np(self));
    std::size_t size = pthread_get_stacksize_np(self);
    range.low = high - size;
    range.margin = marginFor(size);
#elif defined(__linux__)
    pthread_attr_t attr;
    if (pthread_getattr_np(pthread_self(), &attr) == 0)
    {
        void* addr = nullptr;
        std::size_t size = 0;
        if (pthread_attr_getstack(&attr, &addr, &size) == 0)
        {
            range.low = reinterpret_cast<std::uintptr_t>(addr);
            range.margin = marginFor(size);
        }
        pthread_attr_destroy(&attr);
    }
#endif
    return range;
}

} // namespace

bool isStackNearlyExhausted()
{
    thread_local const StackRange range = queryStackRange();
    if (range.low == 0)
    {
        return false;
    }
    // 지원하는 모든 플랫폼에서 스택은 낮은 주소 쪽으로 자란다.
    // 지역 변수 주소 대신 프레임 주소를 쓴다. ASan 의 use-after-return 검사는 지역 변수를 힙의
    // 가짜 스택에 두므로 그 주소로는 실제 스택 위치를 알 수 없다.
#if defined(_MSC_VER)
    auto current = reinterpret_cast<std::uintptr_t>(_AddressOfReturnAddress());
#else
    auto current = reinterpret_cast<std::uintptr_t>(__builtin_frame_address(0));
#endif
    return current < range.low + range.margin;
}

} // namespace common
} // namespace kingsejong
