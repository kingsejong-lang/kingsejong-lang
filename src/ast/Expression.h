#pragma once

/**
 * @file Expression.h
 * @brief AST 표현식 노드 정의
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Node.h"
#include "lexer/JosaRecognizer.h"
#include <vector>

namespace kingsejong {
namespace ast {

// Forward declarations
class Statement;
class BlockStatement;

// ============================================================================
// 기본 리터럴 표현식
// ============================================================================

/**
 * @class Identifier
 * @brief 식별자 노드 (변수명, 함수명 등)
 *
 * @example "변수명", "함수이름", "배열"
 */
class Identifier : public Expression
{
private:
    std::string name_;

public:
    explicit Identifier(const std::string& name) : name_(name) {}

    NodeType type() const override { return NodeType::IDENTIFIER; }

    std::string toString() const override { return name_; }

    const std::string& name() const { return name_; }
};

/**
 * @class IntegerLiteral
 * @brief 정수 리터럴 노드
 *
 * @example 42, 123, -5
 */
class IntegerLiteral : public Expression
{
private:
    int64_t value_;

public:
    explicit IntegerLiteral(int64_t value) : value_(value) {}

    NodeType type() const override { return NodeType::INTEGER_LITERAL; }

    std::string toString() const override { return std::to_string(value_); }

    int64_t value() const { return value_; }
};

/**
 * @class FloatLiteral
 * @brief 실수 리터럴 노드
 *
 * @example 3.14, 2.718, -0.5
 */
class FloatLiteral : public Expression
{
private:
    double value_;

public:
    explicit FloatLiteral(double value) : value_(value) {}

    NodeType type() const override { return NodeType::FLOAT_LITERAL; }

    std::string toString() const override { return std::to_string(value_); }

    double value() const { return value_; }
};

/**
 * @class StringLiteral
 * @brief 문자열 리터럴 노드
 *
 * @example "안녕하세요", "Hello"
 */
class StringLiteral : public Expression
{
private:
    std::string value_;

public:
    explicit StringLiteral(const std::string& value) : value_(value) {}

    NodeType type() const override { return NodeType::STRING_LITERAL; }

    std::string toString() const override { return "\"" + value_ + "\""; }

    const std::string& value() const { return value_; }
};

/**
 * @class BooleanLiteral
 * @brief 불리언 리터럴 노드
 *
 * @example 참, 거짓
 */
class BooleanLiteral : public Expression
{
private:
    bool value_;

public:
    explicit BooleanLiteral(bool value) : value_(value) {}

    NodeType type() const override { return NodeType::BOOLEAN_LITERAL; }

    std::string toString() const override { return value_ ? "참" : "거짓"; }

    bool value() const { return value_; }
};

// ============================================================================
// 연산 표현식
// ============================================================================

/**
 * @class BinaryExpression
 * @brief 이항 연산 표현식
 *
 * @example a + b, x * y, left == right
 */
class BinaryExpression : public Expression
{
private:
    std::unique_ptr<Expression> left_;
    std::string operator_;
    std::unique_ptr<Expression> right_;

public:
    BinaryExpression(
        std::unique_ptr<Expression> left,
        const std::string& op,
        std::unique_ptr<Expression> right
    )
        : left_(std::move(left))
        , operator_(op)
        , right_(std::move(right))
    {}

    NodeType type() const override { return NodeType::BINARY_EXPRESSION; }

    std::string toString() const override
    {
        return "(" + left_->toString() + " " + operator_ + " " + right_->toString() + ")";
    }

    const Expression* left() const { return left_.get(); }
    const std::string& op() const { return operator_; }
    const Expression* right() const { return right_.get(); }
};

/**
 * @class UnaryExpression
 * @brief 단항 연산 표현식
 *
 * @example -x, !조건
 */
class UnaryExpression : public Expression
{
private:
    std::string operator_;
    std::unique_ptr<Expression> operand_;

public:
    UnaryExpression(const std::string& op, std::unique_ptr<Expression> operand)
        : operator_(op)
        , operand_(std::move(operand))
    {}

