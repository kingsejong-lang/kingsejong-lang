#pragma once

/**
 * @file ImportOrderRule.h
 * @brief 임포트 순서 정리 규칙
 * @author KingSejong Team
 * @date 2025-11-23
 */

#include "linter/Rule.h"
#include <vector>
#include <string>

namespace kingsejong {
namespace linter {
namespace rules {

/**
 * @class ImportOrderRule
 * @brief import 문 순서 검사
 *
 * 검사 규칙:
 * - stdlib 임포트가 먼저 와야 함
 * - 사용자 정의 임포트가 나중에 와야 함
 * - 각 그룹 내에서 알파벳 순 정렬
 * - 그룹 간 빈 줄 권장
 */
class ImportOrderRule : public Rule
{
public:
    /// @brief ImportOrderRule 생성자
    ImportOrderRule()
        : Rule("import-order", "import 문 순서 검사")
    {
    }

    /**
     * @brief 프로그램 분석 실행
     * @param program 분석할 AST 프로그램
     */
    void analyze(ast::Program* program) override;

protected:
    /// @brief Import 문 수집
    void onImportStatement(ast::ImportStatement* stmt) override;

private:
    /**
     * @brief 모듈 경로가 stdlib인지 확인
     * @param modulePath 모듈 경로
     * @return stdlib이면 true
     */
    bool isStdlibImport(const std::string& modulePath) const;

    /**
     * @brief 수집된 import 문들의 순서 검사
     */
    void checkImportOrder();

    struct ImportInfo
    {
        std::string modulePath;   ///< 전체 모듈 경로
        int line;                 ///< import 문 줄 번호
        int column;               ///< import 문 열 번호
        bool isStdlib;            ///< stdlib 임포트 여부
    };

    std::vector<ImportInfo> imports_;  ///< 수집된 import 문 목록 (순서대로)
};

} // namespace rules
} // namespace linter
} // namespace kingsejong
