/**
 * @file TestTempDir.h
 * @brief Per-test scratch directories for tests that touch the filesystem.
 *
 * ctest runs every test in its own process, possibly in parallel (ctest -j).
 * Fixtures that share one fixed directory race with each other: one test's
 * TearDown removes files another test is still using. Deriving the directory
 * name from the running test keeps each test isolated.
 */
#pragma once

#include <gtest/gtest.h>
#include <filesystem>
#include <string>

namespace kingsejong::test {

/**
 * @brief Return a fresh, empty directory unique to the current test.
 * @param prefix Human-readable prefix, e.g. "kingsejong_module_test"
 *
 * The directory lives under the system temporary directory and is named
 * <prefix>_<suite>_<test>. Any leftover from an earlier aborted run is removed first.
 */
inline std::filesystem::path uniqueTestDir(const std::string& prefix)
{
    const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
    auto dir = std::filesystem::temp_directory_path()
               / (prefix + "_" + info->test_suite_name() + "_" + info->name());
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

} // namespace kingsejong::test
