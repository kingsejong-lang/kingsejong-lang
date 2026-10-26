/**
 * @file EvaluatorTest.cpp
 * @brief Evaluator 클래스 테스트
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include <gtest/gtest.h>
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "parser/Parser.h"
#include "lexer/Lexer.h"

using namespace kingsejong;

// ============================================================================
// 헬퍼 함수
// ============================================================================

/**
 * @brief 코드를 파싱하고 평가하는 헬퍼 함수
 * @param input 입력 코드
 * @return 평가 결과 Value
 */
static evaluator::Value evalInput(const std::string& input)
{
    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_EQ(parser.errors().size(), 0) << "파서 에러: " << (parser.errors().empty() ? "" : parser.errors()[0]);

    evaluator::Evaluator evaluator;
    return evaluator.evalProgram(program.get());
}

// ============================================================================
// 리터럴 평가 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateIntegerLiteral)
{
    auto result = evalInput("42");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 42);
}

TEST(EvaluatorTest, ShouldEvaluateFloatLiteral)
{
    auto result = evalInput("3.14");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 3.14, 1e-9);
}

TEST(EvaluatorTest, ShouldEvaluateStringLiteral)
{
    auto result = evalInput("\"안녕하세요\"");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "안녕하세요");
}

TEST(EvaluatorTest, ShouldEvaluateBooleanLiteral)
{
    auto result1 = evalInput("참");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("거짓");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

// ============================================================================
// 산술 연산 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateIntegerAddition)
{
    auto result = evalInput("5 + 3");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 8);
}

TEST(EvaluatorTest, ShouldEvaluateIntegerSubtraction)
{
    auto result = evalInput("10 - 4");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 6);
}

TEST(EvaluatorTest, ShouldEvaluateIntegerMultiplication)
{
    auto result = evalInput("6 * 7");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 42);
}

TEST(EvaluatorTest, ShouldEvaluateIntegerDivision)
{
    auto result = evalInput("20 / 5");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 4);
}

TEST(EvaluatorTest, ShouldEvaluateIntegerModulo)
{
    auto result = evalInput("17 % 5");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 2);
}

TEST(EvaluatorTest, ShouldEvaluateComplexArithmetic)
{
    auto result = evalInput("2 + 3 * 4");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 14);  // 우선순위: 3*4=12, 2+12=14
}

TEST(EvaluatorTest, ShouldEvaluateArithmeticWithParentheses)
{
    auto result = evalInput("(2 + 3) * 4");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 20);  // 괄호 우선: 2+3=5, 5*4=20
}

// ============================================================================
// 실수 연산 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateFloatAddition)
{
    auto result = evalInput("3.5 + 2.5");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 6.0, 1e-9);
}

TEST(EvaluatorTest, ShouldEvaluateMixedArithmetic)
{
    auto result = evalInput("10 + 3.5");  // 정수 + 실수 = 실수

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 13.5, 1e-9);
}

// ============================================================================
// 단항 연산 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateNegation)
{
    auto result = evalInput("-5");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), -5);
}

TEST(EvaluatorTest, ShouldEvaluateLogicalNot)
{
    auto result1 = evalInput("!참");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_FALSE(result1.asBoolean());

    auto result2 = evalInput("!거짓");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_TRUE(result2.asBoolean());
}

