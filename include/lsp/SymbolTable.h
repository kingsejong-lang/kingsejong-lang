#pragma once

/**
 * @file SymbolTable.h
 * @brief LSP 심볼 테이블 시스템
 * @author KingSejong Team
 * @date 2025-11-12
 *
 * 소스 코드의 심볼 (변수, 함수 등)을 추적하여
 * Go to Definition, Find References 등의 LSP 고급 기능을 지원합니다.
 */

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <unordered_map>

namespace kingsejong {
namespace lsp {

/**
 * @struct Location
 * @brief 소스 코드 위치 정보
 */
struct Location
{
    int line;                ///< 줄 번호 (1부터 시작)
    int column;              ///< 컬럼 번호 (1부터 시작)
    std::string uri;         ///< 파일 URI

    Location() : line(0), column(0), uri("") {}

    Location(int l, int c, const std::string& u)
        : line(l), column(c), uri(u) {}

    /**
     * @brief 비교 연산자 (map 키로 사용하기 위함)
     */
    bool operator<(const Location& other) const
    {
        if (uri != other.uri) return uri < other.uri;
        if (line != other.line) return line < other.line;
        return column < other.column;
    }

    bool operator==(const Location& other) const
    {
        return uri == other.uri && line == other.line && column == other.column;
    }

    bool operator!=(const Location& other) const
    {
        return !(*this == other);
    }
};

/**
 * @enum SymbolKind
 * @brief 심볼 종류
 *
 * LSP SymbolKind 표준과 호환
 */
enum class SymbolKind
{
    Variable = 13,      ///< 변수
    Function = 12,      ///< 함수
    Parameter = 17,     ///< 매개변수
    Module = 2,         ///< 모듈
    Class = 5,          ///< 클래스 (향후 확장)
    Property = 7        ///< 속성 (향후 확장)
};

/**
 * @struct Symbol
 * @brief 심볼 정보
 *
 * 변수, 함수 등의 정의와 사용 위치를 추적합니다.
 */
struct Symbol
{
    std::string name;                       ///< 심볼 이름
    SymbolKind kind;                        ///< 심볼 종류
    Location definitionLocation;            ///< 정의된 위치
    std::vector<Location> references;       ///< 사용된 위치들
    std::string typeInfo;                   ///< 타입 정보 (선택)
    std::string scope;                      ///< 스코프 ("global", "함수:이름" 등)
    std::string documentation;              ///< 문서 문자열 (Hover용)

    Symbol()
        : name("")
        , kind(SymbolKind::Variable)
        , definitionLocation()
        , references()
        , typeInfo("")
        , scope("global")
        , documentation("")
    {}
};

/**
 * @class SymbolTable
 * @brief 심볼 테이블
 *
 * 소스 코드의 모든 심볼을 추적하여 LSP 고급 기능을 지원합니다.
 *
 * 지원 기능:
 * - 심볼 추가 및 조회
 * - 참조 추가 및 조회
 * - 위치 기반 심볼 찾기 (Go to Definition)
 * - 스코프 기반 심볼 찾기
 * - 종류별 필터링
 *
 * Thread Safety: NOT thread-safe
 * Memory: RAII - map/vector로 자동 관리
 *
 * Example:
 * ```cpp
 * SymbolTable table;
 *
 * // 변수 추가
 * Symbol var;
 * var.name = "x";
 * var.kind = SymbolKind::Variable;
 * var.definitionLocation = {1, 5, "test.ksj"};
 * table.addSymbol(var);
 *
 * // 참조 추가
 * table.addReference("x", {5, 10, "test.ksj"});
 *
 * // 심볼 찾기
 * auto symbol = table.findSymbol("x");
 *
 * // 참조 목록 가져오기
 * auto refs = table.getReferences("x");
 * ```
 */
class SymbolTable
{
public:
    /**
     * @brief 기본 생성자
     */
    SymbolTable() = default;

