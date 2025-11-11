/**
 * @file CompletionProvider.cpp
 * @brief LSP 자동 완성 제공자 구현
 * @author KingSejong Team
 * @date 2025-11-12
 */

#include "lsp/CompletionProvider.h"
#include <regex>
#include <algorithm>

namespace kingsejong {
namespace lsp {

CompletionProvider::CompletionProvider()
{
    // 키워드 완성 항목 초기화
    keywordCompletions_ = {
        // 선언 키워드
        CompletionItem("변수", CompletionItemKind::Keyword, "변수 선언", "가변 변수를 선언합니다. 예: 변수 x = 10"),
        CompletionItem("상수", CompletionItemKind::Keyword, "상수 선언", "불변 상수를 선언합니다. 예: 상수 PI = 3.14"),
        CompletionItem("함수", CompletionItemKind::Keyword, "함수 선언", "함수를 선언합니다. 예: 함수 더하기(a, b) { 반환 a + b }"),
        CompletionItem("반환", CompletionItemKind::Keyword, "함수 반환", "함수에서 값을 반환합니다. 예: 반환 결과"),

        // 제어 흐름 키워드
        CompletionItem("만약", CompletionItemKind::Keyword, "조건문", "조건에 따라 분기합니다. 예: 만약 (x > 0) { ... }"),
        CompletionItem("아니면", CompletionItemKind::Keyword, "조건문 else", "만약 조건이 거짓일 때 실행합니다. 예: 아니면 { ... }"),
        CompletionItem("반복", CompletionItemKind::Keyword, "반복문", "조건이 참인 동안 반복합니다. 예: 반복 (i < 10) { ... }"),
        CompletionItem("계속", CompletionItemKind::Keyword, "반복 계속", "반복문의 다음 반복으로 건너뜁니다."),
        CompletionItem("중단", CompletionItemKind::Keyword, "반복 중단", "반복문을 즉시 종료합니다."),

        // 리터럴 키워드
        CompletionItem("참", CompletionItemKind::Keyword, "불린 참", "참(true) 값을 나타냅니다."),
        CompletionItem("거짓", CompletionItemKind::Keyword, "불린 거짓", "거짓(false) 값을 나타냅니다."),
        CompletionItem("없음", CompletionItemKind::Keyword, "null 값", "없음(null) 값을 나타냅니다."),

        // 내장 함수
        CompletionItem("출력", CompletionItemKind::Keyword, "내장 함수", "값을 출력합니다. 예: 출력(\"안녕하세요\")"),
        CompletionItem("타입", CompletionItemKind::Keyword, "내장 함수", "값의 타입을 반환합니다. 예: 타입(x)"),
        CompletionItem("길이", CompletionItemKind::Keyword, "내장 함수", "문자열이나 배열의 길이를 반환합니다. 예: 길이(\"안녕\")"),
    };
}

std::vector<CompletionProvider::CompletionItem> CompletionProvider::provideCompletions(
    const DocumentManager::Document& document,
    int line,
    int character)
{
    (void)line;      // 향후 컨텍스트 기반 완성에 사용 예정
    (void)character;

    std::vector<CompletionItem> items;

    // 1. 키워드 완성 추가
    auto keywords = getKeywordCompletions();
    items.insert(items.end(), keywords.begin(), keywords.end());

    // 2. 변수명 완성 추가
    auto variables = getVariableCompletions(document);
    items.insert(items.end(), variables.begin(), variables.end());

    // 3. 함수명 완성 추가
    auto functions = getFunctionCompletions(document);
    items.insert(items.end(), functions.begin(), functions.end());

    return items;
}

std::vector<CompletionProvider::CompletionItem> CompletionProvider::getKeywordCompletions() const
{
    return keywordCompletions_;
}

std::vector<CompletionProvider::CompletionItem> CompletionProvider::getVariableCompletions(
    const DocumentManager::Document& document) const
{
    std::vector<CompletionItem> items;

    // 변수명 추출
    auto varNames = extractVariableNames(document.content);

    // CompletionItem으로 변환
    for (const auto& name : varNames) {
        items.emplace_back(name, CompletionItemKind::Variable, "변수", "");
    }

    return items;
}

std::vector<CompletionProvider::CompletionItem> CompletionProvider::getFunctionCompletions(
    const DocumentManager::Document& document) const
{
    std::vector<CompletionItem> items;

    // 함수 시그니처 추출
    auto signatures = extractFunctionSignatures(document.content);

    // CompletionItem으로 변환
    for (const auto& sig : signatures) {
        // 파라미터 목록 문자열 생성
        std::string params = "(";
        for (size_t i = 0; i < sig.params.size(); i++) {
            if (i > 0) params += ", ";
            params += sig.params[i];
        }
        params += ")";

        std::string detail = "함수 " + sig.name + params;
        items.emplace_back(sig.name, CompletionItemKind::Function, detail, "");
    }

    return items;
}

std::set<std::string> CompletionProvider::extractVariableNames(const std::string& content) const
{
    std::set<std::string> names;

    // 정규식: "변수 name" 또는 "상수 name" 패턴
    // UTF-8 한글 지원을 위해 \w 대신 [^\s=]+ 사용
    std::regex varPattern(R"((변수|상수)\s+([^\s=]+))");

    std::sregex_iterator iter(content.begin(), content.end(), varPattern);
    std::sregex_iterator end;

    while (iter != end) {
        std::smatch match = *iter;
        if (match.size() >= 3) {
            std::string varName = match[2].str();
            names.insert(varName);
        }
        ++iter;
    }

    return names;
}

std::vector<CompletionProvider::FunctionSignature> CompletionProvider::extractFunctionSignatures(
    const std::string& content) const
{
    std::vector<FunctionSignature> signatures;

    // 정규식: "함수 name(param1, param2) {" 패턴
    std::regex funcPattern(R"(함수\s+([^\s(]+)\s*\(([^)]*)\))");

    std::sregex_iterator iter(content.begin(), content.end(), funcPattern);
    std::sregex_iterator end;

    while (iter != end) {
        std::smatch match = *iter;
        if (match.size() >= 3) {
            std::string funcName = match[1].str();
            std::string paramsStr = match[2].str();

            // 파라미터 파싱
            std::vector<std::string> params;
            if (!paramsStr.empty()) {
                std::regex paramPattern(R"([^\s,]+)");
                std::sregex_iterator paramIter(paramsStr.begin(), paramsStr.end(), paramPattern);
                std::sregex_iterator paramEnd;

                while (paramIter != paramEnd) {
                    params.push_back((*paramIter).str());
                    ++paramIter;
                }
            }

            signatures.emplace_back(funcName, params);
        }
        ++iter;
    }

    return signatures;
}

} // namespace lsp
} // namespace kingsejong
