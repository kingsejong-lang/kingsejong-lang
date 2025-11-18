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
#include <functional>
#include <unordered_set>

namespace fs = std::filesystem;

namespace kingsejong {
namespace package {

// SemanticVersion 연산자 구현
bool PackageManager::SemanticVersion::operator<(const SemanticVersion& other) const {
    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    return patch < other.patch;
}

bool PackageManager::SemanticVersion::operator<=(const SemanticVersion& other) const {
    return *this < other || *this == other;
}

bool PackageManager::SemanticVersion::operator>(const SemanticVersion& other) const {
    return !(*this <= other);
}

bool PackageManager::SemanticVersion::operator>=(const SemanticVersion& other) const {
    return !(*this < other);
}

bool PackageManager::SemanticVersion::operator==(const SemanticVersion& other) const {
    return major == other.major && minor == other.minor && patch == other.patch;
}

bool PackageManager::SemanticVersion::operator!=(const SemanticVersion& other) const {
    return !(*this == other);
}

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

PackageManager::SemanticVersion PackageManager::parseVersion(const std::string& versionStr)
{
    SemanticVersion ver;

    // 버전 문자열 정리 (공백, v 접두사 제거)
    std::string cleaned = versionStr;
    if (!cleaned.empty() && cleaned[0] == 'v') {
        cleaned = cleaned.substr(1);
    }

    // 점으로 구분하여 파싱
    size_t pos1 = cleaned.find('.');
    if (pos1 == std::string::npos) {
        // major만 있는 경우 (예: "1")
        ver.major = std::stoi(cleaned);
        return ver;
    }

    ver.major = std::stoi(cleaned.substr(0, pos1));

    size_t pos2 = cleaned.find('.', pos1 + 1);
    if (pos2 == std::string::npos) {
        // major.minor만 있는 경우 (예: "1.2")
        ver.minor = std::stoi(cleaned.substr(pos1 + 1));
        return ver;
    }

    ver.minor = std::stoi(cleaned.substr(pos1 + 1, pos2 - pos1 - 1));

    // patch 부분 (빌드 메타데이터나 pre-release 태그 무시)
    std::string patchStr = cleaned.substr(pos2 + 1);
    size_t dashPos = patchStr.find('-');
    size_t plusPos = patchStr.find('+');
    size_t endPos = std::min(dashPos, plusPos);

    if (endPos != std::string::npos) {
        patchStr = patchStr.substr(0, endPos);
    }

    if (!patchStr.empty()) {
        ver.patch = std::stoi(patchStr);
    }

    return ver;
}

bool PackageManager::isCaretCompatible(const SemanticVersion& base, const SemanticVersion& installed)
{
    // ^1.2.3: >=1.2.3 <2.0.0
    // ^0.2.3: >=0.2.3 <0.3.0 (major가 0이면 minor 기준)
    // ^0.0.3: >=0.0.3 <0.0.4 (major, minor가 0이면 patch 기준)

    if (installed < base) {
        return false;
    }

    if (base.major > 0) {
        // major 버전이 다르면 호환되지 않음
        return installed.major == base.major;
    }

    if (base.minor > 0) {
        // major가 0이고 minor가 다르면 호환되지 않음
        return installed.major == base.major && installed.minor == base.minor;
    }

    // major와 minor가 모두 0이면 patch까지 같아야 함
    return installed == base;
}

bool PackageManager::isTildeCompatible(const SemanticVersion& base, const SemanticVersion& installed)
{
    // ~1.2.3: >=1.2.3 <1.3.0

    if (installed < base) {
        return false;
    }

    // major와 minor가 같아야 함
    return installed.major == base.major && installed.minor == base.minor;
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
    // Phase 7.5: 재귀적 의존성 해결 with 버전 충돌 감지

    std::vector<Dependency> resolved;
    std::unordered_map<std::string, std::string> resolvedVersions; // 패키지명 -> 해결된 버전
    std::unordered_set<std::string> visited; // 순환 의존성 감지

    // 재귀 헬퍼 함수
    std::function<void(const Package&, int)> resolveRecursive;
    resolveRecursive = [&](const Package& currentPkg, int depth) {
        // 순환 의존성 감지 (너무 깊은 재귀 방지)
        if (depth > 100) {
            std::cerr << "⚠️  경고: 순환 의존성 또는 너무 깊은 의존성 트리 감지" << std::endl;
            return;
        }

        for (const auto& dep : currentPkg.dependencies()) {
            // 이미 방문한 패키지인지 확인
            std::string visitKey = dep.name + "@" + dep.version;
            if (visited.count(visitKey) > 0) {
                continue; // 이미 처리됨
            }
            visited.insert(visitKey);

            // 버전 충돌 확인
            if (resolvedVersions.count(dep.name) > 0) {
                std::string existingVersion = resolvedVersions[dep.name];

                // 버전 호환성 확인
                if (!isVersionCompatible(dep.version, existingVersion)) {
                    // 충돌 발생 - 더 높은 버전을 선택
                    SemanticVersion depVer = parseVersion(dep.version);
                    SemanticVersion existingVer = parseVersion(existingVersion);

                    std::string selectedVersion;
                    if (existingVer > depVer) {
                        selectedVersion = existingVersion;
                        std::cerr << "⚠️  버전 충돌 해결: " << dep.name
                                  << " (" << dep.version << " vs " << existingVersion
                                  << ") -> " << selectedVersion << " 선택" << std::endl;
                    } else {
                        selectedVersion = dep.version;
                        resolvedVersions[dep.name] = selectedVersion;
                        std::cerr << "⚠️  버전 충돌 해결: " << dep.name
                                  << " (" << dep.version << " vs " << existingVersion
                                  << ") -> " << selectedVersion << " 선택" << std::endl;
                    }
                }
                continue; // 이미 해결됨
            }

            // 의존성 추가
            resolvedVersions[dep.name] = dep.version;
            resolved.push_back(dep);

            // 전이 의존성 해결 (재귀)
            try {
                // 레지스트리에서 패키지 찾기
                const Package* depPkg = registry_.getPackage(dep.name);
                if (depPkg != nullptr) {
                    resolveRecursive(*depPkg, depth + 1);
                } else {
                    // 패키지가 아직 설치되지 않았으면 다운로드 시도
                    try {
                        Package downloadedPkg = downloadPackage(dep.name, dep.version);
                        registry_.registerPackage(downloadedPkg);
                        resolveRecursive(downloadedPkg, depth + 1);
                    } catch (...) {
                        // 다운로드 실패 - 무시 (나중에 설치 시 처리)
                    }
                }
            } catch (...) {
                // 의존성 해결 실패 - 계속 진행
            }
        }
    };

    // 루트 패키지부터 시작
    resolveRecursive(pkg, 0);

    return resolved;
}

bool PackageManager::isVersionCompatible(const std::string& required, const std::string& installed)
{
    // Phase 7.5: Semantic Versioning 완전 구현

    // 와일드카드 또는 빈 문자열: 모든 버전 허용
    if (required.empty() || required == "*" || required == "x" || required == "X") {
        return true;
    }

    try {
        // OR 연산자 처리 (||)
        size_t orPos = required.find("||");
        if (orPos != std::string::npos) {
            std::string left = required.substr(0, orPos);
            std::string right = required.substr(orPos + 2);
            // 양쪽 공백 제거
            left.erase(0, left.find_first_not_of(" \t"));
            left.erase(left.find_last_not_of(" \t") + 1);
            right.erase(0, right.find_first_not_of(" \t"));
            right.erase(right.find_last_not_of(" \t") + 1);

            return isVersionCompatible(left, installed) || isVersionCompatible(right, installed);
        }

        // 범위 연산자 처리 (-)
        size_t rangePos = required.find(" - ");
        if (rangePos != std::string::npos) {
            std::string minVer = required.substr(0, rangePos);
            std::string maxVer = required.substr(rangePos + 3);
            SemanticVersion min = parseVersion(minVer);
            SemanticVersion max = parseVersion(maxVer);
            SemanticVersion inst = parseVersion(installed);
            return inst >= min && inst <= max;
        }

        std::string req = required;
        SemanticVersion installedVer = parseVersion(installed);

        // ^ 연산자: 호환 가능한 변경
        if (req[0] == '^') {
            SemanticVersion base = parseVersion(req.substr(1));
            return isCaretCompatible(base, installedVer);
        }

        // ~ 연산자: 대략적으로 동등
        if (req[0] == '~') {
            SemanticVersion base = parseVersion(req.substr(1));
            return isTildeCompatible(base, installedVer);
        }

        // >= 연산자
        if (req.size() >= 2 && req.substr(0, 2) == ">=") {
            SemanticVersion base = parseVersion(req.substr(2));
            return installedVer >= base;
        }

        // > 연산자
        if (req[0] == '>') {
            size_t startPos = (req.size() >= 2 && req[1] == '=') ? 2 : 1;
            SemanticVersion base = parseVersion(req.substr(startPos));
            if (startPos == 2) {
                return installedVer >= base;
            }
            return installedVer > base;
        }

        // <= 연산자
        if (req.size() >= 2 && req.substr(0, 2) == "<=") {
            SemanticVersion base = parseVersion(req.substr(2));
            return installedVer <= base;
        }

        // < 연산자
        if (req[0] == '<') {
            size_t startPos = (req.size() >= 2 && req[1] == '=') ? 2 : 1;
            SemanticVersion base = parseVersion(req.substr(startPos));
            if (startPos == 2) {
                return installedVer <= base;
            }
            return installedVer < base;
        }

        // = 연산자 또는 정확한 버전
        if (req[0] == '=') {
            req = req.substr(1);
        }

        // 와일드카드 처리 (1.x, 1.2.x)
        if (req.find('x') != std::string::npos || req.find('X') != std::string::npos) {
            // 1.x.x 또는 1.2.x 형태
            std::replace(req.begin(), req.end(), 'x', '0');
            std::replace(req.begin(), req.end(), 'X', '0');

            SemanticVersion base = parseVersion(req);

            // x가 있었던 위치에 따라 비교
            if (required.find('.') == std::string::npos) {
                // 1.x 형태 - major만 비교
                return installedVer.major == base.major;
            }

            size_t secondDot = required.find('.', required.find('.') + 1);
            if (secondDot != std::string::npos) {
                // 1.2.x 형태 - major.minor 비교
                return installedVer.major == base.major && installedVer.minor == base.minor;
            }

            // 1.x 형태 - major만 비교
            return installedVer.major == base.major;
        }

        // 정확한 버전 일치
        SemanticVersion requiredVer = parseVersion(req);
        return installedVer == requiredVer;

    } catch (const std::exception&) {
        // 파싱 실패 시 문자열 비교로 폴백
        return required == installed;
    }
}

} // namespace package
} // namespace kingsejong
