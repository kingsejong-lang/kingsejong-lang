/**
 * @file VM.cpp
 * @brief VM 구현
 * @author KingSejong Team
 * @date 2025-11-10
 */

#include "VM.h"
#include "jit/JITCompilerT1.h"
#include "jit/HotPathDetector.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Environment.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace kingsejong {
namespace bytecode {

VM::VM()
    : chunk_(nullptr), ip_(0), traceExecution_(false),
      instructionCount_(0), maxInstructions_(VM_DEFAULT_MAX_INSTRUCTIONS),
      maxExecutionTime_(VM_DEFAULT_MAX_EXECUTION_TIME_MS),
      maxStackSize_(VM_DEFAULT_MAX_STACK_SIZE),
      jitEnabled_(true) {  // JIT 기본 활성화
    globals_ = std::make_shared<evaluator::Environment>();

    // JIT 컴파일러 초기화
    jitCompiler_ = std::make_unique<jit::JITCompilerT1>();

    // Hot Path Detector 초기화
    hotPathDetector_ = std::make_unique<jit::HotPathDetector>();
    hotPathDetector_->setLoopThreshold(JIT_LOOP_THRESHOLD);
}

VM::~VM() {
    // JIT 캐시 정리 (NativeFunction은 JITCompiler가 관리)
    jitCache_.clear();
    // unique_ptr이 자동으로 JIT 컴파일러와 Hot Path Detector를 정리합니다
}

VMResult VM::run(Chunk* chunk) {
    chunk_ = chunk;
    ip_ = 0;
    stack_.clear();
    instructionCount_ = 0;
    startTime_ = std::chrono::steady_clock::now();

    try {
        while (true) {
            // 안전 장치 1: 명령어 수 제한
            if (++instructionCount_ > maxInstructions_) {
                runtimeError("최대 명령어 실행 횟수 초과 (" + std::to_string(maxInstructions_) + " 초과). 무한 루프 의심.");
                return VMResult::RUNTIME_ERROR;
            }

            // 안전 장치 2: 실행 시간 제한
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_);
            if (elapsed > maxExecutionTime_) {
                runtimeError("최대 실행 시간 초과 (" + std::to_string(maxExecutionTime_.count()) + "ms 초과). 무한 루프 또는 긴 연산 의심.");
                return VMResult::RUNTIME_ERROR;
            }

            if (traceExecution_) {
                printStack();
                chunk_->disassembleInstruction(ip_);
            }

            VMResult result = executeInstruction();
            if (result == VMResult::HALT) {
                return VMResult::OK;  // 정상 종료
            } else if (result != VMResult::OK) {
                return result;
            }
        }
    } catch (const std::exception& e) {
        runtimeError(e.what());
        return VMResult::RUNTIME_ERROR;
    }
}

evaluator::Value VM::top() const {
    if (stack_.empty()) {
        return evaluator::Value::createNull();
    }
    return stack_.back();
}

uint8_t VM::readByte() {
    return chunk_->read(ip_++);
}

uint16_t VM::readShort() {
    uint16_t value = chunk_->read16(ip_);
    ip_ += 2;
    return value;
}

evaluator::Value VM::readConstant() {
    uint8_t idx = readByte();
    return chunk_->getConstant(idx);
}

std::string VM::readName() {
    uint8_t idx = readByte();
    return chunk_->getName(idx);
}

void VM::push(const evaluator::Value& value) {
    // 안전 장치 3: 스택 크기 제한
    if (stack_.size() >= maxStackSize_) {
        throw std::runtime_error("최대 스택 크기 초과 (" + std::to_string(maxStackSize_) + " 초과). 재귀 호출 또는 메모리 폭주 의심.");
    }
    stack_.push_back(value);
}

evaluator::Value VM::pop() {
    if (stack_.empty()) {
        throw std::runtime_error("Stack underflow");
    }
    evaluator::Value value = stack_.back();
    stack_.pop_back();
    return value;
}

evaluator::Value VM::peek(int distance) const {
    if (distance >= static_cast<int>(stack_.size())) {
        throw std::runtime_error("Stack peek out of range");
    }
    return stack_[stack_.size() - 1 - distance];
}

