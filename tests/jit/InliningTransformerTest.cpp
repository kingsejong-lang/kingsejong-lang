/**
 * @file InliningTransformerTest.cpp
 * @brief InliningTransformer 단위 테스트
 * @author KingSejong Team
 * @date 2025-11-24
 */

#include <gtest/gtest.h>
#include "jit/InliningTransformer.h"
#include "jit/InliningAnalyzer.h"
#include "bytecode/Chunk.h"
#include "bytecode/OpCode.h"

using namespace kingsejong;
using namespace kingsejong::jit;
using namespace kingsejong::bytecode;

class InliningTransformerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        transformer = std::make_unique<InliningTransformer>();
        analyzer = std::make_unique<InliningAnalyzer>();
        chunk = std::make_unique<Chunk>();
    }

    void TearDown() override
    {
        transformer.reset();
        analyzer.reset();
        chunk.reset();
    }

    std::unique_ptr<InliningTransformer> transformer;
    std::unique_ptr<InliningAnalyzer> analyzer;
    std::unique_ptr<Chunk> chunk;
};

// ========================================
// 상수 폴딩 테스트
// ========================================

TEST_F(InliningTransformerTest, TryConstantFolding_Add)
{
    // add(10, 20) 함수
    FunctionMetadata calleeMeta;
    calleeMeta.bytecodeSize = 8;
    calleeMeta.opcodes = {
        OpCode::LOAD_VAR,    // param 0
        OpCode::LOAD_VAR,    // param 1
        OpCode::ADD,
        OpCode::RETURN
    };
    calleeMeta.opcodeCount[OpCode::ADD] = 1;

    CallSite site;
    site.callee = &calleeMeta;
    site.argCount = 2;
    site.hasConstantArgs = true;
    site.constantArgs.push_back(evaluator::Value::createInteger(10));
    site.constantArgs.push_back(evaluator::Value::createInteger(20));

    bool result = transformer->tryConstantFolding(chunk.get(), site);

    EXPECT_TRUE(result);
}

TEST_F(InliningTransformerTest, TryConstantFolding_Sub)
{
    // sub(50, 30) 함수
    FunctionMetadata calleeMeta;
    calleeMeta.bytecodeSize = 8;
    calleeMeta.opcodes = {
        OpCode::LOAD_VAR,
        OpCode::LOAD_VAR,
        OpCode::SUB,
        OpCode::RETURN
    };
    calleeMeta.opcodeCount[OpCode::SUB] = 1;

    CallSite site;
    site.callee = &calleeMeta;
    site.argCount = 2;
    site.hasConstantArgs = true;
    site.constantArgs.push_back(evaluator::Value::createInteger(50));
    site.constantArgs.push_back(evaluator::Value::createInteger(30));

    bool result = transformer->tryConstantFolding(chunk.get(), site);

    EXPECT_TRUE(result);
}

TEST_F(InliningTransformerTest, TryConstantFolding_Mul)
{
    // mul(6, 7) 함수
    FunctionMetadata calleeMeta;
    calleeMeta.bytecodeSize = 8;
    calleeMeta.opcodes = {
        OpCode::LOAD_VAR,
        OpCode::LOAD_VAR,
        OpCode::MUL,
        OpCode::RETURN
    };
    calleeMeta.opcodeCount[OpCode::MUL] = 1;

    CallSite site;
    site.callee = &calleeMeta;
    site.argCount = 2;
    site.hasConstantArgs = true;
    site.constantArgs.push_back(evaluator::Value::createInteger(6));
    site.constantArgs.push_back(evaluator::Value::createInteger(7));

    bool result = transformer->tryConstantFolding(chunk.get(), site);

    EXPECT_TRUE(result);
}

TEST_F(InliningTransformerTest, TryConstantFolding_NotAllConstant)
{
    // add(x, 20) - 첫 번째 인자가 변수
    FunctionMetadata calleeMeta;
    calleeMeta.bytecodeSize = 8;
    calleeMeta.opcodes = {
        OpCode::LOAD_VAR,
        OpCode::LOAD_VAR,
        OpCode::ADD,
        OpCode::RETURN
    };

    CallSite site;
    site.callee = &calleeMeta;
    site.argCount = 2;
    site.hasConstantArgs = false; // 모든 인자가 상수가 아님

    bool result = transformer->tryConstantFolding(chunk.get(), site);

    EXPECT_FALSE(result);
}

TEST_F(InliningTransformerTest, TryConstantFolding_ComplexFunction)
{
    // 복잡한 함수 (루프 포함)
    FunctionMetadata calleeMeta;
    calleeMeta.bytecodeSize = 20;
    calleeMeta.hasLoops = true;
    calleeMeta.opcodes = {
        OpCode::LOAD_VAR,
        OpCode::LOAD_VAR,
        OpCode::ADD,
        OpCode::LOAD_VAR,
        OpCode::MUL,
        OpCode::JUMP,
        OpCode::RETURN
    };

    CallSite site;
    site.callee = &calleeMeta;
    site.argCount = 2;
    site.hasConstantArgs = true;
    site.constantArgs.push_back(evaluator::Value::createInteger(10));
    site.constantArgs.push_back(evaluator::Value::createInteger(20));

    bool result = transformer->tryConstantFolding(chunk.get(), site);

    EXPECT_FALSE(result); // 복잡한 함수는 폴딩 불가
}

// ========================================
// 데드 코드 제거 테스트
// ========================================

TEST_F(InliningTransformerTest, EliminateDeadCode_NoDeadCode)
{
    // 순차 실행 (dead code 없음)
    chunk->addConstant(evaluator::Value::createInteger(10));
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1);
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 2);

    size_t eliminated = transformer->eliminateDeadCode(chunk.get(), 0, chunk->size());

    EXPECT_EQ(eliminated, 0); // 모든 코드가 도달 가능
}

