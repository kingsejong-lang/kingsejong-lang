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
    bool endInclusive_;                         ///< 끝 값 포함 여부 (까지=true, 미만=false)

public:
    RangeForStatement(
        const std::string& varName,
        std::unique_ptr<Expression> start,
        std::unique_ptr<Expression> end,
        std::unique_ptr<BlockStatement> body,
        bool endInclusive = true
    )
        : varName_(varName)
        , start_(std::move(start))
        , end_(std::move(end))
        , body_(std::move(body))
        , endInclusive_(endInclusive)
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
    bool endInclusive() const { return endInclusive_; }
};

/**
 * @class ImportStatement
 * @brief 모듈 가져오기 문장
 *
 * @example
 * 가져오기 "math"
 * 가져오기 "utils/helper"
 */
class ImportStatement : public Statement
{
private:
    std::string modulePath_;  ///< 모듈 경로

public:
    explicit ImportStatement(const std::string& modulePath)
        : modulePath_(modulePath)
    {}

    NodeType type() const override { return NodeType::IMPORT_STATEMENT; }

    std::string toString() const override
    {
        return "가져오기 \"" + modulePath_ + "\"";
    }

    const std::string& modulePath() const { return modulePath_; }
};

/**
 * @class CatchClause
 * @brief Catch 절 (예외 처리의 일부)
 *
 * Try-catch 문의 catch 절을 나타냅니다.
 * 에러를 받아서 처리하는 블록입니다.
 */
class CatchClause
{
private:
    std::string errorVarName_;           ///< 에러를 저장할 변수명 (예: e)
    std::unique_ptr<BlockStatement> body_;  ///< catch 블록 본문

public:
    /**
     * @brief CatchClause 생성자
     * @param errorVarName 에러를 저장할 변수명
     * @param body catch 블록 본문
     */
    CatchClause(const std::string& errorVarName, std::unique_ptr<BlockStatement> body)
        : errorVarName_(errorVarName)
        , body_(std::move(body))
    {}

    const std::string& errorVarName() const { return errorVarName_; }
    const BlockStatement* body() const { return body_.get(); }
};

/**
 * @class TryStatement
 * @brief 예외 처리 문장
 *
 * @example
 * 시도 {
 *     결과 = 10 / 0
 * } 오류 (e) {
 *     출력("에러:", e)
 * } 마지막 {
 *     출력("정리")
 * }
 */
class TryStatement : public Statement
{
private:
    std::unique_ptr<BlockStatement> tryBlock_;              ///< 시도 블록
    std::vector<std::unique_ptr<CatchClause>> catchClauses_;  ///< 오류 절 (여러 개 가능)
    std::unique_ptr<BlockStatement> finallyBlock_;          ///< 마지막 블록 (optional)

public:
    /**
     * @brief TryStatement 생성자
     * @param tryBlock 시도 블록
     * @param catchClauses 오류 절 리스트
     * @param finallyBlock 마지막 블록 (nullptr 가능)
     */
    TryStatement(
        std::unique_ptr<BlockStatement> tryBlock,
        std::vector<std::unique_ptr<CatchClause>> catchClauses,
        std::unique_ptr<BlockStatement> finallyBlock = nullptr
    )
        : tryBlock_(std::move(tryBlock))
        , catchClauses_(std::move(catchClauses))
        , finallyBlock_(std::move(finallyBlock))
    {}

    NodeType type() const override { return NodeType::TRY_STATEMENT; }

    std::string toString() const override
    {
        std::string result = "시도 " + tryBlock_->toString();

        for (const auto& catchClause : catchClauses_)
        {
            result += " 오류 (" + catchClause->errorVarName() + ") " +
                     catchClause->body()->toString();
        }

        if (finallyBlock_)
        {
            result += " 마지막 " + finallyBlock_->toString();
        }

        return result;
    }

    const BlockStatement* tryBlock() const { return tryBlock_.get(); }
    const std::vector<std::unique_ptr<CatchClause>>& catchClauses() const { return catchClauses_; }
    const BlockStatement* finallyBlock() const { return finallyBlock_.get(); }
};

