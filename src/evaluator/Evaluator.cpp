/**
 * @file Evaluator.cpp
 * @brief KingSejong 언어의 AST 실행 엔진 구현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Evaluator.h"
#include <sstream>
#include <cmath>

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

        // Statements
        case ast::NodeType::PROGRAM:
            return evalProgram(static_cast<ast::Program*>(node));

        case ast::NodeType::EXPRESSION_STATEMENT:
            return evalExpressionStatement(static_cast<ast::ExpressionStatement*>(node));

        case ast::NodeType::VAR_DECLARATION:
            return evalVarDeclaration(static_cast<ast::VarDeclaration*>(node));

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
    try
    {
        return env_->get(ident->name());
    }
    catch (const std::runtime_error& e)
    {
        throw std::runtime_error("정의되지 않은 변수: " + ident->name());
    }
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

Value Evaluator::evalReturnStatement(ast::ReturnStatement* stmt)
{
    if (stmt->returnValue())
    {
        return eval(const_cast<ast::Expression*>(stmt->returnValue()));
    }
    return Value::createNull();
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
        throw std::runtime_error("반복 횟수는 정수여야 합니다");
    }

    int64_t count = countValue.asInteger();

    if (count < 0)
    {
        throw std::runtime_error("반복 횟수는 0 이상이어야 합니다");
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
            throw std::runtime_error("0으로 나눌 수 없습니다");
        }
        return Value::createInteger(left / right);
    }
    if (op == "%")
    {
        if (right == 0)
        {
            throw std::runtime_error("0으로 나머지 연산을 할 수 없습니다");
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

} // namespace evaluator
} // namespace kingsejong
