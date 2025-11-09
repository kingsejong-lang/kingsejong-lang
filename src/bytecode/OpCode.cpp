/**
 * @file OpCode.cpp
 * @brief OpCode 유틸리티 함수 구현
 * @author KingSejong Team
 * @date 2025-11-10
 */

#include "OpCode.h"
#include <stdexcept>

namespace kingsejong {
namespace bytecode {

std::string opCodeToString(OpCode op) {
    switch (op) {
        // 상수 로드
        case OpCode::LOAD_CONST:        return "LOAD_CONST";
        case OpCode::LOAD_TRUE:         return "LOAD_TRUE";
        case OpCode::LOAD_FALSE:        return "LOAD_FALSE";
        case OpCode::LOAD_NULL:         return "LOAD_NULL";

        // 변수 조작
        case OpCode::LOAD_VAR:          return "LOAD_VAR";
        case OpCode::STORE_VAR:         return "STORE_VAR";
        case OpCode::LOAD_GLOBAL:       return "LOAD_GLOBAL";
        case OpCode::STORE_GLOBAL:      return "STORE_GLOBAL";

        // 산술 연산
        case OpCode::ADD:               return "ADD";
        case OpCode::SUB:               return "SUB";
        case OpCode::MUL:               return "MUL";
        case OpCode::DIV:               return "DIV";
        case OpCode::MOD:               return "MOD";
        case OpCode::NEG:               return "NEG";

        // 비교 연산
        case OpCode::EQ:                return "EQ";
        case OpCode::NE:                return "NE";
        case OpCode::LT:                return "LT";
        case OpCode::GT:                return "GT";
        case OpCode::LE:                return "LE";
        case OpCode::GE:                return "GE";

        // 논리 연산
        case OpCode::AND:               return "AND";
        case OpCode::OR:                return "OR";
        case OpCode::NOT:               return "NOT";

        // 제어 흐름
        case OpCode::JUMP:              return "JUMP";
        case OpCode::JUMP_IF_FALSE:     return "JUMP_IF_FALSE";
        case OpCode::JUMP_IF_TRUE:      return "JUMP_IF_TRUE";
        case OpCode::LOOP:              return "LOOP";

        // 함수
        case OpCode::CALL:              return "CALL";
        case OpCode::RETURN:            return "RETURN";
        case OpCode::BUILD_FUNCTION:    return "BUILD_FUNCTION";

        // 배열
        case OpCode::BUILD_ARRAY:       return "BUILD_ARRAY";
        case OpCode::INDEX_GET:         return "INDEX_GET";
        case OpCode::INDEX_SET:         return "INDEX_SET";
        case OpCode::ARRAY_APPEND:      return "ARRAY_APPEND";

        // 조사
        case OpCode::JOSA_CALL:         return "JOSA_CALL";

        // 스택 조작
        case OpCode::POP:               return "POP";
        case OpCode::DUP:               return "DUP";
        case OpCode::SWAP:              return "SWAP";

        // 기타
        case OpCode::PRINT:             return "PRINT";
        case OpCode::HALT:              return "HALT";

        // 범위
        case OpCode::BUILD_RANGE:       return "BUILD_RANGE";

        // 모듈
        case OpCode::IMPORT:            return "IMPORT";

        default:
            return "UNKNOWN";
    }
}

int opCodeOperandCount(OpCode op) {
    switch (op) {
        // 피연산자 0개
        case OpCode::LOAD_TRUE:
        case OpCode::LOAD_FALSE:
        case OpCode::LOAD_NULL:
        case OpCode::ADD:
        case OpCode::SUB:
        case OpCode::MUL:
        case OpCode::DIV:
        case OpCode::MOD:
        case OpCode::NEG:
        case OpCode::EQ:
        case OpCode::NE:
        case OpCode::LT:
        case OpCode::GT:
        case OpCode::LE:
        case OpCode::GE:
        case OpCode::AND:
        case OpCode::OR:
        case OpCode::NOT:
        case OpCode::RETURN:
        case OpCode::INDEX_GET:
        case OpCode::INDEX_SET:
        case OpCode::ARRAY_APPEND:
        case OpCode::POP:
        case OpCode::DUP:
        case OpCode::SWAP:
        case OpCode::PRINT:
        case OpCode::HALT:
            return 0;

        // 피연산자 1개
        case OpCode::LOAD_CONST:
        case OpCode::LOAD_VAR:
        case OpCode::STORE_VAR:
        case OpCode::LOAD_GLOBAL:
        case OpCode::STORE_GLOBAL:
        case OpCode::JUMP:
        case OpCode::JUMP_IF_FALSE:
        case OpCode::JUMP_IF_TRUE:
        case OpCode::LOOP:
        case OpCode::CALL:
        case OpCode::BUILD_ARRAY:
        case OpCode::IMPORT:
            return 1;

        // 피연산자 2개
        case OpCode::BUILD_FUNCTION:
        case OpCode::JOSA_CALL:
        case OpCode::BUILD_RANGE:
            return 2;

        default:
            return 0;
    }
}

bool isJumpOpCode(OpCode op) {
    return op == OpCode::JUMP ||
           op == OpCode::JUMP_IF_FALSE ||
           op == OpCode::JUMP_IF_TRUE ||
           op == OpCode::LOOP;
}

} // namespace bytecode
} // namespace kingsejong