// ============================================================================
// 비교 연산 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateEquality)
{
    auto result1 = evalInput("5 == 5");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("5 == 3");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

TEST(EvaluatorTest, ShouldEvaluateInequality)
{
    auto result1 = evalInput("5 != 3");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("5 != 5");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

TEST(EvaluatorTest, ShouldEvaluateLessThan)
{
    auto result1 = evalInput("3 < 5");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("5 < 3");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

TEST(EvaluatorTest, ShouldEvaluateGreaterThan)
{
    auto result1 = evalInput("5 > 3");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("3 > 5");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

// ============================================================================
// 논리 연산 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateLogicalAnd)
{
    auto result1 = evalInput("참 && 참");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("참 && 거짓");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

TEST(EvaluatorTest, ShouldEvaluateLogicalOr)
{
    auto result1 = evalInput("참 || 거짓");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("거짓 || 거짓");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

// ============================================================================
// 변수 선언 및 참조 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateVarDeclaration)
{
    auto result = evalInput("정수 x = 42\nx");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 42);
}

TEST(EvaluatorTest, ShouldEvaluateVarDeclarationWithoutInitializer)
{
    auto result = evalInput("정수 x\nx");

    EXPECT_TRUE(result.isNull());
}

TEST(EvaluatorTest, ShouldEvaluateMultipleVarDeclarations)
{
    auto result = evalInput(R"(
        정수 a = 10
        정수 b = 20
        a + b
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 30);
}

TEST(EvaluatorTest, ShouldEvaluateVarReassignment)
{
    auto result = evalInput(R"(
        정수 x = 5
        x = 10
        x
    )");

    // AssignmentStatement가 구현되어 정상 동작
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 10);
}

// ============================================================================
// 문자열 연산 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateStringConcatenation)
{
    auto result = evalInput("\"안녕\" + \"하세요\"");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "안녕하세요");
}

// ============================================================================
// 에러 처리 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldThrowOnUndefinedVariable)
{
    EXPECT_THROW(
        {
            evalInput("undefined_var");
        },
        std::runtime_error
    );
}

TEST(EvaluatorTest, ShouldThrowOnDivisionByZero)
{
    EXPECT_THROW(
        {
            evalInput("10 / 0");
        },
        std::runtime_error
    );
}

TEST(EvaluatorTest, ShouldThrowOnModuloByZero)
{
    EXPECT_THROW(
        {
            evalInput("10 % 0");
        },
        std::runtime_error
    );
}

// ============================================================================
// 복합 표현식 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateComplexExpression)
{
    auto result = evalInput(R"(
        정수 a = 5
        정수 b = 3
        정수 c = 2
        (a + b) * c
    )");

    // Parser-level ASI로 해결됨: 줄이 바뀌면 자동으로 세미콜론 삽입
    // 정수 c = 2 다음에 개행되므로 ASI가 적용되어 (a + b)가 함수 호출로 인식되지 않음
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 16);  // (5+3)*2 = 16
}

TEST(EvaluatorTest, ShouldEvaluateNestedExpressions)
{
    auto result = evalInput("((2 + 3) * (4 + 5)) / 3");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 15);  // ((5)*(9))/3 = 45/3 = 15
}

// ============================================================================
// Truthy/Falsy 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateTruthiness)
{
    auto result1 = evalInput("!0");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());  // 0은 거짓

    auto result2 = evalInput("!1");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());  // 1은 참

    auto result3 = evalInput("!\"\"");
    EXPECT_TRUE(result3.isBoolean());
    EXPECT_TRUE(result3.asBoolean());  // 빈 문자열은 거짓
}

// ============================================================================
// 예외 처리 테스트 (Exception Handling)
// ============================================================================

/**
 * @brief 던지다 문장 테스트 - 에러 값 생성
 */
TEST(EvaluatorTest, ShouldThrowError)
{
    // 잡는 시도 블록이 없으면 ThrowSignal 이 바깥으로 나간다 (main 이 실행 에러로 보고한다)
    try
    {
        evalInput(R"(
            던지다 "에러 발생"
        )");
        FAIL() << "던지다 가 예외로 전파되어야 한다";
    }
    catch (const evaluator::ThrowSignal& signal)
    {
        ASSERT_TRUE(signal.value().isError());
        auto err = signal.value().asError();
        EXPECT_EQ(err->message(), "에러 발생");
        EXPECT_EQ(err->type(), "Error");
        EXPECT_STREQ(signal.what(), "Error: 에러 발생");
    }
}

/**
 * @brief 오류 블록에서 받은 에러를 다시 던지면 원래 타입과 메시지가 유지된다
 */
TEST(EvaluatorTest, ShouldKeepTypeAndMessageWhenRethrowing)
{
    auto result = evalInput(R"(
        문자열 기록 = ""
        시도 {
            시도 {
                던지다 "실패함"
            } 오류 (e) {
                던지다 e
            }
        } 오류 (e) {
            기록 = e
        }
        기록
    )");
    EXPECT_EQ(result.asString(), "Error: 실패함");

    auto runtime = evalInput(R"(
        문자열 기록 = ""
        시도 {
            시도 {
                1 / 0
            } 오류 (e) {
                던지다 e
            }
        } 오류 (e) {
            기록 = e
        }
        기록
    )");
    EXPECT_EQ(runtime.asString().rfind("RuntimeError: 0으로 나눌 수 없습니다", 0), 0u);
}

/**
 * @brief 던지다 뒤의 문장은 실행되지 않는다
 */
TEST(EvaluatorTest, ShouldSkipStatementsAfterThrow)
{
    auto result = evalInput(R"(
        정수 기록 = 0
        시도 {
            던지다 "문제"
            기록 = 1
        } 오류 (e) {
            기록 = 2
        }
        기록
    )");

    EXPECT_EQ(result.asInteger(), 2);
}

/**
 * @brief 함수 안에서 던진 에러는 호출한 쪽의 시도 블록이 잡는다
 */
TEST(EvaluatorTest, ShouldPropagateThrowOutOfFunctions)
{
    auto result = evalInput(R"(
        정수 기록 = 0
        함수 위험() {
            던지다 "깊은 문제"
            기록 = 100
        }
        시도 {
            위험()
            기록 = 기록 + 10
        } 오류 (e) {
            기록 = 기록 + 1
        }
        기록
    )");

    EXPECT_EQ(result.asInteger(), 1);
}

