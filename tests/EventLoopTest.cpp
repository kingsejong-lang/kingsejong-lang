/**
 * @file EventLoopTest.cpp
 * @brief Event Loop 테스트
 * @author KingSejong Team
 * @date 2025-11-19
 *
 * Phase 7.3: Async/Await - Event Loop 테스트
 */

#include <gtest/gtest.h>
#include "evaluator/EventLoop.h"
#include "evaluator/Value.h"
#include <vector>

using namespace kingsejong::evaluator;

// ============================================================================
// EventLoop 기본 테스트
// ============================================================================

TEST(EventLoopTest, ShouldInitializeEmpty)
{
    EventLoop eventLoop;

    EXPECT_TRUE(eventLoop.isEmpty());
    EXPECT_FALSE(eventLoop.hasMicrotasks());
    EXPECT_FALSE(eventLoop.hasTasks());
    EXPECT_FALSE(eventLoop.isRunning());
}

TEST(EventLoopTest, ShouldEnqueueMicrotask)
{
    EventLoop eventLoop;

    eventLoop.enqueueMicrotask([]() {});

    EXPECT_TRUE(eventLoop.hasMicrotasks());
    EXPECT_EQ(eventLoop.microtaskCount(), 1);
}

TEST(EventLoopTest, ShouldEnqueueTask)
{
    EventLoop eventLoop;

    eventLoop.enqueueTask([]() {});

    EXPECT_TRUE(eventLoop.hasTasks());
    EXPECT_EQ(eventLoop.taskCount(), 1);
}

TEST(EventLoopTest, ShouldExecuteMicrotask)
{
    EventLoop eventLoop;
    bool executed = false;

    eventLoop.enqueueMicrotask([&executed]() {
        executed = true;
    });

    eventLoop.run();

    EXPECT_TRUE(executed);
    EXPECT_TRUE(eventLoop.isEmpty());
}

TEST(EventLoopTest, ShouldExecuteMultipleMicrotasks)
{
    EventLoop eventLoop;
    std::vector<int> results;

    eventLoop.enqueueMicrotask([&results]() { results.push_back(1); });
    eventLoop.enqueueMicrotask([&results]() { results.push_back(2); });
    eventLoop.enqueueMicrotask([&results]() { results.push_back(3); });

    eventLoop.run();

    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], 1);
    EXPECT_EQ(results[1], 2);
    EXPECT_EQ(results[2], 3);
}

TEST(EventLoopTest, ShouldExecuteTask)
{
    EventLoop eventLoop;
    bool executed = false;

    eventLoop.enqueueTask([&executed]() {
        executed = true;
    });

    eventLoop.run();

    EXPECT_TRUE(executed);
    EXPECT_TRUE(eventLoop.isEmpty());
}

TEST(EventLoopTest, ShouldExecuteMicrotasksBeforeTasks)
{
    EventLoop eventLoop;
    std::vector<int> results;

    // Task를 먼저 등록
    eventLoop.enqueueTask([&results]() { results.push_back(2); });
    // Microtask를 나중에 등록
    eventLoop.enqueueMicrotask([&results]() { results.push_back(1); });

    eventLoop.run();

    // Microtask가 먼저 실행되어야 함
    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0], 1);  // microtask
    EXPECT_EQ(results[1], 2);  // task
}

TEST(EventLoopTest, ShouldExecuteNewMicrotasksDuringRun)
{
    EventLoop eventLoop;
    std::vector<int> results;

    eventLoop.enqueueMicrotask([&eventLoop, &results]() {
        results.push_back(1);
        // 실행 중에 새로운 microtask 추가
        eventLoop.enqueueMicrotask([&results]() {
            results.push_back(2);
        });
    });

    eventLoop.run();

    // 새로 추가된 microtask도 실행되어야 함
    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0], 1);
    EXPECT_EQ(results[1], 2);
}

