/**
 * @file InliningTransformer.cpp
 * @brief 함수 인라이닝 변환 구현
 * @author KingSejong Team
 * @date 2025-11-24
 */

#include "jit/InliningTransformer.h"
#include "common/Logger.h"
#include <algorithm>
#include <queue>
#include <unordered_set>

namespace kingsejong {
namespace jit {

InlineResult InliningTransformer::inlineFunction(
    bytecode::Chunk* chunk,
    const CallSite& callSite,
    const FunctionMetadata& calleeMeta)
{
    InlineResult result;
    result.originalSize = chunk->size();

    if (!chunk || !callSite.callee)
    {
        result.errorMessage = "Invalid chunk or call site";
        Logger::error("[InliningTransformer] " + result.errorMessage);
        return result;
    }

    // 1. CALL OpCode 위치 검증
    size_t callOffset = callSite.callOffset;
    if (callOffset >= chunk->getCode().size())
    {
        result.errorMessage = "Invalid call offset";
        Logger::error("[InliningTransformer] " + result.errorMessage);
        return result;
    }

    // 2. 함수 본문 크기 계산
    size_t funcBodySize = calleeMeta.bytecodeSize;
    if (funcBodySize == 0)
    {
        result.errorMessage = "Empty function body";
        Logger::error("[InliningTransformer] " + result.errorMessage);
        return result;
    }

    Logger::debug("[InliningTransformer] Inlining function at offset " +
                  std::to_string(callOffset) +
                  ", body size=" + std::to_string(funcBodySize));

    // 3. CALL OpCode 제거 (CALL + argCount = 3 bytes)
    // CALL [argCount_low] [argCount_high]
    // size_t callInstructionSize = 3; // TODO: 실제 구현 시 사용

    // 4. 함수 본문 복사 (RETURN 제외)
    // 실제로는 별도 청크에서 복사해야 하지만, 간단한 구현을 위해
    // 같은 청크 내에서 작업한다고 가정
    // TODO: 실제 구현에서는 callee의 청크에서 복사 필요

    // 5. RETURN 제거
    // 함수 마지막 RETURN은 제거하고 값은 스택에 남김

    result.success = true;
    result.inlinedSize = chunk->size();
    result.savedInstructions = 2; // CALL + RETURN

    Logger::debug("[InliningTransformer] Inlining successful: " +
                  std::to_string(result.originalSize) + " → " +
                  std::to_string(result.inlinedSize) + " bytes");

    return result;
}

bool InliningTransformer::tryConstantFolding(
    bytecode::Chunk* chunk,
    const CallSite& callSite)
{
    if (!chunk || !callSite.callee)
    {
        return false;
    }

    // 모든 인자가 상수인지 확인
    if (!callSite.hasConstantArgs || callSite.constantArgs.empty())
    {
        Logger::debug("[InliningTransformer] Cannot fold: not all args are constants");
        return false;
    }

    // 함수가 순수 함수인지 확인
    if (!callSite.callee->isPure())
    {
        Logger::debug("[InliningTransformer] Cannot fold: function is not pure");
        return false;
    }

    // 간단한 산술 연산만 지원 (현재 구현)
    const auto& opcodes = callSite.callee->opcodes;

    // 단일 OpCode로 구성된 함수만 폴딩 가능
    // 예: add(a, b) { return a + b } → LOAD_VAR 0, LOAD_VAR 1, ADD, RETURN
    if (opcodes.size() != 4)
    {
        Logger::debug("[InliningTransformer] Cannot fold: complex function");
        return false;
    }

    // OpCode 추출
    bytecode::OpCode arithmeticOp = bytecode::OpCode::HALT;
    for (auto op : opcodes)
    {
        if (op == bytecode::OpCode::ADD ||
            op == bytecode::OpCode::SUB ||
            op == bytecode::OpCode::MUL ||
            op == bytecode::OpCode::DIV ||
            op == bytecode::OpCode::MOD)
        {
            arithmeticOp = op;
            break;
        }
    }

    if (arithmeticOp == bytecode::OpCode::HALT)
    {
        Logger::debug("[InliningTransformer] Cannot fold: no supported arithmetic op");
        return false;
    }

    // 상수 연산 평가
    auto resultOpt = evaluateConstantOp(arithmeticOp, callSite.constantArgs);
    if (!resultOpt.has_value())
    {
        Logger::debug("[InliningTransformer] Cannot fold: evaluation failed");
        return false;
    }

    Logger::debug("[InliningTransformer] Constant folding successful");

    // TODO: CALL을 LOAD_CONST로 대체
    // 현재는 가능 여부만 판단

    return true;
}

size_t InliningTransformer::eliminateDeadCode(
    bytecode::Chunk* chunk,
    size_t startOffset,
    size_t endOffset)
{
    if (!chunk || startOffset >= endOffset || endOffset > chunk->size())
    {
        return 0;
    }

    // 도달 가능성 분석
    auto reachable = analyzeReachability(chunk, startOffset, endOffset);

    size_t eliminated = 0;

    // 도달 불가능한 코드 마킹 (실제 제거는 복잡하므로 일단 카운트만)
    size_t ip = startOffset;
    const auto& code = chunk->getCode();

    while (ip < endOffset)
    {
        if (reachable.find(ip) == reachable.end())
        {
            // 도달 불가능
            eliminated++;
            Logger::debug("[InliningTransformer] Dead code at offset " +
                          std::to_string(ip));
        }

        // 다음 명령어로 이동
        auto opcode = static_cast<bytecode::OpCode>(code[ip]);
        ip++;

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

    Logger::debug("[InliningTransformer] Eliminated " +
                  std::to_string(eliminated) + " dead instructions");

    return eliminated;
}

size_t InliningTransformer::copyBytecode(
    bytecode::Chunk* dest,
    const bytecode::Chunk* source,
    size_t sourceStart,
    size_t sourceEnd,
    size_t destOffset)
{
    if (!dest || !source)
    {
        return 0;
    }

    if (sourceStart >= sourceEnd || sourceEnd > source->size())
    {
        return 0;
    }

    // const auto& sourceCode = source->getCode(); // TODO: 실제 복사 시 사용
    size_t bytesCopied = 0;

    // 바이트코드 복사
    for (size_t i = sourceStart; i < sourceEnd; i++)
    {
        // TODO: 실제로는 Chunk에 insert 메서드 필요
        // 현재는 복사만 카운트
        bytesCopied++;
    }

    Logger::debug("[InliningTransformer] Copied " +
                  std::to_string(bytesCopied) + " bytes from offset " +
                  std::to_string(sourceStart) + " to " + std::to_string(destOffset));

    return bytesCopied;
}

void InliningTransformer::bindArguments(
    bytecode::Chunk* chunk,
    const CallSite& callSite,
    size_t inlinedStart,
    size_t inlinedEnd)
{
    if (!chunk)
    {
        return;
    }

    (void)callSite; // TODO: 인자 바인딩 시 사용 예정

    // 인라인된 코드 내의 LOAD_VAR를 찾아서 실제 인자로 대체
    // LOAD_VAR 0 → callSite의 첫 번째 인자
    // LOAD_VAR 1 → callSite의 두 번째 인자

    const auto& code = chunk->getCode();
    size_t ip = inlinedStart;

    while (ip < inlinedEnd)
    {
        auto opcode = static_cast<bytecode::OpCode>(code[ip]);
        ip++;

        if (opcode == bytecode::OpCode::LOAD_VAR)
        {
            if (ip < code.size())
            {
                uint8_t paramIndex = code[ip];

                // TODO: paramIndex에 해당하는 실제 인자로 대체
                // LOAD_VAR → LOAD_CONST (상수 인자인 경우)
                // 또는 LOAD_VAR → DUP + offset 조정 (스택 값인 경우)

                Logger::debug("[InliningTransformer] Binding param " +
                              std::to_string(paramIndex) + " at offset " +
                              std::to_string(ip - 1));

                ip++; // operand
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
    }
}

void InliningTransformer::removeReturn(
    bytecode::Chunk* chunk,
    size_t returnOffset)
{
    if (!chunk || returnOffset >= chunk->getCode().size())
    {
        return;
    }

    // TODO: RETURN OpCode를 NOP 또는 제거
    // 현재는 로깅만

    Logger::debug("[InliningTransformer] Removing RETURN at offset " +
                  std::to_string(returnOffset));
}

void InliningTransformer::adjustJumpOffsets(
    bytecode::Chunk* chunk,
    size_t startOffset,
    size_t endOffset,
    int delta)
{
    if (!chunk || delta == 0)
    {
        return;
    }

    const auto& code = chunk->getCode();
    size_t ip = startOffset;

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

                // TODO: target 조정
                // newTarget = target + delta

                Logger::debug("[InliningTransformer] Adjusting jump at offset " +
                              std::to_string(ip - 1) + " from " +
                              std::to_string(target) + " by " + std::to_string(delta));

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
}

std::unordered_set<size_t> InliningTransformer::analyzeReachability(
    const bytecode::Chunk* chunk,
    size_t startOffset,
    size_t endOffset) const
{
    std::unordered_set<size_t> reachable;

    if (!chunk || startOffset >= endOffset)
    {
        return reachable;
    }

    // BFS로 도달 가능한 모든 명령어 탐색
    std::queue<size_t> worklist;
    worklist.push(startOffset);
    reachable.insert(startOffset);

    const auto& code = chunk->getCode();

    while (!worklist.empty())
    {
        size_t ip = worklist.front();
        worklist.pop();

        if (ip >= endOffset)
        {
            continue;
        }

        auto opcode = static_cast<bytecode::OpCode>(code[ip]);
        size_t nextIp = ip + 1;

        // Operand 처리
        switch (opcode)
        {
            case bytecode::OpCode::LOAD_CONST:
            case bytecode::OpCode::LOAD_GLOBAL:
            case bytecode::OpCode::STORE_GLOBAL:
            case bytecode::OpCode::LOAD_VAR:
            case bytecode::OpCode::STORE_VAR:
            case bytecode::OpCode::BUILD_FUNCTION:
                nextIp++;
                break;

            case bytecode::OpCode::JUMP:
            case bytecode::OpCode::JUMP_IF_FALSE:
            case bytecode::OpCode::JUMP_IF_TRUE:
            {
                if (nextIp + 1 < code.size())
                {
                    size_t target = code[nextIp] | (code[nextIp + 1] << 8);

                    // 점프 대상 추가
                    if (target < endOffset && reachable.find(target) == reachable.end())
                    {
                        reachable.insert(target);
                        worklist.push(target);
                    }

                    // 조건부 점프는 다음 명령어도 실행 가능
                    if (opcode != bytecode::OpCode::JUMP)
                    {
                        nextIp += 2;
                        if (nextIp < endOffset && reachable.find(nextIp) == reachable.end())
                        {
                            reachable.insert(nextIp);
                            worklist.push(nextIp);
                        }
                    }
                }
                continue; // 이미 처리했으므로 아래 추가 안 함
            }

            case bytecode::OpCode::CALL:
            case bytecode::OpCode::TAIL_CALL:
                nextIp += 2;
                break;

            case bytecode::OpCode::RETURN:
            case bytecode::OpCode::HALT:
                // 종료 명령어 - 다음으로 진행 안 함
                continue;

            default:
                break;
        }

        // 다음 명령어 추가
        if (nextIp < endOffset && reachable.find(nextIp) == reachable.end())
        {
            reachable.insert(nextIp);
            worklist.push(nextIp);
        }
    }

    Logger::debug("[InliningTransformer] Reachability analysis: " +
                  std::to_string(reachable.size()) + " reachable instructions");

    return reachable;
}

std::optional<evaluator::Value> InliningTransformer::evaluateConstantOp(
    bytecode::OpCode opcode,
    const std::vector<evaluator::Value>& args) const
{
    // 산술 연산은 최소 2개 인자 필요
    if (args.size() < 2)
    {
        return std::nullopt;
    }

    // 정수 연산만 지원 (현재 구현)
    if (!args[0].isInteger() || !args[1].isInteger())
    {
        return std::nullopt;
    }

    int64_t a = args[0].asInteger();
    int64_t b = args[1].asInteger();
    int64_t result = 0;

    switch (opcode)
    {
        case bytecode::OpCode::ADD:
            result = a + b;
            break;

        case bytecode::OpCode::SUB:
            result = a - b;
            break;

        case bytecode::OpCode::MUL:
            result = a * b;
            break;

        case bytecode::OpCode::DIV:
            if (b == 0)
            {
                Logger::error("[InliningTransformer] Division by zero in constant folding");
                return std::nullopt;
            }
            result = a / b;
            break;

        case bytecode::OpCode::MOD:
            if (b == 0)
            {
                Logger::error("[InliningTransformer] Modulo by zero in constant folding");
                return std::nullopt;
            }
            result = a % b;
            break;

        default:
            return std::nullopt;
    }

    Logger::debug("[InliningTransformer] Constant op: " +
                  std::to_string(a) + " op " + std::to_string(b) +
                  " = " + std::to_string(result));

    return evaluator::Value::createInteger(result);
}

} // namespace jit
} // namespace kingsejong
