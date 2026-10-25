/**
 * @file VM.cpp
 * @brief 바이트코드 가상 머신 구현
 */

#include "VM.h"
#include "Compiler.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "runtime/EventLoop.h"
#include "module/ModuleLoader.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "error/Error.h"
#include "common/Logger.h"
#include "error/ErrorMessages.h"
#include "lexer/JosaRecognizer.h"

namespace kingsejong {
namespace bytecode {

using evaluator::Value;
using evaluator::Environment;
using evaluator::Evaluator;

namespace {

const char* binaryOpName(OpCode op) {
    switch (op) {
        case OpCode::ADD: return "+";
        case OpCode::SUB: return "-";
        case OpCode::MUL: return "*";
        case OpCode::DIV: return "/";
        case OpCode::MOD: return "%";
        case OpCode::EQ:  return "==";
        case OpCode::NE:  return "!=";
        case OpCode::LT:  return "<";
        case OpCode::GT:  return ">";
        case OpCode::LE:  return "<=";
        case OpCode::GE:  return ">=";
        case OpCode::AND: return "&&";
        case OpCode::OR:  return "||";
        default:          return "?";
    }
}

} // namespace

VM::VM()
    : chunk_(nullptr), ip_(0), traceExecution_(false),
      instructionCount_(0), maxInstructions_(VM_DEFAULT_MAX_INSTRUCTIONS),
      maxExecutionTime_(VM_DEFAULT_MAX_EXECUTION_TIME_MS),
      maxStackSize_(VM_DEFAULT_MAX_STACK_SIZE), maxFrames_(VM_DEFAULT_MAX_FRAMES) {
    globals_ = std::make_shared<Environment>();
    env_ = globals_;
}

VM::~VM() {
    // 전역 함수·클래스가 만든 함수-환경 순환 참조를 끊는다 (평가기 소멸자와 같다)
    frames_.clear();
    handlers_.clear();
    pending_.clear();
    stack_.clear();
    env_.reset();
    releaseScope(globals_);
}

void VM::releaseScope(std::shared_ptr<Environment>& scope) {
    if (scope) {
        scope->releaseIfUnreachable(1);
        scope.reset();
    }
}

// ============================================================================
// 실행 루프
// ============================================================================

VMResult VM::run(Chunk* chunk) {
    resetFor(chunk, 0, globals_);
    return runLoop(false);
}

void VM::resetFor(Chunk* chunk, size_t entry, std::shared_ptr<Environment> env) {
    chunk_ = chunk;
    ip_ = entry;
    stack_.clear();
    frames_.clear();
    handlers_.clear();
    pending_.clear();
    env_ = std::move(env);
    lastError_.reset();
    instructionCount_ = 0;
    startTime_ = std::chrono::steady_clock::now();
}

void VM::runModule(ast::Program& program, const std::shared_ptr<Environment>& moduleEnv) {
    // 모듈 코드를 이 청크 끝에 덧붙인다. 모듈 함수의 본문 진입점이 같은 청크에 등록되므로
    // 가져온 쪽이 그 함수를 부를 때도 바이트코드로 돈다. 모듈 AST 는 로더 캐시가 쥐고 있다.
    Compiler compiler;
    size_t entry = 0;
    if (!compiler.compileModule(&program, chunk_, entry)) {
        throw error::RuntimeError("모듈 컴파일 오류: " +
                                  (compiler.errors().empty() ? std::string("알 수 없는 오류") : compiler.errors()[0]));
    }

    // 모듈은 자기 환경에서 별도 VM 으로 돈다 (평가기가 모듈마다 새 Evaluator 를 만드는 것과 같다).
    // 잡히지 않은 에러는 예외로 올라와 가져오기 문 자리에서 던져진 것처럼 처리된다.
    VM moduleVm;
    moduleVm.moduleLoader_ = moduleLoader_;
    moduleVm.traceExecution_ = traceExecution_;
    moduleVm.maxInstructions_ = maxInstructions_;
    moduleVm.maxExecutionTime_ = maxExecutionTime_;
    moduleVm.maxStackSize_ = maxStackSize_;
    moduleVm.maxFrames_ = maxFrames_;
    moduleVm.globals_ = moduleEnv;
    moduleVm.resetFor(chunk_, entry, moduleEnv);
    moduleVm.runLoop(true);
}

VMResult VM::runLoop(bool propagate) {
    while (true) {
        try {
            checkLimits();

            if (traceExecution_) {
                printStack();
                chunk_->disassembleInstruction(ip_);
            }

            if (executeInstruction() == Step::HALT) {
                // 프로그램이 끝나면 이벤트 루프를 돌려 미뤄 둔 작업을 처리한다 (평가기와 같다)
                evaluator::getGlobalEventLoop().run();
                return VMResult::OK;
            }
        } catch (const evaluator::ReturnValue& ret) {
            // 평가기로 해석한 본문에서 새어 나온 반환 (정상 경로에서는 일어나지 않는다)
            push(ret.getValue());
        } catch (error::KingSejongError& e) {
            // 실행 에러와 던지다(ThrowSignal). 시도 블록 안이면 오류 블록으로, 아니면 호출 스택을 붙여
            // 가져온 쪽(하위 VM)으로 올리거나 lastError 로 남기고 끝낸다. 프레임은 아직 그대로다
            if (!canHandle()) {
                attachStackTrace(e);
                if (propagate) throw;
                return fail(e);
            }
            auto* signal = dynamic_cast<evaluator::ThrowSignal*>(&e);
            unwindToHandler(signal ? signal->value() : Value::createError(e.what(), "RuntimeError"));
        } catch (const std::exception& e) {
            // 내장 함수·값 연산이 던진 일반 예외는 실행 에러로 다룬다
            if (!canHandle()) {
                error::KingSejongError converted = error::RuntimeError(e.what());
                attachStackTrace(converted);
                if (propagate) throw converted;
                return fail(converted);
            }
            unwindToHandler(Value::createError(e.what(), "RuntimeError"));
        }
    }
}

void VM::checkLimits() {
    if (++instructionCount_ > maxInstructions_) {
        throw error::RuntimeError(
            Logger::formatString(std::string(error::vm::MAX_INSTRUCTION_LIMIT), std::to_string(maxInstructions_)));
    }
    // 시간 검사는 1024 명령마다 (시계 읽기가 비싸다)
    if ((instructionCount_ & 1023) == 0) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime_);
        if (elapsed.count() > maxExecutionTime_.count()) {
            throw error::RuntimeError(
                Logger::formatString(std::string(error::vm::MAX_EXECUTION_TIME), std::to_string(maxExecutionTime_.count())));
        }
    }
}

