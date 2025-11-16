/**
 * @file JITCompilerT1Test.cpp
 * @brief JIT Compiler Tier 1 테스트
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include <gtest/gtest.h>
#include "jit/JITCompilerT1.h"
#include "bytecode/Chunk.h"
#include "bytecode/OpCode.h"
#include "evaluator/Value.h"

using namespace kingsejong::jit;
using namespace kingsejong::bytecode;
using namespace kingsejong::evaluator;

/**
 * @brief JIT Compiler Tier 1 테스트 픽스처
 */
class JITCompilerT1Test : public ::testing::Test
{
protected:
    JITCompilerT1 compiler_;
    Chunk chunk_;

    void SetUp() override
    {
        // 각 테스트 전에 청크 초기화
        chunk_ = Chunk();
    }

    void TearDown() override
    {
        // JIT 캐시 정리
        compiler_.reset();
    }
};

// ============================================================================
// 기본 산술 연산 테스트
// ============================================================================

TEST_F(JITCompilerT1Test, ShouldCompileSimpleAdd)
{
    // 바이트코드: 5 + 3 = 8
    // LOAD_CONST 0 (5)
    // LOAD_CONST 1 (3)
    // ADD
    // RETURN

    chunk_.addConstant(Value::createInteger(5));
    chunk_.addConstant(Value::createInteger(3));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1); // constant index 0

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2); // constant index 1

    chunk_.writeOpCode(OpCode::ADD, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    // JIT 컴파일
    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    // 실행 (TODO: VM 통합 후 실제 실행 테스트)
    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileSimpleSub)
{
    // 바이트코드: 10 - 3 = 7
    // LOAD_CONST 0 (10)
    // LOAD_CONST 1 (3)
    // SUB
    // RETURN

    chunk_.addConstant(Value::createInteger(10));
    chunk_.addConstant(Value::createInteger(3));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::SUB, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileSimpleMul)
{
    // 바이트코드: 6 * 7 = 42
    // LOAD_CONST 0 (6)
    // LOAD_CONST 1 (7)
    // MUL
    // RETURN

    chunk_.addConstant(Value::createInteger(6));
    chunk_.addConstant(Value::createInteger(7));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::MUL, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileSimpleDiv)
{
    // 바이트코드: 20 / 4 = 5
    // LOAD_CONST 0 (20)
    // LOAD_CONST 1 (4)
    // DIV
    // RETURN

    chunk_.addConstant(Value::createInteger(20));
    chunk_.addConstant(Value::createInteger(4));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::DIV, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileSimpleMod)
{
    // 바이트코드: 17 % 5 = 2
    // LOAD_CONST 0 (17)
    // LOAD_CONST 1 (5)
    // MOD
    // RETURN

    chunk_.addConstant(Value::createInteger(17));
    chunk_.addConstant(Value::createInteger(5));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::MOD, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileNegation)
{
    // 바이트코드: -42
    // LOAD_CONST 0 (42)
    // NEG
    // RETURN

    chunk_.addConstant(Value::createInteger(42));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::NEG, 2);
    chunk_.writeOpCode(OpCode::RETURN, 3);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

// ============================================================================
// 복합 산술 연산 테스트
// ============================================================================

TEST_F(JITCompilerT1Test, ShouldCompileComplexArithmetic)
{
    // 바이트코드: (10 + 5) * 3 = 45
    // LOAD_CONST 0 (10)
    // LOAD_CONST 1 (5)
    // ADD
    // LOAD_CONST 2 (3)
    // MUL
    // RETURN

    chunk_.addConstant(Value::createInteger(10));
    chunk_.addConstant(Value::createInteger(5));
    chunk_.addConstant(Value::createInteger(3));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::ADD, 3);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 4);
    chunk_.write(2, 4);

    chunk_.writeOpCode(OpCode::MUL, 5);
    chunk_.writeOpCode(OpCode::RETURN, 6);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

// ============================================================================
// 에러 처리 테스트
// ============================================================================

TEST_F(JITCompilerT1Test, ShouldHandleEmptyChunk)
{
    // 빈 청크는 컴파일 실패해야 함
    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, 0);
    EXPECT_EQ(nativeFunc, nullptr);
}

TEST_F(JITCompilerT1Test, ShouldHandleInvalidRange)
{
    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    // 잘못된 범위 (endOffset > chunk size)
    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, 100);
    EXPECT_EQ(nativeFunc, nullptr);
}

