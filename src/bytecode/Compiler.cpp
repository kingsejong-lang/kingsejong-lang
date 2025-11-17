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
        auto& statements = program->statements();
        size_t stmtCount = statements.size();

        for (size_t i = 0; i < stmtCount; i++) {
            bool isLastStatement = (i == stmtCount - 1);

            // 마지막 statement가 ExpressionStatement면 결과를 스택에 남김 (REPL 스타일)
            if (isLastStatement && statements[i]->type() == ast::NodeType::EXPRESSION_STATEMENT) {
                auto* exprStmt = static_cast<ast::ExpressionStatement*>(statements[i].get());
                compileExpression(const_cast<ast::Expression*>(exprStmt->expression()));
                // POP 하지 않음 - 결과를 스택에 남김
            } else {
                compileStatement(statements[i].get());
            }
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

    switch (stmt->type()) {
        case ast::NodeType::VAR_DECLARATION:
            compileVarDeclaration(static_cast<ast::VarDeclaration*>(stmt));
            break;
        case ast::NodeType::ASSIGNMENT_STATEMENT:
            compileAssignmentStatement(static_cast<ast::AssignmentStatement*>(stmt));
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
        case ast::NodeType::CLASS_STATEMENT:  // Phase 7.1
            compileClassStatement(static_cast<ast::ClassStatement*>(stmt));
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

    switch (expr->type()) {
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
        case ast::NodeType::NEW_EXPRESSION:  // Phase 7.1
            compileNewExpression(static_cast<ast::NewExpression*>(expr));
            break;
        case ast::NodeType::MEMBER_ACCESS_EXPRESSION:  // Phase 7.1
            compileMemberAccessExpression(static_cast<ast::MemberAccessExpression*>(expr));
            break;
        case ast::NodeType::THIS_EXPRESSION:  // Phase 7.1
            compileThisExpression(static_cast<ast::ThisExpression*>(expr));
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
    if (decl->initializer()) {
        compileExpression(const_cast<ast::Expression*>(decl->initializer()));
    } else {
        emit(OpCode::LOAD_NULL);
    }

    if (scopeDepth_ == 0) {
        // 전역 변수
        size_t nameIdx = chunk_->addName(decl->varName());
        emit(OpCode::STORE_GLOBAL, static_cast<uint8_t>(nameIdx));
    } else {
        // 로컬 변수
        addLocal(decl->varName());
    }
}

void Compiler::compileAssignmentStatement(ast::AssignmentStatement* stmt) {
    // 값을 컴파일 (스택에 푸시)
    compileExpression(const_cast<ast::Expression*>(stmt->value()));

    // 변수에 저장 (타입 추론: 없으면 생성, 있으면 갱신)
    if (scopeDepth_ == 0) {
        // 전역 변수
        size_t nameIdx = chunk_->addName(stmt->varName());
        emit(OpCode::STORE_GLOBAL, static_cast<uint8_t>(nameIdx));
    } else {
        // 로컬 변수: 기존 변수 찾기
        int local = resolveLocal(stmt->varName());
        if (local == -1) {
            // 새 로컬 변수 생성 (타입 추론)
            addLocal(stmt->varName());
        }
        // 로컬 변수는 자동으로 스택에서 관리됨
    }
}

void Compiler::compileExpressionStatement(ast::ExpressionStatement* stmt) {
    compileExpression(const_cast<ast::Expression*>(stmt->expression()));
    emit(OpCode::POP);  // 표현식 결과 제거
}

void Compiler::compileIfStatement(ast::IfStatement* stmt) {
    // 최적화: 상수 조건 검사 (데드 코드 제거)
    bool constResult;
    if (isConstantCondition(const_cast<ast::Expression*>(stmt->condition()), constResult)) {
        if (constResult) {
            // 조건이 항상 참 - then 블록만 컴파일
            compileStatement(const_cast<ast::BlockStatement*>(static_cast<const ast::BlockStatement*>(stmt->thenBranch())));
        } else {
            // 조건이 항상 거짓 - else 블록만 컴파일
            if (stmt->elseBranch()) {
                compileStatement(const_cast<ast::BlockStatement*>(static_cast<const ast::BlockStatement*>(stmt->elseBranch())));
            }
        }
        return;
    }

    // 조건 평가
    compileExpression(const_cast<ast::Expression*>(stmt->condition()));

    // 거짓이면 else로 점프
    size_t thenJump = emitJump(OpCode::JUMP_IF_FALSE);

    // then 블록
    emit(OpCode::POP);  // 조건 값 제거
    compileStatement(const_cast<ast::BlockStatement*>(static_cast<const ast::BlockStatement*>(stmt->thenBranch())));

    // else 블록 건너뛰기
    size_t elseJump = emitJump(OpCode::JUMP);

    // else 블록
    patchJump(thenJump);
    emit(OpCode::POP);  // 조건 값 제거

    if (stmt->elseBranch()) {
        compileStatement(const_cast<ast::BlockStatement*>(static_cast<const ast::BlockStatement*>(stmt->elseBranch())));
    }

    patchJump(elseJump);
}

void Compiler::compileWhileStatement(ast::WhileStatement* stmt) {
    size_t loopStart = currentOffset();
    loopStarts_.push_back(loopStart);
    breakJumps_.push_back({});

    // 조건 평가
    compileExpression(const_cast<ast::Expression*>(stmt->condition()));

    // 거짓이면 루프 종료
    size_t exitJump = emitJump(OpCode::JUMP_IF_FALSE);
    emit(OpCode::POP);  // 조건 값 제거

    // 루프 본문
    compileStatement(const_cast<ast::BlockStatement*>(static_cast<const ast::BlockStatement*>(stmt->body())));

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

void Compiler::compileForStatement([[maybe_unused]] ast::ForStatement* stmt) {
    // TODO: 일반 for 문 구현
    error("For statement not yet implemented");
}

void Compiler::compileReturnStatement(ast::ReturnStatement* stmt) {
    if (stmt->returnValue()) {
        compileExpression(const_cast<ast::Expression*>(stmt->returnValue()));
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
    compileExpression(const_cast<ast::Expression*>(stmt->count()));

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
    compileStatement(const_cast<ast::BlockStatement*>(static_cast<const ast::BlockStatement*>(stmt->body())));

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

void Compiler::compileRangeForStatement([[maybe_unused]] ast::RangeForStatement* stmt) {
    beginScope();

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
    int local = resolveLocal(ident->name());

    if (local != -1) {
        // 로컬 변수
        emit(OpCode::LOAD_VAR, static_cast<uint8_t>(local));
    } else {
        // 전역 변수
        size_t nameIdx = chunk_->addName(ident->name());
        emit(OpCode::LOAD_GLOBAL, static_cast<uint8_t>(nameIdx));
    }
}

void Compiler::compileBinaryExpression(ast::BinaryExpression* expr) {
    // 최적화: 상수 폴딩
    if (tryConstantFoldBinary(expr)) {
        return;
    }

    // 왼쪽 피연산자
    compileExpression(const_cast<ast::Expression*>(expr->left()));

    // 오른쪽 피연산자
    compileExpression(const_cast<ast::Expression*>(expr->right()));

    // 연산자
    const std::string& op = expr->op();
    if (op == "+")          emit(OpCode::ADD);
    else if (op == "-")     emit(OpCode::SUB);
    else if (op == "*")     emit(OpCode::MUL);
    else if (op == "/")     emit(OpCode::DIV);
    else if (op == "%")     emit(OpCode::MOD);
    else if (op == "==")    emit(OpCode::EQ);
    else if (op == "!=")    emit(OpCode::NE);
    else if (op == "<")     emit(OpCode::LT);
    else if (op == ">")     emit(OpCode::GT);
    else if (op == "<=")    emit(OpCode::LE);
    else if (op == ">=")    emit(OpCode::GE);
    else if (op == "&&" || op == "그리고") emit(OpCode::AND);
    else if (op == "||" || op == "또는")   emit(OpCode::OR);
    else error("Unknown binary operator: " + op);
}

void Compiler::compileUnaryExpression(ast::UnaryExpression* expr) {
    // 최적화: 상수 폴딩
    if (tryConstantFoldUnary(expr)) {
        return;
    }

    // 피연산자
    compileExpression(const_cast<ast::Expression*>(expr->operand()));

    // 연산자
    const std::string& op = expr->op();
    if (op == "-" || op == "음수") emit(OpCode::NEG);
    else if (op == "!" || op == "아님") emit(OpCode::NOT);
    else error("Unknown unary operator: " + op);
}

void Compiler::compileCallExpression(ast::CallExpression* expr) {
    // Phase 7.1: CallExpression이 실제로는 클래스 인스턴스화인지 확인
    // 함수 표현식이 Identifier이고, 그 이름이 클래스 이름이면 NEW_INSTANCE 사용
    if (auto* ident = dynamic_cast<ast::Identifier*>(const_cast<ast::Expression*>(expr->function()))) {
        if (classNames_.count(ident->name()) > 0) {
            // 클래스 인스턴스화
            size_t classNameIdx = chunk_->addConstant(evaluator::Value::createString(ident->name()));

            // 생성자 인자들
            for (auto& arg : expr->arguments()) {
                compileExpression(const_cast<ast::Expression*>(arg.get()));
            }

            // NEW_INSTANCE [class_name_index] [arg_count]
            emit(OpCode::NEW_INSTANCE, static_cast<uint8_t>(classNameIdx),
                 static_cast<uint8_t>(expr->arguments().size()));
            return;
        }
    }

    // 일반 함수 호출
    compileExpression(const_cast<ast::Expression*>(expr->function()));

    // 인자들
    for (auto& arg : expr->arguments()) {
        compileExpression(const_cast<ast::Expression*>(arg.get()));
    }

    // 호출
    emit(OpCode::CALL, static_cast<uint8_t>(expr->arguments().size()));
}

void Compiler::compileArrayLiteral(ast::ArrayLiteral* lit) {
    // 요소들
    for (auto& elem : lit->elements()) {
        compileExpression(const_cast<ast::Expression*>(elem.get()));
    }

    // 배열 생성
    emit(OpCode::BUILD_ARRAY, static_cast<uint8_t>(lit->elements().size()));
}

void Compiler::compileIndexExpression(ast::IndexExpression* expr) {
    // 배열
    compileExpression(const_cast<ast::Expression*>(expr->array()));

    // 인덱스
    compileExpression(const_cast<ast::Expression*>(expr->index()));

    // 인덱스 접근
    emit(OpCode::INDEX_GET);
}

void Compiler::compileFunctionLiteral(ast::FunctionLiteral* lit) {
    // 함수를 별도의 청크로 컴파일
    // 간단화를 위해 현재 청크에 인라인으로 컴파일

    // 함수 본체 시작 위치로 점프
    size_t jumpOver = emitJump(OpCode::JUMP);

    // 함수 본체 시작
    size_t functionStart = currentOffset();

    // 새 스코프 시작
    beginScope();

    // 파라미터를 로컬 변수로 추가
    for (const auto& param : lit->parameters()) {
        addLocal(param);
    }

    // 함수 본체 컴파일
    compileBlockStatement(static_cast<ast::BlockStatement*>(lit->body()));

    // 암시적 return null
    emit(OpCode::LOAD_NULL);
    emit(OpCode::RETURN);

    // 스코프 종료
    endScope();

    // 점프 패치
    patchJump(jumpOver);

    // 함수 객체 생성
    emit(OpCode::BUILD_FUNCTION,
         static_cast<uint8_t>(lit->parameters().size()),
         static_cast<uint8_t>((functionStart >> 8) & 0xFF));
    chunk_->write(static_cast<uint8_t>(functionStart & 0xFF), currentLine());
}

void Compiler::compileJosaExpression(ast::JosaExpression* expr) {
    // 객체
    compileExpression(const_cast<ast::Expression*>(expr->object()));

    // 메서드 이름
    const ast::Identifier* methodIdent = static_cast<const ast::Identifier*>(expr->method());
    size_t methodIdx = chunk_->addName(methodIdent->name());

    // 조사 타입 (간단화를 위해 0으로)
    emit(OpCode::JOSA_CALL, 0, static_cast<uint8_t>(methodIdx));
}

void Compiler::compileRangeExpression(ast::RangeExpression* expr) {
    // 시작값
    compileExpression(const_cast<ast::Expression*>(expr->start()));

    // 끝값
    compileExpression(const_cast<ast::Expression*>(expr->end()));

    // 범위 생성 (포함 플래그)
    uint8_t flags = 0;
    if (expr->startInclusive()) flags |= 0x01;
    if (expr->endInclusive()) flags |= 0x02;

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

// ============================================================================
// 최적화 패스
// ============================================================================

bool Compiler::tryConstantFoldBinary(ast::BinaryExpression* expr) {
    // 정수 리터럴 폴딩
    auto* leftInt = dynamic_cast<ast::IntegerLiteral*>(const_cast<ast::Expression*>(expr->left()));
    auto* rightInt = dynamic_cast<ast::IntegerLiteral*>(const_cast<ast::Expression*>(expr->right()));

    if (leftInt && rightInt) {
        int64_t left = leftInt->value();
        int64_t right = rightInt->value();
        int64_t result;
        bool isBoolResult = false;
        bool boolResult = false;

        const std::string& op = expr->op();
        if (op == "+") {
            result = left + right;
        } else if (op == "-") {
            result = left - right;
        } else if (op == "*") {
            result = left * right;
        } else if (op == "/") {
            if (right == 0) return false;
            result = left / right;
        } else if (op == "%") {
            if (right == 0) return false;
            result = left % right;
        } else if (op == "==") {
            isBoolResult = true;
            boolResult = (left == right);
        } else if (op == "!=") {
            isBoolResult = true;
            boolResult = (left != right);
        } else if (op == "<") {
            isBoolResult = true;
            boolResult = (left < right);
        } else if (op == ">") {
            isBoolResult = true;
            boolResult = (left > right);
        } else if (op == "<=") {
            isBoolResult = true;
            boolResult = (left <= right);
        } else if (op == ">=") {
            isBoolResult = true;
            boolResult = (left >= right);
        } else {
            return false;  // 폴딩 불가능한 연산
        }

        if (isBoolResult) {
            emit(boolResult ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE);
        } else {
            size_t idx = chunk_->addConstant(evaluator::Value::createInteger(result));
            emit(OpCode::LOAD_CONST, static_cast<uint8_t>(idx));
        }
        return true;
    }

    // 부동소수점 리터럴 폴딩
    auto* leftFloat = dynamic_cast<ast::FloatLiteral*>(const_cast<ast::Expression*>(expr->left()));
    auto* rightFloat = dynamic_cast<ast::FloatLiteral*>(const_cast<ast::Expression*>(expr->right()));

    if (leftFloat && rightFloat) {
        double left = leftFloat->value();
        double right = rightFloat->value();
        double result;
        bool isBoolResult = false;
        bool boolResult = false;

        const std::string& op = expr->op();
        if (op == "+") {
            result = left + right;
        } else if (op == "-") {
            result = left - right;
        } else if (op == "*") {
            result = left * right;
        } else if (op == "/") {
            if (right == 0.0) return false;
            result = left / right;
        } else if (op == "==") {
            isBoolResult = true;
            boolResult = (left == right);
        } else if (op == "!=") {
            isBoolResult = true;
            boolResult = (left != right);
        } else if (op == "<") {
            isBoolResult = true;
            boolResult = (left < right);
        } else if (op == ">") {
            isBoolResult = true;
            boolResult = (left > right);
        } else if (op == "<=") {
            isBoolResult = true;
            boolResult = (left <= right);
        } else if (op == ">=") {
            isBoolResult = true;
            boolResult = (left >= right);
        } else {
            return false;
        }

        if (isBoolResult) {
            emit(boolResult ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE);
        } else {
            size_t idx = chunk_->addConstant(evaluator::Value::createFloat(result));
            emit(OpCode::LOAD_CONST, static_cast<uint8_t>(idx));
        }
        return true;
    }

    return false;
}

bool Compiler::tryConstantFoldUnary(ast::UnaryExpression* expr) {
    // 정수 리터럴 단항 연산 폴딩
    auto* intLit = dynamic_cast<ast::IntegerLiteral*>(const_cast<ast::Expression*>(expr->operand()));
    if (intLit && expr->op() == "-") {
        int64_t result = -intLit->value();
        size_t idx = chunk_->addConstant(evaluator::Value::createInteger(result));
        emit(OpCode::LOAD_CONST, static_cast<uint8_t>(idx));
        return true;
    }

    // 부동소수점 리터럴 단항 연산 폴딩
    auto* floatLit = dynamic_cast<ast::FloatLiteral*>(const_cast<ast::Expression*>(expr->operand()));
    if (floatLit && expr->op() == "-") {
        double result = -floatLit->value();
        size_t idx = chunk_->addConstant(evaluator::Value::createFloat(result));
        emit(OpCode::LOAD_CONST, static_cast<uint8_t>(idx));
        return true;
    }

    // 불린 NOT 연산 폴딩
    auto* boolLit = dynamic_cast<ast::BooleanLiteral*>(const_cast<ast::Expression*>(expr->operand()));
    if (boolLit && (expr->op() == "!" || expr->op() == "아님")) {
        emit(boolLit->value() ? OpCode::LOAD_FALSE : OpCode::LOAD_TRUE);
        return true;
    }

    return false;
}

bool Compiler::isConstantCondition(ast::Expression* expr, bool& result) {
    // 불린 리터럴
    auto* boolLit = dynamic_cast<ast::BooleanLiteral*>(expr);
    if (boolLit) {
        result = boolLit->value();
        return true;
    }

    // 정수 리터럴 (0은 거짓, 나머지는 참)
    auto* intLit = dynamic_cast<ast::IntegerLiteral*>(expr);
    if (intLit) {
        result = (intLit->value() != 0);
        return true;
    }

    return false;
}

// ============================================================================
// Phase 7.1: 클래스 시스템 컴파일
// ============================================================================

void Compiler::compileClassStatement(ast::ClassStatement* stmt) {
    // 클래스 이름 기록 (Phase 7.1: CallExpression에서 class instantiation 구분용)
    classNames_.insert(stmt->className());

    // 클래스 이름을 상수 풀에 추가
    size_t classNameIdx = chunk_->addConstant(evaluator::Value::createString(stmt->className()));

    // 필드 개수와 메서드 개수
    uint8_t fieldCount = static_cast<uint8_t>(stmt->fields().size());
    uint8_t methodCount = static_cast<uint8_t>(stmt->methods().size());

    // CLASS_DEF [class_name_index] [field_count] [method_count]
    emit(OpCode::CLASS_DEF, static_cast<uint8_t>(classNameIdx), fieldCount);
    chunk_->write(methodCount, currentLine());

    // 필드 이름들을 상수 풀에 추가
    for (const auto& field : stmt->fields()) {
        size_t fieldNameIdx = chunk_->addConstant(evaluator::Value::createString(field->fieldName()));
        chunk_->write(static_cast<uint8_t>(fieldNameIdx), currentLine());
    }

    // 생성자 컴파일 (있으면)
    if (stmt->constructor()) {
        auto* ctor = stmt->constructor();
        // 생성자를 함수로 컴파일
        std::vector<std::string> params;
        for (const auto& param : ctor->parameters()) {
            params.push_back(param.name);
        }

        // 생성자 본문 컴파일 (나중에 VM에서 처리)
        // TODO: 생성자 바이트코드 생성
    }

    // 메서드 컴파일
    for (const auto& method : stmt->methods()) {
        // 메서드 이름을 상수 풀에 추가
        size_t methodNameIdx = chunk_->addConstant(evaluator::Value::createString(method->methodName()));
        chunk_->write(static_cast<uint8_t>(methodNameIdx), currentLine());

        // 메서드를 함수로 컴파일
        // TODO: 메서드 바이트코드 생성
    }

    // 클래스 정의를 전역 변수에 저장
    size_t globalIdx = chunk_->addName(stmt->className());
    emit(OpCode::STORE_GLOBAL, static_cast<uint8_t>(globalIdx));
}

void Compiler::compileNewExpression(ast::NewExpression* expr) {
    // 클래스 이름을 상수 풀에 추가
    size_t classNameIdx = chunk_->addConstant(evaluator::Value::createString(expr->className()));

    // 인자들을 컴파일 (스택에 푸시)
    for (const auto& arg : expr->arguments()) {
        compileExpression(const_cast<ast::Expression*>(arg.get()));
    }

    // NEW_INSTANCE [class_name_index] [arg_count]
    uint8_t argCount = static_cast<uint8_t>(expr->arguments().size());
    emit(OpCode::NEW_INSTANCE, static_cast<uint8_t>(classNameIdx), argCount);
}

void Compiler::compileMemberAccessExpression(ast::MemberAccessExpression* expr) {
    // 객체 표현식 컴파일 (스택에 객체 푸시)
    compileExpression(const_cast<ast::Expression*>(expr->object()));

    // 필드 이름을 상수 풀에 추가
    size_t fieldNameIdx = chunk_->addConstant(evaluator::Value::createString(expr->memberName()));

    // LOAD_FIELD [field_name_index]
    emit(OpCode::LOAD_FIELD, static_cast<uint8_t>(fieldNameIdx));
}

void Compiler::compileThisExpression(ast::ThisExpression* /* expr */) {
    // LOAD_THIS
    emit(OpCode::LOAD_THIS);
}

void Compiler::optimizePeephole() {
    // 간단한 피홀 최적화 패턴
    // 실제 구현은 생성된 바이트코드를 스캔하여 패턴을 찾아 최적화
    // 현재는 간단한 버전만 구현

    // TODO: 다음 패턴들을 최적화
    // 1. LOAD_CONST + POP -> 제거
    // 2. DUP + POP -> 제거
    // 3. LOAD_VAR X + STORE_VAR X -> 제거 (변수를 읽고 바로 같은 변수에 저장)
    // 4. 연속된 POP 최적화

    // 현재는 컴파일 타임 최적화가 대부분이므로 skip
}

} // namespace bytecode
} // namespace kingsejong