Value VM::top() const {
    if (stack_.empty()) {
        return Value::createNull();
    }
    return stack_.back();
}

// ============================================================================
// 디코딩·스택
// ============================================================================

uint8_t VM::readByte() {
    return chunk_->read(ip_++);
}

uint16_t VM::readShort() {
    uint16_t value = chunk_->read16(ip_);
    ip_ += 2;
    return value;
}

const std::string& VM::readName() {
    return chunk_->getName(readShort());
}

void VM::push(const Value& value) {
    if (stack_.size() >= maxStackSize_) {
        throw std::runtime_error(Logger::formatString(std::string(error::vm::STACK_SIZE_EXCEEDED), std::to_string(maxStackSize_)));
    }
    stack_.push_back(value);
}

Value VM::pop() {
    if (stack_.empty()) {
        throw std::runtime_error(std::string(error::vm::STACK_UNDERFLOW));
    }
    Value value = std::move(stack_.back());
    stack_.pop_back();
    return value;
}

const Value& VM::peek(size_t distance) const {
    if (distance >= stack_.size()) {
        throw std::runtime_error(std::string(error::vm::STACK_PEEK_OUT_OF_RANGE));
    }
    return stack_[stack_.size() - 1 - distance];
}

std::vector<Value> VM::popArguments(size_t count) {
    if (count > stack_.size()) {
        throw std::runtime_error(std::string(error::vm::STACK_UNDERFLOW));
    }
    std::vector<Value> args(stack_.end() - static_cast<std::ptrdiff_t>(count), stack_.end());
    stack_.resize(stack_.size() - count);
    return args;
}

