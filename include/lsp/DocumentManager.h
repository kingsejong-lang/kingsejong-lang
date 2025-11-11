#pragma once

/**
 * @file DocumentManager.h
 * @brief LSP 문서 관리 시스템
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * LSP 클라이언트가 열거나 수정한 문서를 메모리에 관리합니다.
 */

#include <string>
#include <map>
#include <memory>

namespace kingsejong {
namespace lsp {

/**
 * @class DocumentManager
 * @brief 열린 문서들을 메모리에 관리
 *
 * LSP 클라이언트에서 열거나 수정한 문서를 추적합니다.
 * URI를 키로 사용하여 문서에 접근합니다.
 *
 * Thread Safety: NOT thread-safe (단일 스레드)
 * Memory: RAII - map으로 자동 관리
 *
 * @invariant 열린 문서는 항상 유효한 URI를 가짐
 * @invariant 문서 버전은 항상 양수
 *
 * Example:
 * ```cpp
 * DocumentManager manager;
 *
 * // 문서 열기
 * manager.openDocument("file:///test.ksj", "변수 x = 10", 1);
 *
 * // 문서 수정
 * manager.updateDocument("file:///test.ksj", "변수 x = 20", 2);
 *
 * // 문서 조회
 * const auto* doc = manager.getDocument("file:///test.ksj");
 * std::cout << doc->content << std::endl;
 *
 * // 문서 닫기
 * manager.closeDocument("file:///test.ksj");
 * ```
 */
class DocumentManager {
public:
    /**
     * @struct Document
     * @brief 문서 정보
     */
    struct Document {
        std::string uri;       ///< 문서 URI (file:///path/to/file.ksj)
        std::string content;   ///< 문서 내용
        int version;          ///< 문서 버전 (변경 추적용)

        /**
         * @brief 문서 생성자
         * @param u URI
         * @param c 내용
         * @param v 버전
         */
        Document(const std::string& u, const std::string& c, int v)
            : uri(u)
            , content(c)
            , version(v)
        {}
    };

    // 복사 금지, 이동 허용
    DocumentManager() = default;
    DocumentManager(const DocumentManager&) = delete;
    DocumentManager& operator=(const DocumentManager&) = delete;
    DocumentManager(DocumentManager&&) noexcept = default;
    DocumentManager& operator=(DocumentManager&&) noexcept = default;
    ~DocumentManager() = default;

    /**
     * @brief 문서 열기
     * @param uri 문서 URI
     * @param content 초기 내용
     * @param version 초기 버전
     * @throws std::runtime_error if document already open
     *
     * Preconditions:
     * - uri가 비어있지 않음
     * - version > 0
     *
     * Postconditions:
     * - getDocument(uri) != nullptr
     *
     * Complexity: O(log n) where n = number of documents
     */
    void openDocument(const std::string& uri,
                     const std::string& content,
                     int version);

    /**
     * @brief 문서 수정
     * @param uri 문서 URI
     * @param content 새 내용
     * @param version 새 버전
     * @throws std::runtime_error if document not found
     *
     * Preconditions:
     * - getDocument(uri) != nullptr
     *
     * Postconditions:
     * - getDocument(uri)->version == version
     * - getDocument(uri)->content == content
     *
     * Complexity: O(log n)
     */
    void updateDocument(const std::string& uri,
                       const std::string& content,
                       int version);

    /**
     * @brief 문서 닫기
     * @param uri 문서 URI
     *
     * Postconditions:
     * - getDocument(uri) == nullptr
     *
     * Complexity: O(log n)
     */
    void closeDocument(const std::string& uri);

    /**
     * @brief 문서 조회
     * @param uri 문서 URI
     * @return Document 포인터 (없으면 nullptr)
     *
     * Complexity: O(log n)
     */
    const Document* getDocument(const std::string& uri) const;

    /**
     * @brief 모든 문서 조회
     * @return 문서 맵 (URI → Document)
     *
     * Complexity: O(1)
     */
    const std::map<std::string, Document>& getAllDocuments() const {
        return documents_;
    }

private:
    /// URI별 문서 맵 (RAII)
    std::map<std::string, Document> documents_;
};

} // namespace lsp
} // namespace kingsejong
