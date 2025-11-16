#pragma once

/**
 * @file ConfigLoader.h
 * @brief 설정 파일 로더
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include <string>
#include <unordered_map>
#include <optional>
#include <nlohmann/json.hpp>

namespace kingsejong {
namespace config {

using json = nlohmann::json;

/**
 * @class ConfigLoader
 * @brief JSON 설정 파일을 로드하고 파싱하는 유틸리티 클래스
 */
class ConfigLoader
{
public:
    /**
     * @brief 설정 파일 로드
     * @param filepath 설정 파일 경로
     * @return 로드 성공 시 true
     */
    static bool loadFromFile(const std::string& filepath, json& config);

    /**
     * @brief JSON 문자열에서 설정 로드
     * @param jsonString JSON 문자열
     * @return 파싱 성공 시 true
     */
    static bool loadFromString(const std::string& jsonString, json& config);

    /**
     * @brief 설정 파일 존재 여부 확인
     * @param filepath 파일 경로
     * @return 파일이 존재하면 true
     */
    static bool fileExists(const std::string& filepath);

    /**
     * @brief 현재 디렉토리에서 설정 파일 찾기
     * @param filename 찾을 파일 이름 (예: ".ksjlint.json")
     * @return 파일 경로 (찾지 못하면 빈 문자열)
     */
    static std::string findConfigFile(const std::string& filename);
};

} // namespace config
} // namespace kingsejong
