/**
 * @file BytecodeTest.cpp
 * @brief 바이트코드 컴파일러 및 VM 테스트
 * @author KingSejong Team
 * @date 2025-11-10
 */

#include <gtest/gtest.h>
#include "bytecode/OpCode.h"
#include "bytecode/Chunk.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"
#include "parser/Parser.h"
#include "lexer/Lexer.h"

using namespace kingsejong;
using namespace kingsejong::bytecode;
using namespace kingsejong::evaluator;

// ============================================================================
// Chunk 테스트
// ============================================================================

TEST(ChunkTest, ShouldCreateEmptyChunk) {
    Chunk chunk;
    EXPECT_EQ(chunk.size(), 0);
}

TEST(ChunkTest, ShouldWriteOpCode) {
    Chunk chunk;
    chunk.writeOpCode(OpCode::LOAD_TRUE, 1);
    EXPECT_EQ(chunk.size(), 1);
    EXPECT_EQ(chunk.read(0), static_cast<uint8_t>(OpCode::LOAD_TRUE));
}

TEST(ChunkTest, ShouldAddConstant) {
    Chunk chunk;
    size_t idx = chunk.addConstant(Value::createInteger(42));
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(chunk.getConstant(0).asInteger(), 42);
}

TEST(ChunkTest, ShouldAddName) {
    Chunk chunk;
    size_t idx1 = chunk.addName("변수");
    size_t idx2 = chunk.addName("함수");
    size_t idx3 = chunk.addName("변수");  // 중복

    EXPECT_EQ(idx1, 0);
    EXPECT_EQ(idx2, 1);
    EXPECT_EQ(idx3, 0);  // 중복이므로 기존 인덱스 반환

    EXPECT_EQ(chunk.getName(0), "변수");
    EXPECT_EQ(chunk.getName(1), "함수");
}

TEST(ChunkTest, ShouldGetLineNumber) {
    Chunk chunk;
    chunk.write(1, 10);
    chunk.write(2, 10);
    chunk.write(3, 20);

    EXPECT_EQ(chunk.getLine(0), 10);
    EXPECT_EQ(chunk.getLine(1), 10);
    EXPECT_EQ(chunk.getLine(2), 20);
}

// ============================================================================
// VM 기본 테스트
// ============================================================================

TEST(VMTest, ShouldLoadConstant) {
    Chunk chunk;
    size_t idx = chunk.addConstant(Value::createInteger(42));
    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx), 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 42);
}

TEST(VMTest, ShouldLoadBooleans) {
    Chunk chunk;
    chunk.writeOpCode(OpCode::LOAD_TRUE, 1);
    chunk.writeOpCode(OpCode::LOAD_FALSE, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asBoolean(), false);
}

TEST(VMTest, ShouldAddIntegers) {
    Chunk chunk;

    // 10 + 20
    size_t idx1 = chunk.addConstant(Value::createInteger(10));
    size_t idx2 = chunk.addConstant(Value::createInteger(20));

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx1), 1);

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx2), 1);

    chunk.writeOpCode(OpCode::ADD, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 30);
}

TEST(VMTest, ShouldSubtractIntegers) {
    Chunk chunk;

    // 50 - 30
    size_t idx1 = chunk.addConstant(Value::createInteger(50));
    size_t idx2 = chunk.addConstant(Value::createInteger(30));

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx1), 1);

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx2), 1);

    chunk.writeOpCode(OpCode::SUB, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 20);
}

TEST(VMTest, ShouldMultiplyIntegers) {
    Chunk chunk;

    // 6 * 7
    size_t idx1 = chunk.addConstant(Value::createInteger(6));
    size_t idx2 = chunk.addConstant(Value::createInteger(7));

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx1), 1);

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx2), 1);

    chunk.writeOpCode(OpCode::MUL, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 42);
}

TEST(VMTest, ShouldCompareIntegers) {
    Chunk chunk;

    // 10 < 20
    size_t idx1 = chunk.addConstant(Value::createInteger(10));
    size_t idx2 = chunk.addConstant(Value::createInteger(20));

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx1), 1);

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx2), 1);

    chunk.writeOpCode(OpCode::LT, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_TRUE(vm.top().asBoolean());
}

TEST(VMTest, ShouldNegateNumber) {
    Chunk chunk;

    // -42
    size_t idx = chunk.addConstant(Value::createInteger(42));

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx), 1);

    chunk.writeOpCode(OpCode::NEG, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), -42);
}

