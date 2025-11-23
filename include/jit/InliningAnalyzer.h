/**
 * @file InliningAnalyzer.h
 * @brief 함수 인라이닝 분석 및 메타데이터 관리
 * @author KingSejong Team
 * @date 2025-11-24
 */

#pragma once

#include "bytecode/Chunk.h"
#include "bytecode/OpCode.h"
#include "evaluator/Value.h"
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace kingsejong {
namespace jit {

/**
 * @struct FunctionMetadata
 * @brief 함수 메타데이터 - 인라이닝 결정에 사용
 */
struct FunctionMetadata
{
    size_t functionId = 0;              ///< 함수 ID (고유 식별자)
    size_t bytecodeOffset = 0;          ///< 함수 시작 바이트코드 오프셋
    size_t bytecodeSize = 0;            ///< 바이트코드 크기 (명령어 개수)
    uint64_t executionCount = 0;        ///< 실행 횟수 (Hot Function 판별)

    // 함수 특성
    bool hasLoops = false;              ///< 루프 포함 여부
    bool hasRecursion = false;          ///< 재귀 호출 여부 (직접/간접)
    bool hasConditionals = false;       ///< 조건문 포함 여부
    bool hasFunctionCalls = false;      ///< 다른 함수 호출 여부
    uint8_t paramCount = 0;             ///< 매개변수 개수

    // OpCode 통계
    std::vector<bytecode::OpCode> opcodes;  ///< OpCode 시퀀스
    std::unordered_map<bytecode::OpCode, size_t> opcodeCount;  ///< OpCode별 빈도

    /**
     * @brief 함수 복잡도 점수 계산
     * @return 복잡도 점수 (낮을수록 단순, 높을수록 복잡)
     */
    int getComplexityScore() const
    {
        int score = 0;

        // 바이트코드 크기에 비례
        score += bytecodeSize;

        // 루프 있으면 +20
        if (hasLoops) score += 20;

        // 재귀 있으면 +50 (인라이닝 불가)
        if (hasRecursion) score += 50;

        // 조건문 있으면 +5
        if (hasConditionals) score += 5;

        // 함수 호출 있으면 +10
        if (hasFunctionCalls) score += 10;

        return score;
    }

    /**
     * @brief 순수 함수 여부 (부작용 없음)
     * @return true if 순수 함수
     */
    bool isPure() const
    {
        // 전역 변수 접근이나 I/O 없으면 순수 함수
        return !opcodeCount.count(bytecode::OpCode::LOAD_GLOBAL) &&
               !opcodeCount.count(bytecode::OpCode::STORE_GLOBAL);
    }
};

/**
 * @struct CallSite
 * @brief 함수 호출 위치 정보
 */
struct CallSite
{
    size_t callOffset = 0;              ///< CALL OpCode 위치
    FunctionMetadata* callee = nullptr; ///< 호출 대상 함수
    uint8_t argCount = 0;               ///< 인자 개수

    // 상수 인자 최적화를 위한 정보
    std::vector<evaluator::Value> constantArgs;  ///< 상수 인자 (있는 경우)
    bool hasConstantArgs = false;       ///< 모든 인자가 상수인지 여부

    /**
     * @brief 인라이닝 예상 크기 계산
     * @return 인라인 후 예상 바이트코드 크기
     */
    size_t getEstimatedInlineSize() const
    {
        if (!callee) return 0;

        // 함수 본문 크기 + 인자 바인딩 오버헤드
        size_t size = callee->bytecodeSize;

        // 상수 인자가 있으면 상수 전파로 크기 감소 가능
        if (hasConstantArgs) {
            size -= argCount * 2;  // LOAD_LOCAL 제거 가능
        }

        return size;
    }
};

/**
 * @enum InlinePriority
 * @brief 인라이닝 우선순위
 */
enum class InlinePriority
{
    NONE = 0,       ///< 인라이닝 불가
    LOW = 1,        ///< 낮은 우선순위
    MEDIUM = 2,     ///< 중간 우선순위
    HIGH = 3        ///< 높은 우선순위
};

/**
 * @class InliningAnalyzer
 * @brief 함수 인라이닝 분석 엔진
 *
 * 함수의 인라이닝 가능 여부를 판단하고 우선순위를 결정합니다.
 *
 * 인라이닝 조건:
 * - 바이트코드 크기 ≤ MAX_INLINE_SIZE (50)
 * - 실행 횟수 ≥ HOT_FUNCTION_THRESHOLD (100)
 * - 재귀 함수 아님
 * - 타입 안정적
 */
class InliningAnalyzer
{
public:
    // 인라이닝 임계값
    static constexpr size_t MAX_INLINE_SIZE = 50;        ///< 최대 인라인 바이트코드 크기
    static constexpr size_t MAX_INLINE_DEPTH = 3;        ///< 최대 인라이닝 깊이
    static constexpr size_t MAX_TOTAL_INLINE_SIZE = 200; ///< 총 인라인 바이트코드 크기
    static constexpr uint64_t HOT_FUNCTION_THRESHOLD = 100; ///< Hot Function 임계값

    /**
     * @brief 생성자
     */
    InliningAnalyzer() = default;

    /**
     * @brief 소멸자
     */
    ~InliningAnalyzer() = default;

    /**
     * @brief 함수 메타데이터 수집
     * @param chunk 바이트코드 청크
     * @param startOffset 함수 시작 오프셋
     * @param endOffset 함수 종료 오프셋
     * @return 함수 메타데이터
     */
    FunctionMetadata collectMetadata(
        bytecode::Chunk* chunk,
        size_t startOffset,
        size_t endOffset
    );

    /**
     * @brief 인라이닝 가능 여부 판단
     * @param meta 함수 메타데이터
     * @return true if 인라이닝 가능
     */
    bool canInline(const FunctionMetadata& meta) const;

    /**
     * @brief 인라이닝 우선순위 계산
     * @param meta 함수 메타데이터
     * @return 인라이닝 우선순위
     */
    InlinePriority getInlinePriority(const FunctionMetadata& meta) const;

    /**
     * @brief 인라인 후 코드 크기 추정
     * @param site CallSite 정보
     * @return 예상 코드 크기
     */
    size_t estimateCodeSize(const CallSite& site) const;

    /**
     * @brief CallSite 분석
     * @param chunk 바이트코드 청크
     * @param callOffset CALL OpCode 위치
     * @param callee 호출 대상 함수 메타데이터
     * @return CallSite 정보
     */
    CallSite analyzeCallSite(
        bytecode::Chunk* chunk,
        size_t callOffset,
        FunctionMetadata* callee
    );

    /**
     * @brief 재귀 호출 감지
     * @param chunk 바이트코드 청크
     * @param startOffset 함수 시작 오프셋
     * @param endOffset 함수 종료 오프셋
     * @param functionId 함수 ID
     * @return true if 재귀 호출 있음
     */
    bool detectRecursion(
        bytecode::Chunk* chunk,
        size_t startOffset,
        size_t endOffset,
        size_t functionId
    ) const;

    /**
     * @brief 루프 감지
     * @param chunk 바이트코드 청크
     * @param startOffset 함수 시작 오프셋
     * @param endOffset 함수 종료 오프셋
     * @return true if 루프 있음
     */
    bool detectLoops(
        bytecode::Chunk* chunk,
        size_t startOffset,
        size_t endOffset
    ) const;

private:
    /**
     * @brief 조건문 감지
     * @param opcodes OpCode 시퀀스
     * @return true if 조건문 있음
     */
    bool hasConditionals(const std::vector<bytecode::OpCode>& opcodes) const;

    /**
     * @brief 함수 호출 감지
     * @param opcodes OpCode 시퀀스
     * @return true if 함수 호출 있음
     */
    bool hasFunctionCalls(const std::vector<bytecode::OpCode>& opcodes) const;
};

} // namespace jit
} // namespace kingsejong
