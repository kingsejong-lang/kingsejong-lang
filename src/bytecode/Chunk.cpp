/**
 * @file Chunk.cpp
 * @brief Chunk 구현
 * @author KingSejong Team
 * @date 2025-11-10
 */

#include "Chunk.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <iostream>
#include "error/ErrorMessages.h"

namespace kingsejong {
namespace bytecode {

void Chunk::write(uint8_t byte, int line) {
    code_.push_back(byte);

    // 라인 정보 추가 (런-렝스 인코딩)
    if (lines_.empty() || lines_.back().line != line) {
        lines_.push_back({code_.size() - 1, line});
    }
}

void Chunk::writeOpCode(OpCode op, int line) {
    write(static_cast<uint8_t>(op), line);
}

size_t Chunk::addConstant(const evaluator::Value& value) {
    constants_.push_back(value);
    return constants_.size() - 1;
}

size_t Chunk::addName(const std::string& name) {
    // 중복 확인
    for (size_t i = 0; i < names_.size(); i++) {
        if (names_[i] == name) {
            return i;
        }
    }

    names_.push_back(name);
    return names_.size() - 1;
}

uint8_t Chunk::read(size_t offset) const {
    if (offset >= code_.size()) {
        throw std::runtime_error(std::string(error::chunk::READ_OFFSET_OUT_OF_BOUNDS));
    }
    return code_[offset];
}

const evaluator::Value& Chunk::getConstant(size_t index) const {
    if (index >= constants_.size()) {
        throw std::runtime_error(std::string(error::chunk::CONSTANT_INDEX_OUT_OF_BOUNDS));
    }
    return constants_[index];
}

const std::string& Chunk::getName(size_t index) const {
    if (index >= names_.size()) {
        throw std::runtime_error(std::string(error::chunk::NAME_INDEX_OUT_OF_BOUNDS));
    }
    return names_[index];
}

int Chunk::getLine(size_t offset) const {
    // 이진 탐색으로 라인 번호 찾기
    for (auto it = lines_.rbegin(); it != lines_.rend(); ++it) {
        if (offset >= it->offset) {
            return it->line;
        }
    }
    return 1; // 기본값
}

void Chunk::clear() {
    code_.clear();
    constants_.clear();
    names_.clear();
    lines_.clear();
}

void Chunk::patch(size_t offset, uint8_t byte) {
    if (offset >= code_.size()) {
        throw std::runtime_error(std::string(error::chunk::PATCH_OFFSET_OUT_OF_BOUNDS));
    }
    code_[offset] = byte;
}

void Chunk::write16(uint16_t value, int line) {
    write(static_cast<uint8_t>((value >> 8) & 0xFF), line);  // 상위 바이트
    write(static_cast<uint8_t>(value & 0xFF), line);         // 하위 바이트
}

uint16_t Chunk::read16(size_t offset) const {
    uint8_t high = read(offset);
    uint8_t low = read(offset + 1);
    return (static_cast<uint16_t>(high) << 8) | low;
}

std::string Chunk::disassemble(const std::string& name) const {
    std::ostringstream oss;
    oss << "== " << name << " ==\n";

    size_t offset = 0;
    while (offset < code_.size()) {
        offset = disassembleInstruction(offset);
    }

    return oss.str();
}

size_t Chunk::disassembleInstruction(size_t offset) const {
    std::ostringstream oss;

    // 오프셋 출력
    oss << std::setfill('0') << std::setw(4) << offset << " ";

    // 라인 번호 출력
    int line = getLine(offset);
    if (offset > 0 && line == getLine(offset - 1)) {
        oss << "   | ";
    } else {
        oss << std::setw(4) << line << " ";
    }

    // OpCode 읽기
    OpCode op = static_cast<OpCode>(read(offset));
    std::string opName = opCodeToString(op);

    int operandCount = opCodeOperandCount(op);

    if (operandCount == 0) {
        // 피연산자 없음
        oss << opName << "\n";
        std::cout << oss.str();
        return offset + 1;
    } else if (operandCount == 1) {
        // 피연산자 1개
        uint8_t operand = read(offset + 1);
        oss << std::setw(16) << std::left << opName << " " << static_cast<int>(operand);

        // 상수나 이름이면 추가 정보 출력
        if (op == OpCode::LOAD_CONST && operand < constants_.size()) {
            oss << " ('" << constants_[operand].toString() << "')";
        } else if ((op == OpCode::LOAD_VAR || op == OpCode::STORE_VAR ||
                    op == OpCode::LOAD_GLOBAL || op == OpCode::STORE_GLOBAL) &&
                   operand < names_.size()) {
            oss << " ('" << names_[operand] << "')";
        }

        oss << "\n";
        std::cout << oss.str();
        return offset + 2;
    } else if (operandCount == 2) {
        // 피연산자 2개
        uint8_t operand1 = read(offset + 1);
        uint8_t operand2 = read(offset + 2);
        oss << std::setw(16) << std::left << opName
            << " " << static_cast<int>(operand1)
            << " " << static_cast<int>(operand2) << "\n";
        std::cout << oss.str();
        return offset + 3;
    }

    // 알 수 없는 OpCode
    oss << "Unknown opcode " << static_cast<int>(op) << "\n";
    std::cout << oss.str();
    return offset + 1;
}

} // namespace bytecode
} // namespace kingsejong
