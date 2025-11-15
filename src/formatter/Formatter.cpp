/**
 * @file Formatter.cpp
 * @brief Formatter 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "formatter/Formatter.h"
#include <sstream>

namespace kingsejong {
namespace formatter {

using namespace ast;

std::string Formatter::format(const Program* program)
{
    if (!program) return "";

    output_.str("");
    output_.clear();
    currentIndent_ = 0;

    const auto& statements = program->statements();
    for (size_t i = 0; i < statements.size(); ++i)
    {
        formatStatement(statements[i].get());

        // 마지막 문장이 아니면 빈 줄 추가
        if (i < statements.size() - 1)
        {
            writeLine();
        }
    }

    return output_.str();
}

// ============================================================================
// 문장 포맷팅
// ============================================================================

void Formatter::formatStatement(const Statement* stmt)
{
    if (!stmt) return;

    if (auto varDecl = dynamic_cast<const VarDeclaration*>(stmt))
    {
        formatVarDeclaration(varDecl);
    }
    else if (auto assignStmt = dynamic_cast<const AssignmentStatement*>(stmt))
    {
        formatAssignmentStatement(assignStmt);
    }
    else if (auto exprStmt = dynamic_cast<const ExpressionStatement*>(stmt))
    {
        formatExpressionStatement(exprStmt);
    }
    else if (auto retStmt = dynamic_cast<const ReturnStatement*>(stmt))
    {
        formatReturnStatement(retStmt);
    }
    else if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt))
    {
        formatIfStatement(ifStmt);
    }
    else if (auto whileStmt = dynamic_cast<const WhileStatement*>(stmt))
    {
        formatWhileStatement(whileStmt);
    }
    else if (auto forStmt = dynamic_cast<const RangeForStatement*>(stmt))
    {
        formatRangeForStatement(forStmt);
    }
    else if (auto repeatStmt = dynamic_cast<const RepeatStatement*>(stmt))
    {
        formatRepeatStatement(repeatStmt);
    }
    else if (auto blockStmt = dynamic_cast<const BlockStatement*>(stmt))
    {
        formatBlockStatement(blockStmt);
    }
}

void Formatter::formatVarDeclaration(const VarDeclaration* stmt)
{
    writeIndent();

    // 타입 이름
    if (!stmt->typeName().empty())
    {
        write(stmt->typeName());
        write(" ");
    }

    // 변수 이름
    write(stmt->varName());

    // 초기화 값
    if (stmt->initializer())
    {
        if (options_.spaceAroundOperators)
            write(" = ");
        else
            write("=");

        formatExpression(stmt->initializer());
    }

    writeLine();
}

void Formatter::formatAssignmentStatement(const AssignmentStatement* stmt)
{
    writeIndent();
    write(stmt->varName());

    if (options_.spaceAroundOperators)
        write(" = ");
    else
        write("=");

    formatExpression(stmt->value());
    writeLine();
}

void Formatter::formatExpressionStatement(const ExpressionStatement* stmt)
{
    writeIndent();
    formatExpression(stmt->expression());
    writeLine();
}

void Formatter::formatReturnStatement(const ReturnStatement* stmt)
{
    writeIndent();
    write("반환");

    if (stmt->returnValue())
    {
        write(" ");
        formatExpression(stmt->returnValue());
    }

    writeLine();
}

void Formatter::formatIfStatement(const IfStatement* stmt)
{
    writeIndent();
    write("만약");
    write(options_.spaceBeforeBrace ? " (" : "(");
    formatExpression(stmt->condition());
    write(options_.spaceBeforeBrace ? ") " : ")");
    writeLine("{");

    increaseIndent();
    if (auto thenBlock = dynamic_cast<const BlockStatement*>(stmt->thenBranch()))
    {
        for (const auto& s : thenBlock->statements())
        {
            formatStatement(s.get());
        }
    }
    decreaseIndent();

    writeIndent();
    write("}");

    if (stmt->elseBranch())
    {
        write(options_.spaceBeforeBrace ? " " : "");
        writeLine("아니면 {");

        increaseIndent();
        if (auto elseBlock = dynamic_cast<const BlockStatement*>(stmt->elseBranch()))
        {
            for (const auto& s : elseBlock->statements())
            {
                formatStatement(s.get());
            }
        }
        decreaseIndent();

        writeIndent();
        writeLine("}");
    }
    else
    {
        writeLine();
    }
}

void Formatter::formatWhileStatement(const WhileStatement* stmt)
{
    writeIndent();
    write("반복");
    write(options_.spaceBeforeBrace ? " (" : "(");
    formatExpression(stmt->condition());
    write(options_.spaceBeforeBrace ? ") " : ")");
    writeLine("{");

    increaseIndent();
    if (auto bodyBlock = dynamic_cast<const BlockStatement*>(stmt->body()))
    {
        for (const auto& s : bodyBlock->statements())
        {
            formatStatement(s.get());
        }
    }
    decreaseIndent();

    writeIndent();
    writeLine("}");
}

void Formatter::formatRangeForStatement(const RangeForStatement* stmt)
{
    writeIndent();
    write("범위");
    write(options_.spaceBeforeBrace ? " (" : "(");
    write(stmt->varName());
    write(": ");
    formatExpression(stmt->start());
    write("..");
    formatExpression(stmt->end());
    write(options_.spaceBeforeBrace ? ") " : ")");
    writeLine("{");

    increaseIndent();
    if (auto bodyBlock = dynamic_cast<const BlockStatement*>(stmt->body()))
    {
        for (const auto& s : bodyBlock->statements())
        {
            formatStatement(s.get());
        }
    }
    decreaseIndent();

    writeIndent();
    writeLine("}");
}

void Formatter::formatRepeatStatement(const RepeatStatement* stmt)
{
    writeIndent();
    write("반복");
    write(options_.spaceBeforeBrace ? " (" : "(");
    formatExpression(stmt->count());
    write(options_.spaceBeforeBrace ? ") " : ")");
    writeLine("{");

    increaseIndent();
    if (auto bodyBlock = dynamic_cast<const BlockStatement*>(stmt->body()))
    {
        for (const auto& s : bodyBlock->statements())
        {
            formatStatement(s.get());
        }
    }
    decreaseIndent();

    writeIndent();
    writeLine("}");
}

void Formatter::formatBlockStatement(const BlockStatement* stmt)
{
    writeIndent();
    writeLine("{");

    increaseIndent();
    for (const auto& s : stmt->statements())
    {
        formatStatement(s.get());
    }
    decreaseIndent();

    writeIndent();
    writeLine("}");
}

// ============================================================================
// 표현식 포맷팅
// ============================================================================

void Formatter::formatExpression(const Expression* expr)
{
    if (!expr) return;

    if (auto binExpr = dynamic_cast<const BinaryExpression*>(expr))
    {
        formatBinaryExpression(binExpr);
    }
    else if (auto unaryExpr = dynamic_cast<const UnaryExpression*>(expr))
    {
        formatUnaryExpression(unaryExpr);
    }
    else if (auto callExpr = dynamic_cast<const CallExpression*>(expr))
    {
        formatCallExpression(callExpr);
    }
    else if (auto indexExpr = dynamic_cast<const IndexExpression*>(expr))
    {
        formatIndexExpression(indexExpr);
    }
    else if (auto arrayLit = dynamic_cast<const ArrayLiteral*>(expr))
    {
        formatArrayLiteral(arrayLit);
    }
    else if (auto funcLit = dynamic_cast<const FunctionLiteral*>(expr))
    {
        formatFunctionLiteral(funcLit);
    }
    else if (auto ident = dynamic_cast<const Identifier*>(expr))
    {
        formatIdentifier(ident);
    }
    else if (auto intLit = dynamic_cast<const IntegerLiteral*>(expr))
    {
        formatIntegerLiteral(intLit);
    }
    else if (auto floatLit = dynamic_cast<const FloatLiteral*>(expr))
    {
        formatFloatLiteral(floatLit);
    }
    else if (auto strLit = dynamic_cast<const StringLiteral*>(expr))
    {
        formatStringLiteral(strLit);
    }
    else if (auto boolLit = dynamic_cast<const BooleanLiteral*>(expr))
    {
        formatBooleanLiteral(boolLit);
    }
}

void Formatter::formatBinaryExpression(const BinaryExpression* expr)
{
    formatExpression(expr->left());

    if (options_.spaceAroundOperators)
    {
        write(" ");
        write(expr->op());
        write(" ");
    }
    else
    {
        write(expr->op());
    }

    formatExpression(expr->right());
}

void Formatter::formatUnaryExpression(const UnaryExpression* expr)
{
    write(expr->op());
    formatExpression(expr->operand());
}

void Formatter::formatCallExpression(const CallExpression* expr)
{
    formatExpression(expr->function());
    write("(");

    const auto& args = expr->arguments();
    for (size_t i = 0; i < args.size(); ++i)
    {
        formatExpression(args[i].get());

        if (i < args.size() - 1)
        {
            write(",");
            if (options_.spaceAfterComma)
                write(" ");
        }
    }

    write(")");
}

void Formatter::formatIndexExpression(const IndexExpression* expr)
{
    formatExpression(expr->array());
    write("[");
    formatExpression(expr->index());
    write("]");
}

void Formatter::formatArrayLiteral(const ArrayLiteral* expr)
{
    write("[");

    const auto& elements = expr->elements();
    for (size_t i = 0; i < elements.size(); ++i)
    {
        formatExpression(elements[i].get());

        if (i < elements.size() - 1)
        {
            write(",");
            if (options_.spaceAfterComma)
                write(" ");
        }
    }

    write("]");
}

void Formatter::formatFunctionLiteral(const FunctionLiteral* expr)
{
    write("함수(");

    const auto& params = expr->parameters();
    for (size_t i = 0; i < params.size(); ++i)
    {
        write(params[i]);

        if (i < params.size() - 1)
        {
            write(",");
            if (options_.spaceAfterComma)
                write(" ");
        }
    }

    write(options_.spaceBeforeBrace ? ") " : ")");
    writeLine("{");

    increaseIndent();
    if (auto body = dynamic_cast<const BlockStatement*>(expr->body()))
    {
        for (const auto& stmt : body->statements())
        {
            formatStatement(stmt.get());
        }
    }
    decreaseIndent();

    writeIndent();
    write("}");
}

void Formatter::formatIdentifier(const Identifier* expr)
{
    write(expr->name());
}

void Formatter::formatIntegerLiteral(const IntegerLiteral* expr)
{
    write(std::to_string(expr->value()));
}

void Formatter::formatFloatLiteral(const FloatLiteral* expr)
{
    write(std::to_string(expr->value()));
}

void Formatter::formatStringLiteral(const StringLiteral* expr)
{
    write("\"");
    write(expr->value());
    write("\"");
}

void Formatter::formatBooleanLiteral(const BooleanLiteral* expr)
{
    write(expr->value() ? "참" : "거짓");
}

// ============================================================================
// 유틸리티
// ============================================================================

void Formatter::write(const std::string& text)
{
    output_ << text;
}

void Formatter::writeLine(const std::string& text)
{
    output_ << text << "\n";
}

void Formatter::writeIndent()
{
    output_ << getIndentString();
}

void Formatter::increaseIndent()
{
    ++currentIndent_;
}

void Formatter::decreaseIndent()
{
    if (currentIndent_ > 0)
        --currentIndent_;
}

std::string Formatter::getIndentString() const
{
    if (options_.useSpaces)
    {
        return std::string(currentIndent_ * options_.indentSize, ' ');
    }
    else
    {
        return std::string(currentIndent_, '\t');
    }
}

} // namespace formatter
} // namespace kingsejong
