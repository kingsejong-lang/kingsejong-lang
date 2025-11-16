/**
 * @file JITCompilerT1.cpp
 * @brief JIT Compiler Tier 1 구현
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "jit/JITCompilerT1.h"
#include "bytecode/OpCode.h"
#include "types/Type.h"

// asmjit includes (x86 specific)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#pragma GCC diagnostic ignored "-Wnested-anon-types"
#include <asmjit/x86.h>
#pragma GCC diagnostic pop

#include <iostream>
#include <cstring>

namespace kingsejong {
namespace jit {

// JitRuntimeWrapper - Opaque wrapper for asmjit::JitRuntime
class JitRuntimeWrapper {
public:
    asmjit::JitRuntime runtime;
};

JITCompilerT1::JITCompilerT1()
    : runtime_(std::make_unique<JitRuntimeWrapper>())
{
}

JITCompilerT1::~JITCompilerT1()
{
    reset();
}

NativeFunction* JITCompilerT1::compileFunction(bytecode::Chunk* chunk, size_t startOffset, size_t endOffset)
{
    if (!chunk || startOffset >= endOffset || endOffset > chunk->size())
    {
        return nullptr;
    }

    // 캐시 확인
    uint64_t cacheKey = makeCacheKey(chunk, startOffset, endOffset);
    auto it = cache_.find(cacheKey);
    if (it != cache_.end())
    {
        cacheHits_++;
        return it->second;
    }

    cacheMisses_++;

    // 컴파일
    NativeFunction* func = compileRange(chunk, startOffset, endOffset);
    if (func)
    {
        cache_[cacheKey] = func;
        totalCompilations_++;
    }

    return func;
}

NativeFunction* JITCompilerT1::compileLoop(bytecode::Chunk* chunk, size_t loopStartOffset, size_t loopEndOffset)
{
    // 루프는 함수와 동일하게 처리 (Tier 1)
    return compileFunction(chunk, loopStartOffset, loopEndOffset);
}

void JITCompilerT1::freeFunction(NativeFunction* func)
{
    if (!func) return;

    // asmjit runtime은 자동으로 메모리 관리
    // 캐시에서만 제거
    for (auto it = cache_.begin(); it != cache_.end(); ++it)
    {
        if (it->second == func)
        {
            cache_.erase(it);
            break;
        }
    }

    delete func;
}

void JITCompilerT1::reset()
{
    // 모든 캐시된 함수 해제
    for (auto& pair : cache_)
    {
        delete pair.second;
    }
    cache_.clear();

    // 런타임 재생성
    runtime_ = std::make_unique<JitRuntimeWrapper>();

    // 통계 초기화
    totalCompilations_ = 0;
    cacheHits_ = 0;
    cacheMisses_ = 0;
}

void JITCompilerT1::printStatistics() const
{
    std::cout << "\n=== JIT Compiler T1 Statistics ===\n";
    std::cout << "Total Compilations: " << totalCompilations_ << "\n";
    std::cout << "Cache Hits: " << cacheHits_ << "\n";
    std::cout << "Cache Misses: " << cacheMisses_ << "\n";
    std::cout << "Cache Size: " << cache_.size() << "\n";
    std::cout << "==================================\n\n";
}

NativeFunction* JITCompilerT1::compileRange(bytecode::Chunk* chunk, size_t startOffset, size_t endOffset)
{
    using namespace asmjit;
    using namespace asmjit::x86;

    // CodeHolder 생성
    CodeHolder code;
    code.init(runtime_->runtime.environment());

    // Assembler 생성
    Assembler a(&code);

    // 함수 프롤로그
    // rdi: stack pointer (int64_t*)
    // rsi: stack size (size_t)
    a.push(rbp);
    a.mov(rbp, rsp);

    // 가상 스택 포인터 (r12 = stack top index)
    a.xor_(r12, r12);  // stack top = 0

    // 바이트코드 실행
    size_t ip = startOffset;
    while (ip < endOffset)
    {
        auto opCode = static_cast<bytecode::OpCode>(chunk->getCode()[ip]);
        ip++;

        switch (opCode)
        {
            case bytecode::OpCode::LOAD_CONST:
            {
                // 상수 인덱스 읽기
                uint8_t constIndex = chunk->getCode()[ip++];

                // 상수 값 가져오기
                auto constValue = chunk->getConstant(constIndex);

                // TODO: Value 타입 처리
                // 현재는 정수만 지원
                if (constValue.getType() == types::TypeKind::INTEGER)
                {
                    int64_t value = constValue.asInteger();

                    // 스택에 푸시: stack[r12] = value
                    a.mov(rax, value);
                    a.mov(ptr(rdi, r12, 3), rax);  // stack[r12] = rax (8 bytes)
                    a.inc(r12);  // r12++
                }
                break;
            }

            case bytecode::OpCode::ADD:
            {
                // pop b, pop a, push (a + b)
                a.dec(r12);  // r12--
                a.mov(rbx, ptr(rdi, r12, 3));  // rbx = stack[r12] (b)
                a.dec(r12);  // r12--
                a.mov(rax, ptr(rdi, r12, 3));  // rax = stack[r12] (a)
                a.add(rax, rbx);  // rax = a + b
                a.mov(ptr(rdi, r12, 3), rax);  // stack[r12] = rax
                a.inc(r12);  // r12++
                break;
            }

            case bytecode::OpCode::SUB:
            {
                // pop b, pop a, push (a - b)
                a.dec(r12);
                a.mov(rbx, ptr(rdi, r12, 3));  // b
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a
                a.sub(rax, rbx);  // rax = a - b
                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::MUL:
            {
                // pop b, pop a, push (a * b)
                a.dec(r12);
                a.mov(rbx, ptr(rdi, r12, 3));  // b
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a
                a.imul(rax, rbx);  // rax = a * b
                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::DIV:
            {
                // pop b, pop a, push (a / b)
                a.dec(r12);
                a.mov(rbx, ptr(rdi, r12, 3));  // b (divisor)
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a (dividend)
                a.xor_(rdx, rdx);  // rdx = 0 (for idiv)
                a.idiv(rbx);  // rax = rdx:rax / rbx, rdx = remainder
                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::MOD:
            {
                // pop b, pop a, push (a % b)
                a.dec(r12);
                a.mov(rbx, ptr(rdi, r12, 3));  // b (divisor)
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a (dividend)
                a.xor_(rdx, rdx);  // rdx = 0
                a.idiv(rbx);  // rdx = a % b
                a.mov(rax, rdx);  // rax = remainder
                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::NEG:
            {
                // pop a, push (-a)
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a
                a.neg(rax);  // rax = -a
                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::RETURN:
            {
                // 스택 최상위 값을 반환
                if (r12.id() != 0)  // 스택에 값이 있으면
                {
                    a.dec(r12);
                    a.mov(rax, ptr(rdi, r12, 3));  // return stack[top]
                }
                else
                {
                    a.xor_(rax, rax);  // return 0
                }
                break;
            }

            default:
                // 지원하지 않는 opcode
                std::cerr << "JIT Tier 1: Unsupported opcode: "
                          << static_cast<int>(opCode) << "\n";
                return nullptr;
        }
    }

    // 함수 에필로그
    a.pop(rbp);
    a.ret();

    // 코드 최종화
    NativeFunction::FunctionPtr funcPtr;
    Error err = runtime_->runtime.add(&funcPtr, &code);

    if (err != kErrorOk)
    {
        std::cerr << "JIT compilation failed: " << DebugUtils::error_as_string(err) << "\n";
        return nullptr;
    }

    // NativeFunction 생성
    NativeFunction* nativeFunc = new NativeFunction();
    nativeFunc->code = (void*)funcPtr;
    nativeFunc->codeSize = code.code_size();
    nativeFunc->bytecodeOffset = startOffset;

    return nativeFunc;
}

uint64_t JITCompilerT1::makeCacheKey(bytecode::Chunk* chunk, size_t startOffset, size_t endOffset) const
{
    // 간단한 해시: chunk 주소 + start + end
    uint64_t chunkAddr = reinterpret_cast<uint64_t>(chunk);
    return (chunkAddr << 32) | ((startOffset & 0xFFFF) << 16) | (endOffset & 0xFFFF);
}

} // namespace jit
} // namespace kingsejong
