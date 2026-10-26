#pragma once

/**
 * @file Formatter.h
 * @brief KingSejong 코드 포매터
 * @author KingSejong Team
 * @date 2025-11-15
 *
 * AST 를 일관된 스타일의 소스 코드로 다시 쓴다. 원본 소스를 함께 주면 주석과 문장 사이의
 * 빈 줄을 보존한다. 포매터가 만든 코드가 원본과 같은 AST·같은 주석인지는 verify() 로 확인한다.
 */

#include "ast/Node.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include <string>
#include <sstream>
#include <vector>
#include <nlohmann/json.hpp>

namespace kingsejong {
namespace formatter {

/**
 * @struct FormatterOptions
 * @brief 포매터 설정 옵션
 */
struct FormatterOptions
{
    int indentSize = 4;              ///< 들여쓰기 크기 (기본 4칸)
    bool useSpaces = true;            ///< 스페이스 사용 (false면 탭)
    bool spaceAroundOperators = true; ///< 연산자 주변 공백
    bool spaceAfterComma = true;      ///< 콤마 뒤 공백
    bool spaceBeforeBrace = true;     ///< 중괄호 앞 공백
    int maxLineLength = 100;          ///< 최대 줄 길이
};

/**
 * @struct SourceComment
 * @brief 소스에서 찾은 주석 한 개
 */
struct SourceComment
{
    int line = 0;          ///< 1부터 시작하는 줄 번호
    std::string text;      ///< '#' 부터 줄 끝까지 (끝 공백 제외)
    bool trailing = false; ///< 같은 줄 앞에 코드가 있으면 true
};

/**
 * @class Formatter
 * @brief AST 를 소스 코드로 다시 쓰는 포매터
 *
 * @code
 * Formatter formatter;
 * std::string out = formatter.format(program.get(), source);
 * std::string reason;
 * if (!Formatter::verify(program.get(), source, out, reason)) { ... 원본을 두고 reason 을 알린다 ... }
 * @endcode
 */
class Formatter
{
public:
    explicit Formatter(const FormatterOptions& options = FormatterOptions())
        : options_(options)
        , currentIndent_(0)
    {
    }

    /**
     * @brief 프로그램을 포맷팅 (주석·빈 줄 정보 없음, 최상위 문장 사이에 빈 줄 하나)
     */
    std::string format(const ast::Program* program);

    /**
     * @brief 원본 소스를 참고해 포맷팅한다: 주석을 제자리에 두고, 문장 사이의 빈 줄을 하나까지 보존한다
     */
    std::string format(const ast::Program* program, const std::string& source);

    /// 직전 format() 에서 출력하지 못한 노드가 있었는지
    bool hasUnsupportedNodes() const { return !unsupported_.empty(); }

    /// 직전 format() 에서 출력하지 못한 노드 목록 ("노드종류 (줄 N)")
    const std::vector<std::string>& unsupportedNodes() const { return unsupported_; }

    /**
     * @brief 포맷 결과가 원본과 같은 프로그램인지 확인한다
     * @param original 원본 AST
     * @param originalSource 원본 소스 (주석 비교용)
     * @param formatted 포매터가 만든 코드
     * @param reason 다르면 이유
     * @return 다시 파싱한 AST 와 주석 목록이 원본과 같으면 true
     *
     * 포매터의 출력 규칙이 틀려도 파일이 망가지지 않도록, 덮어쓰기 전에 반드시 거친다.
     */
    static bool verify(const ast::Program* original, const std::string& originalSource,
                       const std::string& formatted, std::string& reason);

    /// AST 의 구조와 값(이름, 리터럴, 연산자, 조사 등)을 한 문자열로 펼친다. 위치 정보는 넣지 않는다
    static std::string fingerprint(const ast::Program* program);

    /// 소스에서 주석을 찾는다 (문자열 안의 '#' 은 제외)
    static std::vector<SourceComment> extractComments(const std::string& source);

    bool loadConfig(const std::string& filepath);
    bool loadConfigFromString(const std::string& jsonString);
    bool loadConfigFromCurrentDir();