    NodeType type() const override { return NodeType::UNARY_EXPRESSION; }

    std::string toString() const override
    {
        return "(" + operator_ + operand_->toString() + ")";
    }

    const std::string& op() const { return operator_; }
    const Expression* operand() const { return operand_.get(); }
};

// ============================================================================
// KingSejong 특화 표현식
// ============================================================================

/**
 * @class JosaExpression
 * @brief 조사 표현식 (KingSejong 핵심 기능!)
 *
 * 한국어 조사를 활용한 메서드 체이닝 표현식입니다.
 *
 * @example
 * "배열을 정렬한다"
 * → JosaExpression {
 *     object: Identifier("배열")
 *     josaType: EUL_REUL
 *     method: Identifier("정렬한다")
 *   }
 *
 * "사용자의 이름"
 * → JosaExpression {
 *     object: Identifier("사용자")
 *     josaType: UI
 *     method: Identifier("이름")
 *   }
 */
class JosaExpression : public Expression
{
private:
    std::unique_ptr<Expression> object_;            ///< 명사 (객체)
    lexer::JosaRecognizer::JosaType josaType_;     ///< 조사 타입
    std::unique_ptr<Expression> method_;            ///< 동사/메서드

public:
    JosaExpression(
        std::unique_ptr<Expression> object,
        lexer::JosaRecognizer::JosaType josaType,
        std::unique_ptr<Expression> method
    )
        : object_(std::move(object))
        , josaType_(josaType)
        , method_(std::move(method))
    {}

    NodeType type() const override { return NodeType::JOSA_EXPRESSION; }

    std::string toString() const override
    {
        // 조사 타입을 문자열로 변환
        std::string josaName = lexer::JosaRecognizer::josaTypeToString(josaType_);

        return "JosaExpr(" +
               object_->toString() + " " +
               josaName + " " +
               method_->toString() + ")";
    }

    const Expression* object() const { return object_.get(); }
    lexer::JosaRecognizer::JosaType josaType() const { return josaType_; }
    const Expression* method() const { return method_.get(); }
};

/**
 * @class RangeExpression
 * @brief 범위 표현식 (KingSejong 핵심 기능!)
 *
 * 자연스러운 한국어 범위 표현을 지원합니다.
 *
 * @example
 * "1부터 10까지"
 * → RangeExpression {
 *     start: IntegerLiteral(1)
 *     end: IntegerLiteral(10)
 *     startInclusive: true
 *     endInclusive: true
 *   }
 *   수학적 표현: [1, 10]
 *
 * "1부터 10미만"
 * → RangeExpression {
 *     start: IntegerLiteral(1)
 *     end: IntegerLiteral(10)
 *     startInclusive: true
 *     endInclusive: false
 *   }
 *   수학적 표현: [1, 10)
 *
 * "1초과 10이하"
 * → RangeExpression {
 *     start: IntegerLiteral(1)
 *     end: IntegerLiteral(10)
 *     startInclusive: false
 *     endInclusive: true
 *   }
 *   수학적 표현: (1, 10]
 */
class RangeExpression : public Expression
{
private:
    std::unique_ptr<Expression> start_;     ///< 시작 값
    std::unique_ptr<Expression> end_;       ///< 끝 값
    bool startInclusive_;                   ///< 시작 값 포함 여부
    bool endInclusive_;                     ///< 끝 값 포함 여부

public:
    RangeExpression(
        std::unique_ptr<Expression> start,
        std::unique_ptr<Expression> end,
        bool startInclusive,
        bool endInclusive
    )
        : start_(std::move(start))
        , end_(std::move(end))
        , startInclusive_(startInclusive)
        , endInclusive_(endInclusive)
    {}

    NodeType type() const override { return NodeType::RANGE_EXPRESSION; }

    std::string toString() const override
    {
        // 수학적 표기법으로 표현
        std::string leftBracket = startInclusive_ ? "[" : "(";
        std::string rightBracket = endInclusive_ ? "]" : ")";

        return "Range" + leftBracket +
               start_->toString() + ", " +
               end_->toString() +
               rightBracket;
    }