void VM::runtimeError(const std::string& message) {
    std::cerr << "[런타임 오류] " << message << "\n";
    std::cerr << "[라인 " << chunk_->getLine(ip_ - 1) << "]\n";

    // 스택 추적
    std::cerr << "스택 추적:\n";
    printStack();
}

void VM::printStack() const {
    std::cout << "스택: ";
    if (stack_.empty()) {
        std::cout << "(비어있음)";
    } else {
        for (const auto& value : stack_) {
            std::cout << "[ " << value.toString() << " ] ";
        }
    }
    std::cout << "\n";
}

VMResult VM::executeInstruction() {
    OpCode instruction = static_cast<OpCode>(readByte());

    switch (instruction) {
        // ========================================
        // 상수 로드
        // ========================================
        case OpCode::LOAD_CONST: {
            evaluator::Value constant = readConstant();
            push(constant);
            break;
        }

        case OpCode::LOAD_TRUE:
            push(evaluator::Value::createBoolean(true));
            break;

        case OpCode::LOAD_FALSE:
            push(evaluator::Value::createBoolean(false));
            break;

        case OpCode::LOAD_NULL:
            push(evaluator::Value::createNull());
            break;

        // ========================================
        // 변수 조작
        // ========================================
        case OpCode::LOAD_VAR: {
            uint8_t slot = readByte();
            if (slot >= stack_.size()) {
                runtimeError("로컬 변수 인덱스 범위 초과");
                return VMResult::RUNTIME_ERROR;
            }
            push(stack_[slot]);
            break;
        }

        case OpCode::STORE_VAR: {
            uint8_t slot = readByte();
            if (slot >= stack_.size()) {
                // 스택 확장
                stack_.resize(slot + 1, evaluator::Value::createNull());
            }
            stack_[slot] = peek(0);
            break;
        }

        case OpCode::LOAD_GLOBAL: {
            std::string name = readName();
            try {
                evaluator::Value value = globals_->get(name);
                push(value);
            } catch (const std::exception&) {
                runtimeError("정의되지 않은 변수: " + name);
                return VMResult::RUNTIME_ERROR;
            }
            break;
        }

        case OpCode::STORE_GLOBAL: {
            std::string name = readName();
            globals_->set(name, peek(0));
            break;
        }

        // ========================================
        // 산술 연산
        // ========================================
        case OpCode::ADD:
        case OpCode::SUB:
        case OpCode::MUL:
        case OpCode::DIV:
        case OpCode::MOD:
            return binaryOp(instruction);

        case OpCode::NEG: {
            evaluator::Value value = pop();
            if (value.isInteger()) {
                push(evaluator::Value::createInteger(-value.asInteger()));
            } else if (value.isFloat()) {
                push(evaluator::Value::createFloat(-value.asFloat()));
            } else {
                runtimeError("피연산자는 숫자여야 합니다");
                return VMResult::RUNTIME_ERROR;
            }
            break;
        }

        // ========================================
        // 비교 연산
        // ========================================
        case OpCode::EQ: {
            evaluator::Value b = pop();
            evaluator::Value a = pop();
            push(evaluator::Value::createBoolean(a.equals(b)));
            break;
        }

        case OpCode::NE: {
            evaluator::Value b = pop();
            evaluator::Value a = pop();
            push(evaluator::Value::createBoolean(!a.equals(b)));
            break;
        }

        case OpCode::LT: {
            evaluator::Value b = pop();
            evaluator::Value a = pop();
            push(evaluator::Value::createBoolean(a.lessThan(b)));
            break;
        }

        case OpCode::GT: {
            evaluator::Value b = pop();
            evaluator::Value a = pop();
            push(evaluator::Value::createBoolean(a.greaterThan(b)));
            break;
        }

        case OpCode::LE: {
            evaluator::Value b = pop();
            evaluator::Value a = pop();
            push(evaluator::Value::createBoolean(a.lessThan(b) || a.equals(b)));
            break;
        }

        case OpCode::GE: {
            evaluator::Value b = pop();
            evaluator::Value a = pop();
            push(evaluator::Value::createBoolean(a.greaterThan(b) || a.equals(b)));
            break;
        }

        // ========================================
        // 논리 연산
        // ========================================
        case OpCode::AND: {
            evaluator::Value b = pop();
            evaluator::Value a = pop();
            push(evaluator::Value::createBoolean(a.isTruthy() && b.isTruthy()));
            break;
        }

        case OpCode::OR: {
            evaluator::Value b = pop();
            evaluator::Value a = pop();
            push(evaluator::Value::createBoolean(a.isTruthy() || b.isTruthy()));
            break;
        }

        case OpCode::NOT: {
            evaluator::Value value = pop();
            push(evaluator::Value::createBoolean(!value.isTruthy()));
            break;
        }

        // ========================================
        // 제어 흐름
        // ========================================
        case OpCode::JUMP: {
            uint8_t offset = readByte();
            ip_ += offset;
            break;
        }

        case OpCode::JUMP_IF_FALSE: {
            uint8_t offset = readByte();
            if (!peek(0).isTruthy()) {
                ip_ += offset;
            }
            break;
        }

        case OpCode::JUMP_IF_TRUE: {
            uint8_t offset = readByte();
            if (peek(0).isTruthy()) {
                ip_ += offset;
            }
            break;
        }

        case OpCode::LOOP: {
            uint8_t offset = readByte();
            size_t loopStart = ip_ - offset;

            // JIT 컴파일 및 실행
            if (jitEnabled_) {
                // 루프 백엣지 추적
                hotPathDetector_->trackLoopBackedge(loopStart);

                // 디버그: 루프 추적 확인
                // 루프 프로파일 정보는 HotPathDetector가 관리

                // JIT 캐시 확인: 이미 컴파일된 코드가 있는지
                auto it = jitCache_.find(loopStart);

                if (it != jitCache_.end()) {
                    // JIT 코드가 존재 - 실행!
                    VMResult jitResult = executeJITCode(it->second);

                    if (jitResult == VMResult::OK) {
                        // JIT 실행 성공 - 루프 전체가 완료됨
                        // executeJITCode가 스택 = [dummy, dummy, result] 상태로 남겨놨음
                        // 첫 번째 POP부터 실행해야 cleanup이 올바르게 작동함
                        // exitOffset은 JUMP_IF_FALSE target이므로 -1 필요
                        ip_ = it->second->exitOffset - 1;
                        break;  // LOOP OpCode 종료, 다음 명령어로 진행
                    } else {
                        // JIT 실행 실패 - 인터프리터로 폴백
                        std::cerr << "[VM] JIT 실행 실패, 인터프리터로 폴백\n";
                    }
                } else if (hotPathDetector_->isHot(loopStart, jit::HotPathType::LOOP)) {
                    // 핫 루프이지만 아직 컴파일되지 않았으면 컴파일
                    tryJITCompileLoop(loopStart);
                    // 이번 반복은 인터프리터로 계속 진행
                }
            }

            // 인터프리터 폴백: 루프 백점프 실행
            ip_ -= offset;
            break;
        }

        // ========================================
        // 배열
        // ========================================
        case OpCode::BUILD_ARRAY: {
            uint8_t count = readByte();
            std::vector<evaluator::Value> elements;

            for (int i = 0; i < count; i++) {
                elements.insert(elements.begin(), pop());
            }

            push(evaluator::Value::createArray(elements));
            break;
        }

        case OpCode::INDEX_GET: {
            evaluator::Value index = pop();
            evaluator::Value array = pop();

            if (!array.isArray()) {
                runtimeError("배열이 아닙니다");
                return VMResult::RUNTIME_ERROR;
            }

            if (!index.isInteger()) {
                runtimeError("인덱스는 정수여야 합니다");
                return VMResult::RUNTIME_ERROR;
            }

            int idx = static_cast<int>(index.asInteger());
            auto& arr = array.asArray();

            if (idx < 0 || idx >= static_cast<int>(arr.size())) {
                runtimeError("인덱스 범위 초과");
                return VMResult::RUNTIME_ERROR;
            }

            push(arr[idx]);
            break;
        }

        // ========================================
        // 스택 조작
        // ========================================
        case OpCode::POP: {
            pop();
            break;
        }

        case OpCode::DUP:
            push(peek(0));
            break;

        case OpCode::SWAP: {
            evaluator::Value a = pop();
            evaluator::Value b = pop();
            push(a);
            push(b);
            break;
        }

        // ========================================
        // 기타
        // ========================================
        case OpCode::PRINT: {
            evaluator::Value value = pop();
            std::cout << value.toString() << "\n";
            break;
        }

        case OpCode::HALT:
            return VMResult::HALT;

        // ========================================
        // 미구현
        // ========================================
        // ========================================
        // 함수 관련
        // ========================================
        case OpCode::BUILD_FUNCTION: {
            uint8_t paramCount = readByte();
            uint8_t addrHigh = readByte();
            uint8_t addrLow = readByte();
            size_t funcAddr = (static_cast<size_t>(addrHigh) << 8) | addrLow;

            // 간단화된 함수 표현: [타입=정수, 값1=주소, 값2=파라미터개수]
            // Value에 정수로 인코딩 (주소 << 8 | 파라미터개수)
            int64_t encoded = (static_cast<int64_t>(funcAddr) << 8) | paramCount;
            push(evaluator::Value::createInteger(encoded));
            break;
        }

        case OpCode::CALL: {
            uint8_t argCount = readByte();

            // 함수 가져오기 (인코딩된 정수)
            evaluator::Value funcVal = peek(argCount);
            if (!funcVal.isInteger()) {
                runtimeError("함수가 아닌 값을 호출하려고 했습니다");
                return VMResult::RUNTIME_ERROR;
            }

            int64_t encoded = funcVal.asInteger();
            size_t funcAddr = static_cast<size_t>((encoded >> 8) & FUNC_ADDR_MASK);

            // CallFrame 저장
            frames_.push_back({ip_, stack_.size() - argCount});

            // 함수로 점프
            ip_ = funcAddr;
            break;
        }

        case OpCode::RETURN: {
            evaluator::Value result = pop();

            if (frames_.empty()) {
                // 최상위 레벨 return: 프로그램 종료
                push(result);
                return VMResult::OK;
            }

            // CallFrame 복원
            CallFrame frame = frames_.back();
            frames_.pop_back();

            // 스택 정리: 함수와 인자들 제거
            while (stack_.size() > frame.stackBase) {
                pop();
            }

            // 반환값 푸시
            push(result);

            // IP 복원
            ip_ = frame.returnAddress;
            break;
        }

        case OpCode::JOSA_CALL: {
            [[maybe_unused]] uint8_t josaType = readByte();
            uint8_t methodIdx = readByte();

            // 객체
            evaluator::Value obj = pop();

            // 메서드 이름
            std::string methodName = chunk_->getName(methodIdx);

            // 조사 기반 메서드 호출 (간단화: 내장 메서드만 지원)
            // 실제로는 evaluator의 조사 표현식 평가 로직 사용해야 함
            // 현재는 기본적인 배열 메서드만 지원

            if (obj.isArray()) {
                auto& arr = obj.asArray();

                if (methodName == "정렬한다" || methodName == "정렬") {
                    // 정렬 (간단화: 정수 배열만)
                    std::vector<evaluator::Value> sorted = arr;
                    std::sort(sorted.begin(), sorted.end(),
                              [](const evaluator::Value& a, const evaluator::Value& b) {
                                  if (a.isInteger() && b.isInteger()) {
                                      return a.asInteger() < b.asInteger();
                                  }
                                  return false;
                              });
                    push(evaluator::Value::createArray(sorted));
                } else if (methodName == "역순으로_나열한다" || methodName == "역순") {
                    std::vector<evaluator::Value> reversed = arr;
                    std::reverse(reversed.begin(), reversed.end());
                    push(evaluator::Value::createArray(reversed));
                } else {
                    runtimeError("알 수 없는 배열 메서드: " + methodName);
                    return VMResult::RUNTIME_ERROR;
                }
            } else {
                runtimeError("조사 표현식: 지원되지 않는 타입");
                return VMResult::RUNTIME_ERROR;
            }
            break;
        }

        // ========================================
        // Phase 7.1: 클래스 시스템
        // ========================================
        case OpCode::CLASS_DEF: {
            // CLASS_DEF [class_name_index] [field_count] [method_count]
            uint8_t classNameIdx = readByte();
            uint8_t fieldCount = readByte();
            uint8_t methodCount = readByte();

            // 클래스 이름 읽기
            evaluator::Value classNameVal = chunk_->getConstant(classNameIdx);
            std::string className = classNameVal.asString();

            // 필드 이름들 읽기
            std::vector<std::string> fieldNames;
            for (uint8_t i = 0; i < fieldCount; ++i) {
                uint8_t fieldNameIdx = readByte();
                evaluator::Value fieldNameVal = chunk_->getConstant(fieldNameIdx);
                fieldNames.push_back(fieldNameVal.asString());
            }

            // 생성자 읽기 (있으면)
            uint8_t ctorIdx = readByte();
            std::shared_ptr<evaluator::Function> constructor = nullptr;
            if (ctorIdx != NO_CONSTRUCTOR_FLAG) {
                evaluator::Value ctorVal = chunk_->getConstant(ctorIdx);
                if (ctorVal.isFunction()) {
                    constructor = ctorVal.asFunction();
                }
            }

            // 메서드들 읽기
            std::unordered_map<std::string, std::shared_ptr<evaluator::Function>> methods;
            for (uint8_t i = 0; i < methodCount; ++i) {
                uint8_t methodNameIdx = readByte();
                uint8_t methodFuncIdx = readByte();

                evaluator::Value methodNameVal = chunk_->getConstant(methodNameIdx);
                std::string methodName = methodNameVal.asString();

                evaluator::Value methodFuncVal = chunk_->getConstant(methodFuncIdx);
                if (methodFuncVal.isFunction()) {
                    methods[methodName] = methodFuncVal.asFunction();
                }
            }

            // ClassDefinition 생성
            auto classDef = std::make_shared<evaluator::ClassDefinition>(
                className,
                fieldNames,
                methods,
                constructor,
                ""        // superClass
            );

            // 클래스 정의 저장
            classes_[className] = classDef;

            // 클래스 정의를 스택에 푸시 (STORE_GLOBAL에서 사용)
            push(evaluator::Value::createString(className));
            break;
        }

        case OpCode::NEW_INSTANCE: {
            // NEW_INSTANCE [class_name_index] [arg_count]
            uint8_t classNameIdx = readByte();
            uint8_t argCount = readByte();

            // 클래스 이름 읽기
            evaluator::Value classNameVal = chunk_->getConstant(classNameIdx);
            std::string className = classNameVal.asString();

            // 클래스 정의 찾기
            auto it = classes_.find(className);
            if (it == classes_.end()) {
                runtimeError("정의되지 않은 클래스: " + className);
                return VMResult::RUNTIME_ERROR;
            }

            // 인자들 팝
            std::vector<evaluator::Value> args;
            for (uint8_t i = 0; i < argCount; ++i) {
                args.insert(args.begin(), pop());
            }

            // ClassInstance 생성
            auto instance = std::make_shared<evaluator::ClassInstance>(it->second);

            // 생성자 호출 (있으면)
            auto constructor = it->second->constructor();
            if (constructor) {
                // 생성자 환경 생성
                auto ctorEnv = std::make_shared<evaluator::Environment>(globals_);

                // 매개변수 바인딩
                const auto& params = constructor->parameters();
                if (params.size() != args.size()) {
                    runtimeError("생성자 인자 개수 불일치: 예상 " +
                                std::to_string(params.size()) + ", 실제 " +
                                std::to_string(args.size()));
                    return VMResult::RUNTIME_ERROR;
                }

                for (size_t i = 0; i < params.size(); ++i) {
                    ctorEnv->set(params[i], args[i]);
                }

                // "자신" 바인딩
                ctorEnv->set("자신", evaluator::Value::createClassInstance(instance));
                ctorEnv->set("이", evaluator::Value::createClassInstance(instance));

                // 생성자 본문 실행
                evaluator::Evaluator evaluator(ctorEnv);
                evaluator::Value result = evaluator.eval(constructor->body());

                // 에러 체크
                if (result.isError()) {
                    runtimeError("생성자 실행 오류: " + result.asString());
                    return VMResult::RUNTIME_ERROR;
                }
            }

            // 인스턴스를 스택에 푸시
            push(evaluator::Value::createClassInstance(instance));
            break;
        }

        case OpCode::LOAD_FIELD: {
            // LOAD_FIELD [field_name_index]
            uint8_t fieldNameIdx = readByte();
            evaluator::Value fieldNameVal = chunk_->getConstant(fieldNameIdx);
            std::string fieldName = fieldNameVal.asString();

            // 스택에서 객체 팝
            evaluator::Value objVal = pop();
            if (!objVal.isClassInstance()) {
                runtimeError("필드 접근: 클래스 인스턴스가 아닙니다");
                return VMResult::RUNTIME_ERROR;
            }

            auto instance = objVal.asClassInstance();
            try {
                evaluator::Value fieldValue = instance->getField(fieldName);
                push(fieldValue);
            } catch (const std::exception& e) {
                runtimeError(std::string("필드 접근 오류: ") + e.what());
                return VMResult::RUNTIME_ERROR;
            }
            break;
        }

        case OpCode::STORE_FIELD: {
            // STORE_FIELD [field_name_index]
            uint8_t fieldNameIdx = readByte();
            evaluator::Value fieldNameVal = chunk_->getConstant(fieldNameIdx);
            std::string fieldName = fieldNameVal.asString();

            // 스택에서 값과 객체 팝
            evaluator::Value value = pop();
            evaluator::Value objVal = pop();

            if (!objVal.isClassInstance()) {
                runtimeError("필드 설정: 클래스 인스턴스가 아닙니다");
                return VMResult::RUNTIME_ERROR;
            }

            auto instance = objVal.asClassInstance();
            try {
                instance->setField(fieldName, value);
                push(value);  // 대입 결과를 스택에 푸시
            } catch (const std::exception& e) {
                runtimeError(std::string("필드 설정 오류: ") + e.what());
                return VMResult::RUNTIME_ERROR;
            }
            break;
        }

        case OpCode::CALL_METHOD: {
            // CALL_METHOD [method_name_index] [arg_count]
            uint8_t methodNameIdx = readByte();
            uint8_t argCount = readByte();

            // 메서드 이름 읽기
            evaluator::Value methodNameVal = chunk_->getConstant(methodNameIdx);
            std::string methodName = methodNameVal.asString();

            // 인자들 팝
            std::vector<evaluator::Value> args;
            for (uint8_t i = 0; i < argCount; ++i) {
                args.insert(args.begin(), pop());
            }

            // 객체 팝
            evaluator::Value objVal = pop();
            if (!objVal.isClassInstance()) {
                runtimeError("메서드 호출: 클래스 인스턴스가 아닙니다");
                return VMResult::RUNTIME_ERROR;
            }

            auto instance = objVal.asClassInstance();
            auto classDef = instance->classDef();

            // 메서드 찾기
            auto method = classDef->getMethod(methodName);
            if (!method) {
                runtimeError("정의되지 않은 메서드: " + methodName);
                return VMResult::RUNTIME_ERROR;
            }

            // 메서드 환경 생성
            auto methodEnv = std::make_shared<evaluator::Environment>(globals_);

            // 매개변수 바인딩
            const auto& params = method->parameters();
            if (params.size() != args.size()) {
                runtimeError("메서드 인자 개수 불일치: 예상 " +
                            std::to_string(params.size()) + ", 실제 " +
                            std::to_string(args.size()));
                return VMResult::RUNTIME_ERROR;
            }

            for (size_t i = 0; i < params.size(); ++i) {
                methodEnv->set(params[i], args[i]);
            }

            // "자신"/"이" 바인딩
            methodEnv->set("자신", objVal);
            methodEnv->set("이", objVal);

            // 메서드 본문 실행
            evaluator::Evaluator evaluator(methodEnv);
            evaluator::Value result = evaluator.eval(method->body());

            // 에러 체크
            if (result.isError()) {
                runtimeError("메서드 실행 오류: " + result.asString());
                return VMResult::RUNTIME_ERROR;
            }

            // 결과 푸시
            push(result);
            break;
        }

        case OpCode::LOAD_THIS: {
            // this 스택에서 현재 인스턴스 가져오기
            if (thisStack_.empty()) {
                runtimeError("메서드 또는 생성자 외부에서 'this'를 사용할 수 없습니다");
                return VMResult::RUNTIME_ERROR;
            }

            auto instance = thisStack_.back();
            push(evaluator::Value::createClassInstance(instance));
            break;
        }

        // Phase 7.3: 비동기 OpCode
        case OpCode::BUILD_ASYNC_FUNC: {
            uint8_t paramCount = readByte();
            uint8_t addrHigh = readByte();
            uint8_t addrLow = readByte();
            size_t funcAddr = (static_cast<size_t>(addrHigh) << 8) | addrLow;

            // 비동기 함수는 상위 비트로 표시 (일반 함수와 구분)
            // 인코딩: (funcAddr << 8) | paramCount | 0x80000000 (async 플래그)
            int64_t encoded = (static_cast<int64_t>(funcAddr) << 8) | paramCount | (1LL << 31);
            push(evaluator::Value::createInteger(encoded));
            break;
        }

        case OpCode::ASYNC_CALL: {
            uint8_t argCount = readByte();

            // 함수 가져오기
            evaluator::Value funcVal = peek(argCount);
            if (!funcVal.isInteger()) {
                runtimeError("비동기 함수가 아닌 값을 호출하려고 했습니다");
                return VMResult::RUNTIME_ERROR;
            }

            int64_t encoded = funcVal.asInteger();
            size_t funcAddr = static_cast<size_t>((encoded >> 8) & FUNC_ADDR_MASK);

            // CallFrame 저장
            frames_.push_back({ip_, stack_.size() - argCount});

            // 함수로 점프
            ip_ = funcAddr;

            // Promise 생성하여 반환 (간단한 구현)
            auto promise = std::make_shared<evaluator::Promise>();
            push(evaluator::Value::createPromise(promise));
            break;
        }

        case OpCode::AWAIT: {
            // Promise를 꺼내서 resolved value를 반환
            evaluator::Value promiseVal = pop();
            if (promiseVal.isPromise()) {
                auto promise = promiseVal.asPromise();
                if (promise->state() == evaluator::Promise::State::FULFILLED) {
                    push(promise->value());
                } else if (promise->state() == evaluator::Promise::State::REJECTED) {
                    runtimeError("Promise가 거부되었습니다: " + promise->value().toString());
                    return VMResult::RUNTIME_ERROR;
                } else {
                    // Pending 상태 - 간단한 구현에서는 null 반환
                    push(evaluator::Value::createNull());
                }
            } else {
                // Promise가 아니면 그대로 반환
                push(promiseVal);
            }
            break;
        }

        case OpCode::BUILD_PROMISE: {
            // 새 Promise 생성
            auto promise = std::make_shared<evaluator::Promise>();
            push(evaluator::Value::createPromise(promise));
            break;
        }

        case OpCode::PROMISE_RESOLVE: {
            // Promise를 resolve
            evaluator::Value value = pop();
            evaluator::Value promiseVal = pop();
            if (promiseVal.isPromise()) {
                promiseVal.asPromise()->resolve(value);
            }
            break;
        }

        case OpCode::PROMISE_REJECT: {
            // Promise를 reject
            evaluator::Value reason = pop();
            evaluator::Value promiseVal = pop();
            if (promiseVal.isPromise()) {
                promiseVal.asPromise()->reject(reason);
            }
            break;
        }

        case OpCode::PROMISE_THEN:
        case OpCode::PROMISE_CATCH:
            // then/catch는 VM에서 간단히 처리 (완전한 구현은 Evaluator 사용)
            runtimeError("Promise then/catch는 VM에서 아직 완전히 지원되지 않습니다");
            return VMResult::RUNTIME_ERROR;

        case OpCode::INDEX_SET:
        case OpCode::ARRAY_APPEND:
        case OpCode::BUILD_RANGE:
        case OpCode::IMPORT:
            runtimeError("아직 구현되지 않은 명령어: " + opCodeToString(instruction));
            return VMResult::RUNTIME_ERROR;

        default:
            runtimeError("알 수 없는 명령어");
            return VMResult::RUNTIME_ERROR;
    }

    return VMResult::OK;
}

