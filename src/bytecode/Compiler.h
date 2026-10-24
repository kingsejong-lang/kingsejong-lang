#pragma once
/**
 * @file Compiler.h
 * @brief AST → 바이트코드 컴파일러
 *
 * 변수는 이름으로 환경에 두므로 컴파일러는 스코프를 추적하지 않는다. 함수·메서드 본문은
 * 같은 청크 안에 인라인으로 두고 (앞에 JUMP 로 건너뛴다) 본문 노드 → 진입점을 등록해
 * VM 이 Function 객체를 호출할 때 네이티브 코드를 찾게 한다.
 */

#include "Chunk.h"
#include "ast/Node.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include <string>
#include <vector>

namespace kingsejong {
namespace bytecode {

class Compiler {
public:
    Compiler();

    /// 프로그램을 청크로 컴파일한다. 실패하면 false (메시지는 errors()).
    bool compile(ast::Program* program, Chunk* chunk);

    void compileStatement(ast::Statement* stmt);
    void compileExpression(ast::Expression* expr);

    const std::vector<std::string>& errors() const { return errors_; }

private:
    Chunk* chunk_;
    bool hadError_;
    int currentLine_;
    std::vector<std::string> errors_;

    // ---- 문장 ----
    void compileVarDeclaration(ast::VarDeclaration* decl);
    void compileAssignmentStatement(ast::AssignmentStatement* stmt);
    void compileIfStatement(ast::IfStatement* stmt);
    void compileWhileStatement(ast::WhileStatement* stmt);
    void compileRepeatStatement(ast::RepeatStatement* stmt);
    void compileRangeForStatement(ast::RangeForStatement* stmt);
    void compileReturnStatement(ast::ReturnStatement* stmt);
    void compileBlock(const ast::BlockStatement* block);
    void compileImportStatement(ast::ImportStatement* stmt);
    void compileClassStatement(ast::ClassStatement* stmt);
    void compileTryStatement(ast::TryStatement* stmt);
    void compileThrowStatement(ast::ThrowStatement* stmt);

    // ---- 표현식 ----
    void compileInterpolatedString(ast::InterpolatedString* interp);
    void compileBinaryExpression(ast::BinaryExpression* expr);
    void compileAssignmentExpression(ast::BinaryExpression* expr);
    void compileUnaryExpression(ast::UnaryExpression* expr);
    void compileCallExpression(ast::CallExpression* expr);
    void compileIndexExpression(ast::IndexExpression* expr);
    void compileJosaExpression(ast::JosaExpression* expr);
    void compileArguments(const std::vector<std::unique_ptr<ast::Expression>>& args);

    /// 함수 본문을 인라인으로 컴파일하고 프로토타입 인덱스를 돌려준다
    size_t compileFunctionBody(const std::vector<std::string>& params, ast::Statement* body,
                               bool isAsync, const std::string& name);

    // ---- 최적화 ----
    bool tryConstantFoldBinary(ast::BinaryExpression* expr);
    bool tryConstantFoldUnary(ast::UnaryExpression* expr);
    bool isConstantCondition(const ast::Expression* expr, bool& result);

    // ---- 코드 생성 ----
    void emit(OpCode op);
    void emitByte(uint8_t byte);
    void emit16(uint16_t value);
    void emitConstant(const evaluator::Value& value);
    void emitName(OpCode op, const std::string& name);
    size_t emitJump(OpCode op);            ///< [u16] 자리 표시자를 쓰고 그 위치를 돌려준다
    void patchJump(size_t operandOffset, size_t trailingBytes = 0);  ///< 자리 표시자를 현재 위치까지의 거리로 채운다
    void emitLoop(size_t loopStart);
    size_t currentOffset() const;
    uint16_t nameIndex(const std::string& name);
    void setLine(const ast::Node* node);

    void error(const std::string& message);
};

} // namespace bytecode
} // namespace kingsejong
