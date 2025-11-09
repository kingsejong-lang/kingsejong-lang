/**
 * @file main.cpp
 * @brief KingSejong 언어 실행 파일
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "repl/Repl.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Environment.h"
#include "evaluator/Builtin.h"

/**
 * @brief 파일을 읽고 실행
 * @param filename 실행할 파일 경로
 * @return 종료 코드 (0: 성공, 1: 실패)
 */
int executeFile(const std::string& filename)
{
    // 1. 파일 읽기
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "에러: 파일을 찾을 수 없습니다: " << filename << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    try
    {
        // 2. Lexer
        kingsejong::lexer::Lexer lexer(source);

        // 3. Parser
        kingsejong::parser::Parser parser(lexer);
        auto program = parser.parseProgram();

        // 파서 에러 확인
        if (!parser.errors().empty())
        {
            std::cerr << "파서 에러:\n";
            for (const auto& err : parser.errors())
            {
                std::cerr << "  " << err << "\n";
            }
            std::cerr << "파일: " << filename << "\n";
            return 1;
        }

        // 4. 환경 생성
        auto env = std::make_shared<kingsejong::evaluator::Environment>();

        // 5. Evaluator
        kingsejong::evaluator::Evaluator evaluator(env);
        evaluator.evalProgram(program.get());

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "에러: " << e.what() << "\n";
        std::cerr << "파일: " << filename << "\n";
        return 1;
    }
}

/**
 * @brief 프로그램 진입점
 * @param argc 인자 개수
 * @param argv 인자 배열
 * @return 종료 코드 (0: 성공, 1: 실패)
 */
int main(int argc, char* argv[])
{
    // 내장 함수 등록
    kingsejong::evaluator::Builtin::registerAllBuiltins();

    if (argc == 1)
    {
        // REPL 모드
        kingsejong::repl::Repl repl;
        repl.start();
        return 0;
    }
    else if (argc == 2)
    {
        // 파일 실행 모드
        std::string filename = argv[1];
        return executeFile(filename);
    }
    else
    {
        // 잘못된 사용법
        std::cerr << "사용법: kingsejong [파일명]\n";
        std::cerr << "\n";
        std::cerr << "  kingsejong          - REPL 모드로 실행\n";
        std::cerr << "  kingsejong 파일.ksj  - 파일 실행\n";
        return 1;
    }
}
