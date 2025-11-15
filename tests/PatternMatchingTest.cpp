/**
 * @file PatternMatchingTest.cpp
 * @brief 패턴 매칭 테스트
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * F5.5 패턴 매칭 기능 테스트 (TDD)
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Environment.h"

using namespace kingsejong;

/**
 * @class PatternMatchingTest
 * @brief 패턴 매칭 테스트 픽스처
 */
class PatternMatchingTest : public ::testing::Test {
protected:
    /**
     * @brief 코드를 파싱하고 평가하여 결과를 반환
     */
    std::string eval(const std::string& code) {
        lexer::Lexer lexer(code);
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();

        // 파싱 에러 체크
        const auto& errors = parser.errors();
        if (!errors.empty()) {
            return "PARSE_ERROR: " + errors[0];
        }

        if (!program) {
            return "NULL_PROGRAM";
        }

        auto env = std::make_shared<evaluator::Environment>();
        evaluator::Evaluator evaluator(env);

        try {
            evaluator::Value result = evaluator.eval(program.get());
            return result.toString();
        } catch (const std::exception& e) {
            return "ERROR: " + std::string(e.what());
        }
    }
};

// ============================================================================
// 기본 리터럴 패턴 매칭
// ============================================================================

TEST_F(PatternMatchingTest, IntegerLiteralPattern) {
    std::string code = R"(
정수 x = 1
정수 결과 = x 에 대해 {
    1 -> "하나"
    2 -> "둘"
    3 -> "셋"
}
결과
    )";

    EXPECT_EQ(eval(code), "하나");
}

TEST_F(PatternMatchingTest, IntegerLiteralPattern_SecondCase) {
    std::string code = R"(
정수 x = 2
정수 결과 = x 에 대해 {
    1 -> "하나"
    2 -> "둘"
    3 -> "셋"
}
결과
    )";

    EXPECT_EQ(eval(code), "둘");
}

TEST_F(PatternMatchingTest, StringLiteralPattern) {
    std::string code = R"(
문자열 과일 = "사과"
문자열 결과 = 과일 에 대해 {
    "사과" -> "빨강"
    "바나나" -> "노랑"
    "포도" -> "보라"
}
결과
    )";

    EXPECT_EQ(eval(code), "빨강");
}

TEST_F(PatternMatchingTest, BooleanLiteralPattern) {
    std::string code = R"(
논리 상태 = 참
정수 결과 = 상태 에 대해 {
    참 -> "켜짐"
    거짓 -> "꺼짐"
}
결과
    )";

    EXPECT_EQ(eval(code), "켜짐");
}

// ============================================================================
// 와일드카드 패턴 (_)
// ============================================================================

TEST_F(PatternMatchingTest, WildcardPattern) {
    std::string code = R"(
정수 x = 999
정수 결과 = x 에 대해 {
    1 -> "하나"
    2 -> "둘"
    _ -> "기타"
}
결과
    )";

    EXPECT_EQ(eval(code), "기타");
}

TEST_F(PatternMatchingTest, WildcardPattern_FirstMatch) {
    // 패턴은 위에서 아래로 첫 번째 매칭만 실행
    std::string code = R"(
정수 x = 1
정수 결과 = x 에 대해 {
    1 -> "정확히 1"
    _ -> "기타"
}
결과
    )";

    EXPECT_EQ(eval(code), "정확히 1");
}

// ============================================================================
// 바인딩 패턴 (변수명)
// ============================================================================

TEST_F(PatternMatchingTest, BindingPattern) {
    std::string code = R"(
정수 x = 10
정수 결과 = x 에 대해 {
    1 -> "하나"
    n -> n * 2
}
결과
    )";

    EXPECT_EQ(eval(code), "20");
}

TEST_F(PatternMatchingTest, BindingPattern_WithString) {
    std::string code = R"(
문자열 이름 = "철수"
정수 결과 = 이름 에 대해 {
    "홍길동" -> "안녕, 길동씨"
    name -> "안녕, " + name + "님"
}
결과
    )";

    EXPECT_EQ(eval(code), "안녕, 철수님");
}

// ============================================================================
// 복잡한 표현식
// ============================================================================

TEST_F(PatternMatchingTest, ExpressionAsBody) {
    std::string code = R"(
정수 x = 3
정수 결과 = x 에 대해 {
    1 -> 1 * 1
    2 -> 2 * 2
    3 -> 3 * 3
    _ -> 0
}
결과
    )";

    EXPECT_EQ(eval(code), "9");
}

TEST_F(PatternMatchingTest, NestedMatchExpression) {
    std::string code = R"(
정수 x = 2
정수 y = 3
정수 결과 = x 에 대해 {
    1 -> "x는 1"
    2 -> y 에 대해 {
        1 -> "x는 2, y는 1"
        2 -> "x는 2, y는 2"
        3 -> "x는 2, y는 3"
    }
    _ -> "기타"
}
결과
    )";

    EXPECT_EQ(eval(code), "x는 2, y는 3");
}

// ============================================================================
// 매칭 실패 (없는 케이스)
// ============================================================================

TEST_F(PatternMatchingTest, NoMatchingCase) {
    std::string code = R"(
정수 x = 999
정수 결과 = x 에 대해 {
    1 -> "하나"
    2 -> "둘"
}
결과
    )";

    // 매칭되는 케이스가 없으면 에러 또는 nil 반환
    // 구현에 따라 동작이 달라질 수 있음
    std::string result = eval(code);
    // 에러 메시지 또는 nil 예상
    EXPECT_TRUE(
        result.find("ERROR") != std::string::npos ||
        result == "nil" ||
        result.empty()
    );
}

// ============================================================================
// 실용적인 예제
// ============================================================================

TEST_F(PatternMatchingTest, FizzBuzz) {
    std::string code = R"(
fizzbuzz = 함수(n) {
    반환 n 에 대해 {
        15 -> "FizzBuzz"
        12 -> "Fizz"
        10 -> "Buzz"
        9 -> "Fizz"
        6 -> "Fizz"
        5 -> "Buzz"
        3 -> "Fizz"
        _ -> n
    }
}

fizzbuzz(15)
    )";

    EXPECT_EQ(eval(code), "FizzBuzz");
}

TEST_F(PatternMatchingTest, DayOfWeek) {
    std::string code = R"(
정수 요일 = 함수(n) {
    반환 n 에 대해 {
        1 -> "월요일"
        2 -> "화요일"
        3 -> "수요일"
        4 -> "목요일"
        5 -> "금요일"
        6 -> "토요일"
        7 -> "일요일"
        _ -> "잘못된 입력"
    }
}

요일(3)
    )";

    EXPECT_EQ(eval(code), "수요일");
}
