/**
 * @file Evaluator.cpp
 * @brief KingSejong 언어의 AST 실행 엔진 구현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Evaluator.h"
#include "Builtin.h"
#include "runtime/EventLoop.h"
#include "../error/Error.h"
#include "../module/ModuleLoader.h"
#include "jit/HotPathDetector.h"
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace kingsejong {
namespace evaluator {

// ============================================================================
// 생성자
// ============================================================================

Evaluator::Evaluator()
    : env_(std::make_shared<Environment>())
    , startTime_(std::chrono::steady_clock::now())  // evalProgram 없이 쓰여도 시간 한도가 맞게 잡힌다
{
}

Evaluator::Evaluator(std::shared_ptr<Environment> environment)
    : env_(environment)
    , startTime_(std::chrono::steady_clock::now())
{
}

Evaluator::~Evaluator()
{
    // 환경을 바깥과 같이 쓰는 경우(REPL, 모듈 로더)는 use_count 가 맞지 않아 손대지 않는다.
    leaveScope(env_);
}

void Evaluator::leaveScope(std::shared_ptr<Environment>& scope)
{
    if (scope)
    {
        scope->releaseIfUnreachable(1);
        scope.reset();
    }
}

// ============================================================================
// 메인 평가 함수
// ============================================================================

Value Evaluator::eval(ast::Node* node)
{
    if (!node)
    {
        return Value::createNull();
    }

    // 안전 장치 체크 (무한 루프 방지)
    checkSafetyLimits();

    // 디버거 등 관찰자에게 문장 단위로 알린다
    if (observer_)
    {
        if (auto* stmt = ast::node_cast<ast::Statement>(node))
        {
            observer_->onStatement(stmt, env_);
        }
    }

    switch (node->type())
    {
        // Expressions
        case ast::NodeType::IDENTIFIER:
            return evalIdentifier(static_cast<ast::Identifier*>(node));

        case ast::NodeType::INTEGER_LITERAL:
            return evalIntegerLiteral(static_cast<ast::IntegerLiteral*>(node));

        case ast::NodeType::FLOAT_LITERAL:
            return evalFloatLiteral(static_cast<ast::FloatLiteral*>(node));

        case ast::NodeType::STRING_LITERAL:
            return evalStringLiteral(static_cast<ast::StringLiteral*>(node));

        case ast::NodeType::INTERPOLATED_STRING:
            return evalInterpolatedString(static_cast<ast::InterpolatedString*>(node));

        case ast::NodeType::BOOLEAN_LITERAL:
            return evalBooleanLiteral(static_cast<ast::BooleanLiteral*>(node));

        case ast::NodeType::BINARY_EXPRESSION:
            return evalBinaryExpression(static_cast<ast::BinaryExpression*>(node));

        case ast::NodeType::UNARY_EXPRESSION:
            return evalUnaryExpression(static_cast<ast::UnaryExpression*>(node));

        case ast::NodeType::FUNCTION_LITERAL:
            return evalFunctionLiteral(static_cast<ast::FunctionLiteral*>(node));

        case ast::NodeType::ASYNC_FUNCTION_LITERAL:
            return evalAsyncFunctionLiteral(static_cast<ast::AsyncFunctionLiteral*>(node));

        case ast::NodeType::AWAIT_EXPRESSION:
            return evalAwaitExpression(static_cast<ast::AwaitExpression*>(node));

        case ast::NodeType::CALL_EXPRESSION:
            return evalCallExpression(static_cast<ast::CallExpression*>(node));

        case ast::NodeType::MATCH_EXPRESSION:
            return evalMatchExpression(static_cast<ast::MatchExpression*>(node));

        case ast::NodeType::ARRAY_LITERAL:
            return evalArrayLiteral(static_cast<ast::ArrayLiteral*>(node));

        case ast::NodeType::DICTIONARY_LITERAL:
            return evalDictionaryLiteral(static_cast<ast::DictionaryLiteral*>(node));

        case ast::NodeType::INDEX_EXPRESSION:
            return evalIndexExpression(static_cast<ast::IndexExpression*>(node));

        case ast::NodeType::JOSA_EXPRESSION:
            return evalJosaExpression(static_cast<ast::JosaExpression*>(node));

        case ast::NodeType::MEMBER_ACCESS_EXPRESSION:
            return evalMemberAccessExpression(static_cast<ast::MemberAccessExpression*>(node));

        case ast::NodeType::THIS_EXPRESSION:
            return evalThisExpression(static_cast<ast::ThisExpression*>(node));

        // Statements
        case ast::NodeType::PROGRAM:
            return evalProgram(static_cast<ast::Program*>(node));

        case ast::NodeType::EXPRESSION_STATEMENT:
            return evalExpressionStatement(static_cast<ast::ExpressionStatement*>(node));

        case ast::NodeType::VAR_DECLARATION:
            return evalVarDeclaration(static_cast<ast::VarDeclaration*>(node));

        case ast::NodeType::ASSIGNMENT_STATEMENT:
            return evalAssignmentStatement(static_cast<ast::AssignmentStatement*>(node));

        case ast::NodeType::RETURN_STATEMENT:
            return evalReturnStatement(static_cast<ast::ReturnStatement*>(node));

        case ast::NodeType::IF_STATEMENT:
            return evalIfStatement(static_cast<ast::IfStatement*>(node));

        case ast::NodeType::WHILE_STATEMENT:
            return evalWhileStatement(static_cast<ast::WhileStatement*>(node));

        case ast::NodeType::REPEAT_STATEMENT:
            return evalRepeatStatement(static_cast<ast::RepeatStatement*>(node));

        case ast::NodeType::RANGE_FOR_STATEMENT:
            return evalRangeForStatement(static_cast<ast::RangeForStatement*>(node));

        case ast::NodeType::BLOCK_STATEMENT:
            return evalBlockStatement(static_cast<ast::BlockStatement*>(node));

        case ast::NodeType::IMPORT_STATEMENT:
            return evalImportStatement(static_cast<ast::ImportStatement*>(node));

        case ast::NodeType::THROW_STATEMENT:
            return evalThrowStatement(static_cast<ast::ThrowStatement*>(node));

        case ast::NodeType::TRY_STATEMENT:
            return evalTryStatement(static_cast<ast::TryStatement*>(node));

        case ast::NodeType::CLASS_STATEMENT:
            return evalClassStatement(static_cast<ast::ClassStatement*>(node));

        default:
        {
            std::string msg = "평가되지 않은 노드 타입: " + ast::nodeTypeToString(node->type());
            throw error::RuntimeError(msg);
        }
    }
}

Value Evaluator::evalProgram(ast::Program* program)
{
    // 실행 시작 시간 기록
    startTime_ = std::chrono::steady_clock::now();
    evaluationCount_ = 0;

    Value result = Value::createNull();

    for (const auto& stmt : program->statements())
    {
        result = eval(stmt.get());
    }

    // 프로그램 실행 후 Event Loop 실행 (microtask 처리)
    auto& eventLoop = getGlobalEventLoop();
    eventLoop.run();

    return result;
}

Value Evaluator::evalExpression(ast::Expression* expr)
{
    return eval(expr);
}

Value Evaluator::evalStatement(ast::Statement* stmt)
{
    return eval(stmt);
}

// ============================================================================
// Expression 평가 함수들
// ============================================================================

Value Evaluator::evalIntegerLiteral(ast::IntegerLiteral* lit)
{
    return Value::createInteger(lit->value());
}

Value Evaluator::evalFloatLiteral(ast::FloatLiteral* lit)
{
    return Value::createFloat(lit->value());
}

Value Evaluator::evalStringLiteral(ast::StringLiteral* lit)
{
    return Value::createString(lit->value());
}

Value Evaluator::evalInterpolatedString(ast::InterpolatedString* interp)
{
    // Phase 7.2: 문자열 보간 평가
    const auto& parts = interp->parts();
    const auto& expressions = interp->expressions();

    std::string result;
    for (size_t i = 0; i < parts.size(); ++i)
    {
        // 문자열 파트 추가
        result += parts[i];

        // 표현식 평가 및 추가
        if (i < expressions.size())
        {
            result += interpolationText(eval(const_cast<ast::Expression*>(expressions[i].get())));
        }
    }

    return Value::createString(result);
}

std::string Evaluator::interpolationText(const Value& value)
{
    if (value.isString())
    {
        return value.asString();
    }
    if (value.isInteger())
    {
        return std::to_string(value.asInteger());
    }
    if (value.isFloat())
    {
        return std::to_string(value.asFloat());
    }
    if (value.isBoolean())
    {
        return value.asBoolean() ? "참" : "거짓";
    }
    // 기타 타입은 기본 toString 사용
    return value.toString();
}

Value Evaluator::evalBooleanLiteral(ast::BooleanLiteral* lit)
{
    return Value::createBoolean(lit->value());
}

Value Evaluator::evalIdentifier(ast::Identifier* ident)
{
    return lookupName(*env_, ident->name());
}

Value Evaluator::lookupName(const Environment& env, const std::string& name)
{
    // 0. 언어 참조의 null 리터럴 "없음". 변수로 가려지지 않는 한 null 이다.
    if (name == "없음" && !env.existsInChain(name))
    {
        return Value::createNull();
    }

    // 1. 먼저 환경에서 변수 조회
    if (env.existsInChain(name))
    {
        return env.get(name);
    }

    // 2. 내장 함수 조회
    if (Builtin::hasBuiltin(name))
    {
        return Builtin::getBuiltin(name);
    }

    // 3. 정의되지 않음
    throw error::NameError(
        "정의되지 않은 변수 '" + name + "'를 사용하려고 합니다.\n" +
        "해결 방법: 변수를 먼저 선언하세요. 예: 정수 " + name + " = 값"
    );
}

Value Evaluator::evalBinaryExpression(ast::BinaryExpression* expr)
{
    const std::string& op = expr->op();

    // Phase 7.1: 할당 연산자 특별 처리
    if (op == "=")
    {
        // 우변 평가
        Value value = eval(const_cast<ast::Expression*>(expr->right()));

        // 좌변이 MemberAccessExpression이면 필드 할당
        if (auto* memberAccess = ast::node_cast<ast::MemberAccessExpression>(expr->left()))
        {
            Value obj = eval(const_cast<ast::Expression*>(memberAccess->object()));
            assignMember(obj, memberAccess->memberName(), value);
            return value;
        }
        // 좌변이 Identifier이면 변수 할당
        if (auto* ident = ast::node_cast<ast::Identifier>(expr->left()))
        {
            env_->set(ident->name(), value);
            return value;
        }
        // 좌변이 IndexExpression이면 배열 요소·사전 키 할당 (배열[i] = 값, 사전["키"] = 값)
        if (auto* indexExpr = ast::node_cast<ast::IndexExpression>(expr->left()))
        {
            Value container = eval(const_cast<ast::Expression*>(indexExpr->array()));
            Value key = eval(const_cast<ast::Expression*>(indexExpr->index()));
            assignIndex(container, key, value);
            return value;
        }
        throw error::RuntimeError("할당의 좌변이 올바르지 않습니다 (변수, 필드, 배열[인덱스], 사전[키] 만 가능합니다)");
    }

    Value left = eval(const_cast<ast::Expression*>(expr->left()));
    Value right = eval(const_cast<ast::Expression*>(expr->right()));
    return binaryOperation(left, op, right);
}

Value Evaluator::binaryOperation(const Value& left, const std::string& op, const Value& right)
{
    // 비교 연산자
    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
    {
        return applyComparisonOperation(left, op, right);
    }

    // 논리 연산자 (그리고/또는 은 &&/|| 와 같다)
    if (op == "&&" || op == "||" || op == "그리고" || op == "또는")
    {
        return applyLogicalOperation(left, op, right);
    }

    // 산술 연산자
    // 정수 + 정수 = 정수
    if (left.isInteger() && right.isInteger())
    {
        return applyIntegerOperation(left.asInteger(), op, right.asInteger());
    }

    // 실수가 포함된 경우 실수 연산
    if ((left.isInteger() || left.isFloat()) && (right.isInteger() || right.isFloat()))
    {
        double leftVal = left.isInteger() ? static_cast<double>(left.asInteger()) : left.asFloat();
        double rightVal = right.isInteger() ? static_cast<double>(right.asInteger()) : right.asFloat();
        return applyFloatOperation(leftVal, op, rightVal);
    }

    // 배열 + 배열: 이어 붙인 새 배열 (표준 라이브러리가 "결과 = 결과 + [쌍]" 으로 쓴다)
    if (op == "+" && left.isArray() && right.isArray())
    {
        std::vector<Value> joined = left.asArray();
        const auto& tail = right.asArray();
        joined.insert(joined.end(), tail.begin(), tail.end());
        return Value::createArray(joined);
    }

    // 문자열 연결
    if (op == "+" && left.isString() && right.isString())
    {
        return Value::createString(left.asString() + right.asString());
    }

    // 문자열 * 정수 (또는 정수 * 문자열): 반복. "=" * 60 은 구분선 그리기에 쓴다. 0 이하면 빈 문자열.
    if (op == "*" && ((left.isString() && right.isInteger()) || (left.isInteger() && right.isString())))
    {
        const std::string& text = left.isString() ? left.asString() : right.asString();
        int64_t count = left.isString() ? right.asInteger() : left.asInteger();
        std::string repeated;
        for (int64_t i = 0; i < count; ++i)
        {
            repeated += text;
        }
        return Value::createString(repeated);
    }

    throw error::RuntimeError("지원되지 않는 연산: " + left.toString() + " " + op + " " + right.toString());
}

Value Evaluator::evalUnaryExpression(ast::UnaryExpression* expr)
{
    Value operand = eval(const_cast<ast::Expression*>(expr->operand()));
    return unaryOperation(expr->op(), operand);
}

Value Evaluator::unaryOperation(const std::string& op, const Value& operand)
{
    if (op == "-")
    {
        if (operand.isInteger())
        {
            return Value::createInteger(-operand.asInteger());
        }
        if (operand.isFloat())
        {
            return Value::createFloat(-operand.asFloat());
        }
        throw error::TypeError("음수 연산은 숫자에만 적용 가능합니다");
    }

    if (op == "!")
    {
        return Value::createBoolean(!operand.isTruthy());
    }

    throw error::RuntimeError("지원되지 않는 단항 연산자: " + op);
}

/**
 * @brief 함수 리터럴 평가
 *
 * 함수 정의를 평가하여 Function 객체를 생성합니다.
 * 현재 환경을 클로저로 캡처합니다.
 */
