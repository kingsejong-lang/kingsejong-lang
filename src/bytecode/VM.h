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
#include <unordered_map>

namespace kingsejong {

// Forward declarations
namespace jit {
    class JITCompilerT1;
    class JITCompilerT2;
    class HotPathDetector;
    struct NativeFunction;
}

namespace bytecode {

// ============================================================================
// VM 설정 상수
// ============================================================================

/// @name 실행 제한 기본값
/// @{
constexpr size_t VM_DEFAULT_MAX_INSTRUCTIONS = 10000000;    ///< 최대 명령어 수 (10M)
constexpr int VM_DEFAULT_MAX_EXECUTION_TIME_MS = 5000;      ///< 최대 실행 시간 (5초)
constexpr size_t VM_DEFAULT_MAX_STACK_SIZE = 10000;         ///< 최대 스택 크기
/// @}

/// @name JIT 설정
/// @{
constexpr int JIT_LOOP_THRESHOLD = 100;                     ///< JIT 컴파일 루프 임계값
/// @}

/// @name OpCode 인코딩 상수
/// @{
constexpr uint8_t NO_CONSTRUCTOR_FLAG = 0xFF;               ///< 생성자 없음 플래그
constexpr uint8_t RANGE_START_INCLUSIVE = 0x01;             ///< 범위 시작 포함 플래그
constexpr uint8_t RANGE_END_INCLUSIVE = 0x02;               ///< 범위 끝 포함 플래그
constexpr size_t FUNC_ADDR_MASK = 0xFFFF;                   ///< 함수 주소 마스크
/// @}

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

    // JIT 컴파일러
    std::unique_ptr<jit::JITCompilerT1> jitCompiler_;  ///< JIT Tier 1 컴파일러
    std::unique_ptr<jit::JITCompilerT2> jitCompilerT2_;  ///< JIT Tier 2 컴파일러 (인라이닝)
    std::unique_ptr<jit::HotPathDetector> hotPathDetector_;  ///< 핫 패스 감지기
    bool jitEnabled_;                           ///< JIT 활성화 여부 (기본: true)
    std::unordered_map<size_t, jit::NativeFunction*> jitCache_;  ///< JIT 캐시 (ip -> NativeFunction)

    // Phase 7.1: 클래스 시스템
    std::unordered_map<std::string, std::shared_ptr<evaluator::ClassDefinition>> classes_;  ///< 클래스 정의 맵
    std::vector<std::shared_ptr<evaluator::ClassInstance>> thisStack_;  ///< this 스택 (메서드 호출 시)

public:
    /**
     * @brief VM 생성자
     */
    VM();

    /**
     * @brief VM 소멸자
     */
    ~VM();

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
     * @brief JIT 활성화 설정
     * @param enabled true면 JIT 활성화
     */
    void setJITEnabled(bool enabled) { jitEnabled_ = enabled; }

    /**
     * @brief JIT 활성화 여부 반환
     * @return JIT 활성화 여부
     */
    bool isJITEnabled() const { return jitEnabled_; }

    /**
     * @brief JIT 통계 출력
     */
    void printJITStatistics() const;

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

    /**
     * @brief 상수 로드 OpCode 실행 (LOAD_CONST, LOAD_TRUE, LOAD_FALSE, LOAD_NULL)
     * @param instruction OpCode
     * @return 실행 결과
     */
    VMResult executeConstantOps(OpCode instruction);

    /**
     * @brief 변수 조작 OpCode 실행 (LOAD_VAR, STORE_VAR, LOAD_GLOBAL, STORE_GLOBAL)
     * @param instruction OpCode
     * @return 실행 결과
     */
    VMResult executeVariableOps(OpCode instruction);

    /**
     * @brief 산술 연산 OpCode 실행 (ADD, SUB, MUL, DIV, MOD, NEG)
     * @param instruction OpCode
     * @return 실행 결과
     */
    VMResult executeArithmeticOps(OpCode instruction);

    /**
     * @brief 비교 연산 OpCode 실행 (EQ, NE, LT, LE, GT, GE)
     * @param instruction OpCode
     * @return 실행 결과
     */
    VMResult executeComparisonOps(OpCode instruction);

    /**
     * @brief 논리 연산 OpCode 실행 (AND, OR, NOT)
     * @param instruction OpCode
     * @return 실행 결과
     */
    VMResult executeLogicalOps(OpCode instruction);

    /**
     * @brief 제어 흐름 OpCode 실행 (JUMP, JUMP_IF_FALSE, LOOP, CALL, RETURN, HALT, POP)
     * @param instruction OpCode
     * @return 실행 결과
     */
    VMResult executeControlFlowOps(OpCode instruction);

    /**
     * @brief 배열/딕셔너리 OpCode 실행 (BUILD_ARRAY, BUILD_DICT, INDEX_GET, etc.)
     * @param instruction OpCode
     * @return 실행 결과
     */
    VMResult executeArrayOps(OpCode instruction);

    /**
     * @brief Promise/Async OpCode 실행 (AWAIT, BUILD_PROMISE, PROMISE_RESOLVE, etc.)
     * @param instruction OpCode
     * @return 실행 결과
     */
    VMResult executePromiseOps(OpCode instruction);

    /**
     * @brief 기타 OpCode 실행 (PRINT, BUILD_RANGE, IMPORT, etc.)
     * @param instruction OpCode
     * @return 실행 결과
     */
    VMResult executeMiscOps(OpCode instruction);

    // 산술 연산
    VMResult binaryOp(OpCode op);

    // JIT 관련
    /**
     * @brief 루프 감지 및 JIT 컴파일 시도
     * @param loopStart 루프 시작 주소
     */
    void tryJITCompileLoop(size_t loopStart);

    /**
     * @brief JIT 컴파일된 코드 실행
     * @param nativeFunc 네이티브 함수
     * @return 실행 결과
     */
    VMResult executeJITCode(jit::NativeFunction* nativeFunc);
};

} // namespace bytecode
} // namespace kingsejong
