/**
 * @file CommandParser.h
 * @brief 디버거 명령어 파서
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * 디버거 REPL에서 사용자 입력을 파싱하여 명령어로 변환합니다.
 */

#ifndef KINGSEJONG_DEBUGGER_COMMANDPARSER_H
#define KINGSEJONG_DEBUGGER_COMMANDPARSER_H

#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace kingsejong {
namespace debugger {

/**
 * @enum CommandType
 * @brief 디버거 명령어 타입
 */
enum class CommandType {
    BREAK,      ///< 브레이크포인트 설정 (break, b)
    DELETE,     ///< 브레이크포인트 삭제 (delete, d)
    RUN,        ///< 프로그램 실행 (run, r)
    CONTINUE,   ///< 실행 계속 (continue, c)
    STEP,       ///< 단계 실행 (step into) (step, s)
    NEXT,       ///< 다음 줄 실행 (step over) (next, n)
    PRINT,      ///< 변수 출력 (print, p)
    BACKTRACE,  ///< 호출 스택 출력 (backtrace, bt)
    LIST,       ///< 소스 코드 표시 (list, l)
    WATCH,      ///< 와치포인트 설정 (watch, w)
    UNWATCH,    ///< 와치포인트 삭제 (unwatch, uw)
    HELP,       ///< 도움말 (help, h)
    QUIT,       ///< 종료 (quit, q)
    UNKNOWN     ///< 알 수 없는 명령어
};

/**
 * @struct Command
 * @brief 파싱된 디버거 명령어
 */
struct Command {
    CommandType type;                    ///< 명령어 타입
    std::vector<std::string> args;       ///< 명령어 인자

    /**
     * @brief Command 생성자
     * @param type 명령어 타입
     * @param args 명령어 인자 (기본값: 빈 벡터)
     */
    Command(CommandType type, const std::vector<std::string>& args = {})
        : type(type), args(args) {}
};

/**
 * @class CommandParser
 * @brief 디버거 명령어 파서
 *
 * 사용자 입력 문자열을 파싱하여 Command 객체로 변환합니다.
 *
 * @example
 * @code
 * CommandParser parser;
 * Command cmd = parser.parse("break main.ksj:10");
 * if (cmd.type == CommandType::BREAK) {
 *     // 브레이크포인트 설정
 * }
 * @endcode
 */
class CommandParser {
public:
    /**
     * @brief 생성자
     */
    CommandParser() = default;

    /**
     * @brief 소멸자
     */
    ~CommandParser() = default;

    // RAII: Rule of Five
    CommandParser(const CommandParser&) = delete;
    CommandParser& operator=(const CommandParser&) = delete;
    CommandParser(CommandParser&&) = default;
    CommandParser& operator=(CommandParser&&) = default;

    /**
     * @brief 명령어 문자열 파싱
     * @param input 사용자 입력 문자열
     * @return 파싱된 Command 객체
     * @throws std::invalid_argument 입력이 빈 문자열인 경우
     *
     * 지원 명령어:
     * - break <file>:<line> [condition]  : 브레이크포인트 설정
     * - delete <id>                      : 브레이크포인트 삭제
     * - run                              : 프로그램 실행
     * - continue                         : 실행 계속
     * - step                             : 단계 실행 (함수 내부 진입)
     * - next                             : 다음 줄 실행 (함수 건너뛰기)
     * - print <expr>                     : 변수/표현식 출력
     * - backtrace                        : 호출 스택 출력
     * - list [line]                      : 소스 코드 표시
     * - watch <variable>                 : 와치포인트 설정
     * - unwatch <variable>               : 와치포인트 삭제
     * - help                             : 도움말
     * - quit                             : 종료
     *
     * 별칭 지원:
     * - b = break, d = delete, r = run, c = continue
     * - s = step, n = next, p = print, bt = backtrace
     * - l = list, w = watch, uw = unwatch
     * - h = help, q = quit
     */
    Command parse(const std::string& input);

private:
    /**
     * @brief 입력 문자열을 토큰으로 분할
     * @param input 입력 문자열
     * @return 토큰 벡터
     */
    std::vector<std::string> tokenize(const std::string& input);

    /**
     * @brief 명령어 이름을 CommandType으로 변환
     * @param commandName 명령어 이름 (별칭 포함)
     * @return CommandType
     */
    CommandType getCommandType(const std::string& commandName) const;

    /**
     * @brief 문자열 trim (앞뒤 공백 제거)
     * @param str 입력 문자열
     * @return trim된 문자열
     */
    std::string trim(const std::string& str) const;
};

} // namespace debugger
} // namespace kingsejong

#endif // KINGSEJONG_DEBUGGER_COMMANDPARSER_H
