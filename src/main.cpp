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
#include "semantic/SemanticAnalyzer.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Environment.h"
#include "evaluator/Builtin.h"
#include "lsp/LanguageServer.h"
#include "lsp/JsonRpc.h"
#include "error/ErrorReporter.h"
#include "module/ModuleLoader.h"
#include "version/Version.h"
#include "formatter/Formatter.h"
#include "formatter/FormatterConfig.h"

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
        // 2. Lexer (파일명 포함)
        kingsejong::lexer::Lexer lexer(source, filename);

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

        // 4. Semantic Analyzer (의미 분석)
        kingsejong::semantic::SemanticAnalyzer semanticAnalyzer;
        bool semanticOk = semanticAnalyzer.analyze(program.get());

        // Semantic 에러 확인
        if (!semanticOk || !semanticAnalyzer.errors().empty())
        {
            for (const auto& err : semanticAnalyzer.errors())
            {
                std::cerr << "Semantic Error: " << err.toString() << "\n";
            }
            return 1;
        }

        // 5. 환경 생성
        auto env = std::make_shared<kingsejong::evaluator::Environment>();

        // 6. ModuleLoader 생성 및 설정
        auto moduleLoader = std::make_shared<kingsejong::module::ModuleLoader>(".");

        // 7. Evaluator
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
 * @brief 코드 포맷팅 실행
 * @param argc 인자 개수
 * @param argv 인자 배열
 * @return 종료 코드 (0: 성공, 1: 실패)
 */
int runFormatter(int argc, char* argv[])
{
    std::string filename;
    std::string configFile;

    // 인자 파싱
    for (int i = 2; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--config" && i + 1 < argc)
        {
            configFile = argv[++i];
        }
        else if (arg[0] != '-')
        {
            filename = arg;
        }
    }

    if (filename.empty())
    {
        std::cerr << "에러: 포맷팅할 파일을 지정해주세요.\n";
        std::cerr << "사용법: kingsejong fmt [--config 설정파일] 파일명.ksj\n";
        return 1;
    }

    try
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

        // 2. Lexer & Parser
        kingsejong::lexer::Lexer lexer(source, filename);
        kingsejong::parser::Parser parser(lexer);
        auto program = parser.parseProgram();

        if (!parser.errors().empty())
        {
            std::cerr << "파싱 에러:\n";
            for (const auto& err : parser.errors())
            {
                std::cerr << "  " << err << "\n";
            }
            return 1;
        }

        // 3. Formatter 설정
        kingsejong::formatter::Formatter formatter;

        // 설정 파일이 지정되었으면 로드
        if (!configFile.empty())
        {
            if (!formatter.loadConfig(configFile))
            {
                std::cerr << "경고: 설정 파일을 로드할 수 없습니다: " << configFile << "\n";
                std::cerr << "기본 설정을 사용합니다.\n";
            }
        }
        else
        {
            // .ksjfmtrc 파일 자동 검색
            formatter.loadConfigFromCurrentDir();
        }

        // 4. 포맷팅 실행
        std::string formatted = formatter.format(program.get());

        // 5. 파일에 쓰기 (원본 덮어쓰기)
        std::ofstream outFile(filename);
        if (!outFile.is_open())
        {
            std::cerr << "에러: 파일을 쓸 수 없습니다: " << filename << "\n";
            return 1;
        }

        outFile << formatted;
        outFile.close();

        std::cout << "포맷팅 완료: " << filename << "\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "에러: " << e.what() << "\n";
        return 1;
    }
}

/**
 * @brief 도움말 출력
 */
void printHelp()
{
    std::cout << "사용법: kingsejong [옵션] [파일명]\n";
    std::cout << "\n";
    std::cout << "옵션:\n";
    std::cout << "  -h, --help      이 도움말 메시지 출력\n";
    std::cout << "  -v, --version   버전 정보 출력\n";
    std::cout << "  --lsp           LSP 서버 모드로 실행\n";
    std::cout << "\n";
    std::cout << "서브커맨드:\n";
    std::cout << "  fmt             코드 포맷팅\n";
    std::cout << "\n";
    std::cout << "사용 예시:\n";
    std::cout << "  kingsejong                    REPL 모드로 실행\n";
    std::cout << "  kingsejong 파일.ksj            파일 실행\n";
    std::cout << "  kingsejong --version          버전 정보 출력\n";
    std::cout << "  kingsejong --lsp              LSP 서버 모드\n";
    std::cout << "  kingsejong fmt 파일.ksj        코드 포맷팅\n";
    std::cout << "  kingsejong fmt --config .ksjfmtrc 파일.ksj\n";
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

    // 인자가 1개 이상인 경우 옵션 확인
    if (argc >= 2)
    {
        std::string arg = argv[1];

        // --version 또는 -v
        if (arg == "--version" || arg == "-v")
        {
            std::cout << kingsejong::version::getFullVersionString() << "\n";
            return 0;
        }

        // --help 또는 -h
        if (arg == "--help" || arg == "-h")
        {
            printHelp();
            return 0;
        }

        // --lsp 플래그
        if (arg == "--lsp")
        {
            return runLspServer();
        }

        // fmt 서브커맨드
        if (arg == "fmt")
        {
            return runFormatter(argc, argv);
        }
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
        std::cerr << "에러: 잘못된 인자입니다.\n\n";
        printHelp();
        return 1;
    }
}
