/**
 * @file ErrorReporter.cpp
 * @brief ErrorReporter 구현
 * @author KingSejong Team
 * @date 2025-11-11
 */

#include "ErrorReporter.h"
#include <sstream>
#include <algorithm>
#include <iostream>

namespace kingsejong {
namespace error {

// ============================================================================
// ANSI 컬러 유틸리티
// ============================================================================

std::string ansiColor(AnsiColor color)
{
    switch (color)
    {
        case AnsiColor::RESET:   return "\033[0m";
        case AnsiColor::RED:     return "\033[31m";
        case AnsiColor::GREEN:   return "\033[32m";
        case AnsiColor::YELLOW:  return "\033[33m";
        case AnsiColor::BLUE:    return "\033[34m";
        case AnsiColor::MAGENTA: return "\033[35m";
        case AnsiColor::CYAN:    return "\033[36m";
        case AnsiColor::BOLD:    return "\033[1m";
        case AnsiColor::DIM:     return "\033[2m";
        default:                 return "";
    }
}

std::string colorize(const std::string& text, AnsiColor color)
{
    return ansiColor(color) + text + ansiColor(AnsiColor::RESET);
}

std::string errorTypeToKorean(ErrorType type)
{
    switch (type)
    {
        case ErrorType::LEXER_ERROR:        return "어휘 오류";
        case ErrorType::PARSER_ERROR:       return "구문 오류";
        case ErrorType::RUNTIME_ERROR:      return "실행 오류";
        case ErrorType::TYPE_ERROR:         return "타입 오류";
        case ErrorType::NAME_ERROR:         return "이름 오류";
        case ErrorType::VALUE_ERROR:        return "값 오류";
        case ErrorType::ZERO_DIVISION_ERROR:return "0으로 나누기 오류";
        case ErrorType::INDEX_ERROR:        return "인덱스 오류";
        case ErrorType::ARGUMENT_ERROR:     return "인자 오류";
        default:                            return "알 수 없는 오류";
    }
}

// ============================================================================
// SourceManager 구현
// ============================================================================

SourceManager::SourceManager() = default;

void SourceManager::loadSource(const std::string& filename, const std::string& content)
{
    SourceFile sourceFile;
    sourceFile.filename = filename;

    // 소스 코드를 줄 단위로 분리
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line))
    {
        sourceFile.lines.push_back(line);
    }

    // 맵에 저장
    sources_[filename] = std::move(sourceFile);
}

std::optional<std::string> SourceManager::getLine(const std::string& filename, int line) const
{
    auto it = sources_.find(filename);
    if (it == sources_.end())
    {
        return std::nullopt;
    }

    const auto& sourceFile = it->second;

    // 줄 번호는 1부터 시작, 벡터는 0부터 시작
    int index = line - 1;
    if (index < 0 || index >= static_cast<int>(sourceFile.lines.size()))
    {
        return std::nullopt;
    }

    return sourceFile.lines[index];
}

std::vector<std::string> SourceManager::getContext(
    const std::string& filename,
    int line,
    int contextLines
) const
{
    auto it = sources_.find(filename);
    if (it == sources_.end())
    {
        return {};
    }

    const auto& sourceFile = it->second;
    std::vector<std::string> context;

    // 시작 줄과 끝 줄 계산 (1-based)
    int startLine = std::max(1, line - contextLines);
    int endLine = std::min(static_cast<int>(sourceFile.lines.size()), line + contextLines);

    // 컨텍스트 수집
    for (int i = startLine; i <= endLine; i++)
    {
        int index = i - 1;  // 0-based index
        if (index >= 0 && index < static_cast<int>(sourceFile.lines.size()))
        {
            context.push_back(sourceFile.lines[index]);
        }
    }

    return context;
}

// ============================================================================
// ErrorFormatter 구현
// ============================================================================

std::string ErrorFormatter::formatError(
    const KingSejongError& error,
    const SourceManager& sourceMgr,
    bool useColor
) const
{
    std::ostringstream oss;

    // 1. 위치 정보 (있는 경우)
    if (error.getLocation().has_value())
    {
        const auto& loc = error.getLocation().value();
        oss << formatLocation(loc, useColor) << "\n\n";

        // 2. 소스 코드 컨텍스트 (소스가 등록된 경우)
        auto context = sourceMgr.getContext(loc.filename, loc.line, 1);
        if (!context.empty())
        {
            int startLine = std::max(1, loc.line - 1);
            oss << formatContext(context, loc.line, loc.column, startLine, useColor) << "\n";
        }
    }

    // 3. 에러 타입 및 메시지
    oss << formatErrorMessage(error, useColor) << "\n";

    return oss.str();
}

