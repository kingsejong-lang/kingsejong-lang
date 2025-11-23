/**
 * @file JITCompilerT2.cpp
 * @brief JIT Compiler Tier 2 구현
 * @author KingSejong Team
 * @date 2025-11-24
 */

#include "jit/JITCompilerT2.h"
#include "bytecode/OpCode.h"
#include <algorithm>
#include <iostream>

namespace kingsejong {
namespace jit {

// ========================================
// 생성자 / 소멸자
// ========================================

JITCompilerT2::JITCompilerT2()
    : analyzer_(std::make_unique<InliningAnalyzer>())
    , transformer_(std::make_unique<InliningTransformer>())
    , tier1Compiler_(std::make_unique<JITCompilerT1>())
{
}

JITCompilerT2::~JITCompilerT2()
{
    // 고유 포인터 자동 해제
}

// ========================================
// 함수 분석
// ========================================

FunctionMetadata JITCompilerT2::analyzeFunction(
    bytecode::Chunk* chunk,
    size_t startOffset,
    size_t endOffset)
{
    // 캐시 확인
    auto it = metadataCache_.find(startOffset);
    if (it != metadataCache_.end()) {
        return it->second;
    }

    // 새로운 분석 수행
    FunctionMetadata meta = analyzer_->collectMetadata(chunk, startOffset, endOffset);

    // 캐시에 저장
    metadataCache_[startOffset] = meta;

    return meta;
}

// ========================================
// 인라이닝 포함 컴파일
// ========================================

CompilationResult JITCompilerT2::compileWithInlining(
    bytecode::Chunk* chunk,
    size_t startOffset,
    size_t endOffset)
{
    totalCompilations_++;

    CompilationResult result;
    result.success = false;

    try {
        // Step 1: 함수 메타데이터 수집
        FunctionMetadata callerMeta = analyzeFunction(chunk, startOffset, endOffset);

        // Step 2: Hot Function 여부 확인
        if (!isHotFunction(callerMeta)) {
            result.errorMessage = "Not a hot function (execution count < threshold)";
            return result;
        }

        // Step 3: 인라인 후보 CallSite 발견
        std::vector<CallSite> candidates = findInlineCandidates(chunk, startOffset, endOffset);

        if (candidates.empty()) {
            result.errorMessage = "No inlinable call sites found";
            return result;
        }

        // Step 4: 우선순위 순으로 정렬
        candidates = sortByPriority(candidates);

        // Step 5: 인라이닝 변환 수행
        size_t totalInlined = 0;
        size_t totalBytecodeReduction = 0;

        for (const auto& callSite : candidates) {
            // 상수 폴딩 시도
            if (transformer_->tryConstantFolding(chunk, callSite)) {
                totalInlined++;
                totalBytecodeReduction += 2; // CALL + RETURN 제거
                continue;
            }

            // 일반 인라이닝
            if (callSite.callee && analyzer_->canInline(*callSite.callee)) {
                InlineResult inlineRes = transformer_->inlineFunction(
                    chunk,
                    callSite,
                    *callSite.callee
                );

                if (inlineRes.success) {
                    totalInlined++;
                    totalBytecodeReduction += inlineRes.savedInstructions;
                }
            }
        }

        // Step 6: 데드 코드 제거
        size_t eliminatedCode = transformer_->eliminateDeadCode(chunk, startOffset, endOffset);
        totalBytecodeReduction += eliminatedCode;

        // Step 7: Tier 1 컴파일 (네이티브 코드 생성)
        // TODO: Tier 1 컴파일러 통합 (현재는 스텁)
        // result.function = tier1Compiler_->compile(chunk, startOffset, endOffset);

        // Step 8: 결과 기록
        result.success = (totalInlined > 0);
        result.inlinedFunctions = totalInlined;
        result.bytecodeReduction = totalBytecodeReduction;

        if (result.success) {
            successfulCompilations_++;
            totalInlinedFunctions_ += totalInlined;
            totalBytecodeReduction_ += totalBytecodeReduction;
        }

    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = std::string("Compilation error: ") + e.what();
    }

    return result;
}

// ========================================
// Hot Function 여부 확인
// ========================================

bool JITCompilerT2::isHotFunction(const FunctionMetadata& meta) const
{
    return meta.executionCount >= InliningAnalyzer::HOT_FUNCTION_THRESHOLD;
}

// ========================================
// 인라인 후보 CallSite 발견
// ========================================

std::vector<CallSite> JITCompilerT2::findInlineCandidates(
    bytecode::Chunk* chunk,
    size_t startOffset,
    size_t endOffset)
{
    std::vector<CallSite> candidates;

    const auto& code = chunk->getCode();

    for (size_t ip = startOffset; ip < endOffset && ip < code.size(); ) {
        auto opcode = static_cast<bytecode::OpCode>(code[ip]);

        // CALL OpCode 발견
        if (opcode == bytecode::OpCode::CALL) {
            CallSite site = analyzeCallSite(chunk, ip);

            // 인라이닝 가능 여부 판단
            // callee가 있으면 canInline()으로 검증, 없으면 기본 후보로 추가
            if (site.callee) {
                if (analyzer_->canInline(*site.callee)) {
                    candidates.push_back(site);
                }
            } else {
                // callee가 없는 경우 (함수 테이블 미구현)도 후보로 추가
                // 실제 컴파일 시 callee를 조회해야 함
                candidates.push_back(site);
            }

            // CALL은 3바이트 (opcode + argCount + funcIndex)
            ip += 3;
        }
        else if (opcode == bytecode::OpCode::RETURN || opcode == bytecode::OpCode::HALT) {
            break;
        }
        else {
            // 다음 명령어로 이동
            ip++;

            // 피연산자 크기 고려 (간단한 휴리스틱)
            if (opcode == bytecode::OpCode::LOAD_CONST ||
                opcode == bytecode::OpCode::LOAD_VAR ||
                opcode == bytecode::OpCode::STORE_VAR) {
                ip++; // 1바이트 피연산자
            }
            else if (opcode == bytecode::OpCode::JUMP ||
                     opcode == bytecode::OpCode::JUMP_IF_FALSE ||
                     opcode == bytecode::OpCode::JUMP_IF_TRUE) {
                ip += 2; // 2바이트 점프 오프셋
            }
        }
    }

    return candidates;
}

// ========================================
// CallSite 분석
// ========================================

CallSite JITCompilerT2::analyzeCallSite(
    bytecode::Chunk* chunk,
    size_t callOffset)
{
    CallSite site;
    site.callOffset = callOffset;

    const auto& code = chunk->getCode();

    // CALL 명령어 검증
    if (callOffset >= code.size()) {
        return site; // Invalid
    }

    auto opcode = static_cast<bytecode::OpCode>(code[callOffset]);
    if (opcode != bytecode::OpCode::CALL) {
        return site; // Not a CALL
    }

    // 인자 개수 추출
    if (callOffset + 1 >= code.size()) {
        return site;
    }
    site.argCount = code[callOffset + 1];

    // 함수 인덱스 추출
    if (callOffset + 2 >= code.size()) {
        return site;
    }
    size_t funcIndex = code[callOffset + 2];

    // 함수 메타데이터 조회 (캐시에서)
    // TODO: 실제로는 VM의 함수 테이블에서 조회해야 함
    // 현재는 간단한 휴리스틱으로 인라이닝 가능 여부만 판단

    // 인자가 모두 상수인지 확인 (간단한 휴리스틱)
    site.hasConstantArgs = false;
    if (callOffset >= site.argCount) {
        bool allConst = true;
        for (size_t i = 0; i < site.argCount; i++) {
            size_t argOffset = callOffset - site.argCount + i;
            if (argOffset >= code.size()) {
                allConst = false;
                break;
            }
            auto argOpcode = static_cast<bytecode::OpCode>(code[argOffset]);
            if (argOpcode != bytecode::OpCode::LOAD_CONST) {
                allConst = false;
                break;
            }
        }
        site.hasConstantArgs = allConst;
    }

    // 임시로 funcIndex 기반 휴리스틱 (실제로는 함수 테이블 조회 필요)
    (void)funcIndex; // Suppress unused warning

    // Note: 인라이닝 가능 여부는 analyzer_->canInline()로 판단
    // 여기서는 CallSite 정보만 수집

    return site;
}

// ========================================
// 인라이닝 우선순위 정렬
// ========================================

std::vector<CallSite> JITCompilerT2::sortByPriority(
    const std::vector<CallSite>& candidates)
{
    std::vector<CallSite> sorted = candidates;

    // 우선순위 기준:
    // 1. 상수 인자 (상수 폴딩 가능)
    // 2. 작은 함수 (바이트코드 크기)
    // 3. 호출 빈도 (executionCount)

    std::sort(sorted.begin(), sorted.end(), [](const CallSite& a, const CallSite& b) {
        // 상수 인자가 있는 경우 우선
        if (a.hasConstantArgs && !b.hasConstantArgs) return true;
        if (!a.hasConstantArgs && b.hasConstantArgs) return false;

        // 함수 크기가 작은 경우 우선
        if (a.callee && b.callee) {
            if (a.callee->bytecodeSize < b.callee->bytecodeSize) return true;
            if (a.callee->bytecodeSize > b.callee->bytecodeSize) return false;

            // 실행 빈도가 높은 경우 우선
            return a.callee->executionCount > b.callee->executionCount;
        }

        return false;
    });

    return sorted;
}

// ========================================
// 통계 출력
// ========================================

void JITCompilerT2::printStatistics() const
{
    std::cout << "\n=== JIT Tier 2 Statistics ===\n";
    std::cout << "Total Compilations: " << totalCompilations_ << "\n";
    std::cout << "Successful Compilations: " << successfulCompilations_ << "\n";

    if (totalCompilations_ > 0) {
        double successRate = (double)successfulCompilations_ / totalCompilations_ * 100.0;
        std::cout << "Success Rate: " << successRate << "%\n";
    }

    std::cout << "Total Inlined Functions: " << totalInlinedFunctions_ << "\n";
    std::cout << "Total Bytecode Reduction: " << totalBytecodeReduction_ << " bytes\n";

    if (successfulCompilations_ > 0) {
        double avgInlined = (double)totalInlinedFunctions_ / successfulCompilations_;
        double avgReduction = (double)totalBytecodeReduction_ / successfulCompilations_;
        std::cout << "Avg Inlined per Compilation: " << avgInlined << "\n";
        std::cout << "Avg Bytecode Reduction: " << avgReduction << " bytes\n";
    }

    std::cout << "=============================\n";
}

// ========================================
// 캐시 초기화
// ========================================

void JITCompilerT2::reset()
{
    metadataCache_.clear();
    totalCompilations_ = 0;
    successfulCompilations_ = 0;
    totalInlinedFunctions_ = 0;
    totalBytecodeReduction_ = 0;
}

} // namespace jit
} // namespace kingsejong
