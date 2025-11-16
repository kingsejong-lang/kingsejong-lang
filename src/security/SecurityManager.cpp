/**
 * @file SecurityManager.cpp
 * @brief 보안 관리자 구현
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "security/SecurityManager.h"
#include <filesystem>
#include <algorithm>

namespace kingsejong {
namespace security {

namespace fs = std::filesystem;

// 정적 멤버 초기화
SecurityMode SecurityManager::currentMode_ = SecurityMode::TRUSTED;
std::unordered_set<std::string> SecurityManager::whitelist_;
std::unordered_set<std::string> SecurityManager::blacklist_;
std::unordered_set<std::string> SecurityManager::readOnlyPaths_;

void SecurityManager::setMode(SecurityMode mode)
{
    currentMode_ = mode;
}

SecurityMode SecurityManager::getMode()
{
    return currentMode_;
}

void SecurityManager::allowPath(const std::string& path)
{
    std::string normalized = normalizePath(path);
    whitelist_.insert(normalized);
}

void SecurityManager::blockPath(const std::string& path)
{
    std::string normalized = normalizePath(path);
    blacklist_.insert(normalized);
}

void SecurityManager::setReadOnly(const std::string& path)
{
    std::string normalized = normalizePath(path);
    readOnlyPaths_.insert(normalized);
}

bool SecurityManager::checkFileAccess(const std::string& path, FileOperation operation)
{
    // 빈 경로는 거부
    if (path.empty())
    {
        return false;
    }

    // 경로 정규화
    std::string normalizedPath = normalizePath(path);

    // 블랙리스트 확인 (최우선)
    if (isBlacklisted(normalizedPath))
    {
        return false;
    }

    // 읽기 전용 경로에서 쓰기/삭제 작업 차단
    if (isReadOnly(normalizedPath))
    {
        if (operation == FileOperation::WRITE || operation == FileOperation::DELETE)
        {
            return false;
        }
    }

    // 모드별 처리
    switch (currentMode_)
    {
    case SecurityMode::TRUSTED:
        // TRUSTED 모드: 블랙리스트만 확인 (위에서 이미 확인)
        return true;

    case SecurityMode::SANDBOX:
        // SANDBOX 모드: 화이트리스트에 있는 경로만 허용
        return isWhitelisted(normalizedPath);

    case SecurityMode::UNTRUSTED:
        // UNTRUSTED 모드: 민감한 경로 차단, /tmp는 허용
        if (isSensitivePath(normalizedPath))
        {
            return false;
        }
        return true;

    default:
        return false;
    }
}

void SecurityManager::reset()
{
    currentMode_ = SecurityMode::TRUSTED;
    whitelist_.clear();
    blacklist_.clear();
    readOnlyPaths_.clear();
}

std::string SecurityManager::normalizePath(const std::string& path)
{
    try
    {
        // 빈 경로 처리
        if (path.empty())
        {
            return "";
        }

        // null 문자 포함 경로는 빈 문자열 반환 (안전 처리)
        if (path.find('\0') != std::string::npos)
        {
            return "";
        }

        fs::path p(path);

        // 상대 경로를 절대 경로로 변환
        if (p.is_relative())
        {
            p = fs::current_path() / p;
        }

        // 경로 정규화 (. 및 .. 처리)
        p = fs::weakly_canonical(p);

        // 문자열로 변환
        std::string normalized = p.string();

        // 디렉토리 경로는 /로 끝나도록
        if (path.back() == '/' && !normalized.empty() && normalized.back() != '/')
        {
            normalized += '/';
        }

        return normalized;
    }
    catch (...)
    {
        // 정규화 실패 시 빈 문자열 반환
        return "";
    }
}

bool SecurityManager::isBlacklisted(const std::string& normalizedPath)
{
    for (const auto& blockedPath : blacklist_)
    {
        if (pathStartsWith(normalizedPath, blockedPath))
        {
            return true;
        }
    }
    return false;
}

bool SecurityManager::isWhitelisted(const std::string& normalizedPath)
{
    for (const auto& allowedPath : whitelist_)
    {
        if (pathStartsWith(normalizedPath, allowedPath))
        {
            return true;
        }
    }
    return false;
}

bool SecurityManager::isReadOnly(const std::string& normalizedPath)
{
    for (const auto& readOnlyPath : readOnlyPaths_)
    {
        if (pathStartsWith(normalizedPath, readOnlyPath))
        {
            return true;
        }
    }
    return false;
}

bool SecurityManager::isSensitivePath(const std::string& normalizedPath)
{
    // UNTRUSTED 모드에서 차단할 민감한 경로들
    static const std::vector<std::string> sensitivePaths = {
        "/etc/",
        "/root/",
        "/sys/",
        "/proc/",
        "/dev/",
        "/boot/",
        "/var/log/",
        "/usr/bin/",
        "/usr/sbin/",
        "/sbin/",
        "/bin/",
        // macOS symlink 경로들
        "/private/etc/",
        "/private/var/log/"
    };

    for (const auto& sensitivePath : sensitivePaths)
    {
        if (pathStartsWith(normalizedPath, sensitivePath))
        {
            return true;
        }
    }

    return false;
}

bool SecurityManager::pathStartsWith(const std::string& path, const std::string& prefix)
{
    // 길이 확인
    if (path.length() < prefix.length())
    {
        return false;
    }

    // 접두사 일치 확인
    if (path.compare(0, prefix.length(), prefix) != 0)
    {
        return false;
    }

    // 정확한 디렉토리 매칭 확인
    // prefix가 /로 끝나면 무조건 true
    if (!prefix.empty() && prefix.back() == '/')
    {
        return true;
    }

    // prefix가 /로 끝나지 않으면 path도 정확히 같거나
    // path[prefix.length()]가 /여야 함
    if (path.length() == prefix.length())
    {
        return true;
    }

    return path[prefix.length()] == '/';
}

} // namespace security
} // namespace kingsejong