std::string ErrorFormatter::formatLocation(const SourceLocation& loc, bool useColor) const
{
    std::ostringstream oss;

    std::string icon = "📍 ";
    std::string text = "오류 위치: " + loc.toString();

    if (useColor)
    {
        oss << icon << colorize(text, AnsiColor::MAGENTA);
    }
    else
    {
        oss << icon << text;
    }

    return oss.str();
}

std::string ErrorFormatter::formatContext(
    const std::vector<std::string>& lines,
    int errorLine,
    int errorCol,
    int startLine,
    bool useColor
) const
{
    std::ostringstream oss;

    // 각 줄을 출력
    for (size_t i = 0; i < lines.size(); i++)
    {
        int currentLine = startLine + static_cast<int>(i);
        std::string lineNumStr = std::to_string(currentLine) + " | ";

        if (useColor)
        {
            oss << colorize(lineNumStr, AnsiColor::DIM);
        }
        else
        {
            oss << lineNumStr;
        }

        oss << lines[i] << "\n";

        // 에러 발생 줄이면 화살표 표시
        if (currentLine == errorLine && errorCol > 0)
        {
            // 줄 번호 부분만큼 공백
            std::string indent(lineNumStr.length(), ' ');

            // 컬럼 위치까지 공백 (1-based column to 0-based)
            std::string spaces(errorCol - 1, ' ');

            std::string arrow = "^";

            if (useColor)
            {
                oss << indent << spaces << colorize(arrow, AnsiColor::RED) << "\n";
            }
            else
            {
                oss << indent << spaces << arrow << "\n";
            }
        }
    }

    return oss.str();
}

std::string ErrorFormatter::formatErrorMessage(const KingSejongError& error, bool useColor) const
{
    std::ostringstream oss;

    std::string icon = "❌ ";
    std::string typeStr = errorTypeToKorean(error.getType());
    std::string message = error.what();

    // 위치 정보가 메시지에 포함되어 있으면 제거 (중복 방지)
    if (error.getLocation().has_value())
    {
        const auto& loc = error.getLocation().value();
        std::string locPrefix = loc.toString() + ": ";
        if (message.find(locPrefix) == 0)
        {
            message = message.substr(locPrefix.length());
        }
    }

    if (useColor)
    {
        oss << icon << colorize(typeStr + ": " + message, AnsiColor::RED);
    }
    else
    {
        oss << icon << typeStr << ": " << message;
    }

    return oss.str();
}

std::string ErrorFormatter::formatHint(const std::string& hint, bool useColor) const
{
    std::ostringstream oss;

    std::string icon = "💡 ";
    std::string header = "도움말:";

    if (useColor)
    {
        oss << "\n" << icon << colorize(header, AnsiColor::CYAN) << "\n";
    }
    else
    {
        oss << "\n" << icon << header << "\n";
    }

    // 힌트 내용을 인덴트하여 출력
    std::istringstream hintStream(hint);
    std::string line;
    while (std::getline(hintStream, line))
    {
        if (useColor)
        {
            oss << "   " << colorize(line, AnsiColor::CYAN) << "\n";
        }
        else
        {
            oss << "   " << line << "\n";
        }
    }

    return oss.str();
}

// ============================================================================
// ErrorReporter 구현
// ============================================================================

ErrorReporter::ErrorReporter()
    : sourceMgr_(std::make_unique<SourceManager>())
    , formatter_(std::make_unique<ErrorFormatter>())
    , colorEnabled_(true)
{
}

void ErrorReporter::registerSource(const std::string& filename, const std::string& content)
{
    sourceMgr_->loadSource(filename, content);
}

void ErrorReporter::registerHint(
    ErrorType type,
    const std::string& pattern,
    const std::string& hint
)
{
    ErrorHint errorHint;
    errorHint.pattern = pattern;
    errorHint.hint = hint;

    hints_[type].push_back(std::move(errorHint));
}

