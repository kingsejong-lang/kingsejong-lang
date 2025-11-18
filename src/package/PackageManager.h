#pragma once

/**
 * @file PackageManager.h
 * @brief KingSejong 패키지 관리자
 * @author KingSejong Team
 * @date 2025-11-18
 */

#include "package/Package.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace kingsejong {
namespace package {

/**
 * @struct PackageRegistry
 * @brief 설치된 패키지 레지스트리
 */
struct PackageRegistry
{
    std::unordered_map<std::string, Package> packages;  ///< 패키지 이름 -> Package

    /**
     * @brief 패키지가 설치되어 있는지 확인
     * @param name 패키지 이름
     * @return 설치 여부
     */
    bool hasPackage(const std::string& name) const {
        return packages.find(name) != packages.end();
    }

    /**
     * @brief 패키지 가져오기
     * @param name 패키지 이름
     * @return Package 포인터 (없으면 nullptr)
     */
    const Package* getPackage(const std::string& name) const {
        auto it = packages.find(name);
        return (it != packages.end()) ? &it->second : nullptr;
    }

    /**
     * @brief 패키지 등록
     * @param pkg 패키지
     */
    void registerPackage(const Package& pkg) {
        packages[pkg.name()] = pkg;
    }
};

/**
 * @class PackageManager
 * @brief KingSejong 패키지 관리자
 *
 * 패키지 설치, 제거, 의존성 해결 등을 담당합니다.
 *
 * 주요 기능:
 * - package.ksj 파일 읽기/쓰기
 * - 패키지 설치 (install)
 * - 패키지 제거 (uninstall)
 * - 의존성 해결 (dependency resolution)
 * - 패키지 정보 조회 (info)
 */
class PackageManager
{
public:
    /**
     * @brief PackageManager 생성자
     * @param projectRoot 프로젝트 루트 디렉토리
     */
    explicit PackageManager(const std::string& projectRoot = ".");

    /**
     * @brief 프로젝트의 package.ksj 로드
     * @return 로드된 Package
     * @throws std::runtime_error 파일이 없거나 파싱 실패 시
     */
    Package loadProjectPackage();

    /**
     * @brief 프로젝트의 package.ksj 저장
     * @param pkg 저장할 Package
     * @throws std::runtime_error 파일 쓰기 실패 시
     */
    void saveProjectPackage(const Package& pkg);

    /**
     * @brief 패키지 초기화 (package.ksj 생성)
     * @param name 패키지 이름
     * @param version 패키지 버전
     * @param description 패키지 설명
     * @return 생성된 Package
     */
    Package initPackage(const std::string& name,
                       const std::string& version = "1.0.0",
                       const std::string& description = "");

    /**
     * @brief 패키지 설치
     * @param packageName 패키지 이름
     * @param version 버전 (빈 문자열이면 최신 버전)
     * @return 설치 성공 여부
     *
     * 예: installPackage("stdlib", "^1.0.0")
     */
    bool installPackage(const std::string& packageName, const std::string& version = "");

    /**
     * @brief 개발 의존성 패키지 설치
     * @param packageName 패키지 이름
     * @param version 버전 (빈 문자열이면 최신 버전)
     * @return 설치 성공 여부
     */
    bool installDevPackage(const std::string& packageName, const std::string& version = "");

    /**
     * @brief 패키지 제거
     * @param packageName 패키지 이름
     * @return 제거 성공 여부
     */
    bool uninstallPackage(const std::string& packageName);

    /**
     * @brief 모든 의존성 설치
     * @return 설치 성공 여부
     *
     * package.ksj에 정의된 모든 의존성을 설치합니다.
     */
    bool installDependencies();

    /**
     * @brief 설치된 패키지 레지스트리 가져오기
     * @return PackageRegistry 참조
     */
    const PackageRegistry& getRegistry() const { return registry_; }

    /**
     * @brief node_modules 경로 가져오기
     * @return node_modules 디렉토리 경로
     */
    std::string getModulesPath() const;

    /**
     * @brief package.ksj 파일 경로 가져오기
     * @return package.ksj 파일 경로
     */
    std::string getPackageFilePath() const;

    /**
     * @brief 패키지 정보 출력
     * @param packageName 패키지 이름 (빈 문자열이면 현재 프로젝트)
     */
    void printPackageInfo(const std::string& packageName = "");

    /**
     * @brief 설치된 모든 패키지 목록 출력
     */
    void listInstalledPackages();

private:
    std::string projectRoot_;    ///< 프로젝트 루트 디렉토리
    PackageRegistry registry_;   ///< 설치된 패키지 레지스트리

    /**
     * @brief node_modules 디렉토리 생성
     */
    void ensureModulesDirectory();

    /**
     * @brief 패키지 다운로드 (미구현 - Phase 7.5에서 중앙 저장소와 연동 예정)
     * @param packageName 패키지 이름
     * @param version 버전
     * @return Package 객체
     */
    Package downloadPackage(const std::string& packageName, const std::string& version);

    /**
     * @brief 의존성 해결 (재귀적)
     * @param pkg 패키지
     * @return 해결된 의존성 목록
     */
    std::vector<Dependency> resolveDependencies(const Package& pkg);

    /**
     * @brief 버전 호환성 확인
     * @param required 요구 버전 (예: "^1.0.0")
     * @param installed 설치된 버전 (예: "1.2.3")
     * @return 호환 여부
     */
    bool isVersionCompatible(const std::string& required, const std::string& installed);
};

} // namespace package
} // namespace kingsejong
