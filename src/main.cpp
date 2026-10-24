/**
 * @file main.cpp
 * @brief KingSejong 언어 실행 파일
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include "repl/Repl.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "semantic/SemanticAnalyzer.h"
#include "evaluator/Evaluator.h"
#include "runtime/Environment.h"
#include "evaluator/Builtin.h"
#include "lsp/LanguageServer.h"
#include "lsp/JsonRpc.h"
#include "error/ErrorReporter.h"
#include "module/ModuleLoader.h"
#include "version/Version.h"
#include "security/SecurityManager.h"
#include "bytecode/Chunk.h"
#include "bytecode/VM.h"
#include "bytecode/Compiler.h"
#include "debugger/Debugger.h"
#include "linter/Linter.h"
#include "formatter/Formatter.h"
#include "formatter/FormatterConfig.h"

/// 실행 파일 위치에서 유도한 모듈 검색 경로 (main 이 채운다)
static std::vector<std::string> g_moduleSearchPaths;

/**
 * @brief 실행 파일 위치를 기준으로 stdlib 를 찾을 수 있는 디렉터리를 모은다.
 *
 * build/bin/kingsejong 처럼 빌드 트리에서 실행하면 저장소 루트(../..)에 stdlib/ 가 있고,
 * 설치본은 <prefix>/bin/kingsejong 옆의 ../share/kingsejong 에 있다. argv[0] 이 PATH 로
 * 찾은 이름이면 디렉터리를 알 수 없으므로 아무것도 추가하지 않는다.
 */
static void collectModuleSearchPaths(const char* argv0)
{
    namespace fs = std::filesystem;
    if (!argv0 || std::string(argv0).find('/') == std::string::npos)
    {
        return;
    }
    std::error_code ec;
    fs::path exe = fs::weakly_canonical(fs::absolute(argv0, ec), ec);
    if (ec)
    {
        return;
    }
    fs::path binDir = exe.parent_path();
    for (const auto& rel : {"../share/kingsejong", "..", "../.."})
    {
        fs::path candidate = (binDir / rel).lexically_normal();
        if (fs::exists(candidate / "stdlib", ec))
        {
            g_moduleSearchPaths.push_back(candidate.string());
        }
    }
}

/**
 * @brief 파일을 읽고 실행
 * @param filename 실행할 파일 경로
 * @return 종료 코드 (0: 성공, 1: 실패)
 */
