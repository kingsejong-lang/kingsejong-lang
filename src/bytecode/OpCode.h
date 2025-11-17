#pragma once

/**
 * @file OpCode.h
 * @brief KingSejong 언어 바이트코드 명령어
 * @author KingSejong Team
 * @date 2025-11-10
 *
 * 스택 기반 가상 머신을 위한 바이트코드 명령어 정의입니다.
 */

#include <string>
#include <cstdint>

namespace kingsejong {
namespace bytecode {

/**
 * @enum OpCode
 * @brief 바이트코드 명령어
 *
 * 스택 기반 VM의 명령어 집합입니다.
 */
enum class OpCode : uint8_t {
    // ========================================
    // 상수 로드
    // ========================================
    LOAD_CONST,         ///< 상수 풀에서 상수 로드: LOAD_CONST [index]
    LOAD_TRUE,          ///< true 로드
    LOAD_FALSE,         ///< false 로드
    LOAD_NULL,          ///< null 로드

    // ========================================
    // 변수 조작
    // ========================================
    LOAD_VAR,           ///< 변수 로드: LOAD_VAR [name_index]
    STORE_VAR,          ///< 변수 저장: STORE_VAR [name_index]
    LOAD_GLOBAL,        ///< 전역 변수 로드: LOAD_GLOBAL [name_index]
    STORE_GLOBAL,       ///< 전역 변수 저장: STORE_GLOBAL [name_index]

    // ========================================
    // 산술 연산
    // ========================================
    ADD,                ///< 덧셈: pop b, pop a, push (a + b)
    SUB,                ///< 뺄셈: pop b, pop a, push (a - b)
    MUL,                ///< 곱셈: pop b, pop a, push (a * b)
    DIV,                ///< 나눗셈: pop b, pop a, push (a / b)
    MOD,                ///< 나머지: pop b, pop a, push (a % b)
    NEG,                ///< 부호 반전: pop a, push (-a)

    // ========================================
    // 비교 연산
    // ========================================
    EQ,                 ///< 같음: pop b, pop a, push (a == b)
    NE,                 ///< 다름: pop b, pop a, push (a != b)
    LT,                 ///< 작음: pop b, pop a, push (a < b)
    GT,                 ///< 큼: pop b, pop a, push (a > b)
    LE,                 ///< 작거나 같음: pop b, pop a, push (a <= b)
    GE,                 ///< 크거나 같음: pop b, pop a, push (a >= b)

    // ========================================
    // 논리 연산
    // ========================================
    AND,                ///< 논리 AND: pop b, pop a, push (a && b)
    OR,                 ///< 논리 OR: pop b, pop a, push (a || b)
    NOT,                ///< 논리 NOT: pop a, push (!a)

    // ========================================
    // 제어 흐름
    // ========================================
    JUMP,               ///< 무조건 점프: JUMP [offset]
    JUMP_IF_FALSE,      ///< 거짓이면 점프: pop a, if (!a) jump [offset]
    JUMP_IF_TRUE,       ///< 참이면 점프: pop a, if (a) jump [offset]
    LOOP,               ///< 루프 점프 (역방향): LOOP [offset]

    // ========================================
    // 함수 관련
    // ========================================
    CALL,               ///< 함수 호출: CALL [arg_count]
    RETURN,             ///< 함수 반환: pop return_value, return
    BUILD_FUNCTION,     ///< 함수 생성: BUILD_FUNCTION [param_count] [body_offset]

    // ========================================
    // 배열
    // ========================================
    BUILD_ARRAY,        ///< 배열 생성: BUILD_ARRAY [element_count]
    INDEX_GET,          ///< 배열 인덱스 접근: pop index, pop array, push array[index]
    INDEX_SET,          ///< 배열 인덱스 설정: pop value, pop index, pop array, array[index] = value
    ARRAY_APPEND,       ///< 배열에 요소 추가: pop value, pop array, array.append(value)

    // ========================================
    // 조사 표현식
    // ========================================
    JOSA_CALL,          ///< 조사 메서드 호출: JOSA_CALL [josa_type] [method_name_index]

    // ========================================
    // 스택 조작
    // ========================================
    POP,                ///< 스택 최상위 제거
    DUP,                ///< 스택 최상위 복제: pop a, push a, push a
    SWAP,               ///< 스택 최상위 두 값 교환: pop a, pop b, push a, push b

    // ========================================
    // 기타
    // ========================================
    PRINT,              ///< 값 출력: pop value, print(value)
    HALT,               ///< 실행 종료

    // ========================================
    // 범위
    // ========================================
    BUILD_RANGE,        ///< 범위 생성: BUILD_RANGE [inclusive_start] [inclusive_end]

    // ========================================
    // 모듈
    // ========================================
    IMPORT,             ///< 모듈 가져오기: IMPORT [module_name_index]

    // ========================================
    // 클래스 (Phase 7.1)
    // ========================================
    CLASS_DEF,          ///< 클래스 정의: CLASS_DEF [class_name_index] [field_count] [method_count]
    NEW_INSTANCE,       ///< 객체 생성: NEW_INSTANCE [class_name_index] [arg_count]
    LOAD_FIELD,         ///< 필드 읽기: pop object, push object.field
    STORE_FIELD,        ///< 필드 쓰기: pop value, pop object, object.field = value
    CALL_METHOD,        ///< 메서드 호출: CALL_METHOD [method_name_index] [arg_count]
    LOAD_THIS,          ///< this (자신) 로드: push this
};

/**
 * @brief OpCode를 문자열로 변환
 * @param op OpCode
 * @return 문자열 표현
 */
std::string opCodeToString(OpCode op);

/**
 * @brief OpCode의 피연산자 개수 반환
 * @param op OpCode
 * @return 피연산자 개수
 */
int opCodeOperandCount(OpCode op);

/**
 * @brief OpCode가 점프 명령인지 확인
 * @param op OpCode
 * @return 점프 명령이면 true
 */
bool isJumpOpCode(OpCode op);

} // namespace bytecode
} // namespace kingsejong
