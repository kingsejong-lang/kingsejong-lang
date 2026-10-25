/**
 * @file ErrorReporterTest.cpp
 * @brief ErrorReporter 시스템 테스트 (TDD)
 * @author KingSejong Team
 * @date 2025-11-11
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../src/error/ErrorReporter.h"
#include "../../src/error/Error.h"
#include "../../src/error/ErrorMessages.h"
#include <sstream>

using namespace kingsejong::error;
using ::testing::HasSubstr;
using ::testing::Not;

// ============================================================================
// SourceManager 테스트
// ============================================================================

class SourceManagerTest : public ::testing::Test {
protected:
    std::unique_ptr<SourceManager> sourceMgr;

    void SetUp() override {
        sourceMgr = std::make_unique<SourceManager>();
    }
};

TEST_F(SourceManagerTest, ShouldLoadSource) {
    std::string code = "x = 10\ny = 20\n출력(x)";
    sourceMgr->loadSource("test.ksj", code);

    auto line1 = sourceMgr->getLine("test.ksj", 1);
    ASSERT_TRUE(line1.has_value());
    EXPECT_EQ(*line1, "x = 10");

    auto line2 = sourceMgr->getLine("test.ksj", 2);
    ASSERT_TRUE(line2.has_value());
    EXPECT_EQ(*line2, "y = 20");

    auto line3 = sourceMgr->getLine("test.ksj", 3);
    ASSERT_TRUE(line3.has_value());
    EXPECT_EQ(*line3, "출력(x)");
}

TEST_F(SourceManagerTest, ShouldReturnNulloptForInvalidLine) {
    sourceMgr->loadSource("test.ksj", "x = 10\ny = 20");

    auto line = sourceMgr->getLine("test.ksj", 5);
    EXPECT_FALSE(line.has_value());
}

TEST_F(SourceManagerTest, ShouldReturnNulloptForUnknownFile) {
    auto line = sourceMgr->getLine("unknown.ksj", 1);
    EXPECT_FALSE(line.has_value());
}

TEST_F(SourceManagerTest, ShouldGetContext) {
    std::string code = "line1\nline2\nline3\nline4\nline5";
    sourceMgr->loadSource("test.ksj", code);

    auto context = sourceMgr->getContext("test.ksj", 3, 1);
    ASSERT_EQ(context.size(), 3u);
    EXPECT_EQ(context[0], "line2");
    EXPECT_EQ(context[1], "line3");
    EXPECT_EQ(context[2], "line4");
}

TEST_F(SourceManagerTest, ShouldHandleEdgeCasesInContext) {
    std::string code = "line1\nline2\nline3";
    sourceMgr->loadSource("test.ksj", code);

    // 파일 시작 부근 (line 1, context 2 → line -1~3, but -1과 0은 없음 → line 1,2,3)
    auto context1 = sourceMgr->getContext("test.ksj", 1, 2);
    EXPECT_EQ(context1.size(), 3u);  // line1, line2, line3 (파일 시작이라 더 앞은 없음)

    // 파일 끝 부근 (line 3, context 2 → line 1~5, but 4와 5는 없음 → line 1,2,3)
    auto context2 = sourceMgr->getContext("test.ksj", 3, 2);
    EXPECT_EQ(context2.size(), 3u);  // line1, line2, line3 (파일 끝이라 더 뒤는 없음)
}

// ============================================================================
// ErrorFormatter 테스트
// ============================================================================

class ErrorFormatterTest : public ::testing::Test {
protected:
    std::unique_ptr<SourceManager> sourceMgr;
    std::unique_ptr<ErrorFormatter> formatter;

    void SetUp() override {
        sourceMgr = std::make_unique<SourceManager>();
        formatter = std::make_unique<ErrorFormatter>();
    }
};

TEST_F(ErrorFormatterTest, ShouldFormatBasicErrorWithoutLocation) {
    auto error = ParserError("변수 이름이 잘못되었습니다");

    std::string formatted = formatter->formatError(error, *sourceMgr, false);

    EXPECT_THAT(formatted, HasSubstr("구문 오류"));
    EXPECT_THAT(formatted, HasSubstr("변수 이름이 잘못되었습니다"));
}

TEST_F(ErrorFormatterTest, ShouldFormatErrorWithLocation) {
    sourceMgr->loadSource("test.ksj", "x = 10\n123변수 = 20\n출력(x)");

    SourceLocation loc("test.ksj", 2, 1);
    auto error = ParserError("변수 이름은 숫자로 시작할 수 없습니다", loc);

    std::string formatted = formatter->formatError(error, *sourceMgr, false);

    EXPECT_THAT(formatted, HasSubstr("test.ksj:2:1"));
    EXPECT_THAT(formatted, HasSubstr("123변수"));
    EXPECT_THAT(formatted, HasSubstr("구문 오류"));
    EXPECT_THAT(formatted, HasSubstr("변수 이름은 숫자로 시작할 수 없습니다"));
}

TEST_F(ErrorFormatterTest, ShouldShowSourceContext) {
    sourceMgr->loadSource("test.ksj", "x = 10\ny = z + 5\n출력(y)");

    SourceLocation loc("test.ksj", 2, 5);
    auto error = NameError("정의되지 않은 변수 'z'", loc);

    std::string formatted = formatter->formatError(error, *sourceMgr, false);

    // 에러 라인과 주변 컨텍스트 확인
    EXPECT_THAT(formatted, HasSubstr("x = 10"));
    EXPECT_THAT(formatted, HasSubstr("y = z + 5"));
    EXPECT_THAT(formatted, HasSubstr("출력(y)"));
}

TEST_F(ErrorFormatterTest, ShouldHighlightErrorPosition) {
    sourceMgr->loadSource("test.ksj", "x = 10\n123변수 = 20\n출력(x)");

    SourceLocation loc("test.ksj", 2, 1);
    auto error = ParserError("변수 이름은 숫자로 시작할 수 없습니다", loc);

    std::string formatted = formatter->formatError(error, *sourceMgr, false);

    // 화살표(^) 또는 하이라이트 확인
    EXPECT_THAT(formatted, HasSubstr("^"));
}

TEST_F(ErrorFormatterTest, ShouldAlignCaretAfterKoreanText) {
    // 렉서의 열은 바이트 단위다. "출력(" 은 7바이트지만 터미널에서는 5칸을 차지하므로
    // 8번째 바이트에서 시작하는 "값들" 아래에 캐럿이 오려면 공백 5칸이 필요하다.
    sourceMgr->loadSource("test.ksj", "출력(값들[5])");

    SourceLocation loc("test.ksj", 1, 8);
    auto error = RuntimeError("인덱스 범위 초과", loc);

    std::string formatted = formatter->formatError(error, *sourceMgr, false);

    EXPECT_THAT(formatted, HasSubstr("1 | 출력(값들[5])\n    " + std::string(5, ' ') + "^\n"));
}

TEST_F(ErrorFormatterTest, ShouldKeepTabsWhenAligningCaret) {
    // 탭은 터미널 설정에 따라 폭이 달라지므로 공백으로 바꾸지 않고 그대로 둔다.
    sourceMgr->loadSource("test.ksj", "\tx = 값 / 0");

    SourceLocation loc("test.ksj", 1, 6);
    auto error = RuntimeError("0으로 나눌 수 없습니다", loc);

    std::string formatted = formatter->formatError(error, *sourceMgr, false);

    EXPECT_THAT(formatted, HasSubstr("1 | \tx = 값 / 0\n    \t    ^\n"));
}

TEST_F(ErrorFormatterTest, ShouldFormatWithLineNumbers) {
    sourceMgr->loadSource("test.ksj", "x = 10\ny = 20\nz = 30");

    SourceLocation loc("test.ksj", 2, 1);
    auto error = RuntimeError("테스트 에러", loc);

    std::string formatted = formatter->formatError(error, *sourceMgr, false);

    // 줄 번호 표시 확인
    EXPECT_THAT(formatted, HasSubstr("1 |"));
    EXPECT_THAT(formatted, HasSubstr("2 |"));
    EXPECT_THAT(formatted, HasSubstr("3 |"));
}

TEST_F(ErrorFormatterTest, ShouldNotUseColorWhenDisabled) {
    auto error = RuntimeError("테스트");

    std::string formatted = formatter->formatError(error, *sourceMgr, false);

    // ANSI 이스케이프 시퀀스가 없어야 함
    EXPECT_THAT(formatted, Not(HasSubstr("\033[")));
}

// ============================================================================
// ErrorReporter 통합 테스트
// ============================================================================

class ErrorReporterTest : public ::testing::Test {
protected:
    std::unique_ptr<ErrorReporter> reporter;

    void SetUp() override {
        reporter = std::make_unique<ErrorReporter>();
        reporter->setColorEnabled(false);  // 테스트에서는 컬러 비활성화
    }
};

TEST_F(ErrorReporterTest, ShouldReportBasicError) {
    auto error = RuntimeError("테스트 에러 메시지");

    std::ostringstream oss;
    reporter->report(error, oss);

    std::string output = oss.str();
    EXPECT_THAT(output, HasSubstr("실행 오류"));
    EXPECT_THAT(output, HasSubstr("테스트 에러 메시지"));
}

TEST_F(ErrorReporterTest, ShouldReportErrorWithSourceContext) {
    reporter->registerSource("example.ksj", "x = 10\n123변수 = 20\n출력(x)");

    SourceLocation loc("example.ksj", 2, 1);
    auto error = ParserError("변수 이름은 숫자로 시작할 수 없습니다", loc);

    std::ostringstream oss;
    reporter->report(error, oss);

    std::string output = oss.str();
    EXPECT_THAT(output, HasSubstr("example.ksj:2:1"));
    EXPECT_THAT(output, HasSubstr("123변수"));
    EXPECT_THAT(output, HasSubstr("구문 오류"));
}

TEST_F(ErrorReporterTest, ShouldShowHintWhenRegistered) {
    reporter->registerHint(
        ErrorType::PARSER_ERROR,
        "변수 이름은 숫자로 시작할 수 없습니다",
        "변수 이름은 한글, 영문, 밑줄(_)로 시작해야 합니다.\n예시: 변수123, _temp, 숫자"
    );

    reporter->registerSource("test.ksj", "123변수 = 10");

    SourceLocation loc("test.ksj", 1, 1);
    auto error = ParserError("변수 이름은 숫자로 시작할 수 없습니다", loc);

    std::ostringstream oss;
    reporter->report(error, oss);

    std::string output = oss.str();
    EXPECT_THAT(output, HasSubstr("도움말"));
    EXPECT_THAT(output, HasSubstr("변수 이름은 한글, 영문, 밑줄(_)로 시작해야 합니다"));
    EXPECT_THAT(output, HasSubstr("예시: 변수123"));
}

TEST_F(ErrorReporterTest, ShouldHandleMultipleSources) {
    reporter->registerSource("file1.ksj", "x = 10");
    reporter->registerSource("file2.ksj", "y = 20");

    SourceLocation loc1("file1.ksj", 1, 1);
    auto error1 = RuntimeError("file1 에러", loc1);

    std::ostringstream oss1;
    reporter->report(error1, oss1);
    EXPECT_THAT(oss1.str(), HasSubstr("file1.ksj"));

    SourceLocation loc2("file2.ksj", 1, 1);
    auto error2 = RuntimeError("file2 에러", loc2);

    std::ostringstream oss2;
    reporter->report(error2, oss2);
    EXPECT_THAT(oss2.str(), HasSubstr("file2.ksj"));
}

TEST_F(ErrorReporterTest, ShouldHandleErrorWithoutRegisteredSource) {
    // 소스가 등록되지 않은 경우에도 에러 메시지 출력 가능해야 함
    SourceLocation loc("unknown.ksj", 1, 1);
    auto error = RuntimeError("알 수 없는 파일 에러", loc);

    std::ostringstream oss;
    EXPECT_NO_THROW(reporter->report(error, oss));

    std::string output = oss.str();
    EXPECT_THAT(output, HasSubstr("알 수 없는 파일 에러"));
}

// ============================================================================
// ANSI 컬러 테스트
// ============================================================================

TEST(AnsiColorTest, ShouldProduceColorCodes) {
    std::string red = ansiColor(AnsiColor::RED);
    EXPECT_FALSE(red.empty());
    EXPECT_THAT(red, HasSubstr("\033["));

    std::string reset = ansiColor(AnsiColor::RESET);
    EXPECT_FALSE(reset.empty());
}

TEST(AnsiColorTest, ShouldColorizeText) {
    std::string text = "에러 메시지";
    std::string colored = colorize(text, AnsiColor::RED);

    EXPECT_THAT(colored, HasSubstr(text));
    EXPECT_THAT(colored, HasSubstr("\033["));  // ANSI 코드 포함
}

// ============================================================================
// 에러 타입별 메시지 테스트
// ============================================================================

TEST(ErrorMessageTest, ShouldHaveKoreanErrorTypeNames) {
    EXPECT_EQ(errorTypeToKorean(ErrorType::LEXER_ERROR), "어휘 오류");
    EXPECT_EQ(errorTypeToKorean(ErrorType::PARSER_ERROR), "구문 오류");
    EXPECT_EQ(errorTypeToKorean(ErrorType::RUNTIME_ERROR), "실행 오류");
    EXPECT_EQ(errorTypeToKorean(ErrorType::TYPE_ERROR), "타입 오류");
    EXPECT_EQ(errorTypeToKorean(ErrorType::NAME_ERROR), "이름 오류");
    EXPECT_EQ(errorTypeToKorean(ErrorType::VALUE_ERROR), "값 오류");
    EXPECT_EQ(errorTypeToKorean(ErrorType::ZERO_DIVISION_ERROR), "0으로 나누기 오류");
    EXPECT_EQ(errorTypeToKorean(ErrorType::INDEX_ERROR), "인덱스 오류");
    EXPECT_EQ(errorTypeToKorean(ErrorType::ARGUMENT_ERROR), "인자 오류");
}

TEST(ErrorMessageTest, ShouldHaveVMErrorMessages) {
    // VM 에러 메시지가 정의되어 있는지 확인
    using namespace kingsejong::error;
    EXPECT_FALSE(vm::STACK_OVERFLOW.empty());
    EXPECT_FALSE(vm::DIVIDE_BY_ZERO.empty());
    EXPECT_FALSE(vm::INDEX_OUT_OF_BOUNDS.empty());
    EXPECT_FALSE(vm::UNDEFINED_VARIABLE.empty());
    EXPECT_FALSE(vm::OPERAND_MUST_BE_NUMBER.empty());
    EXPECT_FALSE(vm::NOT_AN_ARRAY.empty());
    EXPECT_FALSE(vm::CALL_NON_FUNCTION.empty());
    EXPECT_FALSE(vm::UNKNOWN_OPCODE.empty());

    // 한글 메시지인지 확인
    EXPECT_NE(vm::STACK_OVERFLOW.find("스택"), std::string_view::npos);
    EXPECT_NE(vm::DIVIDE_BY_ZERO.find("나눌"), std::string_view::npos);
}

TEST(ErrorMessageTest, ShouldHaveChunkErrorMessages) {
    // Chunk 에러 메시지가 정의되어 있는지 확인
    using namespace kingsejong::error;
    EXPECT_FALSE(chunk::READ_OFFSET_OUT_OF_BOUNDS.empty());
    EXPECT_FALSE(chunk::CONSTANT_INDEX_OUT_OF_BOUNDS.empty());
    EXPECT_FALSE(chunk::NAME_INDEX_OUT_OF_BOUNDS.empty());
    EXPECT_FALSE(chunk::PATCH_OFFSET_OUT_OF_BOUNDS.empty());
}

TEST(ErrorMessageTest, ShouldHaveDebuggerErrorMessages) {
    // Debugger 에러 메시지가 정의되어 있는지 확인
    using namespace kingsejong::error;
    EXPECT_FALSE(debugger::CALLSTACK_POP_EMPTY.empty());
    EXPECT_FALSE(debugger::CALLSTACK_CURRENT_EMPTY.empty());
}

TEST(ErrorMessageTest, ShouldHaveClassErrorMessages) {
    // 클래스 관련 에러 메시지 확인
    using namespace kingsejong::error;
    EXPECT_FALSE(vm::UNDEFINED_CLASS.empty());
    EXPECT_FALSE(vm::CONSTRUCTOR_ARG_MISMATCH.empty());
    EXPECT_FALSE(vm::FIELD_ACCESS_NOT_INSTANCE.empty());
    EXPECT_FALSE(vm::UNDEFINED_METHOD.empty());
    EXPECT_FALSE(vm::THIS_OUTSIDE_CLASS.empty());
}

TEST(ErrorMessageTest, ShouldHaveAsyncErrorMessages) {
    // 비동기 관련 에러 메시지 확인
    using namespace kingsejong::error;
    EXPECT_FALSE(vm::CALL_NON_ASYNC.empty());
    EXPECT_FALSE(vm::PROMISE_REJECTED.empty());

    // 한글 메시지인지 확인
    EXPECT_NE(vm::CALL_NON_ASYNC.find("비동기"), std::string_view::npos);
    EXPECT_NE(vm::PROMISE_REJECTED.find("Promise"), std::string_view::npos);
}

TEST(ErrorMessageTest, ShouldHaveLimitErrorMessages) {
    // 제한 관련 에러 메시지 확인
    using namespace kingsejong::error;
    EXPECT_FALSE(vm::MAX_INSTRUCTION_LIMIT.empty());
    EXPECT_FALSE(vm::MAX_EXECUTION_TIME.empty());
    EXPECT_FALSE(vm::STACK_SIZE_EXCEEDED.empty());

    // 포맷 문자열 포함 확인 ({})
    EXPECT_NE(vm::MAX_INSTRUCTION_LIMIT.find("{}"), std::string_view::npos);
    EXPECT_NE(vm::MAX_EXECUTION_TIME.find("{}"), std::string_view::npos);
}

// ============================================================================
// 복잡한 에러 시나리오 테스트
// ============================================================================

TEST_F(ErrorReporterTest, ShouldFormatComplexParserError) {
    reporter->registerSource("complex.ksj",
        "x = 10\n"
        "123변수 = 20\n"
        "출력(x)\n"
    );

    reporter->registerHint(
        ErrorType::PARSER_ERROR,
        "변수 이름은 숫자로 시작할 수 없습니다",
        "변수 이름은 한글, 영문, 밑줄(_)로 시작해야 합니다.\n"
        "예시: 변수123, _temp, 숫자"
    );

    SourceLocation loc("complex.ksj", 2, 1);
    auto error = ParserError("변수 이름은 숫자로 시작할 수 없습니다", loc);

    std::ostringstream oss;
    reporter->report(error, oss);

    std::string output = oss.str();

    // 모든 요소가 포함되어 있는지 확인
    EXPECT_THAT(output, HasSubstr("complex.ksj:2:1"));  // 위치
    EXPECT_THAT(output, HasSubstr("1 | x = 10"));       // 이전 줄
    EXPECT_THAT(output, HasSubstr("2 | 123변수"));      // 에러 줄
    EXPECT_THAT(output, HasSubstr("3 | 출력(x)"));      // 다음 줄
    EXPECT_THAT(output, HasSubstr("^"));                 // 하이라이트
    EXPECT_THAT(output, HasSubstr("구문 오류"));         // 에러 타입
    EXPECT_THAT(output, HasSubstr("도움말"));            // 힌트
    EXPECT_THAT(output, HasSubstr("예시: 변수123"));     // 힌트 내용
}

TEST_F(ErrorReporterTest, ShouldFormatComplexNameError) {
    reporter->registerSource("name_error.ksj",
        "y = 10\n"
        "x = z + 5\n"
        "출력(x)\n"
    );

    reporter->registerHint(
        ErrorType::NAME_ERROR,
        "정의되지 않은 변수",
        "변수를 먼저 선언하세요.\n"
        "예시: 정수 {name} = 0"
    );

    SourceLocation loc("name_error.ksj", 2, 5);
    auto error = NameError("정의되지 않은 변수 'z'", loc);

    std::ostringstream oss;
    reporter->report(error, oss);

    std::string output = oss.str();

    EXPECT_THAT(output, HasSubstr("name_error.ksj:2:5"));
    EXPECT_THAT(output, HasSubstr("y = 10"));
    EXPECT_THAT(output, HasSubstr("x = z + 5"));
    EXPECT_THAT(output, HasSubstr("이름 오류"));
    EXPECT_THAT(output, HasSubstr("정의되지 않은 변수 'z'"));
}

// ============================================================================
// 메모리 안전성 테스트
// ============================================================================

TEST(MemorySafetyTest, ShouldNotLeakOnRepeatedReports) {
    // AddressSanitizer로 검증
    auto reporter = std::make_unique<ErrorReporter>();
    reporter->setColorEnabled(false);

    for (int i = 0; i < 1000; i++) {
        reporter->registerSource("test" + std::to_string(i) + ".ksj", "x = 10");

        SourceLocation loc("test" + std::to_string(i) + ".ksj", 1, 1);
        auto error = RuntimeError("테스트 에러 " + std::to_string(i), loc);

        std::ostringstream oss;
        reporter->report(error, oss);
    }

    // RAII로 자동 정리되어야 함
}

TEST(MemorySafetyTest, ShouldHandleExceptionsDuringFormatting) {
    auto reporter = std::make_unique<ErrorReporter>();

    // 잘못된 위치 정보
    SourceLocation invalidLoc("nonexistent.ksj", -1, -1);
    auto error = RuntimeError("테스트", invalidLoc);

    std::ostringstream oss;
    EXPECT_NO_THROW(reporter->report(error, oss));
}

TEST(MemorySafetyTest, ShouldHandleLargeErrorMessages) {
    auto reporter = std::make_unique<ErrorReporter>();
    reporter->setColorEnabled(false);

    // 매우 긴 에러 메시지 생성
    std::string longMessage(10000, 'x');  // 10000자 문자열

    SourceLocation loc("test.ksj", 1, 1);
    auto error = RuntimeError(longMessage, loc);

    std::ostringstream oss;
    EXPECT_NO_THROW(reporter->report(error, oss));
    EXPECT_FALSE(oss.str().empty());
}

TEST(MemorySafetyTest, ShouldHandleEmptyStrings) {
    auto reporter = std::make_unique<ErrorReporter>();
    reporter->setColorEnabled(false);

    // 빈 소스 코드
    reporter->registerSource("empty.ksj", "");

    SourceLocation loc("empty.ksj", 1, 1);
    auto error = RuntimeError("", loc);  // 빈 에러 메시지

    std::ostringstream oss;
    EXPECT_NO_THROW(reporter->report(error, oss));
}

TEST(MemorySafetyTest, ShouldHandleZeroLineColumn) {
    auto reporter = std::make_unique<ErrorReporter>();
    reporter->setColorEnabled(false);

    reporter->registerSource("test.ksj", "x = 10");

    // 0번째 줄/열 (경계값 테스트)
    SourceLocation loc("test.ksj", 0, 0);
    auto error = RuntimeError("경계값 테스트", loc);

    std::ostringstream oss;
    EXPECT_NO_THROW(reporter->report(error, oss));
}

TEST(MemorySafetyTest, ShouldHandleVeryLongSourceLine) {
    auto reporter = std::make_unique<ErrorReporter>();
    reporter->setColorEnabled(false);

    // 매우 긴 소스 라인
    std::string longLine(5000, 'x');
    reporter->registerSource("long.ksj", longLine);

    SourceLocation loc("long.ksj", 1, 2500);  // 긴 라인 중간
    auto error = RuntimeError("긴 라인 테스트", loc);

    std::ostringstream oss;
    EXPECT_NO_THROW(reporter->report(error, oss));
}

TEST(MemorySafetyTest, ShouldHandleMultipleSourceFiles) {
    auto reporter = std::make_unique<ErrorReporter>();
    reporter->setColorEnabled(false);

    // 여러 소스 파일 등록
    for (int i = 0; i < 100; i++) {
        reporter->registerSource("file" + std::to_string(i) + ".ksj",
                                 "변수" + std::to_string(i) + " = " + std::to_string(i));
    }

    // 다양한 파일에서 에러 발생
    for (int i = 0; i < 100; i++) {
        SourceLocation loc("file" + std::to_string(i) + ".ksj", 1, 1);
        auto error = RuntimeError("에러 " + std::to_string(i), loc);

        std::ostringstream oss;
        EXPECT_NO_THROW(reporter->report(error, oss));
    }
}

TEST(MemorySafetyTest, ShouldHandleRepeatedHintRegistration) {
    auto reporter = std::make_unique<ErrorReporter>();
    reporter->setColorEnabled(false);

    // 동일한 힌트를 여러 번 등록
    for (int i = 0; i < 100; i++) {
        reporter->registerHint(
            ErrorType::RUNTIME_ERROR,
            "테스트 에러",
            "힌트 " + std::to_string(i)
        );
    }

    SourceLocation loc("test.ksj", 1, 1);
    auto error = RuntimeError("테스트 에러", loc);

    std::ostringstream oss;
    EXPECT_NO_THROW(reporter->report(error, oss));
}

// ============================================================================
// 기본 힌트 시스템 테스트
// ============================================================================

TEST(DefaultHintsTest, ShouldRegisterMultipleHints) {
    auto reporter = std::make_unique<ErrorReporter>();
    reporter->setColorEnabled(false);
    reporter->registerDefaultHints();

    // NAME_ERROR 힌트 테스트
    auto nameError = NameError("정의되지 않은 변수: x");
    std::ostringstream oss1;
    reporter->report(nameError, oss1);
    std::string output1 = oss1.str();
    EXPECT_THAT(output1, HasSubstr("변수를 사용하기 전에 먼저 선언해야 합니다"));

    // TYPE_ERROR 힌트 테스트
    auto typeError = TypeError("음수 연산은 숫자에만 적용 가능합니다");
    std::ostringstream oss2;
    reporter->report(typeError, oss2);
    std::string output2 = oss2.str();
    EXPECT_THAT(output2, HasSubstr("음수 연산자(-)는 정수나 실수에만"));

    // ZERO_DIVISION_ERROR 힌트 테스트
    auto divError = ZeroDivisionError();
    std::ostringstream oss3;
    reporter->report(divError, oss3);
    std::string output3 = oss3.str();
    EXPECT_THAT(output3, HasSubstr("나누기 전에 나누는 수가 0이 아닌지"));
}

TEST(DefaultHintsTest, ShouldProvideHintsFor20PlusErrors) {
    auto reporter = std::make_unique<ErrorReporter>();
    reporter->setColorEnabled(false);
    reporter->registerDefaultHints();

    // 다양한 에러 타입에 대한 힌트 확인
    std::vector<KingSejongError> errors = {
        NameError("정의되지 않은 변수: x"),
        TypeError("음수 연산은 숫자에만 적용 가능합니다"),
        TypeError("값이 정수 타입이 아닙니다"),
        TypeError("값이 실수 타입이 아닙니다"),
        TypeError("값이 문자열 타입이 아닙니다"),
        TypeError("값이 배열 타입이 아닙니다"),
        TypeError("함수만 호출할 수 있습니다"),
        TypeError("배열 인덱스는 정수여야 합니다"),
        TypeError("서로 다른 타입의 값을 비교할 수 없습니다"),
        TypeError("이 타입은 크기 비교를 지원하지 않습니다"),
        ZeroDivisionError(),
        IndexError("인덱스가 배열 범위를 벗어났습니다"),
        ArgumentError("함수의 인자 개수가 일치하지 않습니다"),
        ArgumentError("인자는 배열이어야 합니다"),
        ArgumentError("인자는 문자열이어야 합니다"),
        RuntimeError("지원되지 않는 연산"),
        RuntimeError("지원되지 않는 단항 연산자"),
        RuntimeError("지원되지 않는 정수 연산자"),
        RuntimeError("지원되지 않는 실수 연산자"),
        RuntimeError("지원되지 않는 비교 연산자"),
        RuntimeError("지원되지 않는 논리 연산자"),
        RuntimeError("알 수 없는 조사입니다"),
        ParserError("다음 토큰으로"),
        ParserError("표현식을 파싱할 수 없습니다"),
        ParserError("정수로 변환할 수 없습니다"),
        ParserError("실수로 변환할 수 없습니다")
    };

    // 모든 에러에 대해 힌트가 표시되는지 확인
    int hintsFound = 0;
    for (const auto& error : errors) {
        std::ostringstream oss;
        reporter->report(error, oss);
        std::string output = oss.str();

        // 힌트는 "💡" 문자나 "힌트:" 문자열을 포함함
        if (output.find("💡") != std::string::npos || output.find("예시:") != std::string::npos) {
            hintsFound++;
        }
    }

    // 26개의 에러 중 최소 20개 이상 힌트가 제공되어야 함
    EXPECT_GE(hintsFound, 20);
}
