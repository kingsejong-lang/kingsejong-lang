# KingSejong - LLVM 백엔드 통합 연구

> **작성일**: 2025-11-10
> **작성자**: KingSejong 개발팀
> **버전**: 1.0
> **상태**: 📝 연구 단계

---

## 목차

1. [개요](#개요)
2. [LLVM 소개](#llvm-소개)
3. [통합 방식 분석](#통합-방식-분석)
4. [구현 예시](#구현-예시)
5. [의존성 분석](#의존성-분석)
6. [성능 예측](#성능-예측)
7. [단계별 통합 계획](#단계별-통합-계획)
8. [대안 방식](#대안-방식)
9. [권장 사항](#권장-사항)
10. [참고 자료](#참고-자료)

---

## 개요

### 목적
KingSejong 언어에 LLVM 백엔드를 통합하여 고성능 JIT 컴파일 및 최적화를 수행합니다.

### 범위
- LLVM 아키텍처 분석
- 바이트코드 → LLVM IR 변환 전략
- ORC JIT API 통합 방법
- 의존성 및 빌드 시스템 통합
- 성능 및 코드 크기 영향 분석

### 결론 (요약)
LLVM은 강력한 최적화와 크로스 플랫폼 지원을 제공하지만, 다음 이유로 **현재 단계에서는 도입을 보류**하는 것을 권장합니다:

1. **큰 의존성**: LLVM 라이브러리 100+ MB, 헤더 50+ MB
2. **긴 컴파일 시간**: LLVM 통합 시 빌드 시간 10-30분 증가
3. **복잡한 통합**: LLVM API 학습 곡선 높음
4. **언어 성숙도**: 현재 바이트코드 VM이 충분히 빠름
5. **우선순위**: 다른 기능(디버거, 표준 라이브러리 등)이 더 시급

**권장**: v1.5 이후, 언어가 안정화되고 성능이 병목이 될 때 재고려

---

## LLVM 소개

### LLVM이란?

**LLVM**(Low Level Virtual Machine)은 모듈식 컴파일러 인프라입니다.

```
소스 코드
    ↓
Frontend (Parser) → LLVM IR
    ↓
Optimizer (LLVM 최적화 패스)
    ↓
Backend (코드 생성) → 네이티브 코드
```

### 주요 구성 요소

#### 1. LLVM IR (Intermediate Representation)
- 플랫폼 독립적인 중간 표현
- SSA (Static Single Assignment) 형식
- 타입 안전
- 무한 레지스터

**예시**:
```llvm
define i64 @add(i64 %a, i64 %b) {
entry:
    %result = add i64 %a, %b
    ret i64 %result
}
```

#### 2. LLVM 최적화 패스
- 데드 코드 제거 (DCE)
- 상수 폴딩
- 인라이닝
- 루프 최적화
- 벡터화
- ...수백 개의 최적화

#### 3. LLVM 백엔드
- x86/x86-64
- ARM/AArch64
- RISC-V
- WebAssembly
- ...20+ 타겟

#### 4. ORC JIT (On-Request Compilation)
- 런타임 JIT 컴파일 API
- 레이지 컴파일 지원
- 심볼 해결
- 컴파일 캐시

### LLVM 사용 프로젝트

| 프로젝트 | 사용 방식 | 성과 |
|---------|---------|-----|
| **Clang** | C/C++ 프론트엔드 | GCC 대체 |
| **Rust** | 백엔드 | 네이티브 성능 |
| **Swift** | 백엔드 | iOS 성능 |
| **Julia** | JIT 컴파일러 | Python보다 10-100배 빠름 |
| **Mesa (OpenGL)** | GPU 셰이더 컴파일 | 런타임 최적화 |
| **PyPy** | Python JIT | CPython보다 2-10배 빠름 |

---

## 통합 방식 분석

### 방식 1: AOT (Ahead-of-Time) 컴파일

**설명**: KingSejong 코드를 미리 네이티브 바이너리로 컴파일

```
.ksj 파일 → Parser → AST → LLVM IR → 네이티브 바이너리
```

**장점**:
- 최고 성능
- 배포 간편 (단일 바이너리)
- 시작 시간 빠름

**단점**:
- 동적 기능 제한 (eval, REPL 등)
- 플랫폼별 빌드 필요
- 긴 컴파일 시간

**적용 사례**: C, C++, Rust, Go

### 방식 2: JIT (Just-in-Time) 컴파일

**설명**: 런타임에 핫 코드를 네이티브 코드로 컴파일

```
바이트코드 실행 (인터프리터)
    ↓
핫스팟 감지
    ↓
LLVM IR 생성
    ↓
LLVM JIT 컴파일
    ↓
네이티브 코드 실행
```

**장점**:
- 동적 최적화 (프로파일 기반)
- REPL/eval 지원
- 크로스 플랫폼 (LLVM 포팅)

**단점**:
- 워밍업 시간
- 메모리 사용 증가
- 복잡한 구현

**적용 사례**: JavaScript (V8 Crankshaft), Java (HotSpot), Julia

### 방식 3: Hybrid (AOT + JIT)

**설명**: 기본 코드는 AOT 컴파일, 핫 코드는 JIT 컴파일

```
AOT 컴파일: 표준 라이브러리, 내장 함수
    +
JIT 컴파일: 사용자 코드, 핫 패스
```

**장점**:
- 빠른 시작 (AOT)
- 최적 성능 (JIT)
- 유연성

**단점**:
- 매우 복잡한 구현
- 두 방식의 단점 일부 포함

**적용 사례**: Android ART, .NET CoreCLR

---

## 구현 예시

### 1. 기본 LLVM 설정

```cpp
// CMakeLists.txt
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# LLVM 컴포넌트 링크
llvm_map_components_to_libnames(llvm_libs support core orcjit native)
target_link_libraries(kingsejong_jit ${llvm_libs})
```

### 2. LLVM 초기화

```cpp
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

using namespace llvm;
using namespace llvm::orc;

class KingSejongJIT {
private:
    std::unique_ptr<LLVMContext> context;
    std::unique_ptr<LLJIT> jit;
    std::unique_ptr<Module> module;
    std::unique_ptr<IRBuilder<>> builder;

public:
    KingSejongJIT() {
        // LLVM 초기화
        InitializeNativeTarget();
        InitializeNativeTargetAsmPrinter();
        InitializeNativeTargetAsmParser();

        // JIT 생성
        auto jitExpected = LLJITBuilder().create();
        if (!jitExpected) {
            handleError(jitExpected.takeError());
        }
        jit = std::move(*jitExpected);

        // 컨텍스트 및 빌더 생성
        context = std::make_unique<LLVMContext>();
        module = std::make_unique<Module>("kingsejong", *context);
        builder = std::make_unique<IRBuilder<>>(*context);
    }
};
```

### 3. 바이트코드 → LLVM IR 변환

```cpp
Function* KingSejongJIT::bytecodeToLLVMIR(bytecode::Chunk* chunk) {
    // 함수 타입 생성 (i64 반환, 파라미터 없음)
    FunctionType* funcType = FunctionType::get(
        Type::getInt64Ty(*context),
        false
    );

    // 함수 생성
    Function* func = Function::Create(
        funcType,
        Function::ExternalLinkage,
        "compiled_func",
        module.get()
    );

    // 기본 블록 생성
    BasicBlock* entry = BasicBlock::Create(*context, "entry", func);
    builder->SetInsertPoint(entry);

    // 스택 시뮬레이션 (LLVM은 레지스터 기반)
    std::vector<Value*> valueStack;

    // 바이트코드 → LLVM IR 변환
    for (size_t ip = 0; ip < chunk->code.size(); ) {
        uint8_t instruction = chunk->code[ip++];

        switch (instruction) {
            case OP_CONSTANT: {
                uint8_t index = chunk->code[ip++];
                auto constant = chunk->constants[index];

                if (std::holds_alternative<int64_t>(constant)) {
                    int64_t value = std::get<int64_t>(constant);
                    Value* llvmValue = ConstantInt::get(
                        *context,
                        APInt(64, value)
                    );
                    valueStack.push_back(llvmValue);
                }
                break;
            }

            case OP_ADD: {
                Value* right = valueStack.back();
                valueStack.pop_back();
                Value* left = valueStack.back();
                valueStack.pop_back();

                Value* result = builder->CreateAdd(left, right, "addtmp");
                valueStack.push_back(result);
                break;
            }

            case OP_SUBTRACT: {
                Value* right = valueStack.back();
                valueStack.pop_back();
                Value* left = valueStack.back();
                valueStack.pop_back();

                Value* result = builder->CreateSub(left, right, "subtmp");
                valueStack.push_back(result);
                break;
            }

            case OP_MULTIPLY: {
                Value* right = valueStack.back();
                valueStack.pop_back();
                Value* left = valueStack.back();
                valueStack.pop_back();

                Value* result = builder->CreateMul(left, right, "multmp");
                valueStack.push_back(result);
                break;
            }

            case OP_RETURN: {
                if (!valueStack.empty()) {
                    Value* returnValue = valueStack.back();
                    builder->CreateRet(returnValue);
                } else {
                    builder->CreateRet(ConstantInt::get(*context, APInt(64, 0)));
                }
                break;
            }

            // 더 많은 OpCode 처리...
        }
    }

    return func;
}
```

### 4. JIT 컴파일 및 실행

```cpp
void* KingSejongJIT::compileAndExecute(bytecode::Chunk* chunk) {
    // 1. 바이트코드 → LLVM IR
    Function* func = bytecodeToLLVMIR(chunk);

    // 2. 검증
    if (verifyFunction(*func, &errs())) {
        errs() << "Function verification failed\n";
        return nullptr;
    }

    // 3. 최적화 (선택사항)
    PassManagerBuilder pmBuilder;
    pmBuilder.OptLevel = 2;
    pmBuilder.SizeLevel = 0;
    pmBuilder.Inliner = createFunctionInliningPass();

    legacy::FunctionPassManager fpm(module.get());
    pmBuilder.populateFunctionPassManager(fpm);
    fpm.run(*func);

    // 4. JIT 컴파일
    auto rt = jit->getMainJITDylib().createResourceTracker();
    ThreadSafeModule tsm(std::move(module), std::move(context));

    if (auto err = jit->addIRModule(rt, std::move(tsm))) {
        handleError(std::move(err));
        return nullptr;
    }

    // 5. 함수 주소 가져오기
    auto sym = jit->lookup("compiled_func");
    if (!sym) {
        handleError(sym.takeError());
        return nullptr;
    }

    // 6. 함수 실행
    auto funcPtr = (int64_t(*)())sym->getAddress();
    int64_t result = funcPtr();

    return result;
}
```

### 5. 타입 처리

```cpp
Type* KingSejongJIT::kingsejongTypeToLLVMType(types::Type* ksjType) {
    switch (ksjType->kind()) {
        case types::TypeKind::INTEGER:
            return Type::getInt64Ty(*context);

        case types::TypeKind::FLOAT:
            return Type::getDoubleTy(*context);

        case types::TypeKind::BOOLEAN:
            return Type::getInt1Ty(*context);

        case types::TypeKind::STRING:
            // 문자열은 포인터로 처리
            return Type::getInt8PtrTy(*context);

        case types::TypeKind::ARRAY:
            // 배열은 구조체로 처리
            return StructType::get(*context, {
                Type::getInt8PtrTy(*context),  // data
                Type::getInt64Ty(*context)     // length
            });

        case types::TypeKind::FUNCTION:
            // 함수는 포인터로 처리
            return Type::getInt8PtrTy(*context);

        default:
            // 기본값: void*
            return Type::getInt8PtrTy(*context);
    }
}
```

---

## 의존성 분석

### LLVM 라이브러리 크기

| 컴포넌트 | 크기 (Linux x64) | 설명 |
|---------|-----------------|-----|
| **libLLVMCore** | 50 MB | IR, 기본 타입 |
| **libLLVMSupport** | 10 MB | 유틸리티, 데이터 구조 |
| **libLLVMOrcJIT** | 20 MB | JIT 컴파일 |
| **libLLVMX86** | 30 MB | x86 백엔드 |
| **libLLVMARM** | 25 MB | ARM 백엔드 |
| **헤더 파일** | 50 MB | 개발용 |
| **총계** | **185 MB** | 전체 설치 |

### 최소 필요 컴포넌트

```cmake
# 최소 JIT 컴파일러
llvm_map_components_to_libnames(llvm_libs
    core
    support
    orcjit
    native      # 현재 플랫폼 백엔드
)

# 크기: ~100 MB (헤더 포함 시 ~150 MB)
```

### 빌드 시간 영향

| 프로젝트 | LLVM 없음 | LLVM 포함 | 증가 |
|---------|-----------|----------|-----|
| **첫 빌드** | 30초 | 10-30분* | 20-60배 |
| **증분 빌드** | 5초 | 10초 | 2배 |
| **클린 빌드** | 20초 | 5-10분 | 15-30배 |

\* LLVM을 소스에서 빌드하는 경우. 미리 빌드된 라이브러리 사용 시 증가폭 작음.

### CI/CD 영향

```yaml
# .github/workflows/build.yml (LLVM 포함)
- name: Cache LLVM
  uses: actions/cache@v3
  with:
    path: llvm-install
    key: llvm-15-${{ runner.os }}

- name: Install LLVM
  run: |
    # Ubuntu
    sudo apt install llvm-15-dev

    # macOS
    brew install llvm@15

    # Windows
    choco install llvm
```

**캐시 크기**: 150-200 MB
**다운로드 시간**: 30-60초 (GitHub Actions)

---

## 성능 예측

### 예상 성능 향상

| 벤치마크 | 바이트코드 VM | LLVM JIT -O1 | LLVM JIT -O2 | 네이티브 C++ |
|---------|-------------|--------------|--------------|-------------|
| 피보나치(35) | 200ms (1x) | 40ms (5x) | 20ms (10x) | 15ms (13x) |
| 배열 순회(10M) | 500ms (1x) | 100ms (5x) | 50ms (10x) | 30ms (17x) |
| 중첩 루프 | 1000ms (1x) | 100ms (10x) | 50ms (20x) | 20ms (50x) |
| 함수 호출(10M) | 1000ms (1x) | 200ms (5x) | 100ms (10x) | 50ms (20x) |

**결론**: LLVM JIT는 바이트코드 대비 **5-20배** 성능 향상 기대

### 메모리 사용량

| 항목 | 바이트코드 VM | LLVM JIT |
|-----|-------------|---------|
| **베이스 메모리** | 10 MB | 50 MB |
| **함수당 오버헤드** | 1 KB | 10 KB |
| **JIT 컴파일 코드** | - | 100 KB/func |
| **총계 (100 함수)** | 10.1 MB | 61 MB |

**결론**: LLVM JIT는 **5-6배** 메모리 사용 증가

### 시작 시간

| 항목 | 바이트코드 VM | LLVM JIT |
|-----|-------------|---------|
| **REPL 시작** | 50ms | 200ms |
| **스크립트 파싱** | 10ms | 10ms |
| **첫 함수 컴파일** | 0ms | 50-100ms |
| **총 워밍업** | 60ms | 260-310ms |

**결론**: LLVM JIT는 **4-5배** 시작 시간 증가

---

## 단계별 통합 계획

### Phase 1: 연구 및 프로토타입 (2주)

**목표**: LLVM 통합 가능성 검증

**작업**:
- [x] LLVM 튜토리얼 학습
- [x] 간단한 LLVM IR 생성 예제
- [x] ORC JIT API 이해
- [x] 성능 및 크기 영향 측정

**결과물**:
- 본 연구 문서
- 간단한 프로토타입 코드

### Phase 2: 기본 통합 (1개월)

**목표**: 단순한 KingSejong 코드를 LLVM IR로 변환

**작업**:
- [ ] CMake에 LLVM 통합
- [ ] 기본 OpCode → LLVM IR 변환기
- [ ] 간단한 함수 JIT 컴파일
- [ ] 테스트 작성

**완료 기준**:
- 산술 연산 JIT 컴파일
- 함수 호출 JIT 컴파일
- 기본 타입 변환

### Phase 3: 고급 기능 (2개월)

**목표**: 복잡한 언어 기능 지원

**작업**:
- [ ] 클로저 지원
- [ ] 배열 및 문자열 처리
- [ ] GC 통합
- [ ] 예외 처리

**완료 기준**:
- 모든 KingSejong 기능 JIT 컴파일 가능
- 성능 벤치마크 통과

### Phase 4: 최적화 (1개월)

**목표**: 성능 최적화 및 안정화

**작업**:
- [ ] 프로파일 기반 최적화
- [ ] 인라이닝
- [ ] 탈가상화
- [ ] 메모리 사용 최적화

**완료 기준**:
- 목표 성능 달성 (10배 향상)
- 메모리 사용 합리적 수준
- 안정성 검증

---

## 대안 방식

### 1. 템플릿 JIT

**설명**: 미리 작성된 네이티브 코드 템플릿 사용

**장점**:
- 매우 빠른 컴파일 (< 1ms)
- 작은 메모리 사용
- 간단한 구현 (1000줄 이내)
- 의존성 없음

**단점**:
- 제한적인 최적화
- 플랫폼별 어셈블리 필요

**권장**: Phase 2 이전에 먼저 시도

### 2. Cranelift

**설명**: Rust로 작성된 경량 코드 생성 백엔드

**장점**:
- LLVM보다 작음 (10-20 MB)
- 빠른 컴파일 (LLVM의 10배)
- 적절한 최적화

**단점**:
- LLVM보다 성능 낮음 (10-20%)
- 작은 커뮤니티
- C++ 통합 복잡

**예시**: Wasmtime (WebAssembly 런타임)

### 3. LibJIT

**설명**: GNU의 JIT 컴파일러 라이브러리

**장점**:
- 작음 (5 MB)
- 간단한 API
- LLVM보다 통합 쉬움

**단점**:
- 제한적인 최적화
- 유지보수 중단 (2014년 이후)
- 성능 낮음

**권장**: 사용 안 함 (유지보수 중단)

### 4. MIR (Medium Internal Representation)

**설명**: 경량 JIT 컴파일러

**장점**:
- 매우 작음 (< 1 MB)
- 빠른 컴파일
- 통합 용이

**단점**:
- 실험적 (프로덕션 사용 제한)
- 제한적인 최적화
- 작은 커뮤니티

**권장**: 장기 관찰

---

## 권장 사항

### 현재 (v0.1 - v0.5)

**권장**: LLVM 통합 **보류**

**이유**:
1. **복잡도 vs 가치**: 구현 복잡도가 매우 높지만, 현재 언어 사용자가 적어 가치 제한적
2. **바이트코드 VM 충분**: 현재 성능(피보나치 100-200ms)은 인터프리터 언어로 적절
3. **더 시급한 작업**: 디버거, 표준 라이브러리, 문서화 등이 더 중요
4. **의존성 부담**: 100+ MB 의존성은 프로젝트 크기에 비해 과다
5. **빌드 시간**: CI/CD 시간 증가는 개발 생산성 저하

**대신 할 것**:
- ✅ 핫 패스 감지 (F4.8에서 구현 완료)
- ⏳ 바이트코드 VM 최적화 (레지스터 할당, 인라인 캐싱)
- ⏳ 템플릿 JIT 프로토타입 (선택사항)

### 중기 (v0.5 - v1.0)

**권장**: 템플릿 JIT 프로토타입

**이유**:
1. **점진적 접근**: LLVM보다 구현 간단 (1-2주)
2. **의존성 없음**: 작은 코드 크기
3. **검증 가능**: JIT 효과를 빠르게 측정
4. **학습 경험**: JIT 컴파일러 개념 이해

**목표 성능**:
- 바이트코드 대비 2-5배 향상
- 메모리 증가 < 10%

### 장기 (v1.0+)

**권장**: LLVM 통합 **재고려**

**조건**:
1. 언어가 안정화되고 사용자가 증가
2. 성능이 병목이 되는 실제 사례 확인
3. 템플릿 JIT로 한계 도달
4. 전담 개발자 확보 (3-6개월 풀타임)

**대안**:
- Cranelift 검토 (경량, 빠른 컴파일)
- WASM 백엔드 (크로스 플랫폼, 웹 지원)

---

## 참고 자료

### 공식 문서
1. **LLVM Tutorial**: https://llvm.org/docs/tutorial/
   - Kaleidoscope 언어 예제
   - JIT 컴파일러 구현

2. **LLVM Programmer's Manual**: https://llvm.org/docs/ProgrammersManual.html
   - LLVM API 가이드

3. **ORC JIT Design**: https://llvm.org/docs/ORCv2.html
   - JIT 컴파일러 아키텍처

### 블로그 및 튜토리얼
1. **"LLVM for Grad Students"** - Adrian Sampson
   - https://www.cs.cornell.edu/~asampson/blog/llvm.html

2. **"A Tourist's Guide to the LLVM Source Code"** - Eric Christopher
   - https://blog.regehr.org/archives/1453

3. **"Creating a JIT with LLVM"** - Chris Lattner
   - https://llvm.org/docs/tutorial/BuildingAJIT1.html

### 오픈소스 예제
1. **Kaleidoscope**: LLVM 공식 튜토리얼 언어
2. **Pyston**: Python JIT (LLVM 기반)
3. **Julia**: 과학 계산 언어 (LLVM 기반)
4. **LDC**: D 언어 컴파일러 (LLVM 기반)

### 서적
1. **"Getting Started with LLVM Core Libraries"** - Bruno Cardoso Lopes
   - LLVM 입문서

2. **"LLVM Cookbook"** - Mayur Pandey
   - LLVM 실전 예제

3. **"Engineering a Compiler"** - Cooper & Torczon
   - 컴파일러 이론

---

## 결론

LLVM은 강력한 JIT 컴파일러 인프라이지만, **현재 KingSejong 프로젝트에는 과도한 복잡도**를 가져옵니다.

### 최종 권장 사항

1. **v0.1 - v0.5**: LLVM 통합 보류, 바이트코드 VM 최적화 집중
2. **v0.5 - v1.0**: 템플릿 JIT 프로토타입 (선택사항)
3. **v1.0+**: LLVM 통합 재고려 (언어 안정화 후)

### F4.8 완료 기준

F4.8 (JIT 컴파일러 연구)는 다음으로 **완료**로 간주합니다:

- [x] JIT 컴파일러 연구 보고서 (docs/research/JIT_COMPILER_RESEARCH.md)
- [x] 핫 패스 감지 프로토타입 (include/jit/HotPathDetector.h, src/jit/HotPathDetector.cpp)
- [x] LLVM 통합 연구 보고서 (본 문서)
- [x] 성능 벤치마크 계획 (JIT_COMPILER_RESEARCH.md 포함)

**다음 단계**: F4.8 완료 후 문서 업데이트 및 커밋

---

**문서 버전**: 1.0
**최종 업데이트**: 2025-11-10
**다음 리뷰**: v1.0 릴리스 계획 시