// ============================================================================
// 명령 실행
// ============================================================================

VM::Step VM::executeInstruction() {
    OpCode instruction = static_cast<OpCode>(readByte());

    switch (instruction) {
        // ---- 상수 ----
        case OpCode::LOAD_CONST:
            push(chunk_->getConstant(readShort()));
            break;
        case OpCode::LOAD_TRUE:
            push(Value::createBoolean(true));
            break;
        case OpCode::LOAD_FALSE:
            push(Value::createBoolean(false));
            break;
        case OpCode::LOAD_NULL:
            push(Value::createNull());
            break;

        // ---- 이름 ----
        case OpCode::DECLARE: {
            const std::string& name = readName();
            env_->set(name, pop());
            break;
        }
        case OpCode::LOAD_NAME: {
            const std::string& name = readName();
            push(Evaluator::lookupName(*env_, name));
            break;
        }
        case OpCode::STORE_NAME: {
            const std::string& name = readName();
            env_->setWithLookup(name, peek());
            break;
        }
        case OpCode::PUSH_SCOPE:
            env_ = std::make_shared<Environment>(env_);
            break;
        case OpCode::POP_SCOPE: {
            auto inner = env_;
            env_ = inner->outer();
            releaseScope(inner);
            break;
        }

        // ---- 연산 ----
        case OpCode::ADD: case OpCode::SUB: case OpCode::MUL: case OpCode::DIV: case OpCode::MOD:
        case OpCode::EQ: case OpCode::NE: case OpCode::LT: case OpCode::GT: case OpCode::LE: case OpCode::GE:
        case OpCode::AND: case OpCode::OR: {
            Value right = pop();
            Value left = pop();
            push(Evaluator::binaryOperation(left, binaryOpName(instruction), right));
            break;
        }
        case OpCode::NEG: {
            Value operand = pop();
            push(Evaluator::unaryOperation("-", operand));
            break;
        }
        case OpCode::NOT: {
            Value operand = pop();
            push(Evaluator::unaryOperation("!", operand));
            break;
        }

        // ---- 제어 흐름 ----
        case OpCode::JUMP: {
            uint16_t offset = readShort();
            ip_ += offset;
            break;
        }
        case OpCode::JUMP_IF_FALSE: {
            uint16_t offset = readShort();
            if (!pop().isTruthy()) ip_ += offset;
            break;
        }
        case OpCode::JUMP_IF_TRUE: {
            uint16_t offset = readShort();
            if (pop().isTruthy()) ip_ += offset;
            break;
        }
        case OpCode::LOOP: {
            uint16_t offset = readShort();
            ip_ -= offset;
            break;
        }

        // ---- 반복문 보조 ----
        case OpCode::REPEAT_INIT: {
            Value count = pop();
            if (!count.isInteger()) {
                throw error::TypeError(
                    "반복 횟수는 정수여야 합니다: " + count.toString() + "\n" +
                    "해결 방법: 반복 횟수에 정수 값을 사용하세요. 예: 10번 반복");
            }
            if (count.asInteger() < 0) {
                throw error::ValueError(
                    "반복 횟수는 0 이상이어야 합니다: " + std::to_string(count.asInteger()) + "\n" +
                    "해결 방법: 양수 또는 0을 사용하세요.");
            }
            push(count);
            break;
        }
        case OpCode::REPEAT_NEXT: {
            uint16_t exit = readShort();
            int64_t remaining = peek().asInteger();
            if (remaining > 0) {
                stack_.back() = Value::createInteger(remaining - 1);
            } else {
                pop();
                ip_ += exit;
            }
            break;
        }
        case OpCode::RANGE_FOR_INIT: {
            Value end = pop();
            Value start = pop();
            if (!start.isInteger()) {
                throw std::runtime_error("범위 시작 값은 정수여야 합니다");
            }
            if (!end.isInteger()) {
                throw std::runtime_error("범위 끝 값은 정수여야 합니다");
            }
            push(end);
            push(start);
            break;
        }
        case OpCode::RANGE_FOR_NEXT: {
            const std::string& name = readName();
            uint16_t exit = readShort();
            bool inclusive = readByte() != 0;
            int64_t i = peek(0).asInteger();
            int64_t end = peek(1).asInteger();
            bool inRange = inclusive ? (i <= end) : (i < end);
            if (inRange) {
                env_->set(name, Value::createInteger(i));
                stack_.back() = Value::createInteger(i + 1);
            } else {
                pop();
                pop();
                ip_ += exit;
            }
            break;
        }

        // ---- 함수 ----
        case OpCode::MAKE_FUNCTION: {
            const FunctionProto& proto = chunk_->getProto(readShort());
            auto func = std::make_shared<evaluator::Function>(proto.parameters, proto.body, env_, false, proto.isAsync);
            push(Value::createFunction(func));
            break;
        }
        case OpCode::CALL: {
            size_t at = ip_ - 1;
            uint8_t argCount = readByte();
            callValue(argCount, chunk_->getCallSiteName(at));
            break;
        }
        case OpCode::CALL_METHOD: {
            const std::string& name = readName();
            uint8_t argCount = readByte();
            callMethod(name, argCount);
            break;
        }
        case OpCode::RETURN:
            return doReturn(pop());

        // ---- 컨테이너 ----
        case OpCode::BUILD_ARRAY: {
            uint16_t count = readShort();
            push(Value::createArray(popArguments(count)));
            break;
        }
        case OpCode::BUILD_DICT: {
            uint16_t count = readShort();
            std::vector<Value> items = popArguments(static_cast<size_t>(count) * 2);
            std::unordered_map<std::string, Value> dict;
            for (size_t i = 0; i < items.size(); i += 2) {
                const Value& key = items[i];
                if (!key.isString()) {
                    throw error::TypeError("딕셔너리 키는 문자열이어야 합니다. 실제 타입: " +
                                           types::Type::typeKindToString(key.getType()));
                }
                dict[key.asString()] = items[i + 1];
            }
            push(Value::createDictionary(dict));
            break;
        }
        case OpCode::BUILD_STRING: {
            uint16_t count = readShort();
            std::vector<Value> pieces = popArguments(count);
            std::string text;
            for (const auto& piece : pieces) {
                text += Evaluator::interpolationText(piece);
            }
            push(Value::createString(text));
            break;
        }
        case OpCode::INDEX_GET: {
            Value index = pop();
            Value container = pop();
            push(Evaluator::indexValue(container, index));
            break;
        }
        case OpCode::INDEX_SET: {
            Value key = pop();
            Value container = pop();
            Value value = pop();
            Evaluator::assignIndex(container, key, value);
            push(value);
            break;
        }
        case OpCode::SLICE: {
            uint8_t flags = readByte();
            bool hasStep = (flags & 0x04) != 0;
            Value step;
            if (hasStep) step = pop();
            Value end = pop();
            Value start = pop();
            Value array = pop();
            push(Evaluator::sliceValue(array, start, end, (flags & 0x01) != 0, (flags & 0x02) != 0,
                                       hasStep ? &step : nullptr));
            break;
        }

        // ---- 멤버·조사 ----
        case OpCode::LOAD_FIELD: {
            const std::string& name = readName();
            Value object = pop();
            push(Evaluator::memberValue(object, name));
            break;
        }
        case OpCode::STORE_FIELD: {
            const std::string& name = readName();
            Value object = pop();
            Value value = pop();
            Evaluator::assignMember(object, name, value);
            push(value);
            break;
        }
        case OpCode::JOSA: {
            const std::string& name = readName();
            auto josa = static_cast<lexer::JosaRecognizer::JosaType>(readByte());
            uint8_t argCount = readByte();
            bool isCall = readByte() != 0;
            std::vector<Value> args = popArguments(argCount);
            Value object = pop();
            push(Evaluator::applyJosaMethod(object, josa, name, isCall, args, env_));
            break;
        }
        case OpCode::CONVERT: {
            const std::string& name = readName();
            Value object = pop();
            push(Evaluator::convertWith(object, name, env_));
            break;
        }

        // ---- 클래스·모듈 ----
        case OpCode::CLASS_DEF: {
            const auto* node = chunk_->getNode(readShort());
            const auto* stmt = static_cast<const ast::ClassStatement*>(node);
            env_->set(stmt->className(), Evaluator::makeClass(stmt, env_));
            break;
        }
        case OpCode::IMPORT: {
            // 모듈 컴파일이 청크의 이름 표를 늘려 참조를 무효화할 수 있으므로 복사해 둔다
            std::string modulePath = readName();
            std::string fromFile = readName();
            if (!moduleLoader_) {
                throw error::RuntimeError(
                    std::string("모듈 로더가 설정되지 않았습니다.\n") +
                    "해결 방법: Evaluator에 ModuleLoader를 설정해야 합니다.");
            }
            auto moduleEnv = moduleLoader_->loadModule(
                modulePath, fromFile,
                [this](ast::Program& program, const std::shared_ptr<Environment>& env) { runModule(program, env); });
            for (const auto& [name, value] : moduleEnv->getAllBindings()) {
                env_->set(name, value);
            }
            break;
        }

        // ---- 예외 ----
        case OpCode::TRY_PUSH: {
            uint16_t catchOffset = readShort();
            uint16_t finallyOffset = readShort();
            Handler handler;
            handler.hasCatch = catchOffset != NO_OFFSET;
            handler.hasFinally = finallyOffset != NO_OFFSET;
            handler.catchIp = handler.hasCatch ? ip_ + catchOffset : 0;
            handler.finallyIp = handler.hasFinally ? ip_ + finallyOffset : 0;
            handler.stackDepth = stack_.size();
            handler.frameDepth = frames_.size();
            handler.env = env_;
            handlers_.push_back(std::move(handler));
            break;
        }
        case OpCode::TRY_POP:
            if (!handlers_.empty()) handlers_.pop_back();
            break;
        case OpCode::THROW:
            raise(Evaluator::throwValue(pop()));
            break;
        case OpCode::END_FINALLY: {
            if (pending_.empty()) break;
            PendingAction action = std::move(pending_.back());
            pending_.pop_back();
            if (action.kind == PendingAction::Kind::RETURN) {
                return doReturn(action.value);
            }
            raise(action.value);
            break;
        }

        // ---- 비동기 ----
        case OpCode::AWAIT:
            push(Evaluator::awaitValue(pop()));
            break;
        case OpCode::BUILD_PROMISE:
            push(Value::createPromise(std::make_shared<evaluator::Promise>()));
            break;
        case OpCode::PROMISE_RESOLVE: {
            Value value = pop();
            Value promise = pop();
            if (promise.isPromise()) promise.asPromise()->resolve(value);
            break;
        }
        case OpCode::PROMISE_REJECT: {
            Value reason = pop();
            Value promise = pop();
            if (promise.isPromise()) promise.asPromise()->reject(reason);
            break;
        }

        // ---- 패턴 매칭 ----
        case OpCode::MATCH_PATTERN: {
            const auto* pattern = static_cast<const ast::Pattern*>(chunk_->getNode(readShort()));
            Value value = pop();
            push(Value::createBoolean(Evaluator::matchPattern(pattern, value, *env_, Evaluator::literalValue)));
            break;
        }
        case OpCode::MATCH_FAIL:
            throw error::RuntimeError("매칭되는 패턴이 없습니다");

        // ---- 기타 ----
        case OpCode::EVAL_NODE: {
            auto* node = const_cast<ast::Node*>(chunk_->getNode(readShort()));
            // 평가기에 현재 환경을 감싼 스코프를 주어 평가한다 (소멸 시 그 스코프만 놓는다)
            Evaluator evaluator(std::make_shared<Environment>(env_));
            push(evaluator.eval(node));
            break;
        }
        case OpCode::POP:
            pop();
            break;
        case OpCode::DUP:
            push(peek());
            break;
        case OpCode::PRINT:
            Logger::info(pop().toString());
            break;
        case OpCode::HALT:
            return Step::HALT;
    }

    return Step::CONTINUE;
}