Value Evaluator::evalFunctionLiteral(ast::FunctionLiteral* lit)
{
    // 함수 매개변수
    std::vector<std::string> parameters = lit->parameters();

    // 함수 본문 (BlockStatement)
    ast::Statement* body = lit->body();

    // 클로저: 현재 환경을 캡처
    std::shared_ptr<Environment> closure = env_;

    // Function 객체 생성
    auto func = std::make_shared<Function>(std::move(parameters), body, closure);

    return Value::createFunction(func);
}

/**
 * @brief 비동기 함수 리터럴 평가 (Phase 7.3)
 *
 * 비동기 함수 정의를 평가하여 Function 객체를 생성합니다.
 * async 함수는 호출 시 Promise를 반환합니다.
 */
Value Evaluator::evalAsyncFunctionLiteral(ast::AsyncFunctionLiteral* lit)
{
    // 함수 매개변수
    std::vector<std::string> parameters = lit->parameters();

    // 함수 본문 (BlockStatement)
    ast::Statement* body = lit->body();

    // 클로저: 현재 환경을 캡처
    std::shared_ptr<Environment> closure = env_;

    // Function 객체 생성 (isAsync=true)
    auto func = std::make_shared<Function>(std::move(parameters), body, closure, false, true);

    return Value::createFunction(func);
}

/**
 * @brief await 표현식 평가
 *
 * Promise 가 아직 끝나지 않았으면 이벤트 루프를 돌려 끝날 때까지 기다린다 (awaitValue).
 */
Value Evaluator::evalAwaitExpression(ast::AwaitExpression* expr)
{
    Value value = eval(const_cast<ast::Expression*>(expr->argument()));
    if (value.isError())
    {
        return value;
    }
    return awaitValue(value);
}

Value Evaluator::awaitValue(const Value& value)
{
    if (!value.isPromise())
    {
        return value;
    }

    auto promise = value.asPromise();
    if (!promise->isSettled())
    {
        // 미뤄 둔 작업(then 콜백 등)을 돌리면 이 Promise 가 끝날 수 있다
        getGlobalEventLoop().runUntil([&promise]() { return promise->isSettled(); });
    }
    if (promise->isSettled())
    {
        return promise->value();
    }
    return value;
}

// ============================================================================
// Phase 9.2: evalCallExpression() 헬퍼 메서드들
// ============================================================================

/**
 * @brief Promise 메서드 호출 (.then, .catch)
 */
Value Evaluator::evalPromiseMethodCall(
    ast::MemberAccessExpression* memberAccess,
    const std::vector<std::unique_ptr<ast::Expression>>& arguments,
    const Value& promiseObj
)
{
    std::vector<Value> args;
    for (const auto& argExpr : arguments)
    {
        args.push_back(eval(const_cast<ast::Expression*>(argExpr.get())));
    }
    return promiseMethod(promiseObj, memberAccess->memberName(), args);
}

