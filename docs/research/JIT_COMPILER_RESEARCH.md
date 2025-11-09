# KingSejong JIT 컴파일러 연구 보고서

> **작성일**: 2025-11-10
> **작성자**: KingSejong 개발팀
> **버전**: 1.0
> **상태**: 📝 연구 단계

---

## 📋 목차

1. [개요](#개요)
2. [현재 KingSejong 아키텍처](#현재-kingsejong-아키텍처)
3. [JIT 컴파일 기본 개념](#jit-컴파일-기본-개념)
4. [핫 패스 감지 전략](#핫-패스-감지-전략)
5. [JIT 컴파일러 설계 옵션](#jit-컴파일러-설계-옵션)
6. [LLVM 백엔드 통합](#llvm-백엔드-통합)
7. [성능 예측 및 벤치마크](#성능-예측-및-벤치마크)
8. [구현 로드맵](#구현-로드맵)
9. [권장 사항](#권장-사항)
10. [참고 자료](#참고-자료)

---

## 개요

### 목적
KingSejong 언어의 런타임 성능을 획기적으로 향상시키기 위해 JIT(Just-In-Time) 컴파일러 도입을 연구합니다.

### 목표
- **핫 패스 감지**: 자주 실행되는 코드 경로 식별
- **JIT 컴파일**: 바이트코드를 네이티브 코드로 런타임 컴파일
- **성능 향상**: 10-100배 성능 개선 (특히 반복 코드)
- **점진적 최적화**: 프로그램 실행 중 지속적인 최적화

### 범위
- **Phase 1**: 핫 패스 감지 인프라 구축
- **Phase 2**: 기본 JIT 컴파일러 프로토타입
- **Phase 3**: LLVM 백엔드 통합
- **Phase 4**: 프로덕션 준비 및 최적화

---

## 현재 KingSejong 아키텍처

### 실행 파이프라인

```
소스 코드 (.ksj)
    ↓
Lexer (토큰화)
    ↓
Parser (AST 생성)
    ↓
Compiler (바이트코드 생성)
    ↓
VM (바이트코드 실행)
    ↓
결과
```

### 현재 성능 특성

| 작업 | 현재 성능 | 병목 지점 |
|-----|---------|---------|
| 피보나치(30) | ~100-200ms | VM 디스패치 오버헤드 |
| 배열 순회(1M) | ~50-100ms | 반복적인 명령어 디코딩 |
| 함수 호출(1M) | ~100-200ms | 스택 프레임 생성/제거 |
| 재귀 호출 | ~200-400ms | 콜스택 오버헤드 |

### 현재 최적화 수준

✅ **구현된 최적화**:
- 상수 폴딩 (Constant Folding)
- 데드 코드 제거 (Dead Code Elimination)
- 피홀 최적화 (Peephole Optimization)
- 바이트코드 컴파일 (Bytecode Compilation)
- 마크 & 스윕 GC (Mark & Sweep GC)

📝 **미구현 최적화**:
- JIT 컴파일
- 인라이닝 (Inlining)
- 레지스터 할당 최적화
- 꼬리 호출 최적화 (TCO)

---

## JIT 컴파일 기본 개념

### JIT vs AOT vs 인터프리터

| 방식 | 장점 | 단점 | 적용 사례 |
|-----|-----|-----|---------|
| **인터프리터** | 간단한 구현, 즉시 실행 | 느린 성능 | Python, Ruby (초기) |
| **AOT 컴파일** | 최고 성능, 최적화 가능 | 느린 빌드, 플랫폼 종속 | C, C++, Rust |
| **바이트코드 VM** | 중간 성능, 포터블 | 디스패치 오버헤드 | Python, Ruby, Lua |
| **JIT 컴파일** | 런타임 최적화, 빠른 성능 | 복잡한 구현, 메모리 사용 | JavaScript (V8), Java (HotSpot) |

### JIT 컴파일 프로세스

```
바이트코드 실행 (인터프리터)
    ↓
핫 패스 감지
    ↓
프로파일 수집
    ↓
JIT 컴파일
    ↓
네이티브 코드 생성
    ↓
네이티브 코드 실행
    ↓
디옵티마이제이션 (필요 시)
```

### Tiered Compilation 전략

대부분의 현대 JIT 컴파일러는 다단계 컴파일을 사용합니다:

1. **Tier 0**: 인터프리터 (최초 실행)
2. **Tier 1**: 간단한 JIT 컴파일 (기본 최적화)
3. **Tier 2**: 고급 JIT 컴파일 (프로파일 기반 최적화)

**예시 (Java HotSpot)**:
- C1 컴파일러: 빠른 컴파일, 기본 최적화
- C2 컴파일러: 느린 컴파일, 고급 최적화

---

## 핫 패스 감지 전략

### 1. 실행 카운터 기반

**방법**: 각 함수/루프의 실행 횟수를 카운트

```cpp
// VM.h
class VM {
private:
    std::unordered_map<size_t, uint64_t> executionCounts;
    static constexpr uint64_t JIT_THRESHOLD = 10000;

    void trackExecution(size_t bytecodeOffset) {
        executionCounts[bytecodeOffset]++;

        if (executionCounts[bytecodeOffset] >= JIT_THRESHOLD) {
            triggerJITCompilation(bytecodeOffset);
        }
    }
};
```

**장점**:
- 구현이 간단
- 낮은 오버헤드
- 명확한 임계값 설정 가능

**단점**:
- 단순한 메트릭
- 실행 시간 고려 안 함
- 콜드 스타트 문제

### 2. 샘플링 기반 프로파일링

**방법**: 주기적으로 실행 중인 코드 샘플링

```cpp
// Profiler.h
class Profiler {
private:
    std::map<size_t, uint64_t> samples;
    std::chrono::steady_clock::time_point lastSample;

    void sample() {
        auto now = std::chrono::steady_clock::now();
        if (now - lastSample > std::chrono::milliseconds(10)) {
            size_t currentPC = vm->getPC();
            samples[currentPC]++;
            lastSample = now;
        }
    }
};
```

**장점**:
- 실행 시간 반영
- 낮은 오버헤드 (샘플링)
- 정확한 핫스팟 탐지

**단점**:
- 구현 복잡도 증가
- 타이머/시그널 필요

### 3. 백엣지 카운팅 (Backedge Counting)

**방법**: 루프의 뒤로 가는 점프(backedge) 카운트

```cpp
// 컴파일러가 루프 백엣지에 카운터 삽입
OP_LOOP:
    loopCounters[loopId]++;
    if (loopCounters[loopId] > JIT_THRESHOLD) {
        compileLoop(loopId);
    }
    // 루프 실행
```

**장점**:
- 루프 집중 최적화
- 정확한 반복 횟수 추적
- 작은 오버헤드

**단점**:
- 루프에만 적용
- 함수 호출 최적화 불가

### 권장: 하이브리드 접근

```cpp
class HotPathDetector {
private:
    // 함수 실행 카운터
    std::unordered_map<FunctionId, uint64_t> functionCounts;

    // 루프 백엣지 카운터
    std::unordered_map<LoopId, uint64_t> loopCounts;

    // 프로파일링 데이터
    std::map<size_t, ExecutionProfile> profiles;

public:
    void trackFunctionCall(FunctionId id) {
        if (++functionCounts[id] > FUNCTION_JIT_THRESHOLD) {
            scheduleJIT(id, JIT_LEVEL_1);
        }
    }

    void trackLoopBackedge(LoopId id) {
        if (++loopCounts[id] > LOOP_JIT_THRESHOLD) {
            scheduleJIT(id, JIT_LEVEL_2);
        }
    }
};
```

---

## JIT 컴파일러 설계 옵션

### Option 1: 템플릿 JIT (가장 간단)

**개념**: 미리 작성된 네이티브 코드 템플릿 사용

```cpp
// 각 OpCode에 대한 네이티브 템플릿
void* templates[256];

templates[OP_ADD] = generateAddTemplate();
templates[OP_SUB] = generateSubTemplate();
// ...

// JIT 컴파일: 템플릿 조합
NativeCode* compile(Chunk* chunk) {
    NativeCode* code = new NativeCode();

    for (auto opcode : chunk->code) {
        code->append(templates[opcode]);
    }

    return code;
}
```

**장점**:
- 매우 빠른 컴파일
- 구현 간단
- 디버깅 용이

**단점**:
- 제한적인 최적화
- 크로스 명령어 최적화 불가
- 코드 크기 증가

**예시**: Lua JIT (초기 버전)

### Option 2: 메소드 JIT

**개념**: 함수 단위로 JIT 컴파일

```cpp
class MethodJIT {
public:
    NativeCode* compile(Function* func) {
        // 1. 바이트코드 분석
        auto cfg = buildControlFlowGraph(func);

        // 2. 최적화
        optimize(cfg);

        // 3. 네이티브 코드 생성
        return generateNativeCode(cfg);
    }
};
```

**장점**:
- 함수 내 최적화 가능
- 적절한 컴파일 단위
- 관리 용이

**단점**:
- 크로스 함수 최적화 불가
- 인라이닝 제한적

**예시**: LuaJIT 2.0, JavaScriptCore (초기)

### Option 3: 트레이싱 JIT

**개념**: 실제 실행 경로(trace)를 기록하고 컴파일

```cpp
class TracingJIT {
private:
    std::vector<Instruction> trace;
    bool tracing = false;

public:
    void startTracing(size_t startPC) {
        trace.clear();
        tracing = true;
    }

    void recordInstruction(Instruction inst) {
        if (tracing) {
            trace.push_back(inst);

            // 루프 닫힘 감지
            if (inst.opcode == OP_LOOP && inst.target == startPC) {
                stopTracing();
                compileTrace();
            }
        }
    }

    NativeCode* compileTrace() {
        // 선형화된 trace를 컴파일 (분기 없음!)
        return linearCompile(trace);
    }
};
```

**장점**:
- 실제 실행 경로만 컴파일
- 분기 예측 필요 없음
- 강력한 최적화 가능

**단점**:
- 트레이스 오버헤드
- 가드 조건 필요
- 트레이스 폭발 (trace explosion)

**예시**: TraceMonkey (Firefox), LuaJIT 2.0

### Option 4: LLVM 기반 JIT

**개념**: LLVM을 백엔드로 사용

```cpp
#include <llvm/ExecutionEngine/MCJIT.h>

class LLVMJITCompiler {
private:
    llvm::LLVMContext context;
    std::unique_ptr<llvm::ExecutionEngine> engine;

public:
    NativeCode* compile(Function* func) {
        // 1. 바이트코드 → LLVM IR
        auto module = bytecodeToLLVMIR(func);

        // 2. LLVM 최적화 패스
        runOptimizationPasses(module);

        // 3. 네이티브 코드 생성
        engine->addModule(std::move(module));
        return engine->getFunctionAddress(func->name);
    }
};
```

**장점**:
- 프로덕션 품질 최적화
- 크로스 플랫폼 지원
- 지속적인 LLVM 개선 혜택

**단점**:
- 긴 컴파일 시간
- 큰 의존성
- 복잡한 통합

**예시**: Julia, Rust (MIR → LLVM), Swift

### 권장: 단계적 접근

**Phase 1: 템플릿 JIT** (1-2개월)
- 빠른 프로토타입
- 개념 검증
- 기본 성능 측정

**Phase 2: 메소드 JIT** (3-6개월)
- 함수 단위 컴파일
- 기본 최적화
- 프로덕션 준비

**Phase 3: LLVM 통합** (6-12개월)
- 고급 최적화
- 크로스 플랫폼 안정화
- 최종 목표

---

## LLVM 백엔드 통합

### LLVM 소개

**LLVM**은 모듈식 컴파일러 프레임워크입니다:
- **Frontend**: 소스 코드 → LLVM IR
- **Optimizer**: LLVM IR 최적화
- **Backend**: LLVM IR → 네이티브 코드

### KingSejong → LLVM IR 변환

#### 예시 1: 간단한 덧셈

**KingSejong 코드**:
```kingsejong
x = 10 + 20
출력(x)
```

**바이트코드**:
```
CONSTANT 0    ; 10
CONSTANT 1    ; 20
ADD
DEFINE_GLOBAL "x"
GET_GLOBAL "x"
PRINT
```

**LLVM IR**:
```llvm
define i64 @main() {
entry:
    %0 = add i64 10, 20
    store i64 %0, i64* @x
    %1 = load i64, i64* @x
    call void @print_int(i64 %1)
    ret i64 0
}
```

#### 예시 2: 반복문

**KingSejong 코드**:
```kingsejong
합 = 0
1부터 100까지 반복 (숫자) {
    합 = 합 + 숫자
}
```

**LLVM IR**:
```llvm
define i64 @main() {
entry:
    br label %loop

loop:
    %sum = phi i64 [ 0, %entry ], [ %new_sum, %loop ]
    %i = phi i64 [ 1, %entry ], [ %next_i, %loop ]
    %new_sum = add i64 %sum, %i
    %next_i = add i64 %i, 1
    %cond = icmp sle i64 %next_i, 100
    br i1 %cond, label %loop, label %exit

exit:
    ret i64 %sum
}
```

### LLVM JIT 사용 예시

```cpp
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/IRBuilder.h>

class KingSejongLLVMJIT {
private:
    std::unique_ptr<llvm::orc::LLJIT> jit;
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;

public:
    KingSejongLLVMJIT() : builder(context) {
        auto jitExpected = llvm::orc::LLJITBuilder().create();
        if (!jitExpected) {
            throw std::runtime_error("Failed to create LLJIT");
        }
        jit = std::move(*jitExpected);
    }

    void* compileFunction(bytecode::Chunk* chunk) {
        // 1. 새 LLVM 모듈 생성
        auto module = std::make_unique<llvm::Module>("kingsejong", context);

        // 2. 함수 선언
        auto funcType = llvm::FunctionType::get(
            llvm::Type::getInt64Ty(context),
            false
        );
        auto func = llvm::Function::Create(
            funcType,
            llvm::Function::ExternalLinkage,
            "compiled_func",
            module.get()
        );

        // 3. 기본 블록 생성
        auto bb = llvm::BasicBlock::Create(context, "entry", func);
        builder.SetInsertPoint(bb);

        // 4. 바이트코드 → LLVM IR 변환
        for (size_t ip = 0; ip < chunk->code.size(); ip++) {
            uint8_t instruction = chunk->code[ip];

            switch (instruction) {
                case OP_CONSTANT: {
                    uint8_t index = chunk->code[++ip];
                    auto constant = chunk->constants[index];
                    // LLVM 상수 생성
                    auto llvmConst = llvm::ConstantInt::get(
                        context,
                        llvm::APInt(64, std::get<int64_t>(constant))
                    );
                    // 스택에 푸시 (시뮬레이션)
                    break;
                }

                case OP_ADD: {
                    // 스택에서 두 값 팝 (시뮬레이션)
                    llvm::Value* right = /* ... */;
                    llvm::Value* left = /* ... */;

                    // 덧셈 생성
                    auto result = builder.CreateAdd(left, right);
                    // 스택에 푸시
                    break;
                }

                case OP_RETURN: {
                    builder.CreateRet(/* 반환값 */);
                    break;
                }

                // 더 많은 OpCode 처리...
            }
        }

        // 5. 모듈 최적화
        llvm::PassManagerBuilder pmBuilder;
        pmBuilder.OptLevel = 2;
        // ... 최적화 패스 설정

        // 6. JIT 컴파일
        auto rt = jit->getMainJITDylib().createResourceTracker();
        auto tsm = llvm::orc::ThreadSafeModule(
            std::move(module),
            std::make_unique<llvm::LLVMContext>()
        );

        if (auto err = jit->addIRModule(std::move(tsm))) {
            throw std::runtime_error("Failed to add IR module");
        }

        // 7. 함수 주소 가져오기
        auto sym = jit->lookup("compiled_func");
        if (!sym) {
            throw std::runtime_error("Failed to lookup function");
        }

        return (void*)sym->getAddress();
    }
};
```

### LLVM 최적화 레벨

| 레벨 | 설명 | 컴파일 시간 | 성능 향상 |
|-----|-----|----------|---------|
| `-O0` | 최적화 없음 | 빠름 | 없음 |
| `-O1` | 기본 최적화 | 중간 | 2-3배 |
| `-O2` | 표준 최적화 | 느림 | 5-10배 |
| `-O3` | 적극적 최적화 | 매우 느림 | 10-20배 |

### 권장 전략

**Tier 1 (빠른 JIT)**: `-O1` 또는 템플릿 JIT
- 빠른 컴파일
- 기본 최적화
- 낮은 레이턴시

**Tier 2 (고급 JIT)**: `-O2` 또는 `-O3`
- 느린 컴파일
- 고급 최적화
- 높은 성능

---

## 성능 예측 및 벤치마크

### 예상 성능 향상

| 벤치마크 | 현재 (바이트코드) | 예상 (JIT) | 향상 배수 |
|--------|----------------|-----------|---------|
| 피보나치(35) | 200ms | 20ms | 10배 |
| 배열 순회(10M) | 500ms | 50ms | 10배 |
| 함수 호출(10M) | 1000ms | 100ms | 10배 |
| 루프 집중 | 1000ms | 10ms | 100배 |

### 실제 언어 벤치마크

**JavaScript (V8)**:
- 인터프리터 대비 10-100배 향상
- Native 코드 대비 2-5배 느림

**Java (HotSpot)**:
- 인터프리터 대비 20-50배 향상
- C++ 대비 0.5-2배 범위

**LuaJIT**:
- Lua 인터프리터 대비 10-100배 향상
- C 코드 대비 0.5-1배

### 벤치마크 테스트 케이스

```cpp
// benchmarks/jit_benchmarks.cpp

// 1. 피보나치 (재귀)
BENCHMARK(Fibonacci_Bytecode) {
    vm->execute(R"(
        함수 피보나치(n) {
            만약 (n <= 1) {
                반환 n
            }
            반환 피보나치(n - 1) + 피보나치(n - 2)
        }
        피보나치(30)
    )");
}

BENCHMARK(Fibonacci_JIT) {
    jitvm->execute(R"(
        함수 피보나치(n) {
            만약 (n <= 1) {
                반환 n
            }
            반환 피보나치(n - 1) + 피보나치(n - 2)
        }
        피보나치(30)
    )");
}

// 2. 배열 순회
BENCHMARK(ArrayLoop_Bytecode) {
    vm->execute(R"(
        배열 = 1부터 1000000까지
        합 = 0
        배열을 반복 (요소) {
            합 = 합 + 요소
        }
    )");
}

BENCHMARK(ArrayLoop_JIT) {
    jitvm->execute(R"(
        배열 = 1부터 1000000까지
        합 = 0
        배열을 반복 (요소) {
            합 = 합 + 요소
        }
    )");
}

// 3. 중첩 루프
BENCHMARK(NestedLoop_Bytecode) {
    vm->execute(R"(
        합 = 0
        1부터 1000까지 반복 (i) {
            1부터 1000까지 반복 (j) {
                합 = 합 + 1
            }
        }
    )");
}

BENCHMARK(NestedLoop_JIT) {
    jitvm->execute(R"(
        합 = 0
        1부터 1000까지 반복 (i) {
            1부터 1000까지 반복 (j) {
                합 = 합 + 1
            }
        }
    )");
}
```

---

## 구현 로드맵

### Phase 1: 핫 패스 감지 (2주)

**목표**: 자주 실행되는 코드 식별

**작업**:
- [ ] VM에 실행 카운터 추가
- [ ] 함수 호출 추적
- [ ] 루프 백엣지 추적
- [ ] 프로파일링 데이터 수집
- [ ] 핫 패스 식별 로직

**결과물**:
```cpp
// include/bytecode/HotPathDetector.h
class HotPathDetector {
    void trackFunctionCall(FunctionId id);
    void trackLoopBackedge(size_t loopId);
    bool isHot(FunctionId id) const;
    std::vector<FunctionId> getHotFunctions() const;
};
```

### Phase 2: 템플릿 JIT 프로토타입 (1개월)

**목표**: 기본 JIT 컴파일러 구현

**작업**:
- [ ] 네이티브 코드 생성 인프라
- [ ] OpCode 템플릿 구현
- [ ] 함수 호출 규약 (Calling Convention)
- [ ] 스택 프레임 레이아웃
- [ ] 네이티브 코드 실행

**결과물**:
```cpp
// include/jit/TemplateJIT.h
class TemplateJIT {
    NativeFunction* compile(bytecode::Chunk* chunk);
    void execute(NativeFunction* func);
};
```

### Phase 3: LLVM 통합 연구 (2개월)

**목표**: LLVM 백엔드 통합 가능성 평가

**작업**:
- [ ] LLVM 라이브러리 통합
- [ ] 바이트코드 → LLVM IR 변환기
- [ ] LLVM JIT 컴파일러 설정
- [ ] 최적화 파이프라인 구성
- [ ] 성능 벤치마크

**결과물**:
```cpp
// include/jit/LLVMJIT.h
class LLVMJIT {
    void* compileFunction(bytecode::Chunk* chunk);
    llvm::Module* bytecodeToLLVMIR(bytecode::Chunk* chunk);
};
```

### Phase 4: 벤치마크 및 최적화 (1개월)

**목표**: 성능 측정 및 개선

**작업**:
- [ ] 벤치마크 스위트 작성
- [ ] JIT vs 바이트코드 비교
- [ ] 프로파일링 및 병목 분석
- [ ] 최적화 적용
- [ ] 문서화

**결과물**:
- 성능 보고서
- 벤치마크 결과
- 최적화 가이드

---

## 권장 사항

### 단기 (1-2개월)

**우선순위 1**: 핫 패스 감지 구현
- VM에 카운터 추가
- 프로파일링 인프라
- 최소 오버헤드 (<5%)

**우선순위 2**: 템플릿 JIT 프로토타입
- 개념 증명
- 기본 성능 측정
- 아키텍처 검증

### 중기 (3-6개월)

**우선순위 1**: LLVM 통합
- 바이트코드 → LLVM IR
- 기본 JIT 컴파일
- `-O1` 최적화

**우선순위 2**: Tiered Compilation
- Tier 0: 인터프리터
- Tier 1: 템플릿 JIT
- Tier 2: LLVM JIT

### 장기 (6-12개월)

**우선순위 1**: 고급 최적화
- 인라이닝
- 탈가상화 (Devirtualization)
- 타입 특화 (Type Specialization)

**우선순위 2**: 프로덕션 안정화
- 메모리 관리
- 디버깅 지원
- 크로스 플랫폼 테스트

### 현실적인 첫 단계

**권장**: 핫 패스 감지만 구현
- 완전한 JIT는 매우 복잡
- 핫 패스 감지는 즉시 유용
  - 성능 프로파일링
  - 최적화 대상 식별
  - 향후 JIT 기반

**이유**:
1. **복잡도**: JIT 컴파일러는 6-12개월 작업
2. **의존성**: LLVM은 큰 의존성 (100+ MB)
3. **ROI**: 현재 바이트코드 VM이 충분히 빠름
4. **우선순위**: 다른 기능이 더 중요할 수 있음

---

## 참고 자료

### 논문
1. **"A Survey of Adaptive Optimization in Virtual Machines"**
   - JIT 컴파일러 개요
   - https://dl.acm.org/doi/10.1145/1013963.1013969

2. **"Trace-based Just-in-Time Type Specialization for Dynamic Languages"**
   - 트레이싱 JIT
   - https://dl.acm.org/doi/10.1145/1542476.1542528

3. **"HotSpot Performance Techniques"**
   - Java HotSpot 최적화
   - https://www.oracle.com/java/technologies/whitepaper.html

### 오픈소스 프로젝트
1. **LuaJIT**: https://luajit.org/
   - 매우 빠른 JIT
   - 단순한 아키텍처
   - 참고하기 좋음

2. **JavaScriptCore (JSC)**: https://webkit.org/
   - WebKit JavaScript 엔진
   - Multi-tier JIT

3. **V8**: https://v8.dev/
   - Google JavaScript 엔진
   - TurboFan 최적화 컴파일러

4. **LLVM**: https://llvm.org/
   - 모듈식 컴파일러 프레임워크
   - ORC JIT API

### 튜토리얼
1. **LLVM Tutorial**: https://llvm.org/docs/tutorial/
   - Kaleidoscope 언어 예제
   - JIT 컴파일러 구현

2. **"Crafting Interpreters" by Bob Nystrom**
   - https://craftinginterpreters.com/
   - 바이트코드 VM 구현

3. **"Engineering a Compiler" by Cooper & Torczon**
   - 컴파일러 이론
   - 최적화 기법

### 블로그 포스트
1. **"How JavaScript Engines Work"** - Lin Clark
   - https://hacks.mozilla.org/2017/02/a-crash-course-in-just-in-time-jit-compilers/

2. **"A tour of V8"** - Jay Conrod
   - https://jayconrod.com/posts/54/a-tour-of-v8-crankshaft-the-optimizing-compiler

---

## 결론

### 요약

JIT 컴파일러는 KingSejong의 성능을 10-100배 향상시킬 수 있지만, 구현 복잡도가 매우 높습니다.

### 권장 접근

1. **즉시**: 핫 패스 감지만 구현 (F4.8 범위)
2. **향후**: 언어가 성숙한 후 템플릿 JIT 고려
3. **장기**: LLVM 통합 (v2.0+)

### 다음 단계

**F4.8 완료 기준**:
- ✅ JIT 컴파일러 연구 보고서 (본 문서)
- ✅ 핫 패스 감지 프로토타입 구현
- ✅ LLVM 통합 가능성 평가
- ✅ 성능 벤치마크 계획

**Phase 5 고려 사항**:
- 언어 안정화 우선
- 표준 라이브러리 확장
- 에코시스템 구축
- JIT는 v1.5 이후 고려

---

**문서 버전**: 1.0
**최종 업데이트**: 2025-11-10
**다음 리뷰**: v0.2.0 릴리스 후