/**
 * @class ThrowStatement
 * @brief 예외 던지기 문장
 *
 * @example
 * 던지다 Value::createError("Division by zero")
 * 던지다 에러메시지
 */
class ThrowStatement : public Statement
{
private:
    std::unique_ptr<Expression> value_;  ///< 던질 값 (Expression)

public:
    /**
     * @brief ThrowStatement 생성자
     * @param value 던질 값
     */
    explicit ThrowStatement(std::unique_ptr<Expression> value)
        : value_(std::move(value))
    {}

    NodeType type() const override { return NodeType::THROW_STATEMENT; }

    std::string toString() const override
    {
        return "던지다 " + (value_ ? value_->toString() : "null");
    }

    const Expression* value() const { return value_.get(); }
};

/**
 * @enum AccessModifier
 * @brief 클래스 멤버의 접근 제어자
 *
 * Phase 7.1에서 추가된 클래스 시스템의 접근 제어자입니다.
 */
enum class AccessModifier
{
    PUBLIC,     ///< 공개 (공개)
    PRIVATE     ///< 비공개 (비공개)
};

/**
 * @struct Parameter
 * @brief 함수 및 메서드 파라미터 정보
 */
struct Parameter
{
    std::string typeName;   ///< 타입 이름 (정수, 문자열 등)
    std::string name;       ///< 파라미터 이름

    Parameter(const std::string& t, const std::string& n)
        : typeName(t), name(n)
    {}
};

/**
 * @class FieldDeclaration
 * @brief 클래스 필드 선언
 *
 * Phase 7.1 클래스 시스템의 필드 선언입니다.
 *
 * @example
 * 비공개 문자열 이름
 * 공개 정수 나이
 */
class FieldDeclaration : public Node
{
private:
    AccessModifier access_;
    std::string typeName_;
    std::string fieldName_;
    std::unique_ptr<Expression> initializer_;

public:
    FieldDeclaration(
        AccessModifier access,
        const std::string& typeName,
        const std::string& fieldName,
        std::unique_ptr<Expression> initializer = nullptr
    )
        : access_(access)
        , typeName_(typeName)
        , fieldName_(fieldName)
        , initializer_(std::move(initializer))
    {}

    NodeType type() const override { return NodeType::FIELD_DECLARATION; }

    std::string toString() const override
    {
        std::string result = (access_ == AccessModifier::PUBLIC ? "공개 " : "비공개 ");
        result += typeName_ + " " + fieldName_;
        if (initializer_)
        {
            result += " = " + initializer_->toString();
        }
        return result;
    }

    AccessModifier access() const { return access_; }
    const std::string& typeName() const { return typeName_; }
    const std::string& fieldName() const { return fieldName_; }
    const Expression* initializer() const { return initializer_.get(); }
};

/**
 * @class ConstructorDeclaration
 * @brief 생성자 선언
 *
 * Phase 7.1 클래스 시스템의 생성자 선언입니다.
 *
 * @example
 * 생성자(이름, 나이) {
 *     자신.이름 = 이름
 *     자신.나이 = 나이
 * }
 */
class ConstructorDeclaration : public Node
{
private:
    std::vector<Parameter> parameters_;
    std::unique_ptr<BlockStatement> body_;

public:
    ConstructorDeclaration(
        std::vector<Parameter> parameters,
        std::unique_ptr<BlockStatement> body
    )
        : parameters_(std::move(parameters))
        , body_(std::move(body))
    {}

    NodeType type() const override { return NodeType::CONSTRUCTOR_DECLARATION; }

    std::string toString() const override
    {
        std::string result = "생성자(";
        for (size_t i = 0; i < parameters_.size(); ++i)
        {
            if (i > 0) result += ", ";
            result += parameters_[i].name;
        }
        result += ") { ... }";
        return result;
    }

    const std::vector<Parameter>& parameters() const { return parameters_; }
    const BlockStatement* body() const { return body_.get(); }
};