// ============================================================================
// JIT 캐시 테스트
// ============================================================================

TEST_F(JITCompilerT1Test, ShouldCacheCompiledFunction)
{
    chunk_.addConstant(Value::createInteger(5));
    chunk_.addConstant(Value::createInteger(3));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::ADD, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    // 첫 번째 컴파일
    auto* nativeFunc1 = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc1, nullptr);

    // 같은 범위를 다시 컴파일하면 캐시된 것을 반환해야 함
    auto* nativeFunc2 = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    EXPECT_EQ(nativeFunc1, nativeFunc2);  // 같은 포인터

    compiler_.freeFunction(nativeFunc1);
}

TEST_F(JITCompilerT1Test, ShouldResetCache)
{
    chunk_.addConstant(Value::createInteger(5));
    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);
    chunk_.writeOpCode(OpCode::RETURN, 2);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    // 리셋
    compiler_.reset();

    // 리셋 후에는 새로 컴파일해야 함
    auto* nativeFunc2 = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    EXPECT_NE(nativeFunc, nativeFunc2);  // 다른 포인터

    compiler_.freeFunction(nativeFunc2);
}

// ============================================================================
// 성능 테스트 (컴파일 시간)
// ============================================================================

TEST_F(JITCompilerT1Test, ShouldCompileFast)
{
    // 100개의 ADD 연산을 가진 긴 함수
    for (int i = 0; i < 100; i++)
    {
        chunk_.addConstant(Value::createInteger(1));
        chunk_.writeOpCode(OpCode::LOAD_CONST, i * 2);
        chunk_.write(i, i * 2);

        if (i > 0)
        {
            chunk_.writeOpCode(OpCode::ADD, i * 2 + 1);
        }
    }
    chunk_.writeOpCode(OpCode::RETURN, 200);

    // 컴파일 시간 측정
    auto start = std::chrono::steady_clock::now();
    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    auto end = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    ASSERT_NE(nativeFunc, nullptr);
    EXPECT_LT(duration, 5);  // 5ms 미만

    compiler_.freeFunction(nativeFunc);
}

// ============================================================================
// 변수 접근 테스트
// ============================================================================

TEST_F(JITCompilerT1Test, ShouldCompileLoadVar)
{
    // 바이트코드: LOAD_VAR 0, RETURN
    // 스택 슬롯 0에 있는 값을 로드

    chunk_.writeOpCode(OpCode::LOAD_VAR, 1);
    chunk_.write(0, 1);  // slot 0

    chunk_.writeOpCode(OpCode::RETURN, 2);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileStoreVar)
{
    // 바이트코드: LOAD_CONST 0 (42), STORE_VAR 0, RETURN
    // 상수 42를 스택 슬롯 0에 저장

    chunk_.addConstant(Value::createInteger(42));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::STORE_VAR, 2);
    chunk_.write(0, 2);  // slot 0

    chunk_.writeOpCode(OpCode::RETURN, 3);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileStoreAndLoadVar)
{
    // 바이트코드:
    // LOAD_CONST 0 (100)
    // STORE_VAR 0
    // LOAD_VAR 0
    // RETURN

    chunk_.addConstant(Value::createInteger(100));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::STORE_VAR, 2);
    chunk_.write(0, 2);

    chunk_.writeOpCode(OpCode::LOAD_VAR, 3);
    chunk_.write(0, 3);

    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileVarWithArithmetic)
{
    // 바이트코드:
    // LOAD_CONST 0 (10)
    // STORE_VAR 0       // var a = 10
    // LOAD_VAR 0
    // LOAD_CONST 1 (5)
    // ADD              // a + 5
    // STORE_VAR 1      // var b = a + 5
    // LOAD_VAR 1
    // RETURN

    chunk_.addConstant(Value::createInteger(10));
    chunk_.addConstant(Value::createInteger(5));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::STORE_VAR, 2);
    chunk_.write(0, 2);

    chunk_.writeOpCode(OpCode::LOAD_VAR, 3);
    chunk_.write(0, 3);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 4);
    chunk_.write(1, 4);

    chunk_.writeOpCode(OpCode::ADD, 5);

    chunk_.writeOpCode(OpCode::STORE_VAR, 6);
    chunk_.write(1, 6);

    chunk_.writeOpCode(OpCode::LOAD_VAR, 7);
    chunk_.write(1, 7);

    chunk_.writeOpCode(OpCode::RETURN, 8);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

