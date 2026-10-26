/**
 * @file BytecodeTest.cpp
 * @brief 바이트코드 컴파일러 및 VM 테스트
 */

#include <gtest/gtest.h>
#include "bytecode/OpCode.h"
#include "bytecode/Chunk.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "parser/Parser.h"
#include "lexer/Lexer.h"
#include "module/ModuleLoader.h"
#include "support/TestTempDir.h"
#include <filesystem>
#include <fstream>

using namespace kingsejong;
using namespace kingsejong::bytecode;
using namespace kingsejong::evaluator;

namespace {

/// 소스를 컴파일해 VM 으로 돌린다. 프로그램 AST 는 VM 이 끝날 때까지 살아 있어야 한다.
struct VmRun {
    std::unique_ptr<ast::Program> program;
    Chunk chunk;
    VM vm;
    bool compiled = false;
    VMResult result = VMResult::COMPILE_ERROR;

    explicit VmRun(const std::string& source, module::ModuleLoader* loader = nullptr) {
        Builtin::registerAllBuiltins();
        lexer::Lexer lexer(source);
        parser::Parser parser(lexer);
        program = parser.parseProgram();
        EXPECT_TRUE(parser.errors().empty()) << (parser.errors().empty() ? "" : parser.errors()[0]);
        Compiler compiler;
        compiled = compiler.compile(program.get(), &chunk);
        if (compiled) {
            vm.setModuleLoader(loader);
            result = vm.run(&chunk);
        }
    }

    /// 전역 함수의 본문이 이 청크에 컴파일되어 있는지 (바이트코드로 호출되는지)
    bool compiledInChunk(const std::string& name) const {
        Value value = global(name);
        if (!value.isFunction()) return false;
        size_t entry = 0;
        return chunk.findEntry(value.asFunction()->body(), entry);
    }

    Value global(const std::string& name) const { return vm.globals()->get(name); }
};

/// 같은 소스를 평가기로 돌린 전역 값
Value evaluatorGlobal(const std::string& source, const std::string& name) {
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    auto env = std::make_shared<Environment>();
    Value result;
    {
        Evaluator evaluator(env);
        evaluator.evalProgram(program.get());
        result = env->get(name);
    }
    env->releaseIfUnreachable(1);
    return result;
}

void writeConst(Chunk& chunk, const Value& value) {
    size_t idx = chunk.addConstant(value);
    chunk.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk.write16(static_cast<uint16_t>(idx), 1);
}

} // namespace

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
    EXPECT_EQ(idx3, 0);

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

TEST(ChunkTest, ShouldReadAndPatch16BitOperands) {
    Chunk chunk;
    chunk.writeOpCode(OpCode::JUMP, 1);
    chunk.write16(0, 1);
    chunk.patch16(1, 0x1234);
    EXPECT_EQ(chunk.read16(1), 0x1234);
}

TEST(ChunkTest, ShouldDisassembleWithOperands) {
    Chunk chunk;
    writeConst(chunk, Value::createInteger(7));
    chunk.writeOpCode(OpCode::HALT, 1);
    std::string text = chunk.disassemble("test");
    EXPECT_NE(text.find("LOAD_CONST"), std::string::npos);
    EXPECT_NE(text.find("'7'"), std::string::npos);
    EXPECT_NE(text.find("HALT"), std::string::npos);
}

// ============================================================================
// VM 기본 테스트 (손으로 쓴 바이트코드)
// ============================================================================

TEST(VMTest, ShouldLoadConstant) {
    Chunk chunk;
    writeConst(chunk, Value::createInteger(42));
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 42);
}

