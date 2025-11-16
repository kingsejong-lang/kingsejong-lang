/**
 * @file Linter.cpp
 * @brief Linter 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/Linter.h"
#include "config/ConfigLoader.h"

namespace kingsejong {
namespace linter {

Linter::Linter()
{
}

bool Linter::analyze(ast::Program* program, const std::string& filename)
{
    if (!program)
    {
        return false;
    }

    filename_ = filename;
    issues_.clear();

    // 각 규칙 실행 (설정에서 활성화된 규칙만)
    for (auto& rule : rules_)
    {
        // 규칙이 설정에서 비활성화되어 있으면 건너뜀
        if (!isRuleEnabled(rule->ruleId()))
        {
            continue;
        }

        rule->setLinter(this);
        rule->analyze(program);
    }

    // 에러가 없으면 true
    return errorCount() == 0;
}

void Linter::addRule(std::unique_ptr<Rule> rule)
{
    rules_.push_back(std::move(rule));
}

size_t Linter::errorCount() const
{
    size_t count = 0;
    for (const auto& issue : issues_)
    {
        if (issue.severity == IssueSeverity::ERROR)
        {
            count++;
        }
    }
    return count;
}

size_t Linter::warningCount() const
{
    size_t count = 0;
    for (const auto& issue : issues_)
    {
        if (issue.severity == IssueSeverity::WARNING)
        {
            count++;
        }
    }
    return count;
}

void Linter::clear()
{
    issues_.clear();
}

void Linter::addIssue(const LinterIssue& issue)
{
    issues_.push_back(issue);
}

bool Linter::loadConfig(const std::string& filepath)
{
    json configJson;
    if (!config::ConfigLoader::loadFromFile(filepath, configJson))
    {
        return false;
    }

    return config_.loadFromJson(configJson);
}

bool Linter::loadConfigFromString(const std::string& jsonString)
{
    json configJson;
    if (!config::ConfigLoader::loadFromString(jsonString, configJson))
    {
        return false;
    }

    return config_.loadFromJson(configJson);
}

bool Linter::loadConfigFromCurrentDir()
{
    std::string configPath = config::ConfigLoader::findConfigFile(".ksjlint.json");
    if (configPath.empty())
    {
        return false;  // 설정 파일을 찾지 못함
    }

    return loadConfig(configPath);
}

bool Linter::isRuleEnabled(const std::string& ruleId) const
{
    return config_.isRuleEnabled(ruleId);
}

std::optional<IssueSeverity> Linter::getRuleSeverity(const std::string& ruleId) const
{
    return config_.getRuleSeverity(ruleId);
}

} // namespace linter
} // namespace kingsejong