// ============================================================================
// 호출·복귀
// ============================================================================

void VM::callValue(uint8_t argCount, const std::string& name) {
    Value callee = peek(argCount);

    // 내장 함수
    if (callee.isBuiltinFunction()) {
        std::vector<Value> args = popArguments(argCount);
        pop();  // 피호출자
        push(callee.asBuiltinFunction()(args));
        return;
    }

    if (!callee.isFunction()) {
        throw error::TypeError(
            "함수가 아닌 값을 호출하려고 합니다: " + callee.toString() + "\n" +
            "해결 방법: 함수 이름이 올바른지 확인하고, 함수가 정의되어 있는지 확인하세요.");
    }

    auto func = callee.asFunction();
    size_t stackBase = stack_.size() - argCount - 1;

    // 클래스 생성자
    if (func->classDef()) {
        auto classDef = func->classDef();
        auto ctor = classDef->constructor();
        if (!ctor) {
            std::vector<Value> args = popArguments(argCount);
            pop();
            push(Evaluator::constructInstance(func, args));
            return;
        }
        if (argCount != ctor->parameters().size()) {
            throw error::ArgumentError(
                "생성자 인자 개수가 일치하지 않습니다: 필요 " + std::to_string(ctor->parameters().size()) + "개, " +
                "전달 " + std::to_string(argCount) + "개");
        }
        size_t entry;
        if (!chunk_->findEntry(ctor->body(), entry)) {
            std::vector<Value> args = popArguments(argCount);
            pop();
            push(Evaluator::constructInstance(func, args));
            return;
        }
        auto instance = Value::createClassInstance(std::make_shared<evaluator::ClassInstance>(classDef));
        auto ctorEnv = std::make_shared<Environment>(ctor->closure());
        for (size_t i = 0; i < ctor->parameters().size(); ++i) {
            ctorEnv->set(ctor->parameters()[i], stack_[stackBase + 1 + i]);
        }
        ctorEnv->set("자신", instance);
        enterFunction(ctor, entry, std::move(ctorEnv), stackBase, true, instance, classDef->className() + ".생성자");
        return;
    }

    if (argCount != func->parameters().size()) {
        throw error::ArgumentError(
            "함수 인자 개수가 일치하지 않습니다: 필요 " + std::to_string(func->parameters().size()) + "개, " +
            "전달 " + std::to_string(argCount) + "개\n" +
            "해결 방법: 함수 정의를 확인하고 올바른 개수의 인자를 전달하세요.");
    }

    size_t entry;
    if (!chunk_->findEntry(func->body(), entry)) {
        // 이 청크에 없는 함수 (모듈·평가기가 만든 함수): 평가기로 본문을 해석한다
        std::vector<Value> args = popArguments(argCount);
        pop();
        push(Evaluator::invokeFunction(callee, args, name.empty() ? std::string("<익명 함수>") : name));
        return;
    }

    auto funcEnv = std::make_shared<Environment>(func->closure());
    for (size_t i = 0; i < func->parameters().size(); ++i) {
        funcEnv->set(func->parameters()[i], stack_[stackBase + 1 + i]);
    }
    enterFunction(func, entry, std::move(funcEnv), stackBase, false, Value(),
                  name.empty() ? std::string("<익명 함수>") : name);
}