TEST(VMTest, ShouldLoadBooleans) {
    Chunk chunk;
    chunk.writeOpCode(OpCode::LOAD_TRUE, 1);
    chunk.writeOpCode(OpCode::LOAD_FALSE, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_EQ(vm.top().asBoolean(), false);
}

TEST(VMTest, ShouldAddIntegers) {
    Chunk chunk;
    writeConst(chunk, Value::createInteger(10));
    writeConst(chunk, Value::createInteger(20));
    chunk.writeOpCode(OpCode::ADD, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 30);
}

TEST(VMTest, ShouldSubtractIntegers) {
    Chunk chunk;
    writeConst(chunk, Value::createInteger(50));
    writeConst(chunk, Value::createInteger(30));
    chunk.writeOpCode(OpCode::SUB, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 20);
}

TEST(VMTest, ShouldMultiplyIntegers) {
    Chunk chunk;
    writeConst(chunk, Value::createInteger(6));
    writeConst(chunk, Value::createInteger(7));
    chunk.writeOpCode(OpCode::MUL, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 42);
}

TEST(VMTest, ShouldCompareIntegers) {
    Chunk chunk;
    writeConst(chunk, Value::createInteger(10));
    writeConst(chunk, Value::createInteger(20));
    chunk.writeOpCode(OpCode::LT, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_TRUE(vm.top().asBoolean());
}

TEST(VMTest, ShouldNegateNumber) {
    Chunk chunk;
    writeConst(chunk, Value::createInteger(42));
    chunk.writeOpCode(OpCode::NEG, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), -42);
}

TEST(VMTest, ShouldLogicalNot) {
    Chunk chunk;
    chunk.writeOpCode(OpCode::LOAD_TRUE, 1);
    chunk.writeOpCode(OpCode::NOT, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_FALSE(vm.top().asBoolean());
}

TEST(VMTest, ShouldConcatenateStrings) {
    Chunk chunk;
    writeConst(chunk, Value::createString("안녕"));
    writeConst(chunk, Value::createString("하세요"));
    chunk.writeOpCode(OpCode::ADD, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_EQ(vm.top().asString(), "안녕하세요");
}

TEST(VMTest, ShouldBuildArray) {
    Chunk chunk;
    writeConst(chunk, Value::createInteger(1));
    writeConst(chunk, Value::createInteger(2));
    writeConst(chunk, Value::createInteger(3));
    chunk.writeOpCode(OpCode::BUILD_ARRAY, 1);
    chunk.write16(3, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_TRUE(vm.top().isArray());
    EXPECT_EQ(vm.top().asArray().size(), 3);
}

TEST(VMTest, ShouldAccessArrayIndex) {
    Chunk chunk;
    writeConst(chunk, Value::createInteger(10));
    writeConst(chunk, Value::createInteger(20));
    writeConst(chunk, Value::createInteger(30));
    chunk.writeOpCode(OpCode::BUILD_ARRAY, 1);
    chunk.write16(3, 1);
    writeConst(chunk, Value::createInteger(1));
    chunk.writeOpCode(OpCode::INDEX_GET, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 20);
}

TEST(VMTest, ShouldDeclareAndLoadName) {
    Chunk chunk;
    size_t nameIdx = chunk.addName("x");
    writeConst(chunk, Value::createInteger(42));
    chunk.writeOpCode(OpCode::DECLARE, 1);
    chunk.write16(static_cast<uint16_t>(nameIdx), 1);
    chunk.writeOpCode(OpCode::LOAD_NAME, 1);
    chunk.write16(static_cast<uint16_t>(nameIdx), 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 42);
    EXPECT_EQ(vm.globals()->get("x").asInteger(), 42);
}

TEST(VMTest, ShouldResolveBuiltinNames) {
    Builtin::registerAllBuiltins();
    Chunk chunk;
    size_t nameIdx = chunk.addName("길이");
    chunk.writeOpCode(OpCode::LOAD_NAME, 1);
    chunk.write16(static_cast<uint16_t>(nameIdx), 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_TRUE(vm.top().isBuiltinFunction());
}

TEST(VMTest, ShouldReportUndefinedName) {
    Chunk chunk;
    size_t nameIdx = chunk.addName("없는이름");
    chunk.writeOpCode(OpCode::LOAD_NAME, 1);
    chunk.write16(static_cast<uint16_t>(nameIdx), 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::RUNTIME_ERROR);
}

// ============================================================================
// Compiler 통합 테스트
// ============================================================================

TEST(CompilerTest, ShouldCompileIntegerLiteral) {
    VmRun run("42");
    EXPECT_TRUE(run.compiled);
    EXPECT_GT(run.chunk.size(), 0);
}

TEST(CompilerTest, ShouldCompileAddition) {
    VmRun run("10 + 20");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.vm.top().asInteger(), 30);
}

TEST(CompilerTest, ShouldCompileVariableDeclaration) {
    VmRun run("정수 x = 42");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("x").asInteger(), 42);
}

TEST(CompilerTest, ShouldCompileStringLiteral) {
    VmRun run("\"안녕하세요\"");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.vm.top().asString(), "안녕하세요");
}

TEST(CompilerTest, ShouldCompileBooleanLiteral) {
    VmRun run("참");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_TRUE(run.vm.top().asBoolean());
}

TEST(CompilerTest, ShouldCompileFloatLiteral) {
    VmRun run("3.14");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_NEAR(run.vm.top().asFloat(), 3.14, 0.001);
}

TEST(CompilerTest, ShouldCompileComparison) {
    VmRun run("10 < 20");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_TRUE(run.vm.top().asBoolean());
}

TEST(CompilerTest, ShouldCompileMultiplication) {
    VmRun run("6 * 7");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.vm.top().asInteger(), 42);
}

TEST(CompilerTest, ShouldCompileSubtraction) {
    VmRun run("100 - 58");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.vm.top().asInteger(), 42);
}

