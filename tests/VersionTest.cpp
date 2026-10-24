/**
 * @file VersionTest.cpp
 * @brief 버전 문자열이 CMake 의 project() 버전 한 곳에서 나오는지 확인한다
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "version/Version.h"
#include <regex>

using namespace kingsejong;
using ::testing::HasSubstr;
using ::testing::StartsWith;

TEST(VersionTest, ShouldComeFromBuildDefinition)
{
    // CMake 가 KINGSEJONG_VERSION 을 넘기므로 헤더의 대체값 0.0.0 이 보이면 빌드 설정이 빠진 것이다
    EXPECT_STRNE(version::VERSION, "0.0.0");
    EXPECT_TRUE(std::regex_match(version::VERSION, std::regex(R"(\d+\.\d+\.\d+)"))) << version::VERSION;
}

TEST(VersionTest, ShouldFormatVersionStrings)
{
    EXPECT_EQ(version::getVersionString(), std::string("KingSejong v") + version::VERSION);
    EXPECT_THAT(version::getFullVersionString(), StartsWith(version::getVersionString() + "\nBuild: "));
}
