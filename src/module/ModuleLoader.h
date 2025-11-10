#pragma once

/**
 * @file ModuleLoader.h
 * @brief KingSejong 언어 모듈 로더
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include "../evaluator/Environment.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>

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

    /// 기본 검색 경로
    std::string basePath_;

public:
    /**
     * @brief 생성자
     * @param basePath 모듈 검색 기본 경로 (기본값: 현재 디렉토리)
     */
    explicit ModuleLoader(const std::string& basePath = ".");

    /**
     * @brief 모듈 로딩
     * @param modulePath 모듈 경로 (예: "math", "utils/helper")
     * @return 모듈의 Environment
     * @throws RuntimeError 모듈을 찾을 수 없거나 순환 참조 발생 시
     */
    std::shared_ptr<evaluator::Environment> loadModule(const std::string& modulePath);

    /**
     * @brief 경로 해석
     * @param modulePath 모듈 경로
     * @return 절대 경로
     */
    std::string resolvePath(const std::string& modulePath) const;

    /**
     * @brief 캐시 초기화
     */
    void clearCache();

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

    /**
     * @brief 파일 내용 읽기
     * @param path 파일 경로
     * @return 파일 내용
     */
    std::string readFile(const std::string& path) const;
};

} // namespace module
} // namespace kingsejong