TEST(CompilerTest, ShouldCompileDivision) {
    VmRun run("84 / 2");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.vm.top().asInteger(), 42);
}

TEST(CompilerTest, ShouldCompileNegation) {
    VmRun run("-42");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.vm.top().asInteger(), -42);
}

TEST(CompilerTest, ShouldCompileLogicalNot) {
    VmRun run("!참");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_FALSE(run.vm.top().asBoolean());
}

TEST(CompilerTest, ShouldCompileComplexExpression) {
    VmRun run("(10 + 20) * 2 - 18");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.vm.top().asInteger(), 42);
}

TEST(CompilerTest, ShouldRejectUnsupportedStatementsWithMessage) {
    // 평가기도 값으로 다루지 않는 단독 범위 표현식
    VmRun run("1부터 10까지");
    EXPECT_FALSE(run.compiled);
    ASSERT_FALSE(run.chunk.size() == 0);
}

// ============================================================================
// OpCode 유틸리티 테스트
// ============================================================================

TEST(OpCodeTest, ShouldConvertToString) {
    EXPECT_EQ(opCodeToString(OpCode::ADD), "ADD");
    EXPECT_EQ(opCodeToString(OpCode::LOAD_CONST), "LOAD_CONST");
    EXPECT_EQ(opCodeToString(OpCode::HALT), "HALT");
    EXPECT_EQ(opCodeToString(OpCode::JOSA), "JOSA");
}

TEST(OpCodeTest, ShouldGetOperandCountAndWidth) {
    EXPECT_EQ(opCodeOperandCount(OpCode::ADD), 0);
    EXPECT_EQ(opCodeOperandCount(OpCode::LOAD_CONST), 1);
    EXPECT_EQ(opCodeOperandBytes(OpCode::LOAD_CONST), 2u);
    EXPECT_EQ(opCodeOperandCount(OpCode::CALL), 1);
    EXPECT_EQ(opCodeOperandBytes(OpCode::CALL), 1u);
    EXPECT_EQ(opCodeOperandCount(OpCode::JOSA), 4);
    EXPECT_EQ(opCodeOperandBytes(OpCode::JOSA), 5u);
    EXPECT_EQ(opCodeOperandCount(OpCode::RANGE_FOR_NEXT), 3);
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
    VmRun run("2 + 3");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.vm.top().asInteger(), 5);
    // 폴딩되었으면 LOAD_CONST + HALT 만 있다 (ADD 없음)
    EXPECT_EQ(run.chunk.size(), 1u + 2u + 1u);
    EXPECT_EQ(run.chunk.constantCount(), 1u);
}

TEST(OptimizationTest, ShouldFoldConstantMultiplication) {
    VmRun run("6 * 7");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.vm.top().asInteger(), 42);
    EXPECT_EQ(run.chunk.constantCount(), 1u);
}

TEST(OptimizationTest, ShouldFoldConstantComparison) {
    VmRun run("10 < 20");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_TRUE(run.vm.top().asBoolean());
    EXPECT_EQ(run.chunk.constantCount(), 0u);  // LOAD_TRUE 로 폴딩
}

TEST(OptimizationTest, ShouldFoldConstantNegation) {
    VmRun run("-42");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.vm.top().asInteger(), -42);
    EXPECT_EQ(run.chunk.constantCount(), 1u);
}

TEST(OptimizationTest, ShouldFoldConstantFloatArithmetic) {
    VmRun run("2.5 + 3.5");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_DOUBLE_EQ(run.vm.top().asFloat(), 6.0);
    EXPECT_EQ(run.chunk.constantCount(), 1u);
}

TEST(OptimizationTest, ShouldFoldBooleanNot) {
    VmRun run("!참");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_FALSE(run.vm.top().asBoolean());
    EXPECT_EQ(run.chunk.constantCount(), 0u);
}

TEST(OptimizationTest, ShouldEliminateDeadCodeInIfTrue) {
    VmRun run(R"(
        정수 x = 0
        만약 (참) {
            x = 1
        } 아니면 {
            x = 2
        }
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("x").asInteger(), 1);
    // 아니면 블록의 상수 2 는 컴파일되지 않는다
    EXPECT_EQ(run.chunk.constantCount(), 2u);  // 0, 1
}

TEST(OptimizationTest, ShouldEliminateDeadCodeInIfFalse) {
    VmRun run(R"(
        정수 x = 0
        만약 (거짓) {
            x = 1
        } 아니면 {
            x = 2
        }
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("x").asInteger(), 2);
    EXPECT_EQ(run.chunk.constantCount(), 2u);  // 0, 2
}

TEST(OptimizationTest, ShouldNotFoldNonConstantExpression) {
    VmRun run(R"(
        정수 x = 5
        x + 3
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.vm.top().asInteger(), 8);
}

