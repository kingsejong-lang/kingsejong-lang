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
#include <algorithm>
#include "common/Logger.h"
#include "error/ErrorMessages.h"

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
                runtimeError(Logger::formatString(std::string(error::vm::MAX_INSTRUCTION_LIMIT), std::to_string(maxInstructions_)));
                return VMResult::RUNTIME_ERROR;
            }

            // 안전 장치 2: 실행 시간 제한
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_);
            if (elapsed.count() > maxExecutionTime_.count()) {
                runtimeError(Logger::formatString(std::string(error::vm::MAX_EXECUTION_TIME), std::to_string(maxExecutionTime_.count())));
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
        throw std::runtime_error(Logger::formatString(std::string(error::vm::STACK_SIZE_EXCEEDED), std::to_string(maxStackSize_)));
    }
    stack_.push_back(value);
}

evaluator::Value VM::pop() {
    if (stack_.empty()) {
        throw std::runtime_error(std::string(error::vm::STACK_UNDERFLOW));
    }
    evaluator::Value value = stack_.back();
    stack_.pop_back();
    return value;
}

evaluator::Value VM::peek(int distance) const {
    if (distance >= static_cast<int>(stack_.size())) {
        throw std::runtime_error(std::string(error::vm::STACK_PEEK_OUT_OF_RANGE));
    }
    return stack_[stack_.size() - 1 - distance];
}

void VM::runtimeError(const std::string& message) {
    Logger::error("[런타임 오류] " + message);
    Logger::error("[라인 " + std::to_string(chunk_->getLine(ip_ - 1)) + "]");

    // 스택 추적
    Logger::error("스택 추적:");
    printStack();
}

void VM::printStack() const {
    std::string stackStr = "스택: ";
    if (stack_.empty()) {
        stackStr += "(비어있음)";
    } else {
        for (const auto& value : stack_) {
            stackStr += "[ " + value.toString() + " ] ";
        }
    }
    Logger::info(stackStr);
}

