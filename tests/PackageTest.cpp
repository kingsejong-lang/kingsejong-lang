/**
 * @file PackageTest.cpp
 * @brief 패키지 관리자 테스트
 */

#include <gtest/gtest.h>
#include "package/Package.h"
#include "package/PackageManager.h"
#include <filesystem>
#include <fstream>

using namespace kingsejong::package;
namespace fs = std::filesystem;

class PackageTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 임시 테스트 디렉토리 생성
        testDir = fs::temp_directory_path() / "kingsejong_test";
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
        fs::create_directories(testDir);
    }

    void TearDown() override
    {
        // 테스트 디렉토리 삭제
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
    }

    fs::path testDir;
};

TEST_F(PackageTest, CreatePackage)
{
    Package pkg("test-package", "1.0.0");

    EXPECT_EQ(pkg.name(), "test-package");
    EXPECT_EQ(pkg.version(), "1.0.0");
    EXPECT_TRUE(pkg.isValid());
}

TEST_F(PackageTest, PackageWithMetadata)
{
    Package pkg("my-package", "2.3.4");
    pkg.setDescription("테스트 패키지입니다");
    pkg.setLicense("Apache-2.0");
    pkg.setMain("lib/index.ksj");

    pkg.addAuthor(Author("홍길동", "hong@example.com"));
    pkg.addDependency(Dependency("stdlib", "^1.0.0"));
    pkg.addDevDependency(Dependency("test-lib", "~2.0.0"));
    pkg.addScript("test", "kingsejong test.ksj");

    EXPECT_EQ(pkg.description(), "테스트 패키지입니다");
    EXPECT_EQ(pkg.license(), "Apache-2.0");
    EXPECT_EQ(pkg.main(), "lib/index.ksj");
    EXPECT_EQ(pkg.authors().size(), 1);
    EXPECT_EQ(pkg.dependencies().size(), 1);
    EXPECT_EQ(pkg.devDependencies().size(), 1);
    EXPECT_EQ(pkg.scripts().size(), 1);
}

TEST_F(PackageTest, PackageToJSON)
{
    Package pkg("json-test", "0.1.0");
    pkg.setDescription("JSON 변환 테스트");
    pkg.addAuthor(Author("테스터"));
    pkg.addDependency(Dependency("dep1", "^1.0.0"));

    std::string json = pkg.toJSON();

    EXPECT_NE(json.find("json-test"), std::string::npos);
    EXPECT_NE(json.find("0.1.0"), std::string::npos);
    EXPECT_NE(json.find("JSON 변환 테스트"), std::string::npos);
}

TEST_F(PackageTest, PackageFromJSON)
{
    std::string json = R"({
        "name": "from-json",
        "version": "3.2.1",
        "description": "JSON에서 로드 테스트",
        "license": "BSD-3",
        "main": "src/main.ksj",
        "authors": [
            {"name": "작성자1", "email": "author1@test.com"},
            "작성자2"
        ],
        "dependencies": {
            "dep1": "^1.0.0",
            "dep2": "~2.3.0"
        },
        "dev_dependencies": {
            "test-lib": "^1.5.0"
        },
        "scripts": {
            "build": "ksjpm build",
            "test": "ksjpm test"
        }
    })";

    Package pkg = Package::fromJSON(json);

    EXPECT_EQ(pkg.name(), "from-json");
    EXPECT_EQ(pkg.version(), "3.2.1");
    EXPECT_EQ(pkg.description(), "JSON에서 로드 테스트");
    EXPECT_EQ(pkg.license(), "BSD-3");
    EXPECT_EQ(pkg.main(), "src/main.ksj");
    EXPECT_EQ(pkg.authors().size(), 2);
    EXPECT_EQ(pkg.dependencies().size(), 2);
    EXPECT_EQ(pkg.devDependencies().size(), 1);
    EXPECT_EQ(pkg.scripts().size(), 2);
}

TEST_F(PackageTest, PackageFromFile)
{
    // 테스트 package.ksj 파일 생성
    fs::path packageFile = testDir / "package.ksj";
    std::ofstream file(packageFile);
    file << R"({
        "name": "file-test",
        "version": "1.2.3",
        "description": "파일에서 로드 테스트"
    })";
    file.close();

    Package pkg = Package::fromFile(packageFile.string());

    EXPECT_EQ(pkg.name(), "file-test");
    EXPECT_EQ(pkg.version(), "1.2.3");
    EXPECT_EQ(pkg.description(), "파일에서 로드 테스트");
}

TEST_F(PackageTest, InvalidPackage)
{
    std::string invalidJson = R"({
        "description": "이름과 버전이 없음"
    })";

    EXPECT_THROW(Package::fromJSON(invalidJson), std::runtime_error);
}

TEST_F(PackageTest, PackageManagerInit)
{
    PackageManager pm(testDir.string());

    Package pkg = pm.initPackage("test-project", "0.1.0", "테스트 프로젝트");

    EXPECT_EQ(pkg.name(), "test-project");
    EXPECT_EQ(pkg.version(), "0.1.0");
    EXPECT_EQ(pkg.description(), "테스트 프로젝트");

    // package.ksj 파일이 생성되었는지 확인
    EXPECT_TRUE(fs::exists(testDir / "package.ksj"));
}

TEST_F(PackageTest, PackageManagerLoadSave)
{
    PackageManager pm(testDir.string());

    // 초기화
    Package original = pm.initPackage("load-save-test", "1.0.0");
    original.addDependency(Dependency("stdlib", "^1.0.0"));
    pm.saveProjectPackage(original);

    // 로드
    Package loaded = pm.loadProjectPackage();

    EXPECT_EQ(loaded.name(), "load-save-test");
    EXPECT_EQ(loaded.version(), "1.0.0");
    EXPECT_EQ(loaded.dependencies().size(), 1);
}

TEST_F(PackageTest, PackageManagerModulesPath)
{
    PackageManager pm(testDir.string());

    std::string modulesPath = pm.getModulesPath();
    std::string expected = (testDir / "node_modules").string();

    EXPECT_EQ(modulesPath, expected);
}

TEST_F(PackageTest, PackageRegistryBasic)
{
    PackageRegistry registry;

    Package pkg1("pkg1", "1.0.0");
    Package pkg2("pkg2", "2.0.0");

    registry.registerPackage(pkg1);
    registry.registerPackage(pkg2);

    EXPECT_TRUE(registry.hasPackage("pkg1"));
    EXPECT_TRUE(registry.hasPackage("pkg2"));
    EXPECT_FALSE(registry.hasPackage("pkg3"));

    const Package* retrieved = registry.getPackage("pkg1");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->name(), "pkg1");
    EXPECT_EQ(retrieved->version(), "1.0.0");
}
