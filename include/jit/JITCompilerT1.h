/**
 * @file JITCompilerT1.h
 * @brief JIT Compiler Tier 1 (Baseline JIT) - 빠른 템플릿 기반 JIT 컴파일러
 * @author KingSejong Team
 * @date 2025-11-16
 */

#pragma once

#include "bytecode/Chunk.h"
#include "evaluator/Value.h"
#include <unordered_map>
#include <memory>
#include <cstdint>

namespace kingsejong {
namespace jit {

// Opaque pointer for asmjit::JitRuntime (avoids including heavy asmjit headers)
class JitRuntimeWrapper;

/**
 * @struct NativeFunction
 * @brief 네이티브 코드 래퍼
 */
struct NativeFunction
{
    void* code = nullptr;                     ///< 네이티브 코드 포인터
    size_t codeSize = 0;                      ///< 코드 크기 (bytes)
    size_t bytecodeOffset = 0;                ///< 바이트코드 오프셋
    uint64_t executionCount = 0;              ///< 실행 횟수

    /**
     * @brief 함수 포인터 타입
     * 시그니처: (stackPointer, stackSize) -> int64_t
     */
    using FunctionPtr = int64_t(*)(int64_t*, size_t);

    /**
     * @brief 함수 포인터 반환
     * @return 함수 포인터
     */
    FunctionPtr getFunction() const
    {
        return reinterpret_cast<FunctionPtr>(code);
    }
};

/**
 * @class JITCompilerT1
 * @brief JIT Compiler Tier 1 - 빠른 템플릿 기반 JIT 컴파일러
 *
 * asmjit 라이브러리를 사용하여 바이트코드를 x64 네이티브 코드로 컴파일합니다.
 * Tier 1은 빠른 컴파일 속도를 목표로 하며, 고급 최적화는 Tier 2에서 수행합니다.
 *
 * 특징:
 * - 빠른 컴파일 (< 5ms per function)
 * - 산술 연산, 변수 접근, 제어 흐름 지원
 * - JIT 캐시 (중복 컴파일 방지)
 * - 스택 기반 코드 생성
 *
 * 사용 예시:
 * @code
 * JITCompilerT1 compiler;
 * NativeFunction* func = compiler.compileFunction(chunk, 0, chunk->size());
 * if (func) {
 *     int64_t result = func->getFunction()(stack, stackSize);
 * }
 * compiler.freeFunction(func);
 * @endcode
 */
class JITCompilerT1
{
public:
    /**
     * @brief JIT 컴파일러 생성자
     */
    JITCompilerT1();

    /**
     * @brief JIT 컴파일러 소멸자
     */
    ~JITCompilerT1();

    // 복사 방지
    JITCompilerT1(const JITCompilerT1&) = delete;
    JITCompilerT1& operator=(const JITCompilerT1&) = delete;

    /**
     * @brief 함수 컴파일
     * @param chunk 바이트코드 청크
     * @param startOffset 시작 오프셋
     * @param endOffset 종료 오프셋
     * @return 네이티브 함수 (실패 시 nullptr)
     */
    NativeFunction* compileFunction(bytecode::Chunk* chunk, size_t startOffset, size_t endOffset);

    /**
     * @brief 루프 컴파일
     * @param chunk 바이트코드 청크
     * @param loopStartOffset 루프 시작 오프셋
     * @param loopEndOffset 루프 종료 오프셋
     * @return 네이티브 함수 (실패 시 nullptr)
     */
    NativeFunction* compileLoop(bytecode::Chunk* chunk, size_t loopStartOffset, size_t loopEndOffset);

    /**
     * @brief 네이티브 함수 해제
     * @param func 네이티브 함수
     */
    void freeFunction(NativeFunction* func);

    /**
     * @brief JIT 캐시 초기화
     */
    void reset();

    /**
     * @brief JIT 통계 출력
     */
    void printStatistics() const;

private:
    /**
     * @brief 바이트코드 범위를 네이티브 코드로 컴파일
     * @param chunk 바이트코드 청크
     * @param startOffset 시작 오프셋
     * @param endOffset 종료 오프셋
     * @return 네이티브 함수 (실패 시 nullptr)
     */
    NativeFunction* compileRange(bytecode::Chunk* chunk, size_t startOffset, size_t endOffset);

    /**
     * @brief 캐시 키 생성
     * @param chunk 청크 포인터
     * @param startOffset 시작 오프셋
     * @param endOffset 종료 오프셋
     * @return 캐시 키
     */
    uint64_t makeCacheKey(bytecode::Chunk* chunk, size_t startOffset, size_t endOffset) const;

    // asmjit JIT 런타임 (opaque pointer)
    std::unique_ptr<JitRuntimeWrapper> runtime_;

    // JIT 캐시 (bytecodeOffset -> NativeFunction)
    std::unordered_map<uint64_t, NativeFunction*> cache_;

    // 통계
    uint64_t totalCompilations_ = 0;     ///< 총 컴파일 횟수
    uint64_t cacheHits_ = 0;             ///< 캐시 히트 횟수
    uint64_t cacheMisses_ = 0;           ///< 캐시 미스 횟수
};

} // namespace jit
} // namespace kingsejong
