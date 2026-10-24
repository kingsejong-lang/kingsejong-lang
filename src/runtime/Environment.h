#pragma once

/**
 * @file Environment.h
 * @brief KingSejong 언어 변수 환경 (스코프)
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "runtime/Value.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include <atomic>
#include <vector>

namespace kingsejong {
namespace evaluator {

/**
 * @class Environment
 * @brief 변수 저장 및 스코프 관리
 *
 * 변수를 이름으로 저장하고 조회합니다.
 * 중첩 스코프를 지원하며, 외부 스코프로의 체인을 유지합니다.
 */
class Environment : public std::enable_shared_from_this<Environment>
{
private:
    std::unordered_map<std::string, Value> store_;  ///< 변수 저장소
    std::shared_ptr<Environment> outer_;            ///< 외부(부모) 스코프
    static std::atomic<size_t> liveCount_;          ///< 살아 있는 Environment 개수

public:
    /**
     * @brief 기본 생성자
     *
     * 최상위 스코프(글로벌 스코프)를 생성합니다.
     */
    Environment();

    /**
     * @brief 외부 스코프를 지정하는 생성자
     * @param outer 외부(부모) 스코프
     *
     * 중첩 스코프를 생성할 때 사용합니다.
     */
    explicit Environment(std::shared_ptr<Environment> outer);

    ~Environment();

    /**
     * @brief 현재 살아 있는 Environment 개수
     *
     * 순환 참조 해제가 실제로 동작하는지 테스트에서 확인하는 용도다.
     */
    static size_t liveCount() { return liveCount_.load(); }

    /**
     * @brief 이 환경이 더 이상 바깥에서 닿을 수 없으면 저장소를 비워 순환 참조를 끊는다
     * @param externalHandles 호출자가 들고 있는 이 환경의 shared_ptr 개수
     * @return 저장소를 비웠으면 true
     *
     * 함수는 정의된 환경을 shared_ptr 로 강참조하고, 환경은 그 함수를 변수로 저장한다.
     * 이름 붙은 함수(재귀 포함)를 선언한 스코프는 그래서 refcount 만으로는 영원히 해제되지
     * 않는다. 스코프를 떠날 때 이 함수를 부르면, 이 환경을 붙잡고 있는 참조가 전부
     * 저장소 안에서 출발한 것(저장소 → 함수 → closure → 이 환경)인지 참조 수를 대조한다.
     * 그렇다면 호출자가 손을 놓는 순간 아무도 닿을 수 없는 쓰레기이므로 저장소를 비운다.
     * 반환값, 바깥 변수, 다른 환경 등 어디서든 한 번이라도 더 참조되면 손대지 않는다
     * (보수적: 잘못 비우는 일은 없고, 못 비우는 일만 있을 수 있다).
     */
    bool releaseIfUnreachable(size_t externalHandles);

    /**
     * @brief 변수 설정
     * @param name 변수 이름
     * @param value 저장할 값
     *
     * 현재 스코프에 변수를 저장합니다.
     * 같은 이름의 변수가 있으면 덮어씁니다.
     */
    void set(const std::string& name, const Value& value);

    /**
     * @brief 변수 설정 (스코프 체인 탐색)
     * @param name 변수 이름
     * @param value 저장할 값
     *
     * 스코프 체인을 따라 변수를 찾아 업데이트합니다.
     * 1. 현재 스코프에 변수가 있으면 현재 스코프에서 업데이트
     * 2. 외부 스코프에 변수가 있으면 외부 스코프에서 업데이트 (재귀)
     * 3. 어디에도 없으면 현재 스코프에 새로 생성
     *
     * JavaScript/Python 스타일의 변수 할당 동작을 구현합니다.
     */
    void setWithLookup(const std::string& name, const Value& value);

    /**
     * @brief 변수 조회
     * @param name 변수 이름
     * @return 저장된 값
     * @throws std::runtime_error 변수를 찾을 수 없는 경우
     *
     * 현재 스코프에서 변수를 찾습니다.
     * 없으면 외부 스코프를 재귀적으로 탐색합니다.
     */
    Value get(const std::string& name) const;

    /**
     * @brief 변수 존재 확인 (현재 스코프만)
     * @param name 변수 이름
     * @return 현재 스코프에 변수가 있으면 true
     *
     * 외부 스코프는 확인하지 않습니다.
     */
    bool exists(const std::string& name) const;

    /**
     * @brief 변수 존재 확인 (스코프 체인 전체)
     * @param name 변수 이름
     * @return 현재 또는 외부 스코프에 변수가 있으면 true
     *
     * 현재 스코프부터 외부 스코프까지 재귀적으로 탐색합니다.
     */
    bool existsInChain(const std::string& name) const;

    /**
     * @brief 중첩 스코프 생성
     * @return 현재 Environment를 외부 스코프로 하는 새 Environment
     *
     * 함수 호출, 블록 스코프 등에서 사용합니다.
     */
    std::shared_ptr<Environment> createEnclosed();

    /**
     * @brief 외부 스코프 반환
     * @return 외부 스코프 (없으면 nullptr)
     */
    const std::shared_ptr<Environment>& outer() const { return outer_; }

    /**
     * @brief 저장된 변수 개수 (현재 스코프만)
     * @return 변수 개수
     */
    size_t size() const { return store_.size(); }

    /**
     * @brief 모든 변수 제거 (현재 스코프만)
     */
    void clear() { store_.clear(); }

    /**
     * @brief 변수 이름 목록 반환 (현재 스코프만)
     * @return 변수 이름 벡터
     */
    std::vector<std::string> keys() const;

    /**
     * @brief 모든 변수 바인딩 반환 (현재 스코프만)
     * @return 변수명-값 맵의 복사본
     */
    const std::unordered_map<std::string, Value>& getAllBindings() const { return store_; }
};

} // namespace evaluator
} // namespace kingsejong
