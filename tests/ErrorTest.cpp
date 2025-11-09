/**
 * @file ErrorTest.cpp
 * @brief 에러 처리 시스템 테스트
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include <gtest/gtest.h>
#include "../src/error/Error.h"

using namespace kingsejong::error;

// ============================================================================
// 기본 에러 클래스 테스트
// ============================================================================

TEST(ErrorTest, ShouldCreateBasicError)
{
    KingSejongError err(ErrorType::RUNTIME_ERROR, "테스트 에러");

    EXPECT_EQ(err.getType(), ErrorType::RUNTIME_ERROR);
    EXPECT_STREQ(err.what(), "테스트 에러");
}

TEST(ErrorTest, ShouldCreateErrorWithLocation)
{
    SourceLocation loc("test.ksj", 10, 5);
    KingSejongError err(ErrorType::PARSER_ERROR, "파서 에러 발생", loc);

    EXPECT_EQ(err.getType(), ErrorType::PARSER_ERROR);
    EXPECT_TRUE(err.getLocation().has_value());
    EXPECT_EQ(err.getLocation()->line, 10);
    EXPECT_EQ(err.getLocation()->column, 5);
}

// ============================================================================
// SourceLocation 테스트
// ============================================================================

TEST(ErrorTest, ShouldFormatSourceLocation)
{
    SourceLocation loc1("test.ksj", 10, 5);
    EXPECT_EQ(loc1.toString(), "test.ksj:10:5");

    SourceLocation loc2("test.ksj", 10, 0);
    EXPECT_EQ(loc2.toString(), "test.ksj:10");

    SourceLocation loc3;
    EXPECT_EQ(loc3.toString(), "<unknown>");
}

// ============================================================================
// 에러 타입 문자열 변환 테스트
// ============================================================================

TEST(ErrorTest, ShouldConvertErrorTypeToString)
{
    KingSejongError err1(ErrorType::LEXER_ERROR, "렉서 에러");
    EXPECT_EQ(err1.getTypeString(), "렉서 에러");

    KingSejongError err2(ErrorType::PARSER_ERROR, "파서 에러");
    EXPECT_EQ(err2.getTypeString(), "파서 에러");

    KingSejongError err3(ErrorType::RUNTIME_ERROR, "실행 에러");
    EXPECT_EQ(err3.getTypeString(), "실행 에러");

    KingSejongError err4(ErrorType::TYPE_ERROR, "타입 에러");
    EXPECT_EQ(err4.getTypeString(), "타입 에러");

    KingSejongError err5(ErrorType::NAME_ERROR, "이름 에러");
    EXPECT_EQ(err5.getTypeString(), "이름 에러");
}

// ============================================================================
// 편의 함수 테스트
// ============================================================================

TEST(ErrorTest, ShouldCreateLexerError)
{
    auto err = LexerError("잘못된 토큰");

    EXPECT_EQ(err.getType(), ErrorType::LEXER_ERROR);
    EXPECT_STREQ(err.what(), "잘못된 토큰");
}

TEST(ErrorTest, ShouldCreateParserError)
{
    auto err = ParserError("구문 분석 실패");

    EXPECT_EQ(err.getType(), ErrorType::PARSER_ERROR);
    EXPECT_STREQ(err.what(), "구문 분석 실패");
}

TEST(ErrorTest, ShouldCreateRuntimeError)
{
    auto err = RuntimeError("실행 중 에러 발생");

    EXPECT_EQ(err.getType(), ErrorType::RUNTIME_ERROR);
    EXPECT_STREQ(err.what(), "실행 중 에러 발생");
}

TEST(ErrorTest, ShouldCreateTypeError)
{
    auto err = TypeError("타입이 일치하지 않습니다");

    EXPECT_EQ(err.getType(), ErrorType::TYPE_ERROR);
    EXPECT_STREQ(err.what(), "타입이 일치하지 않습니다");
}

TEST(ErrorTest, ShouldCreateNameError)
{
    auto err = NameError("정의되지 않은 변수: x");

    EXPECT_EQ(err.getType(), ErrorType::NAME_ERROR);
    EXPECT_STREQ(err.what(), "정의되지 않은 변수: x");
}

TEST(ErrorTest, ShouldCreateValueError)
{
    auto err = ValueError("잘못된 값");

    EXPECT_EQ(err.getType(), ErrorType::VALUE_ERROR);
    EXPECT_STREQ(err.what(), "잘못된 값");
}

TEST(ErrorTest, ShouldCreateZeroDivisionError)
{
    auto err = ZeroDivisionError();

    EXPECT_EQ(err.getType(), ErrorType::ZERO_DIVISION_ERROR);
    EXPECT_STREQ(err.what(), "0으로 나눌 수 없습니다");
}

TEST(ErrorTest, ShouldCreateIndexError)
{
    auto err = IndexError("배열 인덱스 범위 초과");

    EXPECT_EQ(err.getType(), ErrorType::INDEX_ERROR);
    EXPECT_STREQ(err.what(), "배열 인덱스 범위 초과");
}

TEST(ErrorTest, ShouldCreateArgumentError)
{
    auto err = ArgumentError("함수 인자 개수가 일치하지 않습니다");

    EXPECT_EQ(err.getType(), ErrorType::ARGUMENT_ERROR);
    EXPECT_STREQ(err.what(), "함수 인자 개수가 일치하지 않습니다");
}

// ============================================================================
// 위치 정보 포함 에러 테스트
// ============================================================================

TEST(ErrorTest, ShouldIncludeLocationInErrorMessage)
{
    SourceLocation loc("example.ksj", 15, 10);
    auto err = RuntimeError("실행 중 에러", loc);

    std::string msg = err.what();
    EXPECT_TRUE(msg.find("example.ksj:15:10") != std::string::npos);
    EXPECT_TRUE(msg.find("실행 중 에러") != std::string::npos);
}

// ============================================================================
// std::runtime_error 호환성 테스트
// ============================================================================

TEST(ErrorTest, ShouldBeCompatibleWithRuntimeError)
{
    try
    {
        throw NameError("정의되지 않은 변수");
    }
    catch (const std::runtime_error& e)
    {
        // KingSejongError는 std::runtime_error로 캐치 가능
        EXPECT_STREQ(e.what(), "정의되지 않은 변수");
    }
}

TEST(ErrorTest, ShouldPreserveErrorTypeAfterCatch)
{
    try
    {
        throw TypeError("타입 불일치");
    }
    catch (const KingSejongError& e)
    {
        EXPECT_EQ(e.getType(), ErrorType::TYPE_ERROR);
        EXPECT_STREQ(e.what(), "타입 불일치");
    }
}
