#pragma once

/**
 * @file Chunk.h
 * @brief 바이트코드 청크
 * @author KingSejong Team
 * @date 2025-11-10
 *
 * 바이트코드 명령어와 상수 풀을 관리하는 청크입니다.
 */

#include "OpCode.h"
#include "evaluator/Value.h"
#include <vector>
#include <string>
#include <cstdint>

namespace kingsejong {
namespace bytecode {

/**
 * @struct LineInfo
 * @brief 소스 코드 라인 정보
 *
 * 디버깅을 위한 라인 번호 정보입니다.
 */
struct LineInfo {
    size_t offset;      ///< 명령어 오프셋
    int line;           ///< 소스 코드 라인 번호
};

/**
 * @class Chunk
 * @brief 바이트코드 청크
 *
 * 바이트코드 명령어 시퀀스와 상수 풀을 관리합니다.
 */
class Chunk {
private:
    std::vector<uint8_t> code_;                     ///< 바이트코드 배열
    std::vector<evaluator::Value> constants_;       ///< 상수 풀
    std::vector<std::string> names_;                ///< 이름 테이블 (변수명, 함수명 등)
    std::vector<LineInfo> lines_;                   ///< 라인 정보

public:
    /**
     * @brief 명령어 작성
     * @param byte 바이트 (OpCode 또는 피연산자)
     * @param line 소스 코드 라인 번호
     */
    void write(uint8_t byte, int line);

    /**
     * @brief OpCode 작성
     * @param op OpCode
     * @param line 소스 코드 라인 번호
     */
    void writeOpCode(OpCode op, int line);

    /**
     * @brief 상수 추가
     * @param value 상수 값
     * @return 상수 풀 인덱스
     */
    size_t addConstant(const evaluator::Value& value);

    /**
     * @brief 이름 추가
     * @param name 이름 (변수명, 함수명 등)
     * @return 이름 테이블 인덱스
     */
    size_t addName(const std::string& name);

    /**
     * @brief 바이트코드 읽기
     * @param offset 오프셋
     * @return 바이트
     */
    uint8_t read(size_t offset) const;

    /**
     * @brief 상수 가져오기
     * @param index 상수 풀 인덱스
     * @return 상수 값
     */
    const evaluator::Value& getConstant(size_t index) const;

    /**
     * @brief 이름 가져오기
     * @param index 이름 테이블 인덱스
     * @return 이름
     */
    const std::string& getName(size_t index) const;

    /**
     * @brief 현재 오프셋 반환
     * @return 바이트코드 크기
     */
    size_t size() const { return code_.size(); }

    /**
     * @brief 라인 번호 가져오기
     * @param offset 명령어 오프셋
     * @return 라인 번호
     */
    int getLine(size_t offset) const;

    /**
     * @brief 바이트코드 디스어셈블 (디버깅용)
     * @param name 청크 이름
     * @return 디스어셈블된 문자열
     */
    std::string disassemble(const std::string& name) const;

    /**
     * @brief 명령어 디스어셈블
     * @param offset 명령어 오프셋
     * @return 다음 명령어 오프셋
     */
    size_t disassembleInstruction(size_t offset) const;

    /**
     * @brief 바이트코드 배열 직접 접근 (VM용)
     * @return 바이트코드 배열
     */
    const std::vector<uint8_t>& getCode() const { return code_; }

    /**
     * @brief 청크 초기화
     */
    void clear();

    /**
     * @brief 특정 위치의 바이트 수정
     * @param offset 오프셋
     * @param byte 새 바이트 값
     */
    void patch(size_t offset, uint8_t byte);

    /**
     * @brief 16비트 피연산자 작성
     * @param value 16비트 값
     * @param line 라인 번호
     */
    void write16(uint16_t value, int line);

    /**
     * @brief 16비트 피연산자 읽기
     * @param offset 오프셋
     * @return 16비트 값
     */
    uint16_t read16(size_t offset) const;
};

} // namespace bytecode
} // namespace kingsejong