VMResult VM::binaryOp(OpCode op) {
    evaluator::Value b = pop();
    evaluator::Value a = pop();

    // 타입 체크
    bool aIsNum = a.isInteger() || a.isFloat();
    bool bIsNum = b.isInteger() || b.isFloat();

    if (!aIsNum || !bIsNum) {
        // 문자열 연결 (ADD만)
        if (op == OpCode::ADD && (a.isString() || b.isString())) {
            std::string result = a.toString() + b.toString();
            push(evaluator::Value::createString(result));
            return VMResult::OK;
        }

        runtimeError("피연산자는 숫자여야 합니다");
        return VMResult::RUNTIME_ERROR;
    }

    // 실수 변환
    double aVal = a.isFloat() ? a.asFloat() : static_cast<double>(a.asInteger());
    double bVal = b.isFloat() ? b.asFloat() : static_cast<double>(b.asInteger());

    double result = 0.0;
    switch (op) {
        case OpCode::ADD:   result = aVal + bVal; break;
        case OpCode::SUB:   result = aVal - bVal; break;
        case OpCode::MUL:   result = aVal * bVal; break;
        case OpCode::DIV:
            if (bVal == 0.0) {
                runtimeError("0으로 나눌 수 없습니다");
                return VMResult::RUNTIME_ERROR;
            }
            result = aVal / bVal;
            break;
        case OpCode::MOD:
            if (bVal == 0.0) {
                runtimeError("0으로 나눌 수 없습니다");
                return VMResult::RUNTIME_ERROR;
            }
            result = static_cast<int64_t>(aVal) % static_cast<int64_t>(bVal);
            break;
        default:
            runtimeError("알 수 없는 이항 연산자");
            return VMResult::RUNTIME_ERROR;
    }

    // 결과 타입 결정
    if (a.isFloat() || b.isFloat() || op == OpCode::DIV) {
        push(evaluator::Value::createFloat(result));
    } else {
        push(evaluator::Value::createInteger(static_cast<int64_t>(result)));
    }

    return VMResult::OK;
}

