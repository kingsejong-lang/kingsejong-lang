/**
 * @file bindings.cpp
 * @brief KingSejong 언어 WebAssembly JavaScript 바인딩
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * Emscripten embind를 사용하여 JavaScript에서 호출 가능한 API를 제공합니다.
 */

#include <emscripten/bind.h>
#include <string>
#include <sstream>
#include <iostream>

#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Environment.h"
#include "evaluator/Builtin.h"

using namespace emscripten;
using namespace kingsejong;

/**
 * @class KingSejongInterpreter
 * @brief WebAssembly용 KingSejong 인터프리터 래퍼
 *
 * JavaScript에서 사용하기 쉽도록 인터프리터 기능을 캡슐화합니다.
 */
class KingSejongInterpreter {
public:
    /**
     * @brief 인터프리터 생성자
     *
     * 전역 환경을 초기화하고 내장 함수를 등록합니다.
     */
    KingSejongInterpreter()
        : env_(std::make_shared<evaluator::Environment>())
    {
        // 내장 함수 등록
        evaluator::Builtin::registerAllBuiltins();
    }

    /**
     * @brief 코드를 평가하고 결과를 반환
     * @param code KingSejong 소스 코드
     * @return 실행 결과 또는 에러 메시지
     *
     * 코드를 파싱하고 평가한 후 결과를 문자열로 반환합니다.
     * 에러 발생 시 "ERROR: " 접두사와 함께 에러 메시지를 반환합니다.
     */
    std::string evaluate(const std::string& code) {
        try {
            // 렉싱
            lexer::Lexer lexer(code);

            // 파싱
            parser::Parser parser(lexer);
            auto program = parser.parseProgram();

            // 파서 에러 체크
            const auto& errors = parser.errors();
            if (!errors.empty()) {
                std::ostringstream oss;
                oss << "파싱 오류:\n";
                for (const auto& error : errors) {
                    oss << error << "\n";
                }
                return "ERROR: " + oss.str();
            }

            if (!program) {
                return "ERROR: 프로그램을 파싱할 수 없습니다.";
            }

            // 평가. 출력() 은 std::cout 에 쓰므로, 브라우저에 보여 주려면 실행 중 cout 을 가로챈다.
            // (emscripten 기본값은 cout 을 console.log 로 보내 페이지에는 나타나지 않는다.)
            std::ostringstream captured;
            std::streambuf* previous = std::cout.rdbuf(captured.rdbuf());
            evaluator::Value result;
            try {
                evaluator::Evaluator evaluator(env_);
                result = evaluator.eval(program.get());
            } catch (...) {
                std::cout.rdbuf(previous);
                throw;
            }
            std::cout.rdbuf(previous);

            // 출력된 내용 뒤에 마지막 식의 값(없음 제외)을 붙여 돌려준다
            std::string text = captured.str();
            if (!result.isNull()) {
                text += result.toString();
            } else if (!text.empty() && text.back() == '\n') {
                text.pop_back();
            }
            return text;

        } catch (const std::exception& e) {
            return std::string("ERROR: ") + e.what();
        }
    }

    /**
     * @brief 환경 초기화 (변수 모두 삭제)
     *
     * 새로운 환경을 생성하고 내장 함수를 다시 등록합니다.
     */
    void reset() {
        env_ = std::make_shared<evaluator::Environment>();
        evaluator::Builtin::registerAllBuiltins();
    }

    /**
     * @brief 버전 정보 반환
     * @return KingSejong 버전 문자열
     */
    std::string version() const {
        return "KingSejong v0.3.2 (WebAssembly)";
    }

private:
    std::shared_ptr<evaluator::Environment> env_;
};

// Emscripten 바인딩
EMSCRIPTEN_BINDINGS(kingsejong) {
    class_<KingSejongInterpreter>("KingSejongInterpreter")
        .constructor<>()
        .function("evaluate", &KingSejongInterpreter::evaluate)
        .function("reset", &KingSejongInterpreter::reset)
        .function("version", &KingSejongInterpreter::version)
        ;
}
