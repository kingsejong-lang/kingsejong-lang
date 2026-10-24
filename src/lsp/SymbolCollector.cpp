/**
 * @file SymbolCollector.cpp
 * @brief AST 를 걸어 LSP 심볼 테이블을 채운다
 */

#include "lsp/SymbolCollector.h"
#include "lsp/LspUtils.h"
#include <cctype>

namespace kingsejong {
namespace lsp {

namespace {

bool isIdentifierByte(unsigned char c)
{
    return std::isalnum(c) || c == '_' || c >= 0x80;
}

} // namespace

SymbolCollector::SymbolCollector(SymbolTable& table)
    : symbolTable_(&table)
{
}

void SymbolCollector::collect(const ast::Program* program, const std::string& uri, const std::string& content)
{
    if (!program || !symbolTable_)
    {
        return;
    }
    currentUri_ = uri;
    scopes_.assign(1, "global");
    lines_ = content.empty() ? std::vector<std::string>{} : splitLines(content);
    anonymousCount_ = 0;

    for (const auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }
}

// ============================================================================
// 위치
// ============================================================================

Location SymbolCollector::toLocation(int line, int byteColumn) const
{
    if (line >= 1 && static_cast<size_t>(line) <= lines_.size())
    {
        return Location(line, byteColumnToUtf16(lines_[static_cast<size_t>(line) - 1], byteColumn) + 1, currentUri_);
    }
    return Location(line, byteColumn, currentUri_);
}

Location SymbolCollector::locationOf(const ast::Node* node) const
{
    const auto& loc = node->location();
    return toLocation(loc.line, loc.column);
}

Location SymbolCollector::nameLocation(const ast::Node* node, const std::string& name) const
{
    const auto& loc = node->location();
    if (loc.line >= 1 && static_cast<size_t>(loc.line) <= lines_.size() && !name.empty())
    {
        const std::string& text = lines_[static_cast<size_t>(loc.line) - 1];
        size_t from = loc.column > 0 ? static_cast<size_t>(loc.column - 1) : 0;
        for (size_t pos = text.find(name, from); pos != std::string::npos; pos = text.find(name, pos + 1))
        {
            bool startOk = pos == 0 || !isIdentifierByte(static_cast<unsigned char>(text[pos - 1]));
            size_t after = pos + name.size();
            bool endOk = after >= text.size() || !isIdentifierByte(static_cast<unsigned char>(text[after]));
            if (startOk && endOk)
            {
                return toLocation(loc.line, static_cast<int>(pos) + 1);
            }
        }
    }
    return locationOf(node);
}

// ============================================================================
// 심볼 등록
// ============================================================================

void SymbolCollector::declare(const std::string& name, SymbolKind kind, const Location& location,
                              const std::string& typeInfo)
{
    Symbol symbol;
    symbol.name = name;
    symbol.kind = kind;
    symbol.definitionLocation = location;
    symbol.typeInfo = typeInfo;
    symbol.scope = currentScope();
    symbolTable_->addSymbol(symbol);
}

void SymbolCollector::reference(const std::string& name, const ast::Node* node)
{
    Location location = locationOf(node);
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it)
    {
        if (symbolTable_->addReference(name, *it, location))
        {
            return;
        }
    }
}

// ============================================================================
// 문장
// ============================================================================

void SymbolCollector::visitStatement(const ast::Statement* stmt)
{
    if (!stmt)
    {
        return;
    }

    switch (stmt->type())
    {
        case ast::NodeType::VAR_DECLARATION:
        {
            auto* decl = static_cast<const ast::VarDeclaration*>(stmt);
            const auto* init = decl->initializer();
            if (init && init->type() == ast::NodeType::FUNCTION_LITERAL)
            {
                auto* fn = static_cast<const ast::FunctionLiteral*>(init);
                visitFunction(decl, decl->varName(), fn->parameters(), fn->body());
            }
            else if (init && init->type() == ast::NodeType::ASYNC_FUNCTION_LITERAL)
            {
                auto* fn = static_cast<const ast::AsyncFunctionLiteral*>(init);
                visitFunction(decl, decl->varName(), fn->parameters(), fn->body());
            }
            else
            {
                // 초기식을 먼저 걷는다: "정수 x = x + 1" 의 오른쪽 x 는 바깥 x 다
                visitExpression(init);
                declare(decl->varName(), SymbolKind::Variable, nameLocation(decl, decl->varName()), decl->typeName());
            }
            break;
        }
        case ast::NodeType::ASSIGNMENT_STATEMENT:
        {
            auto* assign = static_cast<const ast::AssignmentStatement*>(stmt);
            const auto* value = assign->value();
            if (value && value->type() == ast::NodeType::FUNCTION_LITERAL)
            {
                auto* fn = static_cast<const ast::FunctionLiteral*>(value);
                visitFunction(assign, assign->varName(), fn->parameters(), fn->body());
            }
            else if (value && value->type() == ast::NodeType::ASYNC_FUNCTION_LITERAL)
            {
                auto* fn = static_cast<const ast::AsyncFunctionLiteral*>(value);
                visitFunction(assign, assign->varName(), fn->parameters(), fn->body());
            }
            else
            {
                visitExpression(value);
                // 보이는 변수가 있으면 그 변수에 쓰는 것이고, 없으면 여기서 새로 만드는 것이다
                bool visible = false;
                for (auto it = scopes_.rbegin(); it != scopes_.rend() && !visible; ++it)
                {
                    visible = symbolTable_->findSymbolInScope(assign->varName(), *it).has_value();
                }
                if (visible)
                {
                    reference(assign->varName(), assign);
                }
                else
                {
                    declare(assign->varName(), SymbolKind::Variable, nameLocation(assign, assign->varName()));
                }
            }
            break;
        }
        case ast::NodeType::EXPRESSION_STATEMENT:
            visitExpression(static_cast<const ast::ExpressionStatement*>(stmt)->expression());
            break;
        case ast::NodeType::RETURN_STATEMENT:
            visitExpression(static_cast<const ast::ReturnStatement*>(stmt)->returnValue());
            break;
        case ast::NodeType::IF_STATEMENT:
        {
            auto* ifStmt = static_cast<const ast::IfStatement*>(stmt);
            visitExpression(ifStmt->condition());
            visitBlock(ifStmt->thenBranch());
            visitBlock(ifStmt->elseBranch());
            break;
        }
        case ast::NodeType::WHILE_STATEMENT:
        {
            auto* w = static_cast<const ast::WhileStatement*>(stmt);
            visitExpression(w->condition());
            visitBlock(w->body());
            break;
        }
        case ast::NodeType::FOR_STATEMENT:
        {
            auto* f = static_cast<const ast::ForStatement*>(stmt);
            visitExpression(f->range());
            declare(f->iterator(), SymbolKind::Variable, nameLocation(f, f->iterator()));
            visitBlock(f->body());
            break;
        }
        case ast::NodeType::RANGE_FOR_STATEMENT:
        {
            auto* r = static_cast<const ast::RangeForStatement*>(stmt);
            visitExpression(r->start());
            visitExpression(r->end());
            declare(r->varName(), SymbolKind::Variable, nameLocation(r, r->varName()));
            visitBlock(r->body());
            break;
        }
        case ast::NodeType::REPEAT_STATEMENT:
        {
            auto* r = static_cast<const ast::RepeatStatement*>(stmt);
            visitExpression(r->count());
            visitBlock(r->body());
            break;
        }
        case ast::NodeType::BLOCK_STATEMENT:
            visitBlock(static_cast<const ast::BlockStatement*>(stmt));
            break;
        case ast::NodeType::TRY_STATEMENT:
        {
            auto* t = static_cast<const ast::TryStatement*>(stmt);
            visitBlock(t->tryBlock());
            for (const auto& clause : t->catchClauses())
            {
                if (!clause->errorVarName().empty())
                {
                    // CatchClause 는 AST 노드가 아니라 위치가 없다. 시도문 줄에서 이름을 찾는다.
                    declare(clause->errorVarName(), SymbolKind::Variable, nameLocation(t, clause->errorVarName()));
                }
                visitBlock(clause->body());
            }
            visitBlock(t->finallyBlock());
            break;
        }
        case ast::NodeType::THROW_STATEMENT:
            visitExpression(static_cast<const ast::ThrowStatement*>(stmt)->value());
            break;
        case ast::NodeType::CLASS_STATEMENT:
            visitClass(static_cast<const ast::ClassStatement*>(stmt));
            break;
        default:
            break;
    }
}

void SymbolCollector::visitBlock(const ast::BlockStatement* block)
{
    if (!block)
    {
        return;
    }
    for (const auto& stmt : block->statements())
    {
        visitStatement(stmt.get());
    }
}

void SymbolCollector::visitFunction(const ast::Node* node, const std::string& name,
                                    const std::vector<std::string>& parameters, const ast::Statement* body)
{
    Location defLocation = nameLocation(node, name);
    declare(name, SymbolKind::Function, defLocation, "함수");

    scopes_.push_back("함수:" + name);
    for (const auto& param : parameters)
    {
        // 매개변수는 함수 이름 뒤 같은 줄에 있다
        Location paramLocation = defLocation;
        if (defLocation.line >= 1 && static_cast<size_t>(defLocation.line) <= lines_.size())
        {
            const std::string& text = lines_[static_cast<size_t>(defLocation.line) - 1];
            size_t from = text.find(name);
            size_t pos = from == std::string::npos ? std::string::npos : text.find(param, from + name.size());
            while (pos != std::string::npos)
            {
                bool startOk = pos == 0 || !isIdentifierByte(static_cast<unsigned char>(text[pos - 1]));
                size_t after = pos + param.size();
                bool endOk = after >= text.size() || !isIdentifierByte(static_cast<unsigned char>(text[after]));
                if (startOk && endOk)
                {
                    paramLocation = toLocation(defLocation.line, static_cast<int>(pos) + 1);
                    break;
                }
                pos = text.find(param, pos + 1);
            }
        }
        declare(param, SymbolKind::Parameter, paramLocation);
    }
    if (body && body->type() == ast::NodeType::BLOCK_STATEMENT)
    {
        visitBlock(static_cast<const ast::BlockStatement*>(body));
    }
    else
    {
        visitStatement(body);
    }
    scopes_.pop_back();
}

void SymbolCollector::visitClass(const ast::ClassStatement* cls)
{
    declare(cls->className(), SymbolKind::Class, nameLocation(cls, cls->className()), "클래스");
    scopes_.push_back("클래스:" + cls->className());
    for (const auto& field : cls->fields())
    {
        declare(field->fieldName(), SymbolKind::Property, nameLocation(field.get(), field->fieldName()));
    }
    if (const auto* ctor = cls->constructor())
    {
        std::vector<std::string> params;
        for (const auto& p : ctor->parameters())
        {
            params.push_back(p.name);
        }
        visitFunction(ctor, "생성자", params, ctor->body());
    }
    for (const auto& method : cls->methods())
    {
        std::vector<std::string> params;
        for (const auto& p : method->parameters())
        {
            params.push_back(p.name);
        }
        visitFunction(method.get(), method->methodName(), params, method->body());
    }
    scopes_.pop_back();
}

// ============================================================================
// 식
// ============================================================================

void SymbolCollector::visitExpression(const ast::Expression* expr)
{
    if (!expr)
    {
        return;
    }

    switch (expr->type())
    {
        case ast::NodeType::IDENTIFIER:
            reference(static_cast<const ast::Identifier*>(expr)->name(), expr);
            break;
        case ast::NodeType::CALL_EXPRESSION:
        {
            auto* call = static_cast<const ast::CallExpression*>(expr);
            visitExpression(call->function());
            for (const auto& arg : call->arguments())
            {
                visitExpression(arg.get());
            }
            break;
        }
        case ast::NodeType::BINARY_EXPRESSION:
        {
            auto* b = static_cast<const ast::BinaryExpression*>(expr);
            visitExpression(b->left());
            visitExpression(b->right());
            break;
        }
        case ast::NodeType::UNARY_EXPRESSION:
            visitExpression(static_cast<const ast::UnaryExpression*>(expr)->operand());
            break;
        case ast::NodeType::INDEX_EXPRESSION:
        {
            auto* i = static_cast<const ast::IndexExpression*>(expr);
            visitExpression(i->array());
            visitExpression(i->index());
            break;
        }
        case ast::NodeType::JOSA_EXPRESSION:
        {
            auto* j = static_cast<const ast::JosaExpression*>(expr);
            visitExpression(j->object());
            visitExpression(j->method());
            break;
        }
        case ast::NodeType::RANGE_EXPRESSION:
        {
            auto* r = static_cast<const ast::RangeExpression*>(expr);
            visitExpression(r->start());
            visitExpression(r->end());
            visitExpression(r->step());
            break;
        }
        case ast::NodeType::ARRAY_LITERAL:
            for (const auto& e : static_cast<const ast::ArrayLiteral*>(expr)->elements())
            {
                visitExpression(e.get());
            }
            break;
        case ast::NodeType::DICTIONARY_LITERAL:
            for (const auto& pair : static_cast<const ast::DictionaryLiteral*>(expr)->pairs())
            {
                visitExpression(pair.first.get());
                visitExpression(pair.second.get());
            }
            break;
        case ast::NodeType::MEMBER_ACCESS_EXPRESSION:
            visitExpression(static_cast<const ast::MemberAccessExpression*>(expr)->object());
            break;
        case ast::NodeType::AWAIT_EXPRESSION:
            visitExpression(static_cast<const ast::AwaitExpression*>(expr)->argument());
            break;
        case ast::NodeType::INTERPOLATED_STRING:
            for (const auto& e : static_cast<const ast::InterpolatedString*>(expr)->expressions())
            {
                visitExpression(e.get());
            }
            break;
        case ast::NodeType::MATCH_EXPRESSION:
        {
            auto* m = static_cast<const ast::MatchExpression*>(expr);
            visitExpression(m->value());
            for (const auto& c : m->cases())
            {
                visitExpression(c.guard());
                visitExpression(c.body());
            }
            break;
        }
        case ast::NodeType::FUNCTION_LITERAL:
        {
            auto* fn = static_cast<const ast::FunctionLiteral*>(expr);
            visitFunction(fn, "익명" + std::to_string(++anonymousCount_), fn->parameters(), fn->body());
            break;
        }
        case ast::NodeType::ASYNC_FUNCTION_LITERAL:
        {
            auto* fn = static_cast<const ast::AsyncFunctionLiteral*>(expr);
            visitFunction(fn, "익명" + std::to_string(++anonymousCount_), fn->parameters(), fn->body());
            break;
        }
        default:
            break;
    }
}

} // namespace lsp
} // namespace kingsejong
