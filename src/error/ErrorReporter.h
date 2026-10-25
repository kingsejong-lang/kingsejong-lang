#pragma once

/**
 * @file ErrorReporter.h
 * @brief 향상된 에러 리포팅 시스템
 * @author KingSejong Team
 * @date 2025-11-11
 *
 * 교육용 언어로서의 사용성을 위한 에러 메시지 개선:
 * - 한글 에러 메시지
 * - 소스 코드 컨텍스트 표시
 * - 에러 위치 시각화
 * - 에러 힌트/제안
 * - ANSI 컬러 출력
 */

#include "Error.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>
#include <ostream>
#include <iostream>

namespace kingsejong {
namespace error {

// ============================================================================
// ANSI 컬러 코드
// ============================================================================

/**
 * @enum AnsiColor
 * @brief 터미널 컬러 출력을 위한 ANSI 컬러
 */
enum class AnsiColor
{
    RESET,      ///< 기본색으로 리셋
    RED,        ///< 빨강 (에러)
    GREEN,      ///< 초록 (성공)
    YELLOW,     ///< 노랑 (경고)
    BLUE,       ///< 파랑 (정보)
    MAGENTA,    ///< 마젠타 (위치)
    CYAN,       ///< 청록 (힌트)
    BOLD,       ///< 굵게
    DIM         ///< 흐리게 (줄 번호 등)
};

/**
 * @brief ANSI 컬러 코드를 문자열로 변환
 * @param color 컬러 타입
 * @return ANSI 이스케이프 시퀀스
 */
std::string ansiColor(AnsiColor color);

/**
 * @brief 텍스트에 컬러 적용
 * @param text 원본 텍스트
 * @param color 적용할 컬러
 * @return 컬러가 적용된 텍스트
 */
std::string colorize(const std::string& text, AnsiColor color);

/**
 * @brief 에러 타입을 한글 문자열로 변환
 * @param type 에러 타입
 * @return 한글 에러 타입 이름
 */
std::string errorTypeToKorean(ErrorType type);

// ============================================================================
// SourceManager: 소스 코드 관리
// ============================================================================

/**
 * @class SourceManager
 * @brief 소스 코드를 관리하고 줄 단위 접근 제공
 *
 * 에러 메시지에서 소스 코드 컨텍스트를 표시하기 위해
 * 소스 파일의 내용을 줄 단위로 저장하고 조회합니다.
 *
 * Thread Safety: NOT thread-safe
 * Memory: RAII - 모든 메모리 자동 관리
 */
class SourceManager
{
public:
    /**
     * @brief 기본 생성자
     */
    SourceManager();

    /**
     * @brief 소멸자 (default)
     */
    ~SourceManager() = default;

    // 복사 금지, 이동 허용
    SourceManager(const SourceManager&) = delete;
    SourceManager& operator=(const SourceManager&) = delete;
    SourceManager(SourceManager&&) = default;
    SourceManager& operator=(SourceManager&&) = default;

    /**
     * @brief 소스 코드 등록
     * @param filename 파일 이름
     * @param content 소스 코드 전체 내용
     *
     * 소스 코드를 줄 단위로 분리하여 저장합니다.
     */
    void loadSource(const std::string& filename, const std::string& content);

    /**
     * @brief 소스가 등록돼 있는지 확인
     */
    bool hasSource(const std::string& filename) const { return sources_.count(filename) > 0; }

    /**
     * @brief 등록되지 않은 파일이면 디스크에서 읽어 등록한다
     * @return 등록돼 있거나 읽어서 등록했으면 true
     *
     * 가져온 모듈 안에서 난 실행 오류는 메인 파일과 달리 미리 등록되지 않으므로, 보고할 때 읽어 온다.
     */
    bool ensureSource(const std::string& filename);

    /**
     * @brief 특정 줄 가져오기
     * @param filename 파일 이름
     * @param line 줄 번호 (1부터 시작)
     * @return 해당 줄의 내용 (없으면 nullopt)
     */
    std::optional<std::string> getLine(const std::string& filename, int line) const;

    /**
     * @brief 에러 주변 컨텍스트 가져오기
     * @param filename 파일 이름
     * @param line 에러 발생 줄
     * @param contextLines 앞뒤로 가져올 줄 수
     * @return 컨텍스트 줄들 (에러 줄 포함)
     *
     * 예: getContext("test.ksj", 5, 1)
     * → 4번째 줄, 5번째 줄, 6번째 줄 반환
     */
    std::vector<std::string> getContext(
        const std::string& filename,
        int line,
        int contextLines = 1
    ) const;

private:
    /**
     * @struct SourceFile
     * @brief 소스 파일 정보
     */
    struct SourceFile
    {
        std::string filename;
        std::vector<std::string> lines;
    };

    std::unordered_map<std::string, SourceFile> sources_;
};

// ============================================================================
// ErrorFormatter: 에러 메시지 포맷팅
// ============================================================================

/**
 * @class ErrorFormatter
 * @brief 에러를 보기 좋게 포맷팅
 *
 * KingSejongError 객체를 받아서 다음을 포함한 포맷팅된 문자열 생성:
 * - 위치 정보 (파일:줄:컬럼)
 * - 소스 코드 컨텍스트
 * - 에러 위치 하이라이트 (화살표)
 * - 에러 타입 및 메시지
 * - 힌트 (있는 경우)
 *
 * Thread Safety: Thread-safe (상태 없음)
 */
class ErrorFormatter
{
public:
    /**
     * @brief 기본 생성자
     */
    ErrorFormatter() = default;

    /**
     * @brief 에러를 포맷팅
     * @param error 에러 객체
     * @param sourceMgr 소스 코드 관리자
     * @param useColor ANSI 컬러 사용 여부
     * @return 포맷팅된 에러 메시지
     */
    std::string formatError(
        const KingSejongError& error,
        const SourceManager& sourceMgr,
        bool useColor = true
    ) const;