int executeFile(const std::string& filename, bool useVm = false)
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
            // 위치가 있는 에러는 파일:줄:열 로 보여 준다
            for (const auto& err : parser.errorDetails())
            {
                std::string message = err.message;
                if (err.location.isValid())
                {
                    message = filename + ":" + std::to_string(err.location.line) + ":" +
                              std::to_string(err.location.column) + ": " + message;
                }
                auto error = kingsejong::error::ParserError(message);
                errorReporter.report(error);
            }
            return 1;
        }

        // 4. ModuleLoader 생성 및 설정 (의미 분석기도 모듈 경로를 알아야 한다)
        auto moduleLoader = std::make_shared<kingsejong::module::ModuleLoader>(".");
        for (const auto& dir : g_moduleSearchPaths)
        {
            moduleLoader->addSearchPath(dir);
        }

        // 5. Semantic Analyzer (의미 분석). 가져온 모듈의 이름을 알도록 경로 해석기를 넘긴다
        kingsejong::semantic::SemanticAnalyzer semanticAnalyzer;
        semanticAnalyzer.setModuleResolver(
            [&moduleLoader](const std::string& modulePath, const std::string& fromFile) {
                return moduleLoader->resolvePath(modulePath, fromFile);
            });
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


        // 6.5 --vm: 바이트코드 컴파일러와 VM 으로 실행. 값·변수 규칙을 평가기와 공유한다.
        if (useVm)
        {
            kingsejong::bytecode::Chunk chunk;
            kingsejong::bytecode::Compiler compiler;
            if (!compiler.compile(program.get(), &chunk))
            {
                std::cerr << "에러: 바이트코드 컴파일에 실패했습니다.\n";
                for (const auto& message : compiler.errors())
                {
                    std::cerr << "  " << message << "\n";
                }
                return 1;
            }
            kingsejong::bytecode::VM vm;
            vm.setModuleLoader(moduleLoader.get());
            auto result = vm.run(&chunk);
            if (result != kingsejong::bytecode::VMResult::OK && result != kingsejong::bytecode::VMResult::HALT)
            {
                return 1;
            }
            return 0;
        }

        // 7. Evaluator. 환경을 넘겨주어 평가기가 유일한 소유자가 되게 한다:
        // 그래야 소멸할 때 전역 함수·클래스가 만든 함수-환경 순환 참조를 끊을 수 있다.
        kingsejong::evaluator::Evaluator evaluator(std::move(env));
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

                // 처리 중 쌓인 서버 알림 (publishDiagnostics 등) 전송
                for (const auto& notification : server.takePendingNotifications())
                {
                    jsonRpc.writeMessage(std::cout, notification);
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

        // 포매터가 다루지 못한 노드가 있으면 원본을 건드리지 않는다.
        // 그대로 쓰면 그 노드의 코드가 파일에서 사라진다.
        if (formatter.hasUnsupportedNodes())
        {
            std::cerr << "에러: 포매터가 아직 지원하지 않는 구문이 있어 파일을 수정하지 않았습니다: " << filename << "\n";
            for (const auto& node : formatter.unsupportedNodes())
            {
                std::cerr << "  - " << node << "\n";
            }
            return 1;
        }

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
 * @brief 린터 실행: kingsejong lint [--config 설정파일] 파일.ksj ...
 * @return 0 (오류 등급 이슈 없음) / 1 (오류 또는 실패)
 */
int runLinter(int argc, char* argv[])
{
    std::vector<std::string> files;
    std::string configFile;

    for (int i = 2; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--config" && i + 1 < argc)
        {
            configFile = argv[++i];
        }
        else if (!arg.empty() && arg[0] != '-')
        {
            files.push_back(arg);
        }
        else
        {
            std::cerr << "에러: 알 수 없는 옵션입니다: " << arg << "\n";
            return 1;
        }
    }

    if (files.empty())
    {
        std::cerr << "에러: 검사할 파일을 지정해주세요.\n";
        std::cerr << "사용법: kingsejong lint [--config 설정파일] 파일명.ksj ...\n";
        return 1;
    }

    size_t totalErrors = 0;
    size_t totalWarnings = 0;
    bool failed = false;

    for (const auto& filename : files)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "에러: 파일을 찾을 수 없습니다: " << filename << "\n";
            failed = true;
            continue;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();

        kingsejong::lexer::Lexer lexer(buffer.str(), filename);
        kingsejong::parser::Parser parser(lexer);
        auto program = parser.parseProgram();
        if (!parser.errors().empty())
        {
            for (const auto& err : parser.errorDetails())
            {
                std::cerr << filename << ":" << err.location.line << ":" << err.location.column
                          << ": 구문 오류: " << err.message << "\n";
            }
            failed = true;
            continue;
        }

        kingsejong::linter::Linter linter;
        linter.addDefaultRules();
        if (!configFile.empty())
        {
            if (!linter.loadConfig(configFile))
            {
                std::cerr << "경고: 설정 파일을 로드할 수 없습니다: " << configFile << "\n";
            }
        }
        else
        {
            linter.loadConfigFromCurrentDir();
        }

        linter.analyze(program.get(), filename);
        for (const auto& issue : linter.issues())
        {
            std::cout << issue.toString() << "\n";
        }
        totalErrors += linter.errorCount();
        totalWarnings += linter.warningCount();
    }

    std::cout << "검사 완료: 파일 " << files.size() << "개, 오류 " << totalErrors
              << "개, 경고 " << totalWarnings << "개\n";
    return (failed || totalErrors > 0) ? 1 : 0;
}