void VM::callMethod(const std::string& name, uint8_t argCount) {
    Value object = peek(argCount);

    if (object.isPromise()) {
        std::vector<Value> args = popArguments(argCount);
        pop();
        push(Evaluator::promiseMethod(object, name, args));
        return;
    }

    if (!object.isClassInstance()) {
        throw error::RuntimeError("메서드 호출은 클래스 인스턴스 또는 Promise에만 가능합니다");
    }

    auto instance = object.asClassInstance();
    auto method = instance->classDef()->getMethod(name);
    if (!method) {
        throw error::RuntimeError("메서드를 찾을 수 없습니다: " + name);
    }
    if (argCount != method->parameters().size()) {
        throw error::ArgumentError(
            "메서드 인자 개수가 일치하지 않습니다: 필요 " + std::to_string(method->parameters().size()) + "개, " +
            "전달 " + std::to_string(argCount) + "개");
    }

    size_t stackBase = stack_.size() - argCount - 1;
    auto methodEnv = std::make_shared<Environment>(method->closure());
    for (size_t i = 0; i < method->parameters().size(); ++i) {
        methodEnv->set(method->parameters()[i], stack_[stackBase + 1 + i]);
    }
    methodEnv->set("자신", object);

    size_t entry;
    if (!chunk_->findEntry(method->body(), entry)) {
        stack_.resize(stackBase);
        push(Evaluator::runBody(method->body(), std::move(methodEnv)));
        return;
    }
    enterFunction(method, entry, std::move(methodEnv), stackBase, false, Value(),
                  instance->classDef()->className() + "." + name);
}

