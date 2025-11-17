#pragma once

/**
 * @file Evaluator.h
 * @brief KingSejong 언어의 AST 실행 엔진
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Value.h"
#include "Environment.h"
#include "../ast/Node.h"
#include "../ast/Expression.h"
#include "../ast/Statement.h"
#include <memory>
#include <stdexcept>
#include <chrono>

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
     * @brief 최대 평가 횟수 설정
     * @param max 최대 평가 횟수
     */
    void setMaxEvaluations(size_t max) { maxEvaluations_ = max; }

    /**
     * @brief 최대 실행 시간 설정 (밀리초)
     * @param ms 최대 실행 시간
     */
    void setMaxExecutionTime(size_t ms) { maxExecutionTime_ = std::chrono::milliseconds(ms); }

private:
    std::shared_ptr<Environment> env_;  ///< 변수 저장 환경
    module::ModuleLoader* moduleLoader_ = nullptr;  ///< 모듈 로더
    jit::HotPathDetector* hotPathDetector_ = nullptr;  ///< 핫 패스 감지기

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
    Value applyIntegerOperation(int64_t left, const std::string& op, int64_t right);

    /**
     * @brief 두 실수 값에 대한 산술 연산 수행
     * @param left 왼쪽 피연산자
     * @param op 연산자 (+, -, *, /)
     * @param right 오른쪽 피연산자
     * @return 연산 결과
     */
    Value applyFloatOperation(double left, const std::string& op, double right);

    /**
     * @brief 두 값의 비교 연산 수행
     * @param left 왼쪽 피연산자
     * @param op 연산자 (==, !=, <, >, <=, >=)
     * @param right 오른쪽 피연산자
     * @return 비교 결과 (불리언)
     */
    Value applyComparisonOperation(const Value& left, const std::string& op, const Value& right);

    /**
     * @brief 논리 연산 수행
     * @param left 왼쪽 피연산자
     * @param op 연산자 (&&, ||)
     * @param right 오른쪽 피연산자
     * @return 논리 연산 결과 (불리언)
     */
    Value applyLogicalOperation(const Value& left, const std::string& op, const Value& right);

    /**
     * @brief 안전 장치 체크 (무한 루프 방지)
     *
     * 평가 횟수와 실행 시간을 체크하여 제한을 초과하면 예외를 던집니다.
     */
    void checkSafetyLimits();
};

} // namespace evaluator
} // namespace kingsejong
