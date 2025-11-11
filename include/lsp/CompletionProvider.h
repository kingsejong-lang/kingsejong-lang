#pragma once

/**
 * @file CompletionProvider.h
 * @brief LSP 자동 완성 제공자
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * 커서 위치에 따라 적절한 자동 완성 항목을 제안합니다.
 */

#include <string>
#include <vector>
#include <set>
#include "lsp/DocumentManager.h"

namespace kingsejong {
namespace lsp {

/**
 * @class CompletionProvider
 * @brief 자동 완성 제공자
 *
 * KingSejong 언어의 자동 완성을 제공합니다.
 * 키워드, 변수명, 함수명, 내장 함수 등을 제안합니다.
 *
 * 제공하는 완성 항목:
 * - 키워드: 변수, 상수, 함수, 만약, 반복 등
 * - 변수명: 문서에서 선언된 변수들
 * - 함수명: 문서에서 선언된 함수들
 * - 내장 함수: 출력, 타입, 길이
 *
 * Thread Safety: NOT thread-safe (단일 스레드)
 * Memory: RAII - 자동 메모리 관리
 *
 * Example:
 * ```cpp
 * CompletionProvider provider;
 * DocumentManager::Document doc("file:///test.ksj", "변수 x = 10", 1);
 *
 * auto items = provider.provideCompletions(doc, 0, 0);
 * for (const auto& item : items) {
 *     std::cout << item.label << " (" << item.detail << ")" << std::endl;
 * }
 * ```
 */
class CompletionProvider {
public:
    /**
     * @enum CompletionItemKind
     * @brief 완성 항목 종류 (LSP 표준)
     *
     * LSP 표준 CompletionItemKind 값들:
     * - Keyword = 14
     * - Variable = 6
     * - Function = 3
     */
    enum class CompletionItemKind {
        Keyword = 14,      ///< 키워드 (변수, 함수, 만약 등)
        Variable = 6,      ///< 변수명
        Function = 3       ///< 함수명
    };

    /**
     * @struct CompletionItem
     * @brief 완성 항목
     *
     * LSP CompletionItem 구조체입니다.
     */
    struct CompletionItem {
        std::string label;          ///< 표시 텍스트
        CompletionItemKind kind;    ///< 종류
        std::string detail;         ///< 상세 설명 (optional)
        std::string documentation;  ///< 문서 (optional)

        /**
         * @brief 완성 항목 생성자
         * @param l 라벨
         * @param k 종류
         * @param d 상세 설명 (기본: 빈 문자열)
         * @param doc 문서 (기본: 빈 문자열)
         */
        CompletionItem(const std::string& l,
                      CompletionItemKind k,
                      const std::string& d = "",
                      const std::string& doc = "")
            : label(l)
            , kind(k)
            , detail(d)
            , documentation(doc)
        {}
    };

    /**
     * @brief 생성자
     *
     * Postconditions:
     * - 키워드 목록 초기화됨
     */
    CompletionProvider();

    // 복사 금지, 이동 허용
    CompletionProvider(const CompletionProvider&) = delete;
    CompletionProvider& operator=(const CompletionProvider&) = delete;
    CompletionProvider(CompletionProvider&&) noexcept = default;
    CompletionProvider& operator=(CompletionProvider&&) noexcept = default;
    ~CompletionProvider() = default;

    /**
     * @brief 자동 완성 항목 제공
     * @param document 문서
     * @param line 줄 번호 (0부터 시작)
     * @param character 컬럼 번호 (0부터 시작)
     * @return 완성 항목 목록
     *
     * 제공하는 완성 항목:
     * 1. 키워드 (변수, 함수, 만약, 반복 등)
     * 2. 문서에서 선언된 변수명
     * 3. 문서에서 선언된 함수명
     * 4. 내장 함수 (출력, 타입, 길이)
     *
     * Complexity: O(n) where n = document size
     *
     * @note 위치(line, character)는 현재 버전에서는 사용하지 않음
     *       향후 컨텍스트 기반 완성으로 확장 가능
     */
    std::vector<CompletionItem> provideCompletions(
        const DocumentManager::Document& document,
        int line,
        int character
    );

private:
    /**
     * @brief 키워드 완성 항목 반환
     * @return 키워드 완성 항목 목록
     *
     * 포함되는 키워드:
     * - 선언: 변수, 상수, 함수, 반환
     * - 제어: 만약, 아니면, 반복, 계속, 중단
     * - 리터럴: 참, 거짓, 없음
     * - 내장: 출력, 타입, 길이
     *
     * Complexity: O(1) - 캐시된 목록 반환
     */
    std::vector<CompletionItem> getKeywordCompletions() const;

    /**
     * @brief 변수명 완성 항목 추출
     * @param document 문서 (변수 추출용)
     * @return 변수명 완성 항목 목록
     *
     * 문서에서 "변수 x = ..." 또는 "상수 PI = ..." 패턴을 찾아
     * 변수/상수명을 추출합니다.
     *
     * Complexity: O(n) where n = document size
     */
    std::vector<CompletionItem> getVariableCompletions(
        const DocumentManager::Document& document
    ) const;

    /**
     * @brief 함수명 완성 항목 추출
     * @param document 문서 (함수 추출용)
     * @return 함수명 완성 항목 목록
     *
     * 문서에서 "함수 name(...) { ... }" 패턴을 찾아
     * 함수명과 파라미터를 추출합니다.
     *
     * Complexity: O(n) where n = document size
     */
    std::vector<CompletionItem> getFunctionCompletions(
        const DocumentManager::Document& document
    ) const;

    /**
     * @brief 변수명 추출 헬퍼
     * @param content 문서 내용
     * @return 변수명 집합 (중복 제거)
     *
     * "변수 name" 또는 "상수 name" 패턴을 정규식으로 찾습니다.
     *
     * Complexity: O(n)
     */
    std::set<std::string> extractVariableNames(const std::string& content) const;

    /**
     * @struct FunctionSignature
     * @brief 함수 시그니처 정보
     */
    struct FunctionSignature {
        std::string name;              ///< 함수명
        std::vector<std::string> params; ///< 파라미터 목록

        FunctionSignature(const std::string& n,
                         const std::vector<std::string>& p)
            : name(n), params(p) {}
    };

    /**
     * @brief 함수 시그니처 추출 헬퍼
     * @param content 문서 내용
     * @return 함수 시그니처 목록
     *
     * "함수 name(param1, param2) { ... }" 패턴을 찾습니다.
     *
     * Complexity: O(n)
     */
    std::vector<FunctionSignature> extractFunctionSignatures(
        const std::string& content
    ) const;

    /// 캐시된 키워드 목록
    std::vector<CompletionItem> keywordCompletions_;
};

} // namespace lsp
} // namespace kingsejong
