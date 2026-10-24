#pragma once

/**
 * @file Evaluator.h
 * @brief KingSejong 언어의 AST 실행 엔진
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "runtime/Value.h"
#include "runtime/Environment.h"
#include "../ast/Node.h"
#include "../ast/Expression.h"
#include "../ast/Statement.h"
#include "../error/Error.h"
#include "lexer/JosaRecognizer.h"
#include <memory>
#include <stdexcept>
#include <chrono>
#include <functional>

namespace kingsejong {

// Forward declarations
namespace module { class ModuleLoader; }
namespace jit { class HotPathDetector; }

namespace evaluator {

/**
 * @class ReturnValue
 * @brief 함수의 return 문을 나타내는 예외
 *
 * 함수 내에서 return 문이 실행되면 이 예외를 던져
 * 블록 실행을 중단하고 값을 반환합니다.
 */
class ReturnValue : public std::exception
{
private:
    Value value_;

public:
    explicit ReturnValue(Value value) : value_(std::move(value)) {}
    const Value& getValue() const { return value_; }
};

/**
 * @class Evaluator
 * @brief AST를 실행하여 Value를 생성하는 평가기
 *
 * AST 노드를 순회하면서 실행하고 결과를 Value로 반환합니다.
 * Environment를 통해 변수의 저장과 조회를 관리합니다.
 */
/**
 * @class ThrowSignal
 * @brief 던지다 로 던진 에러
 *
 * 시도 블록이 잡을 때까지 블록과 함수 호출을 거슬러 올라간다. KingSejongError 를 상속하므로
 * 아무도 잡지 않으면 main 이 실행 에러로 보고한다. 비동기 함수 안에서 던지면 Promise 거부가 된다.
 */
class ThrowSignal : public error::KingSejongError
{
private:
    Value value_;

public:
    explicit ThrowSignal(Value value)
        : error::KingSejongError(error::ErrorType::RUNTIME_ERROR, value.toString())
        , value_(std::move(value))
    {}
    const Value& value() const { return value_; }
};

/**
 * @class ExecutionObserver
 * @brief 평가기의 실행 흐름을 지켜보는 인터페이스 (디버거가 구현한다)
 *
 * 평가기는 관찰자를 알지만 디버거를 알지 않는다. 문장 하나를 평가하기 직전과
 * 함수 호출의 시작·끝에 알린다. 관찰자가 없으면 비용은 포인터 검사 하나다.
 */
class ExecutionObserver
{
public:
    virtual ~ExecutionObserver() = default;

    /// 문장을 평가하기 직전 (BlockStatement 포함)
    virtual void onStatement(ast::Statement* stmt, const std::shared_ptr<Environment>& env) = 0;

    /// 사용자 함수 본문에 들어가기 직전
    virtual void onFunctionEnter(const std::string& name, const ast::SourceLocation& callSite,
                                 const std::shared_ptr<Environment>& functionEnv) = 0;

    /// 함수 본문에서 나온 직후 (정상 반환, 반환문, 예외 모두)
    virtual void onFunctionExit() = 0;
};

class Evaluator
{
public:
    /**
     * @brief Evaluator 생성자
     *
     * 새로운 글로벌 환경을 생성합니다.
     */
    Evaluator();

    /**
     * @brief 환경을 지정하는 Evaluator 생성자
     * @param environment 사용할 환경
     */
    explicit Evaluator(std::shared_ptr<Environment> environment);

    /**
     * @brief 소멸자
     *
     * 이 평가기만 쥐고 있던 환경이면 저장소를 비워 전역 함수·클래스가 만든 순환 참조를 끊는다.
     */
    ~Evaluator();

    /**
     * @brief AST 노드를 평가
     * @param node 평가할 노드
     * @return 평가 결과 Value
     *
     * 노드의 타입에 따라 적절한 평가 함수를 호출합니다.
     */
    Value eval(ast::Node* node);

    /**
     * @brief 프로그램 노드를 평가
     * @param program 프로그램 루트 노드
     * @return 마지막 문장의 평가 결과
     */
    Value evalProgram(ast::Program* program);

    /**
     * @brief 표현식을 평가
     * @param expr 표현식 노드
     * @return 평가 결과 Value
     */
    Value evalExpression(ast::Expression* expr);

    /**
     * @brief 문장을 실행
     * @param stmt 문장 노드
     * @return 실행 결과 (대부분 null, return문은 값 반환)
     */
    Value evalStatement(ast::Statement* stmt);

