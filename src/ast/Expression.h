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
 * @class InterpolatedString
 * @brief 문자열 보간 노드 (Phase 7.2)
 *
 * 문자열 안에 표현식을 삽입할 수 있는 기능입니다.
 * parts[0] + expr[0] + parts[1] + expr[1] + ... + parts[n] 형태로 구성됩니다.
 *
 * @example "이름: ${name}, 나이: ${age}"
 * parts = ["이름: ", ", 나이: ", ""]
 * expressions = [name, age]
 */
class InterpolatedString : public Expression
{
private:
    std::vector<std::string> parts_;                        ///< 문자열 조각들
    std::vector<std::unique_ptr<Expression>> expressions_;  ///< 보간 표현식들

public:
    InterpolatedString(
        std::vector<std::string> parts,
        std::vector<std::unique_ptr<Expression>> expressions
    )
        : parts_(std::move(parts))
        , expressions_(std::move(expressions))
    {}

    NodeType type() const override { return NodeType::INTERPOLATED_STRING; }

    std::string toString() const override
    {
        std::string result = "\"";
        for (size_t i = 0; i < parts_.size(); ++i)
        {
            result += parts_[i];
            if (i < expressions_.size())
            {
                result += "${" + expressions_[i]->toString() + "}";
            }
        }
        result += "\"";
        return result;
    }

    const std::vector<std::string>& parts() const { return parts_; }
    const std::vector<std::unique_ptr<Expression>>& expressions() const { return expressions_; }
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
    std::unique_ptr<Expression> step_;      ///< 스텝 값 (Phase 7.2)

public:
    RangeExpression(
        std::unique_ptr<Expression> start,
        std::unique_ptr<Expression> end,
        bool startInclusive,
        bool endInclusive,
        std::unique_ptr<Expression> step = nullptr
    )
        : start_(std::move(start))
        , end_(std::move(end))
        , startInclusive_(startInclusive)
        , endInclusive_(endInclusive)
        , step_(std::move(step))
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
    const Expression* step() const { return step_.get(); }  // Phase 7.2
    bool hasStep() const { return step_ != nullptr; }       // Phase 7.2
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

// ============================================================================
// 딕셔너리 관련 표현식 (Phase 7.2)
// ============================================================================

/**
 * @class DictionaryLiteral
 * @brief 딕셔너리 리터럴 표현식
 *
 * @example {"이름": "홍길동", "나이": 30}
 */
class DictionaryLiteral : public Expression
{
private:
    std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>> pairs_;

public:
    explicit DictionaryLiteral(
        std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>> pairs
    )
        : pairs_(std::move(pairs))
    {}

    NodeType type() const override { return NodeType::DICTIONARY_LITERAL; }

    std::string toString() const override
    {
        std::string result = "{";

        for (size_t i = 0; i < pairs_.size(); ++i)
        {
            if (i > 0) result += ", ";
            result += pairs_[i].first->toString() + ": " + pairs_[i].second->toString();
        }

        result += "}";
        return result;
    }

    const std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>>& pairs() const
    {
        return pairs_;
    }
};

// ============================================================================
// 패턴 매칭 (F5.5)
// ============================================================================

/**
 * @class Pattern
 * @brief 패턴 매칭 패턴의 기본 클래스
 *
 * 모든 패턴 노드는 이 클래스를 상속받습니다.
 * 패턴은 값과 매칭되는지 검사하고, 변수 바인딩을 수행합니다.
 */
class Pattern : public Node
{
public:
    virtual ~Pattern() = default;
};

/**
 * @class LiteralPattern
 * @brief 리터럴 패턴
 *
 * 특정 리터럴 값과 정확히 일치하는지 확인합니다.
 *
 * @example
 * 1 -> "하나"
 * "hello" -> "안녕"
 * 참 -> "맞음"
 */
class LiteralPattern : public Pattern
{
private:
    std::unique_ptr<Expression> value_;

public:
    explicit LiteralPattern(std::unique_ptr<Expression> value)
        : value_(std::move(value))
    {}

