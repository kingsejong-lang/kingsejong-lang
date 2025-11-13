/**
 * @file CommandParser.cpp
 * @brief 디버거 명령어 파서 구현
 * @author KingSejong Team
 * @date 2025-11-14
 */

#include "debugger/CommandParser.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace kingsejong {
namespace debugger {

// ============================================================================
// Public Methods
// ============================================================================

Command CommandParser::parse(const std::string& input)
{
    // 입력 trim
    std::string trimmedInput = trim(input);

    // 빈 문자열 검증
    if (trimmedInput.empty()) {
        throw std::invalid_argument("CommandParser::parse() - input is empty");
    }

    // 토큰화
    std::vector<std::string> tokens = tokenize(trimmedInput);

    if (tokens.empty()) {
        throw std::invalid_argument("CommandParser::parse() - no tokens found");
    }

    // 첫 번째 토큰을 명령어로 인식
    std::string commandName = tokens[0];

    // 명령어 타입 결정
    CommandType type = getCommandType(commandName);

    // 나머지 토큰을 인자로 처리
    std::vector<std::string> args;

    if (tokens.size() > 1) {
        // 특수 처리: break 명령어의 경우 조건식을 하나의 인자로 결합
        if (type == CommandType::BREAK && tokens.size() > 2) {
            args.push_back(tokens[1]);  // file:line

            // 나머지를 조건식으로 결합
            std::string condition;
            for (size_t i = 2; i < tokens.size(); i++) {
                if (i > 2) condition += " ";
                condition += tokens[i];
            }
            args.push_back(condition);
        }
        // 특수 처리: print 명령어의 경우 표현식을 하나의 인자로 결합
        else if (type == CommandType::PRINT) {
            std::string expression;
            for (size_t i = 1; i < tokens.size(); i++) {
                if (i > 1) expression += " ";
                expression += tokens[i];
            }
            args.push_back(expression);
        }
        // 일반 처리: 각 토큰을 개별 인자로
        else {
            args.assign(tokens.begin() + 1, tokens.end());
        }
    }

    return Command(type, args);
}

// ============================================================================
// Private Methods
// ============================================================================

std::vector<std::string> CommandParser::tokenize(const std::string& input)
{
    std::vector<std::string> tokens;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

CommandType CommandParser::getCommandType(const std::string& commandName) const
{
    // 소문자로 변환
    std::string lowerName = commandName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // 명령어 매칭 (full name과 alias 모두 지원)
    if (lowerName == "break" || lowerName == "b") {
        return CommandType::BREAK;
    } else if (lowerName == "delete" || lowerName == "d") {
        return CommandType::DELETE;
    } else if (lowerName == "run" || lowerName == "r") {
        return CommandType::RUN;
    } else if (lowerName == "continue" || lowerName == "c") {
        return CommandType::CONTINUE;
    } else if (lowerName == "step" || lowerName == "s") {
        return CommandType::STEP;
    } else if (lowerName == "next" || lowerName == "n") {
        return CommandType::NEXT;
    } else if (lowerName == "print" || lowerName == "p") {
        return CommandType::PRINT;
    } else if (lowerName == "backtrace" || lowerName == "bt") {
        return CommandType::BACKTRACE;
    } else if (lowerName == "list" || lowerName == "l") {
        return CommandType::LIST;
    } else if (lowerName == "watch" || lowerName == "w") {
        return CommandType::WATCH;
    } else if (lowerName == "unwatch" || lowerName == "uw") {
        return CommandType::UNWATCH;
    } else if (lowerName == "help" || lowerName == "h") {
        return CommandType::HELP;
    } else if (lowerName == "quit" || lowerName == "q") {
        return CommandType::QUIT;
    } else {
        return CommandType::UNKNOWN;
    }
}

std::string CommandParser::trim(const std::string& str) const
{
    // 앞쪽 공백 찾기
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }

    // 뒤쪽 공백 찾기
    size_t end = str.size();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        end--;
    }

    return str.substr(start, end - start);
}

} // namespace debugger
} // namespace kingsejong
