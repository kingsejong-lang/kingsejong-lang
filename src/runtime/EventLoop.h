#pragma once

/**
 * @file EventLoop.h
 * @brief KingSejong 언어의 비동기 Event Loop
 * @author KingSejong Team
 * @date 2025-11-19
 *
 * Phase 7.3: Async/Await - Event Loop 구현
 * JavaScript의 Event Loop를 모델로 한 비동기 태스크 스케줄링
 */

#include <queue>
#include <functional>
#include <memory>

namespace kingsejong {
namespace evaluator {

/**
 * @class EventLoop
 * @brief 비동기 태스크를 스케줄링하고 실행하는 이벤트 루프
 *
 * JavaScript의 Event Loop를 모델로 하여:
 * - Microtask Queue: Promise 콜백 (then, catch, finally)
 * - Task Queue: setTimeout, setInterval 등 (향후 구현)
 */
class EventLoop
{
public:
    /**
     * @brief EventLoop 생성자
     */
    EventLoop() = default;

    /**
     * @brief Microtask 추가 (Promise 콜백용)
     * @param task 실행할 태스크 함수
     *
     * Promise의 then/catch 콜백은 microtask로 스케줄됩니다.
     * Microtask는 현재 실행이 완료된 후, 다음 task 전에 실행됩니다.
     */
    void enqueueMicrotask(std::function<void()> task);

    /**
     * @brief Task 추가 (setTimeout 등)
     * @param task 실행할 태스크 함수
     *
     * 일반 태스크는 microtask queue가 비워진 후 실행됩니다.
     */
    void enqueueTask(std::function<void()> task);

    /**
     * @brief Event Loop 실행
     *
     * 모든 microtask와 task를 실행합니다:
     * 1. 모든 microtask 실행 (새로 추가된 것 포함)
     * 2. 하나의 task 실행
     * 3. 반복
     */
    void run();

    /**
     * @brief 단일 틱 실행
     *
     * microtask queue만 비웁니다.
     * task queue는 건드리지 않습니다.
     */
    void tick();

    /**
     * @brief Microtask queue가 비어있는지 확인
     * @return 비어있으면 true
     */
    bool hasMicrotasks() const { return !microtaskQueue_.empty(); }

    /**
     * @brief Task queue가 비어있는지 확인
     * @return 비어있으면 true
     */
    bool hasTasks() const { return !taskQueue_.empty(); }

    /**
     * @brief 모든 큐가 비어있는지 확인
     * @return 모든 큐가 비어있으면 true
     */
    bool isEmpty() const { return !hasMicrotasks() && !hasTasks(); }

    /**
     * @brief Event Loop가 실행 중인지 확인
     * @return 실행 중이면 true
     */
    bool isRunning() const { return running_; }

    /**
     * @brief Microtask queue 크기 반환
     * @return microtask 개수
     */
    size_t microtaskCount() const { return microtaskQueue_.size(); }

    /**
     * @brief Task queue 크기 반환
     * @return task 개수
     */
    size_t taskCount() const { return taskQueue_.size(); }

    /**
     * @brief 모든 큐 비우기
     */
    void clear();

private:
    std::queue<std::function<void()>> microtaskQueue_;  ///< Microtask Queue
    std::queue<std::function<void()>> taskQueue_;       ///< Task Queue
    bool running_ = false;                              ///< 실행 중 플래그

    /**
     * @brief Microtask queue 실행
     *
     * 큐가 빌 때까지 모든 microtask를 실행합니다.
     * 실행 중 새로운 microtask가 추가되면 그것도 실행합니다.
     */
    void runMicrotasks();
};

/**
 * @brief 글로벌 Event Loop 인스턴스 가져오기
 * @return EventLoop 참조
 *
 * 싱글톤 패턴으로 전역에서 접근 가능한 Event Loop를 제공합니다.
 */
EventLoop& getGlobalEventLoop();

} // namespace evaluator
} // namespace kingsejong