    /**
     * @brief 현재 환경 반환
     * @return 환경 shared_ptr
     */
    std::shared_ptr<Environment> environment() const { return env_; }

    /**
     * @brief ModuleLoader 설정
     * @param loader 모듈 로더 포인터
     */
    void setModuleLoader(module::ModuleLoader* loader) { moduleLoader_ = loader; }

    /**
     * @brief HotPathDetector 설정
     * @param detector 핫 패스 감지기 포인터
     */
    void setHotPathDetector(jit::HotPathDetector* detector) { hotPathDetector_ = detector; }

    /**
     * @brief 실행 관찰자 설정 (디버거). nullptr 이면 알리지 않는다.
     */
    void setObserver(ExecutionObserver* observer) { observer_ = observer; }

    /**
     * @brief 최대 평가 횟수 설정
     * @param max 최대 평가 횟수
     */
    void setMaxEvaluations(size_t max) { maxEvaluations_ = max; }

    /**
     * @brief 최대 실행 시간 설정 (밀리초)
     * @param ms 최대 실행 시간
     */
    void setMaxExecutionTime(size_t ms) { maxExecutionTime_ = std::chrono::milliseconds(ms); }

    // ------------------------------------------------------------------
    // 값 의미 (value semantics)
    //
    // AST 평가기와 바이트코드 VM 이 같은 규칙으로 값을 다루도록, 노드가 아닌 값만 받는
    // 정적 함수로 두었다. 위의 eval* 멤버는 피연산자를 평가한 뒤 이 함수들을 부른다.
    // ------------------------------------------------------------------

    /// 이름 조회: "없음" → null, 환경 체인, 내장 함수 순. 없으면 NameError
    static Value lookupName(const Environment& env, const std::string& name);
    /// 이항 연산 (비교·논리·산술·문자열 연결·배열 연결·문자열 반복)
    static Value binaryOperation(const Value& left, const std::string& op, const Value& right);
    /// 단항 연산 (-, !)
    static Value unaryOperation(const std::string& op, const Value& operand);
    /// 배열[정수] / 사전["키"] 읽기
    static Value indexValue(const Value& container, const Value& index);
    /// 배열 슬라이싱 (step 은 없으면 nullptr)
    static Value sliceValue(const Value& container, const Value& start, const Value& end,
                            bool startInclusive, bool endInclusive, const Value* step);
    /// 배열[정수] = 값 / 사전["키"] = 값 (배열·사전은 공유되므로 제자리에서 바뀐다)
    static void assignIndex(Value& container, const Value& key, const Value& value);
    /// 클래스 인스턴스의 필드 읽기 / 쓰기
    static Value memberValue(const Value& object, const std::string& name);
    static void assignMember(const Value& object, const std::string& name, const Value& value);
    /// 문자열 보간에서 값이 글자로 바뀌는 규칙
    static std::string interpolationText(const Value& value);
    /// 던지다 의 값 → 에러 값
    static Value throwValue(const Value& value);
    /// 값을 함수로 호출한다 (내장 함수, 사용자 함수, 클래스 생성자, 비동기 함수). 본문은 AST 로 해석한다
    static Value invokeFunction(const Value& callee, const std::vector<Value>& args, const std::string& name);
    /// 조사 표현식 "객체<조사> 이름(인자...)" 의 런타임 규칙 ('의' 멤버 읽기 → 함수 호출 → 배열 메서드)
    static Value applyJosaMethod(const Value& object, lexer::JosaRecognizer::JosaType josa,
                                 const std::string& name, bool isCall, const std::vector<Value>& args,
                                 const std::shared_ptr<Environment>& env);
    /// 변환 관용구 "값을 <이름>으로 변환한다": 이름이 함수면 값을 넘겨 부른다
    static Value convertWith(const Value& object, const std::string& targetName,
                             const std::shared_ptr<Environment>& env);
    /// Promise 의 then/catch (그러면/오류시)
    static Value promiseMethod(const Value& promise, const std::string& methodName, const std::vector<Value>& args);
    /// 클래스 문장으로 클래스 정의를 만들고, 환경에 등록할 생성자 함수 값을 돌려준다
    static Value makeClass(const ast::ClassStatement* stmt, const std::shared_ptr<Environment>& env);

