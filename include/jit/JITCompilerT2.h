/**
 * @file JITCompilerT2.h
 * @brief JIT Compiler Tier 2 (Optimizing JIT) - 인라이닝 최적화 JIT 컴파일러
 * @author KingSejong Team
 * @date 2025-11-24
 */

#pragma once

#include "jit/JITCompilerT1.h"
#include "jit/InliningAnalyzer.h"
#include "jit/InliningTransformer.h"
#include "bytecode/Chunk.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace kingsejong {
namespace jit {

/**
 * @struct CompilationResult
 * @brief JIT Tier 2 컴파일 결과
 */
struct CompilationResult
{
    NativeFunction* function = nullptr;     ///< 컴파일된 네이티브 함수
    bool success = false;                   ///< 컴파일 성공 여부
    size_t inlinedFunctions = 0;            ///< 인라인된 함수 개수
    size_t bytecodeReduction = 0;           ///< 바이트코드 감소량 (CALL/RETURN 제거)
    std::string errorMessage;               ///< 실패 시 오류 메시지
};

/**
 * @class JITCompilerT2
 * @brief JIT Compiler Tier 2 - 인라이닝 최적화 JIT 컴파일러
 *
 * JITCompilerT1을 기반으로 함수 인라이닝 최적화를 추가합니다.
 *
 * 특징:
 * - 함수 인라이닝 (작은 Hot Function)
 * - 상수 전파 (Constant Propagation)
 * - 데드 코드 제거 (Dead Code Elimination)
 * - Hot Function 감지 (≥100회 실행)
 *
 * 성능 목표:
 * - Tier 1 대비 20-30% 성능 향상
 * - 작은 함수 집약 프로그램에서 효과적
 *
 * 사용 예시:
 * @code
 * JITCompilerT2 compiler;
 *
 * // 함수 메타데이터 수집
 * FunctionMetadata meta = compiler.analyzeFunction(chunk, 0, chunk->size());
 *
 * // Hot Function 여부 확인
 * if (meta.executionCount >= InliningAnalyzer::HOT_FUNCTION_THRESHOLD) {
 *     // Tier 2 컴파일 (인라이닝 포함)
 *     CompilationResult result = compiler.compileWithInlining(chunk, 0, chunk->size());
 *     if (result.success) {
 *         // 네이티브 함수 실행
 *         result.function->getFunction()(stack, stackSize);
 *     }
 * }
 * @endcode
 */
class JITCompilerT2
{
public:
    /**
     * @brief 생성자
     */
    JITCompilerT2();

    /**
     * @brief 소멸자
     */
    ~JITCompilerT2();

    // 복사 방지
    JITCompilerT2(const JITCompilerT2&) = delete;
    JITCompilerT2& operator=(const JITCompilerT2&) = delete;

    /**
     * @brief 함수 분석
     * @param chunk 바이트코드 청크
     * @param startOffset 시작 오프셋
     * @param endOffset 종료 오프셋
     * @return 함수 메타데이터
     */
    FunctionMetadata analyzeFunction(
        bytecode::Chunk* chunk,
        size_t startOffset,
        size_t endOffset
    );

    /**
     * @brief 인라이닝 포함 컴파일
     * @param chunk 바이트코드 청크
     * @param startOffset 시작 오프셋
     * @param endOffset 종료 오프셋
     * @return 컴파일 결과
     *
     * 함수 내부의 CallSite를 분석하고, 인라이닝 가능한 호출을 인라인합니다.
     */
    CompilationResult compileWithInlining(
        bytecode::Chunk* chunk,
        size_t startOffset,
        size_t endOffset
    );

    /**
     * @brief Hot Function 여부 확인
     * @param meta 함수 메타데이터
     * @return true if Hot Function
     */
    bool isHotFunction(const FunctionMetadata& meta) const;

    /**
     * @brief 인라이닝 후보 CallSite 발견
     * @param chunk 바이트코드 청크
     * @param startOffset 시작 오프셋
     * @param endOffset 종료 오프셋
     * @return CallSite 목록
     */
    std::vector<CallSite> findInlineCandidates(
        bytecode::Chunk* chunk,
        size_t startOffset,
        size_t endOffset
    );

    /**
     * @brief JIT Tier 2 통계 출력
     */
    void printStatistics() const;

    /**
     * @brief 캐시 초기화
     */
    void reset();

private:
    /**
     * @brief CallSite 분석 및 메타데이터 수집
     * @param chunk 청크
     * @param callOffset CALL OpCode 위치
     * @return CallSite 정보
     */
    CallSite analyzeCallSite(
        bytecode::Chunk* chunk,
        size_t callOffset
    );

    /**
     * @brief 인라이닝 우선순위 순서로 CallSite 정렬
     * @param candidates 후보 CallSite 목록
     * @return 우선순위 순 정렬된 목록
     */
    std::vector<CallSite> sortByPriority(
        const std::vector<CallSite>& candidates
    );

    std::unique_ptr<InliningAnalyzer> analyzer_;        ///< 인라이닝 분석기
    std::unique_ptr<InliningTransformer> transformer_;  ///< 인라이닝 변환기
    std::unique_ptr<JITCompilerT1> tier1Compiler_;      ///< Tier 1 컴파일러 (네이티브 코드 생성)

    // 함수 메타데이터 캐시
    std::unordered_map<size_t, FunctionMetadata> metadataCache_;

    // 통계
    size_t totalCompilations_ = 0;                      ///< 총 컴파일 횟수
    size_t successfulCompilations_ = 0;                 ///< 성공한 컴파일 횟수
    size_t totalInlinedFunctions_ = 0;                  ///< 총 인라인된 함수 수
    size_t totalBytecodeReduction_ = 0;                 ///< 총 바이트코드 감소량
};

} // namespace jit
} // namespace kingsejong