Value Evaluator::promiseMethod(const Value& promiseObj, const std::string& methodName, const std::vector<Value>& args)
{
    auto promise = promiseObj.asPromise();

    // 인자 (콜백 함수)
    if (args.size() != 1)
    {
        throw error::ArgumentError(
            "Promise." + methodName + "()은 콜백 함수 1개를 인자로 받습니다"
        );
    }
    if (!args[0].isFunction())
    {
        throw error::TypeError(
            "Promise." + methodName + "()의 인자는 함수여야 합니다"
        );
    }

    auto callbackFunc = args[0].asFunction();

    // then/catch 는 콜백의 결과로 끝나는 새 Promise 를 돌려준다 (체이닝)
    auto newPromise = std::make_shared<Promise>();

    // Continuation: 값을 첫 매개변수에 묶어 콜백 본문을 실행하고, 그 결과로 새 Promise 를 끝낸다.
    // 콜백이 던진 에러는 새 Promise 의 거부가 된다.
    auto continuation = [callbackFunc, newPromise](const Value& value) -> Value {
        try
        {
            auto funcEnv = std::make_shared<Environment>(callbackFunc->closure());
            if (!callbackFunc->parameters().empty())
            {
                funcEnv->set(callbackFunc->parameters()[0], value);
            }
            Value result = runBody(callbackFunc->body(), std::move(funcEnv));
            if (result.isError())
            {
                newPromise->reject(result);
            }
            else
            {
                newPromise->resolve(result);
            }
            return result;
        }
        catch (const ThrowSignal& signal)
        {
            newPromise->reject(signal.value());
            return signal.value();
        }
    };

    // 메서드에 따라 콜백 등록
    if (methodName == "then" || methodName == "그러면")
    {
        if (promise->state() == Promise::State::FULFILLED)
        {
            // 이미 이행된 경우 즉시 실행
            continuation(promise->value());
            return Value::createPromise(newPromise);
        }
        if (promise->state() == Promise::State::PENDING)
        {
            promise->then(continuation);
            // 원래 Promise 가 거부되면 새 Promise 도 같은 이유로 거부된다 (뒤의 catch 로 전달)
            promise->catchError([newPromise](const Value& reason) -> Value {
                newPromise->reject(reason);
                return reason;
            });
            return Value::createPromise(newPromise);
        }
        // 이미 거부된 경우: 거부가 그대로 전달되므로 원래 Promise 를 돌려준다
    }
    else if (methodName == "catch" || methodName == "오류시")
    {
        if (promise->state() == Promise::State::REJECTED)
        {
            // 이미 거부된 경우 즉시 실행
            continuation(promise->value());
            return Value::createPromise(newPromise);
        }
        if (promise->state() == Promise::State::PENDING)
        {
            promise->catchError(continuation);
            // 원래 Promise 가 이행되면 새 Promise 도 같은 값으로 이행된다 (뒤의 then 으로 전달)
            promise->then([newPromise](const Value& value) -> Value {
                newPromise->resolve(value);
                return value;
            });
            return Value::createPromise(newPromise);
        }
        // 이미 이행된 경우: 값이 그대로 전달되므로 원래 Promise 를 돌려준다
    }
    else
    {
        throw error::RuntimeError("Promise에 '" + methodName + "' 메서드가 없습니다");
    }

    return promiseObj;
}

/**
 * @brief 클래스 인스턴스 메서드 호출
 */
Value Evaluator::evalClassMethodCall(
    ast::MemberAccessExpression* memberAccess,
    const std::vector<std::unique_ptr<ast::Expression>>& arguments,
    const Value& instanceObj
)
{
    auto instance = instanceObj.asClassInstance();
    auto classDef = instance->classDef();

    // 메서드 찾기
    auto method = classDef->getMethod(memberAccess->memberName());
    if (!method)
    {
        throw error::RuntimeError("메서드를 찾을 수 없습니다: " + memberAccess->memberName());
    }

    // 인자 평가
    std::vector<Value> args;
    for (const auto& argExpr : arguments)
    {
        Value argValue = eval(const_cast<ast::Expression*>(argExpr.get()));
        args.push_back(argValue);
    }

    // 매개변수 개수 확인
    if (args.size() != method->parameters().size())
    {
        throw error::ArgumentError(
            "메서드 인자 개수가 일치하지 않습니다: 필요 " + std::to_string(method->parameters().size()) + "개, " +
            "전달 " + std::to_string(args.size()) + "개"
        );
    }

    // 새로운 환경 생성 (메서드의 클로저 기반)
    auto methodEnv = std::make_shared<Environment>(method->closure());

    // 매개변수 바인딩
    for (size_t i = 0; i < method->parameters().size(); ++i)
    {
        methodEnv->set(method->parameters()[i], args[i]);
    }

    // '자신' 키워드를 인스턴스에 바인딩
    methodEnv->set("자신", instanceObj);

    // 기존 환경 저장
    auto previousEnv = env_;

    // 메서드 환경으로 전환
    env_ = methodEnv;

    // 메서드 본문 실행
    Value result = Value::createNull();
    try
    {
        result = eval(method->body());
    }
    catch (const ReturnValue& returnVal)
    {
        result = returnVal.getValue();
    }

    // 환경 복원
    env_ = previousEnv;
    leaveScope(methodEnv);

    return result;
}

/**
 * @brief 클래스 생성자 호출
 */
Value Evaluator::evalConstructorCall(
    std::shared_ptr<Function> func,
    const std::vector<Value>& args
)
{
    return constructInstance(func, args);
}

Value Evaluator::constructInstance(const std::shared_ptr<Function>& func, const std::vector<Value>& args)
{
    auto classDef = func->classDef();
    auto instance = std::make_shared<ClassInstance>(classDef);

    // 생성자가 있으면 실행
    if (classDef->constructor())
    {
        auto ctor = classDef->constructor();

        // 매개변수 개수 확인
        if (args.size() != ctor->parameters().size())
        {
            throw error::ArgumentError(
                "생성자 인자 개수가 일치하지 않습니다: 필요 " + std::to_string(ctor->parameters().size()) + "개, " +
                "전달 " + std::to_string(args.size()) + "개"
            );
        }

        // 새로운 환경 생성 (생성자의 클로저 기반)
        auto ctorEnv = std::make_shared<Environment>(ctor->closure());

        // 매개변수 바인딩
        for (size_t i = 0; i < ctor->parameters().size(); ++i)
        {
            ctorEnv->set(ctor->parameters()[i], args[i]);
        }

        // '자신' 키워드를 인스턴스에 바인딩
        ctorEnv->set("자신", Value::createClassInstance(instance));

        // 생성자 본문 실행 (생성자에서 return 은 무시한다)
        runBody(ctor->body(), std::move(ctorEnv));
    }

    return Value::createClassInstance(instance);
}

Value Evaluator::runBody(ast::Statement* body, std::shared_ptr<Environment> env)
{
    // 본문을 다 돌고 나면 Evaluator 소멸자가 환경을 놓아 함수-환경 순환 참조를 끊는다
    Evaluator evaluator(std::move(env));
    try
    {
        return evaluator.eval(body);
    }
    catch (const ReturnValue& returnVal)
    {
        return returnVal.getValue();
    }
}

Value Evaluator::runCallback(const Value& callback, const std::vector<Value>& args)
{
    auto func = callback.asFunction();
    auto funcEnv = func->closure()->createEnclosed();
    const auto& params = func->parameters();
    for (size_t i = 0; i < params.size() && i < args.size(); ++i)
    {
        funcEnv->set(params[i], args[i]);
    }
    return runBody(func->body(), std::move(funcEnv));
}

Value Evaluator::wrapAsync(const Value& result)
{
    auto promise = std::make_shared<Promise>();
    if (result.isError())
    {
        promise->reject(result);
    }
    else
    {
        promise->resolve(result);
    }
    return Value::createPromise(promise);
}

/**
 * @brief 일반 사용자 정의 함수 호출
 */
Value Evaluator::evalRegularFunctionCall(
    ast::CallExpression* expr,
    std::shared_ptr<Function> func,
    const std::vector<Value>& args
)
{
    // Hot Path Tracking: 함수 호출 추적
    std::string functionName = "anonymous";
    size_t functionId = reinterpret_cast<size_t>(func.get());

    // 함수 이름 추출 시도 (Identifier인 경우)
    if (auto* identExpr = ast::node_cast<ast::Identifier>(const_cast<ast::Expression*>(expr->function())))
    {
        functionName = identExpr->name();
    }

    auto startTime = std::chrono::steady_clock::now();

    // 새로운 환경 생성 (클로저 기반)
    auto funcEnv = std::make_shared<Environment>(func->closure());

    // 매개변수 바인딩
    for (size_t i = 0; i < func->parameters().size(); ++i)
    {
        funcEnv->set(func->parameters()[i], args[i]);
    }

    // 함수 본문 실행 (새로운 환경에서)
    // 기존 환경 저장
    auto previousEnv = env_;

    // 함수 환경으로 전환
    env_ = funcEnv;

    // 관찰자에게 함수 진입을 알리고, 어떤 경로로 나가든 종료를 알린다
    struct ObserverExitGuard
    {
        ExecutionObserver* observer;
        ~ObserverExitGuard() { if (observer) observer->onFunctionExit(); }
    };
    if (observer_)
    {
        observer_->onFunctionEnter(functionName, expr->location(), funcEnv);
    }
    ObserverExitGuard exitGuard{observer_};

    // 함수 본문 실행
    Value result = Value::createNull();
    try
    {
        result = eval(func->body());
    }
    catch (const ReturnValue& returnVal)
    {
        // return 문으로 인한 정상적인 함수 종료
        result = returnVal.getValue();
    }
    catch (const ThrowSignal& signal)
    {
        // 비동기 함수 안에서 던진 에러는 Promise 거부가 된다
        if (!func->isAsync())
        {
            env_ = previousEnv;
            leaveScope(funcEnv);
            throw;
        }
        result = signal.value();
    }
    catch (...)
    {
        env_ = previousEnv;
        leaveScope(funcEnv);
        throw;
    }

    // 환경 복원
    env_ = previousEnv;
    leaveScope(funcEnv);

    // Hot Path Tracking: 실행 시간 기록
    if (hotPathDetector_)
    {
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            endTime - startTime
        ).count();

        hotPathDetector_->trackFunctionCall(functionName, functionId, duration);
    }

    // Phase 7.3: async 함수는 Promise를 반환
    if (func->isAsync())
    {
        // 결과를 Promise로 감싸서 반환
        auto promise = std::make_shared<Promise>();

        // 에러인 경우 reject, 아니면 resolve
        if (result.isError())
        {
            promise->reject(result);
        }
        else
        {
            promise->resolve(result);
        }

        return Value::createPromise(promise);
    }

    return result;
}

