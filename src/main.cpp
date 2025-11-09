/**
 * @file main.cpp
 * @brief KingSejong 언어 실행 파일
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include <iostream>
#include <string>
#include "repl/Repl.h"
#include "evaluator/Builtin.h"

/**
 * @brief 프로그램 진입점
 * @param argc 인자 개수
 * @param argv 인자 배열
 * @return 종료 코드 (0: 성공, 1: 실패)
 */
int main(int argc, char* argv[])
{
    // 내장 함수 등록
    kingsejong::evaluator::Builtin::registerAllBuiltins();

    if (argc == 1)
    {
        // REPL 모드
        kingsejong::repl::Repl repl;
        repl.start();
    }
    else if (argc == 2)
    {
        // 파일 실행 모드 (F1.18에서 구현 예정)
        std::string filename = argv[1];
        std::cerr << "파일 실행 기능은 아직 구현되지 않았습니다.\n";
        std::cerr << "요청된 파일: " << filename << "\n";
        std::cerr << "F1.18에서 구현될 예정입니다.\n";
        return 1;
    }
    else
    {
        // 잘못된 사용법
        std::cerr << "사용법: kingsejong [파일명]\n";
        std::cerr << "\n";
        std::cerr << "  kingsejong          - REPL 모드로 실행\n";
        std::cerr << "  kingsejong 파일.ksj  - 파일 실행 (구현 예정)\n";
        return 1;
    }

    return 0;
}
