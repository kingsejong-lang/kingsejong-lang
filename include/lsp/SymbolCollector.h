#pragma once

/**
 * @file SymbolCollector.h
 * @brief AST 를 걸어 LSP 심볼 테이블을 채운다
 *
 * 정의와 참조의 위치는 AST 노드가 가진 소스 위치를 쓴다. 문서 본문을 같이 주면
 * 열을 LSP 가 쓰는 UTF-16 단위로 바꾸고, 선언문의 변수 이름 위치를 그 줄에서 찾는다.
 * 참조는 안쪽 스코프부터 바깥으로 찾아 실제로 보이는 심볼에만 붙인다.
 */

#include "lsp/SymbolTable.h"
#include "ast/Node.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include <string>
#include <vector>

namespace kingsejong {
namespace lsp {

class SymbolCollector
{
public:
    explicit SymbolCollector(SymbolTable& table);
    SymbolCollector() = default;

    /**
     * @brief 프로그램의 심볼을 수집한다
     * @param program 파싱된 프로그램
     * @param uri 문서 URI
     * @param content 문서 본문 (선택). 주면 열이 UTF-16 단위가 되고 이름 위치가 정확해진다.
     */
    void collect(const ast::Program* program, const std::string& uri, const std::string& content = "");

private:
    void visitStatement(const ast::Statement* stmt);
    void visitExpression(const ast::Expression* expr);
    void visitBlock(const ast::BlockStatement* block);
    void visitFunction(const ast::Node* node, const std::string& name,
                       const std::vector<std::string>& parameters, const ast::Statement* body);
    void visitClass(const ast::ClassStatement* cls);

    /// 심볼을 현재 스코프에 정의한다
    void declare(const std::string& name, SymbolKind kind, const Location& location,
                 const std::string& typeInfo = "");
    /// 이름을 안쪽 스코프부터 찾아 보이는 심볼에 참조를 붙인다. 없으면 무시한다(내장 함수 등).
    void reference(const std::string& name, const ast::Node* node);

    /// 노드의 소스 위치를 Location 으로 바꾼다 (본문이 있으면 UTF-16 열)
    Location locationOf(const ast::Node* node) const;
    /// 노드가 있는 줄에서 노드 열 뒤에 처음 나오는 식별자 name 의 위치 (없으면 노드 위치)
    Location nameLocation(const ast::Node* node, const std::string& name) const;
    Location toLocation(int line, int byteColumn) const;

    const std::string& currentScope() const { return scopes_.back(); }

    SymbolTable* symbolTable_ = nullptr;
    std::string currentUri_;
    std::vector<std::string> scopes_;   ///< 안쪽이 뒤. 맨 앞은 "global"
    std::vector<std::string> lines_;    ///< 문서 본문의 줄 (없으면 비어 있음)
    int anonymousCount_ = 0;
};

} // namespace lsp
} // namespace kingsejong
