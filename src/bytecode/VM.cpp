/**
 * @file VM.cpp
 * @brief VM 구현
 * @author KingSejong Team
 * @date 2025-11-10
 */

#include "VM.h"
#include "jit/JITCompilerT1.h"
#include "jit/HotPathDetector.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace kingsejong {
namespace bytecode {

VM::VM()
    : chunk_(nullptr), ip_(0), traceExecution_(false),
      instructionCount_(0), maxInstructions_(10000000),  // 1천만 명령어
      maxExecutionTime_(5000),  // 5초
      maxStackSize_(10000),  // 1만 개
      jitEnabled_(true) {  // JIT 기본 활성화
    globals_ = std::make_shared<evaluator::Environment>();

    // JIT 컴파일러 초기화
    jitCompiler_ = std::make_unique<jit::JITCompilerT1>();
    std::cerr << "[VM] JIT 컴파일러 초기화 완료\n";

    // Hot Path Detector 초기화
    hotPathDetector_ = std::make_unique<jit::HotPathDetector>();
    hotPathDetector_->setLoopThreshold(100);  // 100번 반복 후 JIT 컴파일
    std::cerr << "[VM] Hot Path Detector 초기화 완료 (임계값: 100)\n";
    std::cerr << "[VM] JIT 활성화: " << (jitEnabled_ ? "YES" : "NO") << "\n";
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

            // JIT 컴파일 시도 (백엣지에서)
            if (jitEnabled_) {
                // 루프 백엣지 추적
                hotPathDetector_->trackLoopBackedge(loopStart);

                // 디버그: 루프 추적 확인
                auto profile = hotPathDetector_->getProfile(loopStart, jit::HotPathType::LOOP);
                if (profile && profile->executionCount % 50 == 0) {
                    std::cerr << "[VM] 루프 백엣지 추적: 시작=" << loopStart
                              << ", 실행횟수=" << profile->executionCount << "\n";
                }

                // 핫 루프인지 확인
                if (hotPathDetector_->isHot(loopStart, jit::HotPathType::LOOP)) {
                    // JIT 캐시 확인
                    auto it = jitCache_.find(loopStart);
                    if (it == jitCache_.end()) {
                        std::cerr << "[VM] 핫 루프 감지! JIT 컴파일 시도\n";
                        // JIT 컴파일 시도 (비동기적으로, 다음 반복을 위해)
                        tryJITCompileLoop(loopStart);
                        // 이번 반복은 인터프리터로 계속 진행
                    }
                }
            }

            // 인터프리터로 루프 백점프 실행
            // (JIT 코드는 향후 함수 전체 컴파일 시 활용)
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
        case OpCode::POP:
            pop();
            break;

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
            size_t funcAddr = static_cast<size_t>((encoded >> 8) & 0xFFFF);
            // uint8_t paramCount = static_cast<uint8_t>(encoded & 0xFF);

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
    std::cerr << "[VM] JIT 컴파일 시도: 루프 시작 = " << loopStart << "\n";

    // 루프 끝 찾기 (현재 IP가 루프 끝)
    size_t loopEnd = ip_;

    // 루프 범위 검증
    if (loopStart >= loopEnd || loopEnd > chunk_->size()) {
        std::cerr << "[VM] JIT 컴파일 실패: 잘못된 루프 범위\n";
        return;
    }

    // JIT 컴파일
    jit::NativeFunction* nativeFunc = jitCompiler_->compileLoop(chunk_, loopStart, loopEnd);

    if (nativeFunc) {
        std::cerr << "[VM] JIT 컴파일 성공: 루프 [" << loopStart << ", " << loopEnd << ")\n";
        jitCache_[loopStart] = nativeFunc;
        hotPathDetector_->markJITCompiled(loopStart, jit::HotPathType::LOOP, jit::JITTier::TIER_1);
    } else {
        std::cerr << "[VM] JIT 컴파일 실패\n";
    }
}

VMResult VM::executeJITCode(jit::NativeFunction* nativeFunc) {
    if (!nativeFunc || !nativeFunc->code) {
        return VMResult::RUNTIME_ERROR;
    }

    std::cerr << "[VM] JIT 코드 실행 시작\n";

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

        // 결과를 스택에 다시 변환
        stack_.clear();
        for (size_t i = 0; i < jitStack.size(); i++) {
            stack_.push_back(evaluator::Value::createInteger(jitStack[i]));
        }

        // 결과 푸시
        stack_.push_back(evaluator::Value::createInteger(result));

        std::cerr << "[VM] JIT 코드 실행 완료, 결과 = " << result << "\n";

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
