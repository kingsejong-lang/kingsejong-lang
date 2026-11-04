/**
 * @file FunctionComplexityRule.cpp
 * @brief 함수 복잡도 검사 규칙 구현
 * @author KingSejong Team
 * @date 2025-11-23
 */

#include "FunctionComplexityRule.h"
#include "linter/Linter.h"
#include "linter/IssueSeverity.h"
#include "lexer/Token.h"
#include "ast/Visitor.h"
#include <sstream>
#include <unordered_map>
#include <vector>

namespace kingsejong {
namespace linter {
namespace rules {

/// 함수를 만날 때마다 복잡도 1 짜리 틀을 쌓고, 분기는 가장 안쪽 함수의 틀에 더한다.
/// 안쪽 함수의 분기는 바깥 함수의 복잡도에 들어가지 않는다
class FunctionComplexityRule::Walker : public ast::Visitor
{
public:
    explicit Walker(FunctionComplexityRule& rule) : rule_(rule) {}

    bool enter(ast::Node& node) override
    {
        switch (node.type())
        {
            case ast::NodeType::ASSIGNMENT_STATEMENT:
            {
                auto& stmt = static_cast<ast::AssignmentStatement&>(node);
                nameFunction(stmt.varName(), stmt.value(), stmt.location());
                break;
            }
            case ast::NodeType::VAR_DECLARATION:
            {
                auto& stmt = static_cast<ast::VarDeclaration&>(node);
                nameFunction(stmt.varName(), stmt.initializer(), stmt.location());
                break;
            }
            case ast::NodeType::FUNCTION_LITERAL:
            case ast::NodeType::ASYNC_FUNCTION_LITERAL:
            {
                // '이름 = 함수(...)' 나 '함수 이름(...)' 이면 그 이름과 문장 위치, 아니면 익명 함수와 함수 위치
                auto named = names_.find(&node);
                if (named != names_.end())
                {
                    frames_.push_back(Frame{named->second.name, 1, named->second.location});
                }
                else
                {
                    frames_.push_back(Frame{"<익명 함수>", 1, node.location()});
                }
                break;
            }
            case ast::NodeType::IF_STATEMENT:
            case ast::NodeType::WHILE_STATEMENT:
            case ast::NodeType::RANGE_FOR_STATEMENT:
            case ast::NodeType::FOR_EACH_STATEMENT:
            case ast::NodeType::REPEAT_STATEMENT:
                addDecision();
                break;
            case ast::NodeType::BINARY_EXPRESSION:
            {
                // 논리 연산자 (&&, ||) 는 단축 평가라 분기 하나를 더한다
                const std::string& op = static_cast<ast::BinaryExpression&>(node).op();
                if (op == "&&" || op == "||")
                {
                    addDecision();
                }
                break;
            }
            default:
                break;
        }
        return true;
    }

    void leave(ast::Node& node) override
    {
        if (node.type() == ast::NodeType::FUNCTION_LITERAL || node.type() == ast::NodeType::ASYNC_FUNCTION_LITERAL)
        {
            Frame frame = frames_.back();
            frames_.pop_back();
            rule_.finishFunction(frame.name, frame.complexity, frame.location);
        }
    }

private:
    struct Frame
    {
        std::string name;
        int complexity;
        ast::SourceLocation location;
    };

    struct Name
    {
        std::string name;
        ast::SourceLocation location;
    };

    void nameFunction(const std::string& name, const ast::Expression* value, const ast::SourceLocation& location)
    {
        if (value && (value->type() == ast::NodeType::FUNCTION_LITERAL ||
                      value->type() == ast::NodeType::ASYNC_FUNCTION_LITERAL))
        {
            names_[value] = Name{name, location};
        }
    }

    void addDecision()
    {
        // 함수 밖(최상위)의 분기는 어느 함수의 복잡도도 아니다
        if (!frames_.empty())
        {
            frames_.back().complexity++;
        }
    }

    FunctionComplexityRule& rule_;
    std::vector<Frame> frames_;
    std::unordered_map<const ast::Node*, Name> names_;
};

void FunctionComplexityRule::analyze(ast::Program* program)
{
    if (!program) return;

    Walker walker(*this);
    for (auto& stmt : program->statements())
    {
        ast::walk(stmt.get(), walker);
    }
}

void FunctionComplexityRule::finishFunction(const std::string& name, int complexity,
                                            const ast::SourceLocation& location)
{
    if (complexity <= maxComplexity_)
    {
        return;
    }

    std::ostringstream oss;
    oss << "함수 '" << name
        << "'의 순환 복잡도가 " << complexity
        << "입니다 (최대 권장: " << maxComplexity_ << "). "
        << "함수를 더 작은 단위로 분해하는 것을 고려하세요.";

    IssueSeverity severity = IssueSeverity::WARNING;
    if (complexity > 20)
    {
        severity = IssueSeverity::ERROR;
        oss << " (매우 복잡함)";
    }

    // 이름이 있으면 편집기가 그 이름에 밑줄을 긋는다
    reportIssue(oss.str(), severity, location.line, location.column, name == "<익명 함수>" ? "" : name);
}

} // namespace rules
} // namespace linter
} // namespace kingsejong
