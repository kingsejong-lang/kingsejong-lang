/**
 * @file Environment.cpp
 * @brief KingSejong 언어 변수 환경 구현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "runtime/Environment.h"
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
 * @brief 환경 하나에서 출발하는 객체 그래프를 걷고, 그 환경이 바깥에서 닿을 수 있는지 판정한다
 *
 * 1단계: 저장소에서 출발해 닿는 객체(함수, 환경, 클래스 정의, 인스턴스, 배열, 사전)를 걷고
 *        객체마다 이 그래프 안에서 발견한 참조 수(internal)와 부모·자식 목록을 기록한다.
 *        - 배열·사전은 참조가 하나뿐일 때만 들어간다. 여러 곳에서 잡힌 큰 배열(함수 인자로
 *          넘긴 배열 등)을 호출마다 전부 훑지 않기 위해서다.
 *        - 환경은 root 의 자손(outer 사슬이 root 를 지나는 것)만 들어간다. 다른 환경은 root 에
 *          닿을 수 없고(호출자는 자기 사슬 밖의 환경에 변수를 쓸 수 없다), 모듈 환경처럼 큰
 *          것을 훑거나 잘못 비우는 일을 피한다.
 *        들어가지 않은 객체는 바깥에서 잡힌 것으로 본다.
 * 2단계: use_count 가 internal 보다 큰 객체는 그래프 밖 어딘가가 잡고 있다(외부 객체).
 *        외부 객체에서 자식 간선을 따라 닿는 객체는 모두 아직 살아 있어야 한다.
 *        root 가 그 안에 들면 비울 수 없다. 아니면 root 와, 역시 바깥에서 닿지 않는 자손
 *        환경들의 저장소를 비운다. 자손 환경도 같이 비워야 그 안의 함수 ↔ 환경 사슬까지 풀린다.
 *
 * 건너뛰거나 못 찾은 참조는 항상 "비우지 않음" 쪽으로만 작용한다: 발견한 참조 수는 실제보다
 * 작거나 같고, 그러면 root 의 수가 맞지 않거나 외부 객체로 분류되기 때문이다.
 */
class ReachabilityGraph
{
public:
    explicit ReachabilityGraph(Environment* root)
        : root_(root)
    {
        for (auto outer = root->outer(); outer; outer = outer->outer())
        {
            ancestors_.insert(outer.get());
        }
    }

    void walkStore(const std::unordered_map<std::string, Value>& store)
    {
        for (const auto& [name, value] : store)
        {
            visitValue(root_, value);
        }
        while (!pending_.empty())
        {
            auto job = std::move(pending_.back());
            pending_.pop_back();
            job();
        }
    }

    /// root 로 들어오는 참조 중 이 그래프 안에서 발견한 수
    size_t rootInternalRefs() const
    {
        auto it = nodes_.find(root_);
        return it == nodes_.end() ? 0 : it->second.internal;
    }

    /**
     * @brief 외부 객체에서 닿는 객체를 표시한다
     * @return root 가 바깥에서 닿을 수 있으면 true
     */
    bool markReachableFromOutside()
    {
        std::vector<const void*> stack;
        for (auto& [key, node] : nodes_)
        {
            if (key != root_ && (node.skipped || node.useCount > node.internal))
            {
                node.external = true;
                stack.push_back(key);
            }
        }
        while (!stack.empty())
        {
            const void* current = stack.back();
            stack.pop_back();
            for (const void* child : nodes_[current].children)
            {
                Node& node = nodes_[child];
                if (!node.external)
                {
                    node.external = true;
                    stack.push_back(child);
                }
            }
        }
        auto it = nodes_.find(root_);
        return it != nodes_.end() && it->second.external;
    }

    /// 걸어 들어간 자손 환경 중 바깥에서 닿지 않는 것들
    std::vector<Environment*> collectableEnvironments() const
    {
        std::vector<Environment*> result;
        for (Environment* env : descendants_)
        {
            auto it = nodes_.find(env);
            if (it != nodes_.end() && !it->second.external)
            {
                result.push_back(env);
            }
        }
        return result;
    }

private:
    struct Node
    {
        size_t internal = 0;             ///< 그래프 안에서 발견한 참조 수
        size_t useCount = 0;             ///< 처음 만났을 때의 shared_ptr use_count
        bool skipped = false;            ///< 들어가지 않음 (바깥에서 잡힌 것으로 취급)
        bool expanded = false;
        bool external = false;           ///< 외부 객체에서 닿음
        std::vector<const void*> children;
    };

    Environment* root_;
    std::unordered_set<const void*> ancestors_;
    std::unordered_map<const void*, Node> nodes_;
    std::vector<Environment*> descendants_;
    std::vector<std::function<void()>> pending_;

    bool isDescendantOfRoot(const Environment& env) const
    {
        for (auto outer = env.outer(); outer; outer = outer->outer())
        {
            if (outer.get() == root_)
            {
                return true;
            }
            if (ancestors_.count(outer.get()))
            {
                return false;
            }
        }
        return false;
    }

