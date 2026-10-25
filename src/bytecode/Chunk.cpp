/**
 * @file Chunk.cpp
 * @brief Chunk 구현
 */

#include "Chunk.h"
#include "error/ErrorMessages.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <iostream>

namespace kingsejong {
namespace bytecode {

void Chunk::write(uint8_t byte, int line, int column) {
    code_.push_back(byte);
    if (lines_.empty() || lines_.back().line != line || lines_.back().column != column) {
        lines_.push_back({code_.size() - 1, line, column});
    }
}

void Chunk::write16(uint16_t value, int line, int column) {
    write(static_cast<uint8_t>((value >> 8) & 0xFF), line, column);
    write(static_cast<uint8_t>(value & 0xFF), line, column);
}

void Chunk::writeOpCode(OpCode op, int line, int column) {
    write(static_cast<uint8_t>(op), line, column);
}

void Chunk::patch(size_t offset, uint8_t byte) {
    if (offset >= code_.size()) {
        throw std::runtime_error(std::string(error::chunk::PATCH_OFFSET_OUT_OF_BOUNDS));
    }
    code_[offset] = byte;
}

void Chunk::patch16(size_t offset, uint16_t value) {
    patch(offset, static_cast<uint8_t>((value >> 8) & 0xFF));
    patch(offset + 1, static_cast<uint8_t>(value & 0xFF));
}

size_t Chunk::addConstant(const evaluator::Value& value) {
    constants_.push_back(value);
    return constants_.size() - 1;
}

size_t Chunk::addName(const std::string& name) {
    for (size_t i = 0; i < names_.size(); i++) {
        if (names_[i] == name) {
            return i;
        }
    }
    names_.push_back(name);
    return names_.size() - 1;
}

size_t Chunk::addProto(FunctionProto proto) {
    protos_.push_back(std::move(proto));
    return protos_.size() - 1;
}

size_t Chunk::addNode(const ast::Node* node) {
    nodes_.push_back(node);
    return nodes_.size() - 1;
}

void Chunk::registerEntry(const ast::Statement* body, size_t entry) {
    entries_[body] = entry;
}

void Chunk::setFile(const std::string& filename) {
    if (files_.empty() || files_.back().filename != filename) {
        files_.push_back({code_.size(), filename});
    }
}

void Chunk::setCallSiteName(size_t offset, const std::string& name) {
    callSites_[offset] = static_cast<uint16_t>(addName(name));
}

uint8_t Chunk::read(size_t offset) const {
    if (offset >= code_.size()) {
        throw std::runtime_error(std::string(error::chunk::READ_OFFSET_OUT_OF_BOUNDS));
    }
    return code_[offset];
}

uint16_t Chunk::read16(size_t offset) const {
    uint8_t high = read(offset);
    uint8_t low = read(offset + 1);
    return static_cast<uint16_t>((static_cast<uint16_t>(high) << 8) | low);
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

const FunctionProto& Chunk::getProto(size_t index) const {
    if (index >= protos_.size()) {
        throw std::runtime_error("함수 프로토타입 인덱스가 범위를 벗어났습니다");
    }
    return protos_[index];
}

const ast::Node* Chunk::getNode(size_t index) const {
    if (index >= nodes_.size()) {
        throw std::runtime_error("AST 노드 인덱스가 범위를 벗어났습니다");
    }
    return nodes_[index];
}

bool Chunk::findEntry(const ast::Statement* body, size_t& entry) const {
    auto it = entries_.find(body);
    if (it == entries_.end()) {
        return false;
    }
    entry = it->second;
    return true;
}

int Chunk::getLine(size_t offset) const {
    for (auto it = lines_.rbegin(); it != lines_.rend(); ++it) {
        if (offset >= it->offset) {
            return it->line;
        }
    }
    return 1;
}

int Chunk::getColumn(size_t offset) const {
    for (auto it = lines_.rbegin(); it != lines_.rend(); ++it) {
        if (offset >= it->offset) {
            return it->column;
        }
    }
    return 0;
}

const std::string& Chunk::getFile(size_t offset) const {
    static const std::string none;
    for (auto it = files_.rbegin(); it != files_.rend(); ++it) {
        if (offset >= it->offset) {
            return it->filename;
        }
    }
    return none;
}

const std::string& Chunk::getCallSiteName(size_t offset) const {
    static const std::string none;
    auto it = callSites_.find(offset);
    if (it == callSites_.end() || it->second >= names_.size()) {
        return none;
    }
    return names_[it->second];
}

void Chunk::clear() {
    code_.clear();
    constants_.clear();
    names_.clear();
    protos_.clear();
    nodes_.clear();
    entries_.clear();
    lines_.clear();
    files_.clear();
    callSites_.clear();
}

std::string Chunk::disassemble(const std::string& name) const {
    std::string out = "== " + name + " ==\n";
    size_t offset = 0;
    while (offset < code_.size()) {
        offset = disassembleInstruction(offset, &out);
    }
    return out;
}

size_t Chunk::disassembleInstruction(size_t offset, std::string* out) const {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << offset << " ";

    int line = getLine(offset);
    if (offset > 0 && line == getLine(offset - 1)) {
        oss << "   | ";
    } else {
        oss << std::setfill(' ') << std::setw(4) << line << " ";
    }

    OpCode op = static_cast<OpCode>(read(offset));
    oss << std::setfill(' ') << std::setw(16) << std::left << opCodeToString(op) << std::right;

    size_t pos = offset + 1;
    const auto& widths = opCodeOperandWidths(op);
    std::vector<size_t> operands;
    for (uint8_t width : widths) {
        if (pos + width > code_.size()) {
            oss << " <잘림>\n";
            if (out) *out += oss.str(); else std::cout << oss.str();
            return code_.size();
        }
        size_t value = (width == 2) ? read16(pos) : read(pos);
        operands.push_back(value);
        oss << " " << value;
        pos += width;
    }

    // 사람이 읽을 보조 정보
    if (!operands.empty()) {
        size_t a = operands[0];
        switch (op) {
            case OpCode::LOAD_CONST:
                if (a < constants_.size()) oss << " ('" << constants_[a].toString() << "')";
                break;
            case OpCode::DECLARE:
            case OpCode::LOAD_NAME:
            case OpCode::STORE_NAME:
            case OpCode::LOAD_FIELD:
            case OpCode::STORE_FIELD:
            case OpCode::JOSA:
            case OpCode::CONVERT:
            case OpCode::RANGE_FOR_NEXT:
            case OpCode::IMPORT:
                if (a < names_.size()) oss << " ('" << names_[a] << "')";
                break;
            case OpCode::CALL_METHOD:
                if (a < names_.size()) oss << " ('" << names_[a] << "')";
                break;
            case OpCode::CALL: {
                const std::string& callee = getCallSiteName(offset);
                if (!callee.empty()) oss << " ('" << callee << "')";
                break;
            }
            case OpCode::MAKE_FUNCTION:
                if (a < protos_.size()) oss << " (" << protos_[a].name << " @" << protos_[a].entry << ")";
                break;
            default:
                break;
        }
    }
    oss << "\n";

    if (out) {
        *out += oss.str();
    } else {
        std::cout << oss.str();
    }
    return pos;
}

} // namespace bytecode
} // namespace kingsejong
