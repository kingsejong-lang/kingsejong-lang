#pragma once

/**
 * @file SymbolCollector.h
 * @brief AST 기반 심볼 수집기
 * @author KingSejong Team
 * @date 2025-11-13
 *
 * AST를 순회하여 SymbolTable을 구축합니다.
 * Go to Definition, Find References 등의 LSP 고급 기능을 위한
 * 심볼 정보를 수집합니다.
 */

#include "lsp/SymbolTable.h"
#include "ast/Node.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include <string>

namespace kingsejong {
namespace lsp {

/**
 * @class SymbolCollector
 * @brief AST 방문자 패턴으로 심볼 수집
 *
 * AST를 재귀적으로 순회하며 다음을 수집합니다:
 * - 변수 선언 → 심볼 추가
 * - 함수 선언 → 심볼 추가
 * - 식별자 사용 → 참조 추가
 * - 함수 호출 → 참조 추가
 *
 * 스코프 추적:
 * - 전역: "global"
 * - 함수 내부: "함수:함수이름"
 *
 * Thread Safety: NOT thread-safe
 * Memory: RAII - 스택 기반 재귀
 *
 * Example:
 * ```cpp
 * SymbolTable table;
 * SymbolCollector collector(table);
 *
 * auto program = parser.parseProgram();
 * collector.collect(program.get(), "example.ksj");
 *
 * // 이제 table에서 심볼 조회 가능
 * auto x = table.findSymbol("x");
 * ```
 */
class SymbolCollector
{
public:
    /**
     * @brief 생성자
     * @param table 심볼을 저장할 SymbolTable 참조
     */
    explicit SymbolCollector(SymbolTable& table);

    /**
     * @brief 기본 생성자 (테스트용)
     */
    SymbolCollector() = default;

    /**
     * @brief AST에서 심볼 수집
     * @param program AST 루트 노드
     * @param uri 파일 URI
     *
     * Preconditions:
     * - program != nullptr
     *
     * Postconditions:
     * - table에 모든 심볼과 참조가 추가됨
     *
     * Complexity: O(n) where n = AST nodes
     */
    void collect(const ast::Program* program, const std::string& uri);

private:
    /**
     * @brief Statement 방문
     */
    void visitStatement(const ast::Statement* stmt);

    /**
     * @brief Expression 방문
     */
    void visitExpression(const ast::Expression* expr);

    /**
     * @brief 변수 선언 처리
     */
    void visitVarDeclaration(const ast::VarDeclaration* varDecl);

    /**
     * @brief 함수 리터럴 처리
     */
    void visitFunctionLiteral(const ast::FunctionLiteral* funcLit, const std::string& funcName);

    /**
     * @brief 식별자 참조 처리
     */
    void visitIdentifier(const ast::Identifier* ident);

    /**
     * @brief 함수 호출 처리
     */
    void visitCallExpression(const ast::CallExpression* call);

    /**
     * @brief 블록 문장 처리
     */
    void visitBlockStatement(const ast::BlockStatement* block);

    /**
     * @brief 현재 줄 번호 가져오기 (AST 노드에서)
     */
    int getLineNumber(const ast::Node* node) const;

private:
    SymbolTable* symbolTable_;      ///< 심볼 테이블 (nullable for default ctor)
    std::string currentUri_;        ///< 현재 파일 URI
    std::string currentScope_;      ///< 현재 스코프
    int currentLine_;               ///< 현재 줄 번호 (임시)
};

} // namespace lsp
} // namespace kingsejong
