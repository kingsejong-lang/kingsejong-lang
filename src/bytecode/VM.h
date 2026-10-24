#pragma once
/**
 * @file VM.h
 * @brief 바이트코드 가상 머신
 *
 * 값 스택은 표현식의 임시 값만 담고, 변수는 평가기와 같은 Environment 체인에 이름으로
 * 산다. 함수 호출은 프레임을 쌓고 클로저 환경을 감싼 새 환경으로 들어간다. 값의 의미
 * (연산·인덱스·조사·호출·클래스)는 Evaluator 의 정적 함수를 그대로 써서 평가기와 결과가 같다.
 */

#include "Chunk.h"
#include "runtime/Value.h"
#include "runtime/Environment.h"
#include <vector>
#include <memory>
#include <chrono>
#include <string>

namespace kingsejong {
namespace module { class ModuleLoader; }
namespace bytecode {

/// @name 실행 제한 기본값
/// @{
constexpr size_t VM_DEFAULT_MAX_INSTRUCTIONS = 100000000;   ///< 최대 명령어 수 (100M)
constexpr int VM_DEFAULT_MAX_EXECUTION_TIME_MS = 5000;      ///< 최대 실행 시간 (5초)
constexpr size_t VM_DEFAULT_MAX_STACK_SIZE = 10000;         ///< 값 스택 최대 크기
constexpr size_t VM_DEFAULT_MAX_FRAMES = 10000;             ///< 호출 깊이 최대
/// @}

enum class VMResult {
    OK,             ///< 성공
    HALT,           ///< 정상 종료 (HALT 명령)
    COMPILE_ERROR,  ///< 컴파일 에러
    RUNTIME_ERROR   ///< 런타임 에러
};

/// 함수 호출 프레임
struct CallFrame {
    size_t returnIp;                                 ///< 복귀 주소
    std::shared_ptr<evaluator::Environment> savedEnv; ///< 호출 전 환경
    std::shared_ptr<evaluator::Environment> frameEnv; ///< 이 호출의 환경 (끝나면 놓는다)
    size_t stackBase;                                ///< 피호출자가 있던 스택 위치 (복귀 시 여기까지 잘라낸다)
    size_t handlerBase;                              ///< 호출 시점의 핸들러 스택 크기
    bool isAsync;                                    ///< 비동기 함수: 결과를 Promise 로 감싼다
    bool isConstructor;                              ///< 생성자: 결과 대신 인스턴스를 돌려준다
    evaluator::Value instance;                       ///< 생성자가 만드는 인스턴스
};

/// 시도/오류/마지막 핸들러
struct Handler {
    size_t catchIp;      ///< 오류 블록 진입점 (hasCatch 일 때)
    size_t finallyIp;    ///< 마지막 블록 진입점 (hasFinally 일 때)
    bool hasCatch;
    bool hasFinally;
    size_t stackDepth;   ///< 등록 시점의 값 스택 크기
    size_t frameDepth;   ///< 등록 시점의 프레임 수
    std::shared_ptr<evaluator::Environment> env;  ///< 등록 시점의 환경
};

/// finally 블록이 끝나면 이어 가야 할 일
struct PendingAction {
    enum class Kind { RETURN, THROW };
    Kind kind;
    evaluator::Value value;
};

class VM {
public:
    VM();
    ~VM();

    void setTraceExecution(bool trace) { traceExecution_ = trace; }
    void setMaxInstructions(size_t max) { maxInstructions_ = max; }
    void setMaxExecutionTime(size_t ms) { maxExecutionTime_ = std::chrono::milliseconds(ms); }
    void setMaxStackSize(size_t max) { maxStackSize_ = max; }
    void setModuleLoader(module::ModuleLoader* loader) { moduleLoader_ = loader; }

    /// 청크를 처음부터 실행한다
    VMResult run(Chunk* chunk);

    /// 스택 맨 위 값 (비어 있으면 null)
    evaluator::Value top() const;

    std::shared_ptr<evaluator::Environment> globals() const { return globals_; }

private:
    enum class Step { CONTINUE, HALT };

    Chunk* chunk_;
    size_t ip_;
    std::vector<evaluator::Value> stack_;
    std::vector<CallFrame> frames_;
    std::vector<Handler> handlers_;
    std::vector<PendingAction> pending_;
    std::shared_ptr<evaluator::Environment> globals_;
    std::shared_ptr<evaluator::Environment> env_;   ///< 현재 환경
    module::ModuleLoader* moduleLoader_ = nullptr;

    bool traceExecution_;
    size_t instructionCount_;
    size_t maxInstructions_;
    std::chrono::steady_clock::time_point startTime_;
    std::chrono::milliseconds maxExecutionTime_;
    size_t maxStackSize_;
    size_t maxFrames_;

    // ---- 디코딩 ----
    uint8_t readByte();
    uint16_t readShort();
    const std::string& readName();

    // ---- 스택 ----
    void push(const evaluator::Value& value);
    evaluator::Value pop();
    const evaluator::Value& peek(size_t distance = 0) const;
    std::vector<evaluator::Value> popArguments(size_t count);

    // ---- 실행 ----
    Step executeInstruction();
    void checkLimits();
    void callValue(uint8_t argCount);
    void callMethod(const std::string& name, uint8_t argCount);
    void enterFunction(const std::shared_ptr<evaluator::Function>& func, size_t entry,
                       std::shared_ptr<evaluator::Environment> funcEnv, size_t stackBase,
                       bool isConstructor, const evaluator::Value& instance);
    Step doReturn(evaluator::Value result);
    void raise(const evaluator::Value& error);
    bool unwindToHandler(const evaluator::Value& error);
    void dropFrame(const CallFrame& frame);
    static void releaseScope(std::shared_ptr<evaluator::Environment>& scope);

    // ---- 보고 ----
    void runtimeError(const std::string& message);
    void printStack() const;
};

} // namespace bytecode
} // namespace kingsejong
