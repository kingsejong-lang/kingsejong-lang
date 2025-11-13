/**
 * @file JSONTest.cpp
 * @brief JSON 처리 내장 함수 테스트
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * JSON 파싱, 문자열화, 파일 읽기/쓰기 테스트
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

class JSONTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Builtin::registerAllBuiltins();

        // 테스트용 임시 디렉토리 생성
        test_dir_ = std::filesystem::temp_directory_path() / "kingsejong_json_test";
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
// JSON 파싱 테스트
// ============================================================================

TEST_F(JSONTest, ShouldParseSimpleObject)
{
    // Arrange
    std::string code = R"(JSON_파싱("{\"name\":\"Alice\",\"age\":30}"))";

    // Act
    Value result = evaluate(code);

    // Assert - JSON object는 [["key", value], ...] 형태
    EXPECT_TRUE(result.isArray());
    auto pairs = result.asArray();
    EXPECT_EQ(pairs.size(), 2u);

    // 각 pair 검증
    for (const auto& pair : pairs) {
        EXPECT_TRUE(pair.isArray());
        EXPECT_EQ(pair.asArray().size(), 2u);
        EXPECT_TRUE(pair.asArray()[0].isString());
    }
}

TEST_F(JSONTest, ShouldParseArray)
{
    // Arrange
    std::string code = R"(JSON_파싱("[1, 2, 3, 4, 5]"))";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isArray());
    auto arr = result.asArray();
    EXPECT_EQ(arr.size(), 5u);
    EXPECT_EQ(arr[0].asInteger(), 1);
    EXPECT_EQ(arr[4].asInteger(), 5);
}

TEST_F(JSONTest, ShouldParseNestedStructure)
{
    // Arrange
    std::string code = R"(JSON_파싱("{\"users\":[{\"name\":\"Alice\"},{\"name\":\"Bob\"}]}"))";

    // Act
    Value result = evaluate(code);

    // Assert - 중첩된 구조
    EXPECT_TRUE(result.isArray());
    auto pairs = result.asArray();
    EXPECT_EQ(pairs.size(), 1u);

    // users 키의 값은 배열이어야 함
    auto users_pair = pairs[0].asArray();
    EXPECT_EQ(users_pair[0].asString(), "users");
    EXPECT_TRUE(users_pair[1].isArray());
}

TEST_F(JSONTest, ShouldParseDifferentTypes)
{
    // Arrange
    std::string code = R"(JSON_파싱("{\"str\":\"hello\",\"num\":42,\"bool\":true,\"null\":null}"))";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isArray());
    auto pairs = result.asArray();
    EXPECT_EQ(pairs.size(), 4u);
}

TEST_F(JSONTest, ShouldThrowOnInvalidJSON)
{
    // Arrange
    std::string code = R"(JSON_파싱("{invalid json}"))";

    // Act & Assert
    EXPECT_THROW(evaluate(code), std::runtime_error);
}

// ============================================================================
// JSON 문자열화 테스트
// ============================================================================

TEST_F(JSONTest, ShouldStringifySimpleValue)
{
    // Arrange
    std::string code = R"(JSON_문자열화(42))";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "42");
}

TEST_F(JSONTest, ShouldStringifyArray)
{
    // Arrange
    std::string code = R"(JSON_문자열화([1, 2, 3]))";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "[1,2,3]");
}

TEST_F(JSONTest, ShouldStringifyObjectWithIndent)
{
    // Arrange
    std::string code = R"(
        obj = [["name", "Alice"], ["age", 30]]
        JSON_문자열화(obj, 2)
    )";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isString());
    std::string json_str = result.asString();
    EXPECT_TRUE(json_str.find("\"name\"") != std::string::npos);
    EXPECT_TRUE(json_str.find("\"age\"") != std::string::npos);
    EXPECT_TRUE(json_str.find("30") != std::string::npos);
}

TEST_F(JSONTest, ShouldStringifyWithoutIndent)
{
    // Arrange - indent -1이면 압축 형식
    std::string code = R"(
        arr = [1, 2, 3]
        JSON_문자열화(arr, -1)
    )";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "[1,2,3]");
}

// ============================================================================
// JSON 파일 읽기/쓰기 테스트
// ============================================================================

TEST_F(JSONTest, ShouldWriteAndReadJSONFile)
{
    // Arrange
    std::string filepath = (test_dir_ / "test.json").string();
    std::string write_code = R"(
        data = [["name", "Alice"], ["age", 30]]
        JSON_파일_쓰기(")" + filepath + R"(", data)
    )";

    // Act - 쓰기
    evaluate(write_code);

    // Assert - 파일 존재
    ASSERT_TRUE(std::filesystem::exists(filepath));

    // Act - 읽기
    std::string read_code = R"(JSON_파일_읽기(")" + filepath + R"("))";
    Value result = evaluate(read_code);

    // Assert
    EXPECT_TRUE(result.isArray());
    auto pairs = result.asArray();
    EXPECT_EQ(pairs.size(), 2u);
}

TEST_F(JSONTest, ShouldWriteJSONArrayToFile)
{
    // Arrange
    std::string filepath = (test_dir_ / "array.json").string();
    std::string code = R"(
        arr = [1, 2, 3, 4, 5]
        JSON_파일_쓰기(")" + filepath + R"(", arr)
    )";

    // Act
    evaluate(code);

    // Assert - 파일 내용 확인
    std::ifstream file(filepath);
    ASSERT_TRUE(file.is_open());
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    EXPECT_TRUE(content.find("[") != std::string::npos);
    EXPECT_TRUE(content.find("1") != std::string::npos);
}

TEST_F(JSONTest, ShouldThrowOnReadNonExistentFile)
{
    // Arrange
    std::string code = R"(JSON_파일_읽기("/nonexistent/file.json"))";

    // Act & Assert
    EXPECT_THROW(evaluate(code), std::runtime_error);
}

TEST_F(JSONTest, ShouldThrowOnReadInvalidJSONFile)
{
    // Arrange - 잘못된 JSON 파일 생성
    std::string filepath = (test_dir_ / "invalid.json").string();
    std::ofstream file(filepath);
    file << "{invalid json content}";
    file.close();

    std::string code = R"(JSON_파일_읽기(")" + filepath + R"("))";

    // Act & Assert
    EXPECT_THROW(evaluate(code), std::runtime_error);
}

// ============================================================================
// 통합 테스트
// ============================================================================

TEST_F(JSONTest, ShouldRoundTripSimpleData)
{
    // Arrange - 간단한 라운드트립 테스트
    std::string filepath = (test_dir_ / "roundtrip.json").string();

    // Act - 배열 생성 → JSON 파일 저장 → 읽기
    std::string code = "JSON_파일_쓰기(\"" + filepath + "\", [1, 2, 3, 4, 5])";
    evaluate(code);

    Value result = evaluate("JSON_파일_읽기(\"" + filepath + "\")");

    // Assert
    EXPECT_TRUE(result.isArray());
    auto arr = result.asArray();
    EXPECT_EQ(arr.size(), 5u);
    EXPECT_EQ(arr[0].asInteger(), 1);
    EXPECT_EQ(arr[4].asInteger(), 5);
}

TEST_F(JSONTest, ShouldRoundTripObjectData)
{
    // Arrange - object 라운드트립 테스트
    std::string filepath = (test_dir_ / "object.json").string();

    // Act - object 형태 배열 생성 → 저장 → 읽기
    evaluate("JSON_파일_쓰기(\"" + filepath + "\", [[\"x\", 10], [\"y\", 20]])");
    Value result = evaluate("JSON_파일_읽기(\"" + filepath + "\")");

    // Assert
    EXPECT_TRUE(result.isArray());
    auto pairs = result.asArray();
    EXPECT_EQ(pairs.size(), 2u);

    // 파일 내용도 확인
    std::ifstream file(filepath);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    EXPECT_TRUE(content.find("\"x\"") != std::string::npos);
    EXPECT_TRUE(content.find("\"y\"") != std::string::npos);
}
