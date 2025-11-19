/**
 * @file Evaluator.cpp
 * @brief KingSejong 언어의 AST 실행 엔진 구현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Evaluator.h"
#include "Builtin.h"
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
{
}

Evaluator::Evaluator(std::shared_ptr<Environment> environment)
    : env_(environment)
{
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
            Value exprValue = eval(const_cast<ast::Expression*>(expressions[i].get()));

            // Value를 문자열로 변환
            if (exprValue.isString())
            {
                result += exprValue.asString();
            }
            else if (exprValue.isInteger())
            {
                result += std::to_string(exprValue.asInteger());
            }
            else if (exprValue.isFloat())
            {
                result += std::to_string(exprValue.asFloat());
            }
            else if (exprValue.isBoolean())
            {
                result += exprValue.asBoolean() ? "참" : "거짓";
            }
            else
            {
                // 기타 타입은 기본 toString 사용
                result += exprValue.toString();
            }
        }
    }

    return Value::createString(result);
}

Value Evaluator::evalBooleanLiteral(ast::BooleanLiteral* lit)
{
    return Value::createBoolean(lit->value());
}

Value Evaluator::evalIdentifier(ast::Identifier* ident)
{
    const std::string& name = ident->name();

    // 1. 먼저 환경에서 변수 조회
    if (env_->existsInChain(name))
    {
        return env_->get(name);
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
        if (auto* memberAccess = dynamic_cast<ast::MemberAccessExpression*>(const_cast<ast::Expression*>(expr->left())))
        {
            // 객체 평가
            Value obj = eval(const_cast<ast::Expression*>(memberAccess->object()));

            if (!obj.isClassInstance())
            {
                throw error::RuntimeError("필드 접근은 클래스 인스턴스에만 가능합니다");
            }

            // 필드 설정
            obj.asClassInstance()->setField(memberAccess->memberName(), value);
            return value;
        }
        // 좌변이 Identifier이면 변수 할당
        else if (auto* ident = dynamic_cast<ast::Identifier*>(const_cast<ast::Expression*>(expr->left())))
        {
            env_->set(ident->name(), value);
            return value;
        }
        else
        {
            throw error::RuntimeError("Invalid left-hand side of assignment");
        }
    }

    Value left = eval(const_cast<ast::Expression*>(expr->left()));
    Value right = eval(const_cast<ast::Expression*>(expr->right()));

    // 비교 연산자
    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
    {
        return applyComparisonOperation(left, op, right);
    }

    // 논리 연산자
    if (op == "&&" || op == "||")
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

    // 문자열 연결
    if (op == "+" && left.isString() && right.isString())
    {
        return Value::createString(left.asString() + right.asString());
    }

    throw error::RuntimeError("지원되지 않는 연산: " + left.toString() + " " + op + " " + right.toString());
}

Value Evaluator::evalUnaryExpression(ast::UnaryExpression* expr)
{
    Value operand = eval(const_cast<ast::Expression*>(expr->operand()));
    const std::string& op = expr->op();

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
 * @brief await 표현식 평가 (Phase 7.3)
 *
 * await 표현식을 평가합니다.
 * 현재 구현에서는 표현식을 즉시 평가하여 반환합니다.
 * 완전한 비동기 대기 처리는 향후 구현됩니다.
 *
 * TODO: Promise 대기 및 Event Loop 통합
 */