// ============================================================================
// 제어 흐름 테스트
// ============================================================================

TEST_F(JITCompilerT1Test, ShouldCompileLoadTrue)
{
    // 바이트코드: LOAD_TRUE, RETURN

    chunk_.writeOpCode(OpCode::LOAD_TRUE, 1);
    chunk_.writeOpCode(OpCode::RETURN, 2);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileLoadFalse)
{
    // 바이트코드: LOAD_FALSE, RETURN

    chunk_.writeOpCode(OpCode::LOAD_FALSE, 1);
    chunk_.writeOpCode(OpCode::RETURN, 2);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileJump)
{
    // 바이트코드:
    // LOAD_CONST 0 (10)    ; offset 0
    // JUMP 3               ; offset 2, jump to offset 5
    // LOAD_CONST 1 (99)    ; offset 4 (skipped)
    // LOAD_CONST 2 (20)    ; offset 6
    // RETURN               ; offset 8

    chunk_.addConstant(Value::createInteger(10));
    chunk_.addConstant(Value::createInteger(99));
    chunk_.addConstant(Value::createInteger(20));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);  // offset 0
    chunk_.write(0, 1);                          // offset 1

    chunk_.writeOpCode(OpCode::JUMP, 2);        // offset 2
    chunk_.write(3, 2);                          // offset 3: jump +3 to offset 6

    chunk_.writeOpCode(OpCode::LOAD_CONST, 4);  // offset 4 (should be skipped)
    chunk_.write(1, 4);                          // offset 5

    chunk_.writeOpCode(OpCode::LOAD_CONST, 6);  // offset 6
    chunk_.write(2, 6);                          // offset 7

    chunk_.writeOpCode(OpCode::RETURN, 8);      // offset 8

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileJumpIfFalse)
{
    // 바이트코드:
    // LOAD_FALSE           ; offset 0
    // JUMP_IF_FALSE 3      ; offset 1, jump to offset 5
    // LOAD_CONST 0 (99)    ; offset 3 (skipped)
    // LOAD_CONST 1 (20)    ; offset 5
    // RETURN               ; offset 7

    chunk_.addConstant(Value::createInteger(99));
    chunk_.addConstant(Value::createInteger(20));

    chunk_.writeOpCode(OpCode::LOAD_FALSE, 1);       // offset 0
    chunk_.writeOpCode(OpCode::JUMP_IF_FALSE, 2);   // offset 1
    chunk_.write(3, 2);                               // offset 2: jump +3 to offset 5

    chunk_.writeOpCode(OpCode::LOAD_CONST, 3);       // offset 3 (should be skipped)
    chunk_.write(0, 3);                               // offset 4

    chunk_.writeOpCode(OpCode::LOAD_CONST, 5);       // offset 5
    chunk_.write(1, 5);                               // offset 6

    chunk_.writeOpCode(OpCode::RETURN, 7);           // offset 7

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileJumpIfTrue)
{
    // 바이트코드:
    // LOAD_TRUE            ; offset 0
    // JUMP_IF_TRUE 3       ; offset 1, jump to offset 5
    // LOAD_CONST 0 (99)    ; offset 3 (skipped)
    // LOAD_CONST 1 (20)    ; offset 5
    // RETURN               ; offset 7

    chunk_.addConstant(Value::createInteger(99));
    chunk_.addConstant(Value::createInteger(20));

    chunk_.writeOpCode(OpCode::LOAD_TRUE, 1);        // offset 0
    chunk_.writeOpCode(OpCode::JUMP_IF_TRUE, 2);    // offset 1
    chunk_.write(3, 2);                               // offset 2: jump +3 to offset 5

    chunk_.writeOpCode(OpCode::LOAD_CONST, 3);       // offset 3 (should be skipped)
    chunk_.write(0, 3);                               // offset 4

    chunk_.writeOpCode(OpCode::LOAD_CONST, 5);       // offset 5
    chunk_.write(1, 5);                               // offset 6

    chunk_.writeOpCode(OpCode::RETURN, 7);           // offset 7

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileConditionalBranch)
{
    // 바이트코드: if-else 패턴
    // LOAD_CONST 0 (1)         ; offset 0: condition = true
    // JUMP_IF_FALSE 4          ; offset 2: if false, jump to else
    // LOAD_CONST 1 (10)        ; offset 4: then branch
    // JUMP 3                   ; offset 6: skip else
    // LOAD_CONST 2 (20)        ; offset 8: else branch
    // RETURN                   ; offset 10

    chunk_.addConstant(Value::createInteger(1));   // true
    chunk_.addConstant(Value::createInteger(10));  // then value
    chunk_.addConstant(Value::createInteger(20));  // else value

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);      // offset 0
    chunk_.write(0, 1);                              // offset 1

    chunk_.writeOpCode(OpCode::JUMP_IF_FALSE, 2);   // offset 2
    chunk_.write(4, 2);                              // offset 3: jump +4 to offset 7

    chunk_.writeOpCode(OpCode::LOAD_CONST, 4);      // offset 4: then
    chunk_.write(1, 4);                              // offset 5

    chunk_.writeOpCode(OpCode::JUMP, 6);            // offset 6
    chunk_.write(3, 6);                              // offset 7: jump +3 to offset 10

    chunk_.writeOpCode(OpCode::LOAD_CONST, 8);      // offset 8: else
    chunk_.write(2, 8);                              // offset 9

    chunk_.writeOpCode(OpCode::RETURN, 10);         // offset 10

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

