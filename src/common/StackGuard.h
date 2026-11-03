#pragma once

/**
 * @file StackGuard.h
 * @brief 현재 스레드의 남은 네이티브 스택을 확인한다
 */

#include <cstddef>

namespace kingsejong {
namespace common {

/**
 * @brief 현재 스레드의 스택이 거의 다 찼는지 확인한다.
 *
 * 평가기는 사용자 함수 호출마다 C++ 재귀가 깊어지므로, 끝나지 않는 재귀는 OS 스택을 넘겨
 * 프로세스가 죽는다(segfault). 호출 횟수로 막으면 스택 크기(Windows 1MB, macOS 보조 스레드 512KB,
 * Linux 8MB)와 빌드 종류(Debug, ASan)에 따라 한도가 맞지 않으므로, 실제 스택의 끝 주소를 OS 에서
 * 얻어 남은 바이트로 판단한다. 스레드마다 처음 한 번만 OS 에 묻고 이후에는 주소 비교만 한다.
 *
 * @return 남은 스택이 여유분보다 적으면 true. 스택 범위를 알 수 없는 플랫폼에서는 항상 false.
 */
bool isStackNearlyExhausted();

} // namespace common
} // namespace kingsejong