    /// 함수 본문을 주어진 환경에서 실행한다. 반환문은 값으로 바꾸고, 끝나면 환경을 놓는다.
    static Value runBody(ast::Statement* body, std::shared_ptr<Environment> env);
    /// 콜백 함수를 인자로 부른다 (매개변수 개수 검사는 호출자가 한다)
    static Value runCallback(const Value& callback, const std::vector<Value>& args);
    /// 클래스 생성자 함수(classDef 가 붙은 Function)로 인스턴스를 만든다
    static Value constructInstance(const std::shared_ptr<Function>& func, const std::vector<Value>& args);
    /// 비동기 함수의 결과를 Promise 로 감싼다 (에러 값이면 reject)
    static Value wrapAsync(const Value& result);
    /// 대기: Promise 면 이벤트 루프를 돌려 끝날 때까지 기다린 뒤 그 값(거부면 이유)을 돌려준다.
    /// 남은 작업이 없는데도 미완료면 끝낼 길이 없으므로 Promise 를 그대로 돌려준다. Promise 가 아니면 그대로
    static Value awaitValue(const Value& value);

    /// 리터럴 패턴의 표현식을 값으로 바꾸는 함수 (평가기는 eval, VM 은 상수 변환)
    using PatternLiteral = std::function<Value(const ast::Expression*)>;
    /// 값이 패턴에 맞으면 바인딩 변수를 bindings 에 넣고 true. 배열 패턴은 요소마다 재귀하며
    /// ...나머지 가 있으면 남은 요소를 배열로 묶는다. 리터럴은 종류가 같고 값이 같아야 한다.
    static bool matchPattern(const ast::Pattern* pattern, const Value& value, Environment& bindings,
                             const PatternLiteral& literal);
    /// 정수·실수·문자열·불리언 리터럴 노드를 값으로 바꾼다. 리터럴이 아니면 RuntimeError
    static Value literalValue(const ast::Expression* expr);
    /// 표현식이 리터럴 노드인지 (패턴을 VM 이 상수로 비교할 수 있는지)
    static bool isLiteralNode(const ast::Expression* expr);

private:
    std::shared_ptr<Environment> env_;  ///< 변수 저장 환경

    /**
     * @brief 스코프를 떠날 때 호출. 환경이 자기 안의 함수에만 붙잡혀 있으면 순환을 끊고 핸들을 놓는다
     * @param scope 호출자가 쥔 마지막 핸들. 돌아오면 비어 있다.
     */
    static void leaveScope(std::shared_ptr<Environment>& scope);
    module::ModuleLoader* moduleLoader_ = nullptr;  ///< 모듈 로더
    jit::HotPathDetector* hotPathDetector_ = nullptr;  ///< 핫 패스 감지기
    ExecutionObserver* observer_ = nullptr;  ///< 실행 관찰자 (디버거)

    // 안전 장치
    size_t evaluationCount_ = 0;  ///< 평가 카운터
    size_t maxEvaluations_ = 10000000;  ///< 최대 평가 횟수 (기본: 10,000,000)
    std::chrono::steady_clock::time_point startTime_;  ///< 실행 시작 시간
    std::chrono::milliseconds maxExecutionTime_ = std::chrono::milliseconds(5000);  ///< 최대 실행 시간 (기본: 5000ms)

    // Expression 평가 함수들

    /**
     * @brief 정수 리터럴 평가
     * @param lit 정수 리터럴 노드
     * @return 정수 Value
     */
    Value evalIntegerLiteral(ast::IntegerLiteral* lit);

    /**
     * @brief 실수 리터럴 평가
     * @param lit 실수 리터럴 노드
     * @return 실수 Value
     */
    Value evalFloatLiteral(ast::FloatLiteral* lit);

    /**
     * @brief 문자열 리터럴 평가
     * @param lit 문자열 리터럴 노드
     * @return 문자열 Value
     */
    Value evalStringLiteral(ast::StringLiteral* lit);

    /**
     * @brief 문자열 보간 평가 (Phase 7.2)
     * @param interp 문자열 보간 노드
     * @return 평가된 문자열 Value
     */
    Value evalInterpolatedString(ast::InterpolatedString* interp);

    /**
     * @brief 불리언 리터럴 평가
     * @param lit 불리언 리터럴 노드
     * @return 불리언 Value
     */
    Value evalBooleanLiteral(ast::BooleanLiteral* lit);

    /**
     * @brief 식별자 평가 (변수 조회)
     * @param ident 식별자 노드
     * @return 변수의 값
     */
    Value evalIdentifier(ast::Identifier* ident);

    /**
     * @brief 이항 연산 평가
     * @param expr 이항 연산 노드
     * @return 연산 결과 Value
     */
    Value evalBinaryExpression(ast::BinaryExpression* expr);

    /**
     * @brief 단항 연산 평가
     * @param expr 단항 연산 노드
     * @return 연산 결과 Value
     */
    Value evalUnaryExpression(ast::UnaryExpression* expr);

