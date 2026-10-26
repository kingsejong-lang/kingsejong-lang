/**
 * @file Compiler.cpp
 * @brief AST → 바이트코드 컴파일러 구현
 */

#include "Compiler.h"
#include "evaluator/Evaluator.h"
#include "lexer/JosaRecognizer.h"
#include <iostream>

namespace kingsejong {
namespace bytecode {

namespace {

uint16_t toU16(size_t value, const char* what) {
    if (value > 0xFFFF) {
        throw std::runtime_error(std::string(what) + lexer::JosaRecognizer::selectOrBoth(what, lexer::JosaRecognizer::JosaType::I_GA) + " 65535 를 넘습니다");
    }
    return static_cast<uint16_t>(value);
}

} // namespace

Compiler::Compiler()
    : chunk_(nullptr), hadError_(false), currentLine_(1), currentColumn_(0) {}

bool Compiler::compile(ast::Program* program, Chunk* chunk) {
    return compileProgram(program, chunk, true);
}

bool Compiler::compileModule(ast::Program* program, Chunk* chunk, size_t& entry) {
    entry = chunk->size();
    return compileProgram(program, chunk, false);
}

bool Compiler::compileProgram(ast::Program* program, Chunk* chunk, bool keepLastValue) {
    chunk_ = chunk;
    hadError_ = false;
    errors_.clear();
    currentFile_.clear();
    currentColumn_ = 0;

    try {
        auto& statements = program->statements();
        size_t count = statements.size();

        for (size_t i = 0; i < count; i++) {
            bool isLast = (i == count - 1);
            // 마지막 문장이 표현식 문장이면 값을 스택에 남긴다 (REPL 스타일, vm.top())
            if (keepLastValue && isLast && statements[i]->type() == ast::NodeType::EXPRESSION_STATEMENT) {
                auto* exprStmt = static_cast<ast::ExpressionStatement*>(statements[i].get());
                setLine(exprStmt);
                compileExpression(const_cast<ast::Expression*>(exprStmt->expression()));
            } else {
                compileStatement(statements[i].get());
            }
        }

        emit(OpCode::HALT);
        return !hadError_;
    } catch (const std::exception& e) {
        error(std::string("Compilation error: ") + e.what());
        return false;
    }
}

// ============================================================================
// 문장
// ============================================================================

void Compiler::compileStatement(ast::Statement* stmt) {
    if (!stmt) return;
    setLine(stmt);

    switch (stmt->type()) {
        case ast::NodeType::VAR_DECLARATION:
            compileVarDeclaration(static_cast<ast::VarDeclaration*>(stmt));
            break;
        case ast::NodeType::ASSIGNMENT_STATEMENT:
            compileAssignmentStatement(static_cast<ast::AssignmentStatement*>(stmt));
            break;
        case ast::NodeType::EXPRESSION_STATEMENT:
            compileExpression(const_cast<ast::Expression*>(
                static_cast<ast::ExpressionStatement*>(stmt)->expression()));
            emit(OpCode::POP);
            break;
        case ast::NodeType::IF_STATEMENT:
            compileIfStatement(static_cast<ast::IfStatement*>(stmt));
            break;
        case ast::NodeType::WHILE_STATEMENT:
            compileWhileStatement(static_cast<ast::WhileStatement*>(stmt));
            break;
        case ast::NodeType::REPEAT_STATEMENT:
            compileRepeatStatement(static_cast<ast::RepeatStatement*>(stmt));
            break;
        case ast::NodeType::RANGE_FOR_STATEMENT:
            compileRangeForStatement(static_cast<ast::RangeForStatement*>(stmt));
            break;
        case ast::NodeType::RETURN_STATEMENT:
            compileReturnStatement(static_cast<ast::ReturnStatement*>(stmt));
            break;
        case ast::NodeType::BLOCK_STATEMENT:
            compileBlock(static_cast<ast::BlockStatement*>(stmt));
            break;
        case ast::NodeType::IMPORT_STATEMENT:
            compileImportStatement(static_cast<ast::ImportStatement*>(stmt));
            break;
        case ast::NodeType::CLASS_STATEMENT:
            compileClassStatement(static_cast<ast::ClassStatement*>(stmt));
            break;
        case ast::NodeType::TRY_STATEMENT:
            compileTryStatement(static_cast<ast::TryStatement*>(stmt));
            break;
        case ast::NodeType::THROW_STATEMENT:
            compileThrowStatement(static_cast<ast::ThrowStatement*>(stmt));
            break;
        default:
            // 평가기도 처리하지 않는 문장 (일반 for 문 등)
            error("지원되지 않는 문장: " + ast::nodeTypeToString(stmt->type()));
    }
}

void Compiler::compileVarDeclaration(ast::VarDeclaration* decl) {
    if (decl->initializer()) {
        compileExpression(const_cast<ast::Expression*>(decl->initializer()));
    } else {
        emit(OpCode::LOAD_NULL);
    }
    emitName(OpCode::DECLARE, decl->varName());
}

void Compiler::compileAssignmentStatement(ast::AssignmentStatement* stmt) {
    // 바깥 스코프에 있으면 거기에, 없으면 현재 스코프에 (평가기의 setWithLookup)
    compileExpression(const_cast<ast::Expression*>(stmt->value()));
    emitName(OpCode::STORE_NAME, stmt->varName());
    emit(OpCode::POP);
}

void Compiler::compileIfStatement(ast::IfStatement* stmt) {
    // 상수 조건이면 한쪽만 컴파일한다
    bool constResult;
    if (isConstantCondition(stmt->condition(), constResult)) {
        if (constResult) {
            compileBlock(stmt->thenBranch());
        } else if (stmt->elseBranch()) {
            compileBlock(stmt->elseBranch());
        }
        return;
    }

    compileExpression(const_cast<ast::Expression*>(stmt->condition()));
    size_t elseJump = emitJump(OpCode::JUMP_IF_FALSE);
    compileBlock(stmt->thenBranch());

    if (stmt->elseBranch()) {
        size_t endJump = emitJump(OpCode::JUMP);
        patchJump(elseJump);
        compileBlock(stmt->elseBranch());
        patchJump(endJump);
    } else {
        patchJump(elseJump);
    }
}

void Compiler::compileWhileStatement(ast::WhileStatement* stmt) {
    size_t loopStart = currentOffset();
    compileExpression(const_cast<ast::Expression*>(stmt->condition()));
    size_t exitJump = emitJump(OpCode::JUMP_IF_FALSE);
    compileBlock(stmt->body());
    emitLoop(loopStart);
    patchJump(exitJump);
}

void Compiler::compileRepeatStatement(ast::RepeatStatement* stmt) {
    // [남은 횟수] 를 스택에 두고 돈다
    compileExpression(const_cast<ast::Expression*>(stmt->count()));
    emit(OpCode::REPEAT_INIT);
    size_t loopStart = currentOffset();
    size_t exitJump = emitJump(OpCode::REPEAT_NEXT);
    compileBlock(stmt->body());
    emitLoop(loopStart);
    patchJump(exitJump);
}

void Compiler::compileRangeForStatement(ast::RangeForStatement* stmt) {
    // [끝, i] 를 스택에 두고, 매 회 반복 변수를 현재 환경에 넣는다 (평가기와 같다)
    compileExpression(const_cast<ast::Expression*>(stmt->start()));
    compileExpression(const_cast<ast::Expression*>(stmt->end()));
    emit(OpCode::RANGE_FOR_INIT);
    size_t loopStart = currentOffset();
    emit(OpCode::RANGE_FOR_NEXT);
    emit16(nameIndex(stmt->varName()));
    size_t exitOperand = currentOffset();
    emit16(0);
    emitByte(stmt->descending() ? 2 : (stmt->endInclusive() ? 1 : 0));  // 0 미만, 1 까지, 2 거꾸로(이상)
    compileBlock(stmt->body());
    emitLoop(loopStart);
    patchJump(exitOperand, 1);  // 탈출 오프셋 뒤에 끝포함 플래그 1바이트가 더 있다
}

void Compiler::compileReturnStatement(ast::ReturnStatement* stmt) {
    if (stmt->returnValue()) {
        compileExpression(const_cast<ast::Expression*>(stmt->returnValue()));
    } else {
        emit(OpCode::LOAD_NULL);
    }
    emit(OpCode::RETURN);
}

void Compiler::compileBlock(const ast::BlockStatement* block) {
    // 블록은 새 스코프를 만들지 않는다 (평가기와 같다)
    if (!block) return;
    for (const auto& s : block->statements()) {
        compileStatement(s.get());
    }
}

void Compiler::compileImportStatement(ast::ImportStatement* stmt) {
    emit(OpCode::IMPORT);
    emit16(nameIndex(stmt->modulePath()));
    emit16(nameIndex(stmt->location().filename));
}

void Compiler::compileClassStatement(ast::ClassStatement* stmt) {
    // 메서드·생성자 본문을 이 청크에 컴파일해 두면 VM 이 네이티브로 부른다.
    // 클래스 정의 자체는 평가기와 같은 규칙(makeClass)으로 런타임에 만든다.
    for (const auto& method : stmt->methods()) {
        std::vector<std::string> params;
        for (const auto& p : method->parameters()) params.push_back(p.name);
        compileFunctionBody(params, const_cast<ast::BlockStatement*>(method->body()), false,
                            stmt->className() + "." + method->methodName());
    }
    if (stmt->constructor()) {
        std::vector<std::string> params;
        for (const auto& p : stmt->constructor()->parameters()) params.push_back(p.name);
        compileFunctionBody(params, const_cast<ast::BlockStatement*>(stmt->constructor()->body()), false,
                            stmt->className() + ".생성자");
    }
    emit(OpCode::CLASS_DEF);
    emit16(toU16(chunk_->addNode(stmt), "AST 노드 수"));
}

void Compiler::compileTryStatement(ast::TryStatement* stmt) {
    // 배치:
    //   [finally 있음] TRY_PUSH none, Lfin        (F)
    //   [catch 있음]   TRY_PUSH Lcatch, none      (C)
    //                  <try 블록>
    //   [catch 있음]   TRY_POP ; JUMP Lafter
    //        Lcatch:   PUSH_SCOPE ; DECLARE 변수 ; <catch 블록> ; POP_SCOPE
    //        Lafter:
    //   [finally 있음] TRY_POP
    //        Lfin:     <finally 블록> ; END_FINALLY
    // 예외가 catch 로 가면 VM 이 에러 문자열을 스택에 올려 두고 Lcatch 로 점프한다.
    // finally 만 있는 핸들러로 가면 예외를 보류한 채 Lfin 을 돌리고 END_FINALLY 가 다시 던진다.
    bool hasCatch = !stmt->catchClauses().empty();
    bool hasFinally = stmt->finallyBlock() != nullptr;

    size_t finallyOperand = 0;
    if (hasFinally) {
        emit(OpCode::TRY_PUSH);
        emit16(NO_OFFSET);
        finallyOperand = currentOffset();
        emit16(0);
    }

    size_t catchOperand = 0;
    if (hasCatch) {
        emit(OpCode::TRY_PUSH);
        catchOperand = currentOffset();
        emit16(0);
        emit16(NO_OFFSET);
    }

    compileBlock(stmt->tryBlock());

    if (hasCatch) {
        emit(OpCode::TRY_POP);
        size_t afterJump = emitJump(OpCode::JUMP);

        patchJump(catchOperand, 2);  // catch 오프셋 뒤에 finally 오프셋 2바이트가 더 있다
        const auto& clause = stmt->catchClauses()[0];  // 현재는 단일 catch 만 (평가기와 같다)
        emit(OpCode::PUSH_SCOPE);
        emitName(OpCode::DECLARE, clause->errorVarName());
        compileBlock(clause->body());
        emit(OpCode::POP_SCOPE);

        patchJump(afterJump);
    }

    if (hasFinally) {
        emit(OpCode::TRY_POP);
        patchJump(finallyOperand);
        compileBlock(stmt->finallyBlock());
        emit(OpCode::END_FINALLY);
    }
}

void Compiler::compileThrowStatement(ast::ThrowStatement* stmt) {
    compileExpression(const_cast<ast::Expression*>(stmt->value()));
    emit(OpCode::THROW);
}

// ============================================================================
// 표현식
// ============================================================================

void Compiler::compileExpression(ast::Expression* expr) {
    if (!expr) {
        emit(OpCode::LOAD_NULL);
        return;
    }
    setLine(expr);

    switch (expr->type()) {
        case ast::NodeType::INTEGER_LITERAL:
            emitConstant(evaluator::Value::createInteger(static_cast<ast::IntegerLiteral*>(expr)->value()));
            break;
        case ast::NodeType::FLOAT_LITERAL:
            emitConstant(evaluator::Value::createFloat(static_cast<ast::FloatLiteral*>(expr)->value()));
            break;
        case ast::NodeType::STRING_LITERAL:
            emitConstant(evaluator::Value::createString(static_cast<ast::StringLiteral*>(expr)->value()));
            break;
        case ast::NodeType::INTERPOLATED_STRING:
            compileInterpolatedString(static_cast<ast::InterpolatedString*>(expr));
            break;
        case ast::NodeType::BOOLEAN_LITERAL:
            emit(static_cast<ast::BooleanLiteral*>(expr)->value() ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE);
            break;
        case ast::NodeType::IDENTIFIER:
            emitName(OpCode::LOAD_NAME, static_cast<ast::Identifier*>(expr)->name());
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
        case ast::NodeType::ARRAY_LITERAL: {
            auto* lit = static_cast<ast::ArrayLiteral*>(expr);
            for (auto& elem : lit->elements()) {
                compileExpression(const_cast<ast::Expression*>(elem.get()));
            }
            emit(OpCode::BUILD_ARRAY);
            emit16(toU16(lit->elements().size(), "배열 리터럴 요소 수"));
            break;
        }
        case ast::NodeType::DICTIONARY_LITERAL: {
            auto* lit = static_cast<ast::DictionaryLiteral*>(expr);
            for (const auto& [key, value] : lit->pairs()) {
                compileExpression(const_cast<ast::Expression*>(key.get()));
                compileExpression(const_cast<ast::Expression*>(value.get()));
            }
            emit(OpCode::BUILD_DICT);
            emit16(toU16(lit->pairs().size(), "사전 리터럴 쌍 수"));
            break;
        }
        case ast::NodeType::INDEX_EXPRESSION:
            compileIndexExpression(static_cast<ast::IndexExpression*>(expr));
            break;
        case ast::NodeType::FUNCTION_LITERAL: {
            auto* lit = static_cast<ast::FunctionLiteral*>(expr);
            size_t proto = compileFunctionBody(lit->parameters(), lit->body(), false, "함수");
            emit(OpCode::MAKE_FUNCTION);
            emit16(toU16(proto, "함수 수"));
            break;
        }
        case ast::NodeType::ASYNC_FUNCTION_LITERAL: {
            auto* lit = static_cast<ast::AsyncFunctionLiteral*>(expr);
            size_t proto = compileFunctionBody(lit->parameters(), lit->body(), true, "비동기 함수");
            emit(OpCode::MAKE_FUNCTION);
            emit16(toU16(proto, "함수 수"));
            break;
        }
        case ast::NodeType::JOSA_EXPRESSION:
            compileJosaExpression(static_cast<ast::JosaExpression*>(expr));
            break;
        case ast::NodeType::MEMBER_ACCESS_EXPRESSION: {
            auto* access = static_cast<ast::MemberAccessExpression*>(expr);
            compileExpression(const_cast<ast::Expression*>(access->object()));
            setLine(expr);
            emitName(OpCode::LOAD_FIELD, access->memberName());
            break;
        }
        case ast::NodeType::THIS_EXPRESSION:
            emitName(OpCode::LOAD_NAME, "자신");
            break;
        case ast::NodeType::NEW_EXPRESSION: {
            auto* newExpr = static_cast<ast::NewExpression*>(expr);
            emitName(OpCode::LOAD_NAME, newExpr->className());
            compileArguments(newExpr->arguments());
            setLine(expr);
            emitCall(static_cast<uint8_t>(newExpr->arguments().size()), newExpr->className());
            break;
        }
        case ast::NodeType::AWAIT_EXPRESSION:
            compileExpression(const_cast<ast::Expression*>(static_cast<ast::AwaitExpression*>(expr)->argument()));
            setLine(expr);
            emit(OpCode::AWAIT);
            break;
        case ast::NodeType::MATCH_EXPRESSION:
            compileMatchExpression(static_cast<ast::MatchExpression*>(expr));
            break;
        default:
            // 평가기도 값으로 평가하지 않는 표현식 (단독 범위 표현식 등)
            error("지원되지 않는 표현식: " + ast::nodeTypeToString(expr->type()));
            emit(OpCode::LOAD_NULL);
    }
}

void Compiler::compileInterpolatedString(ast::InterpolatedString* interp) {
    const auto& parts = interp->parts();
    const auto& expressions = interp->expressions();
    size_t count = 0;
    for (size_t i = 0; i < parts.size(); ++i) {
        emitConstant(evaluator::Value::createString(parts[i]));
        ++count;
        if (i < expressions.size()) {
            compileExpression(const_cast<ast::Expression*>(expressions[i].get()));
            ++count;
        }
    }
    emit(OpCode::BUILD_STRING);
    emit16(toU16(count, "보간 조각 수"));
}

void Compiler::compileBinaryExpression(ast::BinaryExpression* expr) {
    if (expr->op() == "=") {
        compileAssignmentExpression(expr);
        return;
    }

    if (tryConstantFoldBinary(expr)) {
        return;
    }

    compileExpression(const_cast<ast::Expression*>(expr->left()));
    compileExpression(const_cast<ast::Expression*>(expr->right()));

    setLine(expr);   // 피연산자를 거치며 바뀐 위치를 이 식의 위치로 되돌린다 (에러 위치의 줄·열)
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
    else error("지원되지 않는 이항 연산자: " + op);
}

void Compiler::compileAssignmentExpression(ast::BinaryExpression* expr) {
    // 평가기는 우변을 먼저 평가한다
    compileExpression(const_cast<ast::Expression*>(expr->right()));

    const ast::Expression* left = expr->left();
    switch (left->type()) {
        case ast::NodeType::MEMBER_ACCESS_EXPRESSION: {
            auto* access = static_cast<const ast::MemberAccessExpression*>(left);
            compileExpression(const_cast<ast::Expression*>(access->object()));
            setLine(expr);
            emitName(OpCode::STORE_FIELD, access->memberName());
            break;
        }
        case ast::NodeType::IDENTIFIER:
            // 할당 표현식은 현재 스코프에 정의한다 (평가기의 env->set)
            emit(OpCode::DUP);
            emitName(OpCode::DECLARE, static_cast<const ast::Identifier*>(left)->name());
            break;
        case ast::NodeType::INDEX_EXPRESSION: {
            auto* index = static_cast<const ast::IndexExpression*>(left);
            compileExpression(const_cast<ast::Expression*>(index->array()));
            compileExpression(const_cast<ast::Expression*>(index->index()));
            setLine(expr);
            emit(OpCode::INDEX_SET);
            break;
        }
        default:
            error("할당의 좌변이 올바르지 않습니다 (변수, 필드, 배열[인덱스], 사전[키] 만 가능합니다)");
    }
}

void Compiler::compileUnaryExpression(ast::UnaryExpression* expr) {
    if (tryConstantFoldUnary(expr)) {
        return;
    }
    compileExpression(const_cast<ast::Expression*>(expr->operand()));
    setLine(expr);
    const std::string& op = expr->op();
    if (op == "-")      emit(OpCode::NEG);
    else if (op == "!") emit(OpCode::NOT);
    else error("지원되지 않는 단항 연산자: " + op);
}

bool Compiler::isNativePattern(const ast::Pattern* pattern) {
    if (!pattern) return false;
    switch (pattern->type()) {
        case ast::NodeType::LITERAL_PATTERN:
            return evaluator::Evaluator::isLiteralNode(static_cast<const ast::LiteralPattern*>(pattern)->value());
        case ast::NodeType::ARRAY_PATTERN:
            for (const auto& element : static_cast<const ast::ArrayPattern*>(pattern)->elements()) {
                if (!isNativePattern(element.get())) return false;
            }
            return true;
        default:
            return true;
    }
}

void Compiler::compileMatchExpression(ast::MatchExpression* expr) {
    // 리터럴 자리에 일반 표현식이 온 패턴(예: `1 + 2 ->`)은 환경이 있어야 값이 정해지므로
    // 그 매칭 전체를 평가기 규칙으로 돌린다. 파서가 만드는 패턴은 모두 리터럴 노드다.
    for (const auto& matchCase : expr->cases()) {
        if (!isNativePattern(matchCase.pattern())) {
            emit(OpCode::EVAL_NODE);
            emit16(toU16(chunk_->addNode(expr), "AST 노드 수"));
            return;
        }
    }

    // 스택: [값] 을 두고 사례마다 복사해 검사한다. 사례 환경은 PUSH_SCOPE 로 열어
    // 바인딩·가드·본문이 거기서 돌고, 맞지 않으면 POP_SCOPE 로 버린다.
    compileExpression(const_cast<ast::Expression*>(expr->value()));
    std::vector<size_t> endJumps;

    for (const auto& matchCase : expr->cases()) {
        emit(OpCode::PUSH_SCOPE);
        emit(OpCode::DUP);                                  // [값 값]
        emit(OpCode::MATCH_PATTERN);                        // [값 참/거짓]
        emit16(toU16(chunk_->addNode(matchCase.pattern()), "AST 노드 수"));
        size_t patternFail = emitJump(OpCode::JUMP_IF_FALSE);   // [값]

        size_t guardFail = 0;
        bool hasGuard = matchCase.guard() != nullptr;
        if (hasGuard) {
            compileExpression(const_cast<ast::Expression*>(matchCase.guard()));
            guardFail = emitJump(OpCode::JUMP_IF_FALSE);    // [값]
        }

        emit(OpCode::POP);                                  // []
        compileExpression(const_cast<ast::Expression*>(matchCase.body()));   // [결과]
        emit(OpCode::POP_SCOPE);
        endJumps.push_back(emitJump(OpCode::JUMP));

        patchJump(patternFail);
        if (hasGuard) patchJump(guardFail);
        emit(OpCode::POP_SCOPE);                            // [값]
    }

    emit(OpCode::POP);                                      // []
    emit(OpCode::MATCH_FAIL);
    for (size_t jump : endJumps) {
        patchJump(jump);
    }
}

void Compiler::compileArguments(const std::vector<std::unique_ptr<ast::Expression>>& args) {
    if (args.size() > 255) {
        error("인자가 255개를 넘습니다");
        return;
    }
    for (auto& arg : args) {
        compileExpression(const_cast<ast::Expression*>(arg.get()));
    }
}

void Compiler::compileCallExpression(ast::CallExpression* expr) {
    // 메서드 호출: 객체.이름(인자) — 클래스 인스턴스 또는 Promise
    if (expr->function()->type() == ast::NodeType::MEMBER_ACCESS_EXPRESSION) {
        auto* access = static_cast<const ast::MemberAccessExpression*>(expr->function());
        compileExpression(const_cast<ast::Expression*>(access->object()));
        compileArguments(expr->arguments());
        setLine(expr);   // 인자가 여러 줄에 걸쳐도 호출 명령은 호출식의 줄을 갖는다 (호출 스택의 위치)
        emit(OpCode::CALL_METHOD);
        emit16(nameIndex(access->memberName()));
        emitByte(static_cast<uint8_t>(expr->arguments().size()));
        return;
    }

    compileExpression(const_cast<ast::Expression*>(expr->function()));
    compileArguments(expr->arguments());
    setLine(expr);
    std::string callee = "<익명 함수>";
    if (expr->function()->type() == ast::NodeType::IDENTIFIER) {
        callee = static_cast<const ast::Identifier*>(expr->function())->name();
    }
    emitCall(static_cast<uint8_t>(expr->arguments().size()), callee);
}

void Compiler::emitCall(uint8_t argCount, const std::string& callee) {
    size_t at = currentOffset();
    emit(OpCode::CALL);
    emitByte(argCount);
    chunk_->setCallSiteName(at, callee);
}

void Compiler::compileIndexExpression(ast::IndexExpression* expr) {
    compileExpression(const_cast<ast::Expression*>(expr->array()));

    if (expr->index()->type() == ast::NodeType::RANGE_EXPRESSION) {
        auto* range = static_cast<const ast::RangeExpression*>(expr->index());
        compileExpression(const_cast<ast::Expression*>(range->start()));
        compileExpression(const_cast<ast::Expression*>(range->end()));
        uint8_t flags = 0;
        if (range->startInclusive()) flags |= 0x01;
        if (range->endInclusive()) flags |= 0x02;
        if (range->hasStep()) {
            compileExpression(const_cast<ast::Expression*>(range->step()));
            flags |= 0x04;
        }
        setLine(expr);
        emit(OpCode::SLICE);
        emitByte(flags);
        return;
    }

    compileExpression(const_cast<ast::Expression*>(expr->index()));
    setLine(expr);
    emit(OpCode::INDEX_GET);
}

void Compiler::compileJosaExpression(ast::JosaExpression* expr) {
    using JosaType = lexer::JosaRecognizer::JosaType;
    compileExpression(const_cast<ast::Expression*>(expr->object()));

    const ast::Expression* method = expr->method();
    uint8_t josa = static_cast<uint8_t>(expr->josaType());

    // 변환 관용구: "값을 <이름>으로 변환한다"
    if (method->type() == ast::NodeType::JOSA_EXPRESSION) {
        auto* inner = static_cast<const ast::JosaExpression*>(method);
        if (inner->josaType() == JosaType::RO_EURO &&
            inner->method()->type() == ast::NodeType::IDENTIFIER &&
            inner->object()->type() == ast::NodeType::IDENTIFIER) {
            const std::string& verb = static_cast<const ast::Identifier*>(inner->method())->name();
            if (verb == "변환한다" || verb == "바꾼다") {
                setLine(expr);
                emit(OpCode::CONVERT);
                emit16(nameIndex(static_cast<const ast::Identifier*>(inner->object())->name()));
                return;
            }
        }
    }

    std::string name;
    uint8_t argc = 0;
    uint8_t isCall = 0;
    if (method->type() == ast::NodeType::CALL_EXPRESSION) {
        auto* call = static_cast<const ast::CallExpression*>(method);
        isCall = 1;
        if (call->function()->type() == ast::NodeType::IDENTIFIER) {
            name = static_cast<const ast::Identifier*>(call->function())->name();
            compileArguments(call->arguments());
            argc = static_cast<uint8_t>(call->arguments().size());
        }
    } else if (method->type() == ast::NodeType::IDENTIFIER) {
        name = static_cast<const ast::Identifier*>(method)->name();
    }

    setLine(expr);
    emit(OpCode::JOSA);
    emit16(nameIndex(name));
    emitByte(josa);
    emitByte(argc);
    emitByte(isCall);
}

size_t Compiler::compileFunctionBody(const std::vector<std::string>& params, ast::Statement* body,
                                     bool isAsync, const std::string& name) {
    // 본문을 건너뛰는 점프
    size_t skip = emitJump(OpCode::JUMP);
    size_t entry = currentOffset();

    if (body && body->type() == ast::NodeType::BLOCK_STATEMENT) {
        compileBlock(static_cast<const ast::BlockStatement*>(body));
    } else if (body) {
        compileStatement(body);
    }
    // 암시적 반환
    emit(OpCode::LOAD_NULL);
    emit(OpCode::RETURN);

    patchJump(skip);

    FunctionProto proto;
    proto.parameters = params;
    proto.body = body;
    proto.entry = entry;
    proto.isAsync = isAsync;
    proto.name = name;
    chunk_->registerEntry(body, entry);
    return chunk_->addProto(std::move(proto));
}

// ============================================================================
// 최적화
// ============================================================================

bool Compiler::tryConstantFoldBinary(ast::BinaryExpression* expr) {
    const std::string& op = expr->op();
    const ast::Expression* left = expr->left();
    const ast::Expression* right = expr->right();

    if (left->type() == ast::NodeType::INTEGER_LITERAL && right->type() == ast::NodeType::INTEGER_LITERAL) {
        int64_t a = static_cast<const ast::IntegerLiteral*>(left)->value();
        int64_t b = static_cast<const ast::IntegerLiteral*>(right)->value();
        if (op == "+") { emitConstant(evaluator::Value::createInteger(a + b)); return true; }
        if (op == "-") { emitConstant(evaluator::Value::createInteger(a - b)); return true; }
        if (op == "*") { emitConstant(evaluator::Value::createInteger(a * b)); return true; }
        if (op == "/" && b != 0) { emitConstant(evaluator::Value::createInteger(a / b)); return true; }
        if (op == "%" && b != 0) { emitConstant(evaluator::Value::createInteger(a % b)); return true; }
        if (op == "==") { emit(a == b ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE); return true; }
        if (op == "!=") { emit(a != b ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE); return true; }
        if (op == "<")  { emit(a < b ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE); return true; }
        if (op == ">")  { emit(a > b ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE); return true; }
        if (op == "<=") { emit(a <= b ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE); return true; }
        if (op == ">=") { emit(a >= b ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE); return true; }
        return false;
    }

    if (left->type() == ast::NodeType::FLOAT_LITERAL && right->type() == ast::NodeType::FLOAT_LITERAL) {
        double a = static_cast<const ast::FloatLiteral*>(left)->value();
        double b = static_cast<const ast::FloatLiteral*>(right)->value();
        if (op == "+") { emitConstant(evaluator::Value::createFloat(a + b)); return true; }
        if (op == "-") { emitConstant(evaluator::Value::createFloat(a - b)); return true; }
        if (op == "*") { emitConstant(evaluator::Value::createFloat(a * b)); return true; }
        if (op == "/" && b != 0.0) { emitConstant(evaluator::Value::createFloat(a / b)); return true; }
        if (op == "==") { emit(a == b ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE); return true; }
        if (op == "!=") { emit(a != b ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE); return true; }
        if (op == "<")  { emit(a < b ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE); return true; }
        if (op == ">")  { emit(a > b ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE); return true; }
        if (op == "<=") { emit(a <= b ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE); return true; }
        if (op == ">=") { emit(a >= b ? OpCode::LOAD_TRUE : OpCode::LOAD_FALSE); return true; }
        return false;
    }

    return false;
}

bool Compiler::tryConstantFoldUnary(ast::UnaryExpression* expr) {
    const ast::Expression* operand = expr->operand();
    if (expr->op() == "-" && operand->type() == ast::NodeType::INTEGER_LITERAL) {
        emitConstant(evaluator::Value::createInteger(-static_cast<const ast::IntegerLiteral*>(operand)->value()));
        return true;
    }
    if (expr->op() == "-" && operand->type() == ast::NodeType::FLOAT_LITERAL) {
        emitConstant(evaluator::Value::createFloat(-static_cast<const ast::FloatLiteral*>(operand)->value()));
        return true;
    }
    if (expr->op() == "!" && operand->type() == ast::NodeType::BOOLEAN_LITERAL) {
        emit(static_cast<const ast::BooleanLiteral*>(operand)->value() ? OpCode::LOAD_FALSE : OpCode::LOAD_TRUE);
        return true;
    }
    return false;
}

bool Compiler::isConstantCondition(const ast::Expression* expr, bool& result) {
    if (expr->type() == ast::NodeType::BOOLEAN_LITERAL) {
        result = static_cast<const ast::BooleanLiteral*>(expr)->value();
        return true;
    }
    if (expr->type() == ast::NodeType::INTEGER_LITERAL) {
        result = static_cast<const ast::IntegerLiteral*>(expr)->value() != 0;
        return true;
    }
    return false;
}

// ============================================================================
// 코드 생성
// ============================================================================

void Compiler::emit(OpCode op) {
    chunk_->writeOpCode(op, currentLine_, currentColumn_);
}

void Compiler::emitByte(uint8_t byte) {
    chunk_->write(byte, currentLine_, currentColumn_);
}

void Compiler::emit16(uint16_t value) {
    chunk_->write16(value, currentLine_, currentColumn_);
}

void Compiler::emitConstant(const evaluator::Value& value) {
    emit(OpCode::LOAD_CONST);
    emit16(toU16(chunk_->addConstant(value), "상수 수"));
}

void Compiler::emitName(OpCode op, const std::string& name) {
    emit(op);
    emit16(nameIndex(name));
}

uint16_t Compiler::nameIndex(const std::string& name) {
    return toU16(chunk_->addName(name), "이름 수");
}

size_t Compiler::emitJump(OpCode op) {
    emit(op);
    size_t operand = currentOffset();
    emit16(0);
    return operand;
}

void Compiler::patchJump(size_t operandOffset, size_t trailingBytes) {
    // 점프 거리는 명령의 마지막 피연산자 다음 바이트부터 현재 위치까지
    size_t jump = currentOffset() - (operandOffset + 2 + trailingBytes);
    if (jump > 0xFFFF) {
        error("점프 거리가 65535 바이트를 넘습니다");
        return;
    }
    chunk_->patch16(operandOffset, static_cast<uint16_t>(jump));
}

void Compiler::emitLoop(size_t loopStart) {
    emit(OpCode::LOOP);
    // LOOP 의 피연산자 다음 바이트에서 loopStart 까지 되돌아간다
    size_t offset = currentOffset() + 2 - loopStart;
    if (offset > 0xFFFF) {
        error("반복 본문이 65535 바이트를 넘습니다");
        emit16(0);
        return;
    }
    emit16(static_cast<uint16_t>(offset));
}

size_t Compiler::currentOffset() const {
    return chunk_->size();
}

void Compiler::setLine(const ast::Node* node) {
    if (node && node->location().isValid()) {
        currentLine_ = node->location().line;
        currentColumn_ = node->location().column;
        if (node->location().filename != currentFile_) {
            currentFile_ = node->location().filename;
            chunk_->setFile(currentFile_);
        }
    }
}

void Compiler::error(const std::string& message) {
    std::cerr << "[컴파일 오류] " << message << std::endl;
    errors_.push_back(message);
    hadError_ = true;
}

} // namespace bytecode
} // namespace kingsejong
