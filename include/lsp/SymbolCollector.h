#pragma once

/**
 * @file SymbolCollector.h
 * @brief AST 를 걸어 LSP 심볼 테이블을 채운다
 *
 * 순회는 ast::walk 가 하고, 이 클래스는 enter()/leave() 에서 선언·참조·스코프만 다룬다.
 * 정의와 참조의 위치는 AST 노드가 가진 소스 위치를 쓴다. 문서 본문을 같이 주면
 * 열을 LSP 가 쓰는 UTF-16 단위로 바꾸고, 선언문의 변수 이름 위치를 그 줄에서 찾는다.
 * 참조는 안쪽 스코프부터 바깥으로 찾아 실제로 보이는 심볼에만 붙인다.
 */

#include "lsp/SymbolTable.h"
#include "ast/Visitor.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include <string>
#include <vector>

namespace kingsejong {
namespace lsp {

class SymbolCollector : public ast::Visitor
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

    bool enter(ast::Node& node) override;
    void leave(ast::Node& node) override;

private:
    /// 함수 비슷한 노드(함수 리터럴·메서드·생성자)에 들어갈 때: 이름을 정의하고 스코프를 연다
    void enterFunction(const ast::Node& node, const std::string& name, const std::vector<std::string>& parameters);

    void declare(const std::string& name, SymbolKind kind, const Location& location,
                 const std::string& typeInfo = "");
    /// 이름을 안쪽 스코프부터 찾아 보이는 심볼에 참조를 붙인다. 없으면 무시한다(내장 함수 등).
    void reference(const std::string& name, const ast::Node& node);
    bool isVisible(const std::string& name) const;

    Location locationOf(const ast::Node& node) const;
    /// 노드가 있는 줄에서 after 바이트 열 뒤에 처음 나오는 식별자 name 의 위치 (없으면 노드 위치)
    Location nameLocation(const ast::Node& node, const std::string& name, int afterByteColumn = 0) const;
    Location toLocation(int line, int byteColumn) const;

    const std::string& currentScope() const { return scopes_.back(); }

    SymbolTable* symbolTable_ = nullptr;
    std::string currentUri_;
    std::vector<std::string> scopes_;     ///< 안쪽이 뒤. 맨 앞은 "global"
    std::vector<std::string> lines_;      ///< 문서 본문의 줄 (없으면 비어 있음)
    std::string pendingFunctionName_;     ///< "정수 f = 함수(...)" 에서 다음 함수 리터럴이 받을 이름
    const ast::Node* pendingFunctionNode_ = nullptr;  ///< 그 이름이 적힌 선언문 (위치 찾기용)
    int anonymousCount_ = 0;
};

} // namespace lsp
} // namespace kingsejong
