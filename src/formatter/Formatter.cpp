/**
 * @file Formatter.cpp
 * @brief Formatter 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "formatter/Formatter.h"
#include "formatter/FormatterConfig.h"
#include "config/ConfigLoader.h"
#include "ast/Visitor.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include <algorithm>
#include <charconv>
#include <sstream>

namespace kingsejong {
namespace formatter {

using json = nlohmann::json;

using namespace ast;

namespace {

// 파서(Parser::Precedence)와 같은 순서의 결합 세기. 클수록 강하게 묶인다
constexpr int kLowest = 0;
constexpr int kAssign = 1;
constexpr int kRange = 2;
constexpr int kOr = 3;
constexpr int kAnd = 4;
constexpr int kEquals = 5;
constexpr int kLessGreater = 6;
constexpr int kSum = 7;
constexpr int kProduct = 8;
constexpr int kPrefix = 9;
constexpr int kCall = 10;
constexpr int kIndex = 11;
constexpr int kAtom = 12;

int binaryPrecedence(const std::string& op)
{
    if (op == "=") return kAssign;
    if (op == "||" || op == "또는") return kOr;
    if (op == "&&" || op == "그리고") return kAnd;
    if (op == "==" || op == "!=") return kEquals;
    if (op == "<" || op == ">" || op == "<=" || op == ">=") return kLessGreater;
    if (op == "+" || op == "-") return kSum;
    if (op == "*" || op == "/" || op == "%") return kProduct;
    return kLowest;
}

/// 식이 피연산자 자리에서 얼마나 강하게 묶이는지
int expressionPrecedence(const Expression* expr)
{
    switch (expr->type())
    {
        case NodeType::BINARY_EXPRESSION:
            return binaryPrecedence(static_cast<const BinaryExpression*>(expr)->op());
        case NodeType::UNARY_EXPRESSION:
        case NodeType::AWAIT_EXPRESSION:
            return kPrefix;
        case NodeType::RANGE_EXPRESSION:
            return kRange;
        case NodeType::JOSA_EXPRESSION:
        case NodeType::MATCH_EXPRESSION:
            return kCall;
        case NodeType::CALL_EXPRESSION:
            return kCall;
        case NodeType::INDEX_EXPRESSION:
        case NodeType::MEMBER_ACCESS_EXPRESSION:
            return kIndex;
        case NodeType::FUNCTION_LITERAL:
        case NodeType::ASYNC_FUNCTION_LITERAL:
            return kLowest;  // 호출·인덱스의 대상이 되면 괄호로 감싼다
        default:
            return kAtom;
    }
}

std::string escapeString(const std::string& value)
{
    std::string out;
    for (char c : value)
    {
        switch (c)
        {
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            case '\r': out += "\\r"; break;
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            default:   out += c; break;
        }
    }
    return out;
}

/// 다시 읽었을 때 같은 double 이 되는 가장 짧은 표기. 정수처럼 보이면 ".0" 을 붙여 실수로 남긴다
std::string formatDouble(double value)
{
    char buffer[64];
    auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
    std::string text(buffer, result.ptr);
    if (text.find_first_of(".eEn") == std::string::npos)
    {
        text += ".0";
    }
    return text;
}

/// 받침이 있는 글자로 끝나면 true (한글이 아니면 받침 없음으로 본다)
bool endsWithBatchim(const std::string& word)
{
    if (word.size() < 3)
    {
        return false;
    }
    auto b0 = static_cast<unsigned char>(word[word.size() - 3]);
    auto b1 = static_cast<unsigned char>(word[word.size() - 2]);
    auto b2 = static_cast<unsigned char>(word[word.size() - 1]);
    if ((b0 & 0xF0) != 0xE0)
    {
        return false;
    }
    char32_t cp = ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
    if (cp < 0xAC00 || cp > 0xD7A3)
    {
        return false;
    }
    return (cp - 0xAC00) % 28 != 0;
}

/// 이름이 키워드가 아닌 식별자 하나로 읽히면 true ("함수 이름(...)" 선언에는 식별자만 올 수 있다)
bool isPlainIdentifier(const std::string& name)
{
    lexer::Lexer lexer(name);
    auto first = lexer.nextToken();
    return first.type == lexer::TokenType::IDENTIFIER && first.literal == name &&
           lexer.nextToken().type == lexer::TokenType::EOF_TOKEN;
}

std::string accessName(AccessModifier access)
{
    return access == AccessModifier::PRIVATE ? "비공개" : "공개";
}

} // namespace

// ============================================================================
// 진입점
// ============================================================================

std::string Formatter::format(const Program* program)
{
    if (!program) return "";

    output_.str("");
    output_.clear();
    currentIndent_ = 0;
    unsupported_.clear();
    hasSource_ = false;
    sourceLines_.clear();
    comments_.clear();
    nextComment_ = 0;
    pendingTrailing_.clear();
    atBlockStart_ = true;

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

std::string Formatter::format(const Program* program, const std::string& source)
{
    if (!program) return "";

    output_.str("");
    output_.clear();
    currentIndent_ = 0;
    unsupported_.clear();
    hasSource_ = true;
    sourceLines_.clear();
    {
        std::istringstream in(source);
        std::string line;
        while (std::getline(in, line))
        {
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }
            sourceLines_.push_back(line);
        }
    }
    comments_ = extractComments(source);
    nextComment_ = 0;
    pendingTrailing_.clear();
    atBlockStart_ = true;

    for (const auto& stmt : program->statements())
    {
        formatStatement(stmt.get());
    }
    flushCommentsBefore(static_cast<int>(sourceLines_.size()) + 1);

    return output_.str();
}

// ============================================================================
// 주석과 빈 줄
// ============================================================================

std::vector<SourceComment> Formatter::extractComments(const std::string& source)
{
    std::vector<SourceComment> comments;
    int line = 1;
    bool codeOnLine = false;
    char quote = 0;  // 문자열 안이면 여는 따옴표
    for (size_t i = 0; i < source.size(); ++i)
    {
        char c = source[i];
        if (c == '\n')
        {
            ++line;
            codeOnLine = false;
            continue;  // 렉서처럼 줄이 바뀌어도 문자열은 이어진다
        }
        if (quote)
        {
            if (c == '\\' && i + 1 < source.size() && source[i + 1] != '\n')
            {
                ++i;
            }
            else if (c == quote)
            {
                quote = 0;
            }
            continue;
        }
        if (c == '"' || c == '\'')
        {
            quote = c;
            codeOnLine = true;
            continue;
        }
        // 렉서는 '#' 과 '//' 를 둘 다 한 줄 주석으로 건너뛴다
        if (c == '#' || (c == '/' && i + 1 < source.size() && source[i + 1] == '/'))
        {
            size_t end = source.find('\n', i);
            std::string text = source.substr(i, end == std::string::npos ? std::string::npos : end - i);
            while (!text.empty() && (text.back() == ' ' || text.back() == '\t' || text.back() == '\r'))
            {
                text.pop_back();
            }
            comments.push_back({line, text, codeOnLine});
            if (end == std::string::npos)
            {
                break;
            }
            i = end - 1;  // 다음 반복에서 '\n' 을 처리한다
            continue;
        }
        if (c != ' ' && c != '\t' && c != '\r')
        {
            codeOnLine = true;
        }
    }
    return comments;
}

bool Formatter::sourceLineBlank(int line) const
{
    if (line < 1 || line > static_cast<int>(sourceLines_.size()))
    {
        return false;
    }
    const std::string& text = sourceLines_[static_cast<size_t>(line - 1)];
    return text.find_first_not_of(" \t") == std::string::npos;
}

void Formatter::flushCommentsBefore(int line)
{
    while (nextComment_ < comments_.size() && comments_[nextComment_].line < line)
    {
        const auto& comment = comments_[nextComment_];
        if (!comment.trailing && !atBlockStart_ && sourceLineBlank(comment.line - 1))
        {
            writeLine();
        }
        writeIndent();
        writeLine(comment.text);
        ++nextComment_;
    }
}

void Formatter::beginStatement(int line)
{
    if (!hasSource_ || line <= 0)
    {
        return;
    }
    // 주석 묶음 앞의 빈 줄과, 주석(또는 앞 문장)과 이 문장 사이의 빈 줄을 하나씩 남긴다
    flushCommentsBefore(line);
    if (!atBlockStart_ && sourceLineBlank(line - 1))
    {
        writeLine();
    }
    if (nextComment_ < comments_.size() && comments_[nextComment_].line == line &&
        comments_[nextComment_].trailing)
    {
        pendingTrailing_ = comments_[nextComment_].text;
        ++nextComment_;
    }
}

int Formatter::blockEndLine(const Node* block) const
{
    // 블록의 "{" 부터 짝이 맞는 "}" 까지 소스를 훑는다 (문자열·주석 안의 괄호는 건너뛴다)
    const auto& loc = block->location();
    if (!hasSource_ || loc.line <= 0 || loc.line > static_cast<int>(sourceLines_.size()))
    {
        return 0;
    }
    int depth = 0;
    char quote = 0;
    for (size_t l = static_cast<size_t>(loc.line - 1); l < sourceLines_.size(); ++l)
    {
        const std::string& text = sourceLines_[l];
        size_t start = (l == static_cast<size_t>(loc.line - 1) && loc.column > 0)
            ? std::min(static_cast<size_t>(loc.column - 1), text.size()) : 0;
        for (size_t i = start; i < text.size(); ++i)
        {
            char c = text[i];
            if (quote)
            {
                if (c == '\\') ++i;
                else if (c == quote) quote = 0;
                continue;
            }
            if (c == '"' || c == '\'') { quote = c; continue; }
            if (c == '#' || (c == '/' && i + 1 < text.size() && text[i + 1] == '/')) break;
            if (c == '{') ++depth;
            else if (c == '}' && --depth == 0) return static_cast<int>(l) + 1;
        }
    }
    return 0;
}

// ============================================================================
// 문장 포맷팅
// ============================================================================

void Formatter::formatStatement(const Statement* stmt)
{
    if (!stmt) return;

    beginStatement(stmt->location().line);

    switch (stmt->type())
    {
        case NodeType::VAR_DECLARATION:
            formatVarDeclaration(static_cast<const VarDeclaration*>(stmt));
            break;
        case NodeType::ASSIGNMENT_STATEMENT:
            formatAssignmentStatement(static_cast<const AssignmentStatement*>(stmt));
            break;
        case NodeType::EXPRESSION_STATEMENT:
            formatExpressionStatement(static_cast<const ExpressionStatement*>(stmt));
            break;
        case NodeType::RETURN_STATEMENT:
            formatReturnStatement(static_cast<const ReturnStatement*>(stmt));
            break;
        case NodeType::IF_STATEMENT:
            formatIfStatement(static_cast<const IfStatement*>(stmt));
            break;
        case NodeType::WHILE_STATEMENT:
            formatWhileStatement(static_cast<const WhileStatement*>(stmt));
            break;
        case NodeType::RANGE_FOR_STATEMENT:
            formatRangeForStatement(static_cast<const RangeForStatement*>(stmt));
            break;
        case NodeType::REPEAT_STATEMENT:
            formatRepeatStatement(static_cast<const RepeatStatement*>(stmt));
            break;
        case NodeType::BLOCK_STATEMENT:
            formatBlockStatement(static_cast<const BlockStatement*>(stmt));
            break;
        case NodeType::IMPORT_STATEMENT:
            formatImportStatement(static_cast<const ImportStatement*>(stmt));
            break;
        case NodeType::TRY_STATEMENT:
            formatTryStatement(static_cast<const TryStatement*>(stmt));
            break;
        case NodeType::THROW_STATEMENT:
            formatThrowStatement(static_cast<const ThrowStatement*>(stmt));
            break;
        case NodeType::CLASS_STATEMENT:
            formatClassStatement(static_cast<const ClassStatement*>(stmt));
            break;
        default:
            // 분기가 없는 문장을 조용히 빼면 fmt 가 원본을 덮어쓸 때 코드가 사라진다.
            markUnsupported(stmt);
            break;
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

void Formatter::formatBlockBody(const BlockStatement* block)
{
    // 원본에서 한 줄이던 짧은 블록(함수(x) { 반환 x * 2 })은 한 줄로 둔다
    if (hasSource_ && block && block->statements().size() <= 1 &&
        blockEndLine(block) == block->location().line)
    {
        int line = block->location().line;
        bool commentOnLine = nextComment_ < comments_.size() && comments_[nextComment_].line == line;
        if (!commentOnLine)
        {
            if (block->statements().empty())
            {
                write("{}");
                return;
            }
            // 문장 하나를 들여쓰기 없이 따로 써 보고, 한 줄이면 그대로 끼운다
            std::string saved = output_.str();
            int savedIndent = currentIndent_;
            bool savedStart = atBlockStart_;
            // 바깥 문장의 끝 주석은 바깥 문장의 줄 끝에 붙어야 하므로 잠시 빼 둔다
            std::string savedTrailing = pendingTrailing_;
            pendingTrailing_.clear();
            output_.str("");
            currentIndent_ = 0;
            atBlockStart_ = true;
            formatStatement(block->statements()[0].get());
            std::string inner = output_.str();
            output_.str("");
            output_ << saved;
            output_.seekp(0, std::ios::end);
            currentIndent_ = savedIndent;
            atBlockStart_ = savedStart;
            pendingTrailing_ = savedTrailing;
            if (!inner.empty() && inner.back() == '\n')
            {
                inner.pop_back();
            }
            if (inner.find('\n') == std::string::npos)
            {
                write("{ " + inner + " }");
                return;
            }
            // 한 줄이 아니면 아래에서 여러 줄로 다시 쓴다 (따로 쓴 출력은 버렸다)
        }
    }

    writeLine("{");
    increaseIndent();
    if (block)
    {
        for (const auto& s : block->statements())
        {
            formatStatement(s.get());
        }
        // 블록 끝 "}" 앞에 있던 주석은 블록 안에 남긴다
        if (int end = blockEndLine(block); end > 0)
        {
            flushCommentsBefore(end);
        }
    }
    decreaseIndent();
    writeIndent();
    write("}");
}

void Formatter::formatVarDeclaration(const VarDeclaration* stmt)
{
    writeIndent();

    if (!stmt->typeName().empty())
    {
        write(stmt->typeName());
        write(" ");
    }
    write(stmt->varName());

    if (stmt->initializer())
    {
        write(options_.spaceAroundOperators ? " = " : "=");
        formatExpression(stmt->initializer());
    }

    writeLine();
}

void Formatter::formatAssignmentStatement(const AssignmentStatement* stmt)
{
    writeIndent();

    // "함수 이름(...)" / "비동기 함수 이름(...)" 선언은 같은 대입문으로 파싱되므로 선언 형태로 쓴다
    // 원래 선언 형태로 쓴 함수만 선언 형태로 되돌린다 (키워드 이름은 선언 형태로 쓸 수 없다)
    auto declarable = stmt->declarationForm() && isPlainIdentifier(stmt->varName());
    if (auto func = node_cast<FunctionLiteral>(stmt->value()); func && declarable)
    {
        write("함수 " + stmt->varName());
        writeParameters(func->parameters(), func->parameterTypes());
        write(options_.spaceBeforeBrace ? " " : "");
        formatBlockBody(node_cast<BlockStatement>(func->body()));
        writeLine();
        return;
    }
    if (auto func = node_cast<AsyncFunctionLiteral>(stmt->value()); func && declarable)
    {
        write("비동기 함수 " + stmt->varName());
        writeParameters(func->parameters(), func->parameterTypes());
        write(options_.spaceBeforeBrace ? " " : "");
        formatBlockBody(node_cast<BlockStatement>(func->body()));
        writeLine();
        return;
    }

    write(stmt->varName());
    write(options_.spaceAroundOperators ? " = " : "=");
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
    const IfStatement* current = stmt;
    while (true)
    {
        // 조건 괄호는 원래 쓴 대로 둔다 ("만약 (x > 0)" / "만약 x > 0")
        write("만약");
        if (current->conditionParenthesized())
        {
            write(options_.spaceBeforeBrace ? " (" : "(");
            formatExpression(current->condition());
            write(options_.spaceBeforeBrace ? ") " : ")");
        }
        else
        {
            write(" ");
            formatExpression(current->condition());
            write(options_.spaceBeforeBrace ? " " : "");
        }
        formatBlockBody(current->thenBranch());

        const BlockStatement* elseBlock = current->elseBranch();
        if (!elseBlock)
        {
            break;
        }
        write(options_.spaceBeforeBrace ? " 아니면 " : "아니면");

        // "아니면 만약" 은 만약 문 하나를 담은 아니면 블록으로 파싱된다 (원래 그렇게 쓴 경우만 되돌린다)
        if (current->elseIf() && elseBlock->statements().size() == 1)
        {
            if (auto nested = node_cast<IfStatement>(elseBlock->statements()[0].get()))
            {
                beginStatement(nested->location().line);
                current = nested;
                continue;
            }
        }
        formatBlockBody(elseBlock);
        break;
    }
    writeLine();
}

void Formatter::formatWhileStatement(const WhileStatement* stmt)
{
    writeIndent();
    write(stmt->keyword());
    if (stmt->conditionParenthesized())
    {
        write(options_.spaceBeforeBrace ? " (" : "(");
        formatExpression(stmt->condition());
        write(options_.spaceBeforeBrace ? ") " : ")");
    }
    else
    {
        // "반복 (a) && (b) {": 조건이 괄호로 시작하지만 전체를 감싸지는 않았다
        write(" ");
        formatExpression(stmt->condition());
        write(options_.spaceBeforeBrace ? " " : "");
    }
    formatBlockBody(stmt->body());
    writeLine();
}

void Formatter::formatRangeForStatement(const RangeForStatement* stmt)
{
    // i가 1부터 10까지 { ... } / i가 0부터 n 미만 반복한다 { ... }
    // 끝 키워드와 반복 키워드는 원래 쓴 대로 둔다. "까지" 만 앞 식에 붙여 쓴다.
    writeIndent();
    write(stmt->varName());
    write(endsWithBatchim(stmt->varName()) ? "이 " : "가 ");
    formatOperand(stmt->start(), kRange + 1, false);
    write("부터 ");
    formatOperand(stmt->end(), kRange + 1, false);
    write(stmt->endWord() == "까지" ? "까지" : " " + stmt->endWord());
    if (!stmt->loopWord().empty())
    {
        write(" " + stmt->loopWord());
    }
    write(options_.spaceBeforeBrace ? " " : "");
    formatBlockBody(stmt->body());
    writeLine();
}

void Formatter::formatRepeatStatement(const RepeatStatement* stmt)
{
    // 10번 반복한다 { ... }
    writeIndent();
    formatOperand(stmt->count(), kPrefix, false);
    write("번 반복한다");
    write(options_.spaceBeforeBrace ? " " : "");
    formatBlockBody(stmt->body());
    writeLine();
}

void Formatter::formatBlockStatement(const BlockStatement* stmt)
{
    writeIndent();
    formatBlockBody(stmt);
    writeLine();
}

void Formatter::formatImportStatement(const ImportStatement* stmt)
{
    writeIndent();
    write("가져오기 \"" + escapeString(stmt->modulePath()) + "\"");
    writeLine();
}

void Formatter::formatTryStatement(const TryStatement* stmt)
{
    writeIndent();
    write("시도");
    write(options_.spaceBeforeBrace ? " " : "");
    formatBlockBody(stmt->tryBlock());
    for (const auto& clause : stmt->catchClauses())
    {
        write(" 오류 (" + clause->errorVarName() + ")");
        write(options_.spaceBeforeBrace ? " " : "");
        formatBlockBody(clause->body());
    }
    if (stmt->finallyBlock())
    {
        write(" 마지막");
        write(options_.spaceBeforeBrace ? " " : "");
        formatBlockBody(stmt->finallyBlock());
    }
    writeLine();
}

void Formatter::formatThrowStatement(const ThrowStatement* stmt)
{
    writeIndent();
    write("던지다 ");
    formatExpression(stmt->value());
    writeLine();
}

void Formatter::formatClassStatement(const ClassStatement* stmt)
{
    writeIndent();
    write("클래스 " + stmt->className());
    if (!stmt->superClass().empty())
    {
        write(" 상속 " + stmt->superClass());
    }
    writeLine(" {");
    increaseIndent();

    for (const auto& field : stmt->fields())
    {
        beginStatement(field->location().line);
        writeIndent();
        write(accessName(field->access()) + " " + field->typeName() + " " + field->fieldName());
        if (field->initializer())
        {
            write(options_.spaceAroundOperators ? " = " : "=");
            formatExpression(field->initializer());
        }
        writeLine();
    }

    if (const auto* ctor = stmt->constructor())
    {
        beginStatement(ctor->location().line);
        writeIndent();
        write("생성자");
        writeParameters(ctor->parameters());
        write(options_.spaceBeforeBrace ? " " : "");
        formatBlockBody(ctor->body());
        writeLine();
    }

    for (const auto& method : stmt->methods())
    {
        beginStatement(method->location().line);
        writeIndent();
        write(accessName(method->access()) + " 함수 " + method->methodName());
        writeParameters(method->parameters());
        write(options_.spaceBeforeBrace ? " " : "");
        formatBlockBody(method->body());
        writeLine();
    }

    if (int end = blockEndLine(stmt); end > 0)
    {
        flushCommentsBefore(end);
    }
    decreaseIndent();
    writeIndent();
    writeLine("}");
}

void Formatter::writeParameters(const std::vector<std::string>& names, const std::vector<std::string>& types)
{
    write("(");
    for (size_t i = 0; i < names.size(); ++i)
    {
        if (i > 0) write(options_.spaceAfterComma ? ", " : ",");
        if (i < types.size() && !types[i].empty())
        {
            write(types[i] + " ");
        }
        write(names[i]);
    }
    write(")");
}

void Formatter::writeParameters(const std::vector<ast::Parameter>& params)
{
    write("(");
    for (size_t i = 0; i < params.size(); ++i)
    {
        if (i > 0) write(options_.spaceAfterComma ? ", " : ",");
        if (!params[i].typeName.empty())
        {
            write(params[i].typeName + " ");
        }
        write(params[i].name);
    }
    write(")");
}

// ============================================================================
// 표현식 포맷팅
// ============================================================================

void Formatter::formatOperand(const Expression* expr, int parentPrecedence, bool rightSide)
{
    if (!expr) return;
    int own = expressionPrecedence(expr);
    // 왼쪽은 같은 세기까지 괄호 없이(왼쪽 결합), 오른쪽은 같은 세기면 괄호가 필요하다.
    // 원래 괄호가 있던 식은 formatExpression 이 괄호를 쓴다
    bool needParens = !expr->parenthesized() &&
                      (rightSide ? own <= parentPrecedence : own < parentPrecedence);
    if (needParens) write("(");
    formatExpression(expr);
    if (needParens) write(")");
}

void Formatter::formatExpression(const Expression* expr)
{
    if (!expr) return;

    // 원래 괄호로 감싼 식은 의미에 필요 없어도 괄호를 남긴다 (작성자가 묶어 둔 의도)
    if (expr->parenthesized())
    {
        write("(");
    }
    formatExpressionBody(expr);
    if (expr->parenthesized())
    {
        write(")");
    }
}

void Formatter::formatExpressionBody(const Expression* expr)
{
    switch (expr->type())
    {
        case NodeType::BINARY_EXPRESSION:
            formatBinaryExpression(static_cast<const BinaryExpression*>(expr));
            break;
        case NodeType::UNARY_EXPRESSION:
            formatUnaryExpression(static_cast<const UnaryExpression*>(expr));
            break;
        case NodeType::CALL_EXPRESSION:
            formatCallExpression(static_cast<const CallExpression*>(expr));
            break;
        case NodeType::INDEX_EXPRESSION:
            formatIndexExpression(static_cast<const IndexExpression*>(expr));
            break;
        case NodeType::ARRAY_LITERAL:
            formatArrayLiteral(static_cast<const ArrayLiteral*>(expr));
            break;
        case NodeType::DICTIONARY_LITERAL:
            formatDictionaryLiteral(static_cast<const DictionaryLiteral*>(expr));
            break;
        case NodeType::FUNCTION_LITERAL:
            formatFunctionLiteral(static_cast<const FunctionLiteral*>(expr));
            break;
        case NodeType::ASYNC_FUNCTION_LITERAL:
            formatAsyncFunctionLiteral(static_cast<const AsyncFunctionLiteral*>(expr));
            break;
        case NodeType::AWAIT_EXPRESSION:
            write("대기 ");
            formatOperand(static_cast<const AwaitExpression*>(expr)->argument(), kPrefix, false);
            break;
        case NodeType::JOSA_EXPRESSION:
            formatJosaExpression(static_cast<const JosaExpression*>(expr));
            break;
        case NodeType::RANGE_EXPRESSION:
            formatRangeExpression(static_cast<const RangeExpression*>(expr));
            break;
        case NodeType::MATCH_EXPRESSION:
            formatMatchExpression(static_cast<const MatchExpression*>(expr));
            break;
        case NodeType::MEMBER_ACCESS_EXPRESSION:
        {
            auto member = static_cast<const MemberAccessExpression*>(expr);
            formatOperand(member->object(), kCall, false);
            write("." + member->memberName());
            break;
        }
        case NodeType::THIS_EXPRESSION:
            write("자신");
            break;
        case NodeType::IDENTIFIER:
            formatIdentifier(static_cast<const Identifier*>(expr));
            break;
        case NodeType::INTEGER_LITERAL:
            formatIntegerLiteral(static_cast<const IntegerLiteral*>(expr));
            break;
        case NodeType::FLOAT_LITERAL:
            formatFloatLiteral(static_cast<const FloatLiteral*>(expr));
            break;
        case NodeType::STRING_LITERAL:
            formatStringLiteral(static_cast<const StringLiteral*>(expr));
            break;
        case NodeType::INTERPOLATED_STRING:
            formatInterpolatedString(static_cast<const InterpolatedString*>(expr));
            break;
        case NodeType::BOOLEAN_LITERAL:
            formatBooleanLiteral(static_cast<const BooleanLiteral*>(expr));
            break;
        default:
            markUnsupported(expr);
            break;
    }
}

void Formatter::formatBinaryExpression(const BinaryExpression* expr)
{
    int precedence = binaryPrecedence(expr->op());
    formatOperand(expr->left(), precedence, false);
    if (options_.spaceAroundOperators)
    {
        write(" " + expr->op() + " ");
    }
    else
    {
        write(expr->op());
    }
    formatOperand(expr->right(), precedence, true);
}

void Formatter::formatUnaryExpression(const UnaryExpression* expr)
{
    write(expr->op());
    formatOperand(expr->operand(), kPrefix, false);
}

void Formatter::formatCallExpression(const CallExpression* expr)
{
    formatOperand(expr->function(), kCall, false);
    write("(");
    const auto& args = expr->arguments();
    for (size_t i = 0; i < args.size(); ++i)
    {
        if (i > 0) write(options_.spaceAfterComma ? ", " : ",");
        formatExpression(args[i].get());
    }
    write(")");
}

void Formatter::formatIndexExpression(const IndexExpression* expr)
{
    formatOperand(expr->array(), kCall, false);
    write("[");
    formatExpression(expr->index());
    write("]");
}

namespace {

/// 요소가 원본에서 여러 줄에 걸쳐 있었으면 true (포매터도 한 줄에 하나씩 쓴다)
template <typename Items, typename LineOf>
bool spansLines(int openLine, const Items& items, LineOf lineOf)
{
    for (const auto& item : items)
    {
        int line = lineOf(item);
        if (line > 0 && openLine > 0 && line != openLine)
        {
            return true;
        }
    }
    return false;
}

} // namespace

void Formatter::formatArrayLiteral(const ArrayLiteral* expr)
{
    const auto& elements = expr->elements();
    bool multiline = hasSource_ && spansLines(expr->location().line, elements,
                                              [](const auto& e) { return e->location().line; });
    if (!multiline)
    {
        write("[");
        for (size_t i = 0; i < elements.size(); ++i)
        {
            if (i > 0) write(options_.spaceAfterComma ? ", " : ",");
            formatExpression(elements[i].get());
        }
        write("]");
        return;
    }

    writeLine("[");
    increaseIndent();
    for (size_t i = 0; i < elements.size(); ++i)
    {
        flushCommentsBefore(elements[i]->location().line);
        writeIndent();
        formatExpression(elements[i].get());
        writeLine(i + 1 < elements.size() ? "," : "");
    }
    decreaseIndent();
    writeIndent();
    write("]");
}

void Formatter::formatDictionaryLiteral(const DictionaryLiteral* expr)
{
    const auto& pairs = expr->pairs();
    bool multiline = hasSource_ && spansLines(expr->location().line, pairs,
                                              [](const auto& p) { return p.first->location().line; });
    if (!multiline)
    {
        write("{");
        for (size_t i = 0; i < pairs.size(); ++i)
        {
            if (i > 0) write(options_.spaceAfterComma ? ", " : ",");
            formatExpression(pairs[i].first.get());
            write(": ");
            formatExpression(pairs[i].second.get());
        }
        write("}");
        return;
    }

    writeLine("{");
    increaseIndent();
    for (size_t i = 0; i < pairs.size(); ++i)
    {
        flushCommentsBefore(pairs[i].first->location().line);
        writeIndent();
        formatExpression(pairs[i].first.get());
        write(": ");
        formatExpression(pairs[i].second.get());
        writeLine(i + 1 < pairs.size() ? "," : "");
    }
    decreaseIndent();
    writeIndent();
    write("}");
}

void Formatter::formatFunctionLiteral(const FunctionLiteral* expr)
{
    write("함수");
    writeParameters(expr->parameters(), expr->parameterTypes());
    write(options_.spaceBeforeBrace ? " " : "");
    formatBlockBody(node_cast<BlockStatement>(expr->body()));
}

void Formatter::formatAsyncFunctionLiteral(const AsyncFunctionLiteral* expr)
{
    write("비동기 함수");
    writeParameters(expr->parameters(), expr->parameterTypes());
    write(options_.spaceBeforeBrace ? " " : "");
    formatBlockBody(node_cast<BlockStatement>(expr->body()));
}

void Formatter::formatJosaExpression(const JosaExpression* expr)
{
    // 목적어에 조사를 바로 붙인다: 숫자들을 정렬한다. 앞 단계도 조사식이면 괄호 없이 잇는다
    if (node_cast<JosaExpression>(expr->object()))
    {
        formatExpression(expr->object());
    }
    else
    {
        formatOperand(expr->object(), kCall, false);
    }
    write(expr->particle());
    write(" ");
    formatOperand(expr->method(), kCall, false);
}

void Formatter::formatRangeExpression(const RangeExpression* expr)
{
    // 2부터 5까지, 0초과 10미만, 1 이상 9 이하, 1부터 9까지 2씩 (키워드는 원래 쓴 대로)
    formatOperand(expr->start(), kRange + 1, false);
    write(expr->startWord() == "이상" ? " 이상 " : expr->startWord() + " ");
    formatOperand(expr->end(), kRange + 1, false);
    write(expr->endWord() == "이상" || expr->endWord() == "이하" ? " " + expr->endWord() : expr->endWord());
    if (expr->hasStep())
    {
        write(" ");
        formatOperand(expr->step(), kSum, false);
        write("씩");
    }
}

void Formatter::formatMatchExpression(const MatchExpression* expr)
{
    formatOperand(expr->value(), kCall, false);
    writeLine(" 에 대해 {");
    increaseIndent();
    for (const auto& matchCase : expr->cases())
    {
        writeIndent();
        formatPattern(matchCase.pattern());
        if (matchCase.guard())
        {
            write(" when ");
            formatExpression(matchCase.guard());
        }
        write(" -> ");
        formatExpression(matchCase.body());
        writeLine();
    }
    decreaseIndent();
    writeIndent();
    write("}");
}

void Formatter::formatPattern(const Pattern* pattern)
{
    if (!pattern) return;
    switch (pattern->type())
    {
        case NodeType::WILDCARD_PATTERN:
            write("_");
            break;
        case NodeType::LITERAL_PATTERN:
            formatExpression(static_cast<const LiteralPattern*>(pattern)->value());
            break;
        case NodeType::BINDING_PATTERN:
            write(static_cast<const BindingPattern*>(pattern)->name());
            break;
        case NodeType::ARRAY_PATTERN:
        {
            write("[");
            const auto& elements = static_cast<const ArrayPattern*>(pattern)->elements();
            for (size_t i = 0; i < elements.size(); ++i)
            {
                if (i > 0) write(options_.spaceAfterComma ? ", " : ",");
                formatPattern(elements[i].get());
            }
            write("]");
            break;
        }
        default:
            markUnsupported(pattern);
            break;
    }
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
    write(formatDouble(expr->value()));
}

void Formatter::formatStringLiteral(const StringLiteral* expr)
{
    write("\"" + escapeString(expr->value()) + "\"");
}

void Formatter::formatInterpolatedString(const InterpolatedString* expr)
{
    write("\"");
    const auto& parts = expr->parts();
    const auto& expressions = expr->expressions();
    for (size_t i = 0; i < parts.size(); ++i)
    {
        write(escapeString(parts[i]));
        if (i < expressions.size())
        {
            write("${");
            formatExpression(expressions[i].get());
            write("}");
        }
    }
    write("\"");
}

void Formatter::formatBooleanLiteral(const BooleanLiteral* expr)
{
    write(expr->value() ? "참" : "거짓");
}

// ============================================================================
// 검증
// ============================================================================

namespace {

/// 노드마다 종류와 값을 적고, 자식은 괄호로 묶어 구조를 남긴다
class FingerprintVisitor : public Visitor
{
public:
    std::string out;

    bool enter(Node& node) override
    {
        out += "(" + nodeTypeToString(node.type());
        switch (node.type())
        {
            case NodeType::IDENTIFIER:
                out += " " + static_cast<Identifier&>(node).name();
                break;
            case NodeType::INTEGER_LITERAL:
                out += " " + std::to_string(static_cast<IntegerLiteral&>(node).value());
                break;
            case NodeType::FLOAT_LITERAL:
                out += " " + formatDouble(static_cast<FloatLiteral&>(node).value());
                break;
            case NodeType::STRING_LITERAL:
                out += " \"" + escapeString(static_cast<StringLiteral&>(node).value()) + "\"";
                break;
            case NodeType::INTERPOLATED_STRING:
                for (const auto& part : static_cast<InterpolatedString&>(node).parts())
                    out += " \"" + escapeString(part) + "\"";
                break;
            case NodeType::BOOLEAN_LITERAL:
                out += static_cast<BooleanLiteral&>(node).value() ? " 참" : " 거짓";
                break;
            case NodeType::BINARY_EXPRESSION:
                out += " " + static_cast<BinaryExpression&>(node).op();
                break;
            case NodeType::UNARY_EXPRESSION:
                out += " " + static_cast<UnaryExpression&>(node).op();
                break;
            case NodeType::JOSA_EXPRESSION:
                out += " " + static_cast<JosaExpression&>(node).particle();
                break;
            case NodeType::RANGE_EXPRESSION:
            {
                auto& r = static_cast<RangeExpression&>(node);
                out += std::string(r.startInclusive() ? " [" : " (") + (r.endInclusive() ? "]" : ")");
                out += " " + r.startWord() + " " + r.endWord();
                break;
            }
            case NodeType::MEMBER_ACCESS_EXPRESSION:
                out += " ." + static_cast<MemberAccessExpression&>(node).memberName();
                break;
            case NodeType::FUNCTION_LITERAL:
            {
                auto& f = static_cast<FunctionLiteral&>(node);
                for (const auto& p : f.parameters()) out += " " + p;
                for (const auto& t : f.parameterTypes()) out += " :" + t;
                break;
            }
            case NodeType::ASYNC_FUNCTION_LITERAL:
            {
                auto& f = static_cast<AsyncFunctionLiteral&>(node);
                for (const auto& p : f.parameters()) out += " " + p;
                for (const auto& t : f.parameterTypes()) out += " :" + t;
                break;
            }
            case NodeType::VAR_DECLARATION:
            {
                auto& v = static_cast<VarDeclaration&>(node);
                out += " " + v.typeName() + " " + v.varName();
                break;
            }
            case NodeType::ASSIGNMENT_STATEMENT:
                out += " " + static_cast<AssignmentStatement&>(node).varName();
                break;
            case NodeType::RANGE_FOR_STATEMENT:
            {
                auto& r = static_cast<RangeForStatement&>(node);
                out += " " + r.varName() + (r.endInclusive() ? " ]" : " )");
                out += " " + r.endWord() + " " + r.loopWord();
                break;
            }
            case NodeType::IMPORT_STATEMENT:
                out += " " + static_cast<ImportStatement&>(node).modulePath();
                break;
            case NodeType::TRY_STATEMENT:
            {
                auto& t = static_cast<TryStatement&>(node);
                for (const auto& c : t.catchClauses()) out += " catch:" + c->errorVarName();
                out += t.finallyBlock() ? " finally" : "";
                break;
            }
            case NodeType::IF_STATEMENT:
            {
                auto& i = static_cast<IfStatement&>(node);
                out += i.elseBranch() ? " else" : "";
                out += i.conditionParenthesized() ? " ()" : "";
                break;
            }
            case NodeType::WHILE_STATEMENT:
            {
                auto& w = static_cast<WhileStatement&>(node);
                out += " " + w.keyword() + (w.conditionParenthesized() ? " ()" : "");
                break;
            }
            case NodeType::CLASS_STATEMENT:
            {
                auto& c = static_cast<ClassStatement&>(node);
                out += " " + c.className() + " : " + c.superClass();
                break;
            }
            case NodeType::FIELD_DECLARATION:
            {
                auto& f = static_cast<FieldDeclaration&>(node);
                out += " " + accessName(f.access()) + " " + f.typeName() + " " + f.fieldName();
                break;
            }
            case NodeType::METHOD_DECLARATION:
            {
                auto& m = static_cast<MethodDeclaration&>(node);
                out += " " + accessName(m.access()) + " " + m.methodName();
                for (const auto& p : m.parameters()) out += " " + p.typeName + ":" + p.name;
                break;
            }
            case NodeType::CONSTRUCTOR_DECLARATION:
                for (const auto& p : static_cast<ConstructorDeclaration&>(node).parameters())
                    out += " " + p.typeName + ":" + p.name;
                break;
            case NodeType::BINDING_PATTERN:
                out += " " + static_cast<BindingPattern&>(node).name();
                break;
            case NodeType::MATCH_EXPRESSION:
                for (const auto& c : static_cast<MatchExpression&>(node).cases())
                    out += c.guard() ? " g" : " -";
                break;
            default:
                break;
        }
        return true;
    }

    void leave(Node&) override { out += ")"; }
};

} // namespace

std::string Formatter::fingerprint(const Program* program)
{
    FingerprintVisitor visitor;
    walk(const_cast<Program*>(program), visitor);
    return visitor.out;
}

bool Formatter::verify(const Program* original, const std::string& originalSource,
                       const std::string& formatted, std::string& reason)
{
    lexer::Lexer lexer(formatted);
    parser::Parser parser(lexer);
    auto reparsed = parser.parseProgram();
    if (!parser.errors().empty() || !reparsed)
    {
        reason = "포맷 결과를 다시 파싱하지 못했습니다";
        if (!parser.errors().empty())
        {
            reason += ": " + parser.errors().front();
        }
        return false;
    }
    if (fingerprint(original) != fingerprint(reparsed.get()))
    {
        reason = "포맷 결과의 구문 트리가 원본과 다릅니다";
        return false;
    }
    auto before = extractComments(originalSource);
    auto after = extractComments(formatted);
    if (before.size() != after.size() ||
        !std::equal(before.begin(), before.end(), after.begin(),
                    [](const SourceComment& a, const SourceComment& b) { return a.text == b.text; }))
    {
        reason = "포맷 결과에서 주석이 사라지거나 순서가 바뀌었습니다";
        return false;
    }
    return true;
}

// ============================================================================
// 유틸리티
// ============================================================================

void Formatter::write(const std::string& text)
{
    output_ << text;
    if (!text.empty())
    {
        atBlockStart_ = false;
    }
}

void Formatter::writeLine(const std::string& text)
{
    output_ << text;
    if (!pendingTrailing_.empty())
    {
        // 원본에서 이 문장의 첫 줄 끝에 있던 주석
        output_ << "  " << pendingTrailing_;
        pendingTrailing_.clear();
    }
    output_ << "\n";
    if (!text.empty())
    {
        atBlockStart_ = text.back() == '{';
    }
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