    // 복사 금지, 이동 허용
    SymbolTable(const SymbolTable&) = delete;
    SymbolTable& operator=(const SymbolTable&) = delete;
    SymbolTable(SymbolTable&&) noexcept = default;
    SymbolTable& operator=(SymbolTable&&) noexcept = default;
    ~SymbolTable() = default;

    /**
     * @brief 심볼 추가
     * @param symbol 추가할 심볼
     *
     * Postconditions:
     * - findSymbol(symbol.name) returns the symbol
     *
     * Complexity: O(log n)
     */
    void addSymbol(const Symbol& symbol);

    /**
     * @brief 심볼 찾기 (이름으로)
     * @param name 심볼 이름
     * @return 심볼 (없으면 nullopt)
     *
     * 주의: 같은 이름의 심볼이 여러 개 있을 경우 첫 번째 반환
     * 스코프를 고려하려면 findSymbolInScope 사용
     *
     * Complexity: O(log n)
     */
    std::optional<Symbol> findSymbol(const std::string& name) const;

    /**
     * @brief 스코프 기반 심볼 찾기
     * @param name 심볼 이름
     * @param scope 스코프 이름
     * @return 심볼 (없으면 nullopt)
     *
     * Complexity: O(n) where n = symbols with same name
     */
    std::optional<Symbol> findSymbolInScope(const std::string& name, const std::string& scope) const;

    /**
     * @brief 위치 기반 심볼 찾기
     * @param location 위치
     * @return 심볼 (없으면 nullopt)
     *
     * 해당 위치에 정의되었거나 참조된 심볼을 찾습니다.
     * Go to Definition 기능에 사용됩니다.
     *
     * Complexity: O(n * m) where n = symbols, m = avg references
     */
    std::optional<Symbol> findSymbolAtLocation(const Location& location) const;

    /**
     * @brief 참조 추가
     * @param name 심볼 이름
     * @param location 참조 위치
     *
     * Preconditions:
     * - findSymbol(name).has_value() == true
     *
     * Postconditions:
     * - getReferences(name) contains location
     *
     * Complexity: O(log n)
     */
    void addReference(const std::string& name, const Location& location);

    /**
     * @brief 참조 목록 가져오기
     * @param name 심볼 이름
     * @return 참조 위치 목록
     *
     * Find References 기능에 사용됩니다.
     *
     * Complexity: O(log n)
     */
    std::vector<Location> getReferences(const std::string& name) const;

    /**
     * @brief 종류별 심볼 가져오기
     * @param kind 심볼 종류
     * @return 해당 종류의 모든 심볼
     *
     * Complexity: O(n)
     */
    std::vector<Symbol> getAllSymbolsByKind(SymbolKind kind) const;

    /**
     * @brief 모든 심볼 가져오기
     * @return 모든 심볼 목록
     *
     * Complexity: O(n)
     */
    std::vector<Symbol> getAllSymbols() const;

    /**
     * @brief 심볼 테이블 비우기
     *
     * Postconditions:
     * - getAllSymbols().empty() == true
     *
     * Complexity: O(n)
     */
    void clear();

    /**
     * @brief 심볼 개수 반환
     * @return 심볼 개수
     *
     * Complexity: O(1)
     */
    size_t size() const;

private:
    /**
     * @brief 심볼 저장소
     *
     * Key: 심볼 이름 + 스코프
     * Value: 심볼 정보
     *
     * 같은 이름의 심볼이 다른 스코프에 존재할 수 있으므로
     * multimap 사용
     */
    std::multimap<std::string, Symbol> symbols_;

    /**
     * @brief 위치 → 심볼 인덱스
     *
     * 빠른 위치 기반 조회를 위한 인덱스
     * Key: Location
     * Value: 심볼 이름
     */
    std::map<Location, std::string> locationIndex_;
};

} // namespace lsp
} // namespace kingsejong
