/**
 * @file DocumentManager.cpp
 * @brief LSP 문서 관리 시스템 구현
 * @author KingSejong Team
 * @date 2025-11-12
 */

#include "lsp/DocumentManager.h"
#include <stdexcept>

namespace kingsejong {
namespace lsp {

void DocumentManager::openDocument(
    const std::string& uri,
    const std::string& content,
    int version)
{
    // 이미 열린 문서인지 확인
    if (documents_.find(uri) != documents_.end()) {
        throw std::runtime_error("Document already open: " + uri);
    }

    // 문서 추가
    documents_.emplace(uri, Document(uri, content, version));
}

void DocumentManager::updateDocument(
    const std::string& uri,
    const std::string& content,
    int version)
{
    // 문서 찾기
    auto it = documents_.find(uri);
    if (it == documents_.end()) {
        throw std::runtime_error("Document not found: " + uri);
    }

    // 문서 업데이트
    it->second.content = content;
    it->second.version = version;
}

void DocumentManager::closeDocument(const std::string& uri)
{
    // 문서 제거 (없어도 예외 안 던짐)
    documents_.erase(uri);
}

const DocumentManager::Document* DocumentManager::getDocument(const std::string& uri) const
{
    auto it = documents_.find(uri);
    if (it == documents_.end()) {
        return nullptr;
    }
    return &it->second;
}

} // namespace lsp
} // namespace kingsejong
