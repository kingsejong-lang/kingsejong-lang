/**
 * @file SecurityManager.h
 * @brief 보안 관리자 - 파일 접근 제어 및 샌드박스 시스템
 * @author KingSejong Team
 * @date 2025-11-16
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_set>

namespace kingsejong {
namespace security {

/**
 * @enum SecurityMode
 * @brief 보안 모드
 */
enum class SecurityMode
{
    TRUSTED,    ///< 신뢰 모드: 모든 파일 접근 허용
    UNTRUSTED,  ///< 비신뢰 모드: 민감한 경로 차단, /tmp 등만 허용
    SANDBOX     ///< 샌드박스 모드: 화이트리스트에 명시된 경로만 허용
};

/**
 * @enum FileOperation
 * @brief 파일 작업 타입
 */
enum class FileOperation
{
    READ,   ///< 읽기
    WRITE,  ///< 쓰기
    DELETE  ///< 삭제
};

/**
 * @class SecurityManager
 * @brief 보안 관리자 - Singleton 패턴으로 전역 보안 정책 관리
 *
 * 기능:
 * - 보안 모드 설정 (TRUSTED, UNTRUSTED, SANDBOX)
 * - 화이트리스트/블랙리스트 경로 관리
 * - 파일 작업별 접근 제어
 * - 경로 정규화 및 Path Traversal 방어
 */
class SecurityManager
{
public:
    /**
     * @brief 보안 모드 설정
     * @param mode 설정할 보안 모드
     */
    static void setMode(SecurityMode mode);

    /**
     * @brief 현재 보안 모드 조회
     * @return 현재 보안 모드
     */
    static SecurityMode getMode();

    /**
     * @brief 경로 화이트리스트 추가
     * @param path 허용할 경로 (디렉토리는 / 로 끝나야 함)
     */
    static void allowPath(const std::string& path);

    /**
     * @brief 경로 블랙리스트 추가
     * @param path 차단할 경로 (디렉토리는 / 로 끝나야 함)
     */
    static void blockPath(const std::string& path);

    /**
     * @brief 읽기 전용 경로로 설정
     * @param path 읽기 전용으로 설정할 경로
     */
    static void setReadOnly(const std::string& path);

    /**
     * @brief 파일 접근 권한 확인
     * @param path 확인할 파일 경로
     * @param operation 수행할 작업
     * @return 접근 허용 여부
     */
    static bool checkFileAccess(const std::string& path, FileOperation operation);

    /**
     * @brief 보안 관리자 초기화 (모든 설정 리셋)
     */
    static void reset();

private:
    SecurityManager() = default;

    /**
     * @brief 경로 정규화 (절대 경로로 변환, .. 처리)
     * @param path 정규화할 경로
     * @return 정규화된 절대 경로
     */
    static std::string normalizePath(const std::string& path);

    /**
     * @brief 경로가 블랙리스트에 있는지 확인
     * @param normalizedPath 정규화된 경로
     * @return 블랙리스트 포함 여부
     */
    static bool isBlacklisted(const std::string& normalizedPath);

    /**
     * @brief 경로가 화이트리스트에 있는지 확인
     * @param normalizedPath 정규화된 경로
     * @return 화이트리스트 포함 여부
     */
    static bool isWhitelisted(const std::string& normalizedPath);

    /**
     * @brief 경로가 읽기 전용인지 확인
     * @param normalizedPath 정규화된 경로
     * @return 읽기 전용 여부
     */
    static bool isReadOnly(const std::string& normalizedPath);

    /**
     * @brief UNTRUSTED 모드에서 민감한 경로 확인
     * @param normalizedPath 정규화된 경로
     * @return 민감한 경로 여부
     */
    static bool isSensitivePath(const std::string& normalizedPath);

    /**
     * @brief 경로가 prefix로 시작하는지 확인 (디렉토리 매칭용)
     * @param path 확인할 경로
     * @param prefix 접두사
     * @return 접두사 일치 여부
     */
    static bool pathStartsWith(const std::string& path, const std::string& prefix);

    // 전역 상태
    static SecurityMode currentMode_;
    static std::unordered_set<std::string> whitelist_;
    static std::unordered_set<std::string> blacklist_;
    static std::unordered_set<std::string> readOnlyPaths_;
};

} // namespace security
} // namespace kingsejong