Value Evaluator::evalAwaitExpression(ast::AwaitExpression* expr)
{
    // 대기할 표현식 평가
    Value value = eval(const_cast<ast::Expression*>(expr->argument()));

    // 에러 체크
    if (value.isError())
    {
        return value;
    }

    // Promise인 경우 값 추출
    if (value.isPromise())
    {
        auto promise = value.asPromise();

        // Promise가 이미 완료된 경우 값 반환
        if (promise->isSettled())
        {
            return promise->value();
        }
        else
        {
            // TODO: Event Loop에서 Promise 완료 대기
            // 현재는 pending Promise를 그대로 반환
            return value;
        }
    }

    // Promise가 아닌 경우 그대로 반환
    return value;
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
    // Phase 7.1.3: 메서드 호출 처리
    // 함수 표현식이 MemberAccessExpression이면 메서드 호출
    if (auto* memberAccess = dynamic_cast<ast::MemberAccessExpression*>(const_cast<ast::Expression*>(expr->function())))
    {
        // 객체 평가
        Value obj = eval(const_cast<ast::Expression*>(memberAccess->object()));

        // Phase 7.3: Promise 메서드 처리 (.then, .catch)
        if (obj.isPromise())
        {
            auto promise = obj.asPromise();
            const std::string& methodName = memberAccess->memberName();

            // 인자 평가 (콜백 함수)
            if (expr->arguments().size() != 1)
            {
                throw error::ArgumentError(
                    "Promise." + methodName + "()은 콜백 함수 1개를 인자로 받습니다"
                );
            }

            Value callbackValue = eval(const_cast<ast::Expression*>(expr->arguments()[0].get()));
            if (!callbackValue.isFunction())
            {
                throw error::TypeError(
                    "Promise." + methodName + "()의 인자는 함수여야 합니다"
                );
            }

            auto callbackFunc = callbackValue.asFunction();

            // Continuation 생성
            auto continuation = [this, callbackFunc](const Value& value) -> Value {
                // 콜백 함수 실행
                auto funcEnv = std::make_shared<Environment>(callbackFunc->closure());

                // 매개변수 바인딩 (value를 첫 번째 매개변수에)
                if (!callbackFunc->parameters().empty())
                {
                    funcEnv->set(callbackFunc->parameters()[0], value);
                }

                // 기존 환경 저장
                auto previousEnv = env_;
                env_ = funcEnv;

                // 콜백 실행
                Value result = Value::createNull();
                try
                {
                    result = eval(callbackFunc->body());
                }
                catch (const ReturnValue& returnVal)
                {
                    result = returnVal.getValue();
                }

                // 환경 복원
                env_ = previousEnv;

                return result;
            };

            // 메서드에 따라 콜백 등록
            if (methodName == "then" || methodName == "그러면")
            {
                // 이미 이행된 경우 즉시 실행
                if (promise->state() == Promise::State::FULFILLED)
                {
                    Value result = continuation(promise->value());
                    // 결과를 새 Promise로 감싸서 반환 (체이닝 지원)
                    auto newPromise = std::make_shared<Promise>();
                    newPromise->resolve(result);
                    return Value::createPromise(newPromise);
                }
                else if (promise->state() == Promise::State::PENDING)
                {
                    promise->then(continuation);
                }
                // rejected인 경우 아무것도 하지 않음
            }
            else if (methodName == "catch" || methodName == "오류시")
            {
                // 이미 거부된 경우 즉시 실행
                if (promise->state() == Promise::State::REJECTED)
                {
                    Value result = continuation(promise->value());
                    // 결과를 새 Promise로 감싸서 반환
                    auto newPromise = std::make_shared<Promise>();
                    newPromise->resolve(result);
                    return Value::createPromise(newPromise);
                }
                else if (promise->state() == Promise::State::PENDING)
                {
                    promise->catchError(continuation);
                }
                // fulfilled인 경우 아무것도 하지 않음
            }
            else
            {
                throw error::RuntimeError("Promise에 '" + methodName + "' 메서드가 없습니다");
            }

            // 체이닝을 위해 원래 Promise 반환
            return obj;
        }

        // 클래스 인스턴스 메서드 호출
        if (!obj.isClassInstance())
        {
            throw error::RuntimeError("메서드 호출은 클래스 인스턴스 또는 Promise에만 가능합니다");
        }

        auto instance = obj.asClassInstance();
        auto classDef = instance->classDef();

        // 메서드 찾기
        auto method = classDef->getMethod(memberAccess->memberName());
        if (!method)
        {
            throw error::RuntimeError("메서드를 찾을 수 없습니다: " + memberAccess->memberName());
        }

        // 인자 평가
        std::vector<Value> args;
        for (const auto& argExpr : expr->arguments())
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
        methodEnv->set("자신", obj);

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

        return result;
    }

    // 1. 함수 평가
    Value funcValue = eval(const_cast<ast::Expression*>(expr->function()));

    // 2. 인자 평가
    std::vector<Value> args;
    for (const auto& argExpr : expr->arguments())
    {
        Value argValue = eval(const_cast<ast::Expression*>(argExpr.get()));
        args.push_back(argValue);
    }

    // 3. 내장 함수 호출
    if (funcValue.isBuiltinFunction())
    {
        auto fn = funcValue.asBuiltinFunction();
        return fn(args);
    }

    // 4. 사용자 정의 함수 호출
    if (!funcValue.isFunction())
    {
        throw error::TypeError(
            "함수가 아닌 값을 호출하려고 합니다: " + funcValue.toString() + "\n" +
            "해결 방법: 함수 이름이 올바른지 확인하고, 함수가 정의되어 있는지 확인하세요."
        );
    }

    auto func = funcValue.asFunction();

    // Phase 7.1: 클래스 생성자 호출
    if (func->classDef())
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

            // 기존 환경 저장
            auto previousEnv = env_;

            // 생성자 환경으로 전환
            env_ = ctorEnv;

            // 생성자 본문 실행
            try
            {
                eval(ctor->body());
            }
            catch (const ReturnValue&)
            {
                // 생성자에서 return은 무시
            }

            // 환경 복원
            env_ = previousEnv;
        }

        return Value::createClassInstance(instance);
    }

    // 5. 매개변수 개수 확인
    if (args.size() != func->parameters().size())
    {
        throw error::ArgumentError(
            "함수 인자 개수가 일치하지 않습니다: 필요 " + std::to_string(func->parameters().size()) + "개, " +
            "전달 " + std::to_string(args.size()) + "개\n" +
            "해결 방법: 함수 정의를 확인하고 올바른 개수의 인자를 전달하세요."
        );
    }

    // Hot Path Tracking: 함수 호출 추적
    std::string functionName = "anonymous";
    size_t functionId = reinterpret_cast<size_t>(func.get());

    // 함수 이름 추출 시도 (Identifier인 경우)
    if (auto* identExpr = dynamic_cast<ast::Identifier*>(const_cast<ast::Expression*>(expr->function())))
    {
        functionName = identExpr->name();
    }

    auto startTime = std::chrono::steady_clock::now();

    // 6. 새로운 환경 생성 (클로저 기반)
    auto funcEnv = std::make_shared<Environment>(func->closure());

    // 7. 매개변수 바인딩
    for (size_t i = 0; i < func->parameters().size(); ++i)
    {
        funcEnv->set(func->parameters()[i], args[i]);
    }

    // 8. 함수 본문 실행 (새로운 환경에서)
    // 기존 환경 저장
    auto previousEnv = env_;

    // 함수 환경으로 전환
    env_ = funcEnv;

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

    // 환경 복원
    env_ = previousEnv;

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

