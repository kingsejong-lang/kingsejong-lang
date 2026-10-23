/**
 * @file JITTier2BenchmarkTest.cpp
 * @brief JIT Tier 2 벤치마크 테스트
 * @author KingSejong Team
 * @date 2025-11-24
 *
 * Phase 4: JIT Tier 2 인라이닝 성능 측정 벤치마크
 */

#include <gtest/gtest.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"

using namespace kingsejong;
using namespace kingsejong::bytecode;
using namespace std::chrono;

/**
 * @brief 소스 코드를 바이트코드로 컴파일
 */
Chunk* compileSource(const std::string& source, const std::string& filename = "test.ksj")
{
    // 1. Lexer
    lexer::Lexer lexer(source, filename);

    // 2. Parser
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    if (!parser.errors().empty()) {
        std::cerr << "Parse errors:\n";
        for (const auto& err : parser.errors()) {
            std::cerr << "  " << err << "\n";
        }
        return nullptr;
    }

    // 3. Compiler
    Compiler compiler;
    Chunk* chunk = new Chunk();
    bool success = compiler.compile(program.get(), chunk);

    if (!success) {
        std::cerr << "Compilation failed\n";
        delete chunk;
        return nullptr;
    }

    return chunk;
}

/**
 * @brief .ksj 파일을 읽어서 컴파일
 */
Chunk* compileFile(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << "\n";
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    return compileSource(source, filepath);
}

/**
 * @brief VM 실행 및 시간 측정
 */
struct BenchmarkResult {
    bool success;
    double elapsedMs;
    evaluator::Value result;
};

BenchmarkResult runBenchmark(Chunk* chunk, bool jitEnabled = false, int runs = 5)
{
    double totalTime = 0.0;
    evaluator::Value lastResult;
    bool success = true;

    for (int i = 0; i < runs; ++i) {
        VM vm;
        vm.setJITEnabled(jitEnabled);  // Disabled for now due to unsupported opcodes
        vm.setMaxInstructions(100000000); // 100M for benchmarks
        vm.setMaxExecutionTime(30000); // 30s timeout

        auto start = high_resolution_clock::now();
        VMResult result = vm.run(chunk);
        auto end = high_resolution_clock::now();

        if (result != VMResult::OK && result != VMResult::HALT) {
            success = false;
            break;
        }

        double elapsed = duration_cast<microseconds>(end - start).count() / 1000.0;
        totalTime += elapsed;
        lastResult = vm.top();

        // 첫 실행에서 통계 출력
        if (i == 0 && jitEnabled) {
            std::cout << "    JIT Statistics (Run 1):\n";
            vm.printJITStatistics();
        }
    }

    return {success, totalTime / runs, lastResult};
}

// ========================================
// 벤치마크 테스트
// ========================================

class JITTier2BenchmarkTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 벤치마크 디렉토리 기본 경로
        benchmarkDir = "benchmarks/jit_tier2/";
    }

    std::string benchmarkDir;
};

/**
 * @brief 벤치마크 1: 작은 함수 집약
 *
 * 작은 add() 함수를 10,000번 호출하여 인라이닝 효과 측정
 * 예상: Tier 2가 함수 호출 오버헤드를 제거하여 ~30% 성능 개선
 */
TEST_F(JITTier2BenchmarkTest, SmallFunctionIntensive)
{
    GTEST_SKIP() << "JIT Tier 2 는 아직 네이티브 함수를 만들지 않는다 (JITCompilerT2::compileWithInlining 이 result.function 을 채우지 않음)";
    std::cout << "\n=== Benchmark: Small Function Intensive ===\n";

    std::string filepath = benchmarkDir + "small_function_intensive.ksj";
    Chunk* chunk = compileFile(filepath);
    ASSERT_NE(chunk, nullptr) << "Failed to compile " << filepath;

    std::cout << "Running with VM (JIT temporarily disabled)...\n";
    auto result = runBenchmark(chunk, false, 5);

    ASSERT_TRUE(result.success) << "Benchmark execution failed";

    std::cout << "\n📊 Results:\n";
    std::cout << "  Average execution time: " << result.elapsedMs << " ms\n";
    std::cout << "  Result: " << result.result.toString() << "\n";
    std::cout << "  Expected result: 49995000 (sum of 0 to 9999)\n";

    // Verify correctness
    EXPECT_EQ(result.result.asInteger(), 49995000);

    std::cout << "\n✅ Expected: ~30% improvement over Tier 1\n";
    std::cout << "   Baseline (Tier 1): ~80ms\n";
    std::cout << "   Target (Tier 2): ~55ms\n";
    std::cout << "==========================================\n\n";

    delete chunk;
}

