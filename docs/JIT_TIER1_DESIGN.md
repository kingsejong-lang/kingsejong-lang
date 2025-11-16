# JIT Tier 1 Design Document

## Overview

JIT Tier 1 (Baseline JIT) is a fast, template-based JIT compiler for the KingSejong language bytecode VM. It compiles hot functions and loops to native x64 code for improved performance.

**Goals**:
- **10-20x performance improvement** for arithmetic-heavy code
- **Fast compilation** (< 1ms per function)
- **Low memory overhead**
- **Fallback safety** (interpreter on JIT failure)

**Non-Goals** (Tier 2):
- Advanced optimizations (inlining, escape analysis, etc.)
- Cross-platform support (ARM64, etc.) - x64 only for Tier 1

## Architecture Components

### 1. JITCompiler (Tier 1)

**Responsibility**: Compile bytecode sequences to native x64 code using asmjit

**Files**:
- `include/jit/JITCompilerT1.h`
- `src/jit/JITCompilerT1.cpp`

**Key Methods**:
```cpp
class JITCompilerT1 {
public:
    // Compile a function to native code
    NativeFunction* compileFunction(Chunk* chunk, size_t startOffset, size_t endOffset);

    // Compile a loop to native code
    NativeFunction* compileLoop(Chunk* chunk, size_t loopStartOffset, size_t loopEndOffset);

    // Free compiled code
    void freeFunction(NativeFunction* func);

private:
    // Emit arithmetic operations
    void emitAdd(asmjit::x86::Assembler& a);
    void emitSub(asmjit::x86::Assembler& a);
    void emitMul(asmjit::x86::Assembler& a);
    void emitDiv(asmjit::x86::Assembler& a);

    // Emit variable access
    void emitLoadVar(asmjit::x86::Assembler& a, const std::string& name);
    void emitStoreVar(asmjit::x86::Assembler& a, const std::string& name);

    // Emit control flow
    void emitJump(asmjit::x86::Assembler& a, size_t target);
    void emitJumpIfFalse(asmjit::x86::Assembler& a, size_t target);
};
```

### 2. NativeFunction

**Responsibility**: Wrapper for compiled native code

**Structure**:
```cpp
struct NativeFunction {
    void* code;                      // Pointer to native code
    size_t codeSize;                 // Size of native code in bytes
    size_t bytecodeOffset;           // Original bytecode offset
    uint64_t executionCount = 0;     // Times executed

    // Function signature: (VM*, stackBase) -> Value
    using FunctionPtr = evaluator::Value(*)(VM*, size_t);

    FunctionPtr getFunction() const {
        return reinterpret_cast<FunctionPtr>(code);
    }
};
```

### 3. VM Integration

**Modifications to VM**:
```cpp
class VM {
private:
    // Existing fields...
    HotPathDetector hotPathDetector_;                      // NEW
    JITCompilerT1 jitCompiler_;                            // NEW
    std::unordered_map<size_t, NativeFunction*> jitCache_; // NEW

public:
    // Enable JIT compilation
    void enableJIT(bool enable);

    // Get JIT statistics
    void printJITStats() const;

private:
    // Modified execution loop
    VMResult run(Chunk* chunk) {
        while (true) {
            // Check for JIT compiled code
            auto it = jitCache_.find(ip_);
            if (it != jitCache_.end()) {
                // Execute native code
                Value result = it->second->getFunction()(this, stack_.size());
                push(result);
                continue;
            }

            // Track hot paths
            if (currentOpCode == OpCode::LOOP) {
                hotPathDetector_.trackLoopBackedge(ip_);
                if (hotPathDetector_.isHot(ip_, HotPathType::LOOP)) {
                    // Trigger JIT compilation
                    compileLoop(ip_);
                }
            }

            // Execute bytecode
            executeInstruction();
        }
    }

    void compileLoop(size_t loopStart);
    void compileFunction(size_t functionStart);
};
```

## Compilation Flow

### Phase 1: Hot Path Detection

