/**
 * @file InliningTransformer.h
 * @brief 함수 인라이닝 변환 엔진
 * @author KingSejong Team
 * @date 2025-11-24
 */

#pragma once

#include "bytecode/Chunk.h"
#include "bytecode/OpCode.h"
#include "jit/InliningAnalyzer.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <cstdint>

namespace kingsejong {
namespace jit {

/**
 * @struct InlineResult
 * @brief 인라이닝 변환 결과
 */
struct InlineResult
{
    bool success = false;               ///< 변환 성공 여부
    size_t originalSize = 0;            ///< 원본 바이트코드 크기
    size_t inlinedSize = 0;             ///< 인라인 후 바이트코드 크기
    size_t savedInstructions = 0;       ///< 절약된 명령어 수 (CALL/RETURN)
    std::string errorMessage;           ///< 실패 시 오류 메시지
};

/**
 * @class InliningTransformer
 * @brief 함수 인라이닝 변환 엔진
 *
 * 함수 호출을 함수 본문으로 대체하여 호출 오버헤드를 제거합니다.
 *
 * 변환 과정:
 * 1. CALL OpCode를 함수 본문 바이트코드로 대체
 * 2. 인자 바인딩 (매개변수 → 상수/스택 값)
 * 3. RETURN 제거 (마지막 값은 스택에 남김)
 * 4. 점프 오프셋 조정
 */
class InliningTransformer
{
public:
    /**
     * @brief 생성자
     */
    InliningTransformer() = default;

    /**
     * @brief 소멸자
     */
    ~InliningTransformer() = default;

    /**
     * @brief 함수 인라이닝 수행
     * @param chunk 대상 바이트코드 청크
     * @param callSite 호출 위치 정보
     * @param calleeMeta 호출 대상 함수 메타데이터
     * @return 인라이닝 결과
     *
     * CALL OpCode를 함수 본문으로 대체합니다.
     *
     * 예시:
     * ```
     * // 원본
     * LOAD_CONST 10      // arg1
     * LOAD_CONST 20      // arg2
     * CALL 2             // add(10, 20)
     *
     * // 인라인 후
     * LOAD_CONST 10      // arg1
     * LOAD_CONST 20      // arg2
     * ADD                // 함수 본문
     * // (CALL, RETURN 제거됨)
     * ```
     */
    InlineResult inlineFunction(
        bytecode::Chunk* chunk,
        const CallSite& callSite,
        const FunctionMetadata& calleeMeta
    );

    /**
     * @brief 상수 폴딩 시도
     * @param chunk 대상 바이트코드 청크
     * @param callSite 호출 위치 정보
     * @return true if 상수 폴딩 성공
     *
     * 모든 인자가 상수이면 컴파일 타임에 계산합니다.
     *
     * 예시:
     * ```
     * add(10, 20) → LOAD_CONST 30
     * ```
     */
    bool tryConstantFolding(
        bytecode::Chunk* chunk,
        const CallSite& callSite
    );

    /**
     * @brief 데드 코드 제거
     * @param chunk 대상 바이트코드 청크
     * @param startOffset 시작 오프셋
     * @param endOffset 종료 오프셋
     * @return 제거된 명령어 수
     *
     * 사용되지 않는 중간 값과 도달 불가능한 코드를 제거합니다.
     */
    size_t eliminateDeadCode(
        bytecode::Chunk* chunk,
        size_t startOffset,
        size_t endOffset
    );

private:
    /**
     * @brief 함수 본문 바이트코드 복사
     * @param dest 목적지 청크
     * @param source 원본 청크
     * @param sourceStart 원본 시작 오프셋
     * @param sourceEnd 원본 종료 오프셋
     * @param destOffset 목적지 삽입 위치
     * @return 복사된 바이트 수
     */
    size_t copyBytecode(
        bytecode::Chunk* dest,
        const bytecode::Chunk* source,
        size_t sourceStart,
        size_t sourceEnd,
        size_t destOffset
    );

    /**
     * @brief 인자 바인딩
     * @param chunk 대상 청크
     * @param callSite 호출 위치
     * @param inlinedStart 인라인된 코드 시작 위치
     * @param inlinedEnd 인라인된 코드 종료 위치
     *
     * 함수 내부의 매개변수 로드를 실제 인자로 대체합니다.
     *
     * 예시:
     * ```
     * // 함수: add(a, b) { return a + b }
     * // 호출: add(10, 20)
     *
     * // 변환 전 (함수 본문)
     * LOAD_VAR 0  // a (param 0)
     * LOAD_VAR 1  // b (param 1)
     * ADD
     *
     * // 변환 후
     * LOAD_CONST 10  // a → 10
     * LOAD_CONST 20  // b → 20
     * ADD
     * ```
     */
    void bindArguments(
        bytecode::Chunk* chunk,
        const CallSite& callSite,
        size_t inlinedStart,
        size_t inlinedEnd
    );

    /**
     * @brief RETURN OpCode 제거
     * @param chunk 대상 청크
     * @param returnOffset RETURN OpCode 위치
     *
     * 함수의 RETURN을 제거하고 스택에 값을 남깁니다.
     */
    void removeReturn(
        bytecode::Chunk* chunk,
        size_t returnOffset
    );

    /**
     * @brief 점프 오프셋 조정
     * @param chunk 대상 청크
     * @param startOffset 시작 오프셋
     * @param endOffset 종료 오프셋
     * @param delta 오프셋 변화량
     *
     * 인라이닝으로 인한 바이트코드 크기 변화에 따라
     * 모든 JUMP 계열 명령어의 오프셋을 조정합니다.
     */
    void adjustJumpOffsets(
        bytecode::Chunk* chunk,
        size_t startOffset,
        size_t endOffset,
        int delta
    );

    /**
     * @brief 도달 가능성 분석
     * @param chunk 대상 청크
     * @param startOffset 시작 오프셋
     * @param endOffset 종료 오프셋
     * @return 도달 가능한 오프셋 집합
     *
     * 제어 흐름 분석으로 도달 가능한 명령어를 식별합니다.
     */
    std::unordered_set<size_t> analyzeReachability(
        const bytecode::Chunk* chunk,
        size_t startOffset,
        size_t endOffset
    ) const;

    /**
     * @brief 상수 평가
     * @param opcode 연산자
     * @param args 피연산자 (상수)
     * @return 계산 결과 (nullopt if 계산 불가)
     *
     * 컴파일 타임에 상수 연산을 평가합니다.
     */
    std::optional<evaluator::Value> evaluateConstantOp(
        bytecode::OpCode opcode,
        const std::vector<evaluator::Value>& args
    ) const;
};

} // namespace jit
} // namespace kingsejong
