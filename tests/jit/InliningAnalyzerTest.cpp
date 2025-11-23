/**
 * @file InliningAnalyzerTest.cpp
 * @brief InliningAnalyzer 단위 테스트
 * @author KingSejong Team
 * @date 2025-11-24
 */

#include <gtest/gtest.h>
#include "jit/InliningAnalyzer.h"
#include "bytecode/Chunk.h"
#include "bytecode/OpCode.h"

using namespace kingsejong;
using namespace kingsejong::jit;
using namespace kingsejong::bytecode;

class InliningAnalyzerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        analyzer = std::make_unique<InliningAnalyzer>();
        chunk = std::make_unique<Chunk>();
    }

    void TearDown() override
    {
        analyzer.reset();
        chunk.reset();
    }

    std::unique_ptr<InliningAnalyzer> analyzer;
    std::unique_ptr<Chunk> chunk;
};

// ========================================
// FunctionMetadata 수집 테스트
// ========================================

TEST_F(InliningAnalyzerTest, CollectMetadata_SimpleFunction)
{
    // 간단한 함수: LOAD_CONST, ADD, RETURN
    // Constants 추가
    chunk->addConstant(evaluator::Value::createInteger(10));
    chunk->addConstant(evaluator::Value::createInteger(20));

    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1); // constant index 0
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 2);
    chunk->write(1, 2); // constant index 1
    chunk->write(static_cast<uint8_t>(OpCode::ADD), 3);
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 4);

    FunctionMetadata meta = analyzer->collectMetadata(chunk.get(), 0, chunk->size());

    EXPECT_EQ(meta.bytecodeSize, chunk->size());
    EXPECT_FALSE(meta.hasLoops);
    EXPECT_FALSE(meta.hasConditionals);
    EXPECT_EQ(meta.opcodeCount[OpCode::LOAD_CONST], 2);
    EXPECT_EQ(meta.opcodeCount[OpCode::ADD], 1);
    EXPECT_EQ(meta.opcodeCount[OpCode::RETURN], 1);
}

TEST_F(InliningAnalyzerTest, CollectMetadata_WithConditional)
{
    // 조건문 포함: LOAD_CONST, JUMP_IF_FALSE, RETURN
    // Constants 추가
    chunk->addConstant(evaluator::Value::createInteger(1));

    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1); // constant index 0
    chunk->write(static_cast<uint8_t>(OpCode::JUMP_IF_FALSE), 2);
    chunk->write(5, 2); // target low
    chunk->write(0, 2); // target high
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 3);

    FunctionMetadata meta = analyzer->collectMetadata(chunk.get(), 0, chunk->size());

    EXPECT_TRUE(meta.hasConditionals);
    EXPECT_FALSE(meta.hasLoops);
    EXPECT_EQ(meta.opcodeCount[OpCode::JUMP_IF_FALSE], 1);
}

// ========================================
// 인라이닝 가능 여부 테스트
// ========================================

TEST_F(InliningAnalyzerTest, CanInline_SmallHotFunction)
{
    // 작은 Hot Function
    FunctionMetadata meta;
    meta.bytecodeSize = 10;
    meta.executionCount = 150;
    meta.hasRecursion = false;
    meta.hasLoops = false;

    EXPECT_TRUE(analyzer->canInline(meta));
}

TEST_F(InliningAnalyzerTest, CanInline_TooLarge)
{
    // 너무 큰 함수
    FunctionMetadata meta;
    meta.bytecodeSize = 100; // > MAX_INLINE_SIZE (50)
    meta.executionCount = 150;
    meta.hasRecursion = false;

    EXPECT_FALSE(analyzer->canInline(meta));
}

TEST_F(InliningAnalyzerTest, CanInline_NotHot)
{
    // Cold Function
    FunctionMetadata meta;
    meta.bytecodeSize = 10;
    meta.executionCount = 50; // < HOT_FUNCTION_THRESHOLD (100)
    meta.hasRecursion = false;

    EXPECT_FALSE(analyzer->canInline(meta));
}

TEST_F(InliningAnalyzerTest, CanInline_Recursive)
{
    // 재귀 함수
    FunctionMetadata meta;
    meta.bytecodeSize = 10;
    meta.executionCount = 150;
    meta.hasRecursion = true; // 재귀

    EXPECT_FALSE(analyzer->canInline(meta));
}

// ========================================
// 인라이닝 우선순위 테스트
// ========================================

TEST_F(InliningAnalyzerTest, GetInlinePriority_HighForSimple)
{
    // 단순 함수 (산술 연산만)
    FunctionMetadata meta;
    meta.bytecodeSize = 5;
    meta.executionCount = 200;
    meta.hasLoops = false;
    meta.hasConditionals = false;
    meta.hasRecursion = false;

    InlinePriority priority = analyzer->getInlinePriority(meta);

    EXPECT_EQ(priority, InlinePriority::HIGH);
}

TEST_F(InliningAnalyzerTest, GetInlinePriority_MediumForConditional)
{
    // 조건문 포함 함수
    FunctionMetadata meta;
    meta.bytecodeSize = 20;
    meta.executionCount = 200;
    meta.hasLoops = false;
    meta.hasConditionals = true;
    meta.hasRecursion = false;

    InlinePriority priority = analyzer->getInlinePriority(meta);

    EXPECT_EQ(priority, InlinePriority::MEDIUM);
}