/**
 * @brief 함수 호출 평가
 *
 * 함수를 호출하고 결과를 반환합니다.
 * 1. 함수 평가 (Function 객체 얻기)
 * 2. 인자 평가
 * 3. 새로운 환경 생성 (클로저 기반)
 * 4. 매개변수 바인딩
 * 5. 함수 본문 실행
 */
Value Evaluator::evalCallExpression(ast::CallExpression* expr)
{
    // 1. 메서드 호출인지 확인 (MemberAccessExpression)
    if (auto* memberAccess = ast::node_cast<ast::MemberAccessExpression>(
        const_cast<ast::Expression*>(expr->function())))
    {
        Value obj = eval(const_cast<ast::Expression*>(memberAccess->object()));

        // Promise 메서드 호출
        if (obj.isPromise())
        {
            return evalPromiseMethodCall(memberAccess, expr->arguments(), obj);
        }

        // 클래스 인스턴스 메서드 호출
        if (obj.isClassInstance())
        {
            return evalClassMethodCall(memberAccess, expr->arguments(), obj);
        }

        throw error::RuntimeError("메서드 호출은 클래스 인스턴스 또는 Promise에만 가능합니다");
    }

    // 2. 함수 및 인자 평가
    Value funcValue = eval(const_cast<ast::Expression*>(expr->function()));

    std::vector<Value> args;
    for (const auto& argExpr : expr->arguments())
    {
        args.push_back(eval(const_cast<ast::Expression*>(argExpr.get())));
    }

    // 3. 내장 함수 호출
    if (funcValue.isBuiltinFunction())
    {
        return funcValue.asBuiltinFunction()(args);
    }

    // 4. 사용자 정의 함수인지 확인
    if (!funcValue.isFunction())
    {
        throw error::TypeError(
            "함수가 아닌 값을 호출하려고 합니다: " + funcValue.toString() + "\n" +
            "해결 방법: 함수 이름이 올바른지 확인하고, 함수가 정의되어 있는지 확인하세요."
        );
    }

    auto func = funcValue.asFunction();

    // 5. 클래스 생성자 호출
    if (func->classDef())
    {
        return evalConstructorCall(func, args);
    }

    // 6. 매개변수 개수 확인
    if (args.size() != func->parameters().size())
    {
        throw error::ArgumentError(
            "함수 인자 개수가 일치하지 않습니다: 필요 " + std::to_string(func->parameters().size()) + "개, " +
            "전달 " + std::to_string(args.size()) + "개\n" +
            "해결 방법: 함수 정의를 확인하고 올바른 개수의 인자를 전달하세요."
        );
    }

    // 7. 일반 함수 호출
    return evalRegularFunctionCall(expr, func, args);
}

// ============================================================================
// Statement 실행 함수들
// ============================================================================

Value Evaluator::evalWhileStatement(ast::WhileStatement* stmt)
{
    Value result = Value::createNull();
    while (true)
    {
        Value condition = eval(const_cast<ast::Expression*>(stmt->condition()));
        if (!condition.isTruthy())
        {
            break;
        }
        // 안전 한도(최대 평가 횟수·시간)는 eval 이 매 노드마다 검사한다
        result = eval(const_cast<ast::BlockStatement*>(stmt->body()));
    }
    return result;
}

Value Evaluator::evalExpressionStatement(ast::ExpressionStatement* stmt)
{
    return eval(const_cast<ast::Expression*>(stmt->expression()));
}

Value Evaluator::evalVarDeclaration(ast::VarDeclaration* stmt)
{
    Value value = Value::createNull();

    if (stmt->initializer())
    {
        value = eval(const_cast<ast::Expression*>(stmt->initializer()));
    }

    env_->set(stmt->varName(), value);
    return Value::createNull();
}

Value Evaluator::evalAssignmentStatement(ast::AssignmentStatement* stmt)
{
    // 값 평가
    Value value = eval(const_cast<ast::Expression*>(stmt->value()));

    // 환경에 값 설정 (스코프 체인 탐색)
    // 1. 상위 스코프에 변수가 있으면 → 상위 스코프에서 업데이트
    // 2. 없으면 → 현재 스코프에 새로 생성
    // 이것이 타입 추론 기능: `x = 10`처럼 타입 없이 변수를 선언할 수 있음
    env_->setWithLookup(stmt->varName(), value);

    return value;
}

Value Evaluator::evalReturnStatement(ast::ReturnStatement* stmt)
{
    Value returnValue;
    if (stmt->returnValue())
    {
        returnValue = eval(const_cast<ast::Expression*>(stmt->returnValue()));
    }
    else
    {
        returnValue = Value::createNull();
    }

    // return 문은 예외를 던져서 블록 실행을 중단합니다
    throw ReturnValue(returnValue);
}

Value Evaluator::evalIfStatement(ast::IfStatement* stmt)
{
    // 조건식 평가
    Value condition = eval(const_cast<ast::Expression*>(stmt->condition()));

    // 조건이 참이면 then 블록 실행
    if (condition.isTruthy())
    {
        return eval(const_cast<ast::BlockStatement*>(stmt->thenBranch()));
    }
    // 조건이 거짓이고 else 블록이 있으면 else 블록 실행
    else if (stmt->elseBranch())
    {
        return eval(const_cast<ast::BlockStatement*>(stmt->elseBranch()));
    }

    // else 블록이 없으면 null 반환
    return Value::createNull();
}

Value Evaluator::evalBlockStatement(ast::BlockStatement* stmt)
{
    Value result = Value::createNull();

    for (const auto& statement : stmt->statements())
    {
        result = eval(statement.get());
    }

    return result;
}

Value Evaluator::evalRepeatStatement(ast::RepeatStatement* stmt)
{
    // 반복 횟수 평가
    Value countValue = eval(const_cast<ast::Expression*>(stmt->count()));

    if (!countValue.isInteger())
    {
        throw error::TypeError(
            "반복 횟수는 정수여야 합니다: " + countValue.toString() + "\n" +
            "해결 방법: 반복 횟수에 정수 값을 사용하세요. 예: 10번 반복"
        );
    }

    int64_t count = countValue.asInteger();

    // 음수 반복 횟수 검증
    if (count < 0)
    {
        throw error::ValueError(
            "반복 횟수는 0 이상이어야 합니다: " + std::to_string(count) + "\n" +
            "해결 방법: 양수 또는 0을 사용하세요."
        );
    }

    // Hot Path Tracking: 루프 ID 생성
    size_t loopId = reinterpret_cast<size_t>(stmt);

    Value result = Value::createNull();

    // count 횟수만큼 반복
    for (int64_t i = 0; i < count; ++i)
    {
        // Hot Path Tracking: 루프 백엣지 추적
        if (hotPathDetector_)
        {
            hotPathDetector_->trackLoopBackedge(loopId);
        }

        result = eval(const_cast<ast::BlockStatement*>(stmt->body()));
    }

    return result;
}

Value Evaluator::evalRangeForStatement(ast::RangeForStatement* stmt)
{
    // 시작 값 평가
    Value startValue = eval(const_cast<ast::Expression*>(stmt->start()));

    if (!startValue.isInteger())
    {
        throw std::runtime_error("범위 시작 값은 정수여야 합니다");
    }

    // 끝 값 평가
    Value endValue = eval(const_cast<ast::Expression*>(stmt->end()));

    if (!endValue.isInteger())
    {
        throw std::runtime_error("범위 끝 값은 정수여야 합니다");
    }

    int64_t start = startValue.asInteger();
    int64_t end = endValue.asInteger();

    // Hot Path Tracking: 루프 ID 생성
    size_t loopId = reinterpret_cast<size_t>(stmt);

    Value result = Value::createNull();

    // 범위 반복 (끝 값 포함 여부에 따라 조건 변경)
    bool endInclusive = stmt->endInclusive();

    if (endInclusive)
    {
        // 끝 값 포함 (까지, 이하, 이상)
        for (int64_t i = start; i <= end; ++i)
        {
            // Hot Path Tracking: 루프 백엣지 추적
            if (hotPathDetector_)
            {
                hotPathDetector_->trackLoopBackedge(loopId);
            }

            // 반복 변수 설정
            env_->set(stmt->varName(), Value::createInteger(i));

            // 본문 실행
            result = eval(const_cast<ast::BlockStatement*>(stmt->body()));
        }
    }
    else
    {
        // 끝 값 미만 (미만)
        for (int64_t i = start; i < end; ++i)
        {
            // Hot Path Tracking: 루프 백엣지 추적
            if (hotPathDetector_)
            {
                hotPathDetector_->trackLoopBackedge(loopId);
            }

            // 반복 변수 설정
            env_->set(stmt->varName(), Value::createInteger(i));

            // 본문 실행
            result = eval(const_cast<ast::BlockStatement*>(stmt->body()));
        }
    }

    return result;
}

// ============================================================================
// 헬퍼 함수들
// ============================================================================

Value Evaluator::applyIntegerOperation(int64_t left, const std::string& op, int64_t right)
{
    if (op == "+") return Value::createInteger(left + right);
    if (op == "-") return Value::createInteger(left - right);
    if (op == "*") return Value::createInteger(left * right);
    if (op == "/")
    {
        if (right == 0)
        {
            throw error::ZeroDivisionError(
                "0으로 나눌 수 없습니다.\n"
                "해결 방법: 나누는 값이 0이 아닌지 확인하세요. 조건문을 사용하여 검사할 수 있습니다."
            );
        }
        return Value::createInteger(left / right);
    }
    if (op == "%")
    {
        if (right == 0)
        {
            throw error::ZeroDivisionError(
                "0으로 나머지 연산을 할 수 없습니다.\n"
                "해결 방법: 나머지 연산의 제수가 0이 아닌지 확인하세요."
            );
        }
        return Value::createInteger(left % right);
    }

    throw error::RuntimeError("지원되지 않는 정수 연산자: " + op);
}

