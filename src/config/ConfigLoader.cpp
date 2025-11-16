/**
 * @file ConfigLoader.cpp
 * @brief 설정 파일 로더 구현
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "config/ConfigLoader.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

namespace kingsejong {
namespace config {

bool ConfigLoader::loadFromFile(const std::string& filepath, json& config)
{
    if (!fileExists(filepath))
    {
        return false;
    }

    try
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            return false;
        }

        file >> config;
        return true;
    }
    catch (const json::parse_error& e)
    {
        std::cerr << "JSON 파싱 오류 (" << filepath << "): " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "설정 파일 로드 오류 (" << filepath << "): " << e.what() << std::endl;
        return false;
    }
}

bool ConfigLoader::loadFromString(const std::string& jsonString, json& config)
{
    try
    {
        config = json::parse(jsonString);
        return true;
    }
    catch (const json::parse_error& e)
    {
        std::cerr << "JSON 파싱 오류: " << e.what() << std::endl;
        return false;
    }
}

bool ConfigLoader::fileExists(const std::string& filepath)
{
    return std::filesystem::exists(filepath);
}

std::string ConfigLoader::findConfigFile(const std::string& filename)
{
    // 현재 디렉토리에서 시작하여 상위 디렉토리로 이동하며 설정 파일 찾기
    std::filesystem::path currentPath = std::filesystem::current_path();

    while (true)
    {
        std::filesystem::path configPath = currentPath / filename;
        if (std::filesystem::exists(configPath))
        {
            return configPath.string();
        }

        // 루트 디렉토리에 도달하면 중단
        if (currentPath == currentPath.root_path())
        {
            break;
        }

        // 상위 디렉토리로 이동
        currentPath = currentPath.parent_path();
    }

    return "";  // 찾지 못함
}

} // namespace config
} // namespace kingsejong
