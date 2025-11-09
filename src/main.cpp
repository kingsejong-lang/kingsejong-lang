#include <iostream>
#include <string>
#include "evaluator/Builtin.h"

int main(int argc, char* argv[]) {
    // 내장 함수 등록
    kingsejong::evaluator::Builtin::registerAllBuiltins();

    std::cout << "KingSejong Programming Language v0.1.0\n";
    std::cout << "한글로 프로그래밍하는 자연스러운 언어\n";
    std::cout << "\n";

    if (argc > 1) {
        std::cerr << "파일 실행 기능은 아직 구현되지 않았습니다.\n";
        std::cerr << "요청된 파일: " << argv[1] << "\n";
        std::cerr << "현재는 기본 프로젝트 설정 단계입니다.\n";
        return 1;
    }

    std::cout << "환영합니다! KingSejong 언어 개발이 시작되었습니다.\n";
    std::cout << "REPL 및 인터프리터 기능은 Phase 1에서 구현될 예정입니다.\n";

    return 0;
}