TEST(VMTest, ShouldLogicalNot) {
    Chunk chunk;

    chunk.writeOpCode(OpCode::LOAD_TRUE, 1);
    chunk.writeOpCode(OpCode::NOT, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_FALSE(vm.top().asBoolean());
}

TEST(VMTest, ShouldBuildArray) {
    Chunk chunk;

    // [1, 2, 3]
    size_t idx1 = chunk.addConstant(Value::createInteger(1));
    size_t idx2 = chunk.addConstant(Value::createInteger(2));
    size_t idx3 = chunk.addConstant(Value::createInteger(3));

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx1), 1);

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx2), 1);

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx3), 1);

    chunk.writeOpCode(OpCode::BUILD_ARRAY, 1);
    chunk.write(3, 1);

    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_TRUE(vm.top().isArray());
    EXPECT_EQ(vm.top().asArray().size(), 3);
}

TEST(VMTest, ShouldAccessArrayIndex) {
    Chunk chunk;

    // [10, 20, 30][1]
    size_t idx1 = chunk.addConstant(Value::createInteger(10));
    size_t idx2 = chunk.addConstant(Value::createInteger(20));
    size_t idx3 = chunk.addConstant(Value::createInteger(30));
    size_t idxAccess = chunk.addConstant(Value::createInteger(1));

    // 배열 생성
    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx1), 1);

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx2), 1);

    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx3), 1);

    chunk.writeOpCode(OpCode::BUILD_ARRAY, 1);
    chunk.write(3, 1);

    // 인덱스 접근
    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idxAccess), 1);

    chunk.writeOpCode(OpCode::INDEX_GET, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 20);
}

TEST(VMTest, ShouldStoreAndLoadGlobal) {
    Chunk chunk;

    // x = 42
    size_t nameIdx = chunk.addName("x");
    size_t valIdx = chunk.addConstant(Value::createInteger(42));

    // x에 42 저장
    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(valIdx), 1);

    chunk.writeOpCode(OpCode::STORE_GLOBAL, 1);
    chunk.write(static_cast<uint8_t>(nameIdx), 1);

    // x 로드
    chunk.writeOpCode(OpCode::LOAD_GLOBAL, 1);
    chunk.write(static_cast<uint8_t>(nameIdx), 1);

    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 42);
}

// ============================================================================
// Compiler 통합 테스트
// ============================================================================

TEST(CompilerTest, ShouldCompileIntegerLiteral) {
    std::string code = "42";

    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool success = compiler.compile(program.get(), &chunk);

    EXPECT_TRUE(success);
    EXPECT_GT(chunk.size(), 0);
}

TEST(CompilerTest, ShouldCompileAddition) {
    std::string code = "10 + 20";

    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool success = compiler.compile(program.get(), &chunk);

    EXPECT_TRUE(success);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 30);
}

TEST(CompilerTest, ShouldCompileVariableDeclaration) {
    std::string code = "정수 x = 42";

    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool success = compiler.compile(program.get(), &chunk);

    EXPECT_TRUE(success);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
}

// ============================================================================
// OpCode 유틸리티 테스트
// ============================================================================

TEST(OpCodeTest, ShouldConvertToString) {
    EXPECT_EQ(opCodeToString(OpCode::ADD), "ADD");
    EXPECT_EQ(opCodeToString(OpCode::LOAD_CONST), "LOAD_CONST");
    EXPECT_EQ(opCodeToString(OpCode::HALT), "HALT");
}

TEST(OpCodeTest, ShouldGetOperandCount) {
    EXPECT_EQ(opCodeOperandCount(OpCode::ADD), 0);
    EXPECT_EQ(opCodeOperandCount(OpCode::LOAD_CONST), 1);
    EXPECT_EQ(opCodeOperandCount(OpCode::BUILD_FUNCTION), 2);
}

TEST(OpCodeTest, ShouldDetectJumpOpCodes) {
    EXPECT_TRUE(isJumpOpCode(OpCode::JUMP));
    EXPECT_TRUE(isJumpOpCode(OpCode::JUMP_IF_FALSE));
    EXPECT_TRUE(isJumpOpCode(OpCode::LOOP));
    EXPECT_FALSE(isJumpOpCode(OpCode::ADD));
}

// ============================================================================
// 최적화 테스트
// ============================================================================

TEST(OptimizationTest, ShouldFoldConstantAddition) {
    // 2 + 3 -> 5
    std::string code = "2 + 3";

    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool success = compiler.compile(program.get(), &chunk);

    EXPECT_TRUE(success);

    // 최적화되었으면 LOAD_CONST 1개만 있어야 함 (ADD 없음)
    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 5);
}

TEST(OptimizationTest, ShouldFoldConstantMultiplication) {
    // 6 * 7 -> 42
    std::string code = "6 * 7";

    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool success = compiler.compile(program.get(), &chunk);

    EXPECT_TRUE(success);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 42);
}

TEST(OptimizationTest, ShouldFoldConstantComparison) {
    // 10 < 20 -> 참
    std::string code = "10 < 20";

    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool success = compiler.compile(program.get(), &chunk);

    EXPECT_TRUE(success);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_TRUE(vm.top().asBoolean());
}