    const Expression* start() const { return start_.get(); }
    const Expression* end() const { return end_.get(); }
    bool startInclusive() const { return startInclusive_; }
    bool endInclusive() const { return endInclusive_; }
};

// ============================================================================
// 함수 관련 표현식
// ============================================================================

/**
 * @class FunctionLiteral
 * @brief 함수 리터럴 표현식
 *
 * @example 함수(a, b) { 반환 a + b; }
 */
class FunctionLiteral : public Expression
{
private:
    std::vector<std::string> parameters_;          ///< 매개변수 이름 리스트
    std::unique_ptr<Statement> body_;              ///< 함수 본문 (BlockStatement)

public:
    FunctionLiteral(
        std::vector<std::string> parameters,
        std::unique_ptr<Statement> body
    )
        : parameters_(std::move(parameters))
        , body_(std::move(body))
    {}

    NodeType type() const override { return NodeType::FUNCTION_LITERAL; }

    std::string toString() const override
    {
        std::string result = "함수(";
        for (size_t i = 0; i < parameters_.size(); ++i)
        {
            if (i > 0) result += ", ";
            result += parameters_[i];
        }
        result += ") { ... }";
        return result;
    }

    const std::vector<std::string>& parameters() const { return parameters_; }
    Statement* body() const { return body_.get(); }
};

/**
 * @class CallExpression
 * @brief 함수 호출 표현식
 *
 * @example 덧셈(5, 3), 출력("안녕")
 */
class CallExpression : public Expression
{
private:
    std::unique_ptr<Expression> function_;              ///< 호출할 함수
    std::vector<std::unique_ptr<Expression>> arguments_; ///< 인자 목록

public:
    CallExpression(
        std::unique_ptr<Expression> function,
        std::vector<std::unique_ptr<Expression>> arguments
    )
        : function_(std::move(function))
        , arguments_(std::move(arguments))
    {}

    NodeType type() const override { return NodeType::CALL_EXPRESSION; }

    std::string toString() const override
    {
        std::string result = function_->toString() + "(";

        for (size_t i = 0; i < arguments_.size(); ++i)
        {
            if (i > 0) result += ", ";
            result += arguments_[i]->toString();
        }

        result += ")";
        return result;
    }

    const Expression* function() const { return function_.get(); }
    const std::vector<std::unique_ptr<Expression>>& arguments() const { return arguments_; }
};

// ============================================================================
// 배열 관련 표현식
// ============================================================================

/**
 * @class ArrayLiteral
 * @brief 배열 리터럴 표현식
 *
 * @example [1, 2, 3], ["a", "b", "c"]
 */
class ArrayLiteral : public Expression
{
private:
    std::vector<std::unique_ptr<Expression>> elements_;

public:
    explicit ArrayLiteral(std::vector<std::unique_ptr<Expression>> elements)
        : elements_(std::move(elements))
    {}

    NodeType type() const override { return NodeType::ARRAY_LITERAL; }

    std::string toString() const override
    {
        std::string result = "[";

        for (size_t i = 0; i < elements_.size(); ++i)
        {
            if (i > 0) result += ", ";
            result += elements_[i]->toString();
        }

        result += "]";
        return result;
    }

    const std::vector<std::unique_ptr<Expression>>& elements() const { return elements_; }
};

/**
 * @class IndexExpression
 * @brief 배열 인덱스 접근 표현식
 *
 * @example arr[0], matrix[i][j]
 */
class IndexExpression : public Expression
{
private:
    std::unique_ptr<Expression> array_;
    std::unique_ptr<Expression> index_;

public:
    IndexExpression(
        std::unique_ptr<Expression> array,
        std::unique_ptr<Expression> index
    )
        : array_(std::move(array))
        , index_(std::move(index))
    {}

    NodeType type() const override { return NodeType::INDEX_EXPRESSION; }

    std::string toString() const override
    {
        return array_->toString() + "[" + index_->toString() + "]";
    }

    const Expression* array() const { return array_.get(); }
    const Expression* index() const { return index_.get(); }
};

} // namespace ast
} // namespace kingsejong
