/**
 * @file InliningAnalyzer.cpp
 * @brief 함수 인라이닝 분석 구현
 * @author KingSejong Team
 * @date 2025-11-24
 */

#include "jit/InliningAnalyzer.h"
#include "common/Logger.h"
#include <algorithm>
#include <unordered_set>

namespace kingsejong {
namespace jit {

FunctionMetadata InliningAnalyzer::collectMetadata(
    bytecode::Chunk* chunk,
    size_t startOffset,
    size_t endOffset)
{
    FunctionMetadata meta;
    meta.bytecodeOffset = startOffset;
    meta.bytecodeSize = endOffset - startOffset;

    if (!chunk || startOffset >= endOffset || endOffset > chunk->size())
    {
        Logger::error("[InliningAnalyzer] Invalid chunk or offsets");
        return meta;
    }

    // OpCode 수집 및 분석
    size_t ip = startOffset;
    const auto& code = chunk->getCode();

    while (ip < endOffset)
    {
        auto opcode = static_cast<bytecode::OpCode>(code[ip]);
        meta.opcodes.push_back(opcode);
        meta.opcodeCount[opcode]++;

        ip++;

        // OpCode별 오퍼랜드 처리
        switch (opcode)
        {
            case bytecode::OpCode::LOAD_CONST:
            case bytecode::OpCode::LOAD_GLOBAL:
            case bytecode::OpCode::STORE_GLOBAL:
            case bytecode::OpCode::LOAD_VAR:
            case bytecode::OpCode::STORE_VAR:
            case bytecode::OpCode::BUILD_FUNCTION:
                ip++; // 1-byte operand
                break;

            case bytecode::OpCode::JUMP:
            case bytecode::OpCode::JUMP_IF_FALSE:
            case bytecode::OpCode::JUMP_IF_TRUE:
            case bytecode::OpCode::CALL:
            case bytecode::OpCode::TAIL_CALL:
                ip += 2; // 2-byte operand
                break;

            default:
                // No operand
                break;
        }
    }

    // 함수 특성 분석
    meta.hasLoops = detectLoops(chunk, startOffset, endOffset);
    meta.hasConditionals = hasConditionals(meta.opcodes);
    meta.hasFunctionCalls = hasFunctionCalls(meta.opcodes);
    meta.hasRecursion = false; // 호출자가 설정해야 함 (functionId 필요)

    Logger::debug("[InliningAnalyzer] Collected metadata: size=" +
                  std::to_string(meta.bytecodeSize) +
                  ", loops=" + (meta.hasLoops ? "yes" : "no") +
                  ", calls=" + (meta.hasFunctionCalls ? "yes" : "no"));

    return meta;
}

bool InliningAnalyzer::canInline(const FunctionMetadata& meta) const
{
    // 1. 크기 제한
    if (meta.bytecodeSize > MAX_INLINE_SIZE)
    {
        Logger::debug("[InliningAnalyzer] Cannot inline: size too large (" +
                      std::to_string(meta.bytecodeSize) + " > " +
                      std::to_string(MAX_INLINE_SIZE) + ")");
        return false;
    }

    // 2. 재귀 함수는 인라이닝 불가
    if (meta.hasRecursion)
    {
        Logger::debug("[InliningAnalyzer] Cannot inline: has recursion");
        return false;
    }

    // 3. Hot Function 체크 (실행 횟수)
    if (meta.executionCount < HOT_FUNCTION_THRESHOLD)
    {
        Logger::debug("[InliningAnalyzer] Cannot inline: not hot enough (" +
                      std::to_string(meta.executionCount) + " < " +
                      std::to_string(HOT_FUNCTION_THRESHOLD) + ")");
        return false;
    }

    // 4. 복잡도 체크
    int complexity = meta.getComplexityScore();
    if (complexity > 100)
    {
        Logger::debug("[InliningAnalyzer] Cannot inline: too complex (score=" +
                      std::to_string(complexity) + ")");
        return false;
    }

    Logger::debug("[InliningAnalyzer] Can inline: all checks passed");
    return true;
}

InlinePriority InliningAnalyzer::getInlinePriority(const FunctionMetadata& meta) const
{
    if (!canInline(meta))
    {
        return InlinePriority::NONE;
    }

    // 복잡도 점수로 우선순위 결정
    int complexity = meta.getComplexityScore();

    // 단순 함수 (산술 연산만) - 최고 우선순위
    if (complexity <= 10 && !meta.hasLoops && !meta.hasConditionals)
    {
        Logger::debug("[InliningAnalyzer] Priority: HIGH (simple function)");
        return InlinePriority::HIGH;
    }

    // 조건문 1-2개 포함 - 중간 우선순위
    if (complexity <= 30 && meta.hasConditionals && !meta.hasLoops)
    {
        Logger::debug("[InliningAnalyzer] Priority: MEDIUM (with conditionals)");
        return InlinePriority::MEDIUM;
    }

    // 루프 포함 - 낮은 우선순위
    if (meta.hasLoops)
    {
        Logger::debug("[InliningAnalyzer] Priority: LOW (has loops)");
        return InlinePriority::LOW;
    }

    // 기본 - 중간 우선순위
    Logger::debug("[InliningAnalyzer] Priority: MEDIUM (default)");
    return InlinePriority::MEDIUM;
}

size_t InliningAnalyzer::estimateCodeSize(const CallSite& site) const
{
    return site.getEstimatedInlineSize();
}

CallSite InliningAnalyzer::analyzeCallSite(
    bytecode::Chunk* chunk,
    size_t callOffset,
    FunctionMetadata* callee)
{
    CallSite site;
    site.callOffset = callOffset;
    site.callee = callee;

    if (!chunk || !callee)
    {
        Logger::error("[InliningAnalyzer] Invalid chunk or callee");
        return site;
    }

    const auto& code = chunk->getCode();
    if (callOffset >= code.size())
    {
        Logger::error("[InliningAnalyzer] Invalid call offset");
        return site;
    }

    // CALL OpCode 확인
    auto opcode = static_cast<bytecode::OpCode>(code[callOffset]);
    if (opcode != bytecode::OpCode::CALL && opcode != bytecode::OpCode::TAIL_CALL)
    {
        Logger::error("[InliningAnalyzer] Not a CALL opcode at offset " +
                      std::to_string(callOffset));
        return site;
    }

    // 인자 개수 읽기 (CALL 다음 바이트)
    if (callOffset + 1 < code.size())
    {
        site.argCount = code[callOffset + 1];
    }

    // TODO: 상수 인자 분석 (스택 역추적 필요)
    // 현재는 간단한 구현만 제공
    site.hasConstantArgs = false;

    Logger::debug("[InliningAnalyzer] Analyzed call site: offset=" +
                  std::to_string(callOffset) + ", args=" +
                  std::to_string(site.argCount));

    return site;
}

bool InliningAnalyzer::detectRecursion(
    bytecode::Chunk* chunk,
    size_t startOffset,
    size_t endOffset,
    size_t functionId) const
{
    (void)functionId; // TODO: 실제 함수 ID 비교 구현 필요

    if (!chunk)
    {
        return false;
    }

    const auto& code = chunk->getCode();
    size_t ip = startOffset;

    while (ip < endOffset)
    {
        auto opcode = static_cast<bytecode::OpCode>(code[ip]);
        ip++;

        // CALL 또는 TAIL_CALL 발견
        if (opcode == bytecode::OpCode::CALL || opcode == bytecode::OpCode::TAIL_CALL)
        {
            if (ip + 1 < code.size())
            {
                // TODO: 실제 호출 대상 함수 ID와 비교
                // 현재는 간단한 휴리스틱만 사용
                // 함수 내부에서 CALL이 있으면 재귀 가능성 있음으로 표시
                return true;
            }
        }

        // Operand skip
        switch (opcode)
        {
            case bytecode::OpCode::LOAD_CONST:
            case bytecode::OpCode::LOAD_GLOBAL:
            case bytecode::OpCode::STORE_GLOBAL:
            case bytecode::OpCode::LOAD_VAR:
            case bytecode::OpCode::STORE_VAR:
            case bytecode::OpCode::BUILD_FUNCTION:
                ip++;
                break;

            case bytecode::OpCode::JUMP:
            case bytecode::OpCode::JUMP_IF_FALSE:
            case bytecode::OpCode::JUMP_IF_TRUE:
            case bytecode::OpCode::CALL:
            case bytecode::OpCode::TAIL_CALL:
                ip += 2;
                break;

            default:
                break;
        }
    }

    return false;
}

bool InliningAnalyzer::detectLoops(
    bytecode::Chunk* chunk,
    size_t startOffset,
    size_t endOffset) const
{
    if (!chunk)
    {
        return false;
    }

    const auto& code = chunk->getCode();
    size_t ip = startOffset;

    // 루프 감지: 역방향 점프 (JUMP_IF_FALSE 또는 JUMP)
    while (ip < endOffset)
    {
        auto opcode = static_cast<bytecode::OpCode>(code[ip]);
        ip++;

        if (opcode == bytecode::OpCode::JUMP ||
            opcode == bytecode::OpCode::JUMP_IF_FALSE ||
            opcode == bytecode::OpCode::JUMP_IF_TRUE)
        {
            if (ip + 1 < code.size())
            {
                // 2-byte jump target (little-endian)
                size_t target = code[ip] | (code[ip + 1] << 8);

                // 역방향 점프 = 루프
                if (target < ip - 1)
                {
                    Logger::debug("[InliningAnalyzer] Loop detected: backward jump from " +
                                  std::to_string(ip - 1) + " to " + std::to_string(target));
                    return true;
                }

                ip += 2;
            }
        }
        else
        {
            // Operand skip
            switch (opcode)
            {
                case bytecode::OpCode::LOAD_CONST:
                case bytecode::OpCode::LOAD_GLOBAL:
                case bytecode::OpCode::STORE_GLOBAL:
                case bytecode::OpCode::LOAD_VAR:
                case bytecode::OpCode::STORE_VAR:
                case bytecode::OpCode::BUILD_FUNCTION:
                    ip++;
                    break;

                case bytecode::OpCode::CALL:
                case bytecode::OpCode::TAIL_CALL:
                    ip += 2;
                    break;

                default:
                    break;
            }
        }
    }

    return false;
}

bool InliningAnalyzer::hasConditionals(const std::vector<bytecode::OpCode>& opcodes) const
{
    for (auto opcode : opcodes)
    {
        if (opcode == bytecode::OpCode::JUMP_IF_FALSE ||
            opcode == bytecode::OpCode::JUMP_IF_TRUE)
        {
            return true;
        }
    }
    return false;
}

bool InliningAnalyzer::hasFunctionCalls(const std::vector<bytecode::OpCode>& opcodes) const
{
    for (auto opcode : opcodes)
    {
        if (opcode == bytecode::OpCode::CALL ||
            opcode == bytecode::OpCode::TAIL_CALL)
        {
            return true;
        }
    }
    return false;
}

} // namespace jit
} // namespace kingsejong