    /**
     * @brief 힌트 포맷팅
     * @param hint 힌트 메시지
     * @param useColor 컬러 사용 여부
     * @return "💡 도움말: 힌트" 형식
     */
    std::string formatHint(const std::string& hint, bool useColor) const;

private:
    /**
     * @brief 위치 정보 포맷팅
     * @param loc 소스 위치
     * @param useColor 컬러 사용 여부
     * @return "📍 오류 위치: file:line:column" 형식
     */
    std::string formatLocation(const SourceLocation& loc, bool useColor) const;

    /**
     * @brief 소스 코드 컨텍스트 포맷팅
     * @param lines 컨텍스트 줄들
     * @param errorLine 에러 발생 줄 (절대 줄 번호)
     * @param errorCol 에러 발생 컬럼
     * @param startLine 컨텍스트 시작 줄 (절대 줄 번호)
     * @param useColor 컬러 사용 여부
     * @return 줄 번호와 화살표가 포함된 컨텍스트
     */
    std::string formatContext(
        const std::vector<std::string>& lines,
        int errorLine,
        int errorCol,
        int startLine,
        bool useColor
    ) const;

    /**
     * @brief 에러 타입 및 메시지 포맷팅
     * @param error 에러 객체
     * @param useColor 컬러 사용 여부
     * @return "❌ 에러타입: 메시지" 형식
     */
    std::string formatErrorMessage(const KingSejongError& error, bool useColor) const;

    /**
     * @brief 호출 스택 포맷팅
     * @param frames 프레임 목록 (가장 안쪽부터)
     * @param useColor 컬러 사용 여부
     * @return "호출 스택 (가장 안쪽부터):" 머리와 "  함수 (파일:줄)" 줄들
     */
    std::string formatStackTrace(const std::vector<StackFrame>& frames, bool useColor) const;
};

// ============================================================================
// ErrorReporter: 통합 에러 리포팅 시스템
// ============================================================================

/**
 * @class ErrorReporter
 * @brief 에러 리포팅 통합 시스템
 *
 * 소스 관리, 에러 포맷팅, 힌트 시스템을 통합하여
 * 사용자 친화적인 에러 메시지를 생성합니다.
 *
 * Usage:
 * ```cpp
 * ErrorReporter reporter;
 * reporter.registerSource("test.ksj", sourceCode);
 * reporter.registerHint(ErrorType::PARSER_ERROR, "pattern", "hint");
 *
 * auto error = ParserError("message", location);
 * reporter.report(error);  // stderr로 출력
 * ```
 *
 * Thread Safety: NOT thread-safe
 * Memory: RAII - unique_ptr로 모든 리소스 관리
 */
class ErrorReporter
{
public:
    /**
     * @brief 기본 생성자
     */
    ErrorReporter();

    /**
     * @brief 소멸자 (default, unique_ptr가 자동 정리)
     */
    ~ErrorReporter() = default;

    // 복사 금지, 이동 허용
    ErrorReporter(const ErrorReporter&) = delete;
    ErrorReporter& operator=(const ErrorReporter&) = delete;
    ErrorReporter(ErrorReporter&&) = default;
    ErrorReporter& operator=(ErrorReporter&&) = default;

    /**
     * @brief 소스 코드 등록
     * @param filename 파일 이름
     * @param content 소스 코드 전체 내용
     */
    void registerSource(const std::string& filename, const std::string& content);

    /**
     * @brief 에러 힌트 등록
     * @param type 에러 타입
     * @param pattern 에러 메시지 패턴 (부분 일치)
     * @param hint 힌트 메시지
     *
     * 특정 에러 패턴에 대해 힌트를 등록합니다.
     * 에러 메시지가 pattern을 포함하면 해당 힌트를 표시합니다.
     */
    void registerHint(
        ErrorType type,
        const std::string& pattern,
        const std::string& hint
    );

    /**
     * @brief 에러 리포트
     * @param error 에러 객체
     * @param out 출력 스트림 (기본: std::cerr)
     *
     * 에러를 포맷팅하여 출력합니다.
     * 예외 발생 시에도 최소한의 정보는 출력합니다 (예외 안전).
     */
    void report(const KingSejongError& error, std::ostream& out = std::cerr);

    /**
     * @brief 컬러 출력 활성화/비활성화
     * @param enabled true면 컬러 사용, false면 비활성화
     */
    void setColorEnabled(bool enabled);

    /**
     * @brief 기본 에러 힌트 등록
     *
     * KingSejong 언어의 일반적인 에러 상황에 대한 힌트를 등록합니다.
     * REPL이나 인터프리터 시작 시 호출하여 사용자 경험을 향상시킵니다.
     */
    void registerDefaultHints();

private:
    std::unique_ptr<SourceManager> sourceMgr_;      ///< 소스 관리자 (RAII)
    std::unique_ptr<ErrorFormatter> formatter_;     ///< 에러 포맷터 (RAII)
    bool colorEnabled_;                             ///< 컬러 출력 활성화 여부

    /**
     * @struct ErrorHint
     * @brief 에러 힌트 정보
     */
    struct ErrorHint
    {
        std::string pattern;    ///< 에러 메시지 패턴
        std::string hint;       ///< 힌트 메시지
    };

    /// 에러 타입별 힌트 맵
    std::unordered_map<ErrorType, std::vector<ErrorHint>> hints_;

    /**
     * @brief 에러에 맞는 힌트 찾기
     * @param error 에러 객체
     * @return 힌트 메시지 (없으면 빈 문자열)
     */
    std::string findHint(const KingSejongError& error) const;
};

} // namespace error
} // namespace kingsejong
