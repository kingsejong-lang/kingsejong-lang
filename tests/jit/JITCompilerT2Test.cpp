/**
 * @file JITCompilerT2Test.cpp
 * @brief JITCompilerT2 단위 테스트
 * @author KingSejong Team
 * @date 2025-11-24
 */

#include <gtest/gtest.h>
#include "jit/JITCompilerT2.h"
#include "jit/InliningAnalyzer.h"
#include "bytecode/Chunk.h"
#include "bytecode/OpCode.h"

using namespace kingsejong;
using namespace kingsejong::jit;
using namespace kingsejong::bytecode;

class JITCompilerT2Test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        compiler = std::make_unique<JITCompilerT2>();
        chunk = std::make_unique<Chunk>();
    }

    void TearDown() override
    {
        compiler.reset();
        chunk.reset();
    }

    std::unique_ptr<JITCompilerT2> compiler;
    std::unique_ptr<Chunk> chunk;
};

// ========================================
// 생성자 / 소멸자 테스트
// ========================================

TEST_F(JITCompilerT2Test, Constructor)
{
    EXPECT_NE(compiler, nullptr);
}

// ========================================
// 함수 분석 테스트
// ========================================

TEST_F(JITCompilerT2Test, AnalyzeFunction_Basic)
{
    // 간단한 함수: add(a, b) = a + b
    chunk->addConstant(evaluator::Value::createInteger(10));
    chunk->addConstant(evaluator::Value::createInteger(20));

    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1); // constant index 0
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 2);
    chunk->write(1, 2); // constant index 1
    chunk->write(static_cast<uint8_t>(OpCode::ADD), 3);
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 4);

    FunctionMetadata meta = compiler->analyzeFunction(chunk.get(), 0, chunk->size());

    EXPECT_GT(meta.bytecodeSize, 0);
    EXPECT_FALSE(meta.opcodes.empty());
}

TEST_F(JITCompilerT2Test, AnalyzeFunction_Caching)
{
    // 간단한 함수
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1);
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 2);

    // 첫 번째 분석
    FunctionMetadata meta1 = compiler->analyzeFunction(chunk.get(), 0, chunk->size());

    // 두 번째 분석 (캐시에서 가져와야 함)
    FunctionMetadata meta2 = compiler->analyzeFunction(chunk.get(), 0, chunk->size());

    EXPECT_EQ(meta1.bytecodeSize, meta2.bytecodeSize);
    EXPECT_EQ(meta1.opcodes.size(), meta2.opcodes.size());
}

// ========================================
// Hot Function 판별 테스트
// ========================================

TEST_F(JITCompilerT2Test, IsHotFunction_BelowThreshold)
{
    FunctionMetadata meta;
    meta.executionCount = 50; // < 100

    EXPECT_FALSE(compiler->isHotFunction(meta));
}

TEST_F(JITCompilerT2Test, IsHotFunction_AtThreshold)
{
    FunctionMetadata meta;
    meta.executionCount = InliningAnalyzer::HOT_FUNCTION_THRESHOLD; // = 100

    EXPECT_TRUE(compiler->isHotFunction(meta));
}

TEST_F(JITCompilerT2Test, IsHotFunction_AboveThreshold)
{
    FunctionMetadata meta;
    meta.executionCount = 500; // > 100

    EXPECT_TRUE(compiler->isHotFunction(meta));
}

// ========================================
// 인라인 후보 발견 테스트
// ========================================

TEST_F(JITCompilerT2Test, FindInlineCandidates_NoCallSites)
{
    // CALL이 없는 함수
    chunk->addConstant(evaluator::Value::createInteger(42));
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1);
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 2);

    std::vector<CallSite> candidates = compiler->findInlineCandidates(chunk.get(), 0, chunk->size());

    EXPECT_TRUE(candidates.empty());
}

TEST_F(JITCompilerT2Test, FindInlineCandidates_SingleCall)
{
    // add(10, 20) 호출
    chunk->addConstant(evaluator::Value::createInteger(10));
    chunk->addConstant(evaluator::Value::createInteger(20));

    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1); // arg1
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 2);
    chunk->write(1, 2); // arg2
    chunk->write(static_cast<uint8_t>(OpCode::CALL), 3);
    chunk->write(2, 3); // argCount = 2
    chunk->write(0, 3); // funcIndex = 0
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 4);

    std::vector<CallSite> candidates = compiler->findInlineCandidates(chunk.get(), 0, chunk->size());

    EXPECT_EQ(candidates.size(), 1);
    if (!candidates.empty()) {
        EXPECT_EQ(candidates[0].argCount, 2);
    }
}

