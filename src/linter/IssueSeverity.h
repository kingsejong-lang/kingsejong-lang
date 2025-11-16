#pragma once

/**
 * @file IssueSeverity.h
 * @brief Linter 이슈 심각도 정의
 * @author KingSejong Team
 * @date 2025-11-16
 */

namespace kingsejong {
namespace linter {

/**
 * @enum IssueSeverity
 * @brief 이슈 심각도
 */
enum class IssueSeverity
{
    ERROR,      ///< 에러 (반드시 수정)
    WARNING,    ///< 경고 (수정 권장)
    INFO,       ///< 정보 (참고)
    HINT        ///< 힌트 (제안)
};

} // namespace linter
} // namespace kingsejong
