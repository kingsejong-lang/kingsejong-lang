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

bool isFunctionLiteral(const ast::Expression* expr)
{
    return expr && (expr->type() == ast::NodeType::FUNCTION_LITERAL ||
                    expr->type() == ast::NodeType::ASYNC_FUNCTION_LITERAL);
}

const std::vector<std::string>& parametersOf(const ast::Node& node)
{
    if (node.type() == ast::NodeType::ASYNC_FUNCTION_LITERAL)
    {
        return static_cast<const ast::AsyncFunctionLiteral&>(node).parameters();
    }
    return static_cast<const ast::FunctionLiteral&>(node).parameters();
}

std::vector<std::string> parameterNames(const std::vector<ast::Parameter>& params)
{
    std::vector<std::string> names;
    for (const auto& p : params)
    {
        names.push_back(p.name);
    }
    return names;
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
    pendingFunctionName_.clear();
    pendingFunctionNode_ = nullptr;
    anonymousCount_ = 0;

    ast::walk(const_cast<ast::Program*>(program), *this);
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

Location SymbolCollector::locationOf(const ast::Node& node) const
{
    const auto& loc = node.location();
    return toLocation(loc.line, loc.column);
}

Location SymbolCollector::nameLocation(const ast::Node& node, const std::string& name, int afterByteColumn) const
{
    const auto& loc = node.location();
    if (loc.line >= 1 && static_cast<size_t>(loc.line) <= lines_.size() && !name.empty())
    {
        const std::string& text = lines_[static_cast<size_t>(loc.line) - 1];
        int fromColumn = afterByteColumn > 0 ? afterByteColumn : loc.column;
        size_t from = fromColumn > 0 ? static_cast<size_t>(fromColumn - 1) : 0;
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

bool SymbolCollector::isVisible(const std::string& name) const
{
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it)
    {
        if (symbolTable_->findSymbolInScope(name, *it).has_value())
        {
            return true;
        }
    }
    return false;
}

void SymbolCollector::reference(const std::string& name, const ast::Node& node)
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

void SymbolCollector::enterFunction(const ast::Node& node, const std::string& name,
                                    const std::vector<std::string>& parameters)
{
    // 이름이 선언문("정수 f = 함수(...)", "함수 f(...)")에서 왔으면 그 문장에서 이름을 찾는다
    const ast::Node& named = pendingFunctionNode_ ? *pendingFunctionNode_ : node;
    Location defLocation = nameLocation(named, name);
    declare(name, SymbolKind::Function, defLocation, "함수");

    scopes_.push_back("함수:" + name);
    // 매개변수는 함수 이름 뒤 같은 줄에 있다. 이름 위치 뒤에서부터 찾는다.
    int afterName = 0;
    if (defLocation.line >= 1 && static_cast<size_t>(defLocation.line) <= lines_.size())
    {
        const std::string& text = lines_[static_cast<size_t>(defLocation.line) - 1];
        size_t namePos = text.find(name);
        afterName = namePos == std::string::npos ? 0 : static_cast<int>(namePos + name.size()) + 1;
    }
    for (const auto& param : parameters)
    {
        declare(param, SymbolKind::Parameter, nameLocation(named, param, afterName));
    }
}

// ============================================================================
// 순회 훅
// ============================================================================

bool SymbolCollector::enter(ast::Node& node)
{
    using ast::NodeType;
    switch (node.type())
    {
        case NodeType::VAR_DECLARATION:
        {
            auto& decl = static_cast<const ast::VarDeclaration&>(node);
            if (isFunctionLiteral(decl.initializer()))
            {
                pendingFunctionName_ = decl.varName();
                pendingFunctionNode_ = &node;
            }
            // 일반 변수는 초기식을 걷고 난 뒤 leave() 에서 선언한다: "정수 x = x + 1" 의 오른쪽 x 는 바깥 x 다
            break;
        }
        case NodeType::ASSIGNMENT_STATEMENT:
        {
            auto& assign = static_cast<const ast::AssignmentStatement&>(node);
            if (isFunctionLiteral(assign.value()))
            {
                pendingFunctionName_ = assign.varName();
                pendingFunctionNode_ = &node;
            }
            break;
        }
        case NodeType::FUNCTION_LITERAL:
        case NodeType::ASYNC_FUNCTION_LITERAL:
        {
            std::string name = pendingFunctionName_.empty()
                ? "익명" + std::to_string(++anonymousCount_)
                : pendingFunctionName_;
            enterFunction(node, name, parametersOf(node));
            pendingFunctionName_.clear();
            pendingFunctionNode_ = nullptr;
            break;
        }
        case NodeType::FOR_STATEMENT:
        {
            auto& f = static_cast<const ast::ForStatement&>(node);
            declare(f.iterator(), SymbolKind::Variable, nameLocation(node, f.iterator()));
            break;
        }
        case NodeType::RANGE_FOR_STATEMENT:
        {
            auto& r = static_cast<const ast::RangeForStatement&>(node);
            declare(r.varName(), SymbolKind::Variable, nameLocation(node, r.varName()));
            break;
        }
        case NodeType::TRY_STATEMENT:
        {
            // CatchClause 는 노드가 아니라 위치가 없다. 시도문 줄에서 이름을 찾는다.
            for (const auto& clause : static_cast<const ast::TryStatement&>(node).catchClauses())
            {
                if (!clause->errorVarName().empty())
                {
                    declare(clause->errorVarName(), SymbolKind::Variable, nameLocation(node, clause->errorVarName()));
                }
            }
            break;
        }
        case NodeType::CLASS_STATEMENT:
        {
            auto& cls = static_cast<const ast::ClassStatement&>(node);
            declare(cls.className(), SymbolKind::Class, nameLocation(node, cls.className()), "클래스");
            scopes_.push_back("클래스:" + cls.className());
            break;
        }
        case NodeType::FIELD_DECLARATION:
        {
            auto& field = static_cast<const ast::FieldDeclaration&>(node);
            declare(field.fieldName(), SymbolKind::Property, nameLocation(node, field.fieldName()));
            break;
        }
        case NodeType::METHOD_DECLARATION:
        {
            auto& method = static_cast<const ast::MethodDeclaration&>(node);
            enterFunction(node, method.methodName(), parameterNames(method.parameters()));
            break;
        }
        case NodeType::CONSTRUCTOR_DECLARATION:
        {
            auto& ctor = static_cast<const ast::ConstructorDeclaration&>(node);
            enterFunction(node, "생성자", parameterNames(ctor.parameters()));
            break;
        }
        case NodeType::IDENTIFIER:
            reference(static_cast<const ast::Identifier&>(node).name(), node);
            break;
        default:
            break;
    }
    return true;
}

void SymbolCollector::leave(ast::Node& node)
{
    using ast::NodeType;
    switch (node.type())
    {
        case NodeType::VAR_DECLARATION:
        {
            auto& decl = static_cast<const ast::VarDeclaration&>(node);
            if (!isFunctionLiteral(decl.initializer()))
            {
                declare(decl.varName(), SymbolKind::Variable, nameLocation(node, decl.varName()), decl.typeName());
            }
            break;
        }
        case NodeType::ASSIGNMENT_STATEMENT:
        {
            auto& assign = static_cast<const ast::AssignmentStatement&>(node);
            if (isFunctionLiteral(assign.value()))
            {
                break;
            }
            // 보이는 변수가 있으면 그 변수에 쓰는 것이고, 없으면 여기서 새로 만드는 것이다
            if (isVisible(assign.varName()))
            {
                reference(assign.varName(), node);
            }
            else
            {
                declare(assign.varName(), SymbolKind::Variable, nameLocation(node, assign.varName()));
            }
            break;
        }
        case NodeType::FUNCTION_LITERAL:
        case NodeType::ASYNC_FUNCTION_LITERAL:
        case NodeType::METHOD_DECLARATION:
        case NodeType::CONSTRUCTOR_DECLARATION:
        case NodeType::CLASS_STATEMENT:
            scopes_.pop_back();
            break;
        default:
            break;
    }
}

} // namespace lsp
} // namespace kingsejong
