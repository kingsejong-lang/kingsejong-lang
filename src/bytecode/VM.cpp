/**
 * @file VM.cpp
 * @brief VM 구현
 * @author KingSejong Team
 * @date 2025-11-10
 */

#include "VM.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace kingsejong {
namespace bytecode {

VM::VM()
    : chunk_(nullptr), ip_(0), traceExecution_(false) {
    globals_ = std::make_shared<evaluator::Environment>();
}

VMResult VM::run(Chunk* chunk) {
    chunk_ = chunk;
    ip_ = 0;
    stack_.clear();

    try {
        while (true) {
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

} // namespace bytecode
} // namespace kingsejong