void VM::enterFunction(const std::shared_ptr<evaluator::Function>& func, size_t entry,
                       std::shared_ptr<Environment> funcEnv, size_t stackBase,
                       bool isConstructor, const Value& instance, std::string name) {
    if (frames_.size() >= maxFrames_) {
        throw error::RuntimeError(
            "함수 호출 깊이가 너무 깊습니다 (" + std::to_string(maxFrames_) + " 초과). 끝나지 않는 재귀 호출인지 확인하세요.");
    }
    CallFrame frame;
    frame.name = std::move(name);
    frame.returnIp = ip_;
    frame.savedEnv = env_;
    frame.frameEnv = funcEnv;
    frame.stackBase = stackBase;
    frame.handlerBase = handlers_.size();
    frame.isAsync = func->isAsync();
    frame.isConstructor = isConstructor;
    frame.instance = instance;
    frames_.push_back(std::move(frame));

    stack_.resize(stackBase);   // 피호출자와 인자는 환경에 묶였으니 치운다
    env_ = std::move(funcEnv);
    ip_ = entry;
}

VM::Step VM::doReturn(Value result) {
    if (frames_.empty()) {
        // 최상위 반환: 결과를 남기고 끝낸다
        push(result);
        return Step::HALT;
    }

    CallFrame& frame = frames_.back();

    // 이 프레임 안에 finally 가 있는 핸들러가 남아 있으면 먼저 그 블록을 돌린다
    while (handlers_.size() > frame.handlerBase) {
        Handler handler = handlers_.back();
        handlers_.pop_back();
        if (handler.hasFinally) {
            pending_.push_back({PendingAction::Kind::RETURN, result});
            stack_.resize(handler.stackDepth);
            env_ = handler.env;
            ip_ = handler.finallyIp;
            return Step::CONTINUE;
        }
    }

    CallFrame done = std::move(frames_.back());
    frames_.pop_back();

    Value value = done.isConstructor ? done.instance
                : done.isAsync       ? Evaluator::wrapAsync(result)
                                     : result;

    stack_.resize(done.stackBase);
    ip_ = done.returnIp;
    dropFrame(done);
    push(value);
    return Step::CONTINUE;
}