TEST_F(InliningAnalyzerTest, GetInlinePriority_LowForLoop)
{
    // 루프 포함 함수
    FunctionMetadata meta;
    meta.bytecodeSize = 30;
    meta.executionCount = 200;
    meta.hasLoops = true;
    meta.hasConditionals = false;
    meta.hasRecursion = false;

    InlinePriority priority = analyzer->getInlinePriority(meta);

    EXPECT_EQ(priority, InlinePriority::LOW);
}

TEST_F(InliningAnalyzerTest, GetInlinePriority_NoneForNonInlinable)
{
    // 인라이닝 불가 함수
    FunctionMetadata meta;
    meta.bytecodeSize = 100; // 너무 큼
    meta.executionCount = 200;
    meta.hasRecursion = false;

    InlinePriority priority = analyzer->getInlinePriority(meta);

    EXPECT_EQ(priority, InlinePriority::NONE);
}

// ========================================
// 루프 감지 테스트
// ========================================

TEST_F(InliningAnalyzerTest, DetectLoops_BackwardJump)
{
    // 루프: 역방향 JUMP (ip=5에서 ip=0으로)
    chunk->write(static_cast<uint8_t>(OpCode::LOAD_CONST), 1);
    chunk->write(0, 1); // offset 1
    chunk->write(static_cast<uint8_t>(OpCode::JUMP), 2);
    chunk->write(0, 2); // target = 0 (역방향)
    chunk->write(0, 2);

    bool hasLoops = analyzer->detectLoops(chunk.get(), 0, chunk->size());

    EXPECT_TRUE(hasLoops);
}

TEST_F(InliningAnalyzerTest, DetectLoops_ForwardJump)
{
    // 순방향 JUMP만 (루프 아님)
    chunk->write(static_cast<uint8_t>(OpCode::JUMP), 1);
    chunk->write(5, 1); // target = 5 (순방향)
    chunk->write(0, 1);
    chunk->write(static_cast<uint8_t>(OpCode::RETURN), 2);

    bool hasLoops = analyzer->detectLoops(chunk.get(), 0, chunk->size());

    EXPECT_FALSE(hasLoops);
}

// ========================================
// CallSite 분석 테스트
// ========================================

TEST_F(InliningAnalyzerTest, AnalyzeCallSite_Basic)
{
    // CALL OpCode (2 arguments)
    chunk->write(static_cast<uint8_t>(OpCode::CALL), 1);
    chunk->write(2, 1); // arg count = 2

    FunctionMetadata calleeMeta;
    calleeMeta.bytecodeSize = 10;

    CallSite site = analyzer->analyzeCallSite(chunk.get(), 0, &calleeMeta);

    EXPECT_EQ(site.callOffset, 0);
    EXPECT_EQ(site.argCount, 2);
    EXPECT_EQ(site.callee, &calleeMeta);
}

TEST_F(InliningAnalyzerTest, EstimateCodeSize)
{
    FunctionMetadata calleeMeta;
    calleeMeta.bytecodeSize = 15;

    CallSite site;
    site.callee = &calleeMeta;
    site.argCount = 2;
    site.hasConstantArgs = false;

    size_t estimatedSize = analyzer->estimateCodeSize(site);

    EXPECT_EQ(estimatedSize, 15);
}

TEST_F(InliningAnalyzerTest, EstimateCodeSize_WithConstantArgs)
{
    FunctionMetadata calleeMeta;
    calleeMeta.bytecodeSize = 15;

    CallSite site;
    site.callee = &calleeMeta;
    site.argCount = 2;
    site.hasConstantArgs = true; // 상수 인자

    size_t estimatedSize = analyzer->estimateCodeSize(site);

    // 상수 전파로 크기 감소 (argCount * 2)
    EXPECT_LT(estimatedSize, 15);
}

// ========================================
// 복잡도 점수 테스트
// ========================================

TEST_F(InliningAnalyzerTest, ComplexityScore_Simple)
{
    FunctionMetadata meta;
    meta.bytecodeSize = 10;
    meta.hasLoops = false;
    meta.hasRecursion = false;
    meta.hasConditionals = false;
    meta.hasFunctionCalls = false;

    int score = meta.getComplexityScore();

    EXPECT_EQ(score, 10); // bytecodeSize만
}

TEST_F(InliningAnalyzerTest, ComplexityScore_WithLoop)
{
    FunctionMetadata meta;
    meta.bytecodeSize = 10;
    meta.hasLoops = true;
    meta.hasRecursion = false;
    meta.hasConditionals = false;
    meta.hasFunctionCalls = false;

    int score = meta.getComplexityScore();

    EXPECT_EQ(score, 30); // 10 + 20 (loop penalty)
}

TEST_F(InliningAnalyzerTest, ComplexityScore_WithRecursion)
{
    FunctionMetadata meta;
    meta.bytecodeSize = 10;
    meta.hasLoops = false;
    meta.hasRecursion = true;
    meta.hasConditionals = false;
    meta.hasFunctionCalls = false;

    int score = meta.getComplexityScore();

    EXPECT_EQ(score, 60); // 10 + 50 (recursion penalty)
}

// ========================================
// 순수 함수 테스트
// ========================================

TEST_F(InliningAnalyzerTest, IsPure_NoGlobalAccess)
{
    FunctionMetadata meta;
    // 전역 변수 접근 없음

    EXPECT_TRUE(meta.isPure());
}

TEST_F(InliningAnalyzerTest, IsPure_WithGlobalAccess)
{
    FunctionMetadata meta;
    meta.opcodeCount[OpCode::LOAD_GLOBAL] = 1; // 전역 변수 읽기

    EXPECT_FALSE(meta.isPure());
}
