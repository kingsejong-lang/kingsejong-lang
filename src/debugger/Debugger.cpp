/**
 * @file Debugger.cpp
 * @brief KingSejong 언어 대화형 디버거 구현
 * @author KingSejong Team
 * @date 2025-11-14
 */

#include "debugger/Debugger.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include <sstream>
#include <iomanip>

namespace kingsejong {
namespace debugger {

// ============================================================================
// Public Methods
// ============================================================================

Debugger::Debugger()
    : breakpoints_(std::make_unique<BreakpointManager>())
    , callStack_(std::make_unique<CallStack>())
    , watchpoints_(std::make_unique<WatchpointManager>())
    , parser_(std::make_unique<CommandParser>())
    , sourceViewer_(std::make_unique<SourceCodeViewer>())
    , state_(DebuggerState::IDLE)
    , stepOverDepth_(0)
{
}

void Debugger::step()
{
    state_ = DebuggerState::STEPPING;
}

void Debugger::next()
{
    state_ = DebuggerState::STEPPING_OVER;
    stepOverDepth_ = callStack_->depth();
}

void Debugger::continueExecution()
{
    state_ = DebuggerState::RUNNING;
}

void Debugger::pause()
{
    state_ = DebuggerState::PAUSED;
}

bool Debugger::shouldPause(const error::SourceLocation& location, const evaluator::Environment& env)
{
    // 1. 브레이크포인트 체크
    if (breakpoints_->shouldBreak(location, env)) {
        state_ = DebuggerState::PAUSED;
        return true;
    }

    // 2. STEPPING 상태 - 항상 멈춤
    if (state_ == DebuggerState::STEPPING) {
        state_ = DebuggerState::PAUSED;
        return true;
    }

    // 3. STEPPING_OVER 상태 - 스택 깊이 체크
    if (state_ == DebuggerState::STEPPING_OVER) {
        // 현재 스택 깊이가 step over 시작 깊이 이하면 멈춤
        // (함수에서 돌아왔거나 같은 레벨)
        if (callStack_->depth() <= stepOverDepth_) {
            state_ = DebuggerState::PAUSED;
            return true;
        }
    }

    // 4. 와치포인트 체크
    auto changedVars = watchpoints_->checkChanges(env);
    if (!changedVars.empty()) {
        state_ = DebuggerState::PAUSED;
        return true;
    }

    return false;
}

void Debugger::repl(
    ast::Program* program,
    std::shared_ptr<evaluator::Environment> env,
    std::istream& input,
    std::ostream& output)
{
    (void)program;  // 현재 사용하지 않음 - 향후 프로그램 실행 기능에 사용 예정

    output << "KingSejong Debugger v0.1.0\n";
    output << "명령어 도움말: 'help' 또는 'h'\n";
    output << "종료: 'quit' 또는 'q'\n\n";

    std::string line;
    bool running = true;

    while (running) {
        output << "(ksj-debug) ";
        output.flush();

        if (!std::getline(input, line)) {
            break;  // EOF
        }

        // 빈 줄 무시
        if (line.empty() || line.find_first_not_of(" \t") == std::string::npos) {
            continue;
        }

        try {
            // 명령어 파싱
            Command cmd = parser_->parse(line);

            // 명령어 실행
            running = handleCommand(cmd, env, output);

        } catch (const std::exception& e) {
            output << "오류: " << e.what() << "\n";
        }
    }

    output << "디버거를 종료합니다.\n";
}

// ============================================================================
// Private Methods
// ============================================================================

bool Debugger::handleCommand(
    const Command& cmd,
    std::shared_ptr<evaluator::Environment> env,
    std::ostream& output)
{
    switch (cmd.type) {
        case CommandType::BREAK: {
            if (cmd.args.empty()) {
                output << "사용법: break <file>:<line> [condition]\n";
                break;
            }

            // file:line 파싱
            std::string locStr = cmd.args[0];
            size_t colonPos = locStr.find(':');
            if (colonPos == std::string::npos) {
                output << "잘못된 형식: <file>:<line>\n";
                break;
            }

            std::string file = locStr.substr(0, colonPos);
            int line = std::stoi(locStr.substr(colonPos + 1));

            error::SourceLocation loc(file, line, 1);

            // 조건식이 있으면 추가
            if (cmd.args.size() > 1) {
                breakpoints_->add(loc, cmd.args[1]);
                output << "조건부 브레이크포인트 설정: " << file << ":" << line
                       << " (조건: " << cmd.args[1] << ")\n";
            } else {
                breakpoints_->add(loc);
                output << "브레이크포인트 설정: " << file << ":" << line << "\n";
            }
            break;
        }

        case CommandType::DELETE: {
            if (cmd.args.empty()) {
                output << "사용법: delete <id>\n";
                output << "현재 브레이크포인트 목록:\n";
                int id = 1;
                for (const auto& bp : breakpoints_->getAll()) {
                    output << "  " << id++ << ": " << bp.first.filename
                           << ":" << bp.first.line << "\n";
                }
                break;
            }

            // 모든 브레이크포인트 삭제
            if (cmd.args[0] == "all") {
                breakpoints_->clear();
                output << "모든 브레이크포인트를 삭제했습니다.\n";
            } else {
                output << "ID로 브레이크포인트 삭제는 아직 구현되지 않았습니다.\n";
                output << "'delete all'을 사용하여 모두 삭제할 수 있습니다.\n";
            }
            break;
        }

        case CommandType::STEP:
            step();
            output << "단계 실행 모드 (step into)\n";
            printStatus(output);
            break;

        case CommandType::NEXT:
            next();
            output << "다음 줄 실행 모드 (step over)\n";
            printStatus(output);
            break;

        case CommandType::CONTINUE:
            continueExecution();
            output << "계속 실행 중...\n";
            printStatus(output);
            break;

        case CommandType::PRINT: {
            if (cmd.args.empty()) {
                output << "사용법: print <expression>\n";
                break;
            }

            try {
                // 표현식 파싱 및 평가
                lexer::Lexer lexer(cmd.args[0]);
                parser::Parser parser(lexer);
                auto prog = parser.parseProgram();

                if (!parser.errors().empty() || !prog || prog->statements().empty()) {
                    output << "표현식 파싱 오류\n";
                    break;
                }

                auto* exprStmt = dynamic_cast<ast::ExpressionStatement*>(
                    prog->statements()[0].get()
                );

                if (!exprStmt || !exprStmt->expression()) {
                    output << "유효한 표현식이 아닙니다\n";
                    break;
                }

                evaluator::Evaluator evaluator(env);
                evaluator::Value result = evaluator.evalExpression(
                    const_cast<ast::Expression*>(exprStmt->expression())
                );

                output << cmd.args[0] << " = " << result.toString() << "\n";

            } catch (const std::exception& e) {
                output << "평가 오류: " << e.what() << "\n";
            }
            break;
        }

        case CommandType::BACKTRACE: {
            output << "호출 스택:\n";
            const auto& frames = callStack_->getAll();
            if (frames.empty()) {
                output << "  (비어있음)\n";
            } else {
                for (size_t i = 0; i < frames.size(); i++) {
                    output << "  #" << i << " " << frames[i].functionName
                           << " at " << frames[i].location.filename
                           << ":" << frames[i].location.line << "\n";
                }
            }
            break;
        }

        case CommandType::LIST: {
            int line = sourceViewer_->getCurrentLine();

            // 인자가 있으면 해당 라인 주변 표시
            if (!cmd.args.empty()) {
                line = std::stoi(cmd.args[0]);
            }

            if (line == 0) {
                output << "소스 코드가 로드되지 않았습니다.\n";
                break;
            }

            std::string formatted = sourceViewer_->format(line, 5);
            if (formatted.empty()) {
                output << "소스 코드를 표시할 수 없습니다.\n";
            } else {
                output << formatted << "\n";
            }
            break;
        }

        case CommandType::WATCH: {
            if (cmd.args.empty()) {
                output << "사용법: watch <variable>\n";
                break;
            }

            bool added = watchpoints_->add(cmd.args[0], *env);
            if (added) {
                output << "와치포인트 설정: " << cmd.args[0] << "\n";
            } else {
                output << "와치포인트 설정 실패 (변수가 없거나 이미 존재)\n";
            }
            break;
        }

        case CommandType::UNWATCH: {
            if (cmd.args.empty()) {
                output << "사용법: unwatch <variable>\n";
                break;
            }

            bool removed = watchpoints_->remove(cmd.args[0]);
            if (removed) {
                output << "와치포인트 제거: " << cmd.args[0] << "\n";
            } else {
                output << "와치포인트를 찾을 수 없습니다: " << cmd.args[0] << "\n";
            }
            break;
        }

        case CommandType::HELP:
            printHelp(output);
            break;

        case CommandType::QUIT:
            return false;  // 종료

        case CommandType::RUN:
            output << "run 명령어는 아직 구현되지 않았습니다.\n";
            break;

        case CommandType::UNKNOWN:
            output << "알 수 없는 명령어입니다. 'help'를 입력하여 도움말을 확인하세요.\n";
            break;
    }

    return true;  // 계속 실행
}

void Debugger::printHelp(std::ostream& output) const
{
    output << "\n=== KingSejong Debugger 명령어 ===\n\n";
    output << "실행 제어:\n";
    output << "  break, b <file>:<line> [condition] - 브레이크포인트 설정\n";
    output << "  delete, d <id>                     - 브레이크포인트 삭제\n";
    output << "  step, s                            - 단계 실행 (함수 내부 진입)\n";
    output << "  next, n                            - 다음 줄 실행 (함수 건너뛰기)\n";
    output << "  continue, c                        - 계속 실행\n\n";

    output << "검사:\n";
    output << "  print, p <expression>              - 표현식 평가 및 출력\n";
    output << "  backtrace, bt                      - 호출 스택 출력\n";
    output << "  list, l [line]                     - 소스 코드 표시\n";
    output << "  watch, w <variable>                - 와치포인트 설정\n";
    output << "  unwatch, uw <variable>             - 와치포인트 제거\n\n";

    output << "기타:\n";
    output << "  help, h                            - 이 도움말 표시\n";
    output << "  quit, q                            - 디버거 종료\n\n";
}

void Debugger::printStatus(std::ostream& output) const
{
    output << "상태: ";
    switch (state_) {
        case DebuggerState::IDLE:
            output << "대기";
            break;
        case DebuggerState::RUNNING:
            output << "실행 중";
            break;
        case DebuggerState::STEPPING:
            output << "단계 실행";
            break;
        case DebuggerState::STEPPING_OVER:
            output << "다음 줄 실행";
            break;
        case DebuggerState::PAUSED:
            output << "일시 정지";
            break;
    }
    output << "\n";

    output << "브레이크포인트: " << breakpoints_->getAll().size() << "개\n";
    output << "와치포인트: " << watchpoints_->getAll().size() << "개\n";
    output << "호출 스택 깊이: " << callStack_->depth() << "\n";
}

} // namespace debugger
} // namespace kingsejong
