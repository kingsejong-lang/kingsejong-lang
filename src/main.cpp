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
#include <cstring>
#include "repl/Repl.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Environment.h"
#include "evaluator/Builtin.h"
#include "lsp/LanguageServer.h"
#include "lsp/JsonRpc.h"
#include "error/ErrorReporter.h"
#include "module/ModuleLoader.h"

/**
 * @brief 파일을 읽고 실행
 * @param filename 실행할 파일 경로
 * @return 종료 코드 (0: 성공, 1: 실패)
 */
int executeFile(const std::string& filename)
{
    // ErrorReporter 생성 및 기본 힌트 등록
    kingsejong::error::ErrorReporter errorReporter;
    errorReporter.registerDefaultHints();

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

    // 소스 코드 등록 (에러 메시지에서 사용)
    errorReporter.registerSource(filename, source);

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
            // ErrorReporter로 각 에러를 출력
            for (const auto& err : parser.errors())
            {
                auto error = kingsejong::error::ParserError(err);
                errorReporter.report(error);
            }
            return 1;
        }

        // 4. 환경 생성
        auto env = std::make_shared<kingsejong::evaluator::Environment>();

        // 5. ModuleLoader 생성 및 설정
        auto moduleLoader = std::make_shared<kingsejong::module::ModuleLoader>(".");

        // 6. Evaluator
        kingsejong::evaluator::Evaluator evaluator(env);
        evaluator.setModuleLoader(moduleLoader.get());
        evaluator.evalProgram(program.get());

        return 0;
    }
    catch (const kingsejong::error::KingSejongError& e)
    {
        // KingSejong 에러는 ErrorReporter로 출력
        errorReporter.report(e);
        return 1;
    }
    catch (const std::exception& e)
    {
        // 기타 예외는 RuntimeError로 변환
        auto error = kingsejong::error::RuntimeError(e.what());
        errorReporter.report(error);
        return 1;
    }
}

/**
 * @brief LSP 서버 모드 실행
 * @return 종료 코드 (0: 성공, 1: 실패)
 */
int runLspServer()
{
    using namespace kingsejong::lsp;

    try
    {
        LanguageServer server;
        JsonRpc jsonRpc;

        // stdin에서 메시지를 읽고 stdout으로 응답 전송
        while (true)
        {
            try
            {
                // JSON-RPC 메시지 읽기 (Content-Length 헤더 포함)
                auto request = jsonRpc.readMessage(std::cin);

                // 요청 처리
                auto response = server.handleRequest(request);

                // 응답 전송 (notification이면 빈 응답)
                if (!response.is_null())
                {
                    jsonRpc.writeMessage(std::cout, response);
                }

                // exit 메서드 처리 시 종료
                if (request.contains("method") && request["method"] == "exit")
                {
                    break;
                }
            }
            catch (const std::exception& e)
            {
                // 메시지 파싱 실패 시 에러 응답
                std::cerr << "LSP Error: " << e.what() << "\n";
                // EOF 또는 치명적 에러 시 종료
                if (std::cin.eof())
                {
                    break;
                }
            }
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "LSP Server Error: " << e.what() << "\n";
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

    // --lsp 플래그 확인
    if (argc == 2 && std::strcmp(argv[1], "--lsp") == 0)
    {
        return runLspServer();
    }

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
        std::cerr << "  kingsejong --lsp    - LSP 서버 모드\n";
        return 1;
    }
}
