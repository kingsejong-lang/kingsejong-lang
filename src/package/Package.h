#pragma once

/**
 * @file Package.h
 * @brief KingSejong 패키지 메타데이터
 * @author KingSejong Team
 * @date 2025-11-18
 */

#include <string>
#include <vector>
#include <unordered_map>

namespace kingsejong {
namespace package {

/**
 * @struct Dependency
 * @brief 패키지 의존성 정보
 */
struct Dependency
{
    std::string name;     ///< 패키지 이름
    std::string version;  ///< 버전 (예: "1.0.0", "^1.2.0", "~1.0.5")

    Dependency() = default;
    Dependency(const std::string& n, const std::string& v)
        : name(n), version(v) {}
};

/**
 * @struct Author
 * @brief 패키지 작성자 정보
 */
struct Author
{
    std::string name;   ///< 작성자 이름
    std::string email;  ///< 이메일

    Author() = default;
    Author(const std::string& n, const std::string& e = "")
        : name(n), email(e) {}
};

/**
 * @class Package
 * @brief KingSejong 패키지 메타데이터
 *
 * package.ksj 파일에서 로드되는 패키지 정보를 나타냅니다.
 *
 * 예시 package.ksj:
 * {
 *   "name": "my-package",
 *   "version": "1.0.0",
 *   "description": "My awesome package",
 *   "author": "홍길동",
 *   "license": "MIT",
 *   "main": "src/index.ksj",
 *   "dependencies": {
 *     "stdlib": "^1.0.0",
 *     "http-client": "~2.1.0"
 *   },
 *   "dev_dependencies": {
 *     "test-framework": "^1.0.0"
 *   }
 * }
 */
class Package
{
public:
    /**
     * @brief 기본 생성자
     */
    Package() = default;

    /**
     * @brief 패키지 생성자
     * @param name 패키지 이름
     * @param version 패키지 버전
     */
    Package(const std::string& name, const std::string& version);

    // Getters
    const std::string& name() const { return name_; }
    const std::string& version() const { return version_; }
    const std::string& description() const { return description_; }
    const std::string& license() const { return license_; }
    const std::string& main() const { return main_; }
    const std::vector<Author>& authors() const { return authors_; }
    const std::vector<Dependency>& dependencies() const { return dependencies_; }
    const std::vector<Dependency>& devDependencies() const { return devDependencies_; }
    const std::unordered_map<std::string, std::string>& scripts() const { return scripts_; }

    // Setters
    void setName(const std::string& name) { name_ = name; }
    void setVersion(const std::string& version) { version_ = version; }
    void setDescription(const std::string& desc) { description_ = desc; }
    void setLicense(const std::string& license) { license_ = license; }
    void setMain(const std::string& main) { main_ = main; }

    /**
     * @brief 작성자 추가
     * @param author 작성자 정보
     */
    void addAuthor(const Author& author);

    /**
     * @brief 의존성 추가
     * @param dep 의존성 정보
     */
    void addDependency(const Dependency& dep);

    /**
     * @brief 개발 의존성 추가
     * @param dep 의존성 정보
     */
    void addDevDependency(const Dependency& dep);

    /**
     * @brief 스크립트 추가
     * @param name 스크립트 이름
     * @param command 실행 명령
     */
    void addScript(const std::string& name, const std::string& command);

    /**
     * @brief 패키지가 유효한지 확인
     * @return 이름과 버전이 있으면 true
     */
    bool isValid() const;

    /**
     * @brief 패키지 정보를 JSON 형식으로 변환
     * @return JSON 문자열
     */
    std::string toJSON() const;

    /**
     * @brief JSON에서 패키지 로드
     * @param json JSON 문자열
     * @return 파싱된 Package 객체
     * @throws std::runtime_error JSON 파싱 실패 시
     */
    static Package fromJSON(const std::string& json);

    /**
     * @brief package.ksj 파일에서 패키지 로드
     * @param filePath 파일 경로
     * @return 로드된 Package 객체
     * @throws std::runtime_error 파일 읽기/파싱 실패 시
     */
    static Package fromFile(const std::string& filePath);

private:
    std::string name_;         ///< 패키지 이름
    std::string version_;      ///< 패키지 버전 (Semantic Versioning)
    std::string description_;  ///< 패키지 설명
    std::string license_;      ///< 라이선스
    std::string main_;         ///< 메인 진입점 파일

    std::vector<Author> authors_;                 ///< 작성자 목록
    std::vector<Dependency> dependencies_;        ///< 의존성 목록
    std::vector<Dependency> devDependencies_;     ///< 개발 의존성 목록
    std::unordered_map<std::string, std::string> scripts_;  ///< 스크립트 목록
};

} // namespace package
} // namespace kingsejong
