#pragma once

#include <string_view>

namespace kingsejong {
namespace error {

namespace vm {
    constexpr std::string_view MAX_INSTRUCTION_LIMIT = "최대 명령어 실행 횟수 초과 ({} 초과). 무한 루프 의심.";
    constexpr std::string_view MAX_EXECUTION_TIME = "최대 실행 시간 초과 ({}ms 초과). 무한 루프 또는 긴 연산 의심.";
    constexpr std::string_view STACK_OVERFLOW = "스택 오버플로우";
    constexpr std::string_view LOCAL_VAR_INDEX_OUT_OF_BOUNDS = "로컬 변수 인덱스 범위 초과";
    constexpr std::string_view UNDEFINED_VARIABLE = "정의되지 않은 변수: {}";
    constexpr std::string_view OPERAND_MUST_BE_NUMBER = "피연산자는 숫자여야 합니다";
    constexpr std::string_view OPERAND_MUST_BE_NUMBER_OR_STRING = "피연산자는 숫자 또는 문자열이어야 합니다";
    constexpr std::string_view NOT_AN_ARRAY = "배열이 아닙니다";
    constexpr std::string_view INDEX_MUST_BE_INTEGER = "인덱스는 정수여야 합니다";
    constexpr std::string_view INDEX_OUT_OF_BOUNDS = "인덱스 범위 초과";
    constexpr std::string_view CALL_NON_FUNCTION = "함수가 아닌 값을 호출하려고 했습니다";
    constexpr std::string_view UNKNOWN_ARRAY_METHOD = "알 수 없는 배열 메서드: {}";
    constexpr std::string_view UNSUPPORTED_JOSA_TYPE = "조사 표현식: 지원되지 않는 타입";
    constexpr std::string_view UNDEFINED_CLASS = "정의되지 않은 클래스: {}";
    constexpr std::string_view CONSTRUCTOR_ARG_MISMATCH = "생성자 인자 개수 불일치: 예상 {}, 실제 {}";
    constexpr std::string_view CONSTRUCTOR_EXECUTION_ERROR = "생성자 실행 오류: {}";
    constexpr std::string_view FIELD_ACCESS_NOT_INSTANCE = "필드 접근: 클래스 인스턴스가 아닙니다";
    constexpr std::string_view FIELD_ACCESS_ERROR = "필드 접근 오류: {}";
    constexpr std::string_view FIELD_SET_NOT_INSTANCE = "필드 설정: 클래스 인스턴스가 아닙니다";
    constexpr std::string_view FIELD_SET_ERROR = "필드 설정 오류: {}";
    constexpr std::string_view METHOD_CALL_NOT_INSTANCE = "메서드 호출: 클래스 인스턴스가 아닙니다";
    constexpr std::string_view UNDEFINED_METHOD = "정의되지 않은 메서드: {}";
    constexpr std::string_view METHOD_ARG_MISMATCH = "메서드 인자 개수 불일치: 예상 {}, 실제 {}";
    constexpr std::string_view METHOD_EXECUTION_ERROR = "메서드 실행 오류: {}";
    constexpr std::string_view THIS_OUTSIDE_CLASS = "메서드 또는 생성자 외부에서 'this'를 사용할 수 없습니다";
    constexpr std::string_view CALL_NON_ASYNC = "비동기 함수가 아닌 값을 호출하려고 했습니다";
    constexpr std::string_view PROMISE_REJECTED = "Promise가 거부되었습니다: {}";
    constexpr std::string_view PROMISE_THEN_CATCH_UNSUPPORTED = "Promise then/catch는 VM에서 아직 완전히 지원되지 않습니다";
    constexpr std::string_view UNIMPLEMENTED_OPCODE = "아직 구현되지 않은 명령어: {}";
    constexpr std::string_view UNKNOWN_OPCODE = "알 수 없는 명령어";
    constexpr std::string_view DIVIDE_BY_ZERO = "0으로 나눌 수 없습니다";
    constexpr std::string_view UNKNOWN_BINARY_OP = "알 수 없는 이항 연산자";
    constexpr std::string_view JIT_EXECUTION_EXCEPTION = "JIT 코드 실행 중 예외: {}";
    constexpr std::string_view STACK_SIZE_EXCEEDED = "최대 스택 크기 초과 ({} 초과). 재귀 호출 또는 메모리 폭주 의심.";
    constexpr std::string_view STACK_UNDERFLOW = "Stack underflow";
    constexpr std::string_view STACK_PEEK_OUT_OF_RANGE = "Stack peek out of range";
}

namespace chunk {
    constexpr std::string_view READ_OFFSET_OUT_OF_BOUNDS = "Chunk::read: offset out of bounds";
    constexpr std::string_view CONSTANT_INDEX_OUT_OF_BOUNDS = "Chunk::getConstant: index out of bounds";
    constexpr std::string_view NAME_INDEX_OUT_OF_BOUNDS = "Chunk::getName: index out of bounds";
    constexpr std::string_view PATCH_OFFSET_OUT_OF_BOUNDS = "Chunk::patch: offset out of bounds";
}

namespace debugger {
    constexpr std::string_view CALLSTACK_POP_EMPTY = "CallStack::pop() - stack is empty";
    constexpr std::string_view CALLSTACK_CURRENT_EMPTY = "CallStack::current() - stack is empty";
}

} // namespace error
} // namespace kingsejong