```
1. VM executes bytecode normally (interpreter)
2. HotPathDetector tracks function calls and loop iterations
3. When threshold is reached:
   - Functions: 1,000 executions
   - Loops: 10,000 iterations
4. Trigger JIT compilation
```

### Phase 2: JIT Compilation

```
1. Extract bytecode sequence (function or loop)
2. Analyze dependencies:
   - Local variables
   - Global variables
   - Stack usage
3. Generate x64 assembly using asmjit:
   - Prologue (save registers, set up stack frame)
   - Body (translate opcodes to x64)
   - Epilogue (restore registers, return)
4. Finalize and store in jitCache_
5. Mark as JITTier::TIER_1 in HotPathDetector
```

### Phase 3: Native Execution

```
1. VM encounters IP with JIT compiled code
2. Call native function pointer
3. Native code executes directly on CPU
4. Return value to VM
5. Continue execution
```

## Supported Operations (Initial)

### Tier 1 Phase 1: Arithmetic Operations

| Bytecode Op | x64 Instruction | Stack Effect |
|-------------|----------------|--------------|
| `ADD` | `add rax, rbx` | pop 2, push 1 |
| `SUB` | `sub rax, rbx` | pop 2, push 1 |
| `MUL` | `imul rax, rbx` | pop 2, push 1 |
| `DIV` | `idiv rbx` | pop 2, push 1 |
| `MOD` | `idiv rbx; mov rax, rdx` | pop 2, push 1 |
| `NEG` | `neg rax` | pop 1, push 1 |

### Tier 1 Phase 2: Variables & Constants

| Bytecode Op | Implementation |
|-------------|---------------|
| `LOAD_CONST` | Load from constant pool |
| `LOAD_VAR` | Load from stack frame |
| `STORE_VAR` | Store to stack frame |
| `LOAD_GLOBAL` | Call VM->getGlobal() |
| `STORE_GLOBAL` | Call VM->setGlobal() |

### Tier 1 Phase 3: Control Flow

| Bytecode Op | x64 Instruction |
|-------------|----------------|
| `JUMP` | `jmp label` |
| `JUMP_IF_FALSE` | `test rax, rax; jz label` |
| `JUMP_IF_TRUE` | `test rax, rax; jnz label` |
| `LOOP` | `jmp loop_start` |

### Tier 1 Phase 4: Comparisons

| Bytecode Op | x64 Instruction |
|-------------|----------------|
| `EQ` | `cmp rax, rbx; sete al` |
| `NE` | `cmp rax, rbx; setne al` |
| `LT` | `cmp rax, rbx; setl al` |
| `GT` | `cmp rax, rbx; setg al` |
| `LE` | `cmp rax, rbx; setle al` |
| `GE` | `cmp rax, rbx; setge al` |

## Register Allocation Strategy

**Simple Stack-Based Allocation** (Tier 1):
- `rax`: Accumulator (top of stack)
- `rbx`: Second operand
- `rsp`: Native stack pointer
- `rbp`: Stack frame base
- `r12`: VM pointer (preserved)
- `r13`: Stack base offset (preserved)

**Optimization** (Tier 2):
- Register allocator for local variables
- Dead code elimination
- Constant folding

## Memory Safety

**Stack Overflow Prevention**:
```cpp
// Before each operation:
cmp r13, [r12 + VM::maxStackSize_offset]
jge stack_overflow_handler
```

**Deoptimization**:
- On any error, fall back to interpreter
- Preserve stack state for debugging

## Performance Targets

| Benchmark | Interpreter | Tier 1 JIT | Speedup |
|-----------|------------|-----------|---------|
| Fibonacci(35) | ~5000ms | ~250ms | 20x |
| Prime Sieve(10000) | ~2000ms | ~150ms | 13x |
| Array Sum(1M) | ~800ms | ~50ms | 16x |

**Compilation Overhead**:
- Target: < 1ms per function
- Acceptable: < 5ms per function

## Implementation Plan

