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

namespace kingsejong {
namespace bytecode {

/**
 * @enum VMResult
 * @brief VM 실행 결과
 */
enum class VMResult {
    OK,                 ///< 성공
    COMPILE_ERROR,      ///< 컴파일 에러
    RUNTIME_ERROR       ///< 런타임 에러
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
    std::shared_ptr<evaluator::Environment> globals_;  ///< 전역 환경

    // 디버그
    bool traceExecution_;                       ///< 실행 추적 플래그

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