VMResult VM::executeInstruction() {
    OpCode instruction = static_cast<OpCode>(readByte());

    // Phase 9: OpCode 카테고리별 분기
    // 상수 로드 (LOAD_CONST, LOAD_TRUE, LOAD_FALSE, LOAD_NULL)
    if (instruction >= OpCode::LOAD_CONST && instruction <= OpCode::LOAD_NULL) {
        return executeConstantOps(instruction);
    }

    // 변수 조작 (LOAD_VAR, STORE_VAR, LOAD_GLOBAL, STORE_GLOBAL)
    if (instruction >= OpCode::LOAD_VAR && instruction <= OpCode::STORE_GLOBAL) {
        return executeVariableOps(instruction);
    }

    // 산술 연산 (ADD, SUB, MUL, DIV, MOD, NEG)
    if (instruction >= OpCode::ADD && instruction <= OpCode::NEG) {
        return executeArithmeticOps(instruction);
    }

    // 비교 연산 (EQ, NE, LT, GT, LE, GE)
    if (instruction >= OpCode::EQ && instruction <= OpCode::GE) {
        return executeComparisonOps(instruction);
    }

    // 논리 연산 (AND, OR, NOT)
    if (instruction >= OpCode::AND && instruction <= OpCode::NOT) {
        return executeLogicalOps(instruction);
    }

    // 제어 흐름 (JUMP, JUMP_IF_FALSE, JUMP_IF_TRUE, LOOP, BUILD_FUNCTION, CALL, RETURN, HALT)
    if ((instruction >= OpCode::JUMP && instruction <= OpCode::LOOP) ||
        (instruction >= OpCode::CALL && instruction <= OpCode::BUILD_FUNCTION) ||
        instruction == OpCode::HALT) {
        return executeControlFlowOps(instruction);
    }

    // 배열 (BUILD_ARRAY, INDEX_GET, INDEX_SET, ARRAY_APPEND, JOSA_CALL)
    if ((instruction >= OpCode::BUILD_ARRAY && instruction <= OpCode::ARRAY_APPEND) ||
        instruction == OpCode::JOSA_CALL) {
        return executeArrayOps(instruction);
    }

    // Promise/Async (BUILD_ASYNC_FUNC, ASYNC_CALL, AWAIT, PROMISE_RESOLVE, PROMISE_REJECT, BUILD_PROMISE, PROMISE_THEN, PROMISE_CATCH)
    if (instruction >= OpCode::BUILD_ASYNC_FUNC && instruction <= OpCode::PROMISE_CATCH) {
        return executePromiseOps(instruction);
    }

    // 기타 (POP, DUP, SWAP, PRINT, BUILD_RANGE, IMPORT, CLASS_DEF, NEW_INSTANCE, LOAD_FIELD, STORE_FIELD, CALL_METHOD, LOAD_THIS)
    return executeMiscOps(instruction);
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
        runtimeError(std::string(error::vm::OPERAND_MUST_BE_NUMBER));
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
                runtimeError(std::string(error::vm::DIVIDE_BY_ZERO));
                return VMResult::RUNTIME_ERROR;
            }
            result = aVal / bVal;
            break;
        case OpCode::MOD:
            if (bVal == 0.0) {
                runtimeError(std::string(error::vm::DIVIDE_BY_ZERO));
                return VMResult::RUNTIME_ERROR;
            }
            result = static_cast<int64_t>(aVal) % static_cast<int64_t>(bVal);
            break;
        default:
            runtimeError(std::string(error::vm::UNKNOWN_BINARY_OP));
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
        // JIT 실행 중 스택이 증가할 수 있으므로 최대 스택 크기만큼 할당
        std::vector<int64_t> jitStack(maxStackSize_);
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
        // 첫 번째 인자: 스택 버퍼 포인터 (충분한 크기 확보됨)
        // 두 번째 인자: 현재 스택의 유효 데이터 개수 (x9 초기값)
        auto funcPtr = nativeFunc->getFunction();
        int64_t result = funcPtr(jitStack.data(), stack_.size());

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
        Logger::error(Logger::formatString(std::string(error::vm::JIT_EXECUTION_EXCEPTION), e.what()));
        return VMResult::RUNTIME_ERROR;
    }
}

void VM::printJITStatistics() const {
    Logger::info("\n=== VM JIT Statistics ===");
    Logger::info("JIT Enabled: " + std::string(jitEnabled_ ? "Yes" : "No"));
    Logger::info("JIT Cache Size: " + std::to_string(jitCache_.size()));

    if (jitCompiler_) {
        jitCompiler_->printStatistics();
    }

    if (hotPathDetector_) {
        hotPathDetector_->printStatistics();
    }

    Logger::info("=========================\n");
}

// ============================================================================
// Phase 9: executeInstruction() 분해 - OpCode 카테고리별 메서드
// ============================================================================

VMResult VM::executeConstantOps(OpCode instruction) {
    switch (instruction) {
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

        default:
            runtimeError(Logger::formatString(std::string(error::vm::UNIMPLEMENTED_OPCODE), opCodeToString(instruction)));
            return VMResult::RUNTIME_ERROR;
    }

    return VMResult::OK;
}

VMResult VM::executeVariableOps(OpCode instruction) {
    switch (instruction) {
        case OpCode::LOAD_VAR: {
            uint8_t slot = readByte();
            if (slot >= stack_.size()) {
                runtimeError(std::string(error::vm::LOCAL_VAR_INDEX_OUT_OF_BOUNDS));
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
                runtimeError(Logger::formatString(std::string(error::vm::UNDEFINED_VARIABLE), name));
                return VMResult::RUNTIME_ERROR;
            }
            break;
        }

        case OpCode::STORE_GLOBAL: {
            std::string name = readName();
            globals_->set(name, peek(0));
            break;
        }

        default:
            runtimeError(Logger::formatString(std::string(error::vm::UNIMPLEMENTED_OPCODE), opCodeToString(instruction)));
            return VMResult::RUNTIME_ERROR;
    }

    return VMResult::OK;
}

