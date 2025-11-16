#pragma once

/**
 * @file VM.h
 * @brief 스택 기반 가상 머신
 * @author KingSejong Team
 * @date 2025-11-10
 *
 * 바이트코드를 실행하는 스택 기반 VM입니다.
 */

#include "Chunk.h"
#include "evaluator/Value.h"
#include "evaluator/Environment.h"
#include <vector>
#include <memory>
#include <chrono>
#include <csignal>

namespace kingsejong {
namespace bytecode {

/**
 * @enum VMResult
 * @brief VM 실행 결과
 */
enum class VMResult {
    OK,                 ///< 성공
    HALT,               ///< 정상 종료 (HALT 명령)
    COMPILE_ERROR,      ///< 컴파일 에러
    RUNTIME_ERROR       ///< 런타임 에러
};

/**
 * @struct CallFrame
 * @brief 함수 호출 프레임
 */
struct CallFrame {
    size_t returnAddress;      ///< 반환 주소
    size_t stackBase;          ///< 스택 베이스 포인터
};

/**
 * @class VM
 * @brief 가상 머신
 *
 * 바이트코드를 실행하는 스택 기반 VM입니다.
 */
class VM {
private:
    Chunk* chunk_;                              ///< 실행 중인 청크
    size_t ip_;                                 ///< Instruction Pointer
    std::vector<evaluator::Value> stack_;       ///< 값 스택
    std::vector<CallFrame> frames_;             ///< 호출 프레임 스택
    std::shared_ptr<evaluator::Environment> globals_;  ///< 전역 환경

    // 디버그
    bool traceExecution_;                       ///< 실행 추적 플래그

    // 안전 장치
    size_t instructionCount_;                   ///< 실행된 명령어 카운터
    size_t maxInstructions_;                    ///< 최대 명령어 수 (기본: 10,000,000)
    std::chrono::steady_clock::time_point startTime_;  ///< 실행 시작 시간
    std::chrono::milliseconds maxExecutionTime_; ///< 최대 실행 시간 (기본: 5000ms)
    size_t maxStackSize_;                       ///< 최대 스택 크기 (기본: 10000)

public:
    /**
     * @brief VM 생성자
     */
    VM();

    /**
     * @brief 디버그 모드 설정
     * @param trace true면 실행 추적
     */
    void setTraceExecution(bool trace) { traceExecution_ = trace; }

    /**
     * @brief 최대 명령어 수 설정
     * @param max 최대 명령어 수
     */
    void setMaxInstructions(size_t max) { maxInstructions_ = max; }

    /**
     * @brief 최대 실행 시간 설정 (밀리초)
     * @param ms 최대 실행 시간
     */
    void setMaxExecutionTime(size_t ms) { maxExecutionTime_ = std::chrono::milliseconds(ms); }

    /**
     * @brief 최대 스택 크기 설정
     * @param max 최대 스택 크기
     */
    void setMaxStackSize(size_t max) { maxStackSize_ = max; }

    /**
     * @brief 청크 실행
     * @param chunk 실행할 청크
     * @return 실행 결과
     */
    VMResult run(Chunk* chunk);

    /**
     * @brief 스택 최상위 값 반환
     * @return 최상위 값
     */
    evaluator::Value top() const;

    /**
     * @brief 전역 환경 반환
     * @return 전역 환경
     */
    std::shared_ptr<evaluator::Environment> globals() const { return globals_; }

private:
    /**
     * @brief 다음 바이트 읽기
     * @return 바이트
     */
    uint8_t readByte();

    /**
     * @brief 16비트 읽기
     * @return 16비트 값
     */
    uint16_t readShort();

    /**
     * @brief 상수 읽기
     * @return 상수 값
     */
    evaluator::Value readConstant();

    /**
     * @brief 이름 읽기
     * @return 이름
     */
    std::string readName();

    /**
     * @brief 스택에 푸시
     * @param value 값
     */
    void push(const evaluator::Value& value);

    /**
     * @brief 스택에서 팝
     * @return 값
     */
    evaluator::Value pop();

    /**
     * @brief 스택 최상위 값 반환 (제거하지 않음)
     * @return 최상위 값
     */
    evaluator::Value peek(int distance = 0) const;

    /**
     * @brief 런타임 에러
     * @param message 에러 메시지
     */
    void runtimeError(const std::string& message);

    /**
     * @brief 스택 추적 출력
     */
    void printStack() const;

    /**
     * @brief 명령어 실행
     * @return 실행 결과
     */
    VMResult executeInstruction();

    // 산술 연산
    VMResult binaryOp(OpCode op);
};

} // namespace bytecode
} // namespace kingsejong
