/**
 * @file ProcessIOTest.cpp
 * @brief 프로그램 바깥과의 경계: 명령줄_인자, 오류_출력, 파일_바이트_읽기/쓰기 테스트
 *
 * 명령줄 해석(스크립트 뒤의 인자, -- 뒤의 인자)은 CMakeLists.txt 의 cli-arguments 종단 테스트가 본다.
 */

#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"
#include "support/TestTempDir.h"

using namespace kingsejong;
using namespace kingsejong::evaluator;

class ProcessIOTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Builtin::registerAllBuiltins();
        test_dir_ = kingsejong::test::uniqueTestDir("kingsejong_process_io");
    }

    void TearDown() override
    {
        // 명령줄 인자는 프로세스 전역이므로 다른 테스트에 남기지 않는다
        Builtin::setCommandLineArguments({});
        std::filesystem::remove_all(test_dir_);
    }

    Value evaluate(const std::string& code)
    {
        lexer::Lexer lexer(code);
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();
        EXPECT_TRUE(parser.errors().empty()) << (parser.errors().empty() ? "" : parser.errors()[0]);
        Evaluator evaluator;
        return evaluator.evalProgram(program.get());
    }

    /// VM 으로 돌리고 전역 변수 '기록' 을 문자열로 돌려준다
    std::string runVm(const std::string& code)
    {
        lexer::Lexer lexer(code);
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();
        bytecode::Chunk chunk;
        bytecode::Compiler compiler;
        EXPECT_TRUE(compiler.compile(program.get(), &chunk));
        bytecode::VM vm;
        vm.run(&chunk);
        return vm.globals()->get("기록").toString();
    }

    std::string path(const std::string& name) const { return (test_dir_ / name).generic_string(); }

    std::filesystem::path test_dir_;
};

/// 실행하는 동안 표준 출력과 표준 오류를 따로 가로챈다
class CaptureStreams
{
public:
    CaptureStreams()
        : previousOut_(std::cout.rdbuf(out_.rdbuf())), previousErr_(std::cerr.rdbuf(err_.rdbuf()))
    {
    }
    ~CaptureStreams()
    {
        std::cout.rdbuf(previousOut_);
        std::cerr.rdbuf(previousErr_);
    }
    std::string out() const { return out_.str(); }
    std::string err() const { return err_.str(); }

private:
    std::ostringstream out_;
    std::ostringstream err_;
    std::streambuf* previousOut_;
    std::streambuf* previousErr_;
};

// ============================================================================
// 명령줄_인자
// ============================================================================

TEST_F(ProcessIOTest, ShouldReturnScriptPathAndArguments)
{
    Builtin::setCommandLineArguments({"합계.ksj", "1", "두 단어"});
    Value result = evaluate("명령줄_인자()");
    ASSERT_TRUE(result.isArray());
    EXPECT_EQ(result.toString(), "[합계.ksj, 1, 두 단어]");
    EXPECT_EQ(runVm("기록 = 명령줄_인자()"), "[합계.ksj, 1, 두 단어]");
}

TEST_F(ProcessIOTest, ShouldReturnEmptyArrayWithoutScript)
{
    Value result = evaluate("명령줄_인자()");
    ASSERT_TRUE(result.isArray());
    EXPECT_TRUE(result.asArray().empty());
}

TEST_F(ProcessIOTest, ShouldRejectArgumentsToArgumentFunction)
{
    EXPECT_THROW(evaluate("명령줄_인자(1)"), std::exception);
}

TEST_F(ProcessIOTest, ShouldParseNumericArguments)
{
    Builtin::setCommandLineArguments({"합계.ksj", "3", "4"});
    Value result = evaluate("인자 = 명령줄_인자()\n정수(인자[1]) + 정수(인자[2])");
    EXPECT_EQ(result.toString(), "7");
}

// ============================================================================
// 오류_출력
// ============================================================================

TEST_F(ProcessIOTest, ShouldWriteToStandardErrorOnly)
{
    CaptureStreams capture;
    evaluate("출력(\"정상\")\n오류_출력(\"문제:\", 42, 참, 없음)");
    EXPECT_EQ(capture.out(), "정상\n");
    EXPECT_EQ(capture.err(), "문제: 42 참 없음\n");
}

TEST_F(ProcessIOTest, ShouldWriteSameFormatInVm)
{
    CaptureStreams capture;
    runVm("오류_출력([1, 2], 1.5)\n기록 = 1");
    EXPECT_EQ(capture.err(), "[1, 2] 1.5\n");
    EXPECT_EQ(capture.out(), "");
}

// ============================================================================
// 파일_바이트_읽기 / 파일_바이트_쓰기
// ============================================================================

TEST_F(ProcessIOTest, ShouldRoundTripAllByteValues)
{
    std::string file = path("all.bin");
    evaluate("바이트들 = []\ni가 0부터 256 미만 {\n    바이트들 = 바이트들 + [i]\n}\n파일_바이트_쓰기(\"" + file + "\", 바이트들)");
    EXPECT_EQ(std::filesystem::file_size(file), 256u);
    Value result = evaluate("읽은 = 파일_바이트_읽기(\"" + file + "\")\n[길이(읽은), 읽은[0], 읽은[10], 읽은[13], 읽은[255]]");
    EXPECT_EQ(result.toString(), "[256, 0, 10, 13, 255]");
}

TEST_F(ProcessIOTest, ShouldReadUtf8TextAsBytes)
{
    std::string file = path("text.txt");
    std::ofstream(file, std::ios::binary) << "A가\r\n";
    Value result = evaluate("파일_바이트_읽기(\"" + file + "\")");
    // "가" 는 UTF-8 3바이트, 줄 끝 CR LF 도 그대로 읽는다
    EXPECT_EQ(result.toString(), "[65, 234, 176, 128, 13, 10]");
}

TEST_F(ProcessIOTest, ShouldReadEmptyFileAsEmptyArray)
{
    std::string file = path("empty.bin");
    std::ofstream(file, std::ios::binary).close();
    EXPECT_EQ(evaluate("파일_바이트_읽기(\"" + file + "\")").toString(), "[]");
}

TEST_F(ProcessIOTest, ShouldLeaveFileUntouchedWhenAByteIsInvalid)
{
    std::string file = path("keep.bin");
    evaluate("파일_바이트_쓰기(\"" + file + "\", [1, 2, 3])");
    for (const std::string bad : {"[1, 256]", "[-1]", "[1.0]", "[\"a\"]"})
    {
        EXPECT_THROW(evaluate("파일_바이트_쓰기(\"" + file + "\", " + bad + ")"), std::exception) << bad;
    }
    EXPECT_EQ(evaluate("파일_바이트_읽기(\"" + file + "\")").toString(), "[1, 2, 3]");
}

TEST_F(ProcessIOTest, ShouldReportMissingFileAndBadArguments)
{
    EXPECT_THROW(evaluate("파일_바이트_읽기(\"" + path("none.bin") + "\")"), std::exception);
    EXPECT_THROW(evaluate("파일_바이트_읽기(1)"), std::exception);
    EXPECT_THROW(evaluate("파일_바이트_쓰기(\"" + path("x.bin") + "\", \"abc\")"), std::exception);
}