Value Evaluator::applyFloatOperation(double left, const std::string& op, double right)
{
    if (op == "+") return Value::createFloat(left + right);
    if (op == "-") return Value::createFloat(left - right);
    if (op == "*") return Value::createFloat(left * right);
    if (op == "/")
    {
        if (std::abs(right) < 1e-9)
        {
            throw std::runtime_error("0으로 나눌 수 없습니다");
        }
        return Value::createFloat(left / right);
    }

    throw error::RuntimeError("지원되지 않는 실수 연산자: " + op);
}

Value Evaluator::applyComparisonOperation(const Value& left, const std::string& op, const Value& right)
{
    if (op == "==") return Value::createBoolean(left.equals(right));
    if (op == "!=") return Value::createBoolean(!left.equals(right));
    if (op == "<") return Value::createBoolean(left.lessThan(right));
    if (op == ">") return Value::createBoolean(left.greaterThan(right));
    if (op == "<=") return Value::createBoolean(left.lessThan(right) || left.equals(right));
    if (op == ">=") return Value::createBoolean(left.greaterThan(right) || left.equals(right));

    throw error::RuntimeError("지원되지 않는 비교 연산자: " + op);
}

Value Evaluator::applyLogicalOperation(const Value& left, const std::string& op, const Value& right)
{
    if (op == "&&" || op == "그리고")
    {
        // 단축 평가: left가 거짓이면 right를 평가하지 않음
        if (!left.isTruthy()) return Value::createBoolean(false);
        return Value::createBoolean(right.isTruthy());
    }

    if (op == "||" || op == "또는")
    {
        // 단축 평가: left가 참이면 right를 평가하지 않음
        if (left.isTruthy()) return Value::createBoolean(true);
        return Value::createBoolean(right.isTruthy());
    }

    throw error::RuntimeError("지원되지 않는 논리 연산자: " + op);
}

Value Evaluator::evalArrayLiteral(ast::ArrayLiteral* lit)
{
    std::vector<Value> elements;

    for (const auto& elem : lit->elements())
    {
        Value value = eval(const_cast<ast::Expression*>(elem.get()));
        elements.push_back(value);
    }

    return Value::createArray(elements);
}

Value Evaluator::evalDictionaryLiteral(ast::DictionaryLiteral* lit)
{
    std::unordered_map<std::string, Value> dict;

    for (const auto& [keyExpr, valueExpr] : lit->pairs())
    {
        // key 평가
        Value key = eval(const_cast<ast::Expression*>(keyExpr.get()));

        // key는 문자열이어야 함
        if (!key.isString())
        {
            throw error::TypeError("딕셔너리 키는 문자열이어야 합니다. 실제 타입: " + types::Type::typeKindToString(key.getType()));
        }

        // value 평가
        Value value = eval(const_cast<ast::Expression*>(valueExpr.get()));

        dict[key.asString()] = value;
    }

    return Value::createDictionary(dict);
}

Value Evaluator::evalIndexExpression(ast::IndexExpression* expr)
{
    Value left = eval(const_cast<ast::Expression*>(expr->array()));

    if (!left.isArray() && !left.isDictionary())
    {
        throw error::TypeError(
            "배열 또는 딕셔너리가 아닌 값에 인덱스 접근을 시도했습니다: " + left.toString() + "\n" +
            "해결 방법: 인덱스 접근([])은 배열 또는 딕셔너리 타입에만 사용할 수 있습니다."
        );
    }

    // 배열 슬라이싱: 인덱스가 범위 표현식일 때
    const ast::Expression* indexExpr = expr->index();
    if (left.isArray() && indexExpr->type() == ast::NodeType::RANGE_EXPRESSION)
    {
        const auto* rangeExpr = static_cast<const ast::RangeExpression*>(indexExpr);
        Value startValue = eval(const_cast<ast::Expression*>(rangeExpr->start()));
        Value endValue = eval(const_cast<ast::Expression*>(rangeExpr->end()));
        Value stepValue;
        if (rangeExpr->hasStep())
        {
            stepValue = eval(const_cast<ast::Expression*>(rangeExpr->step()));
        }
        return sliceValue(left, startValue, endValue, rangeExpr->startInclusive(), rangeExpr->endInclusive(),
                          rangeExpr->hasStep() ? &stepValue : nullptr);
    }

    Value index = eval(const_cast<ast::Expression*>(expr->index()));
    return indexValue(left, index);
}

Value Evaluator::indexValue(const Value& container, const Value& index)
{
    // 딕셔너리 인덱스 접근 (Phase 7.2)
    if (container.isDictionary())
    {
        // 딕셔너리 키는 문자열이어야 함
        if (!index.isString())
        {
            throw error::TypeError("딕셔너리 키는 문자열이어야 합니다. 실제 타입: " + types::Type::typeKindToString(index.getType()));
        }

        const auto& dict = container.asDictionary();
        std::string key = index.asString();

        auto it = dict.find(key);
        if (it == dict.end())
        {
            throw error::RuntimeError("딕셔너리에 키가 존재하지 않습니다: \"" + key + "\"");
        }

        return it->second;
    }

    // 배열인지 확인
    if (!container.isArray())
    {
        throw error::TypeError(
            "배열 또는 딕셔너리가 아닌 값에 인덱스 접근을 시도했습니다: " + container.toString() + "\n" +
            "해결 방법: 인덱스 접근([])은 배열 또는 딕셔너리 타입에만 사용할 수 있습니다."
        );
    }

    const std::vector<Value>& arr = container.asArray();

    // 인덱스가 정수인지 확인
    if (!index.isInteger())
    {
        throw std::runtime_error("배열 인덱스는 정수여야 합니다");
    }

    int64_t idx = index.asInteger();

    // 음수 인덱스 처리 (Python 스타일)
    if (idx < 0)
    {
        idx = static_cast<int64_t>(arr.size()) + idx;
    }

    // 범위 검사
    if (idx < 0 || idx >= static_cast<int64_t>(arr.size()))
    {
        throw error::IndexError(
            "배열 인덱스 범위를 벗어났습니다: 인덱스 " + std::to_string(idx) +
            ", 배열 크기 " + std::to_string(arr.size()) + "\n" +
            "해결 방법: 인덱스는 0부터 " + std::to_string(arr.size() - 1) + " 사이여야 합니다. " +
            "음수 인덱스(-1, -2 등)도 사용할 수 있습니다."
        );
    }

    return arr[static_cast<size_t>(idx)];
}

Value Evaluator::sliceValue(const Value& container, const Value& startValue, const Value& endValue,
                            bool startInclusive, bool endInclusive, const Value* step)
{
    if (!container.isArray())
    {
        throw error::TypeError(
            "배열 또는 딕셔너리가 아닌 값에 인덱스 접근을 시도했습니다: " + container.toString() + "\n" +
            "해결 방법: 인덱스 접근([])은 배열 또는 딕셔너리 타입에만 사용할 수 있습니다."
        );
    }
    const std::vector<Value>& arr = container.asArray();

    if (!startValue.isInteger())
    {
        throw std::runtime_error("슬라이싱 시작 인덱스는 정수여야 합니다");
    }
    if (!endValue.isInteger())
    {
        throw std::runtime_error("슬라이싱 끝 인덱스는 정수여야 합니다");
    }

    int64_t startIdx = startValue.asInteger();
    int64_t endIdx = endValue.asInteger();

    // 음수 인덱스 처리 (Python 스타일)
    if (startIdx < 0)
    {
        startIdx = static_cast<int64_t>(arr.size()) + startIdx;
    }
    if (endIdx < 0)
    {
        endIdx = static_cast<int64_t>(arr.size()) + endIdx;
    }

    // startInclusive/endInclusive 처리
    if (!startInclusive)
    {
        startIdx++; // "초과"는 다음 인덱스부터
    }
    if (!endInclusive)
    {
        endIdx--; // "미만"은 이전 인덱스까지
    }

    // 범위 검사
    if (startIdx < 0) startIdx = 0;
    if (endIdx >= static_cast<int64_t>(arr.size()))
    {
        endIdx = static_cast<int64_t>(arr.size()) - 1;
    }

    // 빈 슬라이스 처리
    if (startIdx > endIdx)
    {
        return Value::createArray({});
    }

    // Phase 7.2: Step 값 처리
    int64_t stepValue = 1; // 기본값은 1
    if (step)
    {
        if (!step->isInteger())
        {
            throw std::runtime_error("슬라이싱 step 값은 정수여야 합니다");
        }
        stepValue = step->asInteger();
        if (stepValue <= 0)
        {
            throw std::runtime_error("슬라이싱 step 값은 양수여야 합니다");
        }
    }

    // 슬라이싱 수행 (step 적용)
    std::vector<Value> sliced;
    for (int64_t i = startIdx; i <= endIdx; i += stepValue)
    {
        sliced.push_back(arr[static_cast<size_t>(i)]);
    }

    return Value::createArray(sliced);
}

