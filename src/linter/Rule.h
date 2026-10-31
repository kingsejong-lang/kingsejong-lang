#pragma once

/**
 * @file Rule.h
 * @brief Linter 규칙 베이스 클래스
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "ast/Node.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include <string>

namespace kingsejong {
namespace linter {

// Forward declaration
class Linter;
struct LinterIssue;
enum class IssueSeverity;

/**
 * @class Rule
 * @brief Linter 규칙 베이스 클래스
 */
class HookDispatcher;

class Rule
{
public:
    /**
     * @brief Rule 생성자
     * @param ruleId 규칙 ID
     * @param description 규칙 설명
     */
    Rule(const std::string& ruleId, const std::string& description)
        : ruleId_(ruleId), description_(description), linter_(nullptr)
    {
    }

    virtual ~Rule() = default;

    /**
     * @brief 규칙 ID 반환
     */
    const std::string& ruleId() const { return ruleId_; }

    /**
     * @brief 규칙 설명 반환
     */
    const std::string& description() const { return description_; }

    /**
     * @brief 프로그램 분석
     * @param program AST 루트 노드
     */
    virtual void analyze(ast::Program* program) = 0;

    /**
     * @brief Linter 설정 (Linter에서 호출)
     */
    void setLinter(Linter* linter) { linter_ = linter; }

protected:
    /**
     * @brief 이슈 보고
     * @param message 이슈 메시지
     * @param severity 심각도
     * @param line 줄 번호
     * @param column 열 번호
     * @param subject 이슈가 가리키는 이름. 편집기가 문장 첫머리 대신 이 이름에 밑줄을 긋는다
     */
    void reportIssue(
        const std::string& message,
        IssueSeverity severity,
        int line = 0,
        int column = 0,
        const std::string& subject = ""
    );

    /**
     * @brief Statement 순회
     */
    // 순회(ast::walk) 어댑터가 on* 훅을 부를 수 있어야 한다
    friend class HookDispatcher;

    void visitStatement(ast::Statement* stmt);

    /**
     * @brief Expression 순회
     */
    void visitExpression(ast::Expression* expr);

    /**
     * @brief 특정 Statement 타입 처리 (오버라이드 가능)
     */
    virtual void onVarDeclaration(ast::VarDeclaration* /* stmt */) {}
    virtual void onAssignmentStatement(ast::AssignmentStatement* /* stmt */) {}
    virtual void onExpressionStatement(ast::ExpressionStatement* /* stmt */) {}
    virtual void onReturnStatement(ast::ReturnStatement* /* stmt */) {}
    virtual void onIfStatement(ast::IfStatement* /* stmt */) {}
    virtual void onWhileStatement(ast::WhileStatement* /* stmt */) {}
    virtual void onBlockStatement(ast::BlockStatement* /* stmt */) {}
    virtual void onRangeForStatement(ast::RangeForStatement* /* stmt */) {}
    virtual void onRepeatStatement(ast::RepeatStatement* /* stmt */) {}
    virtual void onImportStatement(ast::ImportStatement* /* stmt */) {}

    /**
     * @brief 특정 Expression 타입 처리 (오버라이드 가능)
     */
    virtual void onIdentifier(ast::Identifier* /* expr */) {}
    virtual void onIntegerLiteral(ast::IntegerLiteral* /* expr */) {}
    virtual void onFloatLiteral(ast::FloatLiteral* /* expr */) {}
    virtual void onStringLiteral(ast::StringLiteral* /* expr */) {}
    virtual void onBooleanLiteral(ast::BooleanLiteral* /* expr */) {}
    virtual void onArrayLiteral(ast::ArrayLiteral* /* expr */) {}
    virtual void onBinaryExpression(ast::BinaryExpression* /* expr */) {}
    virtual void onUnaryExpression(ast::UnaryExpression* /* expr */) {}
    virtual void onCallExpression(ast::CallExpression* /* expr */) {}
    virtual void onIndexExpression(ast::IndexExpression* /* expr */) {}
    virtual void onFunctionLiteral(ast::FunctionLiteral* /* expr */) {}
    virtual void onJosaExpression(ast::JosaExpression* /* expr */) {}

    // 자식을 모두 걷은 뒤 불리는 훅. 노드 안에 있는 동안만 켜 두는 상태를 되돌릴 때 쓴다.
    virtual void onVarDeclarationEnd(ast::VarDeclaration* /* stmt */) {}
    virtual void onArrayLiteralEnd(ast::ArrayLiteral* /* expr */) {}

private:
    std::string ruleId_;        ///< 규칙 ID
    std::string description_;   ///< 규칙 설명
    Linter* linter_;            ///< Linter 포인터
};

} // namespace linter
} // namespace kingsejong