TEST(OptimizationTest, ShouldFoldNestedConstantExpression) {
    VmRun run("(2 + 3) * 4");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.vm.top().asInteger(), 20);
}

// ============================================================================
// VM 루프 테스트
// ============================================================================

TEST(VMLoopTest, ShouldRunWhileLoop) {
    VmRun run(R"(
        정수 합 = 0
        정수 i = 0
        동안 (i < 50) {
            합 = 합 + 1
            i = i + 1
        }
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("합").asInteger(), 50);
}

TEST(VMLoopTest, ShouldRunRangeForAndRepeat) {
    VmRun run(R"(
        정수 합 = 0
        i가 1부터 10까지 {
            합 = 합 + i
        }
        정수 횟수 = 0
        3번 반복한다 {
            횟수 = 횟수 + 1
        }
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("합").asInteger(), 55);
    EXPECT_EQ(run.global("횟수").asInteger(), 3);
    EXPECT_EQ(run.global("i").asInteger(), 10);  // 반복 변수는 평가기처럼 바깥에 남는다
}

TEST(VMLoopTest, ShouldHandleHighIterationCount) {
    VmRun run(R"(
        정수 합 = 0
        i가 1부터 10000까지 {
            합 = 합 + 1
        }
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("합").asInteger(), 10000);
}

TEST(VMLoopTest, ShouldRespectMaxInstructions) {
    // 무한 루프: LOAD_CONST 1; ADD; LOOP
    Chunk chunk;
    writeConst(chunk, Value::createInteger(0));
    size_t loopStart = chunk.size();
    writeConst(chunk, Value::createInteger(1));
    chunk.writeOpCode(OpCode::ADD, 1);
    chunk.writeOpCode(OpCode::LOOP, 1);
    chunk.write16(static_cast<uint16_t>(chunk.size() + 2 - loopStart), 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    vm.setMaxInstructions(1000);
    EXPECT_EQ(vm.run(&chunk), VMResult::RUNTIME_ERROR);
}

// ============================================================================
// 평가기와의 동치 (같은 소스, 같은 결과)
// ============================================================================

TEST(VMParityTest, ShouldCallFunctionsRecursively) {
    const std::string source = R"(
        함수 피보나치(n) {
            만약 (n < 2) { 반환 n }
            반환 피보나치(n - 1) + 피보나치(n - 2)
        }
        정수 결과 = 피보나치(15)
    )";
    VmRun run(source);
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("결과").asInteger(), 610);
    EXPECT_EQ(run.global("결과").asInteger(), evaluatorGlobal(source, "결과").asInteger());
}

TEST(VMParityTest, ShouldCaptureClosures) {
    VmRun run(R"(
        함수 만들기() {
            정수 횟수 = 0
            반환 함수() {
                횟수 = 횟수 + 1
                반환 횟수
            }
        }
        정수 세기 = 만들기()
        세기()
        세기()
        정수 결과 = 세기()
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("결과").asInteger(), 3);
}

TEST(VMParityTest, ShouldPassFunctionsToArrayMethods) {
    VmRun run(R"(
        정수 배열 = [3, 1, 2, 5, 4]
        정수 짝수 = 배열을 걸러낸다(함수(x) { 반환 x % 2 == 0 })
        정수 합 = 배열을 축약한다(0, 함수(누적, 현재) { 반환 누적 + 현재 })
        정수 정렬됨 = 배열을 정렬한다
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("짝수").asArray().size(), 2u);
    EXPECT_EQ(run.global("합").asInteger(), 15);
    EXPECT_EQ(run.global("정렬됨").asArray()[0].asInteger(), 1);
}

TEST(VMParityTest, ShouldCatchThrownErrorsAndRunFinally) {
    VmRun run(R"(
        정수 기록 = ""
        시도 {
            기록 = 기록 + "시도 "
            던지다 "문제"
            기록 = 기록 + "도달하지 않음 "
        } 오류 (e) {
            기록 = 기록 + e + " "
        } 마지막 {
            기록 = 기록 + "마지막"
        }
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("기록").asString(), "시도 Error: 문제 마지막");
}

TEST(VMParityTest, ShouldCatchRuntimeErrorsFromBuiltins) {
    VmRun run(R"(
        정수 결과 = "없음"
        시도 {
            정수 x = 10 / 0
        } 오류 (e) {
            결과 = "잡음"
        }
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("결과").asString(), "잡음");
}

TEST(VMParityTest, ShouldPropagateErrorsThroughFinallyInFunctions) {
    VmRun run(R"(
        정수 기록 = ""
        함수 위험() {
            시도 {
                던지다 "깊은 문제"
            } 마지막 {
                기록 = 기록 + "정리 "
            }
        }
        시도 {
            위험()
        } 오류 (e) {
            기록 = 기록 + e
        }
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("기록").asString(), "정리 Error: 깊은 문제");
}

