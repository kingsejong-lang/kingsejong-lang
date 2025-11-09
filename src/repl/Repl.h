#pragma once

/**
 * @file Repl.h
 * @brief KingSejong 대화형 실행 환경 (REPL)
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include "../lexer/Lexer.h"
#include "../parser/Parser.h"
#include "../evaluator/Evaluator.h"
#include "../evaluator/Environment.h"
#include <string>
#include <memory>

namespace kingsejong {
namespace repl {

/**
 * @class Repl
 * @brief KingSejong 대화형 실행 환경
 *
 * Read-Eval-Print Loop를 구현하여 사용자가 대화형으로
 * KingSejong 코드를 실행할 수 있도록 합니다.
 */
class Repl
{
public:
    /**
     * @brief REPL 시작
     *
     * 환영 메시지를 출력하고 무한 루프로 진입하여
     * 사용자 입력을 받고 평가 및 출력합니다.
     */
    void start();

private:
    /// 전역 환경 (세션 동안 유지)
    std::shared_ptr<evaluator::Environment> env_;

    /**
     * @brief 프롬프트 표시
     * @param continuation 여러 줄 입력 중이면 true (... 표시)
     */
    void printPrompt(bool continuation = false);

    /**
     * @brief 입력 읽기 (여러 줄 지원)
     *
     * 괄호 매칭을 확인하여 완전한 입력이 될 때까지
     * 여러 줄을 읽습니다.
     *
     * @return 완전한 입력 문자열, EOF시 빈 문자열
     */
    std::string readInput();

    /**
     * @brief 입력이 완전한지 확인 (괄호 매칭)
     *
     * 모든 괄호({}, (), [])가 매칭되는지 확인합니다.
     *
     * @param input 입력 문자열
     * @return 완전하면 true
     */
    bool isComplete(const std::string& input);

    /**
     * @brief 입력 평가 및 출력
     *
     * 입력을 파싱하고 평가한 후 결과를 출력합니다.
     * 에러 발생 시 에러 메시지를 출력합니다.
     *
     * @param input 입력 문자열
     */
    void evalAndPrint(const std::string& input);

    /**
     * @brief 특수 명령 처리
     *
     * .exit, .help, .clear, .vars 등의 특수 명령을
     * 처리합니다.
     *
     * @param input 입력 문자열
     * @return 특수 명령이면 true
     */
    bool handleCommand(const std::string& input);

    /**
     * @brief 도움말 출력
     */
    void printHelp();

    /**
     * @brief 환영 메시지 출력
     */
    void printWelcome();

    /**
     * @brief 변수 목록 출력
     *
     * 현재 환경에 정의된 모든 변수를 출력합니다.
     */
    void printVariables();
};

} // namespace repl
} // namespace kingsejong
