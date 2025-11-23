#pragma once

/**
 * @file UnusedImportRule.h
 * @brief 사용하지 않는 임포트 검출 규칙
 * @author KingSejong Team
 * @date 2025-11-23
 */

#include "linter/Rule.h"
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace kingsejong {
namespace linter {
namespace rules {

/**
 * @class UnusedImportRule
 * @brief 사용하지 않는 import 문 감지
 *
 * 감지 방법:
 * - import된 모듈 경로 수집
 * - stdlib 함수 호출 시 모듈 사용 여부 확인
 * - 사용되지 않은 import 보고
 */
class UnusedImportRule : public Rule
{
public:
    /// @brief UnusedImportRule 생성자
    UnusedImportRule()
        : Rule("unused-import", "사용하지 않는 import 문 감지")
    {
    }

    /**
     * @brief 프로그램 분석 실행
     * @param program 분석할 AST 프로그램
     */
    void analyze(ast::Program* program) override;

protected:
    /// @brief Import 문 추적
    void onImportStatement(ast::ImportStatement* stmt) override;

    /// @brief 함수 호출 시 import 사용 여부 확인
    void onCallExpression(ast::CallExpression* expr) override;

private:
    /**
     * @brief 모듈 경로에서 모듈명 추출
     * @param modulePath "stdlib/math" -> "math"
     */
    std::string extractModuleName(const std::string& modulePath) const;

    /**
     * @brief 함수 호출이 어떤 모듈의 함수인지 추정
     * @param functionName 함수 이름
     * @return 모듈명 (추정 불가 시 빈 문자열)
     */
    std::string guessModuleFromFunction(const std::string& functionName) const;

    struct ImportInfo
    {
        std::string modulePath;   ///< 전체 모듈 경로
        int line;                 ///< import 문 줄 번호
        int column;               ///< import 문 열 번호
    };

    std::unordered_map<std::string, ImportInfo> importedModules_;  ///< 모듈명 -> Import 정보
    std::unordered_set<std::string> usedModules_;  ///< 사용된 모듈명 목록
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
