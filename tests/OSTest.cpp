/**
 * @file OSTest.cpp
 * @brief OS 및 환경 변수 내장 함수 테스트
 * @author KingSejong Team
 * @date 2025-11-15
 *
 * 환경 변수, 경로 조작, 시스템 정보 등 OS 관련 함수 테스트
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include <filesystem>
#include <fstream>
#include <cstdlib>

using namespace kingsejong;
using namespace kingsejong::evaluator;

class OSTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Builtin::registerAllBuiltins();

        // 테스트용 임시 디렉토리 생성
        test_dir_ = std::filesystem::temp_directory_path() / "kingsejong_os_test";
        std::filesystem::create_directories(test_dir_);
    }

    void TearDown() override
    {
        // 테스트용 임시 디렉토리 삭제
        if (std::filesystem::exists(test_dir_)) {
            std::filesystem::remove_all(test_dir_);
        }

        // 테스트 환경 변수 정리
        #ifdef _WIN32
            _putenv("KINGSEJONG_TEST_VAR=");
        #else
            unsetenv("KINGSEJONG_TEST_VAR");
        #endif
    }

    Value evaluate(const std::string& code)
    {
        lexer::Lexer lexer(code);
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();

        Evaluator evaluator;
        return evaluator.evalProgram(program.get());
    }

    std::filesystem::path test_dir_;
};

// ============================================================================
// 환경 변수 테스트
// ============================================================================

TEST_F(OSTest, ShouldReadEnvironmentVariable)
{
    // Arrange - PATH는 대부분의 시스템에 존재
    std::string code = "환경변수_읽기(\"PATH\")";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_GT(result.asString().length(), 0u);
}

TEST_F(OSTest, ShouldWriteAndReadEnvironmentVariable)
{
    // Act - 환경 변수 쓰기
    evaluate("환경변수_쓰기(\"KINGSEJONG_TEST_VAR\", \"테스트값\")");

    // Act - 환경 변수 읽기
    Value result = evaluate("환경변수_읽기(\"KINGSEJONG_TEST_VAR\")");

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "테스트값");
}

TEST_F(OSTest, ShouldCheckEnvironmentVariableExists)
{
    // Arrange
    evaluate("환경변수_쓰기(\"KINGSEJONG_TEST_VAR\", \"값\")");

    // Act - 존재하는 변수
    Value result1 = evaluate("환경변수_존재하는가(\"KINGSEJONG_TEST_VAR\")");

    // Assert
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    // Act - 존재하지 않는 변수
    Value result2 = evaluate("환경변수_존재하는가(\"NON_EXISTING_VAR_12345\")");

    // Assert
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

TEST_F(OSTest, ShouldDeleteEnvironmentVariable)
{
    // Arrange
    evaluate("환경변수_쓰기(\"KINGSEJONG_TEST_VAR\", \"값\")");
    ASSERT_TRUE(evaluate("환경변수_존재하는가(\"KINGSEJONG_TEST_VAR\")").asBoolean());

    // Act
    evaluate("환경변수_삭제(\"KINGSEJONG_TEST_VAR\")");

    // Assert
    Value result = evaluate("환경변수_존재하는가(\"KINGSEJONG_TEST_VAR\")");
    EXPECT_FALSE(result.asBoolean());
}

// ============================================================================
// 경로 조작 테스트
// ============================================================================

TEST_F(OSTest, ShouldCombinePaths)
{
    // Act
    Value result = evaluate("경로_결합(\"/home/user\", \"documents\")");

    // Assert
    EXPECT_TRUE(result.isString());
    std::string combined = result.asString();
    EXPECT_TRUE(combined.find("home") != std::string::npos);
    EXPECT_TRUE(combined.find("user") != std::string::npos);
    EXPECT_TRUE(combined.find("documents") != std::string::npos);
}

TEST_F(OSTest, ShouldExtractFilename)
{
    // Act
    Value result = evaluate("파일명_추출(\"/home/user/document.txt\")");

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "document.txt");
}

TEST_F(OSTest, ShouldExtractExtension)
{
    // Act
    Value result = evaluate("확장자_추출(\"/home/user/document.txt\")");

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), ".txt");
}

TEST_F(OSTest, ShouldGetAbsolutePath)
{
    // Act
    Value result = evaluate("절대경로(\".\")");

    // Assert
    EXPECT_TRUE(result.isString());
    std::string abs_path = result.asString();
    EXPECT_GT(abs_path.length(), 0u);
    // 절대 경로는 루트에서 시작
    #ifdef _WIN32
        EXPECT_TRUE(abs_path.find(":") != std::string::npos); // C:\ 같은 형식
    #else
        EXPECT_EQ(abs_path[0], '/'); // Unix는 /로 시작
    #endif
}

TEST_F(OSTest, ShouldCheckPathExists)
{
    // Arrange - 테스트 파일 생성
    std::string test_file = (test_dir_ / "test_path.txt").string();
    std::ofstream file(test_file);
    file << "test";
    file.close();

    // Act & Assert - 존재하는 경로
    Value result1 = evaluate("경로_존재하는가(\"" + test_file + "\")");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    // Act & Assert - 존재하지 않는 경로
    Value result2 = evaluate("경로_존재하는가(\"/non/existing/path/12345\")");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

// ============================================================================
// 디렉토리 테스트
// ============================================================================

TEST_F(OSTest, ShouldGetCurrentDirectory)
{
    // Act
    Value result = evaluate("현재_디렉토리()");

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_GT(result.asString().length(), 0u);
}

TEST_F(OSTest, ShouldGetTemporaryDirectory)
{
    // Act
    Value result = evaluate("임시_디렉토리()");

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_GT(result.asString().length(), 0u);

    // 임시 디렉토리가 실제로 존재하는지 확인
    EXPECT_TRUE(std::filesystem::exists(result.asString()));
}

TEST_F(OSTest, ShouldCheckIsDirectory)
{
    // Arrange - 테스트 디렉토리와 파일 생성
    std::string test_subdir = (test_dir_ / "subdir").string();
    std::filesystem::create_directories(test_subdir);

    std::string test_file = (test_dir_ / "file.txt").string();
    std::ofstream file(test_file);
    file << "test";
    file.close();

    // Act & Assert - 디렉토리
    Value result1 = evaluate("디렉토리인가(\"" + test_subdir + "\")");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    // Act & Assert - 파일
    Value result2 = evaluate("디렉토리인가(\"" + test_file + "\")");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());

    // Act & Assert - 존재하지 않는 경로
    Value result3 = evaluate("디렉토리인가(\"/non/existing/path\")");
    EXPECT_TRUE(result3.isBoolean());
    EXPECT_FALSE(result3.asBoolean());
}

// ============================================================================
// 파일 시스템 테스트
// ============================================================================

TEST_F(OSTest, ShouldCheckFileExists)
{
    // Arrange - 테스트 파일 생성
    std::string test_file = (test_dir_ / "exists_test.txt").string();
    std::ofstream file(test_file);
    file << "test";
    file.close();

    // Act & Assert - 존재하는 파일
    Value result1 = evaluate("파일_존재하는가(\"" + test_file + "\")");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    // Act & Assert - 존재하지 않는 파일
    Value result2 = evaluate("파일_존재하는가(\"/non/existing/file.txt\")");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

TEST_F(OSTest, ShouldCheckIsFile)
{
    // Arrange
    std::string test_file = (test_dir_ / "file_test.txt").string();
    std::ofstream file(test_file);
    file << "test";
    file.close();

    std::string test_subdir = (test_dir_ / "subdir_test").string();
    std::filesystem::create_directories(test_subdir);

    // Act & Assert - 파일
    Value result1 = evaluate("파일인가(\"" + test_file + "\")");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    // Act & Assert - 디렉토리
    Value result2 = evaluate("파일인가(\"" + test_subdir + "\")");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

TEST_F(OSTest, ShouldGetFileSize)
{
    // Arrange - 특정 크기의 파일 생성
    std::string test_file = (test_dir_ / "size_test.txt").string();
    std::ofstream file(test_file);
    std::string content = "Hello, World!"; // 13 bytes (ASCII)
    file << content;
    file.close();

    // Act
    Value result = evaluate("파일_크기(\"" + test_file + "\")");

    // Assert
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), static_cast<int64_t>(content.length()));
}

TEST_F(OSTest, ShouldCopyFile)
{
    // Arrange - 원본 파일 생성
    std::string src_file = (test_dir_ / "source.txt").string();
    std::ofstream file(src_file);
    file << "원본 내용";
    file.close();

    std::string dest_file = (test_dir_ / "destination.txt").string();

    // Act
    evaluate("파일_복사(\"" + src_file + "\", \"" + dest_file + "\")");

    // Assert - 복사된 파일 존재 확인
    EXPECT_TRUE(std::filesystem::exists(dest_file));

    // Assert - 내용 확인
    Value result = evaluate("파일_읽기(\"" + dest_file + "\")");
    EXPECT_EQ(result.asString(), "원본 내용");

    // Assert - 원본 파일도 그대로 존재
    EXPECT_TRUE(std::filesystem::exists(src_file));
}

TEST_F(OSTest, ShouldMoveFile)
{
    // Arrange - 원본 파일 생성
    std::string src_file = (test_dir_ / "move_source.txt").string();
    std::ofstream file(src_file);
    file << "이동할 내용";
    file.close();

    std::string dest_file = (test_dir_ / "move_destination.txt").string();

    // Act
    evaluate("파일_이동(\"" + src_file + "\", \"" + dest_file + "\")");

    // Assert - 이동된 파일 존재 확인
    EXPECT_TRUE(std::filesystem::exists(dest_file));

    // Assert - 원본 파일은 삭제됨
    EXPECT_FALSE(std::filesystem::exists(src_file));

    // Assert - 내용 확인
    Value result = evaluate("파일_읽기(\"" + dest_file + "\")");
    EXPECT_EQ(result.asString(), "이동할 내용");
}

// ============================================================================
// 시스템 정보 테스트
// ============================================================================

TEST_F(OSTest, ShouldGetOSName)
{
    // Act
    Value result = evaluate("OS_이름()");

    // Assert
    EXPECT_TRUE(result.isString());
    std::string os_name = result.asString();
    EXPECT_GT(os_name.length(), 0u);

    // 알려진 OS 이름 중 하나여야 함
    EXPECT_TRUE(
        os_name == "Windows" ||
        os_name == "macOS" ||
        os_name == "Linux" ||
        os_name == "Unix"
    );
}

TEST_F(OSTest, ShouldGetUsername)
{
    // Act
    Value result = evaluate("사용자_이름()");

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_GT(result.asString().length(), 0u);
    EXPECT_NE(result.asString(), "unknown"); // 정상적인 환경에서는 사용자 이름을 가져올 수 있어야 함
}

TEST_F(OSTest, ShouldGetHostname)
{
    // Act
    Value result = evaluate("호스트_이름()");

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_GT(result.asString().length(), 0u);
}