void VM::dropFrame(const CallFrame& frame) {
    env_ = frame.savedEnv;
    auto scope = frame.frameEnv;
    releaseScope(scope);
}

// ============================================================================
// 예외
// ============================================================================

void VM::raise(const Value& error) {
    if (!canHandle()) {
        // 처리되지 않은 에러: 프레임을 걷어 내기 전에 에러 값을 그대로 실은 예외로 바꿔 실행 루프가
        // 호출 스택을 붙여 보고하게 한다. 하위 VM(모듈)에서 새어 나오면 가져온 쪽의 시도 블록이 같은 값을 잡는다
        throw evaluator::ThrowSignal(error);
    }
    unwindToHandler(error);
}

bool VM::canHandle() const {
    if (!handlers_.empty()) {
        return true;
    }
    for (const auto& frame : frames_) {
        if (frame.isAsync) return true;
    }
    return false;
}

bool VM::rejectAsyncFrame(const Value& error, size_t frameDepth) {
    // 핸들러에 닿기 전에 비동기 함수 프레임을 지나면, 에러는 그 함수의 Promise 거부가 된다
    while (frames_.size() > frameDepth) {
        CallFrame frame = std::move(frames_.back());
        frames_.pop_back();
        if (frame.isAsync) {
            while (handlers_.size() > frame.handlerBase) handlers_.pop_back();
            stack_.resize(frame.stackBase);
            ip_ = frame.returnIp;
            dropFrame(frame);
            push(Evaluator::wrapAsync(error));
            return true;
        }
        dropFrame(frame);
    }
    return false;
}

