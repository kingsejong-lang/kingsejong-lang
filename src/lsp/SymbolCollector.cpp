/**
 * @file SymbolCollector.cpp
 * @brief SymbolCollector 구현
 * @author KingSejong Team
 * @date 2025-11-13
 */

#include "lsp/SymbolCollector.h"
#include <iostream>

namespace kingsejong {
namespace lsp {

SymbolCollector::SymbolCollector(SymbolTable& table)
    : symbolTable_(&table)
    , currentUri_("")
    , currentScope_("global")
    , currentLine_(1)
{
}

void SymbolCollector::collect(const ast::Program* program, const std::string& uri)
{
    if (!program || !symbolTable_)
    {
        return;
    }

    currentUri_ = uri;
    currentScope_ = "global";
    currentLine_ = 1;

    // 모든 statement 방문
    for (const auto& stmt : program->statements())
    {
        visitStatement(stmt.get());
    }
}

int SymbolCollector::getLineNumber([[maybe_unused]] const ast::Node* node) const
{
    // TODO: AST 노드에 실제 줄 번호 정보가 추가되면 여기서 가져옴
    // 현재는 임시로 currentLine_ 사용
    return currentLine_;
}

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
            auto varDecl = dynamic_cast<const ast::VarDeclaration*>(stmt);
            visitVarDeclaration(varDecl);
            currentLine_++;
            break;
        }

        case ast::NodeType::ASSIGNMENT_STATEMENT:
        {
            auto assign = dynamic_cast<const ast::AssignmentStatement*>(stmt);

            // 함수 선언인지 확인 (함수 이름(매개변수) {...} 형태)
            if (assign->value() && assign->value()->type() == ast::NodeType::FUNCTION_LITERAL)
            {
                auto funcLit = dynamic_cast<const ast::FunctionLiteral*>(assign->value());
                visitFunctionLiteral(funcLit, assign->varName());
            }
            else
            {
                // 일반 할당: 오른쪽 표현식 방문
                visitExpression(assign->value());
            }
            currentLine_++;
            break;
        }

        case ast::NodeType::EXPRESSION_STATEMENT:
        {
            auto exprStmt = dynamic_cast<const ast::ExpressionStatement*>(stmt);
            visitExpression(exprStmt->expression());
            currentLine_++;
            break;
        }

        case ast::NodeType::RETURN_STATEMENT:
        {
            auto retStmt = dynamic_cast<const ast::ReturnStatement*>(stmt);
            if (retStmt->returnValue())
            {
                visitExpression(retStmt->returnValue());
            }
            currentLine_++;
            break;
        }

        case ast::NodeType::IF_STATEMENT:
        {
            auto ifStmt = dynamic_cast<const ast::IfStatement*>(stmt);
            visitExpression(ifStmt->condition());
            visitBlockStatement(ifStmt->thenBranch());
            if (ifStmt->elseBranch())
            {
                visitBlockStatement(ifStmt->elseBranch());
            }
            break;
        }

        case ast::NodeType::WHILE_STATEMENT:
        {
            auto whileStmt = dynamic_cast<const ast::WhileStatement*>(stmt);
            visitExpression(whileStmt->condition());
            visitBlockStatement(whileStmt->body());
            break;
        }

        default:
            currentLine_++;
            break;
    }
}

void SymbolCollector::visitExpression(const ast::Expression* expr)
{
    if (!expr)
    {
        return;
    }

    switch (expr->type())
    {
        case ast::NodeType::IDENTIFIER:
        {
            auto ident = dynamic_cast<const ast::Identifier*>(expr);
            visitIdentifier(ident);
            break;
        }

        case ast::NodeType::FUNCTION_LITERAL:
        {
            // 함수 리터럴은 변수 선언에서 처리
            break;
        }

        case ast::NodeType::CALL_EXPRESSION:
        {
            auto call = dynamic_cast<const ast::CallExpression*>(expr);
            visitCallExpression(call);
            break;
        }

        case ast::NodeType::BINARY_EXPRESSION:
        {
            auto binary = dynamic_cast<const ast::BinaryExpression*>(expr);
            visitExpression(binary->left());
            visitExpression(binary->right());
            break;
        }

        case ast::NodeType::UNARY_EXPRESSION:
        {
            auto unary = dynamic_cast<const ast::UnaryExpression*>(expr);
            visitExpression(unary->operand());
            break;
        }

        case ast::NodeType::INDEX_EXPRESSION:
        {
            auto index = dynamic_cast<const ast::IndexExpression*>(expr);
            visitExpression(index->array());
            visitExpression(index->index());
            break;
        }

        default:
            // 리터럴 등은 처리할 필요 없음
            break;
    }
}

