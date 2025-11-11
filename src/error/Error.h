#pragma once

/**
 * @file Error.h
 * @brief KingSejong 언어 에러 처리 시스템
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include <string>
#include <stdexcept>
#include <optional>

namespace kingsejong {
namespace error {

/**
 * @enum ErrorType
 * @brief 에러 타입 분류
 */
enum class ErrorType
{
    LEXER_ERROR,        ///< 렉서 에러 (토큰화 실패)
    PARSER_ERROR,       ///< 파서 에러 (구문 분석 실패)
    RUNTIME_ERROR,      ///< 런타임 에러 (실행 중 에러)
    TYPE_ERROR,         ///< 타입 에러 (타입 불일치)
    NAME_ERROR,         ///< 이름 에러 (정의되지 않은 변수)
    VALUE_ERROR,        ///< 값 에러 (잘못된 값)
    ZERO_DIVISION_ERROR,///< 0으로 나누기 에러
    INDEX_ERROR,        ///< 인덱스 에러 (범위 초과)
    ARGUMENT_ERROR      ///< 인자 에러 (함수 호출 시 인자 오류)
};

/**
 * @struct SourceLocation
 * @brief 소스 코드 위치 정보
 */
struct SourceLocation
{
    std::string filename;   ///< 파일 이름
    int line;              ///< 줄 번호 (1부터 시작)
    int column;            ///< 컬럼 번호 (1부터 시작)

    SourceLocation()
        : filename("<unknown>")
        , line(0)
        , column(0)
    {}

    SourceLocation(const std::string& file, int l, int c)
        : filename(file)
        , line(l)
        , column(c)
    {}

    /**
     * @brief 위치 정보를 문자열로 변환
     * @return "filename:line:column" 형식
     */
    std::string toString() const
    {
        if (line > 0 && column > 0)
        {
            return filename + ":" + std::to_string(line) + ":" + std::to_string(column);
        }
        else if (line > 0)
        {
            return filename + ":" + std::to_string(line);
        }
        else
        {
            return filename;
        }
    }
};

/**
 * @class KingSejongError
 * @brief KingSejong 언어의 기본 에러 클래스
 *
 * std::runtime_error를 상속하여 기존 코드와 호환성 유지
 */
class KingSejongError : public std::runtime_error
{
private:
    ErrorType type_;                        ///< 에러 타입
    std::optional<SourceLocation> location_; ///< 소스 위치 (optional)

public:
    /**
     * @brief 기본 생성자
     * @param type 에러 타입
     * @param message 에러 메시지 (한글)
     */
    KingSejongError(ErrorType type, const std::string& message)
        : std::runtime_error(message)
        , type_(type)
        , location_(std::nullopt)
    {}

    /**
     * @brief 위치 정보 포함 생성자
     * @param type 에러 타입
     * @param message 에러 메시지 (한글)
     * @param location 소스 위치
     */
    KingSejongError(ErrorType type, const std::string& message, const SourceLocation& location)
        : std::runtime_error(formatErrorMessage(message, location))
        , type_(type)
        , location_(location)
    {}

    /**
     * @brief 에러 타입 반환
     * @return ErrorType
     */
    ErrorType getType() const { return type_; }

    /**
     * @brief 소스 위치 반환
     * @return optional<SourceLocation>
     */
    const std::optional<SourceLocation>& getLocation() const { return location_; }

    /**
     * @brief 에러 타입을 한글 문자열로 변환
     * @return 한글 에러 타입 이름
     */
    std::string getTypeString() const
    {
        switch (type_)
        {
            case ErrorType::LEXER_ERROR:        return "렉서 에러";
            case ErrorType::PARSER_ERROR:       return "파서 에러";
            case ErrorType::RUNTIME_ERROR:      return "실행 에러";
            case ErrorType::TYPE_ERROR:         return "타입 에러";
            case ErrorType::NAME_ERROR:         return "이름 에러";
            case ErrorType::VALUE_ERROR:        return "값 에러";
            case ErrorType::ZERO_DIVISION_ERROR:return "0으로 나누기 에러";
            case ErrorType::INDEX_ERROR:        return "인덱스 에러";
            case ErrorType::ARGUMENT_ERROR:     return "인자 에러";
            default:                            return "알 수 없는 에러";
        }
    }

private:
    /**
     * @brief 에러 메시지 포맷팅
     * @param message 기본 메시지
     * @param location 위치 정보
     * @return 포맷팅된 메시지
     */
    static std::string formatErrorMessage(const std::string& message, const SourceLocation& location)
    {
        if (location.line > 0)
        {
            return location.toString() + ": " + message;
        }
        return message;
    }
};

// ============================================================================
// 편의 함수들
// ============================================================================

/**
 * @brief 렉서 에러 생성
 */
inline KingSejongError LexerError(const std::string& message)
{
    return KingSejongError(ErrorType::LEXER_ERROR, message);
}

inline KingSejongError LexerError(const std::string& message, const SourceLocation& location)
{
    return KingSejongError(ErrorType::LEXER_ERROR, message, location);
}

/**
 * @brief 파서 에러 생성
 */
inline KingSejongError ParserError(const std::string& message)
{
    return KingSejongError(ErrorType::PARSER_ERROR, message);
}

inline KingSejongError ParserError(const std::string& message, const SourceLocation& location)
{
    return KingSejongError(ErrorType::PARSER_ERROR, message, location);
}

/**
 * @brief 런타임 에러 생성
 */
inline KingSejongError RuntimeError(const std::string& message)
{
    return KingSejongError(ErrorType::RUNTIME_ERROR, message);
}

inline KingSejongError RuntimeError(const std::string& message, const SourceLocation& location)
{
    return KingSejongError(ErrorType::RUNTIME_ERROR, message, location);
}

/**
 * @brief 타입 에러 생성
 */
inline KingSejongError TypeError(const std::string& message)
{
    return KingSejongError(ErrorType::TYPE_ERROR, message);
}

/**
 * @brief 이름 에러 생성
 */
inline KingSejongError NameError(const std::string& message)
{
    return KingSejongError(ErrorType::NAME_ERROR, message);
}

inline KingSejongError NameError(const std::string& message, const SourceLocation& location)
{
    return KingSejongError(ErrorType::NAME_ERROR, message, location);
}

/**
 * @brief 값 에러 생성
 */
inline KingSejongError ValueError(const std::string& message)
{
    return KingSejongError(ErrorType::VALUE_ERROR, message);
}

/**
 * @brief 0으로 나누기 에러 생성
 */
inline KingSejongError ZeroDivisionError(const std::string& message = "0으로 나눌 수 없습니다")
{
    return KingSejongError(ErrorType::ZERO_DIVISION_ERROR, message);
}

/**
 * @brief 인덱스 에러 생성
 */
inline KingSejongError IndexError(const std::string& message)
{
    return KingSejongError(ErrorType::INDEX_ERROR, message);
}

/**
 * @brief 인자 에러 생성
 */
inline KingSejongError ArgumentError(const std::string& message)
{
    return KingSejongError(ErrorType::ARGUMENT_ERROR, message);
}

} // namespace error
} // namespace kingsejong
