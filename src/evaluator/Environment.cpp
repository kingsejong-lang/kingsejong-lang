/**
 * @file Environment.cpp
 * @brief KingSejong 언어 변수 환경 구현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Environment.h"
#include "../error/Error.h"

namespace kingsejong {
namespace evaluator {

// ============================================================================
// 생성자
// ============================================================================

Environment::Environment()
    : outer_(nullptr)
{
}

Environment::Environment(std::shared_ptr<Environment> outer)
    : outer_(outer)
{
}

// ============================================================================
// 변수 관리
// ============================================================================

void Environment::set(const std::string& name, const Value& value)
{
    store_[name] = value;
}

Value Environment::get(const std::string& name) const
{
    // 현재 스코프에서 찾기
    auto it = store_.find(name);
    if (it != store_.end())
    {
        return it->second;
    }

    // 외부 스코프에서 찾기
    if (outer_ != nullptr)
    {
        return outer_->get(name);
    }

    // 변수를 찾을 수 없음
    throw error::NameError("정의되지 않은 변수: " + name);
}

bool Environment::exists(const std::string& name) const
{
    return store_.find(name) != store_.end();
}

bool Environment::existsInChain(const std::string& name) const
{
    // 현재 스코프에서 확인
    if (exists(name))
    {
        return true;
    }

    // 외부 스코프에서 확인
    if (outer_ != nullptr)
    {
        return outer_->existsInChain(name);
    }

    return false;
}

// ============================================================================
// 스코프 관리
// ============================================================================

std::shared_ptr<Environment> Environment::createEnclosed()
{
    return std::make_shared<Environment>(shared_from_this());
}

// ============================================================================
// 유틸리티
// ============================================================================

std::vector<std::string> Environment::keys() const
{
    std::vector<std::string> result;
    result.reserve(store_.size());

    for (const auto& pair : store_)
    {
        result.push_back(pair.first);
    }

    return result;
}

} // namespace evaluator
} // namespace kingsejong