    /**
     * @brief 함수 리터럴 평가
     * @param lit 함수 리터럴 노드
     * @return 함수 Value
     */
    Value evalFunctionLiteral(ast::FunctionLiteral* lit);

    /**
     * @brief 비동기 함수 리터럴 평가 (Phase 7.3)
     * @param lit 비동기 함수 리터럴 노드
     * @return Promise를 반환하는 함수 Value
     */
    Value evalAsyncFunctionLiteral(ast::AsyncFunctionLiteral* lit);

    /**
     * @brief await 표현식 평가 (Phase 7.3)
     * @param expr await 표현식 노드
     * @return Promise의 해결 값 Value
     */
    Value evalAwaitExpression(ast::AwaitExpression* expr);

    /**
     * @brief 함수 호출 평가
     * @param expr 함수 호출 노드
     * @return 함수 실행 결과 Value
     */
    Value evalCallExpression(ast::CallExpression* expr);

    /**
     * @brief 배열 리터럴 평가
     * @param lit 배열 리터럴 노드
     * @return 배열 Value
     */
    Value evalArrayLiteral(ast::ArrayLiteral* lit);

    /**
     * @brief 딕셔너리 리터럴 평가 (Phase 7.2)
     * @param lit 딕셔너리 리터럴 노드
     * @return 딕셔너리 Value
     */
    Value evalDictionaryLiteral(ast::DictionaryLiteral* lit);

    /**
     * @brief 인덱스 표현식 평가
     * @param expr 인덱스 표현식 노드
     * @return 배열 요소 Value
     */
    Value evalIndexExpression(ast::IndexExpression* expr);

    /**
     * @brief 조사 표현식 평가
     * @param expr 조사 표현식 노드
     * @return 메서드 실행 결과 Value
     */
    Value evalJosaExpression(ast::JosaExpression* expr);

    /**
     * @brief 함수 값(사용자 함수·내장 함수·생성자)을 인자와 함께 호출한다
     *
     * 조사 디스패치(x를 f(a) == f(x, a))처럼 CallExpression 노드가 없는 호출에 쓴다.
     */
    Value callValue(const Value& callee, const std::vector<Value>& args,
                    const std::string& name, const ast::SourceLocation& callSite);

    /**
     * @brief 패턴 매칭 표현식 평가
     * @param expr 패턴 매칭 표현식 노드
     * @return 매칭된 케이스의 결과 Value
     */
    Value evalMatchExpression(ast::MatchExpression* expr);

    /**
     * @brief 멤버 접근 표현식 평가
     * @param expr 멤버 접근 표현식 노드
     * @return 필드 값 Value
     */
    Value evalMemberAccessExpression(ast::MemberAccessExpression* expr);

    /**
     * @brief this(자신) 표현식 평가
     * @param expr this 표현식 노드
     * @return 현재 인스턴스 Value
     */
    Value evalThisExpression(ast::ThisExpression* expr);

    // Phase 9.2: evalCallExpression() 헬퍼 메서드들

    /**
     * @brief Promise 메서드 호출 (.then, .catch)
     * @param memberAccess 멤버 접근 표현식
     * @param arguments 함수 인자 목록
     * @param promiseObj Promise 객체
     * @return 체이닝을 위한 Promise Value
     */
    Value evalPromiseMethodCall(
        ast::MemberAccessExpression* memberAccess,
        const std::vector<std::unique_ptr<ast::Expression>>& arguments,
        const Value& promiseObj
    );

    /**
     * @brief 클래스 인스턴스 메서드 호출
     * @param memberAccess 멤버 접근 표현식
     * @param arguments 함수 인자 목록
     * @param instanceObj 클래스 인스턴스 객체
     * @return 메서드 실행 결과 Value
     */
    Value evalClassMethodCall(
        ast::MemberAccessExpression* memberAccess,
        const std::vector<std::unique_ptr<ast::Expression>>& arguments,
        const Value& instanceObj
    );

    /**
     * @brief 클래스 생성자 호출
     * @param func 클래스 생성자 함수
     * @param args 생성자 인자 목록
     * @return 생성된 클래스 인스턴스 Value
     */
    Value evalConstructorCall(
        std::shared_ptr<Function> func,
        const std::vector<Value>& args
    );

    /**
     * @brief 일반 사용자 정의 함수 호출
     * @param expr 함수 호출 표현식
     * @param func 함수 객체
     * @param args 함수 인자 목록
     * @return 함수 실행 결과 Value
     */
    Value evalRegularFunctionCall(
        ast::CallExpression* expr,
        std::shared_ptr<Function> func,
        const std::vector<Value>& args
    );

