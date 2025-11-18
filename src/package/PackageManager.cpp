/**
 * @file PackageManager.cpp
 * @brief KingSejong 패키지 관리자 구현
 * @author KingSejong Team
 * @date 2025-11-18
 */

#include "package/PackageManager.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <stdexcept>

namespace fs = std::filesystem;

namespace kingsejong {
namespace package {

PackageManager::PackageManager(const std::string& projectRoot)
    : projectRoot_(projectRoot)
{
    ensureModulesDirectory();
}

Package PackageManager::loadProjectPackage()
{
    std::string packageFile = getPackageFilePath();

    if (!fs::exists(packageFile)) {
        throw std::runtime_error("package.ksj 파일을 찾을 수 없습니다: " + packageFile);
    }

    return Package::fromFile(packageFile);
}

void PackageManager::saveProjectPackage(const Package& pkg)
{
    std::string packageFile = getPackageFilePath();

    std::ofstream file(packageFile);
    if (!file.is_open()) {
        throw std::runtime_error("package.ksj 파일을 쓸 수 없습니다: " + packageFile);
    }

    file << pkg.toJSON();
}

Package PackageManager::initPackage(const std::string& name,
                                    const std::string& version,
                                    const std::string& description)
{
    std::string packageFile = getPackageFilePath();

    if (fs::exists(packageFile)) {
        throw std::runtime_error("package.ksj 파일이 이미 존재합니다");
    }

    Package pkg(name, version);
    if (!description.empty()) {
        pkg.setDescription(description);
    }

    saveProjectPackage(pkg);

    std::cout << "✅ 패키지 초기화 완료: " << name << " v" << version << std::endl;

    return pkg;
}

bool PackageManager::installPackage(const std::string& packageName, const std::string& version)
{
    try {
        std::cout << "📦 패키지 설치 중: " << packageName;
        if (!version.empty()) {
            std::cout << "@" << version;
        }
        std::cout << std::endl;

        // Phase 7.5 기본 구조: 실제 다운로드는 미구현
        // 현재는 로컬 패키지만 지원
        Package pkg = downloadPackage(packageName, version);

        // 레지스트리에 등록
        registry_.registerPackage(pkg);

        // package.ksj 업데이트
        try {
            Package projectPkg = loadProjectPackage();
            projectPkg.addDependency(Dependency(packageName, version.empty() ? "^1.0.0" : version));
            saveProjectPackage(projectPkg);
        } catch (...) {
            // package.ksj가 없으면 무시
        }

        std::cout << "✅ 설치 완료: " << packageName << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "❌ 설치 실패: " << e.what() << std::endl;
        return false;
    }
}

bool PackageManager::installDevPackage(const std::string& packageName, const std::string& version)
{
    try {
        std::cout << "📦 개발 의존성 설치 중: " << packageName;
        if (!version.empty()) {
            std::cout << "@" << version;
        }
        std::cout << std::endl;

        Package pkg = downloadPackage(packageName, version);
        registry_.registerPackage(pkg);

        // package.ksj 업데이트
        try {
            Package projectPkg = loadProjectPackage();
            projectPkg.addDevDependency(Dependency(packageName, version.empty() ? "^1.0.0" : version));
            saveProjectPackage(projectPkg);
        } catch (...) {
            // package.ksj가 없으면 무시
        }

        std::cout << "✅ 설치 완료: " << packageName << " (dev)" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "❌ 설치 실패: " << e.what() << std::endl;
        return false;
    }
}

bool PackageManager::uninstallPackage(const std::string& packageName)
{
    std::cout << "🗑️  패키지 제거 중: " << packageName << std::endl;

    // 레지스트리에서 제거
    auto it = registry_.packages.find(packageName);
    if (it == registry_.packages.end()) {
        std::cerr << "❌ 패키지가 설치되어 있지 않습니다: " << packageName << std::endl;
        return false;
    }

    registry_.packages.erase(it);

    // node_modules에서 제거
    fs::path modulePath = fs::path(getModulesPath()) / packageName;
    if (fs::exists(modulePath)) {
        fs::remove_all(modulePath);
    }

    std::cout << "✅ 제거 완료: " << packageName << std::endl;
    return true;
}

bool PackageManager::installDependencies()
{
    try {
        Package projectPkg = loadProjectPackage();

        std::cout << "📦 의존성 설치 중..." << std::endl;
        std::cout << "프로젝트: " << projectPkg.name() << " v" << projectPkg.version() << std::endl;

        int installedCount = 0;

        // 일반 의존성 설치
        for (const auto& dep : projectPkg.dependencies()) {
            std::cout << "  - " << dep.name << "@" << dep.version << std::endl;
            if (installPackage(dep.name, dep.version)) {
                installedCount++;
            }
        }

        // 개발 의존성 설치
        for (const auto& dep : projectPkg.devDependencies()) {
            std::cout << "  - " << dep.name << "@" << dep.version << " (dev)" << std::endl;
            if (installDevPackage(dep.name, dep.version)) {
                installedCount++;
            }
        }

        std::cout << "✅ " << installedCount << "개 패키지 설치 완료" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "❌ 의존성 설치 실패: " << e.what() << std::endl;
        return false;
    }
}

std::string PackageManager::getModulesPath() const
{
    return (fs::path(projectRoot_) / "node_modules").string();
}

std::string PackageManager::getPackageFilePath() const
{
    return (fs::path(projectRoot_) / "package.ksj").string();
}

void PackageManager::printPackageInfo(const std::string& packageName)
{
    try {
        Package pkg;

        if (packageName.empty()) {
            // 현재 프로젝트 정보
            pkg = loadProjectPackage();
        } else {
            // 특정 패키지 정보
            const Package* pkgPtr = registry_.getPackage(packageName);
            if (!pkgPtr) {
                std::cerr << "❌ 패키지를 찾을 수 없습니다: " << packageName << std::endl;
                return;
            }
            pkg = *pkgPtr;
        }

        std::cout << "\n" << pkg.name() << "@" << pkg.version() << std::endl;

        if (!pkg.description().empty()) {
            std::cout << pkg.description() << std::endl;
        }

        std::cout << "\n📄 라이선스: " << pkg.license() << std::endl;
        std::cout << "📍 진입점: " << pkg.main() << std::endl;

        // 작성자
        if (!pkg.authors().empty()) {
            std::cout << "\n👤 작성자:" << std::endl;
            for (const auto& author : pkg.authors()) {
                std::cout << "  " << author.name;
                if (!author.email.empty()) {
                    std::cout << " <" << author.email << ">";
                }
                std::cout << std::endl;
            }
        }

        // 의존성
        if (!pkg.dependencies().empty()) {
            std::cout << "\n📦 의존성:" << std::endl;
            for (const auto& dep : pkg.dependencies()) {
                std::cout << "  " << dep.name << ": " << dep.version << std::endl;
            }
        }

        // 개발 의존성
        if (!pkg.devDependencies().empty()) {
            std::cout << "\n🛠️  개발 의존성:" << std::endl;
            for (const auto& dep : pkg.devDependencies()) {
                std::cout << "  " << dep.name << ": " << dep.version << std::endl;
            }
        }

        std::cout << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ 오류: " << e.what() << std::endl;
    }
}

void PackageManager::listInstalledPackages()
{
    if (registry_.packages.empty()) {
        std::cout << "설치된 패키지가 없습니다." << std::endl;
        return;
    }

    std::cout << "\n📦 설치된 패키지 (" << registry_.packages.size() << "개):\n" << std::endl;

    for (const auto& [name, pkg] : registry_.packages) {
        std::cout << "  " << name << "@" << pkg.version();
        if (!pkg.description().empty()) {
            std::cout << " - " << pkg.description();
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
}

// Private methods

void PackageManager::ensureModulesDirectory()
{
    std::string modulesPath = getModulesPath();
    if (!fs::exists(modulesPath)) {
        fs::create_directories(modulesPath);
    }
}

Package PackageManager::downloadPackage(const std::string& packageName, const std::string& version)
{
    // Phase 7.5 기본 구조: 실제 다운로드는 미구현
    // 현재는 로컬 stdlib 패키지만 지원

    if (packageName == "stdlib") {
        // stdlib는 내장 패키지
        Package pkg("stdlib", version.empty() ? "1.0.0" : version);
        pkg.setDescription("KingSejong 표준 라이브러리");
        pkg.setLicense("MIT");
        return pkg;
    }

    // 로컬 node_modules에서 찾기
    fs::path localPath = fs::path(getModulesPath()) / packageName / "package.ksj";
    if (fs::exists(localPath)) {
        return Package::fromFile(localPath.string());
    }

    // TODO: 중앙 저장소에서 다운로드 (Phase 7.5 후속 작업)
    throw std::runtime_error("패키지를 찾을 수 없습니다: " + packageName +
                           " (중앙 저장소 미구현)");
}

std::vector<Dependency> PackageManager::resolveDependencies(const Package& pkg)
{
    // Phase 7.5 기본 구조: 간단한 의존성 해결
    // TODO: 재귀적 의존성 해결, 버전 충돌 해결

    std::vector<Dependency> resolved;

    for (const auto& dep : pkg.dependencies()) {
        resolved.push_back(dep);
    }

    return resolved;
}

bool PackageManager::isVersionCompatible(const std::string& required, const std::string& installed)
{
    // Phase 7.5 기본 구조: 간단한 버전 비교
    // TODO: Semantic Versioning 완전 구현 (^, ~, >=, <, etc.)

    if (required.empty() || required == "*") {
        return true;
    }

    if (required[0] == '^') {
        // ^1.2.3: 1.x.x 범위
        std::string base = required.substr(1);
        return installed >= base;  // 간단한 비교 (실제로는 major version만 비교)
    }

    if (required[0] == '~') {
        // ~1.2.3: 1.2.x 범위
        std::string base = required.substr(1);
        return installed >= base;  // 간단한 비교 (실제로는 minor version까지 비교)
    }

    // 정확한 버전
    return required == installed;
}

} // namespace package
} // namespace kingsejong