// ============================================================================
// 비교 연산 테스트
// ============================================================================

TEST_F(JITCompilerT1Test, ShouldCompileEqualTrue)
{
    // 바이트코드: 5 == 5 -> true (1)
    // LOAD_CONST 0 (5)
    // LOAD_CONST 1 (5)
    // EQ
    // RETURN

    chunk_.addConstant(Value::createInteger(5));
    chunk_.addConstant(Value::createInteger(5));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::EQ, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileEqualFalse)
{
    // 바이트코드: 5 == 3 -> false (0)

    chunk_.addConstant(Value::createInteger(5));
    chunk_.addConstant(Value::createInteger(3));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::EQ, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileNotEqual)
{
    // 바이트코드: 5 != 3 -> true (1)

    chunk_.addConstant(Value::createInteger(5));
    chunk_.addConstant(Value::createInteger(3));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::NE, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileLessThan)
{
    // 바이트코드: 3 < 5 -> true (1)

    chunk_.addConstant(Value::createInteger(3));
    chunk_.addConstant(Value::createInteger(5));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::LT, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileGreaterThan)
{
    // 바이트코드: 5 > 3 -> true (1)

    chunk_.addConstant(Value::createInteger(5));
    chunk_.addConstant(Value::createInteger(3));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::GT, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileLessOrEqual)
{
    // 바이트코드: 3 <= 5 -> true (1)

    chunk_.addConstant(Value::createInteger(3));
    chunk_.addConstant(Value::createInteger(5));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::LE, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileGreaterOrEqual)
{
    // 바이트코드: 5 >= 3 -> true (1)

    chunk_.addConstant(Value::createInteger(5));
    chunk_.addConstant(Value::createInteger(3));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::GE, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileComparisonWithConditional)
{
    // 바이트코드: if (5 > 3) then 100 else 200
    // LOAD_CONST 0 (5)
    // LOAD_CONST 1 (3)
    // GT                   ; offset 4
    // JUMP_IF_FALSE 3      ; offset 5, if false jump to offset 9
    // LOAD_CONST 2 (100)   ; offset 7: then
    // JUMP 2               ; offset 9, skip else
    // LOAD_CONST 3 (200)   ; offset 11: else
    // RETURN               ; offset 13

    chunk_.addConstant(Value::createInteger(5));
    chunk_.addConstant(Value::createInteger(3));
    chunk_.addConstant(Value::createInteger(100));
    chunk_.addConstant(Value::createInteger(200));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);   // offset 0
    chunk_.write(0, 1);                           // offset 1

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);   // offset 2
    chunk_.write(1, 2);                           // offset 3

    chunk_.writeOpCode(OpCode::GT, 4);           // offset 4

    chunk_.writeOpCode(OpCode::JUMP_IF_FALSE, 5); // offset 5
    chunk_.write(3, 5);                           // offset 6: jump +3 to offset 9

    chunk_.writeOpCode(OpCode::LOAD_CONST, 7);   // offset 7: then
    chunk_.write(2, 7);                           // offset 8

    chunk_.writeOpCode(OpCode::JUMP, 9);         // offset 9
    chunk_.write(2, 9);                           // offset 10: jump +2 to offset 12

    chunk_.writeOpCode(OpCode::LOAD_CONST, 11);  // offset 11: else
    chunk_.write(3, 11);                          // offset 12

    chunk_.writeOpCode(OpCode::RETURN, 13);      // offset 13

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

