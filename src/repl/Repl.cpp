/**
 * @file Repl.cpp
 * @brief KingSejong 대화형 실행 환경 구현
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include "Repl.h"
#include "../evaluator/Builtin.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace kingsejong {
namespace repl {

// ============================================================================
// 메인 REPL 루프
// ============================================================================

void Repl::start()
{
    // 전역 환경 생성
    env_ = std::make_shared<evaluator::Environment>();

    // 내장 함수 등록
    evaluator::Builtin::registerAllBuiltins();

    // 환영 메시지
    printWelcome();

    // REPL 루프
    while (true)
    {
        printPrompt();
        std::string input = readInput();

        // EOF (Ctrl+D)
        if (input.empty())
        {
            std::cout << "\n안녕히 가세요!\n";
            break;
        }

        // 특수 명령 처리
        if (handleCommand(input))
        {
            continue;
        }

        // 평가 및 출력
        evalAndPrint(input);
    }
}

// ============================================================================
// 입력 처리
// ============================================================================

void Repl::printPrompt(bool continuation)
{
    if (continuation)
    {
        std::cout << "... ";
    }
    else
    {
        std::cout << ">>> ";
    }
    std::cout.flush();
}

std::string Repl::readInput()
{
    std::string line;
    std::string input;

    while (true)
    {
        // 한 줄 읽기
        if (!std::getline(std::cin, line))
        {
            // EOF (Ctrl+D)
            return "";
        }

        input += line + "\n";

        // 입력이 완전한지 확인
        if (isComplete(input))
        {
            break;
        }

        // 계속 입력 받기
        printPrompt(true);
    }

    return input;
}

bool Repl::isComplete(const std::string& input)
{
    int braceCount = 0;      // {}
    int parenCount = 0;      // ()
    int bracketCount = 0;    // []
    bool inString = false;   // 문자열 내부 여부

    for (size_t i = 0; i < input.length(); ++i)
    {
        char ch = input[i];

        // 문자열 시작/끝
        if (ch == '"' && (i == 0 || input[i-1] != '\\'))
        {
            inString = !inString;
            continue;
        }

        // 문자열 내부는 무시
        if (inString)
        {
            continue;
        }

        // 괄호 카운트
        if (ch == '{') braceCount++;
        else if (ch == '}') braceCount--;
        else if (ch == '(') parenCount++;
        else if (ch == ')') parenCount--;
        else if (ch == '[') bracketCount++;
        else if (ch == ']') bracketCount--;
    }

    // 모든 괄호가 매칭되어야 완전함
    return braceCount == 0 && parenCount == 0 && bracketCount == 0;
}

// ============================================================================
// 평가 및 출력
// ============================================================================

void Repl::evalAndPrint(const std::string& input)
{
    try
    {
        // Lexer
        lexer::Lexer lexer(input);

        // Parser
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();

        // 파서 에러 확인
        if (!parser.errors().empty())
        {
            for (const auto& err : parser.errors())
            {
                std::cerr << "파서 에러: " << err << "\n";
            }
            return;
        }

        // Evaluator
        evaluator::Evaluator evaluator(env_);
        auto result = evaluator.evalProgram(program.get());

        // 결과 출력 (null이 아니면)
        if (!result.isNull())
        {
            std::cout << result.toString() << "\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "에러: " << e.what() << "\n";
    }
}

// ============================================================================
// 특수 명령 처리
// ============================================================================

bool Repl::handleCommand(const std::string& input)
{
    // 공백 제거
    std::string trimmed = input;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

    // .exit, .quit
    if (trimmed == ".exit" || trimmed == ".quit" || trimmed == ".종료")
    {
        std::cout << "안녕히 가세요!\n";
        exit(0);
    }

    // .help, .도움말
    if (trimmed == ".help" || trimmed == ".도움말")
    {
        printHelp();
        return true;
    }

    // .clear, .초기화
    if (trimmed == ".clear" || trimmed == ".초기화")
    {
        env_->clear();
        std::cout << "모든 변수가 초기화되었습니다.\n";
        return true;
    }

    // .vars, .변수
    if (trimmed == ".vars" || trimmed == ".변수")
    {
        printVariables();
        return true;
    }

    return false;
}

// ============================================================================
// UI 출력
// ============================================================================

void Repl::printWelcome()
{
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│   KingSejong 언어 v0.1.0                │\n";
    std::cout << "│   대화형 실행 환경 (REPL)                │\n";
    std::cout << "│                                         │\n";
    std::cout << "│   도움말: .help 또는 .도움말             │\n";
    std::cout << "│   종료: .exit 또는 Ctrl+D               │\n";
    std::cout << "└─────────────────────────────────────────┘\n";
    std::cout << "\n";
}

void Repl::printHelp()
{
    std::cout << "\nKingSejong REPL 명령어:\n\n";
    std::cout << "  .exit, .quit, .종료    - REPL 종료\n";
    std::cout << "  .help, .도움말         - 이 도움말 표시\n";
    std::cout << "  .clear, .초기화        - 모든 변수 초기화\n";
    std::cout << "  .vars, .변수           - 정의된 변수 목록\n";
    std::cout << "\n키보드 단축키:\n\n";
    std::cout << "  Ctrl+D                - REPL 종료 (EOF)\n";
    std::cout << "  Ctrl+C                - 현재 입력 취소\n";
    std::cout << "\n";
}

void Repl::printVariables()
{
    auto keys = env_->keys();

    if (keys.empty())
    {
        std::cout << "정의된 변수가 없습니다.\n";
        return;
    }

    std::cout << "\n정의된 변수:\n\n";

    // 알파벳 순으로 정렬
    std::sort(keys.begin(), keys.end());

    for (const auto& key : keys)
    {
        auto value = env_->get(key);
        std::cout << "  " << key << " = " << value.toString() << "\n";
    }

    std::cout << "\n";
}

} // namespace repl
} // namespace kingsejong
