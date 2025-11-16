#pragma once

/**
 * @file FormatterConfig.h
 * @brief Formatter 설정 구조
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "formatter/Formatter.h"
#include <nlohmann/json.hpp>

namespace kingsejong {
namespace formatter {

using json = nlohmann::json;

/**
 * @struct FormatterConfig
 * @brief Formatter 설정 (FormatterOptions 확장)
 */
struct FormatterConfig
{
    /**
     * @brief JSON에서 FormatterOptions 로드
     * @param config JSON 설정 객체
     * @param options 로드할 FormatterOptions 객체
     * @return 로드 성공 시 true
     */
    static bool loadFromJson(const json& config, FormatterOptions& options)
    {
        try
        {
            if (config.contains("indentSize") && config["indentSize"].is_number_integer())
            {
                options.indentSize = config["indentSize"].get<int>();
            }

            if (config.contains("useSpaces") && config["useSpaces"].is_boolean())
            {
                options.useSpaces = config["useSpaces"].get<bool>();
            }

            if (config.contains("spaceAroundOperators") && config["spaceAroundOperators"].is_boolean())
            {
                options.spaceAroundOperators = config["spaceAroundOperators"].get<bool>();
            }

            if (config.contains("spaceAfterComma") && config["spaceAfterComma"].is_boolean())
            {
                options.spaceAfterComma = config["spaceAfterComma"].get<bool>();
            }

            if (config.contains("spaceBeforeBrace") && config["spaceBeforeBrace"].is_boolean())
            {
                options.spaceBeforeBrace = config["spaceBeforeBrace"].get<bool>();
            }

            if (config.contains("maxLineLength") && config["maxLineLength"].is_number_integer())
            {
                options.maxLineLength = config["maxLineLength"].get<int>();
            }

            return true;
        }
        catch (const std::exception& e)
        {
            return false;
        }
    }

    /**
     * @brief FormatterOptions를 JSON으로 변환
     * @param options FormatterOptions 객체
     * @return JSON 객체
     */
    static json toJson(const FormatterOptions& options)
    {
        json config;
        config["indentSize"] = options.indentSize;
        config["useSpaces"] = options.useSpaces;
        config["spaceAroundOperators"] = options.spaceAroundOperators;
        config["spaceAfterComma"] = options.spaceAfterComma;
        config["spaceBeforeBrace"] = options.spaceBeforeBrace;
        config["maxLineLength"] = options.maxLineLength;
        return config;
    }
};

} // namespace formatter
} // namespace kingsejong