/**
 * @brief 디버거 모드: kingsejong --debug 파일.ksj
 *
 * 프로그램을 파싱한 뒤 디버거 프롬프트를 띄운다. break 로 중단점을 걸고 run 으로
 * 시작하면 평가기가 문장마다 디버거에 알리고, 중단점·step 에서 프롬프트로 돌아온다.
 */
int runDebugger(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "에러: 파일을 찾을 수 없습니다: " << filename << "\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();

    kingsejong::lexer::Lexer lexer(buffer.str(), filename);
    kingsejong::parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    if (!parser.errors().empty())
    {
        for (const auto& err : parser.errorDetails())
        {
            std::cerr << filename << ":" << err.location.line << ":" << err.location.column
                      << ": 구문 오류: " << err.message << "\n";
        }
        return 1;
    }

    auto env = std::make_shared<kingsejong::evaluator::Environment>();
    kingsejong::module::ModuleLoader moduleLoader(".");
    for (const auto& dir : g_moduleSearchPaths)
    {
        moduleLoader.addSearchPath(dir);
    }

    kingsejong::debugger::Debugger debugger;
    debugger.setAttached(true);
    debugger.getSourceViewer().loadFile(filename);

    std::cout << "KingSejong 디버거. 중단점: break " << filename << ":<줄>, 시작: run, 도움말: help\n";
    debugger.repl(program.get(), env);
    if (debugger.quitRequested())
    {
        return 0;
    }

    // 환경의 마지막 핸들을 평가기에 넘겨 소멸 시 순환 참조를 끊게 한다.
    kingsejong::evaluator::Evaluator evaluator(std::move(env));
    evaluator.setModuleLoader(&moduleLoader);
    evaluator.setObserver(&debugger);
    try
    {
        evaluator.evalProgram(program.get());
        std::cout << "\n프로그램이 끝났습니다.\n";
    }
    catch (const kingsejong::debugger::DebuggerExit&)
    {
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "에러: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

/**
 * @brief 도움말 출력
 */
void printHelp()
{
    std::cout << "사용법: kingsejong [옵션] [파일명]\n";
    std::cout << "\n";
    std::cout << "옵션:\n";
    std::cout << "  -h, --help          이 도움말 메시지 출력\n";
    std::cout << "  -v, --version       버전 정보 출력\n";
    std::cout << "  --lsp               LSP 서버 모드로 실행\n";
    std::cout << "  --debug             디버거 모드로 파일 실행 (break/run/step/next/continue/print/backtrace)\n";
    std::cout << "  --vm                바이트코드 VM 으로 실행 (평가기와 같은 결과, 함수 호출이 많은 코드에서 빠름)\n";
    std::cout << "  --sandbox           샌드박스 모드: 현재 디렉터리와 --allow-path 경로만 파일 접근 허용\n";
    std::cout << "  --untrusted         비신뢰 모드: 시스템 경로(/etc, /sys, /proc ...) 접근 차단\n";
    std::cout << "  --allow-path 경로   샌드박스에서 추가로 허용할 경로 (여러 번 지정 가능)\n";
    std::cout << "\n";
    std::cout << "서브커맨드:\n";
    std::cout << "  fmt             코드 포맷팅\n";
    std::cout << "  lint            코드 검사 (규칙 12개, .ksjlint.json 으로 설정)\n";
    std::cout << "\n";
    std::cout << "사용 예시:\n";
    std::cout << "  kingsejong                    REPL 모드로 실행\n";
    std::cout << "  kingsejong 파일.ksj            파일 실행\n";
    std::cout << "  kingsejong --sandbox 파일.ksj  샌드박스에서 파일 실행\n";
    std::cout << "  kingsejong --version          버전 정보 출력\n";
    std::cout << "  kingsejong --lsp              LSP 서버 모드\n";
    std::cout << "  kingsejong fmt 파일.ksj        코드 포맷팅\n";
    std::cout << "  kingsejong fmt --config .ksjfmt.json 파일.ksj\n";
    std::cout << "  kingsejong lint 파일.ksj       코드 검사\n";
    std::cout << "  kingsejong --debug 파일.ksj    디버거로 실행\n";
}

/**
 * @brief 명령행에서 받은 보안 옵션을 SecurityManager 에 적용한다.
 * @param mode "" (기본, TRUSTED) | "sandbox" | "untrusted"
 * @param allowPaths --allow-path 로 받은 경로들
 *
 * 기본값은 TRUSTED 다. 플래그가 없으면 기존 동작과 같다.
 */
void applySecurityOptions(const std::string& mode, const std::vector<std::string>& allowPaths)
{
    using kingsejong::security::SecurityManager;
    using kingsejong::security::SecurityMode;

    if (mode == "sandbox")
    {
        SecurityManager::setMode(SecurityMode::SANDBOX);
        // 스크립트가 자기 디렉터리의 파일은 다룰 수 있어야 쓸모가 있다.
        SecurityManager::allowPath(std::filesystem::current_path().string());
    }
    else if (mode == "untrusted")
    {
        SecurityManager::setMode(SecurityMode::UNTRUSTED);
    }
    for (const auto& path : allowPaths)
    {
        SecurityManager::allowPath(path);
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
    collectModuleSearchPaths(argc > 0 ? argv[0] : nullptr);

    std::vector<std::string> args(argv + 1, argv + argc);

    // 서브커맨드는 자기 인자를 직접 해석한다.
    if (!args.empty() && args[0] == "fmt")
    {
        return runFormatter(argc, argv);
    }
    if (!args.empty() && args[0] == "lint")
    {
        return runLinter(argc, argv);
    }

    std::string filename;
    std::string securityMode;
    std::vector<std::string> allowPaths;
    bool debugMode = false;
    bool vmMode = false;

    for (size_t i = 0; i < args.size(); ++i)
    {
        const std::string& arg = args[i];

        if (arg == "--version" || arg == "-v")
        {
            std::cout << kingsejong::version::getFullVersionString() << "\n";
            return 0;
        }
        if (arg == "--help" || arg == "-h")
        {
            printHelp();
            return 0;
        }
        if (arg == "--lsp")
        {
            return runLspServer();
        }
        if (arg == "--debug")
        {
            debugMode = true;
        }
        else if (arg == "--vm")
        {
            vmMode = true;
        }
        else if (arg == "--sandbox")
        {
            securityMode = "sandbox";
        }
        else if (arg == "--untrusted")
        {
            securityMode = "untrusted";
        }
        else if (arg == "--allow-path")
        {
            if (i + 1 >= args.size())
            {
                std::cerr << "에러: --allow-path 뒤에 경로가 필요합니다.\n";
                return 1;
            }
            allowPaths.push_back(args[++i]);
        }
        else if (!arg.empty() && arg[0] == '-')
        {
            std::cerr << "에러: 알 수 없는 옵션입니다: " << arg << "\n\n";
            printHelp();
            return 1;
        }
        else if (filename.empty())
        {
            filename = arg;
        }
        else
        {
            std::cerr << "에러: 파일은 하나만 지정할 수 있습니다: " << arg << "\n\n";
            printHelp();
            return 1;
        }
    }

    applySecurityOptions(securityMode, allowPaths);

    if (filename.empty())
    {
        if (debugMode)
        {
            std::cerr << "에러: --debug 에는 실행할 파일이 필요합니다.\n";
            return 1;
        }
        // REPL 모드
        kingsejong::repl::Repl repl;
        repl.start();
        return 0;
    }
    if (debugMode)
    {
        return runDebugger(filename);
    }
    return executeFile(filename, vmMode);
}