    NodeType type() const override { return NodeType::LITERAL_PATTERN; }

    std::string toString() const override
    {
        return value_->toString();
    }

    const Expression* value() const { return value_.get(); }
};

/**
 * @class WildcardPattern
 * @brief 와일드카드 패턴 (_)
 *
 * 모든 값과 매칭됩니다. 보통 기본 케이스로 사용됩니다.
 *
 * @example
 * _ -> "기타"
 */
class WildcardPattern : public Pattern
{
public:
    WildcardPattern() = default;

    NodeType type() const override { return NodeType::WILDCARD_PATTERN; }

    std::string toString() const override { return "_"; }
};

/**
 * @class BindingPattern
 * @brief 바인딩 패턴 (변수명)
 *
 * 값을 변수에 바인딩합니다. 모든 값과 매칭되며, 매칭된 값을 변수에 저장합니다.
 *
 * @example
 * x -> x * 2     // x에 값이 바인딩됨
 * name -> "안녕 " + name
 */
class BindingPattern : public Pattern
{
private:
    std::string name_;

public:
    explicit BindingPattern(const std::string& name)
        : name_(name)
    {}

    NodeType type() const override { return NodeType::BINDING_PATTERN; }

    std::string toString() const override { return name_; }

    const std::string& name() const { return name_; }
};

/**
 * @class ArrayPattern
 * @brief 배열 패턴 [a, b, ...rest]
 *
 * 배열을 분해하여 각 요소를 패턴과 매칭합니다.
 * 선택적으로 나머지 요소를 수집할 수 있습니다.
 *
 * @example
 * [첫번째, 두번째] -> 첫번째 + 두번째
 * [head, ...tail] -> head
 * [] -> "빈 배열"
 */
class ArrayPattern : public Pattern
{
private:
    std::vector<std::unique_ptr<Pattern>> elements_;
    std::string rest_;  // 나머지 요소 변수명 (빈 문자열이면 없음)

public:
    ArrayPattern(
        std::vector<std::unique_ptr<Pattern>> elements,
        const std::string& rest = ""
    )
        : elements_(std::move(elements))
        , rest_(rest)
    {}

    NodeType type() const override { return NodeType::ARRAY_PATTERN; }

    std::string toString() const override
    {
        std::string result = "[";
        for (size_t i = 0; i < elements_.size(); i++) {
            if (i > 0) result += ", ";
            result += elements_[i]->toString();
        }
        if (!rest_.empty()) {
            if (!elements_.empty()) result += ", ";
            result += "..." + rest_;
        }
        result += "]";
        return result;
    }

    const std::vector<std::unique_ptr<Pattern>>& elements() const { return elements_; }
    const std::string& rest() const { return rest_; }
};

/**
 * @class MatchCase
 * @brief 패턴 매칭 케이스 (패턴 -> 결과)
 *
 * 하나의 패턴과 그에 대응하는 결과 표현식을 나타냅니다.
 * 선택적으로 가드 조건을 추가할 수 있습니다.
 *
 * @example
 * 1 -> "하나"
 * x when x > 10 -> "큰 수"
 * [a, b] -> a + b
 */
class MatchCase
{
private:
    std::unique_ptr<Pattern> pattern_;
    std::unique_ptr<Expression> guard_;     // 선택적 가드 조건
    std::unique_ptr<Expression> body_;

public:
    MatchCase(
        std::unique_ptr<Pattern> pattern,
        std::unique_ptr<Expression> body,
        std::unique_ptr<Expression> guard = nullptr
    )
        : pattern_(std::move(pattern))
        , guard_(std::move(guard))
        , body_(std::move(body))
    {}

    std::string toString() const
    {
        std::string result = pattern_->toString();
        if (guard_) {
            result += " when " + guard_->toString();
        }
        result += " -> " + body_->toString();
        return result;
    }