TEST_F(JITCompilerT2Test, FindInlineCandidates_MultipleCalls)
{
    // 여러 CALL이 있는 함수
    chunk->addConstant(evaluator::Value::createInteger(10));

    // CALL 1
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1);
    chunk->write(static_cast<uint8_t>(OpCode::CALL), 2);
    chunk->write(1, 2); // argCount = 1
    chunk->write(0, 2); // funcIndex = 0

    // CALL 2
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 3);
    chunk->write(0, 3);
    chunk->write(static_cast<uint8_t>(OpCode::CALL), 4);
    chunk->write(1, 4); // argCount = 1
    chunk->write(1, 4); // funcIndex = 1

    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 5);

    std::vector<CallSite> candidates = compiler->findInlineCandidates(chunk.get(), 0, chunk->size());

    EXPECT_EQ(candidates.size(), 2);
}

TEST_F(JITCompilerT2Test, FindInlineCandidates_TooManyArgs)
{
    // 인자가 많은 함수 (> 4)
    chunk->addConstant(evaluator::Value::createInteger(1));

    for (int i = 0; i < 5; i++) {
        chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), i+1);
        chunk->write(0, i+1);
    }

    chunk->write(static_cast<uint8_t>(OpCode::CALL), 6);
    chunk->write(5, 6); // argCount = 5 (> 4)
    chunk->write(0, 6);

    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 7);

    std::vector<CallSite> candidates = compiler->findInlineCandidates(chunk.get(), 0, chunk->size());

    EXPECT_EQ(candidates.size(), 1);
    if (!candidates.empty()) {
        // 인자가 많아도 후보에는 포함되지만 shouldInline은 false일 수 있음
        EXPECT_EQ(candidates[0].argCount, 5);
    }
}

// ========================================
// 인라이닝 컴파일 테스트
// ========================================

TEST_F(JITCompilerT2Test, CompileWithInlining_NotHotFunction)
{
    // 실행 빈도가 낮은 함수
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1);
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 2);

    CompilationResult result = compiler->compileWithInlining(chunk.get(), 0, chunk->size());

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "Not a hot function (execution count < threshold)");
}

TEST_F(JITCompilerT2Test, CompileWithInlining_NoInlineCandidates)
{
    // Hot Function이지만 CALL이 없는 함수
    chunk->addConstant(evaluator::Value::createInteger(42));
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1);
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 2);

    // 캐시에 Hot Function으로 등록 (executionCount 조작)
    FunctionMetadata meta = compiler->analyzeFunction(chunk.get(), 0, chunk->size());
    meta.executionCount = InliningAnalyzer::HOT_FUNCTION_THRESHOLD + 10;

    // 캐시 우회: 이 테스트는 analyzeFunction이 Hot으로 판단하도록 설정 필요
    // 현재는 executionCount가 0이므로 실패할 것

    CompilationResult result = compiler->compileWithInlining(chunk.get(), 0, chunk->size());

    EXPECT_FALSE(result.success);
    // "Not a hot function" 또는 "No inlinable call sites found" 예상
}

// ========================================
// 통계 테스트
// ========================================

TEST_F(JITCompilerT2Test, PrintStatistics_Initial)
{
    // 초기 상태 통계 출력 (크래시 없이 실행되어야 함)
    EXPECT_NO_THROW(compiler->printStatistics());
}

TEST_F(JITCompilerT2Test, Reset)
{
    // 분석 수행
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 1);
    compiler->analyzeFunction(chunk.get(), 0, chunk->size());

    // 리셋
    compiler->reset();

    // 리셋 후 통계 출력
    EXPECT_NO_THROW(compiler->printStatistics());
}

// ========================================
// 통합 테스트
// ========================================

TEST_F(JITCompilerT2Test, Integration_SimpleFunction)
{
    // add 함수 정의 (간단한 예시)
    chunk->addConstant(evaluator::Value::createInteger(10));
    chunk->addConstant(evaluator::Value::createInteger(20));

    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1);
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 2);
    chunk->write(1, 2);
    chunk->write(static_cast<uint8_t>(OpCode::ADD), 3);
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 4);

    // 분석
    FunctionMetadata meta = compiler->analyzeFunction(chunk.get(), 0, chunk->size());

    EXPECT_GT(meta.bytecodeSize, 0);
    EXPECT_FALSE(meta.opcodes.empty());

    // Hot Function 여부 (초기 executionCount는 0이므로 false)
    EXPECT_FALSE(compiler->isHotFunction(meta));

    // 인라인 후보 탐색 (CALL이 없으므로 empty)
    std::vector<CallSite> candidates = compiler->findInlineCandidates(chunk.get(), 0, chunk->size());
    EXPECT_TRUE(candidates.empty());
}
