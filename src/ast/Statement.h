#pragma once

/**
 * @file Statement.h
 * @brief AST 문장 노드 정의
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Node.h"
#include "Expression.h"
#include "types/Type.h"
#include <vector>

namespace kingsejong {
namespace ast {

/**
 * @class Program
 * @brief 프로그램 루트 노드
 *
 * 전체 프로그램을 나타내는 최상위 노드입니다.
 * 여러 개의 문장들을 포함합니다.
 */
class Program : public Node
{
private:
    std::vector<std::unique_ptr<Statement>> statements_;

public:
    explicit Program(std::vector<std::unique_ptr<Statement>> statements)
        : statements_(std::move(statements))
    {}

    Program() = default;

    NodeType type() const override { return NodeType::PROGRAM; }

    std::string toString() const override
    {
        std::string result = "Program {\n";

        for (const auto& stmt : statements_)
        {
            result += "  " + stmt->toString() + "\n";
        }

        result += "}";
        return result;
    }

    void addStatement(std::unique_ptr<Statement> stmt)
    {
        statements_.push_back(std::move(stmt));
    }

    const std::vector<std::unique_ptr<Statement>>& statements() const
    {
        return statements_;
    }
};

/**
 * @class ExpressionStatement
 * @brief 표현식 문장
 *
 * 표현식을 문장으로 사용합니다.
 *
 * @example
 * x + 1;
 * 함수호출();
 */
class ExpressionStatement : public Statement
{
private:
    std::unique_ptr<Expression> expression_;

public:
    explicit ExpressionStatement(std::unique_ptr<Expression> expression)
        : expression_(std::move(expression))
    {}

    NodeType type() const override { return NodeType::EXPRESSION_STATEMENT; }

    std::string toString() const override
    {
        return expression_ ? expression_->toString() + ";" : ";";
    }

    const Expression* expression() const { return expression_.get(); }
};

/**
 * @class VarDeclaration
 * @brief 변수 선언 문장
 *
 * @example
 * 정수 x = 10;
 * 문자열 이름 = "홍길동";
 */
class VarDeclaration : public Statement
{
private:
    std::string typeName_;      ///< 타입 이름 (정수, 실수, 문자열 등)
    types::Type* varType_;      ///< 타입 객체 포인터 (optional, nullptr이면 타입 추론)
    std::string varName_;       ///< 변수 이름
    std::unique_ptr<Expression> initializer_;  ///< 초기값 (optional)

public:
    VarDeclaration(
        const std::string& typeName,
        const std::string& varName,
        std::unique_ptr<Expression> initializer = nullptr,
        types::Type* varType = nullptr
    )
        : typeName_(typeName)
        , varType_(varType)
        , varName_(varName)
        , initializer_(std::move(initializer))
    {}

    NodeType type() const override { return NodeType::VAR_DECLARATION; }

    std::string toString() const override
    {
        std::string result = typeName_ + " " + varName_;

        if (initializer_)
        {
            result += " = " + initializer_->toString();
        }

        return result + ";";
    }

    const std::string& typeName() const { return typeName_; }
    types::Type* varType() const { return varType_; }
    const std::string& varName() const { return varName_; }
    const Expression* initializer() const { return initializer_.get(); }
};

/**
 * @class AssignmentStatement
 * @brief 변수 할당 문장
 *
 * 이미 선언된 변수에 새로운 값을 할당합니다.
 *
 * @example
 * count = count + 1;
 * x = 10;
 */
class AssignmentStatement : public Statement
{
private:
    std::string varName_;       ///< 변수 이름
    std::unique_ptr<Expression> value_;  ///< 할당할 값

public:
    AssignmentStatement(
        const std::string& varName,
        std::unique_ptr<Expression> value
    )
        : varName_(varName)
        , value_(std::move(value))
    {}

    NodeType type() const override { return NodeType::ASSIGNMENT_STATEMENT; }

    std::string toString() const override
    {
        return varName_ + " = " + (value_ ? value_->toString() : "null") + ";";
    }

    const std::string& varName() const { return varName_; }
    const Expression* value() const { return value_.get(); }
};

/**
 * @class ReturnStatement
 * @brief 반환 문장
 *
 * @example
 * 반환 x + 1;
 * 반환;
 */
class ReturnStatement : public Statement
{
private:
    std::unique_ptr<Expression> returnValue_;

public:
    explicit ReturnStatement(std::unique_ptr<Expression> returnValue = nullptr)
        : returnValue_(std::move(returnValue))
    {}

    NodeType type() const override { return NodeType::RETURN_STATEMENT; }

    std::string toString() const override
    {
        if (returnValue_)
        {
            return "반환 " + returnValue_->toString() + ";";
        }
        return "반환;";
    }

    const Expression* returnValue() const { return returnValue_.get(); }
};

/**
 * @class BlockStatement
 * @brief 블록 문장
 *
 * 여러 문장을 묶어서 하나의 블록으로 만듭니다.
 *
 * @example
 * {
 *     정수 x = 1;
 *     출력(x);
 * }
 */
class BlockStatement : public Statement
{
private:
    std::vector<std::unique_ptr<Statement>> statements_;

public:
    explicit BlockStatement(std::vector<std::unique_ptr<Statement>> statements)
        : statements_(std::move(statements))
    {}

    BlockStatement() = default;

    NodeType type() const override { return NodeType::BLOCK_STATEMENT; }

    std::string toString() const override
    {
        std::string result = "{\n";

        for (const auto& stmt : statements_)
        {
            result += "  " + stmt->toString() + "\n";
        }

        result += "}";
        return result;
    }