bool VM::unwindToHandler(const Value& error) {
    while (!handlers_.empty()) {
        Handler handler = handlers_.back();

        // 핸들러보다 안쪽 프레임을 걷어 낸다. 비동기 프레임을 만나면 거기서 멈춘다
        if (rejectAsyncFrame(error, handler.frameDepth)) {
            return true;
        }
        handlers_.pop_back();
        stack_.resize(handler.stackDepth);
        env_ = handler.env;

        if (handler.hasCatch) {
            // 오류 변수에는 에러를 문자열로 바꿔 묶는다 (평가기와 같다)
            push(Value::createString(error.toString()));
            ip_ = handler.catchIp;
            return true;
        }

        // finally 만 있는 핸들러: 블록을 돌린 뒤 다시 던진다
        pending_.push_back({PendingAction::Kind::THROW, error});
        ip_ = handler.finallyIp;
        return true;
    }

    // 처리할 핸들러가 없다: 비동기 프레임이 있으면 거부로 바꾸고, 아니면 보고하게 한다
    if (rejectAsyncFrame(error, 0)) {
        return true;
    }
    return false;
}

// ============================================================================
// 보고
// ============================================================================

error::SourceLocation VM::locationAt(size_t offset) const {
    if (!chunk_ || chunk_->size() == 0) {
        return error::SourceLocation();   // 줄 0: 알 수 없음
    }
    return error::SourceLocation(chunk_->getFile(offset), chunk_->getLine(offset), chunk_->getColumn(offset));
}

void VM::attachStackTrace(error::KingSejongError& error) const {
    // 명령 하나의 바이트는 모두 같은 줄로 적히므로, 마지막으로 읽은 바이트(ip_-1)가 지금 명령의 위치다.
    // 프레임의 복귀 주소 바로 앞 바이트는 그 프레임을 만든 CALL 명령에 속한다
    size_t at = ip_ > 0 ? ip_ - 1 : 0;
    for (size_t i = frames_.size(); i-- > 0;) {
        error.addStackFrame(frames_[i].name, locationAt(at));
        at = frames_[i].returnIp > 0 ? frames_[i].returnIp - 1 : 0;
    }
    error.addStackFrame("<최상위>", locationAt(at));
}

VMResult VM::fail(const error::KingSejongError& error) {
    lastError_ = std::make_unique<error::KingSejongError>(error);
    return VMResult::RUNTIME_ERROR;
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

} // namespace bytecode
} // namespace kingsejong
