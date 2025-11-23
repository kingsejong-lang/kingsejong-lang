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
        case OpCode::TAIL_CALL:         return "TAIL_CALL";
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

        // 클래스 (Phase 7.1)
        case OpCode::CLASS_DEF:         return "CLASS_DEF";
        case OpCode::NEW_INSTANCE:      return "NEW_INSTANCE";
        case OpCode::LOAD_FIELD:        return "LOAD_FIELD";
        case OpCode::STORE_FIELD:       return "STORE_FIELD";
        case OpCode::CALL_METHOD:       return "CALL_METHOD";
        case OpCode::LOAD_THIS:         return "LOAD_THIS";

        // 비동기 (Phase 7.3)
        case OpCode::BUILD_ASYNC_FUNC:  return "BUILD_ASYNC_FUNC";
        case OpCode::ASYNC_CALL:        return "ASYNC_CALL";
        case OpCode::AWAIT:             return "AWAIT";
        case OpCode::PROMISE_RESOLVE:   return "PROMISE_RESOLVE";
        case OpCode::PROMISE_REJECT:    return "PROMISE_REJECT";
        case OpCode::BUILD_PROMISE:     return "BUILD_PROMISE";
        case OpCode::PROMISE_THEN:      return "PROMISE_THEN";
        case OpCode::PROMISE_CATCH:     return "PROMISE_CATCH";

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
        case OpCode::LOAD_THIS:           // 클래스: this 로드
        case OpCode::AWAIT:               // 비동기: await (pop promise, push value)
        case OpCode::PROMISE_RESOLVE:     // 비동기: resolve
        case OpCode::PROMISE_REJECT:      // 비동기: reject
        case OpCode::BUILD_PROMISE:       // 비동기: 새 promise 생성
        case OpCode::PROMISE_THEN:        // 비동기: then 콜백 등록
        case OpCode::PROMISE_CATCH:       // 비동기: catch 콜백 등록
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
        case OpCode::TAIL_CALL:
        case OpCode::BUILD_ARRAY:
        case OpCode::IMPORT:
        case OpCode::LOAD_FIELD:          // 클래스: 필드 읽기 [field_name_index]
        case OpCode::STORE_FIELD:         // 클래스: 필드 쓰기 [field_name_index]
        case OpCode::ASYNC_CALL:          // 비동기: async 함수 호출 [arg_count]
            return 1;

        // 피연산자 2개
        case OpCode::BUILD_FUNCTION:
        case OpCode::JOSA_CALL:
        case OpCode::BUILD_RANGE:
        case OpCode::NEW_INSTANCE:        // 클래스: 객체 생성 [class_name_index] [arg_count]
        case OpCode::CALL_METHOD:         // 클래스: 메서드 호출 [method_name_index] [arg_count]
        case OpCode::BUILD_ASYNC_FUNC:    // 비동기: 비동기 함수 생성 [param_count] [body_offset]
            return 2;

        // 피연산자 3개
        case OpCode::CLASS_DEF:           // 클래스: 클래스 정의 [class_name_index] [field_count] [method_count]
            return 3;

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