void VM::tryJITCompileLoop(size_t loopStart) {
    // 루프 끝 찾기 (현재 IP가 루프 끝)
    size_t loopEnd = ip_;

    // 루프 범위 검증
    if (loopStart >= loopEnd || loopEnd > chunk_->size()) {
        return;
    }

    // JIT 컴파일
    jit::NativeFunction* nativeFunc = jitCompiler_->compileLoop(chunk_, loopStart, loopEnd);

    if (nativeFunc) {
        jitCache_[loopStart] = nativeFunc;
        hotPathDetector_->markJITCompiled(loopStart, jit::HotPathType::LOOP, jit::JITTier::TIER_1);
    }
}

VMResult VM::executeJITCode(jit::NativeFunction* nativeFunc) {
    if (!nativeFunc || !nativeFunc->code) {
        return VMResult::RUNTIME_ERROR;
    }

    try {
        // 스택을 int64_t 배열로 변환
        std::vector<int64_t> jitStack(stack_.size());
        for (size_t i = 0; i < stack_.size(); i++) {
            if (stack_[i].isInteger()) {
                jitStack[i] = stack_[i].asInteger();
            } else if (stack_[i].isFloat()) {
                jitStack[i] = static_cast<int64_t>(stack_[i].asFloat());
            } else if (stack_[i].isBoolean()) {
                jitStack[i] = stack_[i].asBoolean() ? 1 : 0;
            } else {
                jitStack[i] = 0;
            }
        }

        // JIT 코드 실행
        auto funcPtr = nativeFunc->getFunction();
        int64_t result = funcPtr(jitStack.data(), jitStack.size());

        // 스택을 비우고 결과 + cleanup을 위한 dummy 값들 push
        // 인터프리터 HALT 시: [i, sum] (스택 크기=2)
        // JIT는 [result, dummy, dummy]로 push → POP 2번 → [result]
        // 주의: 스택은 LIFO이므로 순서가 중요!
        stack_.clear();
        stack_.push_back(evaluator::Value::createInteger(result));  // sum (bottom)
        stack_.push_back(evaluator::Value::createInteger(0));       // dummy (i)
        stack_.push_back(evaluator::Value::createInteger(0));       // dummy (condition) (top)

        nativeFunc->executionCount++;
        return VMResult::OK;
    } catch (const std::exception& e) {
        std::cerr << "[VM] JIT 코드 실행 중 예외: " << e.what() << "\n";
        return VMResult::RUNTIME_ERROR;
    }
}

void VM::printJITStatistics() const {
    std::cout << "\n=== VM JIT Statistics ===\n";
    std::cout << "JIT Enabled: " << (jitEnabled_ ? "Yes" : "No") << "\n";
    std::cout << "JIT Cache Size: " << jitCache_.size() << "\n";

    if (jitCompiler_) {
        jitCompiler_->printStatistics();
    }

    if (hotPathDetector_) {
        hotPathDetector_->printStatistics();
    }

    std::cout << "=========================\n\n";
}

} // namespace bytecode
} // namespace kingsejong