    void addStatement(std::unique_ptr<Statement> stmt)
    {
        statements_.push_back(std::move(stmt));
    }

    const std::vector<std::unique_ptr<Statement>>& statements() const
    {
        return statements_;
    }
};

/**
 * @class IfStatement
 * @brief 조건 문장
 *
 * @example
 * 만약 (x > 0) {
 *     출력("양수");
 * } 아니면 {
 *     출력("음수 또는 0");
 * }
 */
class IfStatement : public Statement
{
private:
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<BlockStatement> thenBranch_;
    std::unique_ptr<BlockStatement> elseBranch_;  ///< optional

public:
    IfStatement(
        std::unique_ptr<Expression> condition,
        std::unique_ptr<BlockStatement> thenBranch,
        std::unique_ptr<BlockStatement> elseBranch = nullptr
    )
        : condition_(std::move(condition))
        , thenBranch_(std::move(thenBranch))
        , elseBranch_(std::move(elseBranch))
    {}

    NodeType type() const override { return NodeType::IF_STATEMENT; }

    std::string toString() const override
    {
        std::string result = "만약 (" + condition_->toString() + ") " +
                            thenBranch_->toString();

        if (elseBranch_)
        {
            result += " 아니면 " + elseBranch_->toString();
        }

        return result;
    }

    const Expression* condition() const { return condition_.get(); }
    const BlockStatement* thenBranch() const { return thenBranch_.get(); }
    const BlockStatement* elseBranch() const { return elseBranch_.get(); }
};

/**
 * @class WhileStatement
 * @brief 반복 문장 (while)
 *
 * @example
 * (x > 0) 동안 {
 *     x = x - 1;
 * }
 */
class WhileStatement : public Statement
{
private:
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<BlockStatement> body_;

public:
    WhileStatement(
        std::unique_ptr<Expression> condition,
        std::unique_ptr<BlockStatement> body
    )
        : condition_(std::move(condition))
        , body_(std::move(body))
    {}

    NodeType type() const override { return NodeType::WHILE_STATEMENT; }

    std::string toString() const override
    {
        return "(" + condition_->toString() + ") 동안 " + body_->toString();
    }

    const Expression* condition() const { return condition_.get(); }
    const BlockStatement* body() const { return body_.get(); }
};

/**
 * @class ForStatement
 * @brief For 반복문
 *
 * @example
 * i가 1부터 10까지 반복 {
 *     출력(i);
 * }
 */
class ForStatement : public Statement
{
private:
    std::string iterator_;                      ///< 반복 변수명
    std::unique_ptr<Expression> range_;         ///< 범위 (RangeExpression)
    std::unique_ptr<BlockStatement> body_;      ///< 반복 본문

public:
    ForStatement(
        const std::string& iterator,
        std::unique_ptr<Expression> range,
        std::unique_ptr<BlockStatement> body
    )
        : iterator_(iterator)
        , range_(std::move(range))
        , body_(std::move(body))
    {}

    NodeType type() const override { return NodeType::FOR_STATEMENT; }

    std::string toString() const override
    {
        return iterator_ + "가 " + range_->toString() + " 반복 " +
               body_->toString();
    }

    const std::string& iterator() const { return iterator_; }
    const Expression* range() const { return range_.get(); }
    const BlockStatement* body() const { return body_.get(); }
};

/**
 * @class RepeatStatement
 * @brief N번 반복문
 *
 * @example
 * 10번 반복한다 {
 *     출력("안녕");
 * }
 */
class RepeatStatement : public Statement
{
private:
    std::unique_ptr<Expression> count_;         ///< 반복 횟수 표현식
    std::unique_ptr<BlockStatement> body_;      ///< 반복 본문

public:
    RepeatStatement(
        std::unique_ptr<Expression> count,
        std::unique_ptr<BlockStatement> body
    )
        : count_(std::move(count))
        , body_(std::move(body))
    {}

    NodeType type() const override { return NodeType::REPEAT_STATEMENT; }

    std::string toString() const override
    {
        return count_->toString() + "번 반복 " + body_->toString();
    }

    const Expression* count() const { return count_.get(); }
    const BlockStatement* body() const { return body_.get(); }
};

/**
 * @class RangeForStatement
 * @brief 범위 기반 for 반복문
 *
 * @example
 * i가 1부터 10까지 반복한다 {
 *     출력(i);
 * }
 */
class RangeForStatement : public Statement
{
private:
    std::string varName_;                       ///< 반복 변수 이름
    std::unique_ptr<Expression> start_;         ///< 시작 값 표현식
    std::unique_ptr<Expression> end_;           ///< 끝 값 표현식
    std::unique_ptr<BlockStatement> body_;      ///< 반복 본문

public:
    RangeForStatement(
        const std::string& varName,
        std::unique_ptr<Expression> start,
        std::unique_ptr<Expression> end,
        std::unique_ptr<BlockStatement> body
    )
        : varName_(varName)
        , start_(std::move(start))
        , end_(std::move(end))
        , body_(std::move(body))
    {}

    NodeType type() const override { return NodeType::RANGE_FOR_STATEMENT; }

    std::string toString() const override
    {
        return varName_ + "가 " + start_->toString() + "부터 " +
               end_->toString() + "까지 " + body_->toString();
    }

    const std::string& varName() const { return varName_; }
    const Expression* start() const { return start_.get(); }
    const Expression* end() const { return end_.get(); }
    const BlockStatement* body() const { return body_.get(); }
};

} // namespace ast
} // namespace kingsejong