    const Pattern* pattern() const { return pattern_.get(); }
    const Expression* guard() const { return guard_.get(); }
    const Expression* body() const { return body_.get(); }
};

/**
 * @class MatchExpression
 * @brief 패턴 매칭 표현식
 *
 * 값을 여러 패턴과 매칭하여 첫 번째로 매칭되는 케이스의 결과를 반환합니다.
 *
 * @example
 * 값에 대해 {
 *     1 -> "하나"
 *     2 -> "둘"
 *     x when x > 10 -> "큰 수"
 *     _ -> "기타"
 * }
 */
class MatchExpression : public Expression
{
private:
    std::unique_ptr<Expression> value_;
    std::vector<MatchCase> cases_;

public:
    MatchExpression(
        std::unique_ptr<Expression> value,
        std::vector<MatchCase> cases
    )
        : value_(std::move(value))
        , cases_(std::move(cases))
    {}

    NodeType type() const override { return NodeType::MATCH_EXPRESSION; }

    std::string toString() const override
    {
        std::string result = value_->toString() + "에 대해 {\n";
        for (const auto& c : cases_) {
            result += "    " + c.toString() + "\n";
        }
        result += "}";
        return result;
    }

    const Expression* value() const { return value_.get(); }
    const std::vector<MatchCase>& cases() const { return cases_; }
};

/**
 * @class MemberAccessExpression
 * @brief 멤버 접근 표현식
 *
 * Phase 7.1 클래스 시스템의 멤버 접근입니다.
 * 객체의 필드나 메서드에 접근할 때 사용됩니다.
 *
 * @example
 * 홍길동.이름
 * 홍길동.나이
 * 홍길동.인사하기()
 */
class MemberAccessExpression : public Expression
{
private:
    std::unique_ptr<Expression> object_;
    std::string memberName_;

public:
    MemberAccessExpression(
        std::unique_ptr<Expression> object,
        const std::string& memberName
    )
        : object_(std::move(object))
        , memberName_(memberName)
    {}

    NodeType type() const override { return NodeType::MEMBER_ACCESS_EXPRESSION; }

    std::string toString() const override
    {
        return object_->toString() + "." + memberName_;
    }

    const Expression* object() const { return object_.get(); }
    const std::string& memberName() const { return memberName_; }
};

/**
 * @class ThisExpression
 * @brief this 표현식
 *
 * Phase 7.1 클래스 시스템의 this 키워드입니다.
 * 메서드나 생성자 내에서 현재 인스턴스를 참조할 때 사용됩니다.
 *
 * @example
 * 자신.이름 = 이름
 * 자신.나이 = 나이
 */
class ThisExpression : public Expression
{
public:
    ThisExpression() = default;

    NodeType type() const override { return NodeType::THIS_EXPRESSION; }

    std::string toString() const override
    {
        return "자신";
    }
};

/**
 * @class NewExpression
 * @brief 객체 생성 표현식
 *
 * Phase 7.1 클래스 시스템의 객체 생성입니다.
 * 클래스 이름과 생성자 인자를 받아 새 인스턴스를 생성합니다.
 *
 * @example
 * 사람("홍길동", 30)
 * 계좌(10000)
 * 사각형(5.0, 3.0)
 */
class NewExpression : public Expression
{
private:
    std::string className_;
    std::vector<std::unique_ptr<Expression>> arguments_;

public:
    NewExpression(
        const std::string& className,
        std::vector<std::unique_ptr<Expression>> arguments
    )
        : className_(className)
        , arguments_(std::move(arguments))
    {}

    NodeType type() const override { return NodeType::NEW_EXPRESSION; }

    std::string toString() const override
    {
        std::string result = className_ + "(";
        for (size_t i = 0; i < arguments_.size(); ++i)
        {
            if (i > 0) result += ", ";
            result += arguments_[i]->toString();
        }
        result += ")";
        return result;
    }

    const std::string& className() const { return className_; }
    const std::vector<std::unique_ptr<Expression>>& arguments() const { return arguments_; }
};

} // namespace ast
} // namespace kingsejong
