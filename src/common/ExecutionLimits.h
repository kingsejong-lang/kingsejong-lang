#pragma once

/**
 * @file ExecutionLimits.h
 * @brief 평가기·VM 이 무한 루프를 끊는 실행 한도
 *
 * 평가기와 VM 은 만들어질 때 current() 를 읽는다. 함수 본문·모듈·콜백을 돌리는 평가기도 새로 만들어지므로
 * 프로세스 전체에 한 값을 둔다. 기본값은 플레이그라운드·REPL·테스트처럼 끝나지 않는 코드를 끊어야 하는 곳의 한도다.
 * CLI 는 파일을 실행할 때 --sandbox·--untrusted 가 아니면 unlimited() 로 바꾼다(다른 언어처럼 끝날 때까지 돈다).
 */

#include <chrono>
#include <cstddef>
#include <limits>

namespace kingsejong {
namespace common {

struct ExecutionLimits
{
    size_t maxEvaluations = 10000000;        ///< 평가기 한 인스턴스의 최대 평가 횟수
    size_t maxInstructions = 100000000;      ///< VM 의 최대 명령어 수
    std::chrono::milliseconds maxTime{5000}; ///< 최대 실행 시간

    /// 지금 만들어지는 평가기·VM 이 쓸 한도. 실행을 시작하기 전에 바꾼다
    static ExecutionLimits& current()
    {
        static ExecutionLimits limits;
        return limits;
    }

    /// 한도 없음 (횟수·시간 모두 최댓값이라 검사가 걸리지 않는다)
    static ExecutionLimits unlimited()
    {
        return {std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max(),
                std::chrono::milliseconds::max()};
    }
};

} // namespace common
} // namespace kingsejong
