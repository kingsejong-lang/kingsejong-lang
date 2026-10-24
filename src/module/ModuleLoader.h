#pragma once

/**
 * @file ModuleLoader.h
 * @brief KingSejong 언어 모듈 로더
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include "runtime/Environment.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace kingsejong {

// Forward declaration
namespace ast {
    class Program;
}

namespace module {

/**
 * @brief 모듈 캐시 엔트리
 *
 * Environment와 AST를 함께 저장하여 AST 수명을 보장합니다.
 * Function 객체가 AST 노드(body_)에 대한 raw pointer를 가지므로,
 * AST가 살아있어야 합니다.
 */
struct ModuleCacheEntry
{
    std::shared_ptr<evaluator::Environment> env;  ///< 모듈 환경
    std::unique_ptr<ast::Program> ast;             ///< 모듈 AST (수명 관리)
};

/**
 * @class ModuleLoader
 * @brief 모듈 로딩 및 캐싱을 담당하는 클래스
 *
 * 모듈 파일(.ksj)을 로딩하고, 캐싱하며, 순환 참조를 방지합니다.
 */
class ModuleLoader
{
private:
    /// 모듈 캐시: 경로 → ModuleCacheEntry
    std::unordered_map<std::string, ModuleCacheEntry> cache_;

    /// 현재 로딩 중인 모듈 (순환 참조 방지)
    std::unordered_set<std::string> loading_;

    /// 기본 검색 경로 (보통 프로세스 작업 디렉터리)
    std::string basePath_;

    /// 추가 검색 경로. 호출 파일 디렉터리 → 여기 → basePath_ → KINGSEJONG_PATH → 설치 stdlib 순으로 찾는다.
    std::vector<std::string> searchPaths_;

public:
    /**
     * @brief 생성자
     * @param basePath 모듈 검색 기본 경로 (기본값: 현재 디렉토리)
     */
    explicit ModuleLoader(const std::string& basePath = ".");

    /**
     * @brief 모듈 검색 경로를 추가한다 (basePath 보다 먼저, 호출 파일 디렉터리보다 뒤에 본다)
     */
    void addSearchPath(const std::string& dir);

    /**
     * @brief 모듈 경로가 해석될 수 있는 후보 파일 경로를 순서대로 돌려준다
     * @param modulePath 가져오기 문의 문자열
     * @param fromFile 가져오기 문이 있는 파일 (상대 경로의 기준). 비어 있으면 basePath 만 쓴다
     */
    std::vector<std::string> candidatePaths(const std::string& modulePath, const std::string& fromFile = "") const;

    /// 모듈 프로그램을 실행해 모듈 환경을 채우는 함수. 비어 있으면 평가기로 실행한다.
    using Executor = std::function<void(ast::Program& program, const std::shared_ptr<evaluator::Environment>& moduleEnv)>;

    /**
     * @brief 모듈 로딩
     * @param modulePath 모듈 경로 (예: "math", "utils/helper")
     * @param fromFile 가져오기 문이 있는 파일 (상대 경로의 기준)
     * @param executor 모듈 코드를 돌릴 실행기. 비어 있으면 평가기 (VM 은 자기 실행기를 넘긴다)
     * @return 모듈의 Environment
     * @throws RuntimeError 모듈을 찾을 수 없거나 순환 참조 발생 시
     */
    std::shared_ptr<evaluator::Environment> loadModule(const std::string& modulePath, const std::string& fromFile = "",
                                                       const Executor& executor = nullptr);

    /**
     * @brief 경로 해석
     * @param modulePath 모듈 경로
     * @return 절대 경로
     */
    std::string resolvePath(const std::string& modulePath, const std::string& fromFile = "") const;

    /**
     * @brief 캐시 초기화
     */
    void clearCache();

    /**
     * @brief 소멸자. 캐시된 모듈 환경의 순환 참조를 끊고 비운다
     */
    ~ModuleLoader();

    /**
     * @brief 모듈이 캐시되어 있는지 확인
     * @param modulePath 모듈 경로
     * @return 캐시 여부
     */
    bool isCached(const std::string& modulePath) const;

private:
    /**
     * @brief 파일 존재 여부 확인
     * @param path 파일 경로
     * @return 파일 존재 여부
     */
    bool fileExists(const std::string& path) const;

    /// 캐시·순환 검사용 키: 정규화한 절대 경로 ("./x.ksj" 와 "x.ksj" 가 같아진다)
    std::string cacheKey(const std::string& resolvedPath) const;

    /**
     * @brief 파일 내용 읽기
     * @param path 파일 경로
     * @return 파일 내용
     */
    std::string readFile(const std::string& path) const;
};

} // namespace module
} // namespace kingsejong