VMResult VM::executeArithmeticOps(OpCode instruction) {
    switch (instruction) {
        case OpCode::ADD: {
            if (peek(0).isInteger() && peek(1).isInteger()) {
                // 정수 덧셈
                evaluator::Value b = pop();
                evaluator::Value a = pop();
                push(evaluator::Value::createInteger(a.asInteger() + b.asInteger()));
            } else if ((peek(0).isInteger() || peek(0).isFloat()) && (peek(1).isInteger() || peek(1).isFloat())) {
                // 실수 덧셈
                evaluator::Value b = pop();
                evaluator::Value a = pop();
                push(evaluator::Value::createFloat(a.asFloat() + b.asFloat()));
            } else if (peek(0).isString() && peek(1).isString()) {
                // 문자열 연결
                evaluator::Value b = pop();
                evaluator::Value a = pop();
                push(evaluator::Value::createString(a.asString() + b.asString()));
            } else {
                runtimeError(std::string(error::vm::OPERAND_MUST_BE_NUMBER_OR_STRING));
                return VMResult::RUNTIME_ERROR;
            }
            break;
        }

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
                runtimeError(std::string(error::vm::OPERAND_MUST_BE_NUMBER));
                return VMResult::RUNTIME_ERROR;
            }
            break;
        }

        default:
            runtimeError(Logger::formatString(std::string(error::vm::UNIMPLEMENTED_OPCODE), opCodeToString(instruction)));
            return VMResult::RUNTIME_ERROR;
    }

    return VMResult::OK;
}

VMResult VM::executeComparisonOps(OpCode instruction) {
    evaluator::Value b = pop();
    evaluator::Value a = pop();

    switch (instruction) {
        case OpCode::EQ:
            push(evaluator::Value::createBoolean(a.equals(b)));
            break;

        case OpCode::NE:
            push(evaluator::Value::createBoolean(!a.equals(b)));
            break;

        case OpCode::LT:
            push(evaluator::Value::createBoolean(a.lessThan(b)));
            break;

        case OpCode::GT:
            push(evaluator::Value::createBoolean(a.greaterThan(b)));
            break;

        case OpCode::LE:
            push(evaluator::Value::createBoolean(a.lessThan(b) || a.equals(b)));
            break;

        case OpCode::GE:
            push(evaluator::Value::createBoolean(a.greaterThan(b) || a.equals(b)));
            break;

        default:
            runtimeError(Logger::formatString(std::string(error::vm::UNIMPLEMENTED_OPCODE), opCodeToString(instruction)));
            return VMResult::RUNTIME_ERROR;
    }

    return VMResult::OK;
}

VMResult VM::executeLogicalOps(OpCode instruction) {
    switch (instruction) {
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
            evaluator::Value a = pop();
            push(evaluator::Value::createBoolean(!a.isTruthy()));
            break;
        }

        default:
            runtimeError(Logger::formatString(std::string(error::vm::UNIMPLEMENTED_OPCODE), opCodeToString(instruction)));
            return VMResult::RUNTIME_ERROR;
    }

    return VMResult::OK;
}

