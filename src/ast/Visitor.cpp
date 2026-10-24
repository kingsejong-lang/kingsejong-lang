/**
 * @file Visitor.cpp
 * @brief AST 순회의 단일 정의
 */

#include "ast/Visitor.h"
#include "ast/Statement.h"
#include "ast/Expression.h"

namespace kingsejong {
namespace ast {

namespace {

// 자식 포인터가 const 로 노출되는 노드가 많다. 순회는 노드를 바꾸지 않지만 Visitor 가
// 비-const 참조를 받으므로 (린터 규칙의 기존 on* 훅 서명) 여기서만 const 를 벗긴다.
template <typename T>
void walkOne(const T* child, Visitor& visitor)
{
    walk(const_cast<T*>(child), visitor);
}

template <typename Container>
void walkEach(const Container& children, Visitor& visitor)
{
    for (const auto& child : children)
    {
        walk(child.get(), visitor);
    }
}

} // namespace

void walkChildren(Node* node, Visitor& visitor)
{
    if (!node)
    {
        return;
    }

    switch (node->type())
    {
        // ---- 프로그램·문장 ----
        case NodeType::PROGRAM:
            walkEach(static_cast<Program*>(node)->statements(), visitor);
            break;
        case NodeType::EXPRESSION_STATEMENT:
            walkOne(static_cast<ExpressionStatement*>(node)->expression(), visitor);
            break;
        case NodeType::VAR_DECLARATION:
            walkOne(static_cast<VarDeclaration*>(node)->initializer(), visitor);
            break;
        case NodeType::ASSIGNMENT_STATEMENT:
            walkOne(static_cast<AssignmentStatement*>(node)->value(), visitor);
            break;
        case NodeType::RETURN_STATEMENT:
            walkOne(static_cast<ReturnStatement*>(node)->returnValue(), visitor);
            break;
        case NodeType::IF_STATEMENT:
        {
            auto* s = static_cast<IfStatement*>(node);
            walkOne(s->condition(), visitor);
            walkOne(s->thenBranch(), visitor);
            walkOne(s->elseBranch(), visitor);
            break;
        }
        case NodeType::WHILE_STATEMENT:
        {
            auto* s = static_cast<WhileStatement*>(node);
            walkOne(s->condition(), visitor);
            walkOne(s->body(), visitor);
            break;
        }
        case NodeType::FOR_STATEMENT:
        {
            auto* s = static_cast<ForStatement*>(node);
            walkOne(s->range(), visitor);
            walkOne(s->body(), visitor);
            break;
        }
        case NodeType::REPEAT_STATEMENT:
        {
            auto* s = static_cast<RepeatStatement*>(node);
            walkOne(s->count(), visitor);
            walkOne(s->body(), visitor);
            break;
        }
        case NodeType::RANGE_FOR_STATEMENT:
        {
            auto* s = static_cast<RangeForStatement*>(node);
            walkOne(s->start(), visitor);
            walkOne(s->end(), visitor);
            walkOne(s->body(), visitor);
            break;
        }
        case NodeType::BLOCK_STATEMENT:
            walkEach(static_cast<BlockStatement*>(node)->statements(), visitor);
            break;
        case NodeType::IMPORT_STATEMENT:
            break;
        case NodeType::TRY_STATEMENT:
        {
            auto* s = static_cast<TryStatement*>(node);
            walkOne(s->tryBlock(), visitor);
            for (const auto& clause : s->catchClauses())
            {
                walkOne(clause->body(), visitor);  // CatchClause 자체는 노드가 아니다
            }
            walkOne(s->finallyBlock(), visitor);
            break;
        }
        case NodeType::THROW_STATEMENT:
            walkOne(static_cast<ThrowStatement*>(node)->value(), visitor);
            break;
        case NodeType::CLASS_STATEMENT:
        {
            auto* s = static_cast<ClassStatement*>(node);
            walkEach(s->fields(), visitor);
            walkOne(s->constructor(), visitor);
            walkEach(s->methods(), visitor);
            break;
        }
        case NodeType::FIELD_DECLARATION:
            walkOne(static_cast<FieldDeclaration*>(node)->initializer(), visitor);
            break;
        case NodeType::METHOD_DECLARATION:
            walkOne(static_cast<MethodDeclaration*>(node)->body(), visitor);
            break;
        case NodeType::CONSTRUCTOR_DECLARATION:
            walkOne(static_cast<ConstructorDeclaration*>(node)->body(), visitor);
            break;

        // ---- 식 ----
        case NodeType::INTERPOLATED_STRING:
            walkEach(static_cast<InterpolatedString*>(node)->expressions(), visitor);
            break;
        case NodeType::BINARY_EXPRESSION:
        {
            auto* e = static_cast<BinaryExpression*>(node);
            walkOne(e->left(), visitor);
            walkOne(e->right(), visitor);
            break;
        }
        case NodeType::UNARY_EXPRESSION:
            walkOne(static_cast<UnaryExpression*>(node)->operand(), visitor);
            break;
        case NodeType::JOSA_EXPRESSION:
        {
            auto* e = static_cast<JosaExpression*>(node);
            walkOne(e->object(), visitor);
            walkOne(e->method(), visitor);
            break;
        }
        case NodeType::RANGE_EXPRESSION:
        {
            auto* e = static_cast<RangeExpression*>(node);
            walkOne(e->start(), visitor);
            walkOne(e->end(), visitor);
            walkOne(e->step(), visitor);
            break;
        }
        case NodeType::FUNCTION_LITERAL:
            walk(static_cast<FunctionLiteral*>(node)->body(), visitor);
            break;
        case NodeType::ASYNC_FUNCTION_LITERAL:
            walk(static_cast<AsyncFunctionLiteral*>(node)->body(), visitor);
            break;
        case NodeType::CALL_EXPRESSION:
        {
            auto* e = static_cast<CallExpression*>(node);
            walkOne(e->function(), visitor);
            walkEach(e->arguments(), visitor);
            break;
        }
        case NodeType::AWAIT_EXPRESSION:
            walkOne(static_cast<AwaitExpression*>(node)->argument(), visitor);
            break;
        case NodeType::ARRAY_LITERAL:
            walkEach(static_cast<ArrayLiteral*>(node)->elements(), visitor);
            break;
        case NodeType::INDEX_EXPRESSION:
        {
            auto* e = static_cast<IndexExpression*>(node);
            walkOne(e->array(), visitor);
            walkOne(e->index(), visitor);
            break;
        }
        case NodeType::DICTIONARY_LITERAL:
            for (const auto& pair : static_cast<DictionaryLiteral*>(node)->pairs())
            {
                walk(pair.first.get(), visitor);
                walk(pair.second.get(), visitor);
            }
            break;
        case NodeType::MATCH_EXPRESSION:
        {
            auto* e = static_cast<MatchExpression*>(node);
            walkOne(e->value(), visitor);
            for (const auto& matchCase : e->cases())  // MatchCase 자체는 노드가 아니다
            {
                walkOne(matchCase.pattern(), visitor);
                walkOne(matchCase.guard(), visitor);
                walkOne(matchCase.body(), visitor);
            }
            break;
        }
        case NodeType::LITERAL_PATTERN:
            walkOne(static_cast<LiteralPattern*>(node)->value(), visitor);
            break;
        case NodeType::ARRAY_PATTERN:
            walkEach(static_cast<ArrayPattern*>(node)->elements(), visitor);
            break;
        case NodeType::MEMBER_ACCESS_EXPRESSION:
            walkOne(static_cast<MemberAccessExpression*>(node)->object(), visitor);
            break;
        case NodeType::NEW_EXPRESSION:
            walkEach(static_cast<NewExpression*>(node)->arguments(), visitor);
            break;

        // ---- 잎 ----
        case NodeType::IDENTIFIER:
        case NodeType::INTEGER_LITERAL:
        case NodeType::FLOAT_LITERAL:
        case NodeType::STRING_LITERAL:
        case NodeType::BOOLEAN_LITERAL:
        case NodeType::ASSIGNMENT_EXPRESSION:
        case NodeType::WILDCARD_PATTERN:
        case NodeType::BINDING_PATTERN:
        case NodeType::THIS_EXPRESSION:
            break;
    }
}

void walk(Node* node, Visitor& visitor)
{
    if (!node)
    {
        return;
    }
    if (!visitor.enter(*node))
    {
        return;
    }
    walkChildren(node, visitor);
    visitor.leave(*node);
}

} // namespace ast
} // namespace kingsejong
