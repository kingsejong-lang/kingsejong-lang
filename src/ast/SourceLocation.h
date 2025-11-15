#pragma once

/**
 * @file SourceLocation.h
 * @brief 소스 코드 위치 정보를 표현하는 구조체
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include <string>
#include <sstream>

namespace kingsejong {
namespace ast {

/**
 * @struct SourceLocation
 * @brief 소스 코드의 위치 정보 (파일명, 줄, 열)
 *
 * 모든 Token과 AST Node가 소스 코드에서의 위치를 추적할 수 있도록 합니다.
 * 에러 메시지 출력 시 정확한 위치를 표시하는 데 사용됩니다.
 *
 * @example
 * ```cpp
 * SourceLocation loc("test.ks", 10, 5);
 * std::cout << loc.toString() << std::endl;  // "test.ks:10:5"
 * ```
 */
struct SourceLocation
{
    std::string filename;  ///< 파일 이름 (빈 문자열이면 stdin 또는 익명)
    int line;              ///< 줄 번호 (1부터 시작)
    int column;            ///< 열 번호 (1부터 시작)

    /**
     * @brief 기본 생성자 - 위치 정보 없음
     */
    SourceLocation()
        : filename(""), line(0), column(0)
    {
    }

    /**
     * @brief 위치 정보 생성자
     * @param file 파일 이름
     * @param l 줄 번호
     * @param c 열 번호
     */
    SourceLocation(const std::string& file, int l, int c)
        : filename(file), line(l), column(c)
    {
    }

    /**
     * @brief 파일명 없이 줄/열만 지정하는 생성자
     * @param l 줄 번호
     * @param c 열 번호
     */
    SourceLocation(int l, int c)
        : filename(""), line(l), column(c)
    {
    }

    /**
     * @brief 위치 정보가 유효한지 확인
     * @return 줄 번호가 1 이상이면 true
     *
     * 줄 번호가 0이면 위치 정보가 설정되지 않은 것으로 간주합니다.
     */
    bool isValid() const
    {
        return line > 0;
    }

    /**
     * @brief 위치 정보를 문자열로 변환 (에러 메시지용)
     * @return "filename:line:column" 형식의 문자열
     *
     * @example
     * - filename이 있는 경우: "test.ks:10:5"
     * - filename이 없는 경우: "10:5"
     * - 위치 정보가 없는 경우: "<unknown>"
     */
    std::string toString() const
    {
        if (!isValid())
        {
            return "<unknown>";
        }

        std::ostringstream oss;
        if (!filename.empty())
        {
            oss << filename << ":";
        }
        oss << line << ":" << column;
        return oss.str();
    }

    /**
     * @brief 두 위치가 같은지 비교
     * @param other 비교할 다른 위치
     * @return 파일명, 줄, 열이 모두 같으면 true
     */
    bool operator==(const SourceLocation& other) const
    {
        return filename == other.filename &&
               line == other.line &&
               column == other.column;
    }

    /**
     * @brief 두 위치가 다른지 비교
     * @param other 비교할 다른 위치
     * @return 파일명, 줄, 열 중 하나라도 다르면 true
     */
    bool operator!=(const SourceLocation& other) const
    {
        return !(*this == other);
    }

    /**
     * @brief 위치 정보를 업데이트
     * @param l 새 줄 번호
     * @param c 새 열 번호
     */
    void update(int l, int c)
    {
        line = l;
        column = c;
    }

    /**
     * @brief 파일명을 포함하여 위치 정보 업데이트
     * @param file 새 파일 이름
     * @param l 새 줄 번호
     * @param c 새 열 번호
     */
    void update(const std::string& file, int l, int c)
    {
        filename = file;
        line = l;
        column = c;
    }
};

/**
 * @brief 위치 범위를 표현하는 구조체
 *
 * AST 노드가 소스 코드에서 차지하는 범위를 나타냅니다.
 * 예: "정수 x = 10" 문장의 시작과 끝 위치
 */
struct SourceRange
{
    SourceLocation start;  ///< 시작 위치
    SourceLocation end;    ///< 끝 위치

    /**
     * @brief 기본 생성자
     */
    SourceRange()
        : start(), end()
    {
    }

    /**
     * @brief 범위 생성자
     * @param s 시작 위치
     * @param e 끝 위치
     */
    SourceRange(const SourceLocation& s, const SourceLocation& e)
        : start(s), end(e)
    {
    }

    /**
     * @brief 단일 위치에서 범위 생성 (시작 = 끝)
     * @param loc 위치
     */
    explicit SourceRange(const SourceLocation& loc)
        : start(loc), end(loc)
    {
    }

    /**
     * @brief 범위가 유효한지 확인
     * @return 시작과 끝 위치가 모두 유효하면 true
     */
    bool isValid() const
    {
        return start.isValid() && end.isValid();
    }

    /**
     * @brief 범위를 문자열로 변환
     * @return "start-end" 형식의 문자열
     */
    std::string toString() const
    {
        if (!isValid())
        {
            return "<unknown range>";
        }

        // 같은 파일, 같은 줄이면 간단히 표시
        if (start.filename == end.filename && start.line == end.line)
        {
            std::ostringstream oss;
            if (!start.filename.empty())
            {
                oss << start.filename << ":";
            }
            oss << start.line << ":" << start.column << "-" << end.column;
            return oss.str();
        }

        // 다른 줄이면 전체 범위 표시
        return start.toString() + " - " + end.toString();
    }
};

} // namespace ast
} // namespace kingsejong
