/**
 * @file ModuleTest.cpp
 * @brief 모듈 시스템 테스트
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include <gtest/gtest.h>
#include "module/ModuleLoader.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Environment.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include <fstream>
#include <filesystem>

using namespace kingsejong::module;
using namespace kingsejong::evaluator;
using namespace kingsejong::lexer;
using namespace kingsejong::parser;

/**
 * @class ModuleTestFixture
 * @brief 모듈 테스트를 위한 픽스처
 */
class ModuleTestFixture : public ::testing::Test
{
protected:
    std::string testModulePath_ = "test_modules";

    void SetUp() override
    {
        // 테스트 모듈 디렉토리 생성
        std::filesystem::create_directories(testModulePath_);
    }

    void TearDown() override
    {
        // 테스트 모듈 디렉토리 정리
        std::filesystem::remove_all(testModulePath_);
    }

    /**
     * @brief 테스트 모듈 파일 생성
     * @param moduleName 모듈 이름 (확장자 제외)
     * @param content 모듈 내용
     */
    void createTestModule(const std::string& moduleName, const std::string& content)
    {
        std::string filePath = testModulePath_ + "/" + moduleName + ".ksj";
        std::ofstream file(filePath);
        file << content;
        file.close();
    }
};

/**
 * @test 기본 모듈 로딩
 */
TEST_F(ModuleTestFixture, ShouldLoadBasicModule)
{
    // math.ksj 모듈 생성
    createTestModule("math", R"(
        정수 PI = 3
        정수 E = 2
    )");

    ModuleLoader loader(testModulePath_);
    auto moduleEnv = loader.loadModule("math");

    EXPECT_NE(moduleEnv, nullptr);
    EXPECT_EQ(moduleEnv->get("PI").asInteger(), 3);
    EXPECT_EQ(moduleEnv->get("E").asInteger(), 2);
}

/**
 * @test 모듈 캐싱
 */
TEST_F(ModuleTestFixture, ShouldCacheLoadedModule)
{
    createTestModule("cached", R"(
        정수 value = 100
    )");

    ModuleLoader loader(testModulePath_);

    // 첫 번째 로딩
    auto env1 = loader.loadModule("cached");
    EXPECT_TRUE(loader.isCached("cached"));

    // 두 번째 로딩 (캐시에서 가져옴)
    auto env2 = loader.loadModule("cached");

    // 같은 Environment 객체를 반환해야 함
    EXPECT_EQ(env1, env2);
}

/**
 * @test 순환 참조 감지
 */
TEST_F(ModuleTestFixture, ShouldDetectCircularDependency)
{
    createTestModule("a", R"(
        가져오기 "b"
        정수 valueA = 1
    )");

    createTestModule("b", R"(
        가져오기 "a"
        정수 valueB = 2
    )");

    ModuleLoader loader(testModulePath_);

    EXPECT_THROW(loader.loadModule("a"), std::exception);
}

/**
 * @test 존재하지 않는 모듈 로딩 시 예외 발생
 */
TEST_F(ModuleTestFixture, ShouldThrowWhenModuleNotFound)
{
    ModuleLoader loader(testModulePath_);

    EXPECT_THROW(loader.loadModule("nonexistent"), std::exception);
}

/**
 * @test Import문을 통한 모듈 로딩
 */
TEST_F(ModuleTestFixture, ShouldLoadModuleViaImportStatement)
{
    createTestModule("utils", R"(
        정수 MAX_VALUE = 100
        정수 MIN_VALUE = 0
    )");

    std::string source = R"(
        가져오기 "utils"
        정수 result = MAX_VALUE + MIN_VALUE
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_TRUE(parser.errors().empty());

    auto env = std::make_shared<Environment>();
    Evaluator evaluator(env);

    ModuleLoader loader(testModulePath_);
    evaluator.setModuleLoader(&loader);

    evaluator.evalProgram(program.get());

    EXPECT_EQ(env->get("MAX_VALUE").asInteger(), 100);
    EXPECT_EQ(env->get("MIN_VALUE").asInteger(), 0);
    EXPECT_EQ(env->get("result").asInteger(), 100);
}

/**
 * @test 함수가 포함된 모듈 로딩
 */
TEST_F(ModuleTestFixture, ShouldLoadModuleWithFunctions)
{
    createTestModule("math_utils", R"(
        함수 더하기(a, b) {
            반환 a + b
        }

        함수 곱하기(x, y) {
            반환 x * y
        }
    )");

    std::string source = R"(
        가져오기 "math_utils"
        정수 sum = 더하기(3, 4)
        정수 product = 곱하기(5, 6)
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_TRUE(parser.errors().empty());

    auto env = std::make_shared<Environment>();
    Evaluator evaluator(env);

    ModuleLoader loader(testModulePath_);
    evaluator.setModuleLoader(&loader);

    evaluator.evalProgram(program.get());

    EXPECT_EQ(env->get("sum").asInteger(), 7);
    EXPECT_EQ(env->get("product").asInteger(), 30);
}

/**
 * @test 캐시 초기화
 */
TEST_F(ModuleTestFixture, ShouldClearCache)
{
    createTestModule("temp", R"(
        정수 value = 42
    )");

    ModuleLoader loader(testModulePath_);

    loader.loadModule("temp");
    EXPECT_TRUE(loader.isCached("temp"));

    loader.clearCache();
    EXPECT_FALSE(loader.isCached("temp"));
}

/**
 * @test 경로 해석
 */
TEST_F(ModuleTestFixture, ShouldResolvePath)
{
    ModuleLoader loader(testModulePath_);

    // 모듈 이름만 주어진 경우
    std::string path1 = loader.resolvePath("math");
    EXPECT_EQ(path1, testModulePath_ + "/math.ksj");

    // 상대 경로
    std::string path2 = loader.resolvePath("./utils");
    EXPECT_EQ(path2, testModulePath_ + "/./utils.ksj");

    // 이미 .ksj로 끝나는 경우
    std::string path3 = loader.resolvePath("test.ksj");
    EXPECT_EQ(path3, "test.ksj");
}
