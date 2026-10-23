/**
 * @file Environment.cpp
 * @brief KingSejong 언어 변수 환경 구현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Environment.h"
#include "../error/Error.h"
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <functional>

namespace kingsejong {
namespace evaluator {

// ============================================================================
// 생성자
// ============================================================================

std::atomic<size_t> Environment::liveCount_{0};

Environment::Environment()
    : outer_(nullptr)
{
    ++liveCount_;
}

Environment::Environment(std::shared_ptr<Environment> outer)
    : outer_(outer)
{
    ++liveCount_;
}

Environment::~Environment()
{
    --liveCount_;
}

// ============================================================================
// 변수 관리
// ============================================================================

void Environment::set(const std::string& name, const Value& value)
{
    store_[name] = value;
}

void Environment::setWithLookup(const std::string& name, const Value& value)
{
    // 1. 현재 스코프에 변수가 있으면 현재 스코프에서 업데이트
    if (exists(name))
    {
        store_[name] = value;
        return;
    }

    // 2. 외부 스코프에 변수가 있는지 확인
    if (outer_ != nullptr && outer_->existsInChain(name))
    {
        // 외부 스코프에서 재귀적으로 업데이트
        outer_->setWithLookup(name, value);
        return;
    }

    // 3. 어디에도 없으면 현재 스코프에 새로 생성
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


// ============================================================================
// 순환 참조 해제
// ============================================================================

namespace {

/**
 * @brief 환경 하나에서 출발하는 객체 그래프의 "안쪽 참조" 수를 센다
 *
 * 객체마다 이 그래프 안에서 발견한 참조 수(internal)를 세고, 그 수가 use_count 와
 * 같아진 객체만 더 들어가 본다. use_count 가 더 크면 바깥 어딘가가 잡고 있는 것이고,
 * 그 객체에서 뻗는 간선은 세지 않으므로 그 너머에 있는 closure 참조는 발견되지 않는다.
 * 그러면 root 의 use_count 와 발견한 참조 수가 어긋나 호출자가 비우기를 포기한다.
 * 즉 발견한 수 <= 실제 수, 호출자 핸들 수 <= 실제 핸들 수가 항상 성립하므로
 * 두 합이 use_count 와 같다는 것은 둘 다 정확히 맞았다는 뜻이다.
 */
class ReachabilityCounter
{
public:
    explicit ReachabilityCounter(const Environment* root)
        : root_(root)
    {
        for (auto outer = root->outer(); outer; outer = outer->outer())
        {
            ancestors_.insert(outer.get());
        }
    }

    size_t rootInternalRefs() const
    {
        auto it = internal_.find(root_);
        return it == internal_.end() ? 0 : it->second;
    }

    void walkStore(const std::unordered_map<std::string, Value>& store)
    {
        for (const auto& [name, value] : store)
        {
            visitValue(value);
        }
        drain();
    }

private:
    const Environment* root_;
    std::unordered_set<const void*> ancestors_;
    std::unordered_map<const void*, size_t> internal_;
    std::unordered_set<const void*> expanded_;
    std::vector<std::function<void()>> pending_;

    void drain()
    {
        while (!pending_.empty())
        {
            auto job = std::move(pending_.back());
            pending_.pop_back();
            job();
        }
    }

    // 간선을 하나 발견했다. 대상의 참조가 모두 이 그래프 안에서 왔다고 확인되면 안으로 들어간다.
    template <typename T, typename Expand>
    void edge(const std::shared_ptr<T>& target, Expand expand)
    {
        if (!target)
        {
            return;
        }
        const void* key = target.get();
        size_t count = ++internal_[key];
        if (count == static_cast<size_t>(target.use_count()) && expanded_.insert(key).second)
        {
            // raw pointer 만 잡아 두어 use_count 를 바꾸지 않는다. 그래프 밖에서 잡힌 객체가 없음을
            // 확인한 뒤에만 들어가므로, 호출자가 root 핸들을 쥐고 있는 동안 대상은 살아 있다.
            const T* raw = target.get();
            pending_.push_back([raw, expand]() { expand(*raw); });
        }
    }

    void edgeEnvironment(const std::shared_ptr<Environment>& env)
    {
        if (!env)
        {
            return;
        }
        if (env.get() == root_)
        {
            // root 자체는 걷지 않는다. 여기로 돌아오는 간선 수만 센다.
            ++internal_[root_];
            return;
        }
        if (ancestors_.count(env.get()))
        {
            // 바깥 스코프는 root 를 저장소에 담지 않는 한 root 에 닿지 못하고, 담았다면
            // 그 참조는 발견되지 않아 호출자가 포기한다. 전역 환경 전체를 걷는 비용을 피한다.
            return;
        }
        edge(env, [this](const Environment& e) {
            for (const auto& [name, value] : e.getAllBindings())
            {
                visitValue(value);
            }
            edgeEnvironment(e.outer());
        });
    }

    void edgeFunction(const std::shared_ptr<Function>& fn)
    {
        edge(fn, [this](const Function& f) {
            edgeEnvironment(f.closure());
            edgeClassDef(f.classDef());
        });
    }

    void edgeClassDef(const std::shared_ptr<ClassDefinition>& def)
    {
        edge(def, [this](const ClassDefinition& d) {
            for (const auto& [name, method] : d.methods())
            {
                edgeFunction(method);
            }
            edgeFunction(d.constructor());
        });
    }

    void visitValue(const Value& value)
    {
        std::visit(
            [this](const auto& item) {
                using T = std::decay_t<decltype(item)>;
                if constexpr (std::is_same_v<T, std::shared_ptr<Function>>)
                {
                    edgeFunction(item);
                }
                else if constexpr (std::is_same_v<T, std::shared_ptr<std::vector<Value>>>)
                {
                    edge(item, [this](const std::vector<Value>& arr) {
                        for (const auto& v : arr)
                        {
                            visitValue(v);
                        }
                    });
                }
                else if constexpr (std::is_same_v<T, std::shared_ptr<std::unordered_map<std::string, Value>>>)
                {
                    edge(item, [this](const std::unordered_map<std::string, Value>& dict) {
                        for (const auto& [k, v] : dict)
                        {
                            visitValue(v);
                        }
                    });
                }
                else if constexpr (std::is_same_v<T, std::shared_ptr<ClassInstance>>)
                {
                    edge(item, [this](const ClassInstance& inst) {
                        edgeClassDef(inst.classDef());
                        for (const auto& [k, v] : inst.fields())
                        {
                            visitValue(v);
                        }
                    });
                }
                // Promise 는 콜백 안에 함수를 숨기고 있을 수 있는데 들여다볼 수 없다.
                // 세지 않으면 그 함수의 closure 참조가 발견되지 않아 보수적으로 포기된다.
            },
            value.data());
    }
};

} // namespace

bool Environment::releaseIfUnreachable(size_t externalHandles)
{
    const auto useCount = static_cast<size_t>(weak_from_this().use_count());
    if (useCount <= externalHandles)
    {
        // 아무 함수도 이 환경을 붙잡지 않았다. 호출자가 놓으면 그냥 사라진다.
        return false;
    }

    ReachabilityCounter counter(this);
    counter.walkStore(store_);

    if (useCount != externalHandles + counter.rootInternalRefs())
    {
        return false;
    }

    // 모든 참조가 저장소 안에서 출발했다. 저장소를 비우면 사슬이 끊겨 전부 해제된다.
    store_.clear();
    return true;
}

} // namespace evaluator
} // namespace kingsejong
