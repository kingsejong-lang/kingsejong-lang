/**
 * @file Formatter.cpp
 * @brief Formatter 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "formatter/Formatter.h"
#include "formatter/FormatterConfig.h"
#include "config/ConfigLoader.h"
#include <sstream>

namespace kingsejong {
namespace formatter {

using json = nlohmann::json;

using namespace ast;

std::string Formatter::format(const Program* program)
{
    if (!program) return "";

    output_.str("");
    output_.clear();
    currentIndent_ = 0;
    unsupported_.clear();

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

    switch (stmt->type())
    {
        case NodeType::VAR_DECLARATION:
        {
            auto varDecl = static_cast<const VarDeclaration*>(stmt);
            formatVarDeclaration(varDecl);
            break;
        }
        case NodeType::ASSIGNMENT_STATEMENT:
        {
            auto assignStmt = static_cast<const AssignmentStatement*>(stmt);
            formatAssignmentStatement(assignStmt);
            break;
        }
        case NodeType::EXPRESSION_STATEMENT:
        {
            auto exprStmt = static_cast<const ExpressionStatement*>(stmt);
            formatExpressionStatement(exprStmt);
            break;
        }
        case NodeType::RETURN_STATEMENT:
        {
            auto retStmt = static_cast<const ReturnStatement*>(stmt);
            formatReturnStatement(retStmt);
            break;
        }
        case NodeType::IF_STATEMENT:
        {
            auto ifStmt = static_cast<const IfStatement*>(stmt);
            formatIfStatement(ifStmt);
            break;
        }
        case NodeType::WHILE_STATEMENT:
        {
            auto whileStmt = static_cast<const WhileStatement*>(stmt);
            formatWhileStatement(whileStmt);
            break;
        }
        case NodeType::RANGE_FOR_STATEMENT:
        {
            auto forStmt = static_cast<const RangeForStatement*>(stmt);
            formatRangeForStatement(forStmt);
            break;
        }
        case NodeType::REPEAT_STATEMENT:
        {
            auto repeatStmt = static_cast<const RepeatStatement*>(stmt);
            formatRepeatStatement(repeatStmt);
            break;
        }
        case NodeType::BLOCK_STATEMENT:
        {
            auto blockStmt = static_cast<const BlockStatement*>(stmt);
            formatBlockStatement(blockStmt);
            break;
        }
        default:
        {
            // 분기가 없는 문장을 조용히 빼면 fmt 가 원본을 덮어쓸 때 코드가 사라진다.
            markUnsupported(stmt);
            break;
        }
    }
}

void Formatter::markUnsupported(const Node* node)
{
    std::string entry = nodeTypeToString(node->type());
    if (node->location().isValid())
    {
        entry += " (줄 " + std::to_string(node->location().line) + ")";
    }
    unsupported_.push_back(entry);
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
    if (auto thenBlock = node_cast<BlockStatement>(stmt->thenBranch()))
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
        if (auto elseBlock = node_cast<BlockStatement>(stmt->elseBranch()))
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
    if (auto bodyBlock = node_cast<BlockStatement>(stmt->body()))
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
    if (auto bodyBlock = node_cast<BlockStatement>(stmt->body()))
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
    if (auto bodyBlock = node_cast<BlockStatement>(stmt->body()))
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

    switch (expr->type())
    {
        case NodeType::BINARY_EXPRESSION:
        {
            auto binExpr = static_cast<const BinaryExpression*>(expr);
            formatBinaryExpression(binExpr);
            break;
        }
        case NodeType::UNARY_EXPRESSION:
        {
            auto unaryExpr = static_cast<const UnaryExpression*>(expr);
            formatUnaryExpression(unaryExpr);
            break;
        }
        case NodeType::CALL_EXPRESSION:
        {
            auto callExpr = static_cast<const CallExpression*>(expr);
            formatCallExpression(callExpr);
            break;
        }
        case NodeType::INDEX_EXPRESSION:
        {
            auto indexExpr = static_cast<const IndexExpression*>(expr);
            formatIndexExpression(indexExpr);
            break;
        }
        case NodeType::ARRAY_LITERAL:
        {
            auto arrayLit = static_cast<const ArrayLiteral*>(expr);
            formatArrayLiteral(arrayLit);
            break;
        }
        case NodeType::FUNCTION_LITERAL:
        {
            auto funcLit = static_cast<const FunctionLiteral*>(expr);
            formatFunctionLiteral(funcLit);
            break;
        }
        case NodeType::IDENTIFIER:
        {
            auto ident = static_cast<const Identifier*>(expr);
            formatIdentifier(ident);
            break;
        }
        case NodeType::INTEGER_LITERAL:
        {
            auto intLit = static_cast<const IntegerLiteral*>(expr);
            formatIntegerLiteral(intLit);
            break;
        }
        case NodeType::FLOAT_LITERAL:
        {
            auto floatLit = static_cast<const FloatLiteral*>(expr);
            formatFloatLiteral(floatLit);
            break;
        }
        case NodeType::STRING_LITERAL:
        {
            auto strLit = static_cast<const StringLiteral*>(expr);
            formatStringLiteral(strLit);
            break;
        }
        case NodeType::BOOLEAN_LITERAL:
        {
            auto boolLit = static_cast<const BooleanLiteral*>(expr);
            formatBooleanLiteral(boolLit);
            break;
        }
        default:
        {
            markUnsupported(expr);
            break;
        }
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
    if (auto body = node_cast<BlockStatement>(expr->body()))
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

bool Formatter::loadConfig(const std::string& filepath)
{
    json configJson;
    if (!config::ConfigLoader::loadFromFile(filepath, configJson))
    {
        return false;
    }

    return FormatterConfig::loadFromJson(configJson, options_);
}

bool Formatter::loadConfigFromString(const std::string& jsonString)
{
    json configJson;
    if (!config::ConfigLoader::loadFromString(jsonString, configJson))
    {
        return false;
    }

    return FormatterConfig::loadFromJson(configJson, options_);
}

bool Formatter::loadConfigFromCurrentDir()
{
    std::string configPath = config::ConfigLoader::findConfigFile(".ksjfmt.json");
    if (configPath.empty())
    {
        return false;  // 설정 파일을 찾지 못함
    }

    return loadConfig(configPath);
}

} // namespace formatter
} // namespace kingsejong
