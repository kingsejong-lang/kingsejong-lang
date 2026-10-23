#pragma once

/**
 * @file Visitor.h
 * @brief AST 순회의 단일 정의
 *
 * 노드마다 어떤 자식이 있는지는 walk() 한 곳에만 적는다. 린터 규칙, LSP 심볼 수집기처럼
 * 트리를 "걷기만" 하면 되는 코드는 Visitor 를 상속해 enter()/leave() 에서 관심 있는 노드만
 * 고르면 된다. 새 노드 종류가 생기면 walkChildren() 의 switch 에 한 줄 더하면 모든 순회가 따라온다.
 *
 * 사용 예:
 *   struct Counter : ast::Visitor {
 *       int identifiers = 0;
 *       bool enter(ast::Node& node) override {
 *           if (node.type() == ast::NodeType::IDENTIFIER) ++identifiers;
 *           return true;   // 자식도 걷는다. false 면 이 노드 아래는 건너뛴다.
 *       }
 *   };
 *   Counter c; ast::walk(program, c);
 */

#include "ast/Node.h"

namespace kingsejong {
namespace ast {

class Visitor
{
public:
    virtual ~Visitor() = default;

    /// 노드에 들어갈 때. false 를 돌려주면 자식을 걷지 않고 leave() 도 부르지 않는다.
    virtual bool enter(Node& /* node */) { return true; }

    /// 자식을 모두 걷은 뒤.
    virtual void leave(Node& /* node */) {}
};

/// node 와 그 아래 전체를 깊이 우선으로 걷는다. node 가 null 이면 아무것도 하지 않는다.
void walk(Node* node, Visitor& visitor);

/// node 자신은 건너뛰고 자식만 걷는다 (enter 에서 직접 처리한 뒤 일부만 걷고 싶을 때).
void walkChildren(Node* node, Visitor& visitor);

} // namespace ast
} // namespace kingsejong