void Evaluator::assignIndex(Value& container, const Value& key, const Value& value)
{
    if (container.isArray())
    {
        if (!key.isInteger())
        {
            throw error::TypeError("배열 인덱스는 정수여야 합니다: " + key.toString());
        }
        auto& elements = container.asArray();  // 배열은 공유되므로 제자리에서 바뀐다
        int64_t i = key.asInteger();
        if (i < 0 || static_cast<size_t>(i) >= elements.size())
        {
            throw error::RuntimeError(
                "배열 인덱스가 범위를 벗어났습니다: " + std::to_string(i) +
                " (길이 " + std::to_string(elements.size()) + ")");
        }
        elements[static_cast<size_t>(i)] = value;
        return;
    }
    if (container.isDictionary())
    {
        if (!key.isString())
        {
            throw error::TypeError("사전의 키는 문자열이어야 합니다: " + key.toString());
        }
        container.asDictionary()[key.asString()] = value;
        return;
    }
    throw error::TypeError("인덱스 할당은 배열이나 사전에만 할 수 있습니다: " + container.toString());
}

Value Evaluator::evalImportStatement(ast::ImportStatement* stmt)
{
    // ModuleLoader가 설정되지 않은 경우
    if (!moduleLoader_)
    {
        throw error::RuntimeError(
            std::string("모듈 로더가 설정되지 않았습니다.\n") +
            "해결 방법: Evaluator에 ModuleLoader를 설정해야 합니다."
        );
    }

    // 모듈 로딩
    // 가져오기 문이 있는 파일을 기준으로 상대 경로를 해석한다
    std::string modulePath = stmt->modulePath();
    auto moduleEnv = moduleLoader_->loadModule(modulePath, stmt->location().filename);

    // 모듈의 모든 변수를 현재 환경으로 가져오기
    for (const auto& [name, value] : moduleEnv->getAllBindings())
    {
        env_->set(name, value);
    }

    return Value::createNull();
}

/**
 * @brief 던지다 문장 실행
 *
 * 표현식을 평가하여 에러 Value를 생성하고 반환합니다.
 *
 * @example
 * 던지다 "에러 메시지"  → Error Value 반환
 * 던지다 에러_생성()    → 함수 결과를 에러로 변환
 */
Value Evaluator::evalThrowStatement(ast::ThrowStatement* stmt)
{
    // 에러는 예외로 전파되어 블록의 나머지 문장을 건너뛰고 가장 가까운 시도 블록에서 잡힌다
    throw ThrowSignal(throwValue(eval(const_cast<ast::Expression*>(stmt->value()))));
}

Value Evaluator::throwValue(const Value& value)
{
    // 이미 에러 타입이면 그대로 반환
    if (value.isError())
    {
        return value;
    }

    // 문자열인 경우 에러 메시지로 변환
    if (value.isString())
    {
        return Value::createError(value.asString());
    }

    // 그 외의 타입은 toString()으로 변환하여 에러 생성
    return Value::createError(value.toString());
}

/**
 * @brief 시도-오류-마지막 블록 실행
 *
 * 예외 처리 메커니즘:
 * 1. try 블록 실행
 * 2. 에러 발생 시 catch 블록 실행 (에러 변수 바인딩)
 * 3. finally 블록은 항상 실행 (에러 유무 관계없이)
 * 4. catch되지 않은 에러는 전파
 *
 * @example
 * 시도 {
 *     던지다 "에러"
 * } 오류 (e) {
 *     출력(e)
 * } 마지막 {
 *     정리()
 * }
 */
Value Evaluator::evalTryStatement(ast::TryStatement* stmt)
{
    Value result = Value::createNull();
    Value caughtError = Value::createNull();
    bool hasError = false;

    // 1. 시도 블록
    try
    {
        result = eval(const_cast<ast::BlockStatement*>(stmt->tryBlock()));

        // 에러 값이 블록의 값으로 흘러나온 경우도 잡는다 (에러 값을 돌려주는 함수)
        if (result.isError())
        {
            caughtError = result;
            hasError = true;
        }
    }
    catch (const ReturnValue&)
    {
        // 반환문은 시도 블록을 통과한다. 마지막 블록만 돌리고 다시 던진다
        if (stmt->finallyBlock())
        {
            eval(const_cast<ast::BlockStatement*>(stmt->finallyBlock()));
        }
        throw;
    }
    catch (const ThrowSignal& signal)
    {
        caughtError = signal.value();
        hasError = true;
    }
    catch (const std::exception& e)
    {
        // 내장 함수·연산이 던진 런타임 에러도 오류 블록으로 간다 (메시지를 그대로 전한다)
        caughtError = Value::createError(e.what(), "RuntimeError");
        hasError = true;
    }

    // 2. 오류 블록 (에러가 있고 오류 절이 있을 때)
    if (hasError && !stmt->catchClauses().empty())
    {
        const auto& catchClause = stmt->catchClauses()[0];  // 현재는 단일 오류 절만

        // 에러 변수를 묶을 새 스코프. 에러는 문자열로 바뀌어 묶인다
        auto catchEnv = std::make_shared<Environment>(env_);
        auto prevEnv = env_;
        env_ = catchEnv;
        env_->set(catchClause->errorVarName(), Value::createString(caughtError.toString()));

        try
        {
            result = eval(const_cast<ast::BlockStatement*>(catchClause->body()));
        }
        catch (...)
        {
            // 오류 블록이 다시 던지면 마지막 블록은 돌리고 전파한다
            env_ = prevEnv;
            leaveScope(catchEnv);
            if (stmt->finallyBlock())
            {
                eval(const_cast<ast::BlockStatement*>(stmt->finallyBlock()));
            }
            throw;
        }

        env_ = prevEnv;
        leaveScope(catchEnv);
        hasError = false;
    }

    // 3. 마지막 블록 (항상)
    if (stmt->finallyBlock())
    {
        eval(const_cast<ast::BlockStatement*>(stmt->finallyBlock()));
    }

    // 4. 잡지 못한 에러는 바깥으로 전파
    if (hasError)
    {
        throw ThrowSignal(caughtError);
    }

    return result;
}

Value Evaluator::callValue(const Value& callee, const std::vector<Value>& args,
                           const std::string& name, const ast::SourceLocation& callSite)
{
    // 관찰자(디버거)가 없으면 공용 호출 규칙을 그대로 쓴다
    if (!observer_)
    {
        return invokeFunction(callee, args, name);
    }

    if (callee.isBuiltinFunction())
    {
        return callee.asBuiltinFunction()(args);
    }
    if (!callee.isFunction())
    {
        throw error::TypeError(
            "함수가 아닌 값을 호출하려고 합니다: " + name + "\n" +
            "해결 방법: 조사 뒤에는 함수 이름이나 배열 메서드가 와야 합니다."
        );
    }
    auto func = callee.asFunction();
    if (func->classDef())
    {
        return constructInstance(func, args);
    }
    if (args.size() != func->parameters().size())
    {
        throw error::ArgumentError(
            "함수 인자 개수가 일치하지 않습니다: 필요 " + std::to_string(func->parameters().size()) + "개, " +
            "전달 " + std::to_string(args.size()) + "개 (" + name + ")\n" +
            "해결 방법: 조사 앞의 값이 첫 인자로 들어간다는 점을 확인하세요."
        );
    }

    auto funcEnv = std::make_shared<Environment>(func->closure());
    for (size_t i = 0; i < func->parameters().size(); ++i)
    {
        funcEnv->set(func->parameters()[i], args[i]);
    }

    auto previousEnv = env_;
    env_ = funcEnv;

    struct ObserverExitGuard
    {
        ExecutionObserver* observer;
        ~ObserverExitGuard() { if (observer) observer->onFunctionExit(); }
    };
    observer_->onFunctionEnter(name, callSite, funcEnv);
    ObserverExitGuard exitGuard{observer_};

    Value result = Value::createNull();
    try
    {
        result = eval(func->body());
    }
    catch (const ReturnValue& returnVal)
    {
        result = returnVal.getValue();
    }
    catch (const ThrowSignal& signal)
    {
        // 비동기 함수 안에서 던진 에러는 Promise 거부가 된다
        if (!func->isAsync())
        {
            env_ = previousEnv;
            leaveScope(funcEnv);
            throw;
        }
        result = signal.value();
    }
    catch (...)
    {
        env_ = previousEnv;
        leaveScope(funcEnv);
        throw;
    }
    env_ = previousEnv;
    leaveScope(funcEnv);

    if (func->isAsync())
    {
        return wrapAsync(result);
    }
    return result;
}

Value Evaluator::invokeFunction(const Value& callee, const std::vector<Value>& args, const std::string& name)
{
    if (callee.isBuiltinFunction())
    {
        return callee.asBuiltinFunction()(args);
    }
    if (!callee.isFunction())
    {
        throw error::TypeError(
            "함수가 아닌 값을 호출하려고 합니다: " + name + "\n" +
            "해결 방법: 조사 뒤에는 함수 이름이나 배열 메서드가 와야 합니다."
        );
    }
    auto func = callee.asFunction();
    if (func->classDef())
    {
        return constructInstance(func, args);
    }
    if (args.size() != func->parameters().size())
    {
        throw error::ArgumentError(
            "함수 인자 개수가 일치하지 않습니다: 필요 " + std::to_string(func->parameters().size()) + "개, " +
            "전달 " + std::to_string(args.size()) + "개 (" + name + ")\n" +
            "해결 방법: 조사 앞의 값이 첫 인자로 들어간다는 점을 확인하세요."
        );
    }

    auto funcEnv = std::make_shared<Environment>(func->closure());
    for (size_t i = 0; i < func->parameters().size(); ++i)
    {
        funcEnv->set(func->parameters()[i], args[i]);
    }

    Value result;
    try
    {
        result = runBody(func->body(), std::move(funcEnv));
    }
    catch (const ThrowSignal& signal)
    {
        // 비동기 함수 안에서 던진 에러는 Promise 거부가 된다
        if (!func->isAsync()) throw;
        return wrapAsync(signal.value());
    }
    if (func->isAsync())
    {
        return wrapAsync(result);
    }
    return result;
}