    // Statement 실행 함수들

    /**
     * @brief 표현식 문장 실행
     * @param stmt 표현식 문장 노드
     * @return 표현식의 평가 결과
     */
    Value evalExpressionStatement(ast::ExpressionStatement* stmt);

    /**
     * @brief 변수 선언 실행
     * @param stmt 변수 선언 노드
     * @return null Value
     */
    Value evalVarDeclaration(ast::VarDeclaration* stmt);

    /**
     * @brief 변수 할당 실행
     * @param stmt 변수 할당 노드
     * @return 할당된 Value
     */
    Value evalAssignmentStatement(ast::AssignmentStatement* stmt);

    /**
     * @brief 반환 문장 실행
     * @param stmt 반환 문장 노드
     * @return 반환할 Value
     */
    Value evalReturnStatement(ast::ReturnStatement* stmt);

    /**
     * @brief 조건 문장 실행
     * @param stmt 조건 문장 노드
     * @return 실행된 블록의 평가 결과
     */
    Value evalIfStatement(ast::IfStatement* stmt);

    /**
     * @brief 조건 반복문 실행: 조건이 참인 동안 본문을 반복한다
     */
    Value evalWhileStatement(ast::WhileStatement* stmt);

    /**
     * @brief N번 반복 문장 실행
     * @param stmt N번 반복 문장 노드
     * @return 마지막 반복의 평가 결과
     */
    Value evalRepeatStatement(ast::RepeatStatement* stmt);

    /**
     * @brief 범위 반복 문장 실행
     * @param stmt 범위 반복 문장 노드
     * @return 마지막 반복의 평가 결과
     */
    Value evalRangeForStatement(ast::RangeForStatement* stmt);

    /**
     * @brief 블록 문장 실행
     * @param stmt 블록 문장 노드
     * @return 마지막 문장의 평가 결과
     */
    Value evalBlockStatement(ast::BlockStatement* stmt);

    /**
     * @brief 모듈 가져오기 실행
     * @param stmt 모듈 가져오기 노드
     * @return null Value
     */
    Value evalImportStatement(ast::ImportStatement* stmt);

    /**
     * @brief 예외 던지기 실행
     * @param stmt 예외 던지기 노드
     * @return 에러 Value
     */
    Value evalThrowStatement(ast::ThrowStatement* stmt);

    /**
     * @brief 예외 처리 블록 실행
     * @param stmt 예외 처리 블록 노드
     * @return 블록 실행 결과
     */
    Value evalTryStatement(ast::TryStatement* stmt);

    /**
     * @brief 클래스 정의 실행
     * @param stmt 클래스 정의 노드
     * @return null Value
     */
    Value evalClassStatement(ast::ClassStatement* stmt);

    // 헬퍼 함수들

    /**
     * @brief 두 정수 값에 대한 산술 연산 수행
     * @param left 왼쪽 피연산자
     * @param op 연산자 (+, -, *, /, %)
     * @param right 오른쪽 피연산자
     * @return 연산 결과
     */
    static Value applyIntegerOperation(int64_t left, const std::string& op, int64_t right);

    /**
     * @brief 두 실수 값에 대한 산술 연산 수행
     * @param left 왼쪽 피연산자
     * @param op 연산자 (+, -, *, /)
     * @param right 오른쪽 피연산자
     * @return 연산 결과
     */
    static Value applyFloatOperation(double left, const std::string& op, double right);

    /**
     * @brief 두 값의 비교 연산 수행
     * @param left 왼쪽 피연산자
     * @param op 연산자 (==, !=, <, >, <=, >=)
     * @param right 오른쪽 피연산자
     * @return 비교 결과 (불리언)
     */
    static Value applyComparisonOperation(const Value& left, const std::string& op, const Value& right);

    /**
     * @brief 논리 연산 수행
     * @param left 왼쪽 피연산자
     * @param op 연산자 (&&, ||)
     * @param right 오른쪽 피연산자
     * @return 논리 연산 결과 (불리언)
     */
    static Value applyLogicalOperation(const Value& left, const std::string& op, const Value& right);

    /**
     * @brief 안전 장치 체크 (무한 루프 방지)
     *
     * 평가 횟수와 실행 시간을 체크하여 제한을 초과하면 예외를 던집니다.
     */
    void checkSafetyLimits();
};

} // namespace evaluator
} // namespace kingsejong
