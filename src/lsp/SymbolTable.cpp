/**
 * @file SymbolTable.cpp
 * @brief SymbolTable 구현
 * @author KingSejong Team
 * @date 2025-11-12
 */

#include "lsp/SymbolTable.h"
#include <algorithm>

namespace kingsejong {
namespace lsp {

void SymbolTable::addSymbol(const Symbol& symbol)
{
    // 심볼 저장
    symbols_.insert({symbol.name, symbol});

    // 위치 인덱스 추가 (정의 위치)
    locationIndex_[symbol.definitionLocation] = symbol.name;
}

std::optional<Symbol> SymbolTable::findSymbol(const std::string& name) const
{
    auto it = symbols_.find(name);
    if (it == symbols_.end())
    {
        return std::nullopt;
    }
    return it->second;
}

std::optional<Symbol> SymbolTable::findSymbolInScope(const std::string& name, const std::string& scope) const
{
    auto range = symbols_.equal_range(name);
    for (auto it = range.first; it != range.second; ++it)
    {
        if (it->second.scope == scope)
        {
            return it->second;
        }
    }
    return std::nullopt;
}

std::optional<Symbol> SymbolTable::findSymbolAtLocation(const Location& location) const
{
    // 위치 인덱스에서 먼저 찾기 (정의 위치)
    auto it = locationIndex_.find(location);
    if (it != locationIndex_.end())
    {
        return findSymbol(it->second);
    }

    // 참조 위치에서 찾기
    for (const auto& [name, symbol] : symbols_)
    {
        for (const auto& ref : symbol.references)
        {
            if (ref == location)
            {
                return symbol;
            }
        }
    }

    return std::nullopt;
}

void SymbolTable::addReference(const std::string& name, const Location& location)
{
    // 심볼 찾기
    auto it = symbols_.find(name);
    if (it == symbols_.end())
    {
        return; // 심볼이 없으면 무시
    }

    // 참조 추가 (mutable하게 수정)
    // multimap의 value를 수정하려면 const_cast 필요
    auto& symbol = const_cast<Symbol&>(it->second);
    symbol.references.push_back(location);

    // 위치 인덱스 추가
    locationIndex_[location] = name;
}

std::vector<Location> SymbolTable::getReferences(const std::string& name) const
{
    auto it = symbols_.find(name);
    if (it == symbols_.end())
    {
        return {};
    }
    return it->second.references;
}

std::vector<Symbol> SymbolTable::getAllSymbolsByKind(SymbolKind kind) const
{
    std::vector<Symbol> result;
    for (const auto& [name, symbol] : symbols_)
    {
        if (symbol.kind == kind)
        {
            result.push_back(symbol);
        }
    }
    return result;
}

std::vector<Symbol> SymbolTable::getAllSymbols() const
{
    std::vector<Symbol> result;
    for (const auto& [name, symbol] : symbols_)
    {
        result.push_back(symbol);
    }
    return result;
}

void SymbolTable::clear()
{
    symbols_.clear();
    locationIndex_.clear();
}

size_t SymbolTable::size() const
{
    return symbols_.size();
}

} // namespace lsp
} // namespace kingsejong