// ============================================================================
// Statement 실행 함수들
// ============================================================================

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
    if (op == "&&")
    {
        // 단축 평가: left가 거짓이면 right를 평가하지 않음
        if (!left.isTruthy()) return Value::createBoolean(false);
        return Value::createBoolean(right.isTruthy());
    }

    if (op == "||")
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

    // 딕셔너리 인덱스 접근 (Phase 7.2)
    if (left.isDictionary())
    {
        Value index = eval(const_cast<ast::Expression*>(expr->index()));

        // 딕셔너리 키는 문자열이어야 함
        if (!index.isString())
        {
            throw error::TypeError("딕셔너리 키는 문자열이어야 합니다. 실제 타입: " + types::Type::typeKindToString(index.getType()));
        }

        auto& dict = left.asDictionary();
        std::string key = index.asString();

        auto it = dict.find(key);
        if (it == dict.end())
        {
            throw error::RuntimeError("딕셔너리에 키가 존재하지 않습니다: \"" + key + "\"");
        }

        return it->second;
    }

    // 배열인지 확인
    if (!left.isArray())
    {
        throw error::TypeError(
            "배열 또는 딕셔너리가 아닌 값에 인덱스 접근을 시도했습니다: " + left.toString() + "\n" +
            "해결 방법: 인덱스 접근([])은 배열 또는 딕셔너리 타입에만 사용할 수 있습니다."
        );
    }

    std::vector<Value>& arr = left.asArray();

    // 인덱스 표현식이 RangeExpression인지 확인
    const ast::Expression* indexExpr = expr->index();
    if (indexExpr->type() == ast::NodeType::RANGE_EXPRESSION)
    {
        // 배열 슬라이싱 처리
        const ast::RangeExpression* rangeExpr = static_cast<const ast::RangeExpression*>(indexExpr);

        // 시작 인덱스 평가
        Value startValue = eval(const_cast<ast::Expression*>(rangeExpr->start()));
        if (!startValue.isInteger())
        {
            throw std::runtime_error("슬라이싱 시작 인덱스는 정수여야 합니다");
        }

        // 끝 인덱스 평가
        Value endValue = eval(const_cast<ast::Expression*>(rangeExpr->end()));
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
        if (!rangeExpr->startInclusive())
        {
            startIdx++; // "초과"는 다음 인덱스부터
        }
        if (!rangeExpr->endInclusive())
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
        if (rangeExpr->hasStep())
        {
            Value stepVal = eval(const_cast<ast::Expression*>(rangeExpr->step()));
            if (!stepVal.isInteger())
            {
                throw std::runtime_error("슬라이싱 step 값은 정수여야 합니다");
            }
            stepValue = stepVal.asInteger();
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
    else
    {
        // 일반 인덱스 접근 (정수)
        Value index = eval(const_cast<ast::Expression*>(expr->index()));

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
    std::string modulePath = stmt->modulePath();
    auto moduleEnv = moduleLoader_->loadModule(modulePath);

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
    // 던질 값 평가
    Value value = eval(const_cast<ast::Expression*>(stmt->value()));

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
    bool errorCaught = false;

    // 1. Try 블록 실행
    try
    {
        result = eval(const_cast<ast::BlockStatement*>(stmt->tryBlock()));

        // 에러 Value가 반환된 경우 catch 블록으로 전달
        if (result.isError())
        {
            caughtError = result;
            errorCaught = false;  // catch 블록에서 처리 필요
        }
        else
        {
            errorCaught = true;  // 에러 없음
        }
    }
    catch (const ReturnValue& ret)
    {
        // return 문은 try-catch를 통과시킴
        // finally 블록 실행 후 다시 던짐
        if (stmt->finallyBlock())
        {
            eval(const_cast<ast::BlockStatement*>(stmt->finallyBlock()));
        }
        throw;  // return 예외 재전파
    }
    catch (...)
    {
        // C++ 예외는 런타임 에러로 변환
        caughtError = Value::createError("예기치 않은 내부 에러 발생", "RuntimeError");
        errorCaught = false;
    }

    // 2. Catch 블록 실행 (에러가 있고 catch절이 있는 경우)
    if (!errorCaught && !stmt->catchClauses().empty())
    {
        // 첫 번째 catch 절 실행 (현재는 단일 catch만 지원)
        const auto& catchClause = stmt->catchClauses()[0];

        // 새로운 스코프 생성 (에러 변수 바인딩용)
        auto catchEnv = std::make_shared<Environment>(env_);
        auto prevEnv = env_;
        env_ = catchEnv;

        // 에러 변수 바인딩
        // 에러 Value를 문자열로 변환하여 바인딩
        env_->set(catchClause->errorVarName(),
                 Value::createString(caughtError.toString()));

        // Catch 블록 실행
        result = eval(const_cast<ast::BlockStatement*>(catchClause->body()));

        // 환경 복원
        env_ = prevEnv;

        errorCaught = true;  // 에러가 처리됨
    }

    // 3. Finally 블록 실행 (항상)
    if (stmt->finallyBlock())
    {
        eval(const_cast<ast::BlockStatement*>(stmt->finallyBlock()));
    }

    // 4. 에러 전파
    if (!errorCaught)
    {
        // catch되지 않은 에러는 호출자에게 전파
        return caughtError;
    }

    return result;
}

Value Evaluator::evalJosaExpression(ast::JosaExpression* expr)
{
    // 객체 평가
    Value object = eval(const_cast<ast::Expression*>(expr->object()));

    // 배열이 아닌 타입에 대한 조사 표현식
    if (!object.isArray())
    {
        throw error::TypeError(
            std::string("조사 표현식은 현재 배열에만 사용할 수 있습니다.\n") +
            "해결 방법: 배열 값에 메서드를 적용하세요."
        );
    }

    auto arr = object.asArray();

    // 메서드가 CallExpression인 경우 (함수형 메서드)
    const ast::CallExpression* callExpr = dynamic_cast<const ast::CallExpression*>(expr->method());
    if (callExpr)
    {
        // 함수 이름 획득
        const ast::Identifier* funcIdent = dynamic_cast<const ast::Identifier*>(callExpr->function());
        if (!funcIdent)
        {
            throw error::TypeError(
                std::string("조사 표현식의 메서드 호출에서 함수 이름을 식별할 수 없습니다.\n") +
                "해결 방법: 메서드 이름을 확인하세요."
            );
        }

        std::string methodName = funcIdent->name();

        // 걸러낸다 (filter)
        if (methodName == "걸러낸다" || methodName == "걸러내고")
        {
            if (callExpr->arguments().size() != 1)
            {
                throw error::ArgumentError(
                    std::string("'걸러낸다' 메서드는 정확히 1개의 인자(조건 함수)가 필요합니다.\n") +
                    "해결 방법: 조건 함수를 전달하세요. 예: 걸러낸다(함수(x) { 반환 x > 0 })"
                );
            }

            Value filterFunc = eval(const_cast<ast::Expression*>(callExpr->arguments()[0].get()));
            if (!filterFunc.isFunction())
            {
                throw error::TypeError(
                    std::string("'걸러낸다' 메서드의 인자는 함수여야 합니다.\n") +
                    "해결 방법: 함수 리터럴을 전달하세요."
                );
            }

            std::vector<Value> filtered;
            for (const auto& item : arr)
            {
                // 조건 함수 호출
                auto funcEnv = filterFunc.asFunction()->closure()->createEnclosed();
                const auto& params = filterFunc.asFunction()->parameters();
                if (params.size() != 1)
                {
                    throw error::ArgumentError(
                        std::string("'걸러낸다' 메서드의 조건 함수는 정확히 1개의 매개변수가 필요합니다.\n") +
                        "해결 방법: 함수(x) { ... } 형태로 작성하세요."
                    );
                }
                funcEnv->set(params[0], item);

                Evaluator funcEvaluator(funcEnv);
                Value result;
                try
                {
                    result = funcEvaluator.eval(filterFunc.asFunction()->body());
                }
                catch (const ReturnValue& retVal)
                {
                    result = retVal.getValue();
                }

                if (result.isTruthy())
                {
                    filtered.push_back(item);
                }
            }
            return Value::createArray(filtered);
        }

        // 변환한다 (map)
        if (methodName == "변환한다" || methodName == "변환하고")
        {
            if (callExpr->arguments().size() != 1)
            {
                throw error::ArgumentError(
                    std::string("'변환한다' 메서드는 정확히 1개의 인자(변환 함수)가 필요합니다.\n") +
                    "해결 방법: 변환 함수를 전달하세요. 예: 변환한다(함수(x) { 반환 x * 2 })"
                );
            }

            Value mapFunc = eval(const_cast<ast::Expression*>(callExpr->arguments()[0].get()));
            if (!mapFunc.isFunction())
            {
                throw error::TypeError(
                    std::string("'변환한다' 메서드의 인자는 함수여야 합니다.\n") +
                    "해결 방법: 함수 리터럴을 전달하세요."
                );
            }

            std::vector<Value> mapped;
            for (const auto& item : arr)
            {
                // 변환 함수 호출
                auto funcEnv = mapFunc.asFunction()->closure()->createEnclosed();
                const auto& params = mapFunc.asFunction()->parameters();
                if (params.size() != 1)
                {
                    throw error::ArgumentError(
                        std::string("'변환한다' 메서드의 변환 함수는 정확히 1개의 매개변수가 필요합니다.\n") +
                        "해결 방법: 함수(x) { ... } 형태로 작성하세요."
                    );
                }
                funcEnv->set(params[0], item);

                Evaluator funcEvaluator(funcEnv);
                Value result;
                try
                {
                    result = funcEvaluator.eval(mapFunc.asFunction()->body());
                }
                catch (const ReturnValue& retVal)
                {
                    result = retVal.getValue();
                }

                mapped.push_back(result);
            }
            return Value::createArray(mapped);
        }

        // 축약한다 (reduce)
        if (methodName == "축약한다" || methodName == "축약하고")
        {
            if (callExpr->arguments().size() != 2)
            {
                throw error::ArgumentError(
                    std::string("'축약한다' 메서드는 정확히 2개의 인자(초기값, 축약 함수)가 필요합니다.\n") +
                    "해결 방법: 축약한다(0, 함수(누적, 현재) { 반환 누적 + 현재 })"
                );
            }

            Value initialValue = eval(const_cast<ast::Expression*>(callExpr->arguments()[0].get()));
            Value reduceFunc = eval(const_cast<ast::Expression*>(callExpr->arguments()[1].get()));

            if (!reduceFunc.isFunction())
            {
                throw error::TypeError(
                    std::string("'축약한다' 메서드의 두 번째 인자는 함수여야 합니다.\n") +
                    "해결 방법: 함수 리터럴을 전달하세요."
                );
            }

            const auto& params = reduceFunc.asFunction()->parameters();
            if (params.size() != 2)
            {
                throw error::ArgumentError(
                    std::string("'축약한다' 메서드의 축약 함수는 정확히 2개의 매개변수가 필요합니다.\n") +
                    "해결 방법: 함수(누적, 현재) { ... } 형태로 작성하세요."
                );
            }

            Value accumulator = initialValue;
            for (const auto& item : arr)
            {
                auto funcEnv = reduceFunc.asFunction()->closure()->createEnclosed();
                funcEnv->set(params[0], accumulator);
                funcEnv->set(params[1], item);

                Evaluator funcEvaluator(funcEnv);
                try
                {
                    accumulator = funcEvaluator.eval(reduceFunc.asFunction()->body());
                }
                catch (const ReturnValue& retVal)
                {
                    accumulator = retVal.getValue();
                }
            }
            return accumulator;
        }

        // 찾다 (find)
        if (methodName == "찾다" || methodName == "찾고")
        {
            if (callExpr->arguments().size() != 1)
            {
                throw error::ArgumentError(
                    std::string("'찾다' 메서드는 정확히 1개의 인자(조건 함수)가 필요합니다.\n") +
                    "해결 방법: 조건 함수를 전달하세요. 예: 찾다(함수(x) { 반환 x > 10 })"
                );
            }

            Value findFunc = eval(const_cast<ast::Expression*>(callExpr->arguments()[0].get()));
            if (!findFunc.isFunction())
            {
                throw error::TypeError(
                    std::string("'찾다' 메서드의 인자는 함수여야 합니다.\n") +
                    "해결 방법: 함수 리터럴을 전달하세요."
                );
            }

            for (const auto& item : arr)
            {
                auto funcEnv = findFunc.asFunction()->closure()->createEnclosed();
                const auto& params = findFunc.asFunction()->parameters();
                if (params.size() != 1)
                {
                    throw error::ArgumentError(
                        std::string("'찾다' 메서드의 조건 함수는 정확히 1개의 매개변수가 필요합니다.\n") +
                        "해결 방법: 함수(x) { ... } 형태로 작성하세요."
                    );
                }
                funcEnv->set(params[0], item);

                Evaluator funcEvaluator(funcEnv);
                Value result;
                try
                {
                    result = funcEvaluator.eval(findFunc.asFunction()->body());
                }
                catch (const ReturnValue& retVal)
                {
                    result = retVal.getValue();
                }

                if (result.isTruthy())
                {
                    return item;
                }
            }
            // 찾지 못한 경우 null 반환
            return Value::createNull();
        }

        throw error::NameError(
            "알 수 없는 배열 메서드 '" + methodName + "'입니다.\n" +
            "해결 방법: 지원되는 메서드는 '걸러낸다', '변환한다', '축약한다', '찾다' 등입니다."
        );
    }

    // 메서드가 Identifier인 경우 (기본 메서드)
    const ast::Identifier* methodIdent = dynamic_cast<const ast::Identifier*>(expr->method());
    if (!methodIdent)
    {
        throw error::TypeError(
            std::string("조사 표현식의 메서드는 식별자 또는 메서드 호출이어야 합니다.\n") +
            "해결 방법: 메서드 이름을 확인하세요."
        );
    }

    std::string methodName = methodIdent->name();

    // 정렬 메서드
    if (methodName == "정렬한다" || methodName == "정렬하고")
    {
        std::vector<Value> sorted = arr;
        std::sort(sorted.begin(), sorted.end(), [](const Value& a, const Value& b) {
            return a.lessThan(b);
        });
        return Value::createArray(sorted);
    }

    // 역순 메서드
    if (methodName == "역순으로_나열한다" || methodName == "역순으로_나열하고")
    {
        std::vector<Value> reversed = arr;
        std::reverse(reversed.begin(), reversed.end());
        return Value::createArray(reversed);
    }

    // 메서드를 찾을 수 없음
    throw error::NameError(
        "배열에 대한 메서드 '" + methodName + "'를 찾을 수 없습니다.\n" +
        "해결 방법: 지원되는 메서드는 '정렬한다', '역순으로_나열한다', '걸러낸다', '변환한다' 등입니다."
    );
}

Value Evaluator::evalMatchExpression(ast::MatchExpression* node)
{
    Value matchValue = eval(const_cast<ast::Expression*>(node->value()));

    for (const auto& matchCase : node->cases())
    {
        bool matched = false;
        auto caseEnv = std::make_shared<Environment>(env_);

        auto pattern = matchCase.pattern();
        if (!pattern)
        {
            continue;
        }

        switch (pattern->type())
        {
            case ast::NodeType::LITERAL_PATTERN:
            {
                auto literalPattern = static_cast<const ast::LiteralPattern*>(pattern);
                Value patternValue = eval(const_cast<ast::Expression*>(literalPattern->value()));

                if (matchValue.getType() != patternValue.getType())
                {
                    break;
                }

                if (matchValue.getType() == types::TypeKind::INTEGER &&
                    matchValue.asInteger() == patternValue.asInteger())
                {
                    matched = true;
                }
                else if (matchValue.getType() == types::TypeKind::STRING &&
                         matchValue.asString() == patternValue.asString())
                {
                    matched = true;
                }
                else if (matchValue.getType() == types::TypeKind::BOOLEAN &&
                         matchValue.asBoolean() == patternValue.asBoolean())
                {
                    matched = true;
                }
                break;
            }

            case ast::NodeType::WILDCARD_PATTERN:
            {
                matched = true;
                break;
            }

            case ast::NodeType::BINDING_PATTERN:
            {
                auto bindingPattern = static_cast<const ast::BindingPattern*>(pattern);
                std::string name = bindingPattern->name();
                caseEnv->set(name, matchValue);
                matched = true;
                break;
            }

            case ast::NodeType::ARRAY_PATTERN:
            {
                if (matchValue.getType() != types::TypeKind::ARRAY)
                {
                    break;
                }

                auto arrayPattern = static_cast<const ast::ArrayPattern*>(pattern);
                const auto& elements = arrayPattern->elements();
                const auto& matchArray = matchValue.asArray();

                if (elements.size() != matchArray.size())
                {
                    break;
                }

                matched = true;
                for (size_t i = 0; i < elements.size(); i++)
                {
                    auto elemPattern = elements[i].get();
                    Value elemValue = matchArray[i];

                    if (elemPattern->type() == ast::NodeType::LITERAL_PATTERN)
                    {
                        auto literalPattern = static_cast<const ast::LiteralPattern*>(elemPattern);
                        Value patternValue = eval(const_cast<ast::Expression*>(literalPattern->value()));

                        if (elemValue.getType() != patternValue.getType())
                        {
                            matched = false;
                            break;
                        }

                        if (elemValue.getType() == types::TypeKind::INTEGER &&
                            elemValue.asInteger() != patternValue.asInteger())
                        {
                            matched = false;
                            break;
                        }
                        else if (elemValue.getType() == types::TypeKind::STRING &&
                                 elemValue.asString() != patternValue.asString())
                        {
                            matched = false;
                            break;
                        }
                        else if (elemValue.getType() == types::TypeKind::BOOLEAN &&
                                 elemValue.asBoolean() != patternValue.asBoolean())
                        {
                            matched = false;
                            break;
                        }
                    }
                    else if (elemPattern->type() == ast::NodeType::WILDCARD_PATTERN)
                    {
                        continue;
                    }
                    else if (elemPattern->type() == ast::NodeType::BINDING_PATTERN)
                    {
                        auto bindingPattern = static_cast<const ast::BindingPattern*>(elemPattern);
                        caseEnv->set(bindingPattern->name(), elemValue);
                    }
                    else
                    {
                        matched = false;
                        break;
                    }
                }
                break;
            }

            default:
                break;
        }

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
            Value result = eval(const_cast<ast::Expression*>(matchCase.body()));
            env_ = prevEnv;
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
    // 객체 평가
    Value obj = eval(const_cast<ast::Expression*>(expr->object()));

    if (!obj.isClassInstance())
    {
        throw error::RuntimeError("필드 접근은 클래스 인스턴스에만 가능합니다");
    }

    // 필드 값 반환
    return obj.asClassInstance()->getField(expr->memberName());
}

Value Evaluator::evalThisExpression(ast::ThisExpression* /* expr */)
{
    // 환경에서 "자신" 변수 조회
    return env_->get("자신");
}

Value Evaluator::evalClassStatement(ast::ClassStatement* stmt)
{
    // 필드 이름 추출
    std::vector<std::string> fieldNames;
    for (const auto& field : stmt->fields())
    {
        fieldNames.push_back(field->fieldName());
    }

    // 메서드 추출
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
            env_
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
            env_
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

    // 환경에 클래스 등록 (클래스 이름을 함수처럼 등록)
    // 클래스 생성자 함수 생성
    auto constructorFunc = std::make_shared<Function>(
        std::vector<std::string>{},  // 매개변수 없음 (향후 확장)
        nullptr,  // body는 null
        env_,
        true   // isBuiltin = true (특수 처리)
    );
    constructorFunc->setClassDef(classDef);

    env_->set(stmt->className(), Value::createFunction(constructorFunc));

    return Value::createNull();
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
