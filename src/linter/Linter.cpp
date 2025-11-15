/**
 * @file Linter.cpp
 * @brief Linter 구현
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "linter/Linter.h"

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

    // 각 규칙 실행
    for (auto& rule : rules_)
    {
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

} // namespace linter
} // namespace kingsejong
