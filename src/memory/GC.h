#pragma once

/**
 * @file GC.h
 * @brief KingSejong 언어 가비지 컬렉터
 * @author KingSejong Team
 * @date 2025-11-10
 *
 * Mark & Sweep 알고리즘을 사용하는 가비지 컬렉터입니다.
 * shared_ptr 기반 메모리 관리와 함께 동작하며,
 * 순환 참조를 감지하고 메모리 누수를 방지합니다.
 */

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace kingsejong {

// Forward declarations
namespace evaluator {
    class Environment;
    class Value;
    class Function;
}

namespace memory {

/**
 * @class Object
 * @brief GC가 관리하는 객체의 베이스 클래스
 *
 * 모든 GC 관리 객체는 이 클래스를 상속받습니다.
 * Mark bit와 고유 ID를 가집니다.
 */
class Object {
private:
    bool marked_ = false;      ///< Mark phase에서 설정되는 플래그
    uint64_t id_;              ///< 객체 고유 ID
    static uint64_t nextId_;   ///< 다음 할당될 ID

public:
    Object() : id_(nextId_++) {}
    virtual ~Object() = default;

    /**
     * @brief 객체를 마킹
     */
    void mark() { marked_ = true; }

    /**
     * @brief 마킹 해제
     */
    void unmark() { marked_ = false; }

    /**
     * @brief 마킹 상태 확인
     * @return 마킹되어 있으면 true
     */
    bool isMarked() const { return marked_; }

    /**
     * @brief 객체 ID 반환
     * @return 고유 ID
     */
    uint64_t getId() const { return id_; }

    /**
     * @brief 이 객체가 참조하는 다른 객체들을 반환
     * @return 참조하는 객체들의 포인터 벡터
     *
     * Mark phase에서 재귀적으로 마킹할 때 사용됩니다.
     */
    virtual std::vector<Object*> getReferences() { return {}; }
};

/**
 * @struct GCStats
 * @brief GC 통계 정보
 */
struct GCStats {
    size_t totalAllocations = 0;    ///< 총 할당 횟수
    size_t totalDeallocations = 0;  ///< 총 해제 횟수
    size_t currentObjects = 0;      ///< 현재 관리 중인 객체 수
    size_t gcRunCount = 0;          ///< GC 실행 횟수
    size_t objectsFreed = 0;        ///< 마지막 GC에서 해제된 객체 수
    size_t totalObjectsFreed = 0;   ///< 총 GC로 해제된 객체 수
    size_t bytesAllocated = 0;      ///< 할당된 메모리 (추정)
};

/**
 * @class GarbageCollector
 * @brief 가비지 컬렉터 (Singleton)
 *
 * Mark & Sweep 알고리즘을 구현합니다.
 * - Mark phase: 루트 집합부터 도달 가능한 모든 객체를 마킹
 * - Sweep phase: 마킹되지 않은 객체를 해제
 */
class GarbageCollector {
private:
    // Singleton instance
    static GarbageCollector* instance_;

    // 관리 중인 모든 객체들
    std::unordered_set<Object*> allObjects_;

    // Root set (GC의 출발점)
    std::unordered_set<Object*> roots_;

    // Environment를 weak_ptr로 추적 (순환 참조 방지)
    std::unordered_map<void*, std::weak_ptr<evaluator::Environment>> environments_;

    // 통계
    GCStats stats_;

    // GC 설정
    size_t gcThreshold_ = 100;     ///< GC를 트리거할 객체 수
    bool autoGC_ = true;           ///< 자동 GC 활성화 여부

    // Private constructor (Singleton)
    GarbageCollector() = default;

public:
    // Singleton access
    static GarbageCollector* getInstance();

    // 복사/이동 금지
    GarbageCollector(const GarbageCollector&) = delete;
    GarbageCollector& operator=(const GarbageCollector&) = delete;

    /**
     * @brief 객체를 GC에 등록
     * @param obj 등록할 객체 포인터
     *
     * 새로운 객체가 할당될 때 호출됩니다.
     */
    void registerObject(Object* obj);

    /**
     * @brief 객체를 GC에서 제거
     * @param obj 제거할 객체 포인터
     *
     * 객체가 소멸될 때 호출됩니다.
     */
    void unregisterObject(Object* obj);

    /**
     * @brief 객체를 루트 집합에 추가
     * @param obj 루트로 추가할 객체
     *
     * 루트는 GC의 시작점입니다 (전역 변수, 스택 등).
     */
    void addRoot(Object* obj);

    /**
     * @brief 객체를 루트 집합에서 제거
     * @param obj 루트에서 제거할 객체
     */
    void removeRoot(Object* obj);

    /**
     * @brief Environment를 등록
     * @param env Environment shared_ptr
     *
     * Environment는 순환 참조 가능성이 있으므로 weak_ptr로 추적합니다.
     */
    void registerEnvironment(std::shared_ptr<evaluator::Environment> env);

    /**
     * @brief Environment를 등록 해제
     * @param env Environment 포인터
     */
    void unregisterEnvironment(void* envPtr);

    /**
     * @brief 가비지 컬렉션 수행
     * @return 해제된 객체 수
     *
     * Mark & Sweep 알고리즘을 실행합니다.
     */
    size_t collect();

    /**
     * @brief Mark phase 수행
     *
     * 루트 집합부터 시작하여 도달 가능한 모든 객체를 마킹합니다.
     */
    void markPhase();

    /**
     * @brief Sweep phase 수행
     * @return 해제된 객체 수
     *
     * 마킹되지 않은 객체를 해제합니다.
     */
    size_t sweepPhase();

    /**
     * @brief 객체를 재귀적으로 마킹
     * @param obj 마킹할 객체
     */
    void markObject(Object* obj);

    /**
     * @brief GC 통계 반환
     * @return GCStats 구조체
     */
    const GCStats& getStats() const { return stats_; }

    /**
     * @brief GC 통계를 문자열로 출력
     * @return 통계 문자열
     */
    std::string getStatsString() const;

    /**
     * @brief 자동 GC 활성화/비활성화
     * @param enable true면 활성화
     */
    void setAutoGC(bool enable) { autoGC_ = enable; }

    /**
     * @brief GC 트리거 임계값 설정
     * @param threshold 객체 수 임계값
     */
    void setGCThreshold(size_t threshold) { gcThreshold_ = threshold; }

    /**
     * @brief 자동 GC 확인 및 실행
     *
     * 객체 수가 임계값을 넘으면 자동으로 GC를 실행합니다.
     */
    void checkAndCollect();

    /**
     * @brief 모든 객체 해제 (프로그램 종료 시)
     */
    void cleanup();

    /**
     * @brief 순환 참조 감지
     * @return 순환 참조가 감지되면 true
     */
    bool detectCycles();
};

} // namespace memory
} // namespace kingsejong
