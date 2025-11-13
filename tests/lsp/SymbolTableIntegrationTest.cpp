/**
 * @file SymbolTableIntegrationTest.cpp
 * @brief SymbolTable 통합 테스트 - 파싱부터 수집까지
 * @author KingSejong Team
 * @date 2025-11-13
 *
 * Lexer -> Parser -> SymbolCollector -> SymbolTable 전체 파이프라인 테스트
 */

#include <gtest/gtest.h>
#include "lsp/SymbolCollector.h"
#include "lsp/SymbolTable.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"

using namespace kingsejong::lsp;
using namespace kingsejong::lexer;
using namespace kingsejong::parser;

class SymbolTableIntegrationTest : public ::testing::Test
{
protected:
    SymbolTable symbolTable;

    void collectSymbols(const std::string& code, const std::string& uri)
    {
        Lexer lexer(code);
        Parser parser(lexer);
        auto program = parser.parseProgram();

        // 파서 에러 출력
        const auto& errors = parser.errors();
        if (!errors.empty()) {
            std::cout << "Parser errors:" << std::endl;
            for (const auto& err : errors) {
                std::cout << "  - " << err << std::endl;
            }
        }

        ASSERT_TRUE(program != nullptr) << "Program should not be null";

        symbolTable.clear();
        SymbolCollector collector(symbolTable);
        collector.collect(program.get(), uri);
    }
};

TEST_F(SymbolTableIntegrationTest, ShouldParseAndCollectSimpleVariable)
{
    // Arrange
    std::string code = "정수 x = 10";
    std::string uri = "test.ksj";

    // Act
    collectSymbols(code, uri);

    // Assert - 심볼 테이블이 채워졌는지 확인
    EXPECT_EQ(symbolTable.size(), 1u) << "Should have 1 symbol";

    auto symbol = symbolTable.findSymbol("x");
    ASSERT_TRUE(symbol.has_value()) << "Should find symbol 'x'";
    EXPECT_EQ(symbol->name, "x");
    EXPECT_EQ(symbol->kind, SymbolKind::Variable);
    EXPECT_EQ(symbol->typeInfo, "정수");
}

TEST_F(SymbolTableIntegrationTest, ShouldParseAndCollectTwoVariables)
{
    // Arrange
    std::string code = "정수 x = 10\n정수 y = x + 5";
    std::string uri = "test.ksj";

    // Act
    collectSymbols(code, uri);

    // Assert
    EXPECT_EQ(symbolTable.size(), 2u) << "Should have 2 symbols";

    // x 심볼 확인
    auto x = symbolTable.findSymbol("x");
    ASSERT_TRUE(x.has_value()) << "Should find symbol 'x'";
    EXPECT_EQ(x->name, "x");

    // y 심볼 확인
    auto y = symbolTable.findSymbol("y");
    ASSERT_TRUE(y.has_value()) << "Should find symbol 'y'";
    EXPECT_EQ(y->name, "y");

    // x의 참조 확인
    auto xRefs = symbolTable.getReferences("x");
    std::cout << "x references: " << xRefs.size() << std::endl;
    EXPECT_GE(xRefs.size(), 1u) << "x should have at least 1 reference";
}

TEST_F(SymbolTableIntegrationTest, ShouldParseAndCollectFunction)
{
    // Arrange
    std::string code = "함수 더하기(a, b) {\n    반환 a + b\n}";
    std::string uri = "test.ksj";

    // Act
    collectSymbols(code, uri);

    // Assert
    std::cout << "Symbol table size: " << symbolTable.size() << std::endl;
    auto allSymbols = symbolTable.getAllSymbols();
    for (const auto& sym : allSymbols) {
        std::cout << "  Symbol: " << sym.name
                  << ", kind: " << static_cast<int>(sym.kind)
                  << ", scope: " << sym.scope << std::endl;
    }

    EXPECT_GE(symbolTable.size(), 1u) << "Should have at least function symbol";

    auto func = symbolTable.findSymbol("더하기");
    ASSERT_TRUE(func.has_value()) << "Should find symbol '더하기'";
    EXPECT_EQ(func->kind, SymbolKind::Function);
}

TEST_F(SymbolTableIntegrationTest, DebugCharacterPositions)
{
    // Arrange - 정확한 문자 위치 확인
    std::string code = "정수 x = 10\n정수 y = x + 5";

    std::cout << "Code bytes:" << std::endl;
    for (size_t i = 0; i < code.size(); ++i) {
        unsigned char c = code[i];
        std::cout << i << ": ";
        if (c >= 32 && c < 127) {
            std::cout << "'" << c << "' ";
        } else {
            std::cout << "0x" << std::hex << (int)c << std::dec << " ";
        }
        std::cout << std::endl;
    }

    // 두 번째 줄에서 'x'의 위치 찾기
    std::cout << "\nSearching for 'x' in second line..." << std::endl;
    size_t lineStart = code.find('\n') + 1;
    std::string secondLine = code.substr(lineStart);
    std::cout << "Second line: " << secondLine << std::endl;
    std::cout << "Second line length: " << secondLine.length() << std::endl;

    size_t xPos = secondLine.find('x');
    std::cout << "Position of 'x' in second line: " << xPos << std::endl;
}
