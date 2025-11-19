#pragma once

/**
 * @file SemanticAnalyzer.h
 * @brief 의미 분석기 (Semantic Analyzer) - 타입 검사, 이름 해석, 모호성 해결
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include "semantic/SymbolTable.h"
#include "ast/Node.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "types/Type.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>

namespace kingsejong {
namespace semantic {

/**
 * @struct SemanticError
 * @brief 의미 분석 에러 정보
 */
struct SemanticError
{
    std::string message;    ///< 에러 메시지
    int line;               ///< 에러 발생 줄
    int column;             ///< 에러 발생 열
    std::string filename;   ///< 파일 이름

    SemanticError(const std::string& msg, int l = 0, int c = 0, const std::string& file = "")
        : message(msg), line(l), column(c), filename(file)
    {
    }

    /**
     * @brief 에러 메시지를 형식화하여 반환
     */
    std::string toString() const
    {
        if (line > 0)
        {
            return filename + ":" + std::to_string(line) + ":" + std::to_string(column) + ": " + message;
        }
        return message;
    }
};

/**
 * @class SemanticAnalyzer
 * @brief 의미 분석기 - Parser 이후 AST를 검증하고 보강
 *
 * 4단계 분석:
 * 1. Symbol Table 구축: 모든 변수/함수 등록
 * 2. 이름 해석 (Name Resolution): 모든 참조가 정의된 심볼인지 확인
 * 3. 타입 검사 (Type Checking): 타입 일치성 검증
 * 4. 모호성 해결: 파서에서 휴리스틱으로 처리한 부분을 정확히 판단
 */
class SemanticAnalyzer
{
public:
    /**
     * @brief SemanticAnalyzer 생성자
     */
    SemanticAnalyzer()
        : symbolTable_()
        , inFunction_(false)
        , inAsyncFunction_(false)
        , currentFunctionName_("")
        , expectedReturnType_(nullptr)
    {
        initBuiltinFunctions();
    }

    /**
     * @brief 프로그램 전체를 분석
     * @param program AST 루트 노드
     * @return 성공하면 true, 에러가 있으면 false
     */
    bool analyze(ast::Program* program);

    /**
     * @brief 에러 목록 반환
     */
    const std::vector<SemanticError>& errors() const
    {
        return errors_;
    }

    /**
     * @brief Symbol Table 반환
     */
    const SymbolTable& symbolTable() const
    {
        return symbolTable_;
    }

    /**
     * @brief 에러 초기화 (테스트용)
     */
    void clearErrors()
    {
        errors_.clear();
        symbolTable_.clear();
    }

private:
    SymbolTable symbolTable_;                    ///< 심볼 테이블
    std::vector<SemanticError> errors_;          ///< 에러 목록
    std::unordered_set<std::string> builtins_;   ///< Builtin 함수 목록
    std::string filename_;                        ///< 현재 분석 중인 파일 이름

    // 함수 컨텍스트 추적 (반환 타입 검사용)
    bool inFunction_;                             ///< 현재 함수 내부인지 여부
    bool inAsyncFunction_;                        ///< 현재 비동기 함수 내부인지 여부 (Phase 7.3)
    std::string currentFunctionName_;             ///< 현재 분석 중인 함수 이름
    types::Type* expectedReturnType_;             ///< 함수의 첫 return문에서 추론된 타입

    // ========================================================================
    // Builtin 함수 초기화
    // ========================================================================

    /**
     * @brief Builtin 함수 목록 초기화
     */
    void initBuiltinFunctions();

    /**
     * @brief 식별자가 builtin 함수인지 확인
     */
    bool isBuiltinFunction(const std::string& name) const;

    // ========================================================================
    // Phase 1+2: Symbol Table 구축 및 이름 해석 (통합)
    // ========================================================================

    /**
     * @brief Symbol Table 구축과 이름 해석을 동시에 수행
     * @param program 프로그램 AST
     *
     * 기존 buildSymbolTable + resolveNames를 통합하여
     * Scope를 일관되게 관리하고 변수 스코프 격리를 올바르게 구현
     */
    void analyzeAndResolve(ast::Program* program);

    /**
     * @brief Statement 분석: 심볼 등록 + 이름 해석
     * @param stmt 분석할 문장
     *
     * 변수 선언: 현재 스코프에 등록
     * 식별자 참조: 스코프 체인에서 검색
     * 블록/함수: 새 스코프 생성 및 관리
     */
    void analyzeAndResolveStatement(const ast::Statement* stmt);

    /**
     * @brief Expression의 이름 해석
     * @param expr 분석할 표현식
     *
     * 모든 식별자가 정의된 심볼인지 확인
     */
    void analyzeAndResolveExpression(const ast::Expression* expr);

    // ========================================================================
    // Phase 1: Symbol Table 구축 (레거시 - 참조용)
    // ========================================================================

    /**
     * @brief Symbol Table 구축 (DEPRECATED - analyzeAndResolve 사용)
     * @param program 프로그램 AST
     */
    void buildSymbolTable(ast::Program* program);

    /**
     * @brief Statement를 분석하여 심볼 등록
     */
    void registerSymbolFromStatement(const ast::Statement* stmt);

    /**
     * @brief 변수 선언 등록
     */
    void registerVariable(ast::VarDeclaration* varDecl);

    /**
     * @brief 함수 선언 등록
     */
    void registerFunction(const std::string& name, ast::FunctionLiteral* funcLit);

    // ========================================================================
    // Phase 2: 이름 해석 (레거시 - 참조용)
    // ========================================================================

    /**
     * @brief 이름 해석 (DEPRECATED - analyzeAndResolve 사용)
     * @param program 프로그램 AST
     */
    void resolveNames(ast::Program* program);

    /**
     * @brief Statement의 이름 해석
     */
    void resolveNamesInStatement(const ast::Statement* stmt);

    /**
     * @brief Expression의 이름 해석
     */
    void resolveNamesInExpression(const ast::Expression* expr);

    // ========================================================================
    // Phase 3: 타입 검사 (Type Checking)
    // ========================================================================

    /**
     * @brief 타입 검사
     * @param program 프로그램 AST
     */
    void checkTypes(ast::Program* program);

    /**
     * @brief Statement의 타입 검사
     */
    void checkTypesInStatement(const ast::Statement* stmt);

    /**
     * @brief Expression의 타입 추론 및 검사
     * @return 추론된 타입
     */
    types::Type* inferType(const ast::Expression* expr);

    /**
     * @brief 두 타입이 호환되는지 확인
     */
    bool isTypeCompatible(types::Type* expected, types::Type* actual);

    // ========================================================================
    // Phase 4: 모호성 해결
    // ========================================================================

    /**
     * @brief 모호성 해결: 파서에서 구문적으로 처리한 부분을 의미론적으로 검증
     * @param program 프로그램 AST
     *
     * 예: "i가 1부터 10까지" vs "데이터가 존재한다"
     * Parser는 LL(4) lookahead로 구문적으로 구분하고,
     * Semantic Analyzer는 Symbol Table로 의미론적으로 검증
     */
    void resolveAmbiguities(ast::Program* program);

    /**
     * @brief Statement의 모호성 해결
     */
    void resolveAmbiguitiesInStatement(ast::Statement* stmt);

    // ========================================================================
    // 유틸리티
    // ========================================================================

    /**
     * @brief 에러 추가
     */
    void addError(const std::string& message, int line = 0, int column = 0);
};

} // namespace semantic
} // namespace kingsejong