void SymbolCollector::visitVarDeclaration(const ast::VarDeclaration* varDecl)
{
    if (!varDecl)
    {
        return;
    }

    // 초기화 표현식이 함수 리터럴인지 확인
    bool isFunctionDecl = varDecl->initializer() &&
                          varDecl->initializer()->type() == ast::NodeType::FUNCTION_LITERAL;

    if (isFunctionDecl)
    {
        // 함수 선언: 함수 리터럴 방문에서 처리
        auto funcLit = dynamic_cast<const ast::FunctionLiteral*>(varDecl->initializer());
        visitFunctionLiteral(funcLit, varDecl->varName());
    }
    else
    {
        // 일반 변수 선언
        Symbol symbol;
        symbol.name = varDecl->varName();
        symbol.kind = SymbolKind::Variable;
        symbol.definitionLocation = Location(currentLine_, 1, currentUri_);
        symbol.typeInfo = varDecl->typeName();
        symbol.scope = currentScope_;

        symbolTable_->addSymbol(symbol);

        // 초기화 표현식 방문 (참조 수집)
        if (varDecl->initializer())
        {
            visitExpression(varDecl->initializer());
        }
    }
}

void SymbolCollector::visitFunctionLiteral(const ast::FunctionLiteral* funcLit, const std::string& funcName)
{
    if (!funcLit)
    {
        return;
    }

    // 함수 심볼 추가
    Symbol funcSymbol;
    funcSymbol.name = funcName;
    funcSymbol.kind = SymbolKind::Function;
    funcSymbol.definitionLocation = Location(currentLine_, 1, currentUri_);
    funcSymbol.scope = currentScope_;
    funcSymbol.typeInfo = "함수";

    symbolTable_->addSymbol(funcSymbol);

    // 스코프 변경
    std::string previousScope = currentScope_;
    currentScope_ = "함수:" + funcName;

    // 매개변수 추가
    for (const auto& param : funcLit->parameters())
    {
        Symbol paramSymbol;
        paramSymbol.name = param;
        paramSymbol.kind = SymbolKind::Parameter;
        paramSymbol.definitionLocation = Location(currentLine_, 1, currentUri_);
        paramSymbol.scope = currentScope_;

        symbolTable_->addSymbol(paramSymbol);
    }

    // 함수 본문 방문 (함수 선언 다음 줄부터 시작)
    // "함수 테스트() {" 다음 줄이 함수 본문 첫 줄
    currentLine_++;

    if (funcLit->body())
    {
        auto blockStmt = dynamic_cast<const ast::BlockStatement*>(funcLit->body());
        if (blockStmt)
        {
            visitBlockStatement(blockStmt);
        }
    }

    // 스코프 복원
    currentScope_ = previousScope;
}

void SymbolCollector::visitIdentifier(const ast::Identifier* ident)
{
    if (!ident)
    {
        return;
    }

    // 식별자 참조 추가
    symbolTable_->addReference(ident->name(), Location(currentLine_, 1, currentUri_));
}

void SymbolCollector::visitCallExpression(const ast::CallExpression* call)
{
    if (!call)
    {
        return;
    }

    // 함수 이름 참조 추가
    if (call->function()->type() == ast::NodeType::IDENTIFIER)
    {
        auto funcName = dynamic_cast<const ast::Identifier*>(call->function());
        symbolTable_->addReference(funcName->name(), Location(currentLine_, 1, currentUri_));
    }

    // 인자 표현식 방문
    for (const auto& arg : call->arguments())
    {
        visitExpression(arg.get());
    }
}

void SymbolCollector::visitBlockStatement(const ast::BlockStatement* block)
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

} // namespace lsp
} // namespace kingsejong