/**
 * @brief 비동기 함수 안에서 던진 에러는 Promise 거부가 된다
 */
TEST(EvaluatorTest, ShouldRejectPromiseWhenAsyncFunctionThrows)
{
    auto result = evalInput(R"(
        실패 = 비동기 함수() {
            던지다 "실패함"
        }
        실패()
    )");

    ASSERT_TRUE(result.isPromise());
    EXPECT_EQ(result.asPromise()->state(), evaluator::Promise::State::REJECTED);
    EXPECT_EQ(result.asPromise()->value().toString(), "Error: 실패함");
}

/**
 * @brief 내장 연산의 런타임 에러도 오류 블록이 메시지와 함께 잡는다
 */
TEST(EvaluatorTest, ShouldCatchRuntimeErrorsWithMessage)
{
    auto result = evalInput(R"(
        문자열 메시지 = ""
        시도 {
            정수 x = 10 / 0
        } 오류 (e) {
            메시지 = e
        }
        메시지
    )");

    ASSERT_TRUE(result.isString());
    EXPECT_NE(result.asString().find("RuntimeError: "), std::string::npos);
    EXPECT_NE(result.asString().find("0으로 나눌 수 없습니다"), std::string::npos);
}

/**
 * @brief 시도-오류 블록 테스트 - 에러 잡기
 */
TEST(EvaluatorTest, ShouldCatchError)
{
    auto result = evalInput(R"(
        정수 결과 = 0
        시도 {
            던지다 "문제 발생"
        } 오류 (e) {
            결과 = 42
        }
        결과
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 42);
}

/**
 * @brief 시도-오류 블록 테스트 - 에러 변수 접근
 */
TEST(EvaluatorTest, ShouldAccessErrorVariable)
{
    auto result = evalInput(R"(
        문자열 메시지 = ""
        시도 {
            던지다 "에러 메시지"
        } 오류 (e) {
            메시지 = e
        }
        메시지
    )");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "Error: 에러 메시지");
}

/**
 * @brief 시도-마지막 블록 테스트 - finally 실행 보장
 */
TEST(EvaluatorTest, ShouldExecuteFinallyBlock)
{
    auto result = evalInput(R"(
        정수 카운트 = 0
        시도 {
            카운트 = 1
        } 마지막 {
            카운트 = 카운트 + 10
        }
        카운트
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 11);
}

/**
 * @brief 시도-오류-마지막 블록 테스트 - 에러 발생 시 finally 실행
 */
TEST(EvaluatorTest, ShouldExecuteFinallyAfterError)
{
    auto result = evalInput(R"(
        정수 카운트 = 0
        시도 {
            카운트 = 1
            던지다 "에러"
        } 오류 (e) {
            카운트 = 5
        } 마지막 {
            카운트 = 카운트 + 10
        }
        카운트
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 15);  // 1 → 5 (catch) → 15 (finally)
}

/**
 * @brief 중첩된 시도 블록 테스트
 */
TEST(EvaluatorTest, ShouldHandleNestedTryBlocks)
{
    auto result = evalInput(R"(
        정수 결과 = 0
        시도 {
            시도 {
                던지다 "내부 에러"
            } 오류 (e) {
                결과 = 10
            }
        } 마지막 {
            결과 = 결과 + 5
        }
        결과
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 15);
}

/**
 * @brief 에러 없이 정상 실행되는 시도 블록
 */
TEST(EvaluatorTest, ShouldExecuteTryBlockWithoutError)
{
    auto result = evalInput(R"(
        정수 값 = 0
        시도 {
            값 = 42
        } 오류 (e) {
            값 = -1
        }
        값
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 42);  // catch 블록은 실행되지 않음
}

/**
 * @brief 에러를 잡지 못한 경우 전파
 */
TEST(EvaluatorTest, ShouldPropagateUncaughtError)
{
    // 오류 절이 없으면 마지막 블록을 돌린 뒤 에러가 바깥으로 전파된다
    try
    {
        evalInput(R"(
            시도 {
                던지다 "잡히지 않은 에러"
            } 마지막 {
                정수 x = 1
            }
        )");
        FAIL() << "잡지 않은 에러는 전파되어야 한다";
    }
    catch (const evaluator::ThrowSignal& signal)
    {
        ASSERT_TRUE(signal.value().isError());
        EXPECT_EQ(signal.value().asError()->message(), "잡히지 않은 에러");
    }
}

/**
 * @brief 상속: 부모의 필드·메서드·생성자를 물려받고, 자식이 정의한 메서드가 우선한다
 */