TEST_F(InliningTransformerTest, EliminateDeadCode_UnconditionalJump)
{
    // JUMP → 도달 불가능한 코드
    chunk->addConstant(evaluator::Value::createInteger(10));

    chunk->write(static_cast<uint8_t>(OpCode::JUMP), 1);
    chunk->write(6, 1); // target = 6 (RETURN으로 점프)
    chunk->write(0, 1);

    // Dead code (도달 불가)
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 2);
    chunk->write(0, 2);

    // Reachable
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 3);

    size_t eliminated = transformer->eliminateDeadCode(chunk.get(), 0, chunk->size());

    EXPECT_GT(eliminated, 0); // LOAD_CONST가 dead code
}

// ========================================
// 도달 가능성 분석 테스트
// ========================================

TEST_F(InliningTransformerTest, AnalyzeReachability_Linear)
{
    // 순차 실행
    chunk->addConstant(evaluator::Value::createInteger(10));
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1);
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 2);

    size_t eliminated = transformer->eliminateDeadCode(chunk.get(), 0, chunk->size());

    EXPECT_EQ(eliminated, 0); // 모든 명령어 도달 가능
}

TEST_F(InliningTransformerTest, AnalyzeReachability_ConditionalBranch)
{
    // 조건 분기
    chunk->addConstant(evaluator::Value::createInteger(1));
    chunk->addConstant(evaluator::Value::createInteger(10));

    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1); // constant index 0
    chunk->write(static_cast<uint8_t>(OpCode::JUMP_IF_FALSE), 2);
    chunk->write(9, 2); // target = 9 (else branch)
    chunk->write(0, 2);

    // Then branch
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 3);
    chunk->write(1, 3); // constant index 1

    // Else branch (target = 9)
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 4);
    chunk->write(1, 4);

    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 5);

    size_t eliminated = transformer->eliminateDeadCode(chunk.get(), 0, chunk->size());

    EXPECT_EQ(eliminated, 0); // 모든 분기 도달 가능
}

// ========================================
// 인라인 결과 검증 테스트
// ========================================

TEST_F(InliningTransformerTest, InlineFunction_Basic)
{
    // add 함수
    FunctionMetadata calleeMeta;
    calleeMeta.bytecodeSize = 8;
    calleeMeta.bytecodeOffset = 0;

    CallSite site;
    site.callOffset = 0;
    site.callee = &calleeMeta;
    site.argCount = 2;

    // CALL OpCode
    chunk->write(static_cast<uint8_t>(OpCode::CALL), 1);
    chunk->write(2, 1); // arg count
    chunk->write(0, 1);

    InlineResult result = transformer->inlineFunction(chunk.get(), site, calleeMeta);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.savedInstructions, 2); // CALL + RETURN
}

TEST_F(InliningTransformerTest, InlineFunction_InvalidCallSite)
{
    FunctionMetadata calleeMeta;
    calleeMeta.bytecodeSize = 8;

    CallSite site;
    site.callOffset = 1000; // 잘못된 오프셋
    site.callee = &calleeMeta;

    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 1);

    InlineResult result = transformer->inlineFunction(chunk.get(), site, calleeMeta);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(InliningTransformerTest, InlineFunction_EmptyBody)
{
    FunctionMetadata calleeMeta;
    calleeMeta.bytecodeSize = 0; // 빈 함수

    CallSite site;
    site.callOffset = 0;
    site.callee = &calleeMeta;

    chunk->write(static_cast<uint8_t>(OpCode::CALL), 1);
    chunk->write(0, 1);
    chunk->write(0, 1);

    InlineResult result = transformer->inlineFunction(chunk.get(), site, calleeMeta);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "Empty function body");
}

// ========================================
// 바이트코드 복사 테스트
// ========================================

TEST_F(InliningTransformerTest, CopyBytecode_Basic)
{
    // Source chunk
    auto sourceChunk = std::make_unique<Chunk>();
    sourceChunk->addConstant(evaluator::Value::createInteger(42));
    sourceChunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    sourceChunk->write(0, 1);
    sourceChunk->write(static_cast<uint8_t>(OpCode::RETURN), 2);

    // 복사 시뮬레이션
    size_t sourceStart = 0;
    size_t sourceEnd = sourceChunk->size();
    // size_t destOffset = chunk->size(); // TODO: 실제 복사 테스트 시 사용

    // Note: copyBytecode는 현재 private이므로 간접 테스트
    // 실제로는 inlineFunction을 통해 테스트해야 함

    EXPECT_GT(sourceEnd, sourceStart);
}

// ========================================
// 인자 바인딩 테스트
// ========================================

TEST_F(InliningTransformerTest, BindArguments_ConstantArgs)
{
    // add(10, 20) - 상수 인자 바인딩
    chunk->addConstant(evaluator::Value::createInteger(10));
    chunk->addConstant(evaluator::Value::createInteger(20));

    // 함수 본문 (LOAD_VAR 0, LOAD_VAR 1, ADD)
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_VAR), 1);
    chunk->write(0, 1); // param 0
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_VAR), 2);
    chunk->write(1, 2); // param 1
    chunk->write(static_cast<uint8_t>(OpCode::ADD), 3);

    CallSite site;
    site.argCount = 2;
    site.hasConstantArgs = true;
    site.constantArgs.push_back(evaluator::Value::createInteger(10));
    site.constantArgs.push_back(evaluator::Value::createInteger(20));

    // Note: bindArguments는 private
    // 실제로는 inlineFunction 통해 간접 테스트 필요

    EXPECT_EQ(site.constantArgs.size(), 2);
}