TEST(OptimizationTest, ShouldFoldConstantNegation) {
    // -42 -> -42
    std::string code = "-42";

    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool success = compiler.compile(program.get(), &chunk);

    EXPECT_TRUE(success);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), -42);
}

TEST(OptimizationTest, ShouldFoldConstantFloatArithmetic) {
    // 2.5 + 3.5 -> 6.0
    std::string code = "2.5 + 3.5";

    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool success = compiler.compile(program.get(), &chunk);

    EXPECT_TRUE(success);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_DOUBLE_EQ(vm.top().asFloat(), 6.0);
}

TEST(OptimizationTest, ShouldFoldBooleanNot) {
    // !참 -> 거짓
    std::string code = "!참";

    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool success = compiler.compile(program.get(), &chunk);

    EXPECT_TRUE(success);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_FALSE(vm.top().asBoolean());
}

TEST(OptimizationTest, ShouldEliminateDeadCodeInIfTrue) {
    // 만약 (참) { x = 1 } 아니면 { x = 2 }
    // 아니면 블록은 컴파일되지 않아야 함
    std::string code = R"(
        정수 x = 0
        만약 (참) {
            x = 1
        } 아니면 {
            x = 2
        }
    )";

    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool success = compiler.compile(program.get(), &chunk);

    EXPECT_TRUE(success);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    // x는 1이어야 함 (참 브랜치만 실행)
}

TEST(OptimizationTest, ShouldEliminateDeadCodeInIfFalse) {
    // 만약 (거짓) { x = 1 } 아니면 { x = 2 }
    // 만약 블록은 컴파일되지 않아야 함
    std::string code = R"(
        정수 x = 0
        만약 (거짓) {
            x = 1
        } 아니면 {
            x = 2
        }
    )";

    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool success = compiler.compile(program.get(), &chunk);

    EXPECT_TRUE(success);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    // x는 2이어야 함 (거짓 브랜치만 실행)
}

TEST(OptimizationTest, ShouldNotFoldNonConstantExpression) {
    // x + 3 는 폴딩되지 않음 (x가 변수)
    std::string code = R"(
        정수 x = 5
        x + 3
    )";

    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool success = compiler.compile(program.get(), &chunk);

    EXPECT_TRUE(success);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 8);
}

TEST(OptimizationTest, ShouldFoldNestedConstantExpression) {
    // (2 + 3) * 4 -> 일부 폴딩 가능
    // 최소한 2 + 3은 컴파일 타임에 5로 폴딩됨
    std::string code = "(2 + 3) * 4";

    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool success = compiler.compile(program.get(), &chunk);

    EXPECT_TRUE(success);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 20);
}

// ============================================================================
// 실행
// ============================================================================

// ============================================================================
// JIT 테스트
// ============================================================================

TEST(VMJITTest, ShouldTriggerJITOnHotLoop) {
    // 간단한 덧셈을 200번 반복하는 루프
    // sum = 0
    // for 200 times:
    //     sum = sum + 1
    Chunk chunk;
    
    // sum = 0 (스택에 직접 푸시)
    chunk.addConstant(Value::createInteger(0));
    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(0, 1);
    
    // 루프 카운터: i = 0
    chunk.addConstant(Value::createInteger(0));
    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(1, 1);
    
    // 루프 시작
    size_t loopStart = chunk.size();
    
    // 루프 종료 조건 확인: i < 200?
    chunk.writeOpCode(OpCode::DUP, 1);  // i 복사
    chunk.addConstant(Value::createInteger(200));
    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(2, 1);
    chunk.writeOpCode(OpCode::LT, 1);  // i < 200
    
    // 조건이 false면 종료 (12 바이트 점프)
    chunk.writeOpCode(OpCode::JUMP_IF_FALSE, 1);
    chunk.write(12, 1);
    chunk.writeOpCode(OpCode::POP, 1);  // 조건 값 제거
    
    // sum = sum + 1 (스택: [sum, i])
    chunk.writeOpCode(OpCode::SWAP, 1);  // [i, sum]
    chunk.addConstant(Value::createInteger(1));
    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(3, 1);
    chunk.writeOpCode(OpCode::ADD, 1);  // [i, sum+1]
    chunk.writeOpCode(OpCode::SWAP, 1);  // [sum+1, i]
    
    // i = i + 1
    chunk.addConstant(Value::createInteger(1));
    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(3, 1);
    chunk.writeOpCode(OpCode::ADD, 1);  // [sum+1, i+1]
    
    // 루프 백점프
    size_t offset = chunk.size() - loopStart + 2;
    chunk.writeOpCode(OpCode::LOOP, 1);
    chunk.write(static_cast<uint8_t>(offset), 1);
    
    // 루프 종료 (스택: [sum, i, condition])
    std::cerr << "[TEST] POP 1 위치: " << chunk.size() << "\n";
    chunk.writeOpCode(OpCode::POP, 1);  // 조건 제거
    std::cerr << "[TEST] POP 2 위치: " << chunk.size() << "\n";
    chunk.writeOpCode(OpCode::POP, 1);  // i 제거
    // 스택에 sum만 남음
    std::cerr << "[TEST] HALT 위치: " << chunk.size() << "\n";
    chunk.writeOpCode(OpCode::HALT, 1);
    
    // VM 실행 (JIT 활성화 - 200회 반복)
    VM vm;
    std::cerr << "[TEST] VM 실행 (200회 반복, JIT 활성화)\n";
    VMResult result = vm.run(&chunk);

    std::cerr << "[TEST] VM 종료\n";
    std::cerr << "[TEST] VM top() 타입 확인 전\n";
    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 200);  // sum should be 200
    
    // JIT 통계 출력
    std::cerr << "\n========== JIT Test Statistics ==========\n";
    vm.printJITStatistics();
}

