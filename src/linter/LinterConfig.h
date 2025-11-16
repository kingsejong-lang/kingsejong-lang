#pragma once

/**
 * @file LinterConfig.h
 * @brief Linter 설정 구조
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "linter/IssueSeverity.h"
#include <string>
#include <unordered_map>
#include <optional>
#include <nlohmann/json.hpp>

namespace kingsejong {
namespace linter {

using json = nlohmann::json;

/**
 * @enum RuleSeverityLevel
 * @brief 규칙 심각도 레벨
 */
enum class RuleSeverityLevel
{
    OFF,        ///< 비활성화
    HINT,       ///< 힌트
    INFO,       ///< 정보
    WARNING,    ///< 경고
    ERROR       ///< 에러
};

/**
 * @struct RuleConfig
 * @brief 개별 규칙 설정
 */
struct RuleConfig
{
    RuleSeverityLevel severity;  ///< 심각도
    bool enabled;                ///< 활성화 여부

    RuleConfig()
        : severity(RuleSeverityLevel::WARNING), enabled(true)
    {
    }

    /**
     * @brief 문자열을 RuleSeverityLevel로 변환
     */
    static RuleSeverityLevel stringToSeverity(const std::string& str)
    {
        if (str == "off") return RuleSeverityLevel::OFF;
        if (str == "hint") return RuleSeverityLevel::HINT;
        if (str == "info") return RuleSeverityLevel::INFO;
        if (str == "warning") return RuleSeverityLevel::WARNING;
        if (str == "error") return RuleSeverityLevel::ERROR;
        return RuleSeverityLevel::WARNING;  // 기본값
    }

    /**
     * @brief RuleSeverityLevel을 IssueSeverity로 변환
     */
    static IssueSeverity toIssueSeverity(RuleSeverityLevel level)
    {
        switch (level)
        {
            case RuleSeverityLevel::ERROR:   return IssueSeverity::ERROR;
            case RuleSeverityLevel::WARNING: return IssueSeverity::WARNING;
            case RuleSeverityLevel::INFO:    return IssueSeverity::INFO;
            case RuleSeverityLevel::HINT:    return IssueSeverity::HINT;
            default:                         return IssueSeverity::WARNING;
        }
    }
};

/**
 * @struct LinterConfig
 * @brief Linter 전체 설정
 */
struct LinterConfig
{
    std::unordered_map<std::string, RuleConfig> rules;  ///< 규칙별 설정

    /**
     * @brief JSON에서 설정 로드
     * @param config JSON 설정 객체
     * @return 로드 성공 시 true
     */
    bool loadFromJson(const json& config)
    {
        try
        {
            if (config.contains("rules") && config["rules"].is_object())
            {
                for (auto& [ruleId, value] : config["rules"].items())
                {
                    RuleConfig ruleConfig;

                    if (value.is_string())
                    {
                        // "error", "warning", "off" 등의 문자열
                        std::string severityStr = value.get<std::string>();
                        ruleConfig.severity = RuleConfig::stringToSeverity(severityStr);
                        ruleConfig.enabled = (ruleConfig.severity != RuleSeverityLevel::OFF);
                    }
                    else if (value.is_object())
                    {
                        // { "severity": "error", "enabled": true }
                        if (value.contains("severity"))
                        {
                            ruleConfig.severity = RuleConfig::stringToSeverity(
                                value["severity"].get<std::string>()
                            );
                        }
                        if (value.contains("enabled"))
                        {
                            ruleConfig.enabled = value["enabled"].get<bool>();
                        }
                    }

                    rules[ruleId] = ruleConfig;
                }
            }
            return true;
        }
        catch (const std::exception& e)
        {
            return false;
        }
    }

    /**
     * @brief 규칙이 활성화되어 있는지 확인
     */
    bool isRuleEnabled(const std::string& ruleId) const
    {
        auto it = rules.find(ruleId);
        if (it != rules.end())
        {
            return it->second.enabled && it->second.severity != RuleSeverityLevel::OFF;
        }
        return true;  // 설정되지 않은 규칙은 기본 활성화
    }

    /**
     * @brief 규칙의 심각도 가져오기
     */
    std::optional<IssueSeverity> getRuleSeverity(const std::string& ruleId) const
    {
        auto it = rules.find(ruleId);
        if (it != rules.end())
        {
            return RuleConfig::toIssueSeverity(it->second.severity);
        }
        return std::nullopt;  // 설정되지 않음
    }
};

} // namespace linter
} // namespace kingsejong
