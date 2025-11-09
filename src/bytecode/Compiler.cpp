/**
 * @file Compiler.cpp
 * @brief Compiler 구현
 * @author KingSejong Team
 * @date 2025-11-10
 */

#include "Compiler.h"
#include <iostream>

namespace kingsejong {
namespace bytecode {

Compiler::Compiler()
    : chunk_(nullptr), scopeDepth_(0), hadError_(false) {}

bool Compiler::compile(ast::Program* program, Chunk* chunk) {
    chunk_ = chunk;
    hadError_ = false;

    try {
        for (auto& stmt : program->statements()) {
            compileStatement(stmt.get());
        }

        // 프로그램 종료
        emit(OpCode::HALT);

        return !hadError_;
    } catch (const std::exception& e) {
        error(std::string("Compilation error: ") + e.what());
        return false;
    }
}

void Compiler::compileStatement(ast::Statement* stmt) {
    if (!stmt) return;

    switch (stmt->nodeType()) {
        case ast::NodeType::VAR_DECLARATION:
            compileVarDeclaration(static_cast<ast::VarDeclaration*>(stmt));
            break;
        case ast::NodeType::EXPRESSION_STATEMENT:
            compileExpressionStatement(static_cast<ast::ExpressionStatement*>(stmt));
            break;
        case ast::NodeType::IF_STATEMENT:
            compileIfStatement(static_cast<ast::IfStatement*>(stmt));
            break;
        case ast::NodeType::WHILE_STATEMENT:
            compileWhileStatement(static_cast<ast::WhileStatement*>(stmt));
            break;
        case ast::NodeType::FOR_STATEMENT:
            compileForStatement(static_cast<ast::ForStatement*>(stmt));
            break;
        case ast::NodeType::RETURN_STATEMENT:
            compileReturnStatement(static_cast<ast::ReturnStatement*>(stmt));
            break;
        case ast::NodeType::BLOCK_STATEMENT:
            compileBlockStatement(static_cast<ast::BlockStatement*>(stmt));
            break;
        case ast::NodeType::REPEAT_STATEMENT:
            compileRepeatStatement(static_cast<ast::RepeatStatement*>(stmt));
            break;
        case ast::NodeType::RANGE_FOR_STATEMENT:
            compileRangeForStatement(static_cast<ast::RangeForStatement*>(stmt));
            break;
        case ast::NodeType::IMPORT_STATEMENT:
            compileImportStatement(static_cast<ast::ImportStatement*>(stmt));
            break;
        default:
            error("Unknown statement type");
    }
}

void Compiler::compileExpression(ast::Expression* expr) {
    if (!expr) {
        emit(OpCode::LOAD_NULL);
        return;
    }

    switch (expr->nodeType()) {
        case ast::NodeType::INTEGER_LITERAL:
            compileIntegerLiteral(static_cast<ast::IntegerLiteral*>(expr));
            break;
        case ast::NodeType::FLOAT_LITERAL:
            compileFloatLiteral(static_cast<ast::FloatLiteral*>(expr));
            break;
        case ast::NodeType::STRING_LITERAL:
            compileStringLiteral(static_cast<ast::StringLiteral*>(expr));
            break;
        case ast::NodeType::BOOLEAN_LITERAL:
            compileBooleanLiteral(static_cast<ast::BooleanLiteral*>(expr));
            break;
        case ast::NodeType::IDENTIFIER:
            compileIdentifier(static_cast<ast::Identifier*>(expr));
            break;
        case ast::NodeType::BINARY_EXPRESSION:
            compileBinaryExpression(static_cast<ast::BinaryExpression*>(expr));
            break;
        case ast::NodeType::UNARY_EXPRESSION:
            compileUnaryExpression(static_cast<ast::UnaryExpression*>(expr));
            break;
        case ast::NodeType::CALL_EXPRESSION:
            compileCallExpression(static_cast<ast::CallExpression*>(expr));
            break;
        case ast::NodeType::ARRAY_LITERAL:
            compileArrayLiteral(static_cast<ast::ArrayLiteral*>(expr));
            break;
        case ast::NodeType::INDEX_EXPRESSION:
            compileIndexExpression(static_cast<ast::IndexExpression*>(expr));
            break;
        case ast::NodeType::FUNCTION_LITERAL:
            compileFunctionLiteral(static_cast<ast::FunctionLiteral*>(expr));
            break;
        case ast::NodeType::JOSA_EXPRESSION:
            compileJosaExpression(static_cast<ast::JosaExpression*>(expr));
            break;
        case ast::NodeType::RANGE_EXPRESSION:
            compileRangeExpression(static_cast<ast::RangeExpression*>(expr));
            break;
        default:
            error("Unknown expression type");
            emit(OpCode::LOAD_NULL);
    }
}

// ============================================================================
// 문장 컴파일
// ============================================================================

void Compiler::compileVarDeclaration(ast::VarDeclaration* decl) {
    if (decl->value()) {
        compileExpression(decl->value());
    } else {
        emit(OpCode::LOAD_NULL);
    }

    if (scopeDepth_ == 0) {
        // 전역 변수
        size_t nameIdx = chunk_->addName(decl->name());
        emit(OpCode::STORE_GLOBAL, static_cast<uint8_t>(nameIdx));
    } else {
        // 로컬 변수
        addLocal(decl->name());
    }
}

void Compiler::compileExpressionStatement(ast::ExpressionStatement* stmt) {
    compileExpression(stmt->expression());
    emit(OpCode::POP);  // 표현식 결과 제거
}

void Compiler::compileIfStatement(ast::IfStatement* stmt) {
    // 조건 평가
    compileExpression(stmt->condition());

    // 거짓이면 else로 점프
    size_t thenJump = emitJump(OpCode::JUMP_IF_FALSE);

    // then 블록
    emit(OpCode::POP);  // 조건 값 제거
    compileStatement(stmt->thenBranch());

    // else 블록 건너뛰기
    size_t elseJump = emitJump(OpCode::JUMP);

    // else 블록
    patchJump(thenJump);
    emit(OpCode::POP);  // 조건 값 제거

    if (stmt->elseBranch()) {
        compileStatement(stmt->elseBranch());
    }

    patchJump(elseJump);
}

void Compiler::compileWhileStatement(ast::WhileStatement* stmt) {
    size_t loopStart = currentOffset();
    loopStarts_.push_back(loopStart);
    breakJumps_.push_back({});

    // 조건 평가
    compileExpression(stmt->condition());

    // 거짓이면 루프 종료
    size_t exitJump = emitJump(OpCode::JUMP_IF_FALSE);
    emit(OpCode::POP);  // 조건 값 제거

    // 루프 본문
    compileStatement(stmt->body());

    // 루프 시작으로 점프
    emitLoop(loopStart);

    // 루프 종료
    patchJump(exitJump);
    emit(OpCode::POP);  // 조건 값 제거

    // break 점프 패칭
    for (size_t offset : breakJumps_.back()) {
        patchJump(offset);
    }

    loopStarts_.pop_back();
    breakJumps_.pop_back();
}

void Compiler::compileForStatement(ast::ForStatement* stmt) {
    // TODO: 일반 for 문 구현
    error("For statement not yet implemented");
}

void Compiler::compileReturnStatement(ast::ReturnStatement* stmt) {
    if (stmt->value()) {
        compileExpression(stmt->value());
    } else {
        emit(OpCode::LOAD_NULL);
    }
    emit(OpCode::RETURN);
}

void Compiler::compileBlockStatement(ast::BlockStatement* stmt) {
    beginScope();

    for (auto& s : stmt->statements()) {
        compileStatement(s.get());
    }

    endScope();
}

void Compiler::compileRepeatStatement(ast::RepeatStatement* stmt) {
    // 반복 횟수 평가
    compileExpression(stmt->count());

    size_t loopStart = currentOffset();
    loopStarts_.push_back(loopStart);
    breakJumps_.push_back({});

    // 카운터 복제
    emit(OpCode::DUP);

    // 0과 비교
    size_t constIdx = chunk_->addConstant(evaluator::Value::createInteger(0));
    emit(OpCode::LOAD_CONST, static_cast<uint8_t>(constIdx));
    emit(OpCode::LE);  // counter <= 0

    // 0 이하면 루프 종료
    size_t exitJump = emitJump(OpCode::JUMP_IF_TRUE);
    emit(OpCode::POP);

    // 루프 본문
    compileStatement(stmt->body());

    // 카운터 감소
    constIdx = chunk_->addConstant(evaluator::Value::createInteger(1));
    emit(OpCode::LOAD_CONST, static_cast<uint8_t>(constIdx));
    emit(OpCode::SUB);

    // 루프 시작으로
    emitLoop(loopStart);

    // 루프 종료
    patchJump(exitJump);
    emit(OpCode::POP);  // 카운터 제거
    emit(OpCode::POP);  // 조건 값 제거

    loopStarts_.pop_back();
    breakJumps_.pop_back();
}

void Compiler::compileRangeForStatement(ast::RangeForStatement* stmt) {
    beginScope();

    // 범위 생성
    compileExpression(stmt->range());

    // TODO: 범위 반복 구현
    // 간단화를 위해 에러 처리
    error("Range for statement not yet fully implemented");

    endScope();
}

void Compiler::compileImportStatement(ast::ImportStatement* stmt) {
    size_t nameIdx = chunk_->addName(stmt->modulePath());
    emit(OpCode::IMPORT, static_cast<uint8_t>(nameIdx));
}

// ============================================================================
// 표현식 컴파일
// ============================================================================

void Compiler::compileIntegerLiteral(ast::IntegerLiteral* lit) {
    size_t idx = chunk_->addConstant(evaluator::Value::createInteger(lit->value()));
    emit(OpCode::LOAD_CONST, static_cast<uint8_t>(idx));
}

void Compiler::compileFloatLiteral(ast::FloatLiteral* lit) {
    size_t idx = chunk_->addConstant(evaluator::Value::createFloat(lit->value()));
    emit(OpCode::LOAD_CONST, static_cast<uint8_t>(idx));
}

void Compiler::compileStringLiteral(ast::StringLiteral* lit) {
    size_t idx = chunk_->addConstant(evaluator::Value::createString(lit->value()));
    emit(OpCode::LOAD_CONST, static_cast<uint8_t>(idx));
}

void Compiler::compileBooleanLiteral(ast::BooleanLiteral* lit) {
    emit(lit->value() ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE);
}

void Compiler::compileIdentifier(ast::Identifier* ident) {
    int local = resolveLocal(ident->value());

    if (local != -1) {
        // 로컬 변수
        emit(OpCode::LOAD_VAR, static_cast<uint8_t>(local));
    } else {
        // 전역 변수
        size_t nameIdx = chunk_->addName(ident->value());
        emit(OpCode::LOAD_GLOBAL, static_cast<uint8_t>(nameIdx));
    }
}

void Compiler::compileBinaryExpression(ast::BinaryExpression* expr) {
    // 왼쪽 피연산자
    compileExpression(expr->left());

    // 오른쪽 피연산자
    compileExpression(expr->right());

    // 연산자
    lexer::TokenType op = expr->op();
    switch (op) {
        case lexer::TokenType::PLUS:       emit(OpCode::ADD); break;
        case lexer::TokenType::MINUS:      emit(OpCode::SUB); break;
        case lexer::TokenType::ASTERISK:   emit(OpCode::MUL); break;
        case lexer::TokenType::SLASH:      emit(OpCode::DIV); break;
        case lexer::TokenType::PERCENT:    emit(OpCode::MOD); break;
        case lexer::TokenType::EQ:         emit(OpCode::EQ); break;
        case lexer::TokenType::NOT_EQ:     emit(OpCode::NE); break;
        case lexer::TokenType::LT:         emit(OpCode::LT); break;
        case lexer::TokenType::GT:         emit(OpCode::GT); break;
        case lexer::TokenType::LE:         emit(OpCode::LE); break;
        case lexer::TokenType::GE:         emit(OpCode::GE); break;
        case lexer::TokenType::AND:        emit(OpCode::AND); break;
        case lexer::TokenType::OR:         emit(OpCode::OR); break;
        default:
            error("Unknown binary operator");
    }
}

void Compiler::compileUnaryExpression(ast::UnaryExpression* expr) {
    // 피연산자
    compileExpression(expr->operand());

    // 연산자
    lexer::TokenType op = expr->op();
    switch (op) {
        case lexer::TokenType::MINUS:  emit(OpCode::NEG); break;
        case lexer::TokenType::NOT:    emit(OpCode::NOT); break;
        default:
            error("Unknown unary operator");
    }
}

void Compiler::compileCallExpression(ast::CallExpression* expr) {
    // 함수
    compileExpression(expr->function());

    // 인자들
    for (auto& arg : expr->arguments()) {
        compileExpression(arg.get());
    }

    // 호출
    emit(OpCode::CALL, static_cast<uint8_t>(expr->arguments().size()));
}

void Compiler::compileArrayLiteral(ast::ArrayLiteral* lit) {
    // 요소들
    for (auto& elem : lit->elements()) {
        compileExpression(elem.get());
    }

    // 배열 생성
    emit(OpCode::BUILD_ARRAY, static_cast<uint8_t>(lit->elements().size()));
}

void Compiler::compileIndexExpression(ast::IndexExpression* expr) {
    // 배열
    compileExpression(expr->array());

    // 인덱스
    compileExpression(expr->index());

    // 인덱스 접근
    emit(OpCode::INDEX_GET);
}

void Compiler::compileFunctionLiteral(ast::FunctionLiteral* lit) {
    // TODO: 함수 컴파일 구현
    // 현재는 간단화를 위해 에러
    error("Function literal not yet fully implemented");
}

void Compiler::compileJosaExpression(ast::JosaExpression* expr) {
    // 객체
    compileExpression(expr->object());

    // 메서드 이름
    size_t methodIdx = chunk_->addName(expr->method()->value());

    // 조사 타입 (간단화를 위해 0으로)
    emit(OpCode::JOSA_CALL, 0, static_cast<uint8_t>(methodIdx));
}

void Compiler::compileRangeExpression(ast::RangeExpression* expr) {
    // 시작값
    compileExpression(expr->start());

    // 끝값
    compileExpression(expr->end());

    // 범위 생성 (포함 플래그)
    uint8_t flags = 0;
    if (expr->inclusiveStart()) flags |= 0x01;
    if (expr->inclusiveEnd()) flags |= 0x02;

    emit(OpCode::BUILD_RANGE, flags, 0);
}

// ============================================================================
// 스코프 관리
// ============================================================================

void Compiler::beginScope() {
    scopeDepth_++;
}

void Compiler::endScope() {
    scopeDepth_--;

    // 로컬 변수 제거
    while (!locals_.empty() && locals_.back().depth > scopeDepth_) {
        emit(OpCode::POP);
        locals_.pop_back();
    }
}

void Compiler::addLocal(const std::string& name) {
    if (locals_.size() >= 256) {
        error("Too many local variables in function");
        return;
    }

    locals_.push_back({name, scopeDepth_, false});
}

int Compiler::resolveLocal(const std::string& name) {
    for (int i = static_cast<int>(locals_.size()) - 1; i >= 0; i--) {
        if (locals_[i].name == name) {
            return i;
        }
    }
    return -1;
}

// ============================================================================
// 점프 관련
// ============================================================================

size_t Compiler::emitJump(OpCode op) {
    emit(op, 0xFF);  // 플레이스홀더
    return currentOffset() - 1;
}

void Compiler::patchJump(size_t offset) {
    size_t jump = currentOffset() - offset - 1;

    if (jump > 0xFF) {
        error("Too much code to jump over");
    }

    chunk_->patch(offset, static_cast<uint8_t>(jump));
}

void Compiler::emitLoop(size_t loopStart) {
    size_t offset = currentOffset() - loopStart + 2;

    if (offset > 0xFF) {
        error("Loop body too large");
    }

    emit(OpCode::LOOP, static_cast<uint8_t>(offset));
}

// ============================================================================
// 헬퍼
// ============================================================================

void Compiler::emit(OpCode op) {
    chunk_->writeOpCode(op, currentLine());
}

void Compiler::emit(OpCode op, uint8_t operand) {
    chunk_->writeOpCode(op, currentLine());
    chunk_->write(operand, currentLine());
}

void Compiler::emit(OpCode op, uint8_t operand1, uint8_t operand2) {
    chunk_->writeOpCode(op, currentLine());
    chunk_->write(operand1, currentLine());
    chunk_->write(operand2, currentLine());
}

size_t Compiler::currentOffset() const {
    return chunk_->size();
}

void Compiler::error(const std::string& message) {
    std::cerr << "[컴파일 오류] " << message << std::endl;
    hadError_ = true;
}

} // namespace bytecode
} // namespace kingsejong
