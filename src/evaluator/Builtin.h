#pragma once

/**
 * @file Builtin.h
 * @brief KingSejong 언어 내장 함수 시스템
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include "Value.h"
#include <string>
#include <unordered_map>

namespace kingsejong {
namespace evaluator {

/**
 * @class Builtin
 * @brief 내장 함수 관리 시스템
 *
 * 전역 네임스페이스에 등록된 내장 함수들을 관리합니다.
 * 출력(), 타입(), 길이() 등의 기본 내장 함수를 제공합니다.
 */
class Builtin
{
public:
    /**
     * @brief 내장 함수 등록
     * @param name 함수 이름
     * @param fn 함수 구현체
     */
    static void registerBuiltin(const std::string& name, Value::BuiltinFn fn);

    /**
     * @brief 내장 함수 조회
     * @param name 함수 이름
     * @return Value 객체 (내장 함수 또는 null)
     */
    static Value getBuiltin(const std::string& name);

    /**
     * @brief 내장 함수 존재 확인
     * @param name 함수 이름
     * @return 내장 함수가 존재하면 true
     */
    static bool hasBuiltin(const std::string& name);

    /**
     * @brief 모든 내장 함수 등록 (초기화)
     *
     * 프로그램 시작 시 한 번 호출하여 모든 내장 함수를 등록합니다.
     */
    static void registerAllBuiltins();

private:
    /// 내장 함수 저장소 (함수 이름 → Value)
    static std::unordered_map<std::string, Value> builtins_;
};

} // namespace evaluator
} // namespace kingsejong