TEST(VMParityTest, ShouldRunFinallyBeforeReturning) {
    VmRun run(R"(
        정수 기록 = ""
        함수 f() {
            시도 {
                반환 "값"
            } 마지막 {
                기록 = "마지막 실행"
            }
        }
        정수 결과 = f()
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("결과").asString(), "값");
    EXPECT_EQ(run.global("기록").asString(), "마지막 실행");
}

TEST(VMParityTest, ShouldBuildDictionariesAndInterpolate) {
    VmRun run(R"(
        사전 = {"이름": "세종", "나이": 30}
        사전["도시"] = "서울"
        이름 = 사전["이름"]
        나이 = 사전["나이"]
        도시 = 사전의 도시
        문장 = "${이름}은 ${나이}살, ${도시}"
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("문장").asString(), "세종은 30살, 서울");
}

TEST(VMParityTest, ShouldDefineClassesWithMethodsAndConstructors) {
    VmRun run(R"(
        클래스 계좌 {
            비공개 정수 잔액
            생성자(초기) {
                자신.잔액 = 초기
            }
            공개 함수 입금(금액) {
                자신.잔액 = 자신.잔액 + 금액
                반환 자신.잔액
            }
        }
        정수 a = 계좌(100)
        a.입금(50)
        정수 결과 = a.입금(25)
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("결과").asInteger(), 175);
}

TEST(VMParityTest, ShouldSliceArraysAndAssignIndexes) {
    VmRun run(R"(
        정수 배열 = [1, 2, 3, 4, 5]
        배열[0] = 10
        정수 부분 = 배열[1부터 3까지]
        정수 첫값 = 배열[0]
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("첫값").asInteger(), 10);
    EXPECT_EQ(run.global("부분").asArray().size(), 3u);
}

TEST(VMParityTest, ShouldWrapAsyncResultsInPromises) {
    VmRun run(R"(
        더하기 = 비동기 함수(a, b) {
            반환 a + b
        }
        p = 더하기(1, 2)
        결과 = 대기 더하기(3, 4)
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_TRUE(run.global("p").isPromise());
    EXPECT_EQ(run.global("결과").asInteger(), 7);
}

TEST(VMParityTest, ShouldRejectPromiseWhenAsyncFunctionThrows) {
    VmRun run(R"(
        실패 = 비동기 함수() {
            던지다 "실패함"
        }
        p = 실패()
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    ASSERT_TRUE(run.global("p").isPromise());
    EXPECT_EQ(run.global("p").asPromise()->state(), Promise::State::REJECTED);
}

TEST(VMParityTest, ShouldFailWithExitStatusOnUncaughtThrow) {
    VmRun run(R"(
        함수 f() { 던지다 "미처리" }
        f()
    )");
    EXPECT_EQ(run.result, VMResult::RUNTIME_ERROR);
}

TEST(VMParityTest, ShouldReportArityMismatch) {
    VmRun run(R"(
        함수 f(a) { 반환 a }
        f(1, 2)
    )");
    EXPECT_EQ(run.result, VMResult::RUNTIME_ERROR);
}

TEST(VMParityTest, ShouldMatchPatternsNatively) {
    const std::string source = R"(
함수 분류(값) {
    반환 값 에 대해 {
        0 -> "영"
        [a, b] when a == b -> "같은 쌍"
        [a, b] -> "쌍 " + 문자열(a + b)
        [첫째, _, 셋째] -> "셋 " + 문자열(첫째 + 셋째)
        참 -> "참"
        n when 타입(n) == "정수" -> "정수 " + 문자열(n)
        _ -> "기타"
    }
}
문자열 결과 = 분류(0) + "/" + 분류([3, 3]) + "/" + 분류([1, 2]) + "/" + 분류([1, 2, 3]) + "/" + 분류(참) + "/" + 분류(7) + "/" + 분류("x")
)";
    VmRun run(source);
    ASSERT_TRUE(run.compiled);
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("결과").toString(), "영/같은 쌍/쌍 3/셋 4/참/정수 7/기타");
    EXPECT_EQ(run.global("결과").toString(), evaluatorGlobal(source, "결과").toString());

    // 패턴 매칭이 평가기 위임(EVAL_NODE) 없이 바이트코드로 컴파일되었다
    std::string listing = run.chunk.disassemble("match");
    EXPECT_NE(listing.find("MATCH_PATTERN"), std::string::npos);
    EXPECT_EQ(listing.find("EVAL_NODE"), std::string::npos);
}

