/**
 * @file EvaluatorHotPathTest.cpp
 * @brief Evaluator와 HotPathDetector 통합 테스트
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "jit/HotPathDetector.h"
#include <algorithm>

using namespace kingsejong;
using namespace kingsejong::evaluator;
using namespace kingsejong::jit;

class EvaluatorHotPathTest : public ::testing::Test {
protected:
    void SetUp() override {
        detector = std::make_shared<HotPathDetector>();
        detector->setFunctionThreshold(10);  // 낮은 임계값으로 테스트
        detector->setLoopThreshold(50);
    }

    Value runCode(const std::string& code) {
        lexer::Lexer lexer(code);
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();

        if (parser.errors().size() > 0) {
            for (const auto& err : parser.errors()) {
                std::cout << "Parser error: " << err << std::endl;
            }
        }

        EXPECT_EQ(parser.errors().size(), 0);

        Evaluator evaluator;
        evaluator.setHotPathDetector(detector.get());
        return evaluator.eval(program.get());
    }

    std::shared_ptr<HotPathDetector> detector;
};

// ============================================================================
// 함수 호출 추적 테스트
// ============================================================================

TEST_F(EvaluatorHotPathTest, ShouldTrackFunctionCalls) {
    std::string code = R"(
        함수 피보나치(n) {
            만약 (n <= 1) {
                반환 n
            }
            반환 피보나치(n - 1) + 피보나치(n - 2)
        }

        피보나치(5)
    )";

    runCode(code);

    // 함수가 여러 번 호출되어야 함
    auto& profiles = detector->getFunctionProfiles();
    EXPECT_GT(profiles.size(), 0) << "함수 프로파일이 기록되어야 함";

    // 충분히 호출되었는지 확인
    bool hasHotFunction = false;
    for (const auto& [id, profile] : profiles) {
        if (profile.executionCount >= 10) {
            hasHotFunction = true;
            break;
        }
    }
    EXPECT_TRUE(hasHotFunction) << "핫 함수가 감지되어야 함";
}

TEST_F(EvaluatorHotPathTest, ShouldDetectHotFunction) {
    std::string code = R"(
        함수 계산() {
            반환 42
        }

        i가 1부터 15까지 반복한다 {
            계산()
        }
    )";

    runCode(code);

    auto hotFunctions = detector->getHotFunctions();
    EXPECT_GT(hotFunctions.size(), 0) << "핫 함수가 감지되어야 함";

    // 첫 번째 핫 함수 검증
    size_t hotFuncId = hotFunctions[0];
    auto profile = detector->getProfile(hotFuncId, HotPathType::FUNCTION);
    ASSERT_NE(profile, nullptr);
    EXPECT_GE(profile->executionCount, 15) << "15번 이상 호출되어야 함";
    EXPECT_TRUE(detector->isHot(hotFuncId, HotPathType::FUNCTION));
}

TEST_F(EvaluatorHotPathTest, ShouldTrackFunctionName) {
    std::string code = R"(
        함수 테스트함수() {
            반환 1
        }

        i가 1부터 12까지 반복한다 {
            테스트함수()
        }
    )";

    runCode(code);

    auto& profiles = detector->getFunctionProfiles();
    bool foundCorrectName = false;

    for (const auto& [id, profile] : profiles) {
        if (profile.name == "테스트함수") {
            foundCorrectName = true;
            EXPECT_GE(profile.executionCount, 12);
        }
    }

    EXPECT_TRUE(foundCorrectName) << "함수 이름이 올바르게 추적되어야 함";
}

// ============================================================================
// 루프 백엣지 추적 테스트
// ============================================================================

TEST_F(EvaluatorHotPathTest, ShouldTrackRepeatLoop) {
    std::string code = R"(
        100번 반복한다 {
            정수 x = 1
        }
    )";

    runCode(code);

    auto& profiles = detector->getLoopProfiles();
    EXPECT_GT(profiles.size(), 0) << "루프 프로파일이 기록되어야 함";

    // 루프 백엣지 횟수 확인
    for (const auto& [id, profile] : profiles) {
        EXPECT_EQ(profile.executionCount, 100) << "100회 백엣지가 기록되어야 함";
        EXPECT_TRUE(detector->isHot(id, HotPathType::LOOP)) << "루프가 핫으로 감지되어야 함";
    }
}

TEST_F(EvaluatorHotPathTest, ShouldTrackRangeForLoop) {
    std::string code = R"(
        i가 1부터 80까지 반복한다 {
            정수 y = i * 2
        }
    )";

    runCode(code);

    auto hotLoops = detector->getHotLoops();
    EXPECT_GT(hotLoops.size(), 0) << "핫 루프가 감지되어야 함";

    size_t hotLoopId = hotLoops[0];
    auto profile = detector->getProfile(hotLoopId, HotPathType::LOOP);
    ASSERT_NE(profile, nullptr);
    EXPECT_EQ(profile->executionCount, 80) << "80회 실행되어야 함";
}

TEST_F(EvaluatorHotPathTest, ShouldTrackNestedLoops) {
    std::string code = R"(
        20번 반복한다 {
            10번 반복한다 {
                정수 x = 1
            }
        }
    )";

    runCode(code);

    auto& profiles = detector->getLoopProfiles();
    EXPECT_EQ(profiles.size(), 2) << "2개의 루프가 추적되어야 함";

    // 외부 루프: 20회, 내부 루프: 200회
    std::vector<uint64_t> counts;
    for (const auto& [id, profile] : profiles) {
        counts.push_back(profile.executionCount);
    }

    std::sort(counts.begin(), counts.end());
    EXPECT_EQ(counts[0], 20) << "외부 루프는 20회";
    EXPECT_EQ(counts[1], 200) << "내부 루프는 200회";
}

// ============================================================================
// 함수 + 루프 통합 테스트
// ============================================================================

TEST_F(EvaluatorHotPathTest, ShouldTrackFunctionWithLoop) {
    std::string code = R"(
        함수 합계(n) {
            정수 결과 = 0
            정수 limit = n
            i가 1부터 limit까지 반복한다 {
                결과 = 결과 + i
            }
            반환 결과
        }

        j가 1부터 12까지 반복한다 {
            합계(10)
        }
    )";

    runCode(code);

    // 함수 추적 확인
    auto hotFunctions = detector->getHotFunctions();
    EXPECT_GT(hotFunctions.size(), 0) << "함수가 핫으로 감지되어야 함";

    // 루프 추적 확인
    auto hotLoops = detector->getHotLoops();
    EXPECT_GT(hotLoops.size(), 0) << "루프가 핫으로 감지되어야 함";
}

// ============================================================================
// 실행 시간 추적 테스트
// ============================================================================

TEST_F(EvaluatorHotPathTest, ShouldTrackExecutionTime) {
    std::string code = R"(
        함수 계산() {
            정수 sum = 0
            i가 1부터 100까지 반복한다 {
                sum = sum + i
            }
            반환 sum
        }

        j가 1부터 10까지 반복한다 {
            계산()
        }
    )";

    runCode(code);

    auto& profiles = detector->getFunctionProfiles();
    for (const auto& [id, profile] : profiles) {
        if (profile.name == "계산") {
            EXPECT_GT(profile.totalTime, 0) << "총 실행 시간이 기록되어야 함";
            EXPECT_GT(profile.avgTime, 0) << "평균 실행 시간이 기록되어야 함";
            EXPECT_EQ(profile.avgTime, profile.totalTime / profile.executionCount)
                << "평균 시간이 올바르게 계산되어야 함";
        }
    }
}

// ============================================================================
// 비활성화 테스트
// ============================================================================

TEST_F(EvaluatorHotPathTest, ShouldNotTrackWhenDetectorNotSet) {
    std::string code = R"(
        함수 테스트() {
            반환 42
        }

        100번 반복한다 {
            테스트()
        }
    )";

    // HotPathDetector를 설정하지 않음
    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    Evaluator evaluator;  // detector 미설정
    evaluator.eval(program.get());

    // detector가 설정되지 않았으므로 프로파일이 비어있어야 함
    EXPECT_EQ(detector->getFunctionProfiles().size(), 0);
    EXPECT_EQ(detector->getLoopProfiles().size(), 0);
}

// ============================================================================
// 통계 출력 테스트
// ============================================================================

TEST_F(EvaluatorHotPathTest, ShouldPrintStatistics) {
    std::string code = R"(
        함수 테스트(n) {
            정수 결과 = 0
            정수 limit = n
            i가 1부터 limit까지 반복한다 {
                결과 = 결과 + i
            }
            반환 결과
        }

        15번 반복한다 {
            테스트(20)
        }
    )";

    runCode(code);

    // 통계 출력이 크래시 없이 실행되어야 함
    EXPECT_NO_THROW(detector->printStatistics());
    EXPECT_NO_THROW(detector->printHotPaths(5));
}