    const FormatterOptions& getOptions() const { return options_; }
    void setOptions(const FormatterOptions& options) { options_ = options; }

private:
    // 문장 포맷팅
    void formatStatement(const ast::Statement* stmt);
    void formatVarDeclaration(const ast::VarDeclaration* stmt);
    void formatAssignmentStatement(const ast::AssignmentStatement* stmt);
    void formatExpressionStatement(const ast::ExpressionStatement* stmt);
    void formatReturnStatement(const ast::ReturnStatement* stmt);
    void formatIfStatement(const ast::IfStatement* stmt);
    void formatWhileStatement(const ast::WhileStatement* stmt);
    void formatRangeForStatement(const ast::RangeForStatement* stmt);
    void formatRepeatStatement(const ast::RepeatStatement* stmt);
    void formatBlockStatement(const ast::BlockStatement* stmt);
    void formatImportStatement(const ast::ImportStatement* stmt);
    void formatTryStatement(const ast::TryStatement* stmt);
    void formatThrowStatement(const ast::ThrowStatement* stmt);
    void formatClassStatement(const ast::ClassStatement* stmt);

    /// 현재 줄에 "{" 를 쓰고, 문장들을 한 단계 들여 쓴 뒤 "}" 를 쓴다 (마지막 줄바꿈은 쓰지 않는다)
    void formatBlockBody(const ast::BlockStatement* block);

    // 표현식 포맷팅
    void formatExpression(const ast::Expression* expr);
    void formatExpressionBody(const ast::Expression* expr);
    void formatBinaryExpression(const ast::BinaryExpression* expr);
    void formatUnaryExpression(const ast::UnaryExpression* expr);
    void formatCallExpression(const ast::CallExpression* expr);
    void formatIndexExpression(const ast::IndexExpression* expr);
    void formatArrayLiteral(const ast::ArrayLiteral* expr);
    void formatDictionaryLiteral(const ast::DictionaryLiteral* expr);
    void formatFunctionLiteral(const ast::FunctionLiteral* expr);
    void formatAsyncFunctionLiteral(const ast::AsyncFunctionLiteral* expr);
    void formatJosaExpression(const ast::JosaExpression* expr);
    void formatRangeExpression(const ast::RangeExpression* expr);
    void formatMatchExpression(const ast::MatchExpression* expr);
    void formatPattern(const ast::Pattern* pattern);
    void formatIdentifier(const ast::Identifier* expr);
    void formatIntegerLiteral(const ast::IntegerLiteral* expr);
    void formatFloatLiteral(const ast::FloatLiteral* expr);
    void formatStringLiteral(const ast::StringLiteral* expr);
    void formatInterpolatedString(const ast::InterpolatedString* expr);
    void formatBooleanLiteral(const ast::BooleanLiteral* expr);

    /// 부모의 우선순위보다 약하게 묶이는 피연산자면 괄호로 감싼다
    void formatOperand(const ast::Expression* expr, int parentPrecedence, bool rightSide);
    void writeParameters(const std::vector<std::string>& names, const std::vector<std::string>& types);
    void writeParameters(const std::vector<ast::Parameter>& params);

    // 주석·빈 줄
    void beginStatement(int line);
    void flushCommentsBefore(int line);
    int blockEndLine(const ast::Node* block) const;
    bool sourceLineBlank(int line) const;

    // 유틸리티
    void write(const std::string& text);
    void writeLine(const std::string& text = "");
    void writeIndent();
    void increaseIndent();
    void decreaseIndent();
    std::string getIndentString() const;

    /// 미지원 노드를 기록한다. 조용히 빼는 대신 호출자가 알 수 있게 한다.
    void markUnsupported(const ast::Node* node);

    FormatterOptions options_;
    std::stringstream output_;
    std::vector<std::string> unsupported_;
    int currentIndent_;

    // 원본 소스가 있을 때만 쓰는 상태
    bool hasSource_ = false;
    std::vector<std::string> sourceLines_;
    std::vector<SourceComment> comments_;
    size_t nextComment_ = 0;
    std::string pendingTrailing_;   ///< 지금 문장의 첫 줄 끝에 붙일 주석
    bool atBlockStart_ = true;      ///< 방금 "{" 를 썼거나 파일 맨 앞이면 true (빈 줄을 넣지 않는다)
};

} // namespace formatter
} // namespace kingsejong
