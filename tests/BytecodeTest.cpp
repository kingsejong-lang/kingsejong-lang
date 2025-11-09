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
    parser::Parser parser(&lexer);
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
    parser::Parser parser(&lexer);
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
    parser::Parser parser(&lexer);
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
// 실행
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
