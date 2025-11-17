#pragma once

/**
 * @file Compiler.h
 * @brief AST를 바이트코드로 컴파일
 * @author KingSejong Team
 * @date 2025-11-10
 *
 * AST 노드를 방문하여 바이트코드를 생성합니다.
 */

#include "Chunk.h"
#include "ast/Node.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace kingsejong {
namespace bytecode {

/**
 * @struct Local
 * @brief 로컬 변수 정보
 */
struct Local {
    std::string name;       ///< 변수 이름
    int depth;              ///< 스코프 깊이
    bool isCaptured;        ///< 클로저에 캡처되었는지
};

/**
 * @class Compiler
 * @brief AST → Bytecode 컴파일러
 *
 * AST를 순회하며 바이트코드를 생성합니다.
 */
class Compiler {
private:
    Chunk* chunk_;                          ///< 현재 청크
    std::vector<Local> locals_;             ///< 로컬 변수 테이블
    int scopeDepth_;                        ///< 현재 스코프 깊이

    // 점프 패칭을 위한 스택
    std::vector<size_t> loopStarts_;        ///< 루프 시작 오프셋
    std::vector<std::vector<size_t>> breakJumps_;  ///< break 점프 오프셋

public:
    /**
     * @brief Compiler 생성자
     */
    Compiler();

    /**
     * @brief 프로그램 컴파일
     * @param program AST 루트 노드
     * @param chunk 출력 청크
     * @return 컴파일 성공 여부
     */
    bool compile(ast::Program* program, Chunk* chunk);

    /**
     * @brief 문장 컴파일
     * @param stmt 문장 노드
     */
    void compileStatement(ast::Statement* stmt);

    /**
     * @brief 표현식 컴파일
     * @param expr 표현식 노드
     */
    void compileExpression(ast::Expression* expr);

private:
    // 문장 컴파일
    void compileVarDeclaration(ast::VarDeclaration* decl);
    void compileAssignmentStatement(ast::AssignmentStatement* stmt);
    void compileExpressionStatement(ast::ExpressionStatement* stmt);
    void compileIfStatement(ast::IfStatement* stmt);
    void compileWhileStatement(ast::WhileStatement* stmt);
    void compileForStatement(ast::ForStatement* stmt);
    void compileReturnStatement(ast::ReturnStatement* stmt);
    void compileBlockStatement(ast::BlockStatement* stmt);
    void compileRepeatStatement(ast::RepeatStatement* stmt);
    void compileRangeForStatement(ast::RangeForStatement* stmt);
    void compileImportStatement(ast::ImportStatement* stmt);
    void compileClassStatement(ast::ClassStatement* stmt);  // Phase 7.1

    // 표현식 컴파일
    void compileIntegerLiteral(ast::IntegerLiteral* lit);
    void compileFloatLiteral(ast::FloatLiteral* lit);
    void compileStringLiteral(ast::StringLiteral* lit);
    void compileBooleanLiteral(ast::BooleanLiteral* lit);
    void compileIdentifier(ast::Identifier* ident);
    void compileBinaryExpression(ast::BinaryExpression* expr);
    void compileUnaryExpression(ast::UnaryExpression* expr);
    void compileCallExpression(ast::CallExpression* expr);
    void compileArrayLiteral(ast::ArrayLiteral* lit);
    void compileIndexExpression(ast::IndexExpression* expr);
    void compileFunctionLiteral(ast::FunctionLiteral* lit);
    void compileJosaExpression(ast::JosaExpression* expr);
    void compileRangeExpression(ast::RangeExpression* expr);
    void compileNewExpression(ast::NewExpression* expr);          // Phase 7.1
    void compileMemberAccessExpression(ast::MemberAccessExpression* expr);  // Phase 7.1
    void compileThisExpression(ast::ThisExpression* expr);        // Phase 7.1

    // 스코프 관리
    void beginScope();
    void endScope();
    void addLocal(const std::string& name);
    int resolveLocal(const std::string& name);

    // 점프 관련
    size_t emitJump(OpCode op);
    void patchJump(size_t offset);
    void emitLoop(size_t loopStart);

    // 헬퍼
    void emit(OpCode op);
    void emit(OpCode op, uint8_t operand);
    void emit(OpCode op, uint8_t operand1, uint8_t operand2);
    size_t currentOffset() const;
    int currentLine() const { return 1; } // 간단화를 위해 1로 고정

    // 최적화 패스
    bool tryConstantFoldBinary(ast::BinaryExpression* expr);
    bool tryConstantFoldUnary(ast::UnaryExpression* expr);
    bool isConstantCondition(ast::Expression* expr, bool& result);
    void optimizePeephole();

    // 에러 처리
    void error(const std::string& message);
    bool hadError_ = false;
};

} // namespace bytecode
} // namespace kingsejong
