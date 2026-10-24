/**
 * @file Version.h
 * @brief KingSejong 언어 버전 정보
 *
 * 버전 번호는 CMakeLists.txt 의 project(... VERSION ...) 한 곳에서 정하고
 * 컴파일 정의 KINGSEJONG_VERSION 으로 들어온다. --version, REPL 배너, LSP serverInfo 가 이 값을 쓴다.
 */

#pragma once

#include <string>

#ifndef KINGSEJONG_VERSION
#define KINGSEJONG_VERSION "0.0.0"
#endif

namespace kingsejong {
namespace version {

// 버전 번호 (예: "0.5.0")
constexpr const char* VERSION = KINGSEJONG_VERSION;

// 빌드 정보
constexpr const char* BUILD_DATE = __DATE__;
constexpr const char* BUILD_TIME = __TIME__;

/**
 * @brief 전체 버전 문자열 반환
 * @return 버전 문자열 (예: "KingSejong v0.5.0")
 */
inline std::string getVersionString() {
    return std::string("KingSejong v") + VERSION;
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