VMResult VM::executeControlFlowOps(OpCode instruction) {
    switch (instruction) {
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

                // JIT 캐시 확인
                auto it = jitCache_.find(loopStart);

                if (it != jitCache_.end()) {
                    // JIT 코드 실행
                    VMResult jitResult = executeJITCode(it->second);

                    if (jitResult == VMResult::OK) {
                        ip_ = it->second->exitOffset - 1;
                        break;
                    } else {
                        Logger::warn("[VM] JIT 실행 실패, 인터프리터로 폴백");
                    }
                } else if (hotPathDetector_->isHot(loopStart, jit::HotPathType::LOOP)) {
                    // 핫 루프 컴파일
                    tryJITCompileLoop(loopStart);
                }
            }

            // 인터프리터 폴백
            ip_ -= offset;
            break;
        }

        case OpCode::BUILD_FUNCTION: {
            uint8_t paramCount = readByte();
            uint8_t addrHigh = readByte();
            uint8_t addrLow = readByte();
            size_t funcAddr = (static_cast<size_t>(addrHigh) << 8) | addrLow;

            int64_t encoded = (static_cast<int64_t>(funcAddr) << 8) | paramCount;
            push(evaluator::Value::createInteger(encoded));
            break;
        }

        case OpCode::CALL: {
            uint8_t argCount = readByte();

            evaluator::Value funcVal = peek(argCount);
            if (!funcVal.isInteger()) {
                runtimeError(std::string(error::vm::CALL_NON_FUNCTION));
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
                // 최상위 레벨 return
                push(result);
                return VMResult::OK;
            }

            // CallFrame 복원
            CallFrame frame = frames_.back();
            frames_.pop_back();

            // 스택 정리
            while (stack_.size() > frame.stackBase) {
                pop();
            }

            // 반환값 푸시
            push(result);

            // IP 복원
            ip_ = frame.returnAddress;
            break;
        }

        case OpCode::HALT:
            return VMResult::HALT;

        default:
            runtimeError(Logger::formatString(std::string(error::vm::UNIMPLEMENTED_OPCODE), opCodeToString(instruction)));
            return VMResult::RUNTIME_ERROR;
    }

    return VMResult::OK;
}

// ============================================================================
// Phase 9 Step 7: executeArrayOps
// ============================================================================

VMResult VM::executeArrayOps(OpCode instruction) {
    switch (instruction) {
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
                runtimeError(std::string(error::vm::NOT_AN_ARRAY));
                return VMResult::RUNTIME_ERROR;
            }

            if (!index.isInteger()) {
                runtimeError(std::string(error::vm::INDEX_MUST_BE_INTEGER));
                return VMResult::RUNTIME_ERROR;
            }

            int idx = static_cast<int>(index.asInteger());
            auto& arr = array.asArray();

            if (idx < 0 || idx >= static_cast<int>(arr.size())) {
                runtimeError(std::string(error::vm::INDEX_OUT_OF_BOUNDS));
                return VMResult::RUNTIME_ERROR;
            }

            push(arr[idx]);
            break;
        }

        case OpCode::INDEX_SET:
        case OpCode::ARRAY_APPEND:
            runtimeError(Logger::formatString(std::string(error::vm::UNIMPLEMENTED_OPCODE), opCodeToString(instruction)));
            return VMResult::RUNTIME_ERROR;

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
                    runtimeError(Logger::formatString(std::string(error::vm::UNKNOWN_ARRAY_METHOD), methodName));
                    return VMResult::RUNTIME_ERROR;
                }
            } else {
                runtimeError(std::string(error::vm::UNSUPPORTED_JOSA_TYPE));
                return VMResult::RUNTIME_ERROR;
            }
            break;
        }

        default:
            runtimeError(Logger::formatString(std::string(error::vm::UNIMPLEMENTED_OPCODE), opCodeToString(instruction)));
            return VMResult::RUNTIME_ERROR;
    }

    return VMResult::OK;
}

// ============================================================================
// Phase 9 Step 8: executePromiseOps
// ============================================================================

VMResult VM::executePromiseOps(OpCode instruction) {
    switch (instruction) {
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
                runtimeError(std::string(error::vm::CALL_NON_ASYNC));
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
                    runtimeError(Logger::formatString(std::string(error::vm::PROMISE_REJECTED), promise->value().toString()));
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
            runtimeError(std::string(error::vm::PROMISE_THEN_CATCH_UNSUPPORTED));
            return VMResult::RUNTIME_ERROR;

        default:
            runtimeError(Logger::formatString(std::string(error::vm::UNIMPLEMENTED_OPCODE), opCodeToString(instruction)));
            return VMResult::RUNTIME_ERROR;
    }

    return VMResult::OK;
}