Value Evaluator::evalJosaExpression(ast::JosaExpression* expr)
{
    // 객체 평가
    Value object = eval(const_cast<ast::Expression*>(expr->object()));

    // 1. '의' 는 소유격: 클래스 인스턴스의 필드나 사전의 키를 읽는다 ("사용자의 이름")
    if (expr->josaType() == lexer::JosaRecognizer::JosaType::UI)
    {
        if (auto* member = ast::node_cast<ast::Identifier>(expr->method()))
        {
            return applyJosaMethod(object, expr->josaType(), member->name(), false, {}, env_);
        }
    }

    // 2. 변환 관용구: "값을 문자열로 변환한다" 는 "문자열(값)". 로/으로 뒤의 이름이 함수(내장·사용자)면
    //    그 함수에 값을 넘긴다. 동사는 변환한다/바꾼다.
    if (auto* inner = ast::node_cast<ast::JosaExpression>(expr->method()))
    {
        auto* verb = ast::node_cast<ast::Identifier>(inner->method());
        auto* target = ast::node_cast<ast::Identifier>(inner->object());
        if (inner->josaType() == lexer::JosaRecognizer::JosaType::RO_EURO && verb && target &&
            (verb->name() == "변환한다" || verb->name() == "바꾼다"))
        {
            return convertWith(object, target->name(), env_);
        }
    }

    // 3·4. 메서드 이름과 인자를 뽑아 공용 규칙에 맡긴다 (함수 호출 또는 배열 메서드)
    std::string name;
    bool isCall = false;
    std::vector<Value> args;
    if (auto* call = ast::node_cast<ast::CallExpression>(expr->method()))
    {
        isCall = true;
        if (auto* fn = ast::node_cast<ast::Identifier>(call->function()))
        {
            name = fn->name();
            for (const auto& arg : call->arguments())
            {
                args.push_back(eval(const_cast<ast::Expression*>(arg.get())));
            }
        }
    }
    else if (auto* ident = ast::node_cast<ast::Identifier>(expr->method()))
    {
        name = ident->name();
    }
    return applyJosaMethod(object, expr->josaType(), name, isCall, args, env_);
}

Value Evaluator::convertWith(const Value& object, const std::string& name, const std::shared_ptr<Environment>& env)
{
    if (env->existsInChain(name) || Builtin::hasBuiltin(name))
    {
        Value callee = env->existsInChain(name) ? env->get(name) : Builtin::getBuiltin(name);
        return invokeFunction(callee, {object}, name);
    }
    throw error::NameError(
        "'" + name + "' 로 변환하는 함수가 없습니다.\n"
        "해결 방법: 문자열, 정수, 실수 중 하나이거나 정의한 함수 이름을 쓰세요."
    );
}

Value Evaluator::applyJosaMethod(const Value& object, lexer::JosaRecognizer::JosaType josa,
                                 const std::string& name, bool isCall, const std::vector<Value>& args,
                                 const std::shared_ptr<Environment>& env)
{
    // 1. '의' 소유격 (메서드가 식별자일 때): 클래스 인스턴스의 필드나 사전의 키
    if (josa == lexer::JosaRecognizer::JosaType::UI && !isCall && !name.empty())
    {
        if (object.isClassInstance())
        {
            return object.asClassInstance()->getField(name);
        }
        if (object.isDictionary())
        {
            const auto& dict = object.asDictionary();
            auto it = dict.find(name);
            if (it == dict.end())
            {
                throw error::NameError(
                    "사전에 '" + name + "' 키가 없습니다.\n" +
                    "해결 방법: 키 이름을 확인하세요."
                );
            }
            return it->second;
        }
        throw error::TypeError(
            "'의'는 클래스 인스턴스나 사전의 멤버를 읽을 때 쓸 수 있습니다: " + object.toString() + "\n" +
            "해결 방법: 배열 메서드에는 을/를 을 쓰세요."
        );
    }

    // 3. 메서드 이름이 사용자 함수나 내장 함수이면 객체를 첫 인자로 넘겨 호출한다
    //    ("x를 f(a)" 는 "f(x, a)", "값을 출력" 은 "출력(값)")
    if (!name.empty() && (env->existsInChain(name) || Builtin::hasBuiltin(name)))
    {
        Value callee = env->existsInChain(name) ? env->get(name) : Builtin::getBuiltin(name);
        std::vector<Value> callArgs;
        callArgs.reserve(args.size() + 1);
        callArgs.push_back(object);
        callArgs.insert(callArgs.end(), args.begin(), args.end());
        return invokeFunction(callee, callArgs, name);
    }

    // 4. 내장 배열 메서드 (걸러낸다, 변환한다, 축약한다, 찾다, 정렬한다, 역순으로_나열한다)
    if (!object.isArray())
    {
        throw error::TypeError(
            std::string("조사 표현식은 현재 배열에만 사용할 수 있습니다.\n") +
            "해결 방법: 배열 값에 메서드를 적용하세요."
        );
    }

    const auto& arr = object.asArray();

    // 메서드가 호출형인 경우 (함수형 메서드)
    if (isCall)
    {
        if (name.empty())
        {
            throw error::TypeError(
                std::string("조사 표현식의 메서드 호출에서 함수 이름을 식별할 수 없습니다.\n") +
                "해결 방법: 메서드 이름을 확인하세요."
            );
        }

        // 콜백 하나를 받는 메서드의 인자 검사
        auto requireCallback = [&](const char* method, const char* example, size_t paramCount) {
            if (args.size() != 1)
            {
                throw error::ArgumentError(
                    std::string("'") + method + "' 메서드는 정확히 1개의 인자(" + example + ")가 필요합니다.\n" +
                    "해결 방법: " + example + "를 전달하세요."
                );
            }
            if (!args[0].isFunction())
            {
                throw error::TypeError(
                    std::string("'") + method + "' 메서드의 인자는 함수여야 합니다.\n" +
                    "해결 방법: 함수 리터럴을 전달하세요."
                );
            }
            if (args[0].asFunction()->parameters().size() != paramCount)
            {
                throw error::ArgumentError(
                    std::string("'") + method + "' 메서드의 " + example + "는 정확히 " + std::to_string(paramCount) +
                    "개의 매개변수가 필요합니다.\n" +
                    "해결 방법: 함수(x) { ... } 형태로 작성하세요."
                );
            }
        };

        // 걸러낸다 (filter)
        if (name == "걸러낸다" || name == "걸러내고")
        {
            requireCallback("걸러낸다", "조건 함수", 1);
            std::vector<Value> filtered;
            for (const auto& item : arr)
            {
                if (runCallback(args[0], {item}).isTruthy())
                {
                    filtered.push_back(item);
                }
            }
            return Value::createArray(filtered);
        }

        // 변환한다 (map)
        if (name == "변환한다" || name == "변환하고")
        {
            requireCallback("변환한다", "변환 함수", 1);
            std::vector<Value> mapped;
            for (const auto& item : arr)
            {
                mapped.push_back(runCallback(args[0], {item}));
            }
            return Value::createArray(mapped);
        }

        // 축약한다 (reduce)
        if (name == "축약한다" || name == "축약하고")
        {
            if (args.size() != 2)
            {
                throw error::ArgumentError(
                    std::string("'축약한다' 메서드는 정확히 2개의 인자(초기값, 축약 함수)가 필요합니다.\n") +
                    "해결 방법: 축약한다(0, 함수(누적, 현재) { 반환 누적 + 현재 })"
                );
            }
            if (!args[1].isFunction())
            {
                throw error::TypeError(
                    std::string("'축약한다' 메서드의 두 번째 인자는 함수여야 합니다.\n") +
                    "해결 방법: 함수 리터럴을 전달하세요."
                );
            }
            if (args[1].asFunction()->parameters().size() != 2)
            {
                throw error::ArgumentError(
                    std::string("'축약한다' 메서드의 축약 함수는 정확히 2개의 매개변수가 필요합니다.\n") +
                    "해결 방법: 함수(누적, 현재) { ... } 형태로 작성하세요."
                );
            }
            Value accumulator = args[0];
            for (const auto& item : arr)
            {
                accumulator = runCallback(args[1], {accumulator, item});
            }
            return accumulator;
        }

        // 찾다 (find)
        if (name == "찾다" || name == "찾고")
        {
            requireCallback("찾다", "조건 함수", 1);
            for (const auto& item : arr)
            {
                if (runCallback(args[0], {item}).isTruthy())
                {
                    return item;
                }
            }
            // 찾지 못한 경우 null 반환
            return Value::createNull();
        }

        throw error::NameError(
            "알 수 없는 배열 메서드 '" + name + "'입니다.\n" +
            "해결 방법: 지원되는 메서드는 '걸러낸다', '변환한다', '축약한다', '찾다' 등입니다."
        );
    }

    // 메서드가 식별자인 경우 (기본 메서드)
    if (name.empty())
    {
        throw error::TypeError(
            std::string("조사 표현식의 메서드는 식별자 또는 메서드 호출이어야 합니다.\n") +
            "해결 방법: 메서드 이름을 확인하세요."
        );
    }

    // 정렬 메서드
    if (name == "정렬한다" || name == "정렬하고")
    {
        std::vector<Value> sorted = arr;
        std::sort(sorted.begin(), sorted.end(), [](const Value& a, const Value& b) {
            return a.lessThan(b);
        });
        return Value::createArray(sorted);
    }

    // 역순 메서드
    if (name == "역순으로_나열한다" || name == "역순으로_나열하고")
    {
        std::vector<Value> reversed = arr;
        std::reverse(reversed.begin(), reversed.end());
        return Value::createArray(reversed);
    }

    // 메서드를 찾을 수 없음
    throw error::NameError(
        "배열에 대한 메서드 '" + name + "'를 찾을 수 없습니다.\n" +
        "해결 방법: 지원되는 메서드는 '정렬한다', '역순으로_나열한다', '걸러낸다', '변환한다' 등입니다."
    );
}