TEST(EventLoopTest, ShouldTickOnlyMicrotasks)
{
    EventLoop eventLoop;
    std::vector<int> results;

    eventLoop.enqueueMicrotask([&results]() { results.push_back(1); });
    eventLoop.enqueueTask([&results]() { results.push_back(2); });

    eventLoop.tick();  // microtask만 실행

    // Microtask만 실행됨
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], 1);

    // Task는 아직 남아있음
    EXPECT_TRUE(eventLoop.hasTasks());
}

TEST(EventLoopTest, ShouldClearQueues)
{
    EventLoop eventLoop;

    eventLoop.enqueueMicrotask([]() {});
    eventLoop.enqueueTask([]() {});

    EXPECT_FALSE(eventLoop.isEmpty());

    eventLoop.clear();

    EXPECT_TRUE(eventLoop.isEmpty());
}

// ============================================================================
// 글로벌 Event Loop 테스트
// ============================================================================

TEST(EventLoopTest, ShouldGetGlobalEventLoop)
{
    auto& loop1 = getGlobalEventLoop();
    auto& loop2 = getGlobalEventLoop();

    // 싱글톤: 같은 인스턴스
    EXPECT_EQ(&loop1, &loop2);
}

TEST(EventLoopTest, ShouldRunGlobalEventLoop)
{
    auto& eventLoop = getGlobalEventLoop();
    eventLoop.clear();  // 이전 상태 정리

    bool executed = false;
    eventLoop.enqueueMicrotask([&executed]() {
        executed = true;
    });

    eventLoop.run();

    EXPECT_TRUE(executed);
}

// ============================================================================
// Promise와 Event Loop 통합 테스트
// ============================================================================

TEST(EventLoopTest, ShouldSchedulePromiseThenCallback)
{
    auto& eventLoop = getGlobalEventLoop();
    eventLoop.clear();

    // Promise 생성 및 then 등록
    auto promise = std::make_shared<kingsejong::evaluator::Promise>();

    bool callbackExecuted = false;
    promise->then([&callbackExecuted](const kingsejong::evaluator::Value&) {
        callbackExecuted = true;
        return kingsejong::evaluator::Value::createNull();
    });

    // resolve하면 콜백이 microtask queue에 등록됨
    promise->resolve(kingsejong::evaluator::Value::createInteger(42));

    // run() 전에는 실행되지 않음
    EXPECT_FALSE(callbackExecuted);

    // Event Loop 실행
    eventLoop.run();

    // 이제 실행됨
    EXPECT_TRUE(callbackExecuted);
}

TEST(EventLoopTest, ShouldSchedulePromiseCatchCallback)
{
    auto& eventLoop = getGlobalEventLoop();
    eventLoop.clear();

    auto promise = std::make_shared<kingsejong::evaluator::Promise>();

    bool callbackExecuted = false;
    promise->catchError([&callbackExecuted](const kingsejong::evaluator::Value&) {
        callbackExecuted = true;
        return kingsejong::evaluator::Value::createNull();
    });

    // reject하면 콜백이 microtask queue에 등록됨
    promise->reject(kingsejong::evaluator::Value::createError("test error"));

    EXPECT_FALSE(callbackExecuted);

    eventLoop.run();

    EXPECT_TRUE(callbackExecuted);
}

TEST(EventLoopTest, ShouldScheduleAlreadyResolvedPromiseThen)
{
    auto& eventLoop = getGlobalEventLoop();
    eventLoop.clear();

    auto promise = std::make_shared<kingsejong::evaluator::Promise>();

    // 먼저 resolve
    promise->resolve(kingsejong::evaluator::Value::createInteger(100));

    bool callbackExecuted = false;
    // 이미 resolved된 promise에 then 등록
    promise->then([&callbackExecuted](const kingsejong::evaluator::Value&) {
        callbackExecuted = true;
        return kingsejong::evaluator::Value::createNull();
    });

    // 아직 실행되지 않음 (microtask queue에만 등록)
    EXPECT_FALSE(callbackExecuted);

    eventLoop.run();

    EXPECT_TRUE(callbackExecuted);
}
