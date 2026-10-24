#pragma once
/**
 * @file OpCode.h
 * @brief KingSejong 언어 바이트코드 명령어
 *
 * 스택 기반 가상 머신의 명령어 집합. 피연산자는 명령어 뒤에 바이트로 이어지며
 * 폭은 opCodeOperandWidths() 가 말해 준다 (1 = u8, 2 = u16, 빅엔디언).
 *
 * 변수는 평가기와 같은 Environment 체인에 이름으로 저장하고, 함수는 평가기와 같은
 * Function 객체(클로저 환경 포함)다. 그래서 VM 이 만든 함수를 평가기가 부를 수도,
 * 평가기가 만든 함수(모듈·콜백)를 VM 이 부를 수도 있다.
 */

#include <string>
#include <vector>
#include <cstdint>

namespace kingsejong {
namespace bytecode {

/// 피연산자에 "없음" 을 나타내는 u16 값 (TRY_PUSH 의 catch/finally 오프셋)
constexpr uint16_t NO_OFFSET = 0xFFFF;

/**
 * @enum OpCode
 * @brief 바이트코드 명령어
 */
enum class OpCode : uint8_t {
    // ---- 상수 ----
    LOAD_CONST,      ///< [u16 상수] 상수 풀의 값을 푸시
    LOAD_TRUE,       ///< 참 푸시
    LOAD_FALSE,      ///< 거짓 푸시
    LOAD_NULL,       ///< null 푸시

    // ---- 이름 (환경 체인) ----
    DECLARE,         ///< [u16 이름] pop 값 → 현재 환경에 정의 (변수 선언·매개변수)
    LOAD_NAME,       ///< [u16 이름] 환경 체인 → 내장 함수 순으로 찾아 푸시 ("없음" 은 null)
    STORE_NAME,      ///< [u16 이름] 스택 맨 위 값을 바깥 스코프까지 찾아 대입 (값은 남긴다)
    PUSH_SCOPE,      ///< 현재 환경을 감싸는 새 환경을 만든다 (catch 변수용)
    POP_SCOPE,       ///< 바깥 환경으로 돌아간다

    // ---- 연산 (평가기와 같은 규칙) ----
    ADD, SUB, MUL, DIV, MOD, NEG,
    EQ, NE, LT, GT, LE, GE,
    AND, OR, NOT,

    // ---- 제어 흐름 ----
    JUMP,            ///< [u16 앞으로]
    JUMP_IF_FALSE,   ///< [u16 앞으로] pop 조건, 거짓이면 점프
    JUMP_IF_TRUE,    ///< [u16 앞으로] pop 조건, 참이면 점프
    LOOP,            ///< [u16 뒤로]

    // ---- 반복문 보조 ----
    REPEAT_INIT,     ///< pop 횟수 → 정수·0 이상 검사 → 푸시
    REPEAT_NEXT,     ///< [u16 탈출] 맨 위 남은 횟수가 0 보다 크면 1 빼고 계속, 아니면 pop 후 점프
    RANGE_FOR_INIT,  ///< pop 끝, pop 시작 → 정수 검사 → 끝, 시작(=i) 푸시
    RANGE_FOR_NEXT,  ///< [u16 변수] [u16 탈출] [u8 끝포함] i 가 범위 안이면 변수에 넣고 i+1, 아니면 둘 pop 후 점프

    // ---- 함수 ----
    MAKE_FUNCTION,   ///< [u16 프로토] 현재 환경을 클로저로 잡은 Function 푸시
    CALL,            ///< [u8 인자수] 스택: 피호출자, 인자... → 결과
    CALL_METHOD,     ///< [u16 이름] [u8 인자수] 스택: 객체, 인자... → 결과 (클래스 인스턴스·Promise)
    RETURN,          ///< pop 결과 → 프레임 복귀 (비동기 함수는 Promise 로, 생성자는 인스턴스로)

    // ---- 컨테이너 ----
    BUILD_ARRAY,     ///< [u16 개수]
    BUILD_DICT,      ///< [u16 쌍 개수] 스택: 키, 값, 키, 값, ...
    BUILD_STRING,    ///< [u16 개수] 문자열 보간: 값들을 글자로 바꿔 이어 붙인다
    INDEX_GET,       ///< pop 인덱스, pop 컨테이너 → 값
    INDEX_SET,       ///< pop 키, pop 컨테이너, pop 값 → 대입, 값 푸시
    SLICE,           ///< [u8 플래그] pop (step), 끝, 시작, 배열 → 부분 배열. 비트0 시작포함, 비트1 끝포함, 비트2 step 있음

    // ---- 멤버·조사 ----
    LOAD_FIELD,      ///< [u16 이름] pop 객체 → 필드
    STORE_FIELD,     ///< [u16 이름] pop 객체, pop 값 → 대입, 값 푸시
    JOSA,            ///< [u16 이름] [u8 조사] [u8 인자수] [u8 호출형] pop 인자..., pop 객체 → 결과
    CONVERT,         ///< [u16 이름] "값을 이름으로 변환한다": pop 객체 → 이름 함수에 넘긴 결과

    // ---- 클래스·모듈 ----
    CLASS_DEF,       ///< [u16 노드] 클래스 문장 노드로 클래스를 만들어 현재 환경에 등록
    IMPORT,          ///< [u16 경로] [u16 파일] 모듈을 불러 바인딩을 현재 환경에 복사

    // ---- 예외 ----
    TRY_PUSH,        ///< [u16 catch] [u16 finally] 핸들러 등록 (오프셋은 명령 끝 기준, NO_OFFSET 은 없음)
    TRY_POP,         ///< 가장 안쪽 핸들러 해제
    THROW,           ///< pop 값 → 에러 값으로 바꿔 던진다
    END_FINALLY,     ///< finally 블록 끝: 보류된 반환·예외가 있으면 이어 간다

    // ---- 비동기 ----
    AWAIT,           ///< pop 값: 완료된 Promise 면 그 값, 아니면 그대로
    BUILD_PROMISE,   ///< 새 Promise 푸시
    PROMISE_RESOLVE, ///< pop 값, pop Promise → 이행
    PROMISE_REJECT,  ///< pop 이유, pop Promise → 거부

    // ---- 기타 ----
    EVAL_NODE,       ///< [u16 노드] AST 표현식을 평가기로 계산해 푸시 (패턴 매칭)
    POP,
    DUP,
    PRINT,           ///< pop 값 → 로그 출력 (디버깅)
    HALT
};

/// OpCode 이름
std::string opCodeToString(OpCode op);

/// 피연산자 각각의 바이트 폭 (1 또는 2)
const std::vector<uint8_t>& opCodeOperandWidths(OpCode op);

/// 피연산자 개수
int opCodeOperandCount(OpCode op);

/// 피연산자 전체 바이트 수
size_t opCodeOperandBytes(OpCode op);

/// 점프 명령인지
bool isJumpOpCode(OpCode op);

} // namespace bytecode
} // namespace kingsejong
