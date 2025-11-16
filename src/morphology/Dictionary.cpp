/**
 * @file Dictionary.cpp
 * @brief 한글 형태소 사전 구현
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "Dictionary.h"

namespace kingsejong {
namespace morphology {

Dictionary::Dictionary()
{
    initializeDefaultDictionary();
}

WordType Dictionary::getWordType(const std::string& word) const
{
    // 우선순위: Builtin > Josa > Noun > Verb
    if (isBuiltinFunc(word)) return WordType::BUILTIN_FUNC;
    if (isBuiltinVar(word)) return WordType::BUILTIN_VAR;
    if (isJosa(word)) return WordType::JOSA;
    if (isNoun(word)) return WordType::NOUN;
    if (isVerb(word)) return WordType::VERB;
    return WordType::UNKNOWN;
}

void Dictionary::initializeDefaultDictionary()
{
    // ========================================
    // 조사 (Josa)
    // ========================================
    // 1글자 조사
    addJosa("을");
    addJosa("를");
    addJosa("이");
    addJosa("가");
    addJosa("은");
    addJosa("는");
    addJosa("의");
    addJosa("에");
    addJosa("로");
    addJosa("과");
    addJosa("와");
    addJosa("도");
    addJosa("만");

    // 2글자 조사
    addJosa("에서");
    addJosa("으로");
    addJosa("부터");
    addJosa("까지");
    addJosa("하고");
    addJosa("에게");
    addJosa("한테");
    addJosa("에다");
    addJosa("보다");

    // 조사처럼 동작하는 접미사 (반복/범위 키워드)
    addJosa("번");  // "3번", "n번"처럼 사용됨

    // ========================================
    // 일반 명사 (특수 케이스)
    // ========================================
    // 2글자 명사 (마지막 글자 "이"로 끝나지만 조사가 아님)
    addNoun("나이");
    addNoun("거리");
    addNoun("자리");
    addNoun("머리");
    addNoun("다리");
    addNoun("가지");
    addNoun("먼지");
    addNoun("연기");
    addNoun("놀이");
    addNoun("도시");
    addNoun("차이");  // "이" 조사와 충돌 방지

    // 프로그래밍 관련 일반 명사
    addNoun("배열");
    addNoun("함수");
    addNoun("변수");
    addNoun("조건");
    addNoun("반복");
    addNoun("타입");
    addNoun("객체");
    addNoun("클래스");
    addNoun("파일");
    addNoun("경로");
    addNoun("문자열");
    addNoun("숫자");
    addNoun("결과");
    addNoun("값");
    addNoun("인덱스");
    addNoun("크기");
    addNoun("내용");
    addNoun("이름");
    addNoun("확장자");
    addNoun("디렉토리");
    addNoun("온도");  // "도" 조사와 충돌 방지

    // 범위 키워드 (조사 분리 방지)
    addNoun("부터");
    addNoun("까지");
    addNoun("미만");
    addNoun("초과");
    addNoun("이하");
    addNoun("이상");

    // 반복 키워드
    addNoun("반복");
    addNoun("반복한다");
    addNoun("각각");

    // 제어문 키워드
    addNoun("만약");
    addNoun("아니면");
    addNoun("동안");

    // 함수 키워드
    addNoun("반환");
    addNoun("가져오기");

    // 타입 키워드 (일부는 이미 추가됨)
    addNoun("문자");
    addNoun("논리");

    // 불리언 리터럴
    addNoun("참");
    addNoun("거짓");

    // ========================================
    // Builtin 변수 (stdlib에서 제공)
    // ========================================
    addBuiltinVar("경로");
    addBuiltinVar("절대경로");
    addBuiltinVar("상대경로");
    addBuiltinVar("작업디렉토리");
    addBuiltinVar("홈디렉토리");
    addBuiltinVar("임시디렉토리");

    // ========================================
    // Builtin 함수 (stdlib에서 제공)
    // ========================================
    // 기본 함수
    addBuiltinFunc("출력");
    addBuiltinFunc("타입");
    addBuiltinFunc("길이");

    // 문자열 관련
    addBuiltinFunc("분리");
    addBuiltinFunc("찾기");
    addBuiltinFunc("바꾸기");
    addBuiltinFunc("대문자");
    addBuiltinFunc("소문자");

    // 타입 변환
    addBuiltinFunc("정수");
    addBuiltinFunc("실수");

    // 수학 함수
    addBuiltinFunc("반올림");
    addBuiltinFunc("올림");
    addBuiltinFunc("내림");
    addBuiltinFunc("절대값");
    addBuiltinFunc("제곱근");
    addBuiltinFunc("제곱");

    // 파일 I/O
    addBuiltinFunc("파일_읽기");
    addBuiltinFunc("파일_쓰기");
    addBuiltinFunc("파일_추가");
    addBuiltinFunc("파일_존재");
    addBuiltinFunc("줄별_읽기");

    // 환경변수
    addBuiltinFunc("환경변수_읽기");
    addBuiltinFunc("환경변수_쓰기");
    addBuiltinFunc("환경변수_존재하는가");
    addBuiltinFunc("환경변수_삭제");

    // 디렉토리
    addBuiltinFunc("현재_디렉토리");
    addBuiltinFunc("디렉토리_변경");
    addBuiltinFunc("디렉토리_생성");
    addBuiltinFunc("디렉토리_삭제");
    addBuiltinFunc("디렉토리_목록");
    addBuiltinFunc("디렉토리_나열");
    addBuiltinFunc("디렉토리인가");
    addBuiltinFunc("임시_디렉토리");

    // 파일 시스템
    addBuiltinFunc("파일_존재하는가");
    addBuiltinFunc("파일_삭제");
    addBuiltinFunc("파일_복사");
    addBuiltinFunc("파일_이동");
    addBuiltinFunc("파일_크기");
    addBuiltinFunc("파일인가");

    // 경로
    addBuiltinFunc("경로_결합");
    addBuiltinFunc("파일명_추출");
    addBuiltinFunc("확장자_추출");
    addBuiltinFunc("절대경로");
    addBuiltinFunc("경로_존재하는가");

    // OS 정보
    addBuiltinFunc("OS_이름");
    addBuiltinFunc("사용자_이름");
    addBuiltinFunc("호스트_이름");
    addBuiltinFunc("프로세스_종료");

    // JSON
    addBuiltinFunc("JSON_파싱");
    addBuiltinFunc("JSON_문자열화");
    addBuiltinFunc("JSON_파일_읽기");
    addBuiltinFunc("JSON_파일_쓰기");

    // 시간/날짜
    addBuiltinFunc("현재_시간");
    addBuiltinFunc("현재_날짜");
    addBuiltinFunc("시간_포맷");
    addBuiltinFunc("타임스탬프");
    addBuiltinFunc("슬립");
    addBuiltinFunc("현재_시각");
    addBuiltinFunc("현재_날짜시간");

    // 정규표현식
    addBuiltinFunc("정규표현식_일치");
    addBuiltinFunc("정규표현식_검색");
    addBuiltinFunc("정규표현식_모두_찾기");
    addBuiltinFunc("정규표현식_치환");
    addBuiltinFunc("정규표현식_분리");
    addBuiltinFunc("이메일_검증");
    addBuiltinFunc("URL_검증");
    addBuiltinFunc("전화번호_검증");
    addBuiltinFunc("정규표현식_추출");
    addBuiltinFunc("정규표현식_개수");

    // 암호화/해시
    addBuiltinFunc("Base64_인코딩");
    addBuiltinFunc("Base64_디코딩");
    addBuiltinFunc("문자열_해시");
    addBuiltinFunc("파일_해시");
    addBuiltinFunc("해시_비교");
    addBuiltinFunc("체크섬");
    addBuiltinFunc("XOR_암호화");
    addBuiltinFunc("XOR_복호화");
    addBuiltinFunc("시저_암호화");
    addBuiltinFunc("시저_복호화");
    addBuiltinFunc("랜덤_문자열");
    addBuiltinFunc("랜덤_숫자");

    // ========================================
    // 동사
    // ========================================
    addVerb("정렬하다");
    addVerb("출력하다");
    addVerb("입력하다");
    addVerb("실행하다");
    addVerb("종료하다");
    addVerb("반환하다");
}

} // namespace morphology
} // namespace kingsejong
