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
#include <sstream>
#include <cmath>
#include <algorithm>

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

        case ast::NodeType::BOOLEAN_LITERAL:
            return evalBooleanLiteral(static_cast<ast::BooleanLiteral*>(node));

        case ast::NodeType::BINARY_EXPRESSION:
            return evalBinaryExpression(static_cast<ast::BinaryExpression*>(node));

        case ast::NodeType::UNARY_EXPRESSION:
            return evalUnaryExpression(static_cast<ast::UnaryExpression*>(node));

        case ast::NodeType::FUNCTION_LITERAL:
            return evalFunctionLiteral(static_cast<ast::FunctionLiteral*>(node));

        case ast::NodeType::CALL_EXPRESSION:
            return evalCallExpression(static_cast<ast::CallExpression*>(node));

        case ast::NodeType::ARRAY_LITERAL:
            return evalArrayLiteral(static_cast<ast::ArrayLiteral*>(node));

        case ast::NodeType::INDEX_EXPRESSION:
            return evalIndexExpression(static_cast<ast::IndexExpression*>(node));

        case ast::NodeType::JOSA_EXPRESSION:
            return evalJosaExpression(static_cast<ast::JosaExpression*>(node));

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

        default:
        {
            std::string msg = "평가되지 않은 노드 타입: " + ast::nodeTypeToString(node->type());
            throw std::runtime_error(msg);
        }
    }
}

Value Evaluator::evalProgram(ast::Program* program)
{
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
    Value left = eval(const_cast<ast::Expression*>(expr->left()));
    Value right = eval(const_cast<ast::Expression*>(expr->right()));
    const std::string& op = expr->op();

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

    throw std::runtime_error("지원되지 않는 연산: " + left.toString() + " " + op + " " + right.toString());
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
        throw std::runtime_error("음수 연산은 숫자에만 적용 가능합니다");
    }

    if (op == "!")
    {
        return Value::createBoolean(!operand.isTruthy());
    }

    throw std::runtime_error("지원되지 않는 단항 연산자: " + op);
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

    // 5. 매개변수 개수 확인
    if (args.size() != func->parameters().size())
    {
        throw error::ArgumentError(
            "함수 인자 개수가 일치하지 않습니다: 필요 " + std::to_string(func->parameters().size()) + "개, " +
            "전달 " + std::to_string(args.size()) + "개\n" +
            "해결 방법: 함수 정의를 확인하고 올바른 개수의 인자를 전달하세요."
        );
    }

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
    // 1) 변수가 스코프 체인에 존재하는지 확인
    if (!env_->existsInChain(stmt->varName()))
    {
        throw std::runtime_error("정의되지 않은 변수: " + stmt->varName());
    }

    // 2) 값 평가
    Value value = eval(const_cast<ast::Expression*>(stmt->value()));

    // 3) 환경에 값 갱신
    env_->set(stmt->varName(), value);

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

    Value result = Value::createNull();

    // count 횟수만큼 반복
    for (int64_t i = 0; i < count; ++i)
    {
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

    Value result = Value::createNull();

    // 범위 반복 (start부터 end까지, inclusive)
    for (int64_t i = start; i <= end; ++i)
    {
        // 반복 변수 설정
        env_->set(stmt->varName(), Value::createInteger(i));

        // 본문 실행
        result = eval(const_cast<ast::BlockStatement*>(stmt->body()));
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

    throw std::runtime_error("지원되지 않는 정수 연산자: " + op);
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

    throw std::runtime_error("지원되지 않는 실수 연산자: " + op);
}

Value Evaluator::applyComparisonOperation(const Value& left, const std::string& op, const Value& right)
{
    if (op == "==") return Value::createBoolean(left.equals(right));
    if (op == "!=") return Value::createBoolean(!left.equals(right));
    if (op == "<") return Value::createBoolean(left.lessThan(right));
    if (op == ">") return Value::createBoolean(left.greaterThan(right));
    if (op == "<=") return Value::createBoolean(left.lessThan(right) || left.equals(right));
    if (op == ">=") return Value::createBoolean(left.greaterThan(right) || left.equals(right));

    throw std::runtime_error("지원되지 않는 비교 연산자: " + op);
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

    throw std::runtime_error("지원되지 않는 논리 연산자: " + op);
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

Value Evaluator::evalIndexExpression(ast::IndexExpression* expr)
{
    Value array = eval(const_cast<ast::Expression*>(expr->array()));

    // 배열인지 확인
    if (!array.isArray())
    {
        throw error::TypeError(
            "배열이 아닌 값에 인덱스 접근을 시도했습니다: " + array.toString() + "\n" +
            "해결 방법: 인덱스 접근([])은 배열 타입에만 사용할 수 있습니다."
        );
    }

    std::vector<Value>& arr = array.asArray();

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

        // 슬라이싱 수행
        std::vector<Value> sliced;
        for (int64_t i = startIdx; i <= endIdx; ++i)
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
            "모듈 로더가 설정되지 않았습니다.\n" +
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

Value Evaluator::evalJosaExpression(ast::JosaExpression* expr)
{
    // 객체 평가
    Value object = eval(const_cast<ast::Expression*>(expr->object()));

    // 메서드 이름 획득
    const ast::Identifier* methodIdent = dynamic_cast<const ast::Identifier*>(expr->method());
    if (!methodIdent)
    {
        throw error::TypeError(
            "조사 표현식의 메서드는 식별자여야 합니다.\n" +
            "해결 방법: 메서드 이름을 확인하세요."
        );
    }

    std::string methodName = methodIdent->name();

    // 배열 메서드 처리
    if (object.isArray())
    {
        auto arr = object.asArray();

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
            "해결 방법: 지원되는 메서드는 '정렬한다', '역순으로_나열한다' 등입니다."
        );
    }

    // 배열이 아닌 타입에 대한 조사 표현식
    throw error::TypeError(
        "조사 표현식은 현재 배열에만 사용할 수 있습니다.\n" +
        "해결 방법: 배열 값에 메서드를 적용하세요."
    );
}

} // namespace evaluator
} // namespace kingsejong