void ErrorReporter::report(const KingSejongError& error, std::ostream& out)
{
    try
    {
        // 1. 에러 포맷팅
        std::string formatted = formatter_->formatError(error, *sourceMgr_, colorEnabled_);

        // 2. 힌트 추가 (있는 경우)
        std::string hint = findHint(error);
        if (!hint.empty())
        {
            formatted += formatter_->formatHint(hint, colorEnabled_);
        }

        // 3. 출력
        out << formatted;
    }
    catch (const std::exception& e)
    {
        // 에러 리포팅 중 예외 발생 시에도 최소한의 정보는 출력 (예외 안전)
        out << "[ErrorReporter 내부 오류] " << e.what() << "\n";
        out << "원본 에러: " << error.what() << "\n";
    }
}

void ErrorReporter::setColorEnabled(bool enabled)
{
    colorEnabled_ = enabled;
}

void ErrorReporter::registerDefaultHints()
{
    // ============================================================================
    // NAME_ERROR 힌트 (정의되지 않은 변수)
    // ============================================================================
    registerHint(
        ErrorType::NAME_ERROR,
        "정의되지 않은 변수",
        "변수를 사용하기 전에 먼저 선언해야 합니다.\n"
        "예시: 정수 변수명 = 0"
    );

    // ============================================================================
    // TYPE_ERROR 힌트 (타입 관련 에러)
    // ============================================================================
    registerHint(
        ErrorType::TYPE_ERROR,
        "음수 연산은 숫자에만 적용 가능합니다",
        "음수 연산자(-)는 정수나 실수에만 사용할 수 있습니다.\n"
        "예시: -10, -3.14"
    );

    registerHint(
        ErrorType::TYPE_ERROR,
        "값이 정수 타입이 아닙니다",
        "정수 타입의 값이 필요합니다.\n"
        "문자열을 숫자로 변환하려면 숫자로_변환() 함수를 사용하세요."
    );

    registerHint(
        ErrorType::TYPE_ERROR,
        "값이 실수 타입이 아닙니다",
        "실수 타입의 값이 필요합니다.\n"
        "정수를 실수로 변환하려면 실수로_변환() 함수를 사용하세요."
    );

    registerHint(
        ErrorType::TYPE_ERROR,
        "값이 문자열 타입이 아닙니다",
        "문자열 타입의 값이 필요합니다.\n"
        "다른 타입을 문자열로 변환하려면 문자열로_변환() 함수를 사용하세요."
    );

    registerHint(
        ErrorType::TYPE_ERROR,
        "값이 배열 타입이 아닙니다",
        "배열 타입의 값이 필요합니다.\n"
        "배열 선언 예시: [1, 2, 3]"
    );

    registerHint(
        ErrorType::TYPE_ERROR,
        "함수만 호출할 수 있습니다",
        "함수가 아닌 값에 괄호 ()를 사용할 수 없습니다.\n"
        "함수 정의 예시: 함수 이름(매개변수) { ... }"
    );

    registerHint(
        ErrorType::TYPE_ERROR,
        "배열 인덱스는 정수여야 합니다",
        "배열의 인덱스는 정수만 사용할 수 있습니다.\n"
        "예시: 배열[0], 배열[인덱스]"
    );

    registerHint(
        ErrorType::TYPE_ERROR,
        "서로 다른 타입의 값을 비교할 수 없습니다",
        "같은 타입끼리만 비교할 수 있습니다.\n"
        "정수와 실수는 자동으로 변환되어 비교됩니다."
    );

    registerHint(
        ErrorType::TYPE_ERROR,
        "이 타입은 크기 비교를 지원하지 않습니다",
        "크기 비교(<, >, <=, >=)는 숫자와 문자열만 지원합니다.\n"
        "등호 비교(==, !=)는 모든 타입에서 사용 가능합니다."
    );

    // ============================================================================
    // ZERO_DIVISION_ERROR 힌트 (0으로 나누기)
    // ============================================================================
    registerHint(
        ErrorType::ZERO_DIVISION_ERROR,
        "0으로 나눌 수 없습니다",
        "나누기 전에 나누는 수가 0이 아닌지 확인하세요.\n"
        "예시: 만약 (나누는수 != 0) { 결과 = 나누어지는수 / 나누는수 }"
    );

    // ============================================================================
    // INDEX_ERROR 힌트 (배열 인덱스 범위 초과)
    // ============================================================================
    registerHint(
        ErrorType::INDEX_ERROR,
        "인덱스가 배열 범위를 벗어났습니다",
        "배열의 유효한 인덱스는 0부터 (배열 길이 - 1)까지입니다.\n"
        "길이() 함수로 배열 크기를 확인하세요."
    );

    // ============================================================================
    // ARGUMENT_ERROR 힌트 (함수 인자 오류)
    // ============================================================================
    registerHint(
        ErrorType::ARGUMENT_ERROR,
        "함수의 인자 개수가 일치하지 않습니다",
        "함수를 호출할 때 정의된 매개변수 개수만큼 인자를 전달해야 합니다.\n"
        "함수 정의를 확인하세요."
    );

    registerHint(
        ErrorType::ARGUMENT_ERROR,
        "인자는 배열이어야 합니다",
        "이 함수는 배열을 인자로 받습니다.\n"
        "예시: 길이([1, 2, 3])"
    );

    registerHint(
        ErrorType::ARGUMENT_ERROR,
        "인자는 문자열이어야 합니다",
        "이 함수는 문자열을 인자로 받습니다.\n"
        "다른 타입을 문자열로 변환하려면 문자열로_변환() 함수를 사용하세요."
    );

    // ============================================================================
    // RUNTIME_ERROR 힌트 (런타임 에러)
    // ============================================================================
    registerHint(
        ErrorType::RUNTIME_ERROR,
        "지원되지 않는 연산",
        "이 타입에 대해 해당 연산자를 사용할 수 없습니다.\n"
        "연산 가능한 타입: 정수, 실수, 문자열(+ 연산만)"
    );

    registerHint(
        ErrorType::RUNTIME_ERROR,
        "지원되지 않는 단항 연산자",
        "사용 가능한 단항 연산자: -(음수), !(논리 NOT)"
    );

    registerHint(
        ErrorType::RUNTIME_ERROR,
        "지원되지 않는 정수 연산자",
        "정수 연산자: +, -, *, /, %\n"
        "비교 연산자: ==, !=, <, >, <=, >="
    );

    registerHint(
        ErrorType::RUNTIME_ERROR,
        "지원되지 않는 실수 연산자",
        "실수 연산자: +, -, *, /\n"
        "실수는 나머지 연산(%)을 지원하지 않습니다."
    );

    registerHint(
        ErrorType::RUNTIME_ERROR,
        "지원되지 않는 비교 연산자",
        "사용 가능한 비교 연산자: ==, !=, <, >, <=, >="
    );

    registerHint(
        ErrorType::RUNTIME_ERROR,
        "지원되지 않는 논리 연산자",
        "사용 가능한 논리 연산자: &&(그리고), ||(또는)"
    );

    registerHint(
        ErrorType::RUNTIME_ERROR,
        "알 수 없는 조사입니다",
        "KingSejong 언어에서 지원하는 조사:\n"
        "을/를, 이/가, 은/는, 의, 로/으로, 에서, 에"
    );

    // ============================================================================
    // PARSER_ERROR 힌트 (파서 에러)
    // ============================================================================
    registerHint(
        ErrorType::PARSER_ERROR,
        "다음 토큰으로",
        "문법에 맞지 않는 표현입니다.\n"
        "토큰의 순서와 사용법을 확인하세요."
    );

    registerHint(
        ErrorType::PARSER_ERROR,
        "표현식을 파싱할 수 없습니다",
        "올바른 표현식 형식이 아닙니다.\n"
        "괄호, 연산자, 변수명 등을 확인하세요."
    );

    registerHint(
        ErrorType::PARSER_ERROR,
        "정수로 변환할 수 없습니다",
        "정수는 숫자로만 구성되어야 합니다.\n"
        "예시: 123, -456"
    );

    registerHint(
        ErrorType::PARSER_ERROR,
        "실수로 변환할 수 없습니다",
        "실수는 숫자와 소수점으로 구성되어야 합니다.\n"
        "예시: 3.14, -0.5"
    );

    registerHint(
        ErrorType::VALUE_ERROR,
        "범위 시작값이 종료값보다 큽니다",
        "범위 표현식에서 시작값은 종료값보다 작거나 같아야 합니다.\n"
        "예시: 1부터 10까지, 0부터 100미만"
    );
}

std::string ErrorReporter::findHint(const KingSejongError& error) const
{
    auto it = hints_.find(error.getType());
    if (it == hints_.end())
    {
        return "";
    }

    const auto& hintList = it->second;
    std::string errorMsg = error.what();

    // 에러 메시지에 패턴이 포함된 힌트 찾기
    for (const auto& errorHint : hintList)
    {
        if (errorMsg.find(errorHint.pattern) != std::string::npos)
        {
            return errorHint.hint;
        }
    }

    return "";
}

} // namespace error
} // namespace kingsejong