TEST(EvaluatorTest, ShouldInheritFieldsMethodsAndConstructor)
{
    auto result = evalInput(R"(
        클래스 동물 {
            공개 문자열 이름
            생성자(이름) {
                자신.이름 = 이름
            }
            공개 함수 소개() {
                반환 자신.이름 + ":" + 자신.소리()
            }
            공개 함수 소리() {
                반환 "..."
            }
        }
        클래스 개 상속 동물 {
            공개 정수 나이
            공개 함수 소리() {
                반환 "멍멍"
            }
        }
        클래스 강아지 상속 개 {
            생성자(이름, 나이) {
                자신.이름 = 이름
                자신.나이 = 나이
            }
        }
        결과 = 개("바둑이").소개() + " " + 강아지("초코", 1).소개() + " " + 동물("냥").소개()
        결과
    )");
    EXPECT_EQ(result.asString(), "바둑이:멍멍 초코:멍멍 냥:...");
}

TEST(EvaluatorTest, ShouldRejectUnknownSuperClass)
{
    EXPECT_THROW(evalInput("클래스 개 상속 없는클래스 {\n}\n"), std::runtime_error);
}

/**
 * @brief 상위: 부모 생성자·메서드를 원래 인스턴스로 부르고, 여러 단계 상속에서도 정의한 클래스의 부모를 가리킨다
 */
TEST(EvaluatorTest, ShouldCallParentConstructorAndMethodsWithSuper)
{
    // 아래 프로그램은 내장 함수 문자열() 을 쓴다. ctest 는 테스트마다 프로세스를 따로 띄우므로 직접 등록한다
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput(R"KSJ(
        클래스 동물 {
            공개 문자열 이름
            생성자(이름) {
                자신.이름 = 이름
            }
            공개 함수 소개() {
                반환 "나는 " + 자신.이름 + ", " + 자신.소리()
            }
            공개 함수 소리() {
                반환 "..."
            }
        }
        클래스 개 상속 동물 {
            공개 정수 나이
            생성자(이름, 나이) {
                상위(이름)
                자신.나이 = 나이
            }
            공개 함수 소리() {
                반환 "멍멍 (원래: " + 상위.소리() + ")"
            }
            공개 함수 소개() {
                반환 상위.소개() + ", " + 문자열(자신.나이) + "살"
            }
        }
        클래스 강아지 상속 개 {
            공개 함수 소리() {
                반환 "낑낑 / " + 상위.소리()
            }
        }
        결과 = 개("바둑이", 3).소개() + " | " + 강아지("초코", 1).소개()
        결과
    )KSJ");
    EXPECT_EQ(result.asString(), "나는 바둑이, 멍멍 (원래: ...), 3살 | 나는 초코, 낑낑 / 멍멍 (원래: ...), 1살");
}

TEST(EvaluatorTest, ShouldRejectSuperOutsideSubclass)
{
    EXPECT_THROW(evalInput("클래스 단독 {\n    공개 함수 f() {\n        반환 상위.f()\n    }\n}\n단독().f()\n"),
                 std::runtime_error);
}

// ============================================================================
// 거꾸로 세는 범위 반복문
// ============================================================================

TEST(EvaluatorTest, ShouldCountDownWhenRangeEndsWithIsang)
{
    evaluator::Builtin::registerAllBuiltins();  // 길이() — ctest 는 테스트마다 따로 실행한다
    auto digits = evalInput(
        "자리 = 0\n"
        "i가 3부터 1 이상 {\n자리 = 자리 * 10 + i\n}\n"
        "경로 = \"a/b\"\n"
        "j가 길이(경로) - 1부터 0 이상 반복한다 {\n자리 = 자리 * 10 + j\n}\n"
        "자리");
    EXPECT_EQ(digits.asInteger(), 321210);

    // 시작이 끝보다 작으면 돌지 않는다
    auto count = evalInput("횟수 = 0\ni가 1부터 3 이상 {\n횟수 = 횟수 + 1\n}\n횟수");
    EXPECT_EQ(count.asInteger(), 0);
}

// ============================================================================
// 괄호로 시작하는 조건
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateConditionsThatStartWithParentheses)
{
    // 첫 ')' 에서 조건을 끝내지 않고 식 전체를 조건으로 읽는다
    auto ifResult = evalInput(
        "결과 = 0\n"
        "만약 (1 > 0) && (2 > 1) {\n결과 = 1\n}\n"
        "만약 (1 > 0) && (2 < 1) {\n결과 = 2\n}\n"
        "결과");
    EXPECT_EQ(ifResult.asInteger(), 1);

    auto whileResult = evalInput(
        "x = 0\n"
        "반복 (x < 10) && (x != 3) {\nx = x + 1\n}\n"
        "x");
    EXPECT_EQ(whileResult.asInteger(), 3);
}