TEST(VMParityTest, ShouldThrowCatchableErrorWhenNoPatternMatches) {
    const std::string source = R"(
문자열 메시지 = ""
시도 {
    정수 r = 7 에 대해 { 1 -> 1 }
} 오류 (e) {
    메시지 = e
}
)";
    VmRun run(source);
    ASSERT_TRUE(run.compiled);
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("메시지").toString(), "RuntimeError: 매칭되는 패턴이 없습니다");
    EXPECT_EQ(run.global("메시지").toString(), evaluatorGlobal(source, "메시지").toString());

    VmRun uncaught("정수 r = 7 에 대해 { 1 -> 1 }");
    ASSERT_TRUE(uncaught.compiled);
    EXPECT_EQ(uncaught.result, VMResult::RUNTIME_ERROR);
}

TEST(VMParityTest, ShouldNotLeakEnvironmentsForMatchCases) {
    size_t before = Environment::liveCount();
    {
        VmRun run(R"(
함수 f(x) {
    반환 x 에 대해 {
        [a, b] -> a + b
        n -> n
    }
}
정수 합 = f([1, 2]) + f(3)
)");
        ASSERT_TRUE(run.compiled);
        EXPECT_EQ(run.result, VMResult::OK);
        EXPECT_EQ(run.global("합").asInteger(), 6);
    }
    EXPECT_EQ(Environment::liveCount(), before);
}

/// 모듈 파일을 임시 디렉터리에 만들고 그 디렉터리를 검색 경로로 쓰는 로더
class VMModuleTest : public ::testing::Test {
protected:
    std::filesystem::path dir_;
    std::unique_ptr<module::ModuleLoader> loader_;

    void SetUp() override {
        dir_ = kingsejong::test::uniqueTestDir("kingsejong_vm_module");
        loader_ = std::make_unique<module::ModuleLoader>(dir_.generic_string());
    }
    void TearDown() override {
        loader_.reset();
        std::filesystem::remove_all(dir_);
    }
    void writeModule(const std::string& name, const std::string& source) {
        std::ofstream file(dir_ / (name + ".ksj"));
        file << source;
    }
};

TEST_F(VMModuleTest, ShouldCompileImportedModulesIntoChunk) {
    writeModule("기초", "함수 곱(a, b) { 반환 a * b }\n정수 상수 = 42\n");
    writeModule("수학", R"(
가져오기 "기초"
함수 제곱(x) { 반환 곱(x, x) }
함수 등급(점수) {
    반환 점수 에 대해 {
        100 -> "만점"
        n when n >= 90 -> "A"
        _ -> "B 이하"
    }
}
)");
    VmRun run(R"(
가져오기 "수학"
정수 결과 = 제곱(7) + 곱(3, 4) + 상수
문자열 등급들 = 등급(100) + 등급(95) + 등급(10)
)", loader_.get());
    ASSERT_TRUE(run.compiled);
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("결과").asInteger(), 49 + 12 + 42);
    EXPECT_EQ(run.global("등급들").toString(), "만점AB 이하");

    // 모듈(과 모듈이 가져온 모듈)의 함수가 같은 청크에 컴파일되어 바이트코드로 호출된다
    EXPECT_TRUE(run.compiledInChunk("제곱"));
    EXPECT_TRUE(run.compiledInChunk("곱"));
    EXPECT_TRUE(run.compiledInChunk("등급"));
    EXPECT_TRUE(loader_->isCached("수학"));
}

TEST_F(VMModuleTest, ShouldCatchErrorsThrownByModuleCode) {
    writeModule("실패", "함수 실패() { 던지다 \"모듈 안 오류\" }\n");
    writeModule("즉시실패", "던지다 \"불러오다 실패\"\n");
    VmRun run(R"(
가져오기 "실패"
문자열 첫째 = ""
문자열 둘째 = ""
시도 {
    실패()
} 오류 (e) {
    첫째 = e
}
시도 {
    가져오기 "즉시실패"
} 오류 (e) {
    둘째 = e
}
)", loader_.get());
    ASSERT_TRUE(run.compiled);
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("첫째").toString(), "Error: 모듈 안 오류");
    EXPECT_EQ(run.global("둘째").toString(), "Error: 불러오다 실패");
}

TEST_F(VMModuleTest, ShouldReportCircularImports) {
    writeModule("가", "가져오기 \"나\"\n");
    writeModule("나", "가져오기 \"가\"\n");
    VmRun run("가져오기 \"가\"\n", loader_.get());
    ASSERT_TRUE(run.compiled);
    EXPECT_EQ(run.result, VMResult::RUNTIME_ERROR);
}

