/**
 * @file ClassTest.cpp
 * @brief Phase 7.1 클래스 시스템 통합 테스트
 * @author KingSejong Team
 * @date 2025-11-18
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "semantic/SemanticAnalyzer.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"
#include "evaluator/Value.h"

using namespace kingsejong;
using namespace kingsejong::lexer;
using namespace kingsejong::parser;
using namespace kingsejong::semantic;
using namespace kingsejong::bytecode;
using namespace kingsejong::evaluator;

// ============================================================================
// Parser 테스트 (이미 ParserTest.cpp에 있지만 재확인)
// ============================================================================

TEST(ClassParserTest, ShouldParseSimpleClassDefinition) {
    std::string input = R"(
        클래스 사람 {
            비공개 문자열 이름
            비공개 정수 나이
        }
    )";

    Lexer lexer(input);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);
    ASSERT_EQ(parser.errors().size(), 0);
    ASSERT_EQ(program->statements().size(), 1);

    auto classStmt = dynamic_cast<ast::ClassStatement*>(program->statements()[0].get());
    ASSERT_NE(classStmt, nullptr);
    EXPECT_EQ(classStmt->className(), "사람");
    EXPECT_EQ(classStmt->fields().size(), 2);
    EXPECT_EQ(classStmt->methods().size(), 0);
}

TEST(ClassParserTest, ShouldParseClassWithConstructor) {
    std::string input = R"(
        클래스 계좌 {
            비공개 정수 잔액

            생성자(초기잔액) {
                자신.잔액 = 초기잔액
            }
        }
    )";

    Lexer lexer(input);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);
    ASSERT_EQ(parser.errors().size(), 0);

    auto classStmt = dynamic_cast<ast::ClassStatement*>(program->statements()[0].get());
    ASSERT_NE(classStmt, nullptr);
    EXPECT_EQ(classStmt->className(), "계좌");
    EXPECT_NE(classStmt->constructor(), nullptr);
    EXPECT_EQ(classStmt->constructor()->parameters().size(), 1);
}

TEST(ClassParserTest, ShouldParseClassWithMethods) {
    std::string input = R"(
        클래스 계산기 {
            공개 함수 더하기(a, b) {
                반환 a + b
            }
        }
    )";

    Lexer lexer(input);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    ASSERT_NE(program, nullptr);
    ASSERT_EQ(parser.errors().size(), 0);

    auto classStmt = dynamic_cast<ast::ClassStatement*>(program->statements()[0].get());
    ASSERT_NE(classStmt, nullptr);
    EXPECT_EQ(classStmt->methods().size(), 1);
    EXPECT_EQ(classStmt->methods()[0]->methodName(), "더하기");
}

// ============================================================================
// SemanticAnalyzer 테스트
// ============================================================================

TEST(ClassSemanticTest, ShouldDetectDuplicateFieldNames) {
    std::string input = R"(
        클래스 테스트 {
            비공개 정수 값
            비공개 문자열 값
        }
    )";

    Lexer lexer(input);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);
    ASSERT_GT(analyzer.errors().size(), 0);
    EXPECT_NE(analyzer.errors()[0].message.find("중복된 필드 이름"), std::string::npos);
}

TEST(ClassSemanticTest, ShouldDetectDuplicateMethodNames) {
    std::string input = R"(
        클래스 테스트 {
            공개 함수 메서드() { }
            공개 함수 메서드() { }
        }
    )";

    Lexer lexer(input);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);
    ASSERT_GT(analyzer.errors().size(), 0);
    EXPECT_NE(analyzer.errors()[0].message.find("중복된 메서드 이름"), std::string::npos);
}

TEST(ClassSemanticTest, ShouldDetectUndefinedClass) {
    std::string input = R"(
        obj = 존재하지않는클래스(10)
    )";

    Lexer lexer(input);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_FALSE(result);
    ASSERT_GT(analyzer.errors().size(), 0);
    // Should have error about undefined function or class
    bool foundError = false;
    for (const auto& err : analyzer.errors()) {
        if (err.message.find("존재하지않는클래스") != std::string::npos) {
            foundError = true;
            break;
        }
    }
    EXPECT_TRUE(foundError);
}

TEST(ClassSemanticTest, ShouldRegisterClassInSymbolTable) {
    std::string input = R"(
        클래스 사람 {
            비공개 문자열 이름
        }

        p = 사람()
    )";

    Lexer lexer(input);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    SemanticAnalyzer analyzer;
    bool result = analyzer.analyze(program.get());

    EXPECT_TRUE(result);
    EXPECT_EQ(analyzer.errors().size(), 0);
}

// ============================================================================
// Compiler + VM 통합 테스트
// ============================================================================

TEST(ClassIntegrationTest, ShouldCompileSimpleClassDefinition) {
    std::string input = R"(
        클래스 사람 {
            비공개 문자열 이름
            비공개 정수 나이
        }
    )";

    Lexer lexer(input);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool compiled = compiler.compile(program.get(), &chunk);
    EXPECT_TRUE(compiled);
}

TEST(ClassIntegrationTest, ShouldCreateClassInstance) {
    std::string input = R"(
        클래스 계좌 {
            비공개 정수 잔액
        }

        내계좌 = 계좌()
    )";

    Lexer lexer(input);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool compiled = compiler.compile(program.get(), &chunk);
    ASSERT_TRUE(compiled);

    VM vm;
    VMResult result = vm.run(&chunk);
    EXPECT_EQ(result, VMResult::OK);

    // 전역 변수 '내계좌'가 클래스 인스턴스인지 확인
    auto globals = vm.globals();
    Value accountVal = globals->get("내계좌");
    EXPECT_TRUE(accountVal.isClassInstance());
}

TEST(ClassIntegrationTest, ShouldAccessFieldValues) {
    std::string input = R"(
        클래스 포인트 {
            비공개 정수 x
            비공개 정수 y
        }

        p = 포인트()
    )";

    Lexer lexer(input);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);

    Chunk chunk;
    Compiler compiler;
    bool compiled = compiler.compile(program.get(), &chunk);
    ASSERT_TRUE(compiled);

    VM vm;
    VMResult result = vm.run(&chunk);
    EXPECT_EQ(result, VMResult::OK);

    // 인스턴스 확인
    auto globals = vm.globals();
    Value pointVal = globals->get("p");
    ASSERT_TRUE(pointVal.isClassInstance());

    // 필드가 null로 초기화되었는지 확인
    auto instance = pointVal.asClassInstance();
    Value xVal = instance->getField("x");
    Value yVal = instance->getField("y");
    EXPECT_TRUE(xVal.isNull());
    EXPECT_TRUE(yVal.isNull());
}

TEST(ClassIntegrationTest, ShouldSetFieldValues) {
    std::string input = R"(
        클래스 포인트 {
            비공개 정수 x
            비공개 정수 y
        }

        p = 포인트()
        p.x = 10
        p.y = 20
    )";

    Lexer lexer(input);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(parser.errors().size(), 0);

    Chunk chunk;
    Compiler compiler;
    bool compiled = compiler.compile(program.get(), &chunk);
    ASSERT_TRUE(compiled);

    VM vm;
    VMResult result = vm.run(&chunk);
    EXPECT_EQ(result, VMResult::OK);

    // 인스턴스 확인
    auto globals = vm.globals();
    Value pointVal = globals->get("p");
    ASSERT_TRUE(pointVal.isClassInstance());

    // 필드 값이 제대로 설정되었는지 확인
    auto instance = pointVal.asClassInstance();
    Value xVal = instance->getField("x");
    Value yVal = instance->getField("y");
    ASSERT_TRUE(xVal.isInteger());
    ASSERT_TRUE(yVal.isInteger());
    EXPECT_EQ(xVal.asInteger(), 10);
    EXPECT_EQ(yVal.asInteger(), 20);
}

// ============================================================================
// Value 시스템 테스트
// ============================================================================

TEST(ClassValueTest, ShouldCreateClassDefinition) {
    std::vector<std::string> fieldNames = {"이름", "나이"};
    std::unordered_map<std::string, std::shared_ptr<Function>> methods;

    auto classDef = std::make_shared<ClassDefinition>(
        "사람",
        fieldNames,
        methods,
        nullptr,
        ""
    );

    EXPECT_EQ(classDef->className(), "사람");
    EXPECT_EQ(classDef->fieldNames().size(), 2);
    EXPECT_EQ(classDef->fieldNames()[0], "이름");
    EXPECT_EQ(classDef->fieldNames()[1], "나이");
}

TEST(ClassValueTest, ShouldCreateClassInstance) {
    std::vector<std::string> fieldNames = {"x", "y"};
    std::unordered_map<std::string, std::shared_ptr<Function>> methods;

    auto classDef = std::make_shared<ClassDefinition>(
        "포인트",
        fieldNames,
        methods,
        nullptr,
        ""
    );

    auto instance = std::make_shared<ClassInstance>(classDef);

    EXPECT_EQ(instance->classDef()->className(), "포인트");

    // 필드가 null로 초기화되는지 확인
    Value xVal = instance->getField("x");
    Value yVal = instance->getField("y");
    EXPECT_TRUE(xVal.isNull());
    EXPECT_TRUE(yVal.isNull());
}

TEST(ClassValueTest, ShouldSetAndGetFields) {
    std::vector<std::string> fieldNames = {"값"};
    std::unordered_map<std::string, std::shared_ptr<Function>> methods;

    auto classDef = std::make_shared<ClassDefinition>(
        "컨테이너",
        fieldNames,
        methods,
        nullptr,
        ""
    );

    auto instance = std::make_shared<ClassInstance>(classDef);

    // 필드 값 설정
    instance->setField("값", Value::createInteger(42));

    // 필드 값 읽기
    Value val = instance->getField("값");
    EXPECT_TRUE(val.isInteger());
    EXPECT_EQ(val.asInteger(), 42);
}

TEST(ClassValueTest, ShouldThrowOnInvalidField) {
    std::vector<std::string> fieldNames = {"x"};
    std::unordered_map<std::string, std::shared_ptr<Function>> methods;

    auto classDef = std::make_shared<ClassDefinition>(
        "테스트",
        fieldNames,
        methods,
        nullptr,
        ""
    );

    auto instance = std::make_shared<ClassInstance>(classDef);

    // 존재하지 않는 필드 접근
    EXPECT_THROW(instance->getField("y"), std::runtime_error);
    EXPECT_THROW(instance->setField("y", Value::createInteger(10)), std::runtime_error);
}

TEST(ClassValueTest, ShouldConvertInstanceToString) {
    std::vector<std::string> fieldNames = {"이름"};
    std::unordered_map<std::string, std::shared_ptr<Function>> methods;

    auto classDef = std::make_shared<ClassDefinition>(
        "사람",
        fieldNames,
        methods,
        nullptr,
        ""
    );

    auto instance = std::make_shared<ClassInstance>(classDef);
    Value val = Value::createClassInstance(instance);

    std::string str = val.toString();
    EXPECT_NE(str.find("사람"), std::string::npos);
    EXPECT_NE(str.find("인스턴스"), std::string::npos);
}
