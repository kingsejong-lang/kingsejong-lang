/**
 * @file FileIOTest.cpp
 * @brief 파일 I/O 내장 함수 테스트
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * 파일 읽기/쓰기, 디렉토리 관리 등 파일 I/O 함수 테스트
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include <filesystem>
#include <fstream>

using namespace kingsejong;
using namespace kingsejong::evaluator;

class FileIOTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Builtin::registerAllBuiltins();

        // 테스트용 임시 디렉토리 생성
        test_dir_ = std::filesystem::temp_directory_path() / "kingsejong_test";
        std::filesystem::create_directories(test_dir_);
    }

    void TearDown() override
    {
        // 테스트용 임시 디렉토리 삭제
        if (std::filesystem::exists(test_dir_)) {
            std::filesystem::remove_all(test_dir_);
        }
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
// 파일 읽기/쓰기 테스트
// ============================================================================

TEST_F(FileIOTest, ShouldWriteAndReadFile)
{
    // Arrange
    std::string filepath = (test_dir_ / "test.txt").string();
    std::string code = "파일_쓰기(\"" + filepath + "\", \"안녕하세요\")";

    // Act - 파일 쓰기
    evaluate(code);

    // Assert - 파일 존재 확인
    ASSERT_TRUE(std::filesystem::exists(filepath));

    // Act - 파일 읽기
    std::string read_code = "파일_읽기(\"" + filepath + "\")";
    Value result = evaluate(read_code);

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "안녕하세요");
}

TEST_F(FileIOTest, ShouldAppendToFile)
{
    // Arrange
    std::string filepath = (test_dir_ / "append_test.txt").string();

    // Act - 첫 번째 쓰기
    evaluate("파일_쓰기(\"" + filepath + "\", \"첫 줄\\n\")");

    // Act - 추가 쓰기
    evaluate("파일_추가(\"" + filepath + "\", \"두 번째 줄\\n\")");
    evaluate("파일_추가(\"" + filepath + "\", \"세 번째 줄\")");

    // Assert - 파일 읽기
    Value result = evaluate("파일_읽기(\"" + filepath + "\")");
    EXPECT_EQ(result.asString(), "첫 줄\n두 번째 줄\n세 번째 줄");
}

TEST_F(FileIOTest, ShouldReadFileLineByLine)
{
    // Arrange - 여러 줄 파일 생성
    std::string filepath = (test_dir_ / "lines.txt").string();
    std::ofstream file(filepath);
    file << "첫 번째 줄\n";
    file << "두 번째 줄\n";
    file << "세 번째 줄";
    file.close();

    // Act - 줄별 읽기
    Value result = evaluate("줄별_읽기(\"" + filepath + "\")");

    // Assert
    EXPECT_TRUE(result.isArray());
    auto lines = result.asArray();
    EXPECT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[0].asString(), "첫 번째 줄");
    EXPECT_EQ(lines[1].asString(), "두 번째 줄");
    EXPECT_EQ(lines[2].asString(), "세 번째 줄");
}

TEST_F(FileIOTest, ShouldCheckFileExists)
{
    // Arrange
    std::string existing_file = (test_dir_ / "exists.txt").string();
    std::string non_existing_file = (test_dir_ / "not_exists.txt").string();

    // 파일 생성
    std::ofstream file(existing_file);
    file << "test";
    file.close();

    // Act & Assert - 존재하는 파일
    Value result1 = evaluate("파일_존재(\"" + existing_file + "\")");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    // Act & Assert - 존재하지 않는 파일
    Value result2 = evaluate("파일_존재(\"" + non_existing_file + "\")");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

TEST_F(FileIOTest, ShouldDeleteFile)
{
    // Arrange - 파일 생성
    std::string filepath = (test_dir_ / "to_delete.txt").string();
    std::ofstream file(filepath);
    file << "delete me";
    file.close();

    ASSERT_TRUE(std::filesystem::exists(filepath));

    // Act - 파일 삭제
    evaluate("파일_삭제(\"" + filepath + "\")");

    // Assert
    EXPECT_FALSE(std::filesystem::exists(filepath));
}

TEST_F(FileIOTest, ShouldThrowOnNonExistentFileRead)
{
    // Arrange
    std::string code = "파일_읽기(\"/nonexistent/file.txt\")";

    // Act & Assert
    EXPECT_THROW(evaluate(code), std::runtime_error);
}

// ============================================================================
// 디렉토리 관리 테스트
// ============================================================================

TEST_F(FileIOTest, ShouldCreateDirectory)
{
    // Arrange
    std::string dirpath = (test_dir_ / "new_dir").string();

    // Act
    evaluate("디렉토리_생성(\"" + dirpath + "\")");

    // Assert
    EXPECT_TRUE(std::filesystem::exists(dirpath));
    EXPECT_TRUE(std::filesystem::is_directory(dirpath));
}

TEST_F(FileIOTest, ShouldCreateNestedDirectories)
{
    // Arrange
    std::string dirpath = (test_dir_ / "a" / "b" / "c").string();

    // Act
    evaluate("디렉토리_생성(\"" + dirpath + "\")");

    // Assert
    EXPECT_TRUE(std::filesystem::exists(dirpath));
    EXPECT_TRUE(std::filesystem::is_directory(dirpath));
}

TEST_F(FileIOTest, ShouldDeleteDirectory)
{
    // Arrange - 디렉토리 생성 및 파일 추가
    std::string dirpath = (test_dir_ / "to_delete_dir").string();
    std::filesystem::create_directories(dirpath);

    std::string filepath = (test_dir_ / "to_delete_dir" / "file.txt").string();
    std::ofstream file(filepath);
    file << "test";
    file.close();

    ASSERT_TRUE(std::filesystem::exists(dirpath));

    // Act - 디렉토리 삭제 (하위 파일 포함)
    evaluate("디렉토리_삭제(\"" + dirpath + "\")");

    // Assert
    EXPECT_FALSE(std::filesystem::exists(dirpath));
}

TEST_F(FileIOTest, ShouldListDirectoryContents)
{
    // Arrange - 디렉토리 생성 및 파일 추가
    std::string dirpath = (test_dir_ / "list_test").string();
    std::filesystem::create_directories(dirpath);

    // 파일 생성
    std::ofstream file1((std::filesystem::path(dirpath) / "file1.txt").string());
    file1 << "test1";
    file1.close();

    std::ofstream file2((std::filesystem::path(dirpath) / "file2.txt").string());
    file2 << "test2";
    file2.close();

    // 하위 디렉토리 생성
    std::filesystem::create_directories(std::filesystem::path(dirpath) / "subdir");

    // Act - 디렉토리 목록 조회
    Value result = evaluate("디렉토리_목록(\"" + dirpath + "\")");

    // Assert
    EXPECT_TRUE(result.isArray());
    auto entries = result.asArray();
    EXPECT_EQ(entries.size(), 3u);  // file1.txt, file2.txt, subdir

    // 파일 이름들 확인 (순서는 보장되지 않음)
    std::vector<std::string> names;
    for (const auto& entry : entries) {
        EXPECT_TRUE(entry.isString());
        names.push_back(entry.asString());
    }

    // 모든 예상 항목이 있는지 확인
    EXPECT_NE(std::find(names.begin(), names.end(), "file1.txt"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "file2.txt"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "subdir"), names.end());
}

TEST_F(FileIOTest, ShouldGetCurrentDirectory)
{
    // Act
    Value result = evaluate("현재_디렉토리()");

    // Assert
    EXPECT_TRUE(result.isString());
    std::string current_dir = result.asString();
    EXPECT_FALSE(current_dir.empty());
    EXPECT_TRUE(std::filesystem::exists(current_dir));
}

// ============================================================================
// 에러 처리 테스트
// ============================================================================

TEST_F(FileIOTest, ShouldThrowOnDeleteNonExistentFile)
{
    // Arrange
    std::string code = "파일_삭제(\"/nonexistent/file.txt\")";

    // Act & Assert
    EXPECT_THROW(evaluate(code), std::runtime_error);
}

TEST_F(FileIOTest, ShouldThrowOnDeleteNonExistentDirectory)
{
    // Arrange
    std::string code = "디렉토리_삭제(\"/nonexistent/dir\")";

    // Act & Assert
    EXPECT_THROW(evaluate(code), std::runtime_error);
}

TEST_F(FileIOTest, ShouldThrowOnListNonExistentDirectory)
{
    // Arrange
    std::string code = "디렉토리_목록(\"/nonexistent/dir\")";

    // Act & Assert
    EXPECT_THROW(evaluate(code), std::runtime_error);
}

TEST_F(FileIOTest, ShouldThrowOnListNonDirectory)
{
    // Arrange - 파일 생성
    std::string filepath = (test_dir_ / "not_a_dir.txt").string();
    std::ofstream file(filepath);
    file << "test";
    file.close();

    // Act & Assert - 파일에 대해 디렉토리_목록 호출
    std::string code = "디렉토리_목록(\"" + filepath + "\")";
    EXPECT_THROW(evaluate(code), std::runtime_error);
}

// ============================================================================
// 통합 테스트
// ============================================================================

TEST_F(FileIOTest, ShouldWorkInComplexScenario)
{
    // Arrange & Act - 복잡한 파일 I/O 시나리오
    std::string dirpath = (test_dir_ / "complex").string();

    // Step 1: 디렉토리 생성
    evaluate("디렉토리_생성(\"" + dirpath + "\")");
    ASSERT_TRUE(std::filesystem::exists(dirpath));

    // Step 2: 파일 쓰기
    evaluate("파일_쓰기(\"" + dirpath + "/data.txt\", \"Line 1\\nLine 2\\nLine 3\")");
    ASSERT_TRUE(std::filesystem::exists(dirpath + "/data.txt"));

    // Step 3: 파일 읽기
    Value content = evaluate("파일_읽기(\"" + dirpath + "/data.txt\")");
    EXPECT_TRUE(content.isString());

    // Step 4: 줄별로 읽기
    Value lines = evaluate("줄별_읽기(\"" + dirpath + "/data.txt\")");
    EXPECT_TRUE(lines.isArray());
    EXPECT_EQ(lines.asArray().size(), 3u);

    // Step 5: 파일 존재 확인
    Value exists = evaluate("파일_존재(\"" + dirpath + "/data.txt\")");
    EXPECT_TRUE(exists.isBoolean());
    EXPECT_TRUE(exists.asBoolean());

    // Step 6: 디렉토리 목록
    Value dirList = evaluate("디렉토리_목록(\"" + dirpath + "\")");
    EXPECT_TRUE(dirList.isArray());
    EXPECT_EQ(dirList.asArray().size(), 1u);  // data.txt 파일 1개
}
