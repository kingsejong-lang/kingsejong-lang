#pragma once
/**
 * @file Chunk.h
 * @brief 바이트코드 청크: 코드, 상수 풀, 이름 표, 함수 프로토타입, AST 참조, 줄 정보
 */

#include "OpCode.h"
#include "runtime/Value.h"
#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>

namespace kingsejong {
namespace ast { class Node; class Statement; }
namespace bytecode {

/// 명령어 오프셋 → 소스 줄 (런-렝스)
struct LineInfo {
    size_t offset;
    int line;
};

/**
 * @brief 컴파일된 함수 하나의 정보
 *
 * 본문 코드는 같은 청크 안의 entry 오프셋에서 시작한다. body 는 평가기가 같은 함수를
 * AST 로 해석할 수 있게 남겨 두는 원본 노드다 (콜백·모듈 함수 호환).
 */
struct FunctionProto {
    std::vector<std::string> parameters;
    ast::Statement* body = nullptr;
    size_t entry = 0;
    bool isAsync = false;
    std::string name;
};

class Chunk {
private:
    std::vector<uint8_t> code_;
    std::vector<evaluator::Value> constants_;
    std::vector<std::string> names_;
    std::vector<FunctionProto> protos_;
    std::vector<const ast::Node*> nodes_;
    std::unordered_map<const ast::Statement*, size_t> entries_;   ///< 함수 본문 노드 → 코드 진입점
    std::vector<LineInfo> lines_;

public:
    // ---- 쓰기 ----
    void write(uint8_t byte, int line);
    void write16(uint16_t value, int line);
    void writeOpCode(OpCode op, int line);
    void patch(size_t offset, uint8_t byte);
    void patch16(size_t offset, uint16_t value);

    size_t addConstant(const evaluator::Value& value);
    size_t addName(const std::string& name);
    size_t addProto(FunctionProto proto);
    size_t addNode(const ast::Node* node);
    void registerEntry(const ast::Statement* body, size_t entry);

    // ---- 읽기 ----
    uint8_t read(size_t offset) const;
    uint16_t read16(size_t offset) const;
    const evaluator::Value& getConstant(size_t index) const;
    const std::string& getName(size_t index) const;
    const FunctionProto& getProto(size_t index) const;
    const ast::Node* getNode(size_t index) const;
    /// 함수 본문 노드가 이 청크에 컴파일되어 있으면 진입점을 돌려준다
    bool findEntry(const ast::Statement* body, size_t& entry) const;

    size_t size() const { return code_.size(); }
    size_t constantCount() const { return constants_.size(); }
    int getLine(size_t offset) const;
    const std::vector<uint8_t>& getCode() const { return code_; }

    void clear();

    // ---- 디스어셈블 ----
    std::string disassemble(const std::string& name) const;
    /// 한 명령을 문자열로 풀고 다음 오프셋을 돌려준다 (out 이 nullptr 이면 표준 출력)
    size_t disassembleInstruction(size_t offset, std::string* out = nullptr) const;
};

} // namespace bytecode
} // namespace kingsejong
