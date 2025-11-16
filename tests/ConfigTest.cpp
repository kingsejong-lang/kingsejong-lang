/**
 * @file ConfigTest.cpp
 * @brief 설정 파일 로더 테스트
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include <gtest/gtest.h>
#include "config/ConfigLoader.h"
#include "linter/Linter.h"
#include "linter/LinterConfig.h"
#include "formatter/Formatter.h"
#include "formatter/FormatterConfig.h"
#include <nlohmann/json.hpp>

using namespace kingsejong::config;
using namespace kingsejong::linter;
using namespace kingsejong::formatter;
using json = nlohmann::json;

/**
 * @test JSON 문자열에서 설정 로드
 */
TEST(ConfigTest, ShouldLoadFromString)
{
    std::string jsonStr = R"({
        "test": "value",
        "number": 42
    })";

    json config;
    ASSERT_TRUE(ConfigLoader::loadFromString(jsonStr, config));
    EXPECT_EQ(config["test"], "value");
    EXPECT_EQ(config["number"], 42);
}

/**
 * @test 잘못된 JSON 문자열
 */
TEST(ConfigTest, ShouldRejectInvalidJSON)
{
    std::string jsonStr = "{ invalid json }";

    json config;
    EXPECT_FALSE(ConfigLoader::loadFromString(jsonStr, config));
}

/**
 * @test Linter 설정 로드
 */
TEST(ConfigTest, ShouldLoadLinterConfig)
{
    std::string jsonStr = R"({
        "rules": {
            "unused-variable": "error",
            "dead-code": "warning",
            "empty-block": "off"
        }
    })";

    json configJson;
    ASSERT_TRUE(ConfigLoader::loadFromString(jsonStr, configJson));

    LinterConfig config;
    ASSERT_TRUE(config.loadFromJson(configJson));

    // 규칙 활성화 확인
    EXPECT_TRUE(config.isRuleEnabled("unused-variable"));
    EXPECT_TRUE(config.isRuleEnabled("dead-code"));
    EXPECT_FALSE(config.isRuleEnabled("empty-block"));  // off

    // 심각도 확인
    auto severity1 = config.getRuleSeverity("unused-variable");
    ASSERT_TRUE(severity1.has_value());
    EXPECT_EQ(severity1.value(), IssueSeverity::ERROR);

    auto severity2 = config.getRuleSeverity("dead-code");
    ASSERT_TRUE(severity2.has_value());
    EXPECT_EQ(severity2.value(), IssueSeverity::WARNING);
}

/**
 * @test Formatter 설정 로드
 */
TEST(ConfigTest, ShouldLoadFormatterConfig)
{
    std::string jsonStr = R"({
        "indentSize": 2,
        "useSpaces": false,
        "spaceAroundOperators": false,
        "maxLineLength": 120
    })";

    json configJson;
    ASSERT_TRUE(ConfigLoader::loadFromString(jsonStr, configJson));

    FormatterOptions options;
    ASSERT_TRUE(FormatterConfig::loadFromJson(configJson, options));

    EXPECT_EQ(options.indentSize, 2);
    EXPECT_FALSE(options.useSpaces);
    EXPECT_FALSE(options.spaceAroundOperators);
    EXPECT_EQ(options.maxLineLength, 120);
}

/**
 * @test Linter에 설정 적용
 */
TEST(ConfigTest, ShouldApplyLinterConfig)
{
    std::string jsonStr = R"({
        "rules": {
            "unused-variable": "off"
        }
    })";

    Linter linter;
    ASSERT_TRUE(linter.loadConfigFromString(jsonStr));

    // 규칙이 비활성화되었는지 확인
    EXPECT_FALSE(linter.isRuleEnabled("unused-variable"));

    // 설정되지 않은 규칙은 기본 활성화
    EXPECT_TRUE(linter.isRuleEnabled("dead-code"));
}

/**
 * @test Formatter에 설정 적용
 */
TEST(ConfigTest, ShouldApplyFormatterConfig)
{
    std::string jsonStr = R"({
        "indentSize": 8,
        "useSpaces": false
    })";

    Formatter formatter;
    ASSERT_TRUE(formatter.loadConfigFromString(jsonStr));

    const auto& options = formatter.getOptions();
    EXPECT_EQ(options.indentSize, 8);
    EXPECT_FALSE(options.useSpaces);
}

/**
 * @test Linter 규칙 심각도 변경
 */
TEST(ConfigTest, ShouldChangeLinterRuleSeverity)
{
    std::string jsonStr = R"({
        "rules": {
            "no-magic-number": "error"
        }
    })";

    Linter linter;
    ASSERT_TRUE(linter.loadConfigFromString(jsonStr));

    auto severity = linter.getRuleSeverity("no-magic-number");
    ASSERT_TRUE(severity.has_value());
    EXPECT_EQ(severity.value(), IssueSeverity::ERROR);
}

/**
 * @test 부분 설정 (일부만 지정)
 */
TEST(ConfigTest, ShouldHandlePartialConfig)
{
    std::string jsonStr = R"({
        "indentSize": 2
    })";

    FormatterOptions options;
    options.useSpaces = true;  // 기본값
    options.maxLineLength = 100;  // 기본값

    json configJson;
    ASSERT_TRUE(ConfigLoader::loadFromString(jsonStr, configJson));
    ASSERT_TRUE(FormatterConfig::loadFromJson(configJson, options));

    // 지정된 값만 변경
    EXPECT_EQ(options.indentSize, 2);
    // 나머지는 기존 값 유지
    EXPECT_TRUE(options.useSpaces);
    EXPECT_EQ(options.maxLineLength, 100);
}