/**
 * @class MethodDeclaration
 * @brief 메서드 선언
 *
 * Phase 7.1 클래스 시스템의 메서드 선언입니다.
 *
 * @example
 * 공개 함수 인사하기() {
 *     출력("안녕하세요")
 * }
 *
 * 비공개 함수 내부함수(x, y) {
 *     반환 x + y
 * }
 */
class MethodDeclaration : public Node
{
private:
    AccessModifier access_;
    std::string returnType_;
    std::string methodName_;
    std::vector<Parameter> parameters_;
    std::unique_ptr<BlockStatement> body_;

public:
    MethodDeclaration(
        AccessModifier access,
        const std::string& returnType,
        const std::string& methodName,
        std::vector<Parameter> parameters,
        std::unique_ptr<BlockStatement> body
    )
        : access_(access)
        , returnType_(returnType)
        , methodName_(methodName)
        , parameters_(std::move(parameters))
        , body_(std::move(body))
    {}

    NodeType type() const override { return NodeType::METHOD_DECLARATION; }

    std::string toString() const override
    {
        std::string result = (access_ == AccessModifier::PUBLIC ? "공개 " : "비공개 ");
        result += "함수 " + methodName_ + "(";
        for (size_t i = 0; i < parameters_.size(); ++i)
        {
            if (i > 0) result += ", ";
            result += parameters_[i].name;
        }
        result += ") { ... }";
        return result;
    }

    AccessModifier access() const { return access_; }
    const std::string& returnType() const { return returnType_; }
    const std::string& methodName() const { return methodName_; }
    const std::vector<Parameter>& parameters() const { return parameters_; }
    const BlockStatement* body() const { return body_.get(); }
};

/**
 * @class ClassStatement
 * @brief 클래스 정의 문장
 *
 * Phase 7.1 클래스 시스템의 클래스 정의입니다.
 *
 * @example
 * 클래스 사람 {
 *     비공개 문자열 이름
 *     비공개 정수 나이
 *
 *     생성자(이름, 나이) {
 *         자신.이름 = 이름
 *         자신.나이 = 나이
 *     }
 *
 *     공개 함수 인사하기() {
 *         출력("안녕하세요, 저는 " + 자신.이름 + "입니다")
 *     }
 * }
 */
class ClassStatement : public Statement
{
private:
    std::string className_;
    std::vector<std::unique_ptr<FieldDeclaration>> fields_;
    std::unique_ptr<ConstructorDeclaration> constructor_;
    std::vector<std::unique_ptr<MethodDeclaration>> methods_;
    std::string superClass_;  ///< 상속받을 부모 클래스 (Phase 7.2)

public:
    ClassStatement(
        const std::string& className,
        std::vector<std::unique_ptr<FieldDeclaration>> fields,
        std::unique_ptr<ConstructorDeclaration> constructor,
        std::vector<std::unique_ptr<MethodDeclaration>> methods,
        const std::string& superClass = ""
    )
        : className_(className)
        , fields_(std::move(fields))
        , constructor_(std::move(constructor))
        , methods_(std::move(methods))
        , superClass_(superClass)
    {}

    NodeType type() const override { return NodeType::CLASS_STATEMENT; }

    std::string toString() const override
    {
        std::string result = "클래스 " + className_;
        if (!superClass_.empty())
        {
            result += " 상속 " + superClass_;
        }
        result += " {\n";

        // 필드
        for (const auto& field : fields_)
        {
            result += "  " + field->toString() + "\n";
        }

        // 생성자
        if (constructor_)
        {
            result += "  " + constructor_->toString() + "\n";
        }

        // 메서드
        for (const auto& method : methods_)
        {
            result += "  " + method->toString() + "\n";
        }

        result += "}";
        return result;
    }

    const std::string& className() const { return className_; }
    const std::vector<std::unique_ptr<FieldDeclaration>>& fields() const { return fields_; }
    const ConstructorDeclaration* constructor() const { return constructor_.get(); }
    const std::vector<std::unique_ptr<MethodDeclaration>>& methods() const { return methods_; }
    const std::string& superClass() const { return superClass_; }
};

} // namespace ast
} // namespace kingsejong