TEST(VMParityTest, ShouldNotLeakEnvironmentsForGlobalFunctions) {
    size_t before = Environment::liveCount();
    {
        VmRun run(R"(
            함수 f(n) { 만약 (n == 0) { 반환 0 } 반환 f(n - 1) }
            f(50)
            클래스 점 {
                비공개 정수 x
                생성자(x) { 자신.x = x }
            }
            정수 p = 점(1)
        )");
        EXPECT_EQ(run.result, VMResult::OK);
    }
    EXPECT_EQ(Environment::liveCount(), before);
}

// ============================================================================
// 비동기 OpCode 테스트
// ============================================================================

TEST(VMAsyncTest, ShouldBuildPromise) {
    Chunk chunk;
    chunk.writeOpCode(OpCode::BUILD_PROMISE, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_TRUE(vm.top().isPromise());
}

TEST(VMAsyncTest, ShouldResolvePromise) {
    Chunk chunk;
    chunk.writeOpCode(OpCode::BUILD_PROMISE, 1);
    chunk.writeOpCode(OpCode::DUP, 1);
    writeConst(chunk, Value::createInteger(42));
    chunk.writeOpCode(OpCode::PROMISE_RESOLVE, 1);
    chunk.writeOpCode(OpCode::AWAIT, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 42);
}

TEST(VMAsyncTest, ShouldBuildAsyncFunctionValue) {
    VmRun run("비동기 함수(a, b) { 반환 a + b }");
    EXPECT_EQ(run.result, VMResult::OK);
    ASSERT_TRUE(run.vm.top().isFunction());
    EXPECT_TRUE(run.vm.top().asFunction()->isAsync());
    EXPECT_EQ(run.vm.top().asFunction()->parameters().size(), 2u);
}

TEST(VMAsyncTest, ShouldAwaitNonPromiseValue) {
    Chunk chunk;
    writeConst(chunk, Value::createInteger(123));
    chunk.writeOpCode(OpCode::AWAIT, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 123);
}

TEST(VMAsyncTest, ShouldThrowRejectionReasonWhenAwaitingRejectedPromise) {
    // 평가기와 같다: 거부된 Promise 를 기다리면 거부 이유가 던져져, 잡는 곳이 없으면 실행 에러다
    Chunk chunk;
    chunk.writeOpCode(OpCode::BUILD_PROMISE, 1);
    chunk.writeOpCode(OpCode::DUP, 1);
    writeConst(chunk, Value::createString("에러"));
    chunk.writeOpCode(OpCode::PROMISE_REJECT, 1);
    chunk.writeOpCode(OpCode::AWAIT, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::RUNTIME_ERROR);
}

TEST(VMAsyncTest, ShouldCatchAwaitedRejectionInTryBlock) {
    const char* source = R"(
실패 = 비동기 함수() { 던지다 "실패함" }
메시지 = ""
시도 {
    값 = 대기 실패()
} 오류 (e) {
    메시지 = e
}
)";
    VmRun run(source);
    ASSERT_TRUE(run.compiled);
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_NE(run.global("메시지").toString().find("실패함"), std::string::npos);
    EXPECT_EQ(run.global("메시지").toString(), evaluatorGlobal(source, "메시지").toString());
}

TEST(VMAsyncTest, ShouldRejectAsyncFunctionWhenAwaitedPromiseIsRejected) {
    const char* source = R"(
실패 = 비동기 함수() { 던지다 "실패함" }
바깥 = 비동기 함수() {
    값 = 대기 실패()
    반환 1
}
p = 바깥()
)";
    VmRun run(source);
    ASSERT_TRUE(run.compiled);
    EXPECT_EQ(run.result, VMResult::OK);
    ASSERT_TRUE(run.global("p").isPromise());
    EXPECT_EQ(run.global("p").asPromise()->state(), Promise::State::REJECTED);
    EXPECT_EQ(evaluatorGlobal(source, "p").asPromise()->state(), Promise::State::REJECTED);
}