// ============================================================================
// Phase 9 Step 9: executeMiscOps
// ============================================================================

VMResult VM::executeMiscOps(OpCode instruction) {
    switch (instruction) {
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
            Logger::info(value.toString());
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
                runtimeError(Logger::formatString(std::string(error::vm::UNDEFINED_CLASS), className));
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
                if (argCount != params.size()) {
                    runtimeError(Logger::formatString(std::string(error::vm::CONSTRUCTOR_ARG_MISMATCH), std::to_string(params.size()), std::to_string(argCount)));
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
                    runtimeError(Logger::formatString(std::string(error::vm::CONSTRUCTOR_EXECUTION_ERROR), result.asString()));
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
                runtimeError(std::string(error::vm::FIELD_ACCESS_NOT_INSTANCE));
                return VMResult::RUNTIME_ERROR;
            }

            auto instance = objVal.asClassInstance();
            try {
                evaluator::Value fieldValue = instance->getField(fieldName);
                push(fieldValue);
            } catch (const std::exception& e) {
                runtimeError(Logger::formatString(std::string(error::vm::FIELD_ACCESS_ERROR), e.what()));
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
                runtimeError(std::string(error::vm::FIELD_SET_NOT_INSTANCE));
                return VMResult::RUNTIME_ERROR;
            }

            auto instance = objVal.asClassInstance();
            try {
                instance->setField(fieldName, value);
                push(value);  // 대입 결과를 스택에 푸시
            } catch (const std::exception& e) {
                runtimeError(Logger::formatString(std::string(error::vm::FIELD_SET_ERROR), e.what()));
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
                runtimeError(std::string(error::vm::METHOD_CALL_NOT_INSTANCE));
                return VMResult::RUNTIME_ERROR;
            }

            auto instance = objVal.asClassInstance();
            auto classDef = instance->classDef();

            // 메서드 찾기
            auto method = classDef->getMethod(methodName);
            if (!method) {
                runtimeError(Logger::formatString(std::string(error::vm::UNDEFINED_METHOD), methodName));
                return VMResult::RUNTIME_ERROR;
            }

            // 메서드 환경 생성
            auto methodEnv = std::make_shared<evaluator::Environment>(globals_);

            // 매개변수 바인딩
            const auto& params = method->parameters();
            if (params.size() != args.size()) {
                runtimeError(Logger::formatString(std::string(error::vm::METHOD_ARG_MISMATCH), std::to_string(params.size()), std::to_string(argCount)));
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
                runtimeError(Logger::formatString(std::string(error::vm::METHOD_EXECUTION_ERROR), result.asString()));
                return VMResult::RUNTIME_ERROR;
            }

            // 결과 푸시
            push(result);
            break;
        }

        case OpCode::LOAD_THIS: {
            // this 스택에서 현재 인스턴스 가져오기
            if (thisStack_.empty()) {
                runtimeError(std::string(error::vm::THIS_OUTSIDE_CLASS));
                return VMResult::RUNTIME_ERROR;
            }

            auto instance = thisStack_.back();
            push(evaluator::Value::createClassInstance(instance));
            break;
        }

        // ========================================
        // 미구현 OpCode
        // ========================================
        case OpCode::INDEX_SET:
        case OpCode::ARRAY_APPEND:
        case OpCode::BUILD_RANGE:
        case OpCode::IMPORT:
            runtimeError(Logger::formatString(std::string(error::vm::UNIMPLEMENTED_OPCODE), opCodeToString(instruction)));
            return VMResult::RUNTIME_ERROR;

        default:
            runtimeError(Logger::formatString(std::string(error::vm::UNIMPLEMENTED_OPCODE), opCodeToString(instruction)));
            return VMResult::RUNTIME_ERROR;
    }

    return VMResult::OK;
}

} // namespace bytecode
} // namespace kingsejong
