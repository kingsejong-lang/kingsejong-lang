#pragma once

/**
 * @file Environment.h
 * @brief KingSejong 언어 변수 환경 (스코프)
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Value.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

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
    std::shared_ptr<Environment> outer() const { return outer_; }

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
};

} // namespace evaluator
} // namespace kingsejong