bool Evaluator::isLiteralNode(const ast::Expression* expr)
{
    if (!expr) return false;
    switch (expr->type())
    {
        case ast::NodeType::INTEGER_LITERAL:
        case ast::NodeType::FLOAT_LITERAL:
        case ast::NodeType::STRING_LITERAL:
        case ast::NodeType::BOOLEAN_LITERAL:
            return true;
        default:
            return false;
    }
}

Value Evaluator::literalValue(const ast::Expression* expr)
{
    if (expr)
    {
        switch (expr->type())
        {
            case ast::NodeType::INTEGER_LITERAL:
                return Value::createInteger(static_cast<const ast::IntegerLiteral*>(expr)->value());
            case ast::NodeType::FLOAT_LITERAL:
                return Value::createFloat(static_cast<const ast::FloatLiteral*>(expr)->value());
            case ast::NodeType::STRING_LITERAL:
                return Value::createString(static_cast<const ast::StringLiteral*>(expr)->value());
            case ast::NodeType::BOOLEAN_LITERAL:
                return Value::createBoolean(static_cast<const ast::BooleanLiteral*>(expr)->value());
            default:
                break;
        }
    }
    throw error::RuntimeError("패턴 리터럴이 아닙니다: " + (expr ? expr->toString() : std::string("(없음)")));
}

bool Evaluator::matchPattern(const ast::Pattern* pattern, const Value& value, Environment& bindings,
                             const PatternLiteral& literal)
{
    if (!pattern)
    {
        return false;
    }

    switch (pattern->type())
    {
        case ast::NodeType::LITERAL_PATTERN:
        {
            Value expected = literal(static_cast<const ast::LiteralPattern*>(pattern)->value());
            if (value.getType() != expected.getType())
            {
                return false;
            }
            switch (value.getType())
            {
                case types::TypeKind::INTEGER: return value.asInteger() == expected.asInteger();
                case types::TypeKind::FLOAT:   return value.asFloat() == expected.asFloat();
                case types::TypeKind::STRING:  return value.asString() == expected.asString();
                case types::TypeKind::BOOLEAN: return value.asBoolean() == expected.asBoolean();
                default:                       return false;
            }
        }

        case ast::NodeType::WILDCARD_PATTERN:
            return true;

        case ast::NodeType::BINDING_PATTERN:
            bindings.set(static_cast<const ast::BindingPattern*>(pattern)->name(), value);
            return true;

        case ast::NodeType::ARRAY_PATTERN:
        {
            if (!value.isArray())
            {
                return false;
            }
            auto arrayPattern = static_cast<const ast::ArrayPattern*>(pattern);
            const auto& elements = arrayPattern->elements();
            const auto& array = value.asArray();
            bool hasRest = !arrayPattern->rest().empty();

            if (hasRest ? array.size() < elements.size() : array.size() != elements.size())
            {
                return false;
            }
            for (size_t i = 0; i < elements.size(); i++)
            {
                if (!matchPattern(elements[i].get(), array[i], bindings, literal))
                {
                    return false;
                }
            }
            if (hasRest)
            {
                std::vector<Value> rest(array.begin() + static_cast<std::ptrdiff_t>(elements.size()), array.end());
                bindings.set(arrayPattern->rest(), Value::createArray(rest));
            }
            return true;
        }

        default:
            return false;
    }
}

Value Evaluator::evalMatchExpression(ast::MatchExpression* node)
{
    Value matchValue = eval(const_cast<ast::Expression*>(node->value()));
    PatternLiteral literal = [this](const ast::Expression* expr) {
        return eval(const_cast<ast::Expression*>(expr));
    };

    for (const auto& matchCase : node->cases())
    {
        auto caseEnv = std::make_shared<Environment>(env_);
        bool matched = matchPattern(matchCase.pattern(), matchValue, *caseEnv, literal);

        if (matched)
        {
            if (matchCase.guard())
            {
                auto prevEnv = env_;
                env_ = caseEnv;
                Value guardResult = eval(const_cast<ast::Expression*>(matchCase.guard()));
                env_ = prevEnv;

                if (guardResult.getType() != types::TypeKind::BOOLEAN || !guardResult.asBoolean())
                {
                    continue;
                }
            }

            auto prevEnv = env_;
            env_ = caseEnv;
            Value result;
            try
            {
                result = eval(const_cast<ast::Expression*>(matchCase.body()));
            }
            catch (...)
            {
                env_ = prevEnv;
                leaveScope(caseEnv);
                throw;
            }
            env_ = prevEnv;
            leaveScope(caseEnv);
            return result;
        }
    }

    throw error::RuntimeError("매칭되는 패턴이 없습니다");
}

// ============================================================================
// Phase 7.1: 클래스 시스템
// ============================================================================

Value Evaluator::evalMemberAccessExpression(ast::MemberAccessExpression* expr)
{
    Value obj = eval(const_cast<ast::Expression*>(expr->object()));
    return memberValue(obj, expr->memberName());
}

Value Evaluator::memberValue(const Value& object, const std::string& name)
{
    if (!object.isClassInstance())
    {
        throw error::RuntimeError("필드 접근은 클래스 인스턴스에만 가능합니다");
    }
    return object.asClassInstance()->getField(name);
}

void Evaluator::assignMember(const Value& object, const std::string& name, const Value& value)
{
    if (!object.isClassInstance())
    {
        throw error::RuntimeError("필드 접근은 클래스 인스턴스에만 가능합니다");
    }
    object.asClassInstance()->setField(name, value);
}

Value Evaluator::evalThisExpression(ast::ThisExpression* /* expr */)
{
    // 환경에서 "자신" 변수 조회
    return env_->get("자신");
}

Value Evaluator::evalClassStatement(ast::ClassStatement* stmt)
{
    env_->set(stmt->className(), makeClass(stmt, env_));
    return Value::createNull();
}

Value Evaluator::makeClass(const ast::ClassStatement* stmt, const std::shared_ptr<Environment>& env)
{
    // 필드 이름 추출
    std::vector<std::string> fieldNames;
    for (const auto& field : stmt->fields())
    {
        fieldNames.push_back(field->fieldName());
    }

    // 메서드 추출 (클로저는 클래스를 정의한 환경)
    std::unordered_map<std::string, std::shared_ptr<Function>> methods;
    for (const auto& method : stmt->methods())
    {
        std::vector<std::string> params;
        for (const auto& param : method->parameters())
        {
            params.push_back(param.name);
        }

        auto methodFunc = std::make_shared<Function>(
            params,
            const_cast<ast::BlockStatement*>(method->body()),
            env
        );

        methods[method->methodName()] = methodFunc;
    }

    // 생성자 추출
    std::shared_ptr<Function> constructor = nullptr;
    if (stmt->constructor())
    {
        std::vector<std::string> params;
        for (const auto& param : stmt->constructor()->parameters())
        {
            params.push_back(param.name);
        }

        constructor = std::make_shared<Function>(
            params,
            const_cast<ast::BlockStatement*>(stmt->constructor()->body()),
            env
        );
    }

    // ClassDefinition 생성
    auto classDef = std::make_shared<ClassDefinition>(
        stmt->className(),
        fieldNames,
        methods,
        constructor,
        stmt->superClass()
    );

    // 클래스 이름은 생성자 함수처럼 등록된다: 호출하면 인스턴스가 만들어진다
    auto constructorFunc = std::make_shared<Function>(
        std::vector<std::string>{},  // 매개변수 없음 (향후 확장)
        nullptr,  // body는 null
        env,
        true   // isBuiltin = true (특수 처리)
    );
    constructorFunc->setClassDef(classDef);
    return Value::createFunction(constructorFunc);
}

// ============================================================================
// 안전 장치
// ============================================================================

void Evaluator::checkSafetyLimits()
{
    // 평가 횟수 체크
    if (++evaluationCount_ > maxEvaluations_)
    {
        throw error::RuntimeError(
            "최대 평가 횟수 초과 (" + std::to_string(maxEvaluations_) + " 초과). 무한 루프 의심."
        );
    }

    // 실행 시간 체크 (매 1000번마다 - 성능 최적화)
    if (evaluationCount_ % 1000 == 0)
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_);
        if (elapsed > maxExecutionTime_)
        {
            throw error::RuntimeError(
                "최대 실행 시간 초과 (" + std::to_string(maxExecutionTime_.count()) + "ms 초과). 무한 루프 또는 긴 연산 의심."
            );
        }
    }
}

} // namespace evaluator
} // namespace kingsejong
