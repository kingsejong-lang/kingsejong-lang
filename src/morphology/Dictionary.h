#pragma once

/**
 * @file Dictionary.h
 * @brief 한글 형태소 사전
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include <string>
#include <unordered_set>
#include <vector>

namespace kingsejong {
namespace morphology {

/**
 * @enum WordType
 * @brief 단어의 품사/타입
 */
enum class WordType
{
    NOUN,           ///< 명사 (예: "배열", "함수", "나이")
    VERB,           ///< 동사 (예: "정렬하다", "출력하다")
    JOSA,           ///< 조사 (예: "을", "를", "이", "가")
    BUILTIN_VAR,    ///< Builtin 변수 (예: "경로", "절대경로")
    BUILTIN_FUNC,   ///< Builtin 함수 (예: "디렉토리인가", "파일인가")
    UNKNOWN         ///< 미등록 단어
};

/**
 * @class Dictionary
 * @brief 한글 형태소 사전
 *
 * 명사, 동사, 조사, builtin 함수/변수를 관리합니다.
 * 사전 기반 형태소 분석의 기초가 됩니다.
 */
class Dictionary
{
private:
    std::unordered_set<std::string> nouns_;          ///< 명사 사전
    std::unordered_set<std::string> verbs_;          ///< 동사 사전
    std::unordered_set<std::string> josas_;          ///< 조사 사전
    std::unordered_set<std::string> builtinVars_;    ///< Builtin 변수
    std::unordered_set<std::string> builtinFuncs_;   ///< Builtin 함수

public:
    /**
     * @brief 생성자 - 기본 사전 초기화
     */
    Dictionary();

    /**
     * @brief 단어의 타입 확인
     * @param word 확인할 단어
     * @return 단어 타입
     */
    WordType getWordType(const std::string& word) const;

    /**
     * @brief 명사 여부 확인
     */
    bool isNoun(const std::string& word) const
    {
        return nouns_.count(word) > 0;
    }

    /**
     * @brief 동사 여부 확인
     */
    bool isVerb(const std::string& word) const
    {
        return verbs_.count(word) > 0;
    }

    /**
     * @brief 조사 여부 확인
     */
    bool isJosa(const std::string& word) const
    {
        return josas_.count(word) > 0;
    }

    /**
     * @brief Builtin 변수 여부 확인
     */
    bool isBuiltinVar(const std::string& word) const
    {
        return builtinVars_.count(word) > 0;
    }

    /**
     * @brief Builtin 함수 여부 확인
     */
    bool isBuiltinFunc(const std::string& word) const
    {
        return builtinFuncs_.count(word) > 0;
    }

    /**
     * @brief 명사 추가
     */
    void addNoun(const std::string& word)
    {
        nouns_.insert(word);
    }

    /**
     * @brief 동사 추가
     */
    void addVerb(const std::string& word)
    {
        verbs_.insert(word);
    }

    /**
     * @brief 조사 추가
     */
    void addJosa(const std::string& word)
    {
        josas_.insert(word);
    }

    /**
     * @brief Builtin 변수 추가
     */
    void addBuiltinVar(const std::string& word)
    {
        builtinVars_.insert(word);
    }

    /**
     * @brief Builtin 함수 추가
     */
    void addBuiltinFunc(const std::string& word)
    {
        builtinFuncs_.insert(word);
    }

private:
    /**
     * @brief 기본 사전 초기화
     */
    void initializeDefaultDictionary();
};

} // namespace morphology
} // namespace kingsejong
