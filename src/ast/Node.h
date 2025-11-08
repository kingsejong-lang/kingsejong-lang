#pragma once

/**
 * @file Node.h
 * @brief AST 노드 기본 인터페이스
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include <string>
#include <memory>

namespace kingsejong {
namespace ast {

/**
 * @enum NodeType
 * @brief AST 노드의 종류를 나타내는 열거형
 */
enum class NodeType
{
    // 프로그램
    PROGRAM,                    ///< 프로그램 루트 노드

    // 문장 (Statements)
    EXPRESSION_STATEMENT,       ///< 표현식 문장
    VAR_DECLARATION,            ///< 변수 선언 (정수 x = 10)
    RETURN_STATEMENT,           ///< 반환 문장 (반환 x)
    IF_STATEMENT,               ///< 조건 문장 (만약)
    WHILE_STATEMENT,            ///< 반복 문장 (동안)
    FOR_STATEMENT,              ///< for 반복문
    REPEAT_STATEMENT,           ///< N번 반복 (10번 반복한다)
    RANGE_FOR_STATEMENT,        ///< 범위 반복문 (i가 1부터 10까지)
    BLOCK_STATEMENT,            ///< 블록 문장 { ... }

    // 표현식 (Expressions)
    IDENTIFIER,                 ///< 식별자 (변수명, 함수명)
    INTEGER_LITERAL,            ///< 정수 리터럴 (123)
    FLOAT_LITERAL,              ///< 실수 리터럴 (3.14)
    STRING_LITERAL,             ///< 문자열 리터럴 ("안녕")
    BOOLEAN_LITERAL,            ///< 불리언 리터럴 (참, 거짓)

    // 연산 표현식
    BINARY_EXPRESSION,          ///< 이항 연산 (a + b)
    UNARY_EXPRESSION,           ///< 단항 연산 (-x, !x)
    ASSIGNMENT_EXPRESSION,      ///< 할당 표현식 (x = 10)

    // KingSejong 특화 표현식
    JOSA_EXPRESSION,            ///< 조사 표현식 (배열을 정렬하고)
    RANGE_EXPRESSION,           ///< 범위 표현식 (1부터 10까지)

    // 함수 관련
    FUNCTION_LITERAL,           ///< 함수 리터럴
    CALL_EXPRESSION,            ///< 함수 호출 (함수명(인자))

    // 배열 관련
    ARRAY_LITERAL,              ///< 배열 리터럴 ([1, 2, 3])
    INDEX_EXPRESSION            ///< 인덱스 접근 (arr[0])
};

/**
 * @brief NodeType을 문자열로 변환
 * @param type 노드 타입
 * @return 노드 타입의 문자열 표현
 */
std::string nodeTypeToString(NodeType type);

/**
 * @class Node
 * @brief 모든 AST 노드의 기본 클래스
 *
 * 모든 AST 노드는 이 클래스를 상속받아야 합니다.
 * 가상 소멸자를 통해 다형성을 지원하고 메모리 누수를 방지합니다.
 */
class Node
{
public:
    /**
     * @brief 가상 소멸자
     *
     * 파생 클래스의 올바른 소멸을 보장합니다.
     */
    virtual ~Node() = default;

    /**
     * @brief 노드의 타입을 반환
     * @return NodeType 열거형 값
     */
    virtual NodeType type() const = 0;

    /**
     * @brief 노드를 문자열로 표현
     * @return 노드의 문자열 표현 (디버깅 및 출력용)
     *
     * @example
     * Identifier("x") → "x"
     * IntegerLiteral(42) → "42"
     * BinaryExpression(+, a, b) → "(a + b)"
     */
    virtual std::string toString() const = 0;

    /**
     * @brief 노드의 타입 이름을 반환
     * @return 노드 타입의 문자열 표현
     */
    std::string typeName() const
    {
        return nodeTypeToString(type());
    }
};

/**
 * @class Expression
 * @brief 모든 표현식 노드의 기본 클래스
 *
 * 표현식은 값을 평가할 수 있는 노드입니다.
 * 예: 1 + 2, 변수명, 함수 호출 등
 */
class Expression : public Node
{
public:
    virtual ~Expression() = default;
};

/**
 * @class Statement
 * @brief 모든 문장 노드의 기본 클래스
 *
 * 문장은 실행 가능한 명령입니다.
 * 예: 변수 선언, if문, return문 등
 */
class Statement : public Node
{
public:
    virtual ~Statement() = default;
};

} // namespace ast
} // namespace kingsejong
