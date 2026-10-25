/**
 * @file VMStackTraceTest.cpp
 * @brief 바이트코드 VM 이 잡히지 않은 실행 에러에 붙이는 호출 스택 테스트
 *
 * 컴파일러는 CALL 명령마다 호출식에 적힌 함수 이름과 소스 파일을 청크의 디버그 표에 남기고,
 * VM 은 에러를 받아 줄 곳이 없으면 프레임을 걷어 내기 전에 (함수 이름, 각 CALL 명령의 줄)을
 * 안쪽부터 붙여 lastError() 로 넘긴다. 같은 소스를 평가기로 돌린 호출 스택과 같아야 한다.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "bytecode/Chunk.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "error/Error.h"
#include "error/ErrorReporter.h"
#include "lexer/Lexer.h"
#include "module/ModuleLoader.h"
#include "parser/Parser.h"
#include "support/TestTempDir.h"
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

using namespace kingsejong;
using namespace kingsejong::bytecode;
using ::testing::HasSubstr;
using ::testing::Not;

namespace {

using Frames = std::vector<std::pair<std::string, int>>;

/// (함수 이름, 줄) 쌍으로 줄인 호출 스택
Frames framesOf(const error::KingSejongError& e)
{
    Frames out;
    for (const auto& frame : e.stackTrace())
    {
        out.emplace_back(frame.function, frame.location.line);
    }
    return out;
}

/// 소스를 컴파일해 VM 으로 돌린 결과. 프로그램 AST 는 VM 이 끝날 때까지 살아 있어야 한다
struct VmRun {
    std::unique_ptr<ast::Program> program;
    Chunk chunk;
    VM vm;
    VMResult result = VMResult::COMPILE_ERROR;

    VmRun(const std::string& source, const std::string& filename, module::ModuleLoader* loader = nullptr)
    {
        evaluator::Builtin::registerAllBuiltins();
        lexer::Lexer lexer(source, filename);
        parser::Parser parser(lexer);
        program = parser.parseProgram();
        EXPECT_TRUE(parser.errors().empty()) << (parser.errors().empty() ? "" : parser.errors()[0]);
        Compiler compiler;
        if (compiler.compile(program.get(), &chunk))
        {
            vm.setModuleLoader(loader);
            result = vm.run(&chunk);
        }
    }
};

/// 프로그램(파일 이름 test.ksj)을 VM 으로 돌려 잡히지 않은 에러를 돌려준다. 에러가 안 나면 테스트 실패
error::KingSejongError vmError(const std::string& source, module::ModuleLoader* loader = nullptr,
                               const std::string& filename = "test.ksj")
{
    VmRun run(source, filename, loader);
    EXPECT_EQ(run.result, VMResult::RUNTIME_ERROR);
    if (run.vm.lastError())
    {
        return *run.vm.lastError();
    }
    ADD_FAILURE() << "VM 이 에러를 남기지 않았습니다";
    return error::KingSejongError(error::ErrorType::RUNTIME_ERROR, "(없음)");
}

/// 같은 프로그램을 평가기로 돌렸을 때의 호출 스택 (두 실행기가 같은 스택을 내야 한다)
Frames evaluatorFrames(const std::string& source)
{
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(source, "test.ksj");
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty());
    evaluator::Evaluator eval;
    try
    {
        eval.evalProgram(program.get());
    }
    catch (const error::KingSejongError& e)
    {
        return framesOf(e);
    }
    ADD_FAILURE() << "평가기에서 에러가 나지 않았습니다";
    return {};
}

const char* const kNestedSource =
R"(함수 나누기(a, b) {
    반환 a / b
}
함수 계산(x) {
    반환 나누기(x, 0)
}
계산(10)
)";

} // namespace

TEST(VMStackTraceTest, ChunkShouldTrackSourceFilesAndCallSiteNames)
{
    Chunk chunk;
    EXPECT_EQ(chunk.getFile(0), "");
    EXPECT_EQ(chunk.getCallSiteName(0), "");

    chunk.setFile("a.ksj");
    chunk.writeOpCode(OpCode::LOAD_NULL, 1);
    chunk.writeOpCode(OpCode::LOAD_NULL, 1);
    chunk.setFile("a.ksj");   // 같은 파일은 다시 적지 않는다
    chunk.setFile("b.ksj");
    chunk.writeOpCode(OpCode::CALL, 2);
    chunk.write(0, 2);
    chunk.setCallSiteName(2, "f");

    EXPECT_EQ(chunk.getFile(0), "a.ksj");
    EXPECT_EQ(chunk.getFile(1), "a.ksj");
    EXPECT_EQ(chunk.getFile(2), "b.ksj");
    EXPECT_EQ(chunk.getFile(3), "b.ksj");
    EXPECT_EQ(chunk.getCallSiteName(2), "f");
    EXPECT_EQ(chunk.getCallSiteName(3), "");

    chunk.clear();
    EXPECT_EQ(chunk.getFile(0), "");
    EXPECT_EQ(chunk.getCallSiteName(2), "");
}

TEST(VMStackTraceTest, ShouldShowCalleeNameInDisassembly)
{
    lexer::Lexer lexer("함수 f(a) { 반환 a }\nf(1)\n");
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_TRUE(parser.errors().empty());
    Chunk chunk;
    Compiler compiler;
    ASSERT_TRUE(compiler.compile(program.get(), &chunk));
    EXPECT_THAT(chunk.disassemble("test"), HasSubstr("CALL             1 ('f')"));
}

TEST(VMStackTraceTest, ShouldRecordNestedFunctionCallsInnermostFirst)
{
    auto e = vmError(kNestedSource);
    EXPECT_THAT(e.what(), HasSubstr("0으로 나눌 수 없습니다"));
    EXPECT_EQ(framesOf(e), (Frames{{"나누기", 2}, {"계산", 5}, {"<최상위>", 7}}));
    EXPECT_EQ(framesOf(e), evaluatorFrames(kNestedSource));
    ASSERT_FALSE(e.stackTrace().empty());
    EXPECT_EQ(e.stackTrace()[0].location.filename, "test.ksj");
    EXPECT_EQ(e.stackTrace()[2].location.filename, "test.ksj");
}

TEST(VMStackTraceTest, ShouldRecordFramesForErrorsThrownByBuiltins)
{
    const char* source = R"(함수 변환(글) {
    반환 정수(글)
}
변환("abc")
)";
    auto e = vmError(source);
    EXPECT_THAT(e.what(), HasSubstr("문자열을 정수로 변환할 수 없습니다"));
    EXPECT_EQ(framesOf(e), (Frames{{"변환", 2}, {"<최상위>", 4}}));
    EXPECT_EQ(framesOf(e), evaluatorFrames(source));
}

TEST(VMStackTraceTest, ShouldRecordFramesForUncaughtThrowStatement)
{
    const char* source = R"(함수 실패() {
    던지다 "문제 발생"
}
함수 작업() {
    실패()
}
작업()
)";
    auto e = vmError(source);
    EXPECT_THAT(e.what(), HasSubstr("문제 발생"));
    EXPECT_EQ(framesOf(e), (Frames{{"실패", 2}, {"작업", 5}, {"<최상위>", 7}}));
    EXPECT_EQ(framesOf(e), evaluatorFrames(source));
}

TEST(VMStackTraceTest, ShouldNameFunctionValueByCalledIdentifier)
{
    const char* source = R"(f = 함수(x) {
    반환 x / 0
}
g = f
g(1)
)";
    auto e = vmError(source);
    EXPECT_EQ(framesOf(e), (Frames{{"g", 2}, {"<최상위>", 5}}));
    EXPECT_EQ(framesOf(e), evaluatorFrames(source));
}

TEST(VMStackTraceTest, ShouldNameMethodFramesWithClassName)
{
    const char* source = R"(클래스 계산기 {
    공개 함수 나누기(a, b) {
        반환 a / b
    }
}
계 = 계산기()
계.나누기(1, 0)
)";
    auto e = vmError(source);
    EXPECT_EQ(framesOf(e), (Frames{{"계산기.나누기", 3}, {"<최상위>", 7}}));
    EXPECT_EQ(framesOf(e), evaluatorFrames(source));
}

TEST(VMStackTraceTest, ShouldNameConstructorFrames)
{
    const char* source = R"(클래스 포인트 {
    비공개 정수 x
    생성자(값) {
        자신.x = 1 / 값
    }
}
p = 포인트(0)
)";
    auto e = vmError(source);
    EXPECT_EQ(framesOf(e), (Frames{{"포인트.생성자", 4}, {"<최상위>", 7}}));
    EXPECT_EQ(framesOf(e), evaluatorFrames(source));
}

TEST(VMStackTraceTest, ShouldHaveOnlyTopLevelFrameOutsideFunctions)
{
    auto e = vmError("x = 1\ny = x / 0\n");
    EXPECT_EQ(framesOf(e), (Frames{{"<최상위>", 2}}));
}

TEST(VMStackTraceTest, ShouldRecordCallSiteLineAfterInnerCallReturned)
{
    // 안쪽 호출이 정상으로 돌아온 뒤 같은 함수의 다음 문장에서 난 에러는 그 문장의 줄을 가리켜야 한다
    const char* source = R"(함수 괜찮음() {
    반환 1
}
함수 작업() {
    a = 괜찮음()
    반환 a / 0
}
작업()
)";
    auto e = vmError(source);
    EXPECT_EQ(framesOf(e), (Frames{{"작업", 6}, {"<최상위>", 8}}));
    EXPECT_EQ(framesOf(e), evaluatorFrames(source));
}

TEST(VMStackTraceTest, ShouldRecordCallLineWhenArgumentsSpanLines)
{
    // 인자가 여러 줄에 걸친 호출은 호출식이 시작하는 줄을 프레임 위치로 갖는다
    const char* source = R"(함수 나누기(a, b) {
    반환 a / b
}
나누기(
    1,
    0
)
)";
    auto e = vmError(source);
    EXPECT_EQ(framesOf(e), (Frames{{"나누기", 2}, {"<최상위>", 4}}));
}

TEST(VMStackTraceTest, ShouldLeaveNoErrorWhenCaughtOrRejected)
{
    // 시도 블록이 잡은 에러와 비동기 함수의 거부는 잡히지 않은 에러가 아니다
    VmRun caught(R"(함수 나누기(a, b) { 반환 a / b }
결과 = ""
시도 {
    나누기(1, 0)
} 오류 (e) {
    결과 = e
}
)", "test.ksj");
    EXPECT_EQ(caught.result, VMResult::OK);
    EXPECT_EQ(caught.vm.lastError(), nullptr);
    EXPECT_THAT(caught.vm.globals()->get("결과").toString(), HasSubstr("0으로 나눌 수 없습니다"));

    VmRun rejected(R"(실패 = 비동기 함수() { 던지다 "실패함" }
p = 실패()
)", "test.ksj");
    EXPECT_EQ(rejected.result, VMResult::OK);
    EXPECT_EQ(rejected.vm.lastError(), nullptr);
    ASSERT_TRUE(rejected.vm.globals()->get("p").isPromise());
    EXPECT_EQ(rejected.vm.globals()->get("p").asPromise()->state(), evaluator::Promise::State::REJECTED);
}

TEST(VMStackTraceTest, ShouldResetLastErrorOnNextRun)
{
    lexer::Lexer lexer("x = 1 / 0\n", "test.ksj");
    parser::Parser parser(lexer);
    auto failing = parser.parseProgram();
    Chunk failingChunk;
    Compiler failingCompiler;
    ASSERT_TRUE(failingCompiler.compile(failing.get(), &failingChunk));

    lexer::Lexer okLexer("x = 1\n", "test.ksj");
    parser::Parser okParser(okLexer);
    auto ok = okParser.parseProgram();
    Chunk okChunk;
    Compiler okCompiler;
    ASSERT_TRUE(okCompiler.compile(ok.get(), &okChunk));

    VM vm;
    ASSERT_EQ(vm.run(&failingChunk), VMResult::RUNTIME_ERROR);
    ASSERT_NE(vm.lastError(), nullptr);
    EXPECT_EQ(vm.run(&okChunk), VMResult::OK);
    EXPECT_EQ(vm.lastError(), nullptr);
}

TEST(VMStackTraceTest, ShouldReportStackTraceWithInnermostLocation)
{
    auto e = vmError(kNestedSource);

    error::ErrorReporter reporter;
    reporter.setColorEnabled(false);
    reporter.registerSource("test.ksj", kNestedSource);
    std::ostringstream out;
    reporter.report(e, out);

    EXPECT_THAT(out.str(), HasSubstr("오류 위치: test.ksj:2"));
    EXPECT_THAT(out.str(), HasSubstr("반환 a / b"));
    EXPECT_THAT(out.str(), HasSubstr("0으로 나눌 수 없습니다"));
    EXPECT_THAT(out.str(), HasSubstr("호출 스택 (가장 안쪽부터):\n  나누기 (test.ksj:2)\n  계산 (test.ksj:5)\n  <최상위> (test.ksj:7)\n"));
}

TEST(VMStackTraceTest, ShouldNotReportStackTraceForTopLevelError)
{
    auto e = vmError("x = 1 / 0\n");

    error::ErrorReporter reporter;
    reporter.setColorEnabled(false);
    std::ostringstream out;
    reporter.report(e, out);

    EXPECT_THAT(out.str(), HasSubstr("오류 위치: test.ksj:1"));
    EXPECT_THAT(out.str(), Not(HasSubstr("호출 스택")));
}

/// 모듈 파일을 임시 디렉터리에 만들고 그 디렉터리를 검색 경로로 쓰는 로더
class VMModuleStackTraceTest : public ::testing::Test {
protected:
    std::filesystem::path dir_;
    std::unique_ptr<module::ModuleLoader> loader_;

    void SetUp() override {
        dir_ = kingsejong::test::uniqueTestDir("kingsejong_vm_stack");
        loader_ = std::make_unique<module::ModuleLoader>(dir_.generic_string());
    }
    void TearDown() override {
        loader_.reset();
        std::filesystem::remove_all(dir_);
    }
    void writeModule(const std::string& name, const std::string& source) {
        std::ofstream file(dir_ / (name + ".ksj"));
        file << source;
    }
    static bool endsWith(const std::string& s, const std::string& suffix) {
        return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
};

TEST_F(VMModuleStackTraceTest, ShouldRecordModuleFramesWhenImportedModuleFails)
{
    // main 이 a 를, a 가 b 를 가져오고 b 의 최상위에서 에러가 난다: 모듈마다 자기 파일 위치의 <최상위> 프레임
    writeModule("a", "가져오기 \"b\"\n정수 valueA = 1\n");
    writeModule("b", "정수 x = 0\n정수 y = 1 / x\n");

    auto e = vmError("가져오기 \"a\"\n정수 z = 3\n", loader_.get(), "main.ksj");
    const auto& frames = e.stackTrace();
    ASSERT_EQ(frames.size(), 3u);
    EXPECT_EQ(frames[0].function, "<최상위>");
    EXPECT_TRUE(endsWith(frames[0].location.filename, "b.ksj")) << frames[0].location.filename;
    EXPECT_EQ(frames[0].location.line, 2);
    EXPECT_EQ(frames[1].function, "<최상위>");
    EXPECT_TRUE(endsWith(frames[1].location.filename, "a.ksj")) << frames[1].location.filename;
    EXPECT_EQ(frames[1].location.line, 1);
    EXPECT_EQ(frames[2].function, "<최상위>");
    EXPECT_EQ(frames[2].location.filename, "main.ksj");
    EXPECT_EQ(frames[2].location.line, 1);
}

TEST_F(VMModuleStackTraceTest, ShouldLocateModuleFunctionFramesInModuleFile)
{
    // 가져온 모듈의 함수는 같은 청크에 컴파일되어 바이트코드로 돈다. 그 안에서 난 에러의 프레임은 모듈 파일을 가리킨다
    writeModule("도구", "함수 실패() {\n    던지다 \"모듈 안 오류\"\n}\n");

    auto e = vmError("가져오기 \"도구\"\n\n실패()\n", loader_.get(), "main.ksj");
    EXPECT_THAT(e.what(), HasSubstr("모듈 안 오류"));
    const auto& frames = e.stackTrace();
    ASSERT_EQ(frames.size(), 2u);
    EXPECT_EQ(frames[0].function, "실패");
    EXPECT_TRUE(endsWith(frames[0].location.filename, "도구.ksj")) << frames[0].location.filename;
    EXPECT_EQ(frames[0].location.line, 2);
    EXPECT_EQ(frames[1].function, "<최상위>");
    EXPECT_EQ(frames[1].location.filename, "main.ksj");
    EXPECT_EQ(frames[1].location.line, 3);
}

// ============================================================================
// 열(column): 명령마다 그 명령을 낸 식의 시작 열을 적어 에러 위치에 캐럿을 찍을 수 있게 한다
// ============================================================================

TEST(VMStackTraceTest, ChunkShouldTrackColumnsNextToLines)
{
    Chunk chunk;
    chunk.writeOpCode(OpCode::LOAD_NULL, 1, 5);
    chunk.writeOpCode(OpCode::LOAD_NULL, 1, 9);
    chunk.writeOpCode(OpCode::LOAD_NULL, 2);        // 열을 모르면 0
    EXPECT_EQ(chunk.getLine(0), 1);
    EXPECT_EQ(chunk.getColumn(0), 5);
    EXPECT_EQ(chunk.getColumn(1), 9);
    EXPECT_EQ(chunk.getLine(2), 2);
    EXPECT_EQ(chunk.getColumn(2), 0);
}

TEST(VMStackTraceTest, ShouldRecordColumnOfFailingExpressionAndCallSites)
{
    // 나누기 명령은 피연산자가 아니라 "a / b" 식의 시작 열(a)을, CALL 은 호출식의 시작 열을 갖는다.
    // 열은 렉서가 세는 단위(바이트)와 같다: "    반환 a / b" 에서 a 는 12번째 바이트다.
    auto e = vmError(kNestedSource);
    const auto& frames = e.stackTrace();
    ASSERT_EQ(frames.size(), 3u);
    EXPECT_EQ(frames[0].location.line, 2);
    EXPECT_EQ(frames[0].location.column, 12);   // a / b
    EXPECT_EQ(frames[1].location.line, 5);
    EXPECT_EQ(frames[1].location.column, 12);   // 나누기(x, 0)
    EXPECT_EQ(frames[2].location.line, 7);
    EXPECT_EQ(frames[2].location.column, 1);    // 계산(10)
}

TEST(VMStackTraceTest, ShouldPointAtExpressionStartNotLastOperand)
{
    auto e = vmError("x = 1 / 0\n");
    ASSERT_FALSE(e.stackTrace().empty());
    EXPECT_EQ(e.stackTrace().front().location.column, 5);   // "1 / 0" 의 1 (마지막 피연산자 0 은 9열)

    auto idx = vmError("값 = [1, 2]\n출력(값[5])\n");
    ASSERT_FALSE(idx.stackTrace().empty());
    EXPECT_EQ(idx.stackTrace().front().location.line, 2);
    EXPECT_EQ(idx.stackTrace().front().location.column, 8);   // "값[5]" 의 값
}

TEST(VMStackTraceTest, ShouldPrintColumnAndCaretInReport)
{
    auto e = vmError(kNestedSource);

    error::ErrorReporter reporter;
    reporter.setColorEnabled(false);
    reporter.registerSource("test.ksj", kNestedSource);
    std::ostringstream out;
    reporter.report(e, out);

    // 헤더의 열은 글자 단위: "    반환 a / b" 의 a 는 8번째 글자다 (바이트로는 12)
    EXPECT_THAT(out.str(), HasSubstr("오류 위치: test.ksj:2:8"));
    EXPECT_THAT(out.str(), HasSubstr("^"));
}