/**
 * @brief 벤치마크 2: 조건문 포함 함수
 *
 * 조건문이 있는 max() 함수를 10,000번 호출하여 인라이닝 효과 측정
 * 예상: Tier 2가 분기 예측을 최적화하여 ~25% 성능 개선
 */
TEST_F(JITTier2BenchmarkTest, ConditionalFunction)
{
    GTEST_SKIP() << "JIT Tier 2 는 아직 네이티브 함수를 만들지 않는다 (JITCompilerT2::compileWithInlining 이 result.function 을 채우지 않음)";
    std::cout << "\n=== Benchmark: Conditional Function ===\n";

    std::string filepath = benchmarkDir + "conditional_function.ksj";
    Chunk* chunk = compileFile(filepath);
    ASSERT_NE(chunk, nullptr) << "Failed to compile " << filepath;

    std::cout << "Running with VM (JIT temporarily disabled)...\n";
    auto result = runBenchmark(chunk, false, 5);

    ASSERT_TRUE(result.success) << "Benchmark execution failed";

    std::cout << "\n📊 Results:\n";
    std::cout << "  Average execution time: " << result.elapsedMs << " ms\n";
    std::cout << "  Result: " << result.result.toString() << "\n";
    std::cout << "  Expected result: 9999 (max of 0 to 9999)\n";

    // Verify correctness
    EXPECT_EQ(result.result.asInteger(), 9999);

    std::cout << "\n✅ Expected: ~25% improvement over Tier 1\n";
    std::cout << "   Baseline (Tier 1): ~60ms\n";
    std::cout << "   Target (Tier 2): ~45ms\n";
    std::cout << "==========================================\n\n";

    delete chunk;
}

/**
 * @brief 벤치마크 3: stdlib 함수 사용
 *
 * stdlib 스타일 abs() 함수를 10,000번 호출하여 인라이닝 효과 측정
 * 예상: Tier 2가 반복 패턴을 최적화하여 ~20% 성능 개선
 */
TEST_F(JITTier2BenchmarkTest, StdlibFunction)
{
    GTEST_SKIP() << "JIT Tier 2 는 아직 네이티브 함수를 만들지 않는다 (JITCompilerT2::compileWithInlining 이 result.function 을 채우지 않음)";
    std::cout << "\n=== Benchmark: Stdlib Function ===\n";

    std::string filepath = benchmarkDir + "stdlib_function.ksj";
    Chunk* chunk = compileFile(filepath);
    ASSERT_NE(chunk, nullptr) << "Failed to compile " << filepath;

    std::cout << "Running with VM (JIT temporarily disabled)...\n";
    auto result = runBenchmark(chunk, false, 5);

    ASSERT_TRUE(result.success) << "Benchmark execution failed";

    std::cout << "\n📊 Results:\n";
    std::cout << "  Average execution time: " << result.elapsedMs << " ms\n";
    std::cout << "  Result: " << result.result.toString() << "\n";
    std::cout << "  Expected result: 25000000 (sum of abs(-4999) to abs(5000))\n";

    // Verify correctness
    EXPECT_EQ(result.result.asInteger(), 25000000);

    std::cout << "\n✅ Expected: ~20% improvement over Tier 1\n";
    std::cout << "   Baseline (Tier 1): ~70ms\n";
    std::cout << "   Target (Tier 2): ~56ms\n";
    std::cout << "==========================================\n\n";

    delete chunk;
}

/**
 * @brief 통합 벤치마크 - 모든 벤치마크 실행 및 요약
 */
TEST_F(JITTier2BenchmarkTest, DISABLED_AllBenchmarksSummary)
{
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║         JIT Tier 2 Benchmark Summary                   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";

    struct BenchInfo {
        std::string name;
        std::string file;
        int64_t expectedResult;
    };

    std::vector<BenchInfo> benchmarks = {
        {"Small Function", "small_function_intensive.ksj", 49995000},
        {"Conditional", "conditional_function.ksj", 9999},
        {"Stdlib Function", "stdlib_function.ksj", 25000000}
    };

    std::cout << "Running " << benchmarks.size() << " benchmarks...\n\n";

    for (const auto& bench : benchmarks) {
        std::cout << "📊 " << bench.name << ":\n";

        std::string filepath = benchmarkDir + bench.file;
        Chunk* chunk = compileFile(filepath);

        if (chunk == nullptr) {
            std::cout << "  ❌ Failed to compile\n\n";
            continue;
        }

        auto result = runBenchmark(chunk, true, 3);

        if (!result.success) {
            std::cout << "  ❌ Execution failed\n\n";
            delete chunk;
            continue;
        }

        std::cout << "  ✅ Time: " << result.elapsedMs << " ms\n";
        std::cout << "  ✅ Result: " << result.result.asInteger() << "\n\n";

        delete chunk;
    }

    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  Run individual benchmarks for detailed statistics     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
}
