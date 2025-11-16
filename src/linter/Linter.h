#pragma once

/**
 * @file Linter.h
 * @brief KingSejong 언어 정적 분석 도구 (Linter)
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "ast/Node.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "linter/Rule.h"
#include "linter/IssueSeverity.h"
#include "linter/LinterConfig.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace kingsejong {
namespace linter {

/**
 * @struct LinterIssue
 * @brief Linter 이슈 정보
 */
struct LinterIssue
{
    std::string message;        ///< 이슈 메시지
    IssueSeverity severity;     ///< 심각도
    std::string ruleId;         ///< 규칙 ID
    int line;                   ///< 발생 줄
    int column;                 ///< 발생 열
    std::string filename;       ///< 파일 이름

    LinterIssue(
        const std::string& msg,
        IssueSeverity sev,
        const std::string& rule,
        int l = 0,
        int c = 0,
        const std::string& file = ""
    )
        : message(msg), severity(sev), ruleId(rule),
          line(l), column(c), filename(file)
    {
    }

    /**
     * @brief 심각도를 문자열로 변환
     */
    std::string severityToString() const
    {
        switch (severity)
        {
            case IssueSeverity::ERROR:   return "오류";
            case IssueSeverity::WARNING: return "경고";
            case IssueSeverity::INFO:    return "정보";
            case IssueSeverity::HINT:    return "힌트";
            default:                     return "알 수 없음";
        }
    }

    /**
     * @brief 이슈 메시지를 형식화하여 반환
     */
    std::string toString() const
    {
        std::string result;
        if (!filename.empty())
        {
            result += filename + ":";
        }
        if (line > 0)
        {
            result += std::to_string(line) + ":" + std::to_string(column) + ": ";
        }
        result += severityToString() + " [" + ruleId + "]: " + message;
        return result;
    }
};

/**
 * @class Linter
 * @brief KingSejong 언어 Linter
 */
class Linter
{
public:
    /**
     * @brief Linter 생성자
     */
    Linter();

    /**
     * @brief 프로그램 분석
     * @param program AST 루트 노드
     * @param filename 파일 이름
     * @return 이슈가 없으면 true
     */
    bool analyze(ast::Program* program, const std::string& filename = "");

    /**
     * @brief 규칙 추가
     * @param rule 규칙 포인터
     */
    void addRule(std::unique_ptr<Rule> rule);

    /**
     * @brief 모든 이슈 반환
     * @return 이슈 벡터
     */
    const std::vector<LinterIssue>& issues() const { return issues_; }

    /**
     * @brief 에러 개수 반환
     * @return 에러 개수
     */
    size_t errorCount() const;

    /**
     * @brief 경고 개수 반환
     * @return 경고 개수
     */
    size_t warningCount() const;

    /**
     * @brief 이슈 초기화
     */
    void clear();

    /**
     * @brief 설정 파일 로드
     * @param filepath 설정 파일 경로 (.ksjlint.json)
     * @return 로드 성공 시 true
     */
    bool loadConfig(const std::string& filepath);

    /**
     * @brief JSON 문자열에서 설정 로드
     * @param jsonString JSON 설정 문자열
     * @return 파싱 성공 시 true
     */
    bool loadConfigFromString(const std::string& jsonString);

    /**
     * @brief 현재 디렉토리에서 설정 파일 자동 검색 및 로드
     * @return 로드 성공 시 true
     */
    bool loadConfigFromCurrentDir();

    /**
     * @brief 설정 가져오기
     */
    const LinterConfig& getConfig() const { return config_; }

    /**
     * @brief 규칙이 활성화되어 있는지 확인
     */
    bool isRuleEnabled(const std::string& ruleId) const;

    /**
     * @brief 규칙의 설정된 심각도 가져오기 (설정되지 않았으면 std::nullopt)
     */
    std::optional<IssueSeverity> getRuleSeverity(const std::string& ruleId) const;

private:
    std::vector<std::unique_ptr<Rule>> rules_;   ///< 검사 규칙 목록
    std::vector<LinterIssue> issues_;            ///< 발견된 이슈 목록
    std::string filename_;                        ///< 현재 분석 중인 파일
    LinterConfig config_;                         ///< Linter 설정

    /**
     * @brief 이슈 추가
     */
    void addIssue(const LinterIssue& issue);

    friend class Rule;  // Rule이 addIssue를 호출할 수 있도록
};

} // namespace linter
} // namespace kingsejong
