#pragma once

/**
 * @file Formatter.h
 * @brief 코드 포맷터 - AST 기반 코드 재포맷팅
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "ast/Node.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include <string>
#include <sstream>

namespace kingsejong {
namespace formatter {

/**
 * @struct FormatterOptions
 * @brief 포맷팅 옵션 설정
 */
struct FormatterOptions
{
    int indentSize = 4;              ///< 들여쓰기 크기 (기본 4칸)
    bool useSpaces = true;            ///< 스페이스 사용 (false면 탭)
    bool spaceAroundOperators = true; ///< 연산자 주변 공백
    bool spaceAfterComma = true;      ///< 콤마 뒤 공백
    bool spaceBeforeBrace = true;     ///< 중괄호 앞 공백
    int maxLineLength = 100;          ///< 최대 줄 길이
};

/**
 * @class Formatter
 * @brief AST를 순회하며 일관된 포맷의 코드로 변환
 */
class Formatter
{
public:
    explicit Formatter(const FormatterOptions& options = FormatterOptions())
        : options_(options)
        , currentIndent_(0)
    {
    }

    /**
     * @brief 프로그램 전체를 포맷팅
     */
    std::string format(const ast::Program* program);

private:
    // 문장 포맷팅
    void formatStatement(const ast::Statement* stmt);
    void formatVarDeclaration(const ast::VarDeclaration* stmt);
    void formatAssignmentStatement(const ast::AssignmentStatement* stmt);
    void formatExpressionStatement(const ast::ExpressionStatement* stmt);
    void formatReturnStatement(const ast::ReturnStatement* stmt);
    void formatIfStatement(const ast::IfStatement* stmt);
    void formatWhileStatement(const ast::WhileStatement* stmt);
    void formatRangeForStatement(const ast::RangeForStatement* stmt);
    void formatRepeatStatement(const ast::RepeatStatement* stmt);
    void formatBlockStatement(const ast::BlockStatement* stmt);

    // 표현식 포맷팅
    void formatExpression(const ast::Expression* expr);
    void formatBinaryExpression(const ast::BinaryExpression* expr);
    void formatUnaryExpression(const ast::UnaryExpression* expr);
    void formatCallExpression(const ast::CallExpression* expr);
    void formatIndexExpression(const ast::IndexExpression* expr);
    void formatArrayLiteral(const ast::ArrayLiteral* expr);
    void formatFunctionLiteral(const ast::FunctionLiteral* expr);
    void formatIdentifier(const ast::Identifier* expr);
    void formatIntegerLiteral(const ast::IntegerLiteral* expr);
    void formatFloatLiteral(const ast::FloatLiteral* expr);
    void formatStringLiteral(const ast::StringLiteral* expr);
    void formatBooleanLiteral(const ast::BooleanLiteral* expr);

    // 유틸리티
    void write(const std::string& text);
    void writeLine(const std::string& text = "");
    void writeIndent();
    void increaseIndent();
    void decreaseIndent();
    std::string getIndentString() const;

    FormatterOptions options_;
    std::stringstream output_;
    int currentIndent_;
};

} // namespace formatter
} // namespace kingsejong
