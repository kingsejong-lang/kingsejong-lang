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
#include "semantic/SemanticAnalyzer.h"

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

    // 어떤 예외든 통과시키면 "로더가 없다" 같은 다른 실패도 통과한다. 메시지를 특정한다.
    try
    {
        loader.loadModule("a");
        FAIL() << "순환 참조가 감지되지 않았다";
    }
    catch (const std::exception& e)
    {
        EXPECT_NE(std::string(e.what()).find("순환 참조"), std::string::npos) << e.what();
    }
}

/**
 * @test 모듈 안의 가져오기 (중첩 가져오기) 가 동작한다
 */
TEST_F(ModuleTestFixture, ShouldLoadNestedImports)
{
    createTestModule("base", R"(
        정수 BASE = 10
    )");
    createTestModule("derived", R"(
        가져오기 "base"
        정수 DERIVED = BASE + 5
    )");

    ModuleLoader loader(testModulePath_);
    auto env = loader.loadModule("derived");

    EXPECT_EQ(env->get("DERIVED").asInteger(), 15);
}

/**
 * @test 가져오기 문의 상대 경로는 그 문이 있는 파일을 기준으로 해석된다
 */
TEST_F(ModuleTestFixture, ShouldResolveRelativeToImportingFile)
{
    std::filesystem::create_directories(testModulePath_ + "/sub");
    createTestModule("sub/helper", R"(
        정수 HELPER = 7
    )");
    createTestModule("sub/main_module", R"(
        가져오기 "./helper"
        정수 RESULT = HELPER * 2
    )");

    // basePath 는 상위 디렉터리인데도 sub/ 안의 helper 를 찾아야 한다
    ModuleLoader loader(testModulePath_);
    auto env = loader.loadModule("sub/main_module");

    EXPECT_EQ(env->get("RESULT").asInteger(), 14);
}

/**
 * @test 추가 검색 경로와 KINGSEJONG_PATH 로 모듈을 찾는다
 */
TEST_F(ModuleTestFixture, ShouldSearchAdditionalPaths)
{
    std::filesystem::create_directories(testModulePath_ + "/lib");
    createTestModule("lib/extra", R"(
        정수 EXTRA = 1
    )");

    ModuleLoader loader("some/nonexistent/base");
    loader.addSearchPath(testModulePath_ + "/lib");
    auto env = loader.loadModule("extra");
    EXPECT_EQ(env->get("EXTRA").asInteger(), 1);

    auto candidates = loader.candidatePaths("extra");
    ASSERT_GE(candidates.size(), 2u);
    EXPECT_EQ(candidates[0], (std::filesystem::path(testModulePath_) / "lib" / "extra.ksj").lexically_normal().string());
}

/**
 * @test 짧은 모듈 이름도 예외 없이 해석된다 (예전에는 substr 이 out_of_range 를 던졌다)
 */
TEST_F(ModuleTestFixture, ShouldHandleShortModuleNames)
{
    createTestModule("ab", R"(
        정수 AB = 2
    )");
    ModuleLoader loader(testModulePath_);
    EXPECT_EQ(loader.loadModule("ab")->get("AB").asInteger(), 2);
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

    // 로더가 모듈 환경을 풀려면 가져온 함수를 쥔 이 환경이 먼저 비워져야 한다:
    // 로더 → 환경 → 평가기 순으로 만들어 역순으로 정리되게 한다.
    ModuleLoader loader(testModulePath_);
    auto env = std::make_shared<Environment>();
    {
        Evaluator evaluator(env);
        evaluator.setModuleLoader(&loader);
        evaluator.evalProgram(program.get());

        EXPECT_EQ(env->get("sum").asInteger(), 7);
        EXPECT_EQ(env->get("product").asInteger(), 30);
    }
    env->releaseIfUnreachable(1);
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
    // 경로는 정규화된다 ("./" 가 사라진다)
    std::string path2 = loader.resolvePath("./utils");
    EXPECT_EQ(path2, testModulePath_ + "/utils.ksj");

    // 이미 .ksj로 끝나는 경우
    // 확장자가 있어도 검색 규칙은 같다. 없는 파일이면 첫 후보(basePath)를 돌려준다
    std::string path3 = loader.resolvePath("test.ksj");
    EXPECT_EQ(path3, testModulePath_ + "/test.ksj");
}

/**
 * @test 의미 분석기가 가져온 모듈의 이름을 알아서 "정의되지 않은 식별자"를 내지 않는다
 */
TEST_F(ModuleTestFixture, SemanticAnalyzerKnowsImportedNames)
{
    createTestModule("semmod", R"(
        정수 상수값 = 7
        함수 더하기(a, b) {
            반환 a + b
        }
    )");

    std::string source = R"(
        가져오기 "semmod"
        정수 결과 = 더하기(상수값, 1)
    )";

    Lexer lexer(source, "main.ksj");
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_TRUE(parser.errors().empty());

    ModuleLoader loader(testModulePath_);
    kingsejong::semantic::SemanticAnalyzer analyzer;
    analyzer.setModuleResolver([&loader](const std::string& m, const std::string& f) {
        return loader.resolvePath(m, f);
    });

    EXPECT_TRUE(analyzer.analyze(program.get()));
    EXPECT_TRUE(analyzer.errors().empty()) << analyzer.errors()[0].toString();

    // 해석기를 주지 않으면 예전처럼 가져온 이름을 알 수 없다
    kingsejong::semantic::SemanticAnalyzer plain;
    EXPECT_FALSE(plain.analyze(program.get()));
}