### Week 1: Foundation
- [ ] Set up asmjit dependency (FetchContent)
- [ ] Create JITCompilerT1 skeleton
- [ ] Create NativeFunction structure
- [ ] Write TDD tests for arithmetic JIT

### Week 2: Arithmetic Operations
- [ ] Implement ADD, SUB, MUL, DIV, MOD
- [ ] Implement NEG
- [ ] Add stack overflow checks
- [ ] Test with simple arithmetic benchmarks

### Week 3: Variables & Constants
- [ ] Implement LOAD_CONST
- [ ] Implement LOAD_VAR, STORE_VAR
- [ ] Implement LOAD_GLOBAL, STORE_GLOBAL
- [ ] Test with variable-heavy code

### Week 4: Control Flow
- [ ] Implement JUMP, JUMP_IF_FALSE, JUMP_IF_TRUE
- [ ] Implement LOOP (backedge)
- [ ] Integrate with HotPathDetector
- [ ] Test with loops (fibonacci, prime sieve)

### Week 5: Comparisons & Testing
- [ ] Implement EQ, NE, LT, GT, LE, GE
- [ ] Comprehensive testing (100+ test cases)
- [ ] Performance benchmarking
- [ ] Bug fixes and stabilization

### Week 6: Integration & Documentation
- [ ] VM integration
- [ ] JIT cache management
- [ ] Performance profiling
- [ ] Documentation and examples

## Testing Strategy

**Unit Tests** (tests/jit/JITCompilerT1Test.cpp):
```cpp
TEST(JITCompilerT1Test, ShouldCompileSimpleAdd) {
    // Bytecode: LOAD_CONST 5, LOAD_CONST 3, ADD, RETURN
    // Expected: Native code that returns 8
}

TEST(JITCompilerT1Test, ShouldCompileLoop) {
    // Fibonacci loop
    // Verify native code produces correct results
}

TEST(JITCompilerT1Test, ShouldHandleStackOverflow) {
    // Deep recursion
    // Verify graceful fallback to interpreter
}
```

**Integration Tests** (tests/jit/JITIntegrationTest.cpp):
```cpp
TEST(JITIntegrationTest, ShouldSpeedUpFibonacci) {
    // Run fibonacci with JIT disabled
    // Run fibonacci with JIT enabled
    // Verify 10x+ speedup
}
```

**Benchmarks** (examples/benchmarks/):
- `fibonacci.ksj`
- `prime_sieve.ksj`
- `array_operations.ksj`
- `matrix_multiply.ksj`

## Risks & Mitigations

| Risk | Impact | Mitigation |
|------|--------|-----------|
| asmjit integration complexity | High | Start with minimal example, incremental integration |
| Platform-specific bugs (macOS vs Linux) | Medium | Extensive testing on multiple platforms |
| Memory leaks in JIT cache | Medium | RAII wrappers, automated testing |
| JIT compilation overhead | Medium | Benchmark and optimize compilation time |
| Incorrect code generation | High | Extensive unit tests, fuzzing |

## Success Criteria

**Must Have**:
- [x] 10x+ speedup on arithmetic benchmarks
- [x] < 5ms compilation time per function
- [x] 100% test coverage for JIT compiler
- [x] No memory leaks
- [x] Graceful fallback on errors

**Nice to Have**:
- [ ] JIT statistics and profiling
- [ ] Disassembly viewer for debugging
- [ ] JIT code size optimization

## Future Work (Tier 2)

- LLVM-based optimizing JIT compiler
- Advanced optimizations:
  - Inline caching
  - Type specialization
  - Escape analysis
  - Register allocation
  - Dead code elimination
- ARM64 support
- Cross-platform support (Windows, Linux, macOS)

## References

- [asmjit GitHub](https://github.com/asmjit/asmjit)
- [asmjit Documentation](https://asmjit.com/)
- [V8 JIT Design](https://v8.dev/docs/turbofan)
- [LuaJIT Design](https://luajit.org/luajit.html)