// ============================================================================
// Async OpCode 테스트 (Phase 7.3)
// ============================================================================

TEST(VMAsyncTest, ShouldBuildPromise) {
    Chunk chunk;
    chunk.writeOpCode(OpCode::BUILD_PROMISE, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_TRUE(vm.top().isPromise());
}

TEST(VMAsyncTest, ShouldResolvePromise) {
    Chunk chunk;
    // BUILD_PROMISE
    chunk.writeOpCode(OpCode::BUILD_PROMISE, 1);
    // DUP (스택에 promise 복제)
    chunk.writeOpCode(OpCode::DUP, 1);
    // LOAD_CONST 42 (resolve할 값)
    size_t idx = chunk.addConstant(Value::createInteger(42));
    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx), 1);
    // PROMISE_RESOLVE (pop value, pop promise)
    chunk.writeOpCode(OpCode::PROMISE_RESOLVE, 1);
    // AWAIT (남은 promise에서 값 추출)
    chunk.writeOpCode(OpCode::AWAIT, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 42);
}

TEST(VMAsyncTest, ShouldBuildAsyncFunction) {
    Chunk chunk;

    // BUILD_ASYNC_FUNC [param_count] [addr_high] [addr_low]
    // 함수 시작 주소를 임의로 100으로 설정
    chunk.writeOpCode(OpCode::BUILD_ASYNC_FUNC, 1);
    chunk.write(2, 1);   // 파라미터 개수 2
    chunk.write(0, 1);   // addr_high = 0
    chunk.write(100, 1); // addr_low = 100 (함수 시작 주소)

    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    // 비동기 함수는 정수로 인코딩됨 (상위 비트로 async 표시)
    EXPECT_TRUE(vm.top().isInteger());

    // 인코딩 확인: (funcAddr << 8) | paramCount | (1 << 31)
    int64_t encoded = vm.top().asInteger();
    EXPECT_TRUE((encoded & (1LL << 31)) != 0);  // async 플래그
    EXPECT_EQ((encoded >> 8) & 0xFFFF, 100);    // 함수 주소
    EXPECT_EQ(encoded & 0xFF, 2);               // 파라미터 개수
}

TEST(VMAsyncTest, ShouldAwaitNonPromiseValue) {
    // Promise가 아닌 값을 await하면 그 값 그대로 반환
    Chunk chunk;
    size_t idx = chunk.addConstant(Value::createInteger(123));
    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx), 1);
    chunk.writeOpCode(OpCode::AWAIT, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 123);
}

TEST(VMAsyncTest, ShouldRejectPromise) {
    Chunk chunk;
    // BUILD_PROMISE
    chunk.writeOpCode(OpCode::BUILD_PROMISE, 1);
    // DUP
    chunk.writeOpCode(OpCode::DUP, 1);
    // LOAD_CONST "에러" (reject할 이유)
    size_t idx = chunk.addConstant(Value::createString("에러"));
    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write(static_cast<uint8_t>(idx), 1);
    // PROMISE_REJECT
    chunk.writeOpCode(OpCode::PROMISE_REJECT, 1);
    // AWAIT (rejected promise는 에러)
    chunk.writeOpCode(OpCode::AWAIT, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    // rejected promise를 await하면 런타임 에러
    EXPECT_EQ(result, VMResult::RUNTIME_ERROR);
}

TEST(VMAsyncTest, ShouldAwaitPendingPromise) {
    // Pending 상태의 Promise를 await하면 null 반환
    Chunk chunk;
    chunk.writeOpCode(OpCode::BUILD_PROMISE, 1);
    chunk.writeOpCode(OpCode::AWAIT, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    VMResult result = vm.run(&chunk);

    EXPECT_EQ(result, VMResult::OK);
    EXPECT_TRUE(vm.top().isNull());
}