    // 간선 parent -> target 을 기록한다. 처음 만난 대상은 (들어가도 되면) 안으로 들어간다.
    template <typename T, typename Expand>
    void edge(const void* parent, const std::shared_ptr<T>& target, bool enter, Expand expand)
    {
        if (!target)
        {
            return;
        }
        const void* key = target.get();
        nodes_[parent].children.push_back(key);
        Node& node = nodes_[key];
        if (node.internal == 0)
        {
            node.useCount = static_cast<size_t>(target.use_count());
        }
        ++node.internal;
        if (node.expanded || node.skipped)
        {
            return;
        }
        if (!enter)
        {
            node.skipped = true;
            return;
        }
        node.expanded = true;
        // raw pointer 만 잡아 use_count 를 바꾸지 않는다. 호출자가 root 핸들을 쥐고 있고
        // 걷는 동안 아무것도 바꾸지 않으므로 대상은 살아 있다.
        T* raw = target.get();
        pending_.push_back([raw, expand]() { expand(*raw); });
    }

    void edgeEnvironment(const void* parent, const std::shared_ptr<Environment>& env)
    {
        if (!env)
        {
            return;
        }
        if (env.get() == root_)
        {
            nodes_[parent].children.push_back(root_);
            ++nodes_[root_].internal;
            return;
        }
        if (ancestors_.count(env.get()))
        {
            // 바깥 스코프는 root 를 저장소에 담지 않는 한 root 에 닿지 못하고, 담았다면 그
            // 참조는 여기서 발견되지 않아 수가 맞지 않게 된다. 전역 환경 전체를 걷는 비용을 피한다.
            return;
        }
        bool enter = isDescendantOfRoot(*env);
        edge(parent, env, enter, [this](Environment& e) {
            descendants_.push_back(&e);
            for (const auto& [name, value] : e.getAllBindings())
            {
                visitValue(&e, value);
            }
            edgeEnvironment(&e, e.outer());
        });
    }

    void edgeFunction(const void* parent, const std::shared_ptr<Function>& fn)
    {
        edge(parent, fn, true, [this](Function& f) {
            edgeEnvironment(&f, f.closure());
            edgeClassDef(&f, f.classDef());
            edgeClassDef(&f, f.superClassDef());  // '상위' 가 가리킬 부모 클래스
        });
    }

    void edgeClassDef(const void* parent, const std::shared_ptr<ClassDefinition>& def)
    {
        edge(parent, def, true, [this](ClassDefinition& d) {
            for (const auto& [name, method] : d.methods())
            {
                edgeFunction(&d, method);
            }
            edgeFunction(&d, d.constructor());
        });
    }

    void edgeInstance(const void* parent, const std::shared_ptr<ClassInstance>& instance)
    {
        edge(parent, instance, true, [this](ClassInstance& inst) {
            edgeClassDef(&inst, inst.classDef());
            if (inst.isSuperProxy())
            {
                // '상위' 프록시의 필드는 원래 인스턴스의 것이다. 원래 인스턴스로 가는 참조 하나만 센다
                edgeInstance(&inst, inst.target());
                return;
            }
            for (const auto& [k, v] : inst.fields())
            {
                visitValue(&inst, v);
            }
        });
    }

    void visitValue(const void* parent, const Value& value)
    {
        std::visit(
            [this, parent](const auto& item) {
                using T = std::decay_t<decltype(item)>;
                if constexpr (std::is_same_v<T, std::shared_ptr<Function>>)
                {
                    edgeFunction(parent, item);
                }
                else if constexpr (std::is_same_v<T, std::shared_ptr<std::vector<Value>>>)
                {
                    edge(parent, item, item.use_count() == 1, [this](std::vector<Value>& arr) {
                        for (const auto& v : arr)
                        {
                            visitValue(&arr, v);
                        }
                    });
                }
                else if constexpr (std::is_same_v<T, std::shared_ptr<std::unordered_map<std::string, Value>>>)
                {
                    edge(parent, item, item.use_count() == 1, [this](std::unordered_map<std::string, Value>& dict) {
                        for (const auto& [k, v] : dict)
                        {
                            visitValue(&dict, v);
                        }
                    });
                }
                else if constexpr (std::is_same_v<T, std::shared_ptr<ClassInstance>>)
                {
                    edgeInstance(parent, item);
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

    ReachabilityGraph graph(this);
    graph.walkStore(store_);

    if (useCount != externalHandles + graph.rootInternalRefs())
    {
        return false;
    }
    if (graph.markReachableFromOutside())
    {
        return false;
    }

    // root 에 닿는 모든 참조가 저장소 안에서 출발했다. root 와, 역시 바깥에서 닿지 않는 자손
    // 환경의 저장소를 비우면 사슬이 끊겨 전부 해제된다. 자손의 저장소를 먼저 비운다: root 를
    // 비우는 순간 자손이 해제될 수 있기 때문이다.
    // 저장소를 비우는 순간 다른 자손 환경이 해제될 수 있으므로(그 환경을 잡던 마지막 함수가
    // 이 저장소에 있던 경우), 먼저 모든 저장소를 떼어내 한곳에 모아 두었다가 마지막에 한꺼번에
    // 버린다. 떼어내는 동안은 아무것도 해제되지 않아 포인터가 전부 유효하다.
    std::vector<std::unordered_map<std::string, Value>> detached;
    for (Environment* env : graph.collectableEnvironments())
    {
        detached.emplace_back(std::move(env->store_));
        env->store_.clear();
    }
    detached.emplace_back(std::move(store_));
    store_.clear();
    return true;
}

} // namespace evaluator
} // namespace kingsejong
