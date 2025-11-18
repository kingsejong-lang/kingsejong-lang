/**
 * @file GC.cpp
 * @brief KingSejong 언어 가비지 컬렉터 구현
 * @author KingSejong Team
 * @date 2025-11-10
 */

#include "GC.h"
#include "evaluator/Environment.h"
#include "evaluator/Value.h"
#include <sstream>
#include <algorithm>

namespace kingsejong {
namespace memory {

// Static member initialization
GarbageCollector* GarbageCollector::instance_ = nullptr;
uint64_t Object::nextId_ = 1;

GarbageCollector* GarbageCollector::getInstance() {
    if (instance_ == nullptr) {
        instance_ = new GarbageCollector();
    }
    return instance_;
}

void GarbageCollector::registerObject(Object* obj) {
    if (obj == nullptr) return;

    allObjects_.insert(obj);

    // Phase 7.4: 새 객체는 Young Generation에 추가
    youngGeneration_.insert(obj);
    obj->setGeneration(Generation::YOUNG);

    stats_.totalAllocations++;
    stats_.currentObjects++;
    stats_.youngObjects++;
    stats_.bytesAllocated += sizeof(Object); // 대략적인 추정

    // 자동 GC 확인
    checkAndCollect();
}

void GarbageCollector::unregisterObject(Object* obj) {
    if (obj == nullptr) return;

    allObjects_.erase(obj);
    roots_.erase(obj);

    // Phase 7.4: 두 세대에서 모두 제거
    if (youngGeneration_.erase(obj) > 0) {
        if (stats_.youngObjects > 0) stats_.youngObjects--;
    } else if (oldGeneration_.erase(obj) > 0) {
        if (stats_.oldObjects > 0) stats_.oldObjects--;
    }

    stats_.totalDeallocations++;

    if (stats_.currentObjects > 0) {
        stats_.currentObjects--;
    }
}

void GarbageCollector::addRoot(Object* obj) {
    if (obj == nullptr) return;
    roots_.insert(obj);
}

void GarbageCollector::removeRoot(Object* obj) {
    if (obj == nullptr) return;
    roots_.erase(obj);
}

void GarbageCollector::registerEnvironment(std::shared_ptr<evaluator::Environment> env) {
    if (env == nullptr) return;

    void* ptr = env.get();
    environments_[ptr] = std::weak_ptr<evaluator::Environment>(env);

    stats_.totalAllocations++;
    stats_.currentObjects++;
}

void GarbageCollector::unregisterEnvironment(void* envPtr) {
    if (envPtr == nullptr) return;

    environments_.erase(envPtr);

    if (stats_.currentObjects > 0) {
        stats_.currentObjects--;
    }
    stats_.totalDeallocations++;
}

size_t GarbageCollector::collect() {
    stats_.gcRunCount++;

    // Phase 7.4: Young Generation이 많으면 Minor GC, 아니면 Major GC
    // Young이 전체의 70% 이상이면 Minor GC
    if (stats_.youngObjects > stats_.currentObjects * 0.7) {
        return minorGC();
    } else {
        return majorGC();
    }
}

size_t GarbageCollector::minorGC() {
    stats_.minorGCCount++;

    // 1. Young Generation 객체만 마크 해제
    for (Object* obj : youngGeneration_) {
        obj->unmark();
    }

    // 2. 루트부터 도달 가능한 객체 마킹
    for (Object* root : roots_) {
        markObject(root);
    }

    // 3. Environment 추적
    for (auto it = environments_.begin(); it != environments_.end();) {
        if (auto env = it->second.lock()) {
            ++it;
        } else {
            it = environments_.erase(it);
        }
    }

    // 4. Sweep phase: 마킹되지 않은 Young 객체 해제 & 승격
    std::vector<Object*> toDelete;
    std::vector<Object*> toPromote;

    for (Object* obj : youngGeneration_) {
        if (!obj->isMarked()) {
            toDelete.push_back(obj);
        } else {
            // 마킹된 객체는 나이 증가
            obj->incrementAge();

            // 나이가 임계값을 넘으면 승격 대상
            if (obj->getAge() >= promotionAge_) {
                toPromote.push_back(obj);
            }
        }
    }

    // 5. 객체 승격
    for (Object* obj : toPromote) {
        promoteObject(obj);
    }

    // 6. 객체 삭제
    for (Object* obj : toDelete) {
        youngGeneration_.erase(obj);
        allObjects_.erase(obj);
        delete obj;
    }

    size_t freedCount = toDelete.size();
    stats_.objectsFreed = freedCount;
    stats_.totalObjectsFreed += freedCount;
    stats_.currentObjects -= freedCount;
    stats_.youngObjects -= freedCount;

    return freedCount;
}

size_t GarbageCollector::majorGC() {
    stats_.majorGCCount++;

    // 기존 Mark & Sweep 알고리즘 (전체 객체 대상)

    // 1. 모든 객체 마크 해제
    for (Object* obj : allObjects_) {
        obj->unmark();
    }

    // 2. Mark phase: 루트부터 도달 가능한 모든 객체 마킹
    markPhase();

    // 3. Sweep phase: 마킹되지 않은 객체 해제
    size_t freedCount = sweepPhase();

    stats_.objectsFreed = freedCount;
    stats_.totalObjectsFreed += freedCount;

    return freedCount;
}

void GarbageCollector::markPhase() {
    // 1. 모든 객체의 마크 해제
    for (Object* obj : allObjects_) {
        obj->unmark();
    }

    // 2. 루트 집합부터 재귀적으로 마킹
    for (Object* root : roots_) {
        markObject(root);
    }

    // 3. Environment 루트들도 마킹
    // (Environment는 Object를 상속하지 않으므로 별도 처리)
    // 현재는 weak_ptr만 추적하므로 실제 마킹은 하지 않음
    // 나중에 Environment가 Object를 상속하면 여기서 마킹
}

size_t GarbageCollector::sweepPhase() {
    size_t freedCount = 0;
    std::vector<Object*> toDelete;

    // 마킹되지 않은 객체 수집
    for (Object* obj : allObjects_) {
        if (!obj->isMarked()) {
            toDelete.push_back(obj);
        }
    }

    // 객체 해제
    for (Object* obj : toDelete) {
        allObjects_.erase(obj);

        // Phase 7.4: 세대별 집합에서도 제거
        if (youngGeneration_.erase(obj) > 0) {
            if (stats_.youngObjects > 0) stats_.youngObjects--;
        } else if (oldGeneration_.erase(obj) > 0) {
            if (stats_.oldObjects > 0) stats_.oldObjects--;
        }

        delete obj;
        freedCount++;

        if (stats_.currentObjects > 0) {
            stats_.currentObjects--;
        }
    }

    // 만료된 Environment weak_ptr 정리
    std::vector<void*> expiredEnvs;
    for (auto& pair : environments_) {
        if (pair.second.expired()) {
            expiredEnvs.push_back(pair.first);
        }
    }

    for (void* ptr : expiredEnvs) {
        environments_.erase(ptr);
    }

    return freedCount;
}

void GarbageCollector::markObject(Object* obj) {
    if (obj == nullptr || obj->isMarked()) {
        return; // 이미 마킹되었거나 nullptr
    }

    // 객체 마킹
    obj->mark();

    // 이 객체가 참조하는 다른 객체들도 재귀적으로 마킹
    std::vector<Object*> refs = obj->getReferences();
    for (Object* ref : refs) {
        markObject(ref);
    }
}

void GarbageCollector::checkAndCollect() {
    if (!autoGC_) return;

    // 객체 수가 임계값을 넘으면 GC 실행
    if (stats_.currentObjects >= gcThreshold_) {
        collect();
    }
}

void GarbageCollector::cleanup() {
    // 모든 객체 해제
    std::vector<Object*> toDelete(allObjects_.begin(), allObjects_.end());
    for (Object* obj : toDelete) {
        delete obj;
    }

    allObjects_.clear();
    roots_.clear();
    environments_.clear();

    // Phase 7.4: 세대별 집합도 초기화
    youngGeneration_.clear();
    oldGeneration_.clear();

    // 통계 완전 초기화
    stats_ = GCStats();
}

std::string GarbageCollector::getStatsString() const {
    std::ostringstream oss;
    oss << "=== 가비지 컬렉터 통계 ===\n";
    oss << "총 할당: " << stats_.totalAllocations << "\n";
    oss << "총 해제: " << stats_.totalDeallocations << "\n";
    oss << "현재 객체 수: " << stats_.currentObjects << "\n";
    oss << "GC 실행 횟수: " << stats_.gcRunCount << "\n";
    oss << "마지막 GC에서 해제: " << stats_.objectsFreed << "\n";
    oss << "총 GC로 해제: " << stats_.totalObjectsFreed << "\n";
    oss << "할당된 메모리 (추정): " << stats_.bytesAllocated << " bytes\n";
    return oss.str();
}

bool GarbageCollector::detectCycles() {
    // 간단한 순환 참조 감지: 모든 Environment가 약한 참조를 가지고 있는지 확인
    // 실제로는 더 복잡한 알고리즘이 필요하지만, 현재 구조에서는
    // shared_ptr 순환을 완전히 추적하기 어려움

    // 만료된 weak_ptr 개수 확인
    size_t expiredCount = 0;
    for (const auto& pair : environments_) {
        if (pair.second.expired()) {
            expiredCount++;
        }
    }

    // 만료되지 않은 Environment가 많으면 순환 참조 가능성
    size_t aliveCount = environments_.size() - expiredCount;

    // 휴리스틱: 살아있는 Environment가 100개 이상이면 순환 참조 의심
    return aliveCount > 100;
}

void GarbageCollector::promoteObject(Object* obj) {
    if (obj == nullptr) return;
    if (obj->getGeneration() != Generation::YOUNG) return;

    // Young에서 Old로 이동
    youngGeneration_.erase(obj);
    oldGeneration_.insert(obj);

    // 세대 변경
    obj->setGeneration(Generation::OLD);
    obj->resetAge();

    // 통계 업데이트
    if (stats_.youngObjects > 0) stats_.youngObjects--;
    stats_.oldObjects++;
    stats_.promotions++;
}

} // namespace memory
} // namespace kingsejong