// ============================================================================
// 논리 연산 테스트
// ============================================================================

TEST_F(JITCompilerT1Test, ShouldCompileAndTrue)
{
    // 바이트코드: 1 && 1 -> true (1)
    // LOAD_TRUE
    // LOAD_TRUE
    // AND
    // RETURN

    chunk_.writeOpCode(OpCode::LOAD_TRUE, 1);
    chunk_.writeOpCode(OpCode::LOAD_TRUE, 2);
    chunk_.writeOpCode(OpCode::AND, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileAndFalse)
{
    // 바이트코드: 1 && 0 -> false (0)

    chunk_.writeOpCode(OpCode::LOAD_TRUE, 1);
    chunk_.writeOpCode(OpCode::LOAD_FALSE, 2);
    chunk_.writeOpCode(OpCode::AND, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileOrTrue)
{
    // 바이트코드: 0 || 1 -> true (1)

    chunk_.writeOpCode(OpCode::LOAD_FALSE, 1);
    chunk_.writeOpCode(OpCode::LOAD_TRUE, 2);
    chunk_.writeOpCode(OpCode::OR, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileOrFalse)
{
    // 바이트코드: 0 || 0 -> false (0)

    chunk_.writeOpCode(OpCode::LOAD_FALSE, 1);
    chunk_.writeOpCode(OpCode::LOAD_FALSE, 2);
    chunk_.writeOpCode(OpCode::OR, 3);
    chunk_.writeOpCode(OpCode::RETURN, 4);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileNotTrue)
{
    // 바이트코드: !0 -> true (1)

    chunk_.writeOpCode(OpCode::LOAD_FALSE, 1);
    chunk_.writeOpCode(OpCode::NOT, 2);
    chunk_.writeOpCode(OpCode::RETURN, 3);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileNotFalse)
{
    // 바이트코드: !1 -> false (0)

    chunk_.writeOpCode(OpCode::LOAD_TRUE, 1);
    chunk_.writeOpCode(OpCode::NOT, 2);
    chunk_.writeOpCode(OpCode::RETURN, 3);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileLogicalWithNumbers)
{
    // 바이트코드: (5 > 3) && (2 < 4) -> true
    // LOAD_CONST 0 (5)
    // LOAD_CONST 1 (3)
    // GT
    // LOAD_CONST 2 (2)
    // LOAD_CONST 3 (4)
    // LT
    // AND
    // RETURN

    chunk_.addConstant(Value::createInteger(5));
    chunk_.addConstant(Value::createInteger(3));
    chunk_.addConstant(Value::createInteger(2));
    chunk_.addConstant(Value::createInteger(4));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::GT, 3);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 4);
    chunk_.write(2, 4);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 5);
    chunk_.write(3, 5);

    chunk_.writeOpCode(OpCode::LT, 6);

    chunk_.writeOpCode(OpCode::AND, 7);
    chunk_.writeOpCode(OpCode::RETURN, 8);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}

TEST_F(JITCompilerT1Test, ShouldCompileComplexLogical)
{
    // 바이트코드: !(5 < 3) -> true
    // LOAD_CONST 0 (5)
    // LOAD_CONST 1 (3)
    // LT
    // NOT
    // RETURN

    chunk_.addConstant(Value::createInteger(5));
    chunk_.addConstant(Value::createInteger(3));

    chunk_.writeOpCode(OpCode::LOAD_CONST, 1);
    chunk_.write(0, 1);

    chunk_.writeOpCode(OpCode::LOAD_CONST, 2);
    chunk_.write(1, 2);

    chunk_.writeOpCode(OpCode::LT, 3);
    chunk_.writeOpCode(OpCode::NOT, 4);
    chunk_.writeOpCode(OpCode::RETURN, 5);

    auto* nativeFunc = compiler_.compileFunction(&chunk_, 0, chunk_.size());
    ASSERT_NE(nativeFunc, nullptr);

    EXPECT_NE(nativeFunc->code, nullptr);
    EXPECT_GT(nativeFunc->codeSize, 0);

    compiler_.freeFunction(nativeFunc);
}