TEST(VMAsyncTest, ShouldRunEventLoopUntilAwaitedPromiseSettles) {
    // 아직 끝나지 않은 Promise 는 미뤄 둔 작업을 돌려 끝날 때까지 기다린다 (평가기와 같다)
    auto promise = std::make_shared<Promise>();
    getGlobalEventLoop().enqueueMicrotask([promise]() { promise->resolve(Value::createInteger(7)); });

    Chunk chunk;
    chunk.writeOpCode(OpCode::LOAD_NAME, 1);
    chunk.write16(static_cast<uint16_t>(chunk.addName("p")), 1);
    chunk.writeOpCode(OpCode::AWAIT, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    vm.globals()->set("p", Value::createPromise(promise));
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_EQ(vm.top().asInteger(), 7);
    EXPECT_TRUE(getGlobalEventLoop().isEmpty());
}

TEST(VMAsyncTest, ShouldAwaitPendingPromiseAsPromise) {
    // 끝낼 작업이 하나도 없는 Promise 는 기다릴 길이 없으므로 그대로 돌려준다 (평가기와 같다)
    Chunk chunk;
    chunk.writeOpCode(OpCode::BUILD_PROMISE, 1);
    chunk.writeOpCode(OpCode::AWAIT, 1);
    chunk.writeOpCode(OpCode::HALT, 1);

    VM vm;
    EXPECT_EQ(vm.run(&chunk), VMResult::OK);
    EXPECT_TRUE(vm.top().isPromise());
}

TEST(VMAsyncTest, ShouldChainThenResultOnPendingPromise) {
    // 끝나지 않은 Promise 의 then 은 콜백 결과로 끝나는 새 Promise 를 돌려준다 (평가기와 같다)
    auto promise = std::make_shared<Promise>();
    lexer::Lexer lexer(R"(
결과 = 약속.그러면(함수(값) {
    반환 값 + 10
})
)");
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_TRUE(parser.errors().empty());

    Chunk chunk;
    Compiler compiler;
    ASSERT_TRUE(compiler.compile(program.get(), &chunk));

    VM vm;
    vm.globals()->set("약속", Value::createPromise(promise));
    ASSERT_EQ(vm.run(&chunk), VMResult::OK);

    Value result = vm.globals()->get("결과");
    ASSERT_TRUE(result.isPromise());
    EXPECT_NE(result.asPromise(), promise);

    promise->resolve(Value::createInteger(5));
    getGlobalEventLoop().run();
    EXPECT_EQ(result.asPromise()->state(), Promise::State::FULFILLED);
    EXPECT_EQ(result.asPromise()->value().asInteger(), 15);
}

TEST(BytecodeTest, ShouldChainJosaMethodsWithoutParentheses) {
    const std::string source =
        "arr = [1, 2, 3, 4, 5, 6]\n"
        "제곱합 = arr을 걸러낸다(함수(x) { 반환 x % 2 == 0 })를 변환한다(함수(x) { 반환 x * x })를 축약한다(0, 함수(a, b) { 반환 a + b })\n"
        "사용자 = {\"이름\": \"kim\"}\n"
        "이름 = 사용자의 이름을 대문자로 변환한다\n";
    VmRun run(source);
    ASSERT_TRUE(run.compiled);
    ASSERT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("제곱합").asInteger(), 56);
    EXPECT_EQ(run.global("이름").asString(), "KIM");
    EXPECT_EQ(evaluatorGlobal(source, "제곱합").asInteger(), 56);
}

TEST(VMClassTest, ShouldInheritFieldsMethodsAndConstructor)
{
    VmRun run(R"(
        클래스 동물 {
            공개 문자열 이름
            생성자(이름) {
                자신.이름 = 이름
            }
            공개 함수 소개() {
                반환 자신.이름 + ":" + 자신.소리()
            }
            공개 함수 소리() {
                반환 "..."
            }
        }
        클래스 개 상속 동물 {
            공개 정수 나이
            공개 함수 소리() {
                반환 "멍멍"
            }
        }
        클래스 강아지 상속 개 {
            생성자(이름, 나이) {
                자신.이름 = 이름
                자신.나이 = 나이
            }
        }
        결과 = 개("바둑이").소개() + " " + 강아지("초코", 1).소개() + " " + 동물("냥").소개()
    )");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("결과").asString(), "바둑이:멍멍 초코:멍멍 냥:...");
}

TEST(VMClassTest, ShouldCallParentConstructorAndMethodsWithSuper)
{
    VmRun run(R"KSJ(
        클래스 동물 {
            공개 문자열 이름
            생성자(이름) {
                자신.이름 = 이름
            }
            공개 함수 소개() {
                반환 "나는 " + 자신.이름 + ", " + 자신.소리()
            }
            공개 함수 소리() {
                반환 "..."
            }
        }
        클래스 개 상속 동물 {
            공개 정수 나이
            생성자(이름, 나이) {
                상위(이름)
                자신.나이 = 나이
            }
            공개 함수 소리() {
                반환 "멍멍 (원래: " + 상위.소리() + ")"
            }
            공개 함수 소개() {
                반환 상위.소개() + ", " + 문자열(자신.나이) + "살"
            }
        }
        클래스 강아지 상속 개 {
            공개 함수 소리() {
                반환 "낑낑 / " + 상위.소리()
            }
        }
        결과 = 개("바둑이", 3).소개() + " | " + 강아지("초코", 1).소개()
    )KSJ");
    EXPECT_EQ(run.result, VMResult::OK);
    EXPECT_EQ(run.global("결과").asString(), "나는 바둑이, 멍멍 (원래: ...), 3살 | 나는 초코, 낑낑 / 멍멍 (원래: ...), 1살");
}
