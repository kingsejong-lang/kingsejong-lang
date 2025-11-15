#pragma once

/**
 * @file SymbolTable.h
 * @brief 심볼 테이블 (Symbol Table) - 변수/함수 추적 및 스코프 관리
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "types/Type.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>

namespace kingsejong {
namespace semantic {

/**
 * @enum SymbolKind
 * @brief 심볼의 종류
 */
enum class SymbolKind
{
    VARIABLE,   ///< 변수
    FUNCTION,   ///< 함수
    TYPE,       ///< 타입 (향후 사용자 정의 타입)
    MODULE      ///< 모듈 (향후 모듈 시스템)
};

/**
 * @struct Symbol
 * @brief 심볼 정보
 */
struct Symbol
{
    std::string name;           ///< 심볼 이름
    SymbolKind kind;            ///< 심볼 종류
    types::Type* type;          ///< 타입 정보
    bool isMutable;             ///< 변경 가능 여부 (상수 vs 변수)
    int line;                   ///< 정의된 줄 번호
    int column;                 ///< 정의된 열 번호

    /**
     * @brief Symbol 기본 생성자
     */
    Symbol()
        : name(""), kind(SymbolKind::VARIABLE), type(nullptr),
          isMutable(true), line(0), column(0)
    {
    }

    /**
     * @brief Symbol 생성자
     */
    Symbol(const std::string& name, SymbolKind kind, types::Type* type,
           bool isMutable = true, int line = 0, int column = 0)
        : name(name), kind(kind), type(type), isMutable(isMutable),
          line(line), column(column)
    {
    }
};

/**
 * @class Scope
 * @brief 스코프 (변수/함수의 유효 범위)
 */
class Scope
{
public:
    /**
     * @brief Scope 생성자
     * @param parent 부모 스코프 (nullptr이면 전역 스코프)
     */
    explicit Scope(Scope* parent = nullptr)
        : parent_(parent)
    {
    }

    /**
     * @brief 심볼 정의
     * @param symbol 정의할 심볼
     * @return 성공하면 true, 이미 정의되어 있으면 false
     */
    bool define(const Symbol& symbol)
    {
        if (symbols_.count(symbol.name))
        {
            return false;  // 이미 정의됨
        }
        symbols_[symbol.name] = symbol;
        return true;
    }

    /**
     * @brief 심볼 조회 (현재 스코프에서만)
     * @param name 심볼 이름
     * @return 심볼 포인터 (없으면 nullptr)
     */
    Symbol* lookupLocal(const std::string& name) const
    {
        auto it = symbols_.find(name);
        if (it != symbols_.end())
        {
            return const_cast<Symbol*>(&it->second);
        }
        return nullptr;
    }

    /**
     * @brief 심볼 조회 (부모 스코프까지 탐색)
     * @param name 심볼 이름
     * @return 심볼 포인터 (없으면 nullptr)
     */
    Symbol* lookup(const std::string& name) const
    {
        // 현재 스코프에서 찾기
        auto it = symbols_.find(name);
        if (it != symbols_.end())
        {
            return const_cast<Symbol*>(&it->second);
        }

        // 부모 스코프에서 찾기
        if (parent_)
        {
            return parent_->lookup(name);
        }

        return nullptr;  // 미정의
    }

    /**
     * @brief 심볼이 정의되어 있는지 확인
     * @param name 심볼 이름
     * @return 정의되어 있으면 true
     */
    bool isDefined(const std::string& name) const
    {
        return lookup(name) != nullptr;
    }

    /**
     * @brief 부모 스코프 반환
     * @return 부모 스코프 포인터 (전역 스코프면 nullptr)
     */
    Scope* parent() const
    {
        return parent_;
    }

    /**
     * @brief 현재 스코프의 모든 심볼 반환
     * @return 심볼 맵
     */
    const std::unordered_map<std::string, Symbol>& symbols() const
    {
        return symbols_;
    }

private:
    Scope* parent_;                                      ///< 부모 스코프
    std::unordered_map<std::string, Symbol> symbols_;   ///< 심볼 맵
};

/**
 * @class SymbolTable
 * @brief 심볼 테이블 - 스코프 관리 및 심볼 추적
 */
class SymbolTable
{
public:
    /**
     * @brief SymbolTable 생성자
     */
    SymbolTable()
        : currentScope_(nullptr)
    {
        enterScope();  // 전역 스코프 생성
    }

    /**
     * @brief 새로운 스코프 진입
     */
    void enterScope()
    {
        auto scope = std::make_unique<Scope>(currentScope_);
        currentScope_ = scope.get();
        scopes_.push_back(std::move(scope));
    }

    /**
     * @brief 현재 스코프 탈출
     */
    void exitScope()
    {
        if (currentScope_ && currentScope_->parent())
        {
            currentScope_ = currentScope_->parent();
        }
    }

    /**
     * @brief 심볼 정의
     * @param name 심볼 이름
     * @param kind 심볼 종류
     * @param type 타입
     * @param isMutable 변경 가능 여부
     * @param line 정의된 줄 번호
     * @param column 정의된 열 번호
     * @return 성공하면 true, 이미 정의되어 있으면 false
     */
    bool define(const std::string& name, SymbolKind kind, types::Type* type,
                bool isMutable = true, int line = 0, int column = 0)
    {
        return currentScope_->define(Symbol{name, kind, type, isMutable, line, column});
    }

    /**
     * @brief 심볼 조회
     * @param name 심볼 이름
     * @return 심볼 포인터 (없으면 nullptr)
     */
    Symbol* lookup(const std::string& name) const
    {
        return currentScope_->lookup(name);
    }

    /**
     * @brief 심볼이 정의되어 있는지 확인
     * @param name 심볼 이름
     * @return 정의되어 있으면 true
     */
    bool isDefined(const std::string& name) const
    {
        return currentScope_->isDefined(name);
    }

    /**
     * @brief 심볼이 변수인지 확인
     * @param name 심볼 이름
     * @return 변수이면 true
     */
    bool isVariable(const std::string& name)
    {
        Symbol* symbol = lookup(name);
        return symbol && symbol->kind == SymbolKind::VARIABLE;
    }

    /**
     * @brief 심볼이 함수인지 확인
     * @param name 심볼 이름
     * @return 함수이면 true
     */
    bool isFunction(const std::string& name)
    {
        Symbol* symbol = lookup(name);
        return symbol && symbol->kind == SymbolKind::FUNCTION;
    }

    /**
     * @brief 현재 스코프 반환
     * @return 현재 스코프 포인터
     */
    Scope* currentScope() const
    {
        return currentScope_;
    }

    /**
     * @brief 전역 스코프 반환
     * @return 전역 스코프 포인터
     */
    Scope* globalScope() const
    {
        return scopes_.empty() ? nullptr : scopes_[0].get();
    }

    /**
     * @brief 모든 스코프 초기화 (테스트용)
     */
    void clear()
    {
        scopes_.clear();
        currentScope_ = nullptr;
        enterScope();  // 전역 스코프 재생성
    }

private:
    Scope* currentScope_;                        ///< 현재 스코프
    std::vector<std::unique_ptr<Scope>> scopes_; ///< 모든 스코프 (소유권 관리)
};

} // namespace semantic
} // namespace kingsejong
