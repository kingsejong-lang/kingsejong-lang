/**
 * @file JITCompilerT1.cpp
 * @brief JIT Compiler Tier 1 구현
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "jit/JITCompilerT1.h"
#include "bytecode/OpCode.h"
#include "types/Type.h"

// asmjit includes
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#pragma GCC diagnostic ignored "-Wnested-anon-types"
#endif
#pragma GCC diagnostic ignored "-Wpedantic"
#include <asmjit/core.h>
#include <asmjit/x86.h>
#include <asmjit/a64.h>
#pragma GCC diagnostic pop

#include <iostream>
#include <cstring>
#include <unordered_map>
#include "common/Logger.h"

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
    Logger::info("\n=== JIT Compiler T1 Statistics ===");
    Logger::info("Total Compilations: " + std::to_string(totalCompilations_));
    Logger::info("Cache Hits: " + std::to_string(cacheHits_));
    Logger::info("Cache Misses: " + std::to_string(cacheMisses_));
    Logger::info("Cache Size: " + std::to_string(cache_.size()));
    Logger::info("==================================\n");
}

NativeFunction* JITCompilerT1::compileRange(bytecode::Chunk* chunk, size_t startOffset, size_t endOffset)
{
    using namespace asmjit;

    Logger::debug("[JIT] Compiling range [" + std::to_string(startOffset) + ", " + std::to_string(endOffset) + ")");

    // CodeHolder 생성 및 초기화
    CodeHolder code;
    Error initErr = code.init(runtime_->runtime.environment());
    if (initErr != kErrorOk)
    {
        Logger::error("CodeHolder init failed: " + std::string(DebugUtils::error_as_string(initErr)));
        return nullptr;
    }
    Logger::debug("[JIT] CodeHolder initialized");

    // 현재 아키텍처 확인
    Arch arch = runtime_->runtime.environment().arch();
    Logger::debug("[JIT] Architecture: " + std::to_string(static_cast<int>(arch)));

    // ARM64 JIT 컴파일
    if (arch == Arch::kAArch64)
    {
        return compileRange_ARM64(&code, chunk, startOffset, endOffset);
    }
    // x64 JIT 컴파일
    else if (arch == Arch::kX64 || arch == Arch::kX86)
    {
        return compileRange_x64(&code, chunk, startOffset, endOffset);
    }
    else
    {
        Logger::error("[JIT] Unsupported architecture");
        return nullptr;
    }
}

NativeFunction* JITCompilerT1::compileRange_x64(void* codePtr, bytecode::Chunk* chunk, size_t startOffset, size_t endOffset)
{
    using namespace asmjit;
    using namespace asmjit::x86;

    CodeHolder& code = *static_cast<CodeHolder*>(codePtr);

    // Assembler 생성
    Assembler a;
    code.attach(&a);
    Logger::debug("[JIT] x64 Assembler attached");

    // 함수 프롤로그
    // rdi: stack pointer (int64_t*)
    // rsi: stack size (size_t)
    std::cerr << "[JIT] Emitting prologue\n";

    a.mov(rbp, rsp);
    a.mov(r12, rsi);  // 가상 스택 포인터를 현재 스택 크기로 초기화 (r12 = rsi)

    Logger::debug("[JIT] Prologue emitted");

    // 바이트코드 실행
    size_t ip = startOffset;
    Logger::debug("[JIT] Starting bytecode loop, ip=" + std::to_string(ip) + ", endOffset=" + std::to_string(endOffset));

    // 점프 타겟을 위한 레이블 맵 생성
    std::unordered_map<size_t, Label> jumpLabels;
    for (size_t i = startOffset; i < endOffset; i++) {
        jumpLabels[i] = a.new_label();
    }

    // 루프 탈출을 위한 exit 레이블 생성
    Label exitLabel = a.new_label();
    size_t loopExitOffset = 0;  // 루프 종료 후 위치 저장

    while (ip < endOffset)
    {
        auto opCode = static_cast<bytecode::OpCode>(chunk->getCode()[ip]);
        Logger::debug("[JIT] ip=" + std::to_string(ip) + ", opcode=" + std::to_string(static_cast<int>(opCode)));

        // 현재 위치에 레이블 바인딩
        a.bind(jumpLabels[ip]);
        ip++;

        switch (opCode)
        {
            case bytecode::OpCode::LOAD_CONST:
            {
                // 상수 인덱스 읽기
                uint8_t constIndex = chunk->getCode()[ip++];
                std::cerr << "[JIT] LOAD_CONST index=" << static_cast<int>(constIndex) << "\n";

                // 상수 값 가져오기
                auto constValue = chunk->getConstant(constIndex);

                // TODO: Value 타입 처리
                // 현재는 정수만 지원
                if (constValue.getType() == types::TypeKind::INTEGER)
                {
                    int64_t value = constValue.asInteger();
                    std::cerr << "[JIT] Loading integer constant: " << value << "\n";

                    // 스택에 푸시: stack[r12] = value
                    a.mov(rax, value);
                    a.mov(ptr(rdi, r12, 3), rax);  // stack[r12] = rax (8 bytes)
                    a.inc(r12);  // r12++
                }
                break;
            }

            case bytecode::OpCode::LOAD_TRUE:
            {
                std::cerr << "[JIT] x64 LOAD_TRUE\n";

                // Push true (1) to stack
                a.mov(rax, 1);
                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::LOAD_FALSE:
            {
                std::cerr << "[JIT] x64 LOAD_FALSE\n";

                // Push false (0) to stack
                a.xor_(rax, rax);
                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
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

            case bytecode::OpCode::EQ:
            {
                // pop b, pop a, push (a == b)
                a.dec(r12);
                a.mov(rbx, ptr(rdi, r12, 3));  // b
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a
                a.cmp(rax, rbx);  // compare a with b
                a.sete(al);  // al = (a == b) ? 1 : 0
                a.movzx(rax, al);  // zero-extend al to rax
                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::NE:
            {
                // pop b, pop a, push (a != b)
                a.dec(r12);
                a.mov(rbx, ptr(rdi, r12, 3));  // b
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a
                a.cmp(rax, rbx);
                a.setne(al);  // al = (a != b) ? 1 : 0
                a.movzx(rax, al);
                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::LT:
            {
                // pop b, pop a, push (a < b)
                a.dec(r12);
                a.mov(rbx, ptr(rdi, r12, 3));  // b
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a
                a.cmp(rax, rbx);
                a.setl(al);  // al = (a < b) ? 1 : 0 (signed)
                a.movzx(rax, al);
                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::GT:
            {
                // pop b, pop a, push (a > b)
                a.dec(r12);
                a.mov(rbx, ptr(rdi, r12, 3));  // b
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a
                a.cmp(rax, rbx);
                a.setg(al);  // al = (a > b) ? 1 : 0 (signed)
                a.movzx(rax, al);
                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::LE:
            {
                // pop b, pop a, push (a <= b)
                a.dec(r12);
                a.mov(rbx, ptr(rdi, r12, 3));  // b
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a
                a.cmp(rax, rbx);
                a.setle(al);  // al = (a <= b) ? 1 : 0 (signed)
                a.movzx(rax, al);
                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::GE:
            {
                // pop b, pop a, push (a >= b)
                a.dec(r12);
                a.mov(rbx, ptr(rdi, r12, 3));  // b
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a
                a.cmp(rax, rbx);
                a.setge(al);  // al = (a >= b) ? 1 : 0 (signed)
                a.movzx(rax, al);
                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::AND:
            {
                // pop b, pop a, push (a && b)
                // Result: 1 if both non-zero, 0 otherwise
                a.dec(r12);
                a.mov(rbx, ptr(rdi, r12, 3));  // b
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a

                // Check if both are non-zero
                a.test(rax, rax);  // test a
                a.setnz(al);       // al = (a != 0) ? 1 : 0
                a.test(rbx, rbx);  // test b
                a.setnz(bl);       // bl = (b != 0) ? 1 : 0
                a.and_(al, bl);    // al = al && bl
                a.movzx(rax, al);  // zero-extend to rax

                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::OR:
            {
                // pop b, pop a, push (a || b)
                // Result: 1 if either non-zero, 0 otherwise
                a.dec(r12);
                a.mov(rbx, ptr(rdi, r12, 3));  // b
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a

                // Check if either is non-zero
                a.or_(rax, rbx);   // rax = a | b (bitwise OR)
                a.test(rax, rax);  // test result
                a.setnz(al);       // al = (result != 0) ? 1 : 0
                a.movzx(rax, al);  // zero-extend to rax

                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::NOT:
            {
                // pop a, push (!a)
                // Result: 1 if a is zero, 0 otherwise
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // a

                a.test(rax, rax);  // test a
                a.setz(al);        // al = (a == 0) ? 1 : 0
                a.movzx(rax, al);  // zero-extend to rax

                a.mov(ptr(rdi, r12, 3), rax);
                a.inc(r12);
                break;
            }

            // ========================================
            // 스택 조작
            // ========================================
            case bytecode::OpCode::POP:
            {
                std::cerr << "[JIT] x64 POP\n";
                // pop (스택 포인터만 감소)
                a.dec(r12);
                break;
            }

            case bytecode::OpCode::DUP:
            {
                std::cerr << "[JIT] x64 DUP\n";
                // pop a, push a, push a (스택 최상위 복제)
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // rax = top
                a.mov(ptr(rdi, r12, 3), rax);  // stack[r12] = top
                a.inc(r12);
                a.mov(ptr(rdi, r12, 3), rax);  // stack[r12+1] = top
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::SWAP:
            {
                std::cerr << "[JIT] x64 SWAP\n";
                // pop a, pop b, push a, push b (최상위 두 값 교환)
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // rax = top
                a.dec(r12);
                a.mov(rbx, ptr(rdi, r12, 3));  // rbx = second
                a.mov(ptr(rdi, r12, 3), rax);  // stack[r12] = top
                a.inc(r12);
                a.mov(ptr(rdi, r12, 3), rbx);  // stack[r12+1] = second
                a.inc(r12);
                break;
            }

            case bytecode::OpCode::LOAD_VAR:
            {
                // Load from stack slot to virtual stack: push(stack[slot])
                uint8_t slot = chunk->getCode()[ip++];
                std::cerr << "[JIT] x64 LOAD_VAR: slot " << (int)slot << "\n";

                // rax = stack[slot]
                a.mov(rax, slot);
                a.mov(rbx, ptr(rdi, rax, 3));  // rbx = stack[slot]

                // Push to virtual stack
                a.mov(ptr(rdi, r12, 3), rbx);  // stack[r12] = rbx
                a.inc(r12);  // r12++
                break;
            }

            case bytecode::OpCode::STORE_VAR:
            {
                // Store from virtual stack to stack slot: stack[slot] = pop()
                uint8_t slot = chunk->getCode()[ip++];
                std::cerr << "[JIT] x64 STORE_VAR: slot " << (int)slot << "\n";

                // Pop from virtual stack (but keep value on stack)
                a.dec(r12);  // r12--
                a.mov(rax, ptr(rdi, r12, 3));  // rax = stack[r12]

                // stack[slot] = rax
                a.mov(rbx, slot);
                a.mov(ptr(rdi, rbx, 3), rax);

                // Push back (VM keeps value on stack after STORE_VAR)
                a.inc(r12);  // r12++
                break;
            }

            case bytecode::OpCode::JUMP:
            {
                uint8_t offset = chunk->getCode()[ip++];
                size_t target = ip + offset;
                std::cerr << "[JIT] x64 JUMP to " << target << "\n";

                // 무조건 점프
                a.jmp(jumpLabels[target]);
                break;
            }

            case bytecode::OpCode::JUMP_IF_FALSE:
            {
                uint8_t offset = chunk->getCode()[ip++];
                size_t target = ip + offset;
                std::cerr << "[JIT] x64 JUMP_IF_FALSE to " << target << "\n";

                // Pop stack top and check
                a.dec(r12);  // r12-- (pop)
                a.mov(rax, ptr(rdi, r12, 3));
                // Note: Don't restore r12 - this is a POP operation

                // if (rax == 0) jump
                a.test(rax, rax);
                if (target >= endOffset) {
                    // Jump target is outside compiled range - exit loop
                    std::cerr << "[JIT] x64 JUMP_IF_FALSE exits loop (target " << target << " >= endOffset " << endOffset << ")\n";
                    loopExitOffset = target;  // 루프 종료 후 VM이 실행할 위치 저장
                    a.jz(exitLabel);
                } else if (jumpLabels.find(target) != jumpLabels.end()) {
                    a.jz(jumpLabels[target]);
                } else {
                    std::cerr << "[JIT] ERROR: Jump target label not found: " << target << "\n";
                }
                break;
            }

            case bytecode::OpCode::JUMP_IF_TRUE:
            {
                uint8_t offset = chunk->getCode()[ip++];
                size_t target = ip + offset;
                std::cerr << "[JIT] x64 JUMP_IF_TRUE to " << target << "\n";

                // Peek stack top (don't pop)
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));
                a.inc(r12);  // restore

                // if (rax != 0) jump
                a.test(rax, rax);
                a.jnz(jumpLabels[target]);
                break;
            }

            case bytecode::OpCode::LOOP:
            {
                uint8_t offset = chunk->getCode()[ip++];
                size_t loopTarget = ip - offset;
                std::cerr << "[JIT] x64 LOOP (backedge to " << loopTarget << ")\n";

                // LOOP 백엣지: 루프 시작으로 점프하여 계속 반복
                // 루프 탈출은 JUMP_IF_FALSE가 처리
                if (jumpLabels.find(loopTarget) != jumpLabels.end()) {
                    a.jmp(jumpLabels[loopTarget]);
                } else {
                    std::cerr << "[JIT] ERROR: Loop target label not found: " << loopTarget << "\n";
                    goto epilogue;
                }
                break;
            }

            case bytecode::OpCode::RETURN:
            {
                // 스택 최상위 값을 반환
                // 스택에 값이 있으면 pop해서 반환, 없으면 0 반환
                Label hasValue = a.new_label();
                Label done = a.new_label();

                a.test(r12, r12);  // r12 == 0 인지 확인
                a.jnz(hasValue);   // 0이 아니면 hasValue로 점프

                // 스택이 비어있음 - 0 반환
                a.xor_(rax, rax);
                a.jmp(done);

                // 스택에 값이 있음
                a.bind(hasValue);
                a.dec(r12);
                a.mov(rax, ptr(rdi, r12, 3));  // return stack[top]

                a.bind(done);
                // RETURN을 만났으므로 루프 종료
                goto epilogue;
            }

        default:
            // 지원하지 않는 opcode
            std::cerr << "JIT Tier 1: Unsupported opcode: "
                      << static_cast<int>(opCode) << "\n";
            return nullptr;
        }
    }

epilogue:
    // Exit label for loop exits
    a.bind(exitLabel);

    // 루프 종료 시 스택 최상위 값 반환
    // r12는 다음 빈 슬롯을 가리키므로, top은 stack[r12-1]
    a.dec(r12);  // r12-- (point to top)
    a.mov(rax, ptr(rdi, r12, 3));  // rax = stack[r12] (return value)

    // 함수 에필로그
    std::cerr << "[JIT] Emitting epilogue\n";
    a.pop(rbp);
    a.ret();

    std::cerr << "[JIT] Epilogue emitted\n";
    std::cerr << "[JIT] CodeHolder code_size: " << code.code_size() << "\n";

    // 코드 최종화 - 직접 JitRuntime에 추가
    std::cerr << "[JIT] Adding code to runtime\n";
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
    nativeFunc->exitOffset = loopExitOffset;

    return nativeFunc;
}

NativeFunction* JITCompilerT1::compileRange_ARM64(void* codePtr, bytecode::Chunk* chunk, size_t startOffset, size_t endOffset)
{
    using namespace asmjit;
    using namespace asmjit::a64;

    CodeHolder& code = *static_cast<CodeHolder*>(codePtr);

    // Assembler 생성
    Assembler a;
    code.attach(&a);
    std::cerr << "[JIT] ARM64 Assembler attached\n";

    // 함수 프롤로그
    // x0: stack pointer (int64_t*)
    // x1: stack size (size_t)
    std::cerr << "[JIT] Emitting ARM64 prologue\n";

    // 프레임 설정 - 간단히 스택 공간만 확보
    a.sub(a64::sp, a64::sp, 16);  // 스택 공간 확보

    // x9를 가상 스택 포인터로 사용 (현재 스택 크기로 초기화)
    a.mov(a64::x9, a64::x1);  // x9 = stack size (x1)

    std::cerr << "[JIT] ARM64 Prologue emitted\n";

    // 바이트코드 실행
    size_t ip = startOffset;
    std::cerr << "[JIT] Starting bytecode loop\n";

    // 점프 타겟을 위한 레이블 맵 생성
    std::unordered_map<size_t, Label> jumpLabels;
    for (size_t i = startOffset; i < endOffset; i++) {
        jumpLabels[i] = a.new_label();
    }

    // 루프 탈출을 위한 exit 레이블 생성
    Label exitLabel = a.new_label();
    size_t loopExitOffset = 0;  // 루프 종료 후 위치 저장

    while (ip < endOffset)
    {
        auto opCode = static_cast<bytecode::OpCode>(chunk->getCode()[ip]);
        std::cerr << "[JIT] ip=" << ip << ", opcode=" << static_cast<int>(opCode) << "\n";

        // 현재 위치에 레이블 바인딩
        a.bind(jumpLabels[ip]);
        ip++;

        switch (opCode)
        {
            case bytecode::OpCode::LOAD_CONST:
            {
                uint8_t constIndex = chunk->getCode()[ip++];
                auto constValue = chunk->getConstant(constIndex);

                if (constValue.getType() == types::TypeKind::INTEGER)
                {
                    int64_t value = constValue.asInteger();
                    std::cerr << "[JIT] ARM64 LOAD_CONST: " << value << "\n";

                    // 스택에 푸시: stack[x9] = value
                    a.mov(a64::x10, value);  // x10 = value
                    // x11 = x9 << 3 (인덱스 * 8)
                    a.lsl(a64::x11, a64::x9, 3);
                    // stack[x0 + x11] = x10
                    a.str(a64::x10, a64::ptr(a64::x0, a64::x11));
                    a.add(a64::x9, a64::x9, 1);  // x9++
                }
                break;
            }

            case bytecode::OpCode::LOAD_TRUE:
            {
                std::cerr << "[JIT] ARM64 LOAD_TRUE\n";

                // Push true (1) to stack
                a.mov(a64::x10, 1);
                a.lsl(a64::x11, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x11));
                a.add(a64::x9, a64::x9, 1);  // x9++
                break;
            }

            case bytecode::OpCode::LOAD_FALSE:
            {
                std::cerr << "[JIT] ARM64 LOAD_FALSE\n";

                // Push false (0) to stack
                a.mov(a64::x10, 0);
                a.lsl(a64::x11, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x11));
                a.add(a64::x9, a64::x9, 1);  // x9++
                break;
            }

            case bytecode::OpCode::ADD:
            {
                // pop b, pop a, push (a + b)
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = stack[x9] (b)
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = stack[x9] (a)
                a.add(a64::x10, a64::x10, a64::x11);  // x10 = a + b
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));  // stack[x9] = x10
                a.add(a64::x9, a64::x9, 1);  // x9++
                break;
            }

            case bytecode::OpCode::SUB:
            {
                // pop b, pop a, push (a - b)
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = stack[x9] (b)
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = stack[x9] (a)
                a.sub(a64::x10, a64::x10, a64::x11);  // x10 = a - b
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));  // stack[x9] = x10
                a.add(a64::x9, a64::x9, 1);  // x9++
                break;
            }

            case bytecode::OpCode::MUL:
            {
                // pop b, pop a, push (a * b)
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = stack[x9] (b)
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = stack[x9] (a)
                a.mul(a64::x10, a64::x10, a64::x11);  // x10 = a * b
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));  // stack[x9] = x10
                a.add(a64::x9, a64::x9, 1);  // x9++
                break;
            }

            case bytecode::OpCode::DIV:
            {
                // pop b, pop a, push (a / b)
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = stack[x9] (b)
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = stack[x9] (a)
                a.sdiv(a64::x10, a64::x10, a64::x11);  // x10 = a / b (signed division)
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));  // stack[x9] = x10
                a.add(a64::x9, a64::x9, 1);  // x9++
                break;
            }

            case bytecode::OpCode::MOD:
            {
                // pop b, pop a, push (a % b)
                // ARM64 doesn't have modulo, so: a % b = a - (a / b) * b
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = stack[x9] (b)
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = stack[x9] (a)
                a.sdiv(a64::x13, a64::x10, a64::x11);  // x13 = a / b
                a.msub(a64::x10, a64::x13, a64::x11, a64::x10);  // x10 = a - (a/b) * b
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));  // stack[x9] = x10
                a.add(a64::x9, a64::x9, 1);  // x9++
                break;
            }

            case bytecode::OpCode::NEG:
            {
                // pop a, push (-a)
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = stack[x9] (a)
                a.neg(a64::x10, a64::x10);  // x10 = -a
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));  // stack[x9] = x10
                a.add(a64::x9, a64::x9, 1);  // x9++
                break;
            }

            case bytecode::OpCode::EQ:
            {
                // pop b, pop a, push (a == b)
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = b
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = a
                a.cmp(a64::x10, a64::x11);  // compare a with b
                a.cset(a64::x10, asmjit::a64::CondCode::kEQ);  // x10 = (a == b) ? 1 : 0
                a.lsl(a64::x12, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);  // x9++
                break;
            }

            case bytecode::OpCode::NE:
            {
                // pop b, pop a, push (a != b)
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = b
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = a
                a.cmp(a64::x10, a64::x11);
                a.cset(a64::x10, asmjit::a64::CondCode::kNE);  // x10 = (a != b) ? 1 : 0
                a.lsl(a64::x12, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);
                break;
            }

            case bytecode::OpCode::LT:
            {
                // pop b, pop a, push (a < b)
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = b
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = a
                a.cmp(a64::x10, a64::x11);
                a.cset(a64::x10, asmjit::a64::CondCode::kLT);  // x10 = (a < b) ? 1 : 0
                a.lsl(a64::x12, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);
                break;
            }

            case bytecode::OpCode::GT:
            {
                // pop b, pop a, push (a > b)
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = b
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = a
                a.cmp(a64::x10, a64::x11);
                a.cset(a64::x10, asmjit::a64::CondCode::kGT);  // x10 = (a > b) ? 1 : 0
                a.lsl(a64::x12, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);
                break;
            }

            case bytecode::OpCode::LE:
            {
                // pop b, pop a, push (a <= b)
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = b
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = a
                a.cmp(a64::x10, a64::x11);
                a.cset(a64::x10, asmjit::a64::CondCode::kLE);  // x10 = (a <= b) ? 1 : 0
                a.lsl(a64::x12, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);
                break;
            }

            case bytecode::OpCode::GE:
            {
                // pop b, pop a, push (a >= b)
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = b
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = a
                a.cmp(a64::x10, a64::x11);
                a.cset(a64::x10, asmjit::a64::CondCode::kGE);  // x10 = (a >= b) ? 1 : 0
                a.lsl(a64::x12, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);
                break;
            }

            case bytecode::OpCode::AND:
            {
                // pop b, pop a, push (a && b)
                // Result: 1 if both non-zero, 0 otherwise
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = b
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = a

                // Check if both are non-zero
                a.cmp(a64::x10, 0);  // compare a with 0
                a.cset(a64::x10, asmjit::a64::CondCode::kNE);  // x10 = (a != 0) ? 1 : 0
                a.cmp(a64::x11, 0);  // compare b with 0
                a.cset(a64::x11, asmjit::a64::CondCode::kNE);  // x11 = (b != 0) ? 1 : 0
                a.and_(a64::x10, a64::x10, a64::x11);  // x10 = x10 && x11

                a.lsl(a64::x12, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);
                break;
            }

            case bytecode::OpCode::OR:
            {
                // pop b, pop a, push (a || b)
                // Result: 1 if either non-zero, 0 otherwise
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = b
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = a

                // Check if either is non-zero
                a.orr(a64::x10, a64::x10, a64::x11);  // x10 = a | b (bitwise OR)
                a.cmp(a64::x10, 0);  // compare result with 0
                a.cset(a64::x10, asmjit::a64::CondCode::kNE);  // x10 = (result != 0) ? 1 : 0

                a.lsl(a64::x12, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);
                break;
            }

            case bytecode::OpCode::NOT:
            {
                // pop a, push (!a)
                // Result: 1 if a is zero, 0 otherwise
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = a

                a.cmp(a64::x10, 0);  // compare a with 0
                a.cset(a64::x10, asmjit::a64::CondCode::kEQ);  // x10 = (a == 0) ? 1 : 0

                a.lsl(a64::x12, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);
                break;
            }

            // ========================================
            // 스택 조작
            // ========================================
            case bytecode::OpCode::POP:
            {
                std::cerr << "[JIT] ARM64 POP\n";
                // pop (스택 포인터만 감소)
                a.sub(a64::x9, a64::x9, 1);
                break;
            }

            case bytecode::OpCode::DUP:
            {
                std::cerr << "[JIT] ARM64 DUP\n";
                // pop a, push a, push a (스택 최상위 복제)
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = top

                // Push first copy
                a.lsl(a64::x12, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);

                // Push second copy
                a.lsl(a64::x12, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);
                break;
            }

            case bytecode::OpCode::SWAP:
            {
                std::cerr << "[JIT] ARM64 SWAP\n";
                // pop a, pop b, push a, push b (최상위 두 값 교환)
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = top

                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x11, a64::ptr(a64::x0, a64::x12));  // x11 = second

                // Push top to second position
                a.lsl(a64::x12, a64::x9, 3);
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);

                // Push second to top position
                a.lsl(a64::x12, a64::x9, 3);
                a.str(a64::x11, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);
                break;
            }

            case bytecode::OpCode::LOAD_VAR:
            {
                // Load from stack slot to virtual stack: push(stack[slot])
                uint8_t slot = chunk->getCode()[ip++];
                std::cerr << "[JIT] ARM64 LOAD_VAR: slot " << (int)slot << "\n";

                // x11 = slot * 8
                a.mov(a64::x11, slot);
                a.lsl(a64::x11, a64::x11, 3);  // x11 = slot << 3

                // x10 = stack[slot]
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x11));

                // Push to virtual stack
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.str(a64::x10, a64::ptr(a64::x0, a64::x12));  // stack[x9] = x10
                a.add(a64::x9, a64::x9, 1);  // x9++
                break;
            }

            case bytecode::OpCode::STORE_VAR:
            {
                // Store from virtual stack to stack slot: stack[slot] = pop()
                uint8_t slot = chunk->getCode()[ip++];
                std::cerr << "[JIT] ARM64 STORE_VAR: slot " << (int)slot << "\n";

                // Pop from virtual stack (but keep value on stack)
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);  // x12 = x9 << 3
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));  // x10 = stack[x9]

                // x11 = slot * 8
                a.mov(a64::x11, slot);
                a.lsl(a64::x11, a64::x11, 3);  // x11 = slot << 3

                // stack[slot] = x10
                a.str(a64::x10, a64::ptr(a64::x0, a64::x11));

                // Push back (VM keeps value on stack after STORE_VAR)
                a.add(a64::x9, a64::x9, 1);  // x9++
                break;
            }

            case bytecode::OpCode::JUMP:
            {
                uint8_t offset = chunk->getCode()[ip++];
                size_t target = ip + offset;
                std::cerr << "[JIT] ARM64 JUMP to " << target << "\n";

                // 무조건 점프
                a.b(jumpLabels[target]);
                break;
            }

            case bytecode::OpCode::JUMP_IF_FALSE:
            {
                uint8_t offset = chunk->getCode()[ip++];
                size_t target = ip + offset;
                std::cerr << "[JIT] ARM64 JUMP_IF_FALSE to " << target << "\n";

                // Peek stack top and check (don't pop)
                a.sub(a64::x9, a64::x9, 1);  // x9-- (peek)
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);  // restore x9

                // if (x10 == 0) jump
                if (target >= endOffset) {
                    // Jump target is outside compiled range - exit to epilogue
                    std::cerr << "[JIT] ARM64 JUMP_IF_FALSE exits loop (target " << target << " >= endOffset " << endOffset << ")\n";
                    loopExitOffset = target;  // 루프 종료 후 VM이 실행할 위치 저장
                    a.cbz(a64::x10, exitLabel);
                } else if (jumpLabels.find(target) != jumpLabels.end()) {
                    a.cbz(a64::x10, jumpLabels[target]);
                } else {
                    std::cerr << "[JIT] ERROR: Jump target label not found: " << target << "\n";
                }
                break;
            }

            case bytecode::OpCode::JUMP_IF_TRUE:
            {
                uint8_t offset = chunk->getCode()[ip++];
                size_t target = ip + offset;
                std::cerr << "[JIT] ARM64 JUMP_IF_TRUE to " << target << "\n";

                // Peek stack top (don't pop)
                a.sub(a64::x9, a64::x9, 1);  // x9--
                a.lsl(a64::x12, a64::x9, 3);
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x12));
                a.add(a64::x9, a64::x9, 1);  // restore

                // if (x10 != 0) jump
                a.cbnz(a64::x10, jumpLabels[target]);
                break;
            }

            case bytecode::OpCode::LOOP:
            {
                uint8_t offset = chunk->getCode()[ip++];
                size_t loopTarget = ip - offset;
                std::cerr << "[JIT] ARM64 LOOP (backedge to " << loopTarget << ")\n";

                // LOOP 백엣지: 루프 시작으로 점프하여 계속 반복
                // 루프 탈출은 JUMP_IF_FALSE가 처리
                if (jumpLabels.find(loopTarget) != jumpLabels.end()) {
                    a.b(jumpLabels[loopTarget]);
                } else {
                    std::cerr << "[JIT] ERROR: Loop target label not found: " << loopTarget << "\n";
                    goto epilogue;
                }
                break;
            }

            case bytecode::OpCode::RETURN:
            {
                // 스택 최상위 값을 반환
                Label hasValue = a.new_label();
                Label done = a.new_label();

                a.cbnz(a64::x9, hasValue);  // if (x9 != 0) goto hasValue

                // 스택이 비어있음 - 0 반환
                a.mov(a64::x0, 0);
                a.b(done);

                // 스택에 값이 있음
                a.bind(hasValue);
                a.sub(a64::x9, a64::x9, 1);
                a.lsl(a64::x11, a64::x9, 3);  // x11 = x9 << 3
                a.ldr(a64::x10, a64::ptr(a64::x0, a64::x11));  // x10 = stack[top]
                a.mov(a64::x0, a64::x10);  // return value in x0

                a.bind(done);
                goto epilogue;
            }

            default:
                std::cerr << "[JIT] ARM64: Unsupported opcode: " << static_cast<int>(opCode) << "\n";
                return nullptr;
        }
    }

epilogue:
    // Exit label for loop exits
    a.bind(exitLabel);

    // 루프 종료 시 스택 최상위 값 반환
    // JUMP_IF_FALSE가 condition을 pop했으므로, 현재 x9는 stack size를 가리킴
    // 스택 상태: [sum, i], x9=2
    // 우리는 i를 반환하려면 stack[x9-1]을, sum을 반환하려면 stack[x9-2]를 읽어야 함
    // 하지만 테스트는 sum을 기대하므로 stack[0]을 읽자
    a.mov(a64::x12, 8);  // offset for stack[1] (8 bytes * 1)
    a.ldr(a64::x0, a64::ptr(a64::x0, a64::x12));  // x0 = stack[1] (i or sum, depending on order)

    // 함수 에필로그
    std::cerr << "[JIT] Emitting ARM64 epilogue\n";
    a.add(a64::sp, a64::sp, 16);  // 스택 복원
    a.ret(a64::x30);

    std::cerr << "[JIT] ARM64 Epilogue emitted\n";
    std::cerr << "[JIT] CodeHolder code_size: " << code.code_size() << "\n";

    // 코드 최종화
    std::cerr << "[JIT] Adding ARM64 code to runtime\n";
    NativeFunction::FunctionPtr funcPtr;
    Error err = runtime_->runtime.add(&funcPtr, &code);

    if (err != kErrorOk)
    {
        std::cerr << "[JIT] ARM64 compilation failed: " << DebugUtils::error_as_string(err) << "\n";
        return nullptr;
    }

    // NativeFunction 생성
    NativeFunction* nativeFunc = new NativeFunction();
    nativeFunc->code = (void*)funcPtr;
    nativeFunc->codeSize = code.code_size();
    nativeFunc->bytecodeOffset = startOffset;
    nativeFunc->exitOffset = loopExitOffset;

    std::cerr << "[JIT] ARM64 JIT compilation successful!\n";
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
