/**
 * @file EventLoop.cpp
 * @brief KingSejong 언어의 비동기 Event Loop 구현
 * @author KingSejong Team
 * @date 2025-11-19
 *
 * Phase 7.3: Async/Await - Event Loop 구현
 */

#include "EventLoop.h"

namespace kingsejong {
namespace evaluator {

// ============================================================================
// EventLoop 구현
// ============================================================================

void EventLoop::enqueueMicrotask(std::function<void()> task)
{
    microtaskQueue_.push(std::move(task));
}

void EventLoop::enqueueTask(std::function<void()> task)
{
    taskQueue_.push(std::move(task));
}

void EventLoop::run()
{
    running_ = true;

    // 모든 microtask와 task가 처리될 때까지 반복
    while (!isEmpty())
    {
        // 1. 모든 microtask 실행
        runMicrotasks();

        // 2. task queue에서 하나 실행
        if (!taskQueue_.empty())
        {
            auto task = std::move(taskQueue_.front());
            taskQueue_.pop();
            task();

            // task 실행 후 새로 추가된 microtask 처리
            runMicrotasks();
        }
    }

    running_ = false;
}

void EventLoop::tick()
{
    // microtask queue만 실행
    runMicrotasks();
}

void EventLoop::runMicrotasks()
{
    // microtask queue가 빌 때까지 실행
    // 실행 중 새로운 microtask가 추가될 수 있으므로 while 사용
    while (!microtaskQueue_.empty())
    {
        auto task = std::move(microtaskQueue_.front());
        microtaskQueue_.pop();
        task();
    }
}

void EventLoop::clear()
{
    // 모든 큐 비우기
    while (!microtaskQueue_.empty())
    {
        microtaskQueue_.pop();
    }
    while (!taskQueue_.empty())
    {
        taskQueue_.pop();
    }
}

// ============================================================================
// 글로벌 Event Loop
// ============================================================================

EventLoop& getGlobalEventLoop()
{
    static EventLoop globalEventLoop;
    return globalEventLoop;
}

} // namespace evaluator
} // namespace kingsejong
