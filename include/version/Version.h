/**
 * @file Version.h
 * @brief KingSejong 언어 버전 정보
 * @author KingSejong Team
 * @date 2025-11-17
 */

#pragma once

#include <string>

namespace kingsejong {
namespace version {

// 버전 번호
constexpr const char* VERSION = "0.4.0";
constexpr const char* VERSION_NAME = "JIT Tier 1";

// 빌드 정보
constexpr const char* BUILD_DATE = __DATE__;
constexpr const char* BUILD_TIME = __TIME__;

/**
 * @brief 전체 버전 문자열 반환
 * @return 버전 문자열 (예: "KingSejong v0.4.0 (JIT Tier 1)")
 */
inline std::string getVersionString() {
    return std::string("KingSejong v") + VERSION + " (" + VERSION_NAME + ")";
}

/**
 * @brief 상세 버전 정보 반환
 * @return 상세 버전 문자열 (빌드 날짜/시간 포함)
 */
inline std::string getFullVersionString() {
    return getVersionString() + "\nBuild: " + BUILD_DATE + " " + BUILD_TIME;
}

} // namespace version
} // namespace kingsejong
