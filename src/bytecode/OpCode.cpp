/**
 * @file OpCode.cpp
 * @brief OpCode 이름·피연산자 표
 */

#include "OpCode.h"

namespace kingsejong {
namespace bytecode {

std::string opCodeToString(OpCode op) {
    switch (op) {
        case OpCode::LOAD_CONST:      return "LOAD_CONST";
        case OpCode::LOAD_TRUE:       return "LOAD_TRUE";
        case OpCode::LOAD_FALSE:      return "LOAD_FALSE";
        case OpCode::LOAD_NULL:       return "LOAD_NULL";
        case OpCode::DECLARE:         return "DECLARE";
        case OpCode::LOAD_NAME:       return "LOAD_NAME";
        case OpCode::STORE_NAME:      return "STORE_NAME";
        case OpCode::PUSH_SCOPE:      return "PUSH_SCOPE";
        case OpCode::POP_SCOPE:       return "POP_SCOPE";
        case OpCode::ADD:             return "ADD";
        case OpCode::SUB:             return "SUB";
        case OpCode::MUL:             return "MUL";
        case OpCode::DIV:             return "DIV";
        case OpCode::MOD:             return "MOD";
        case OpCode::NEG:             return "NEG";
        case OpCode::EQ:              return "EQ";
        case OpCode::NE:              return "NE";
        case OpCode::LT:              return "LT";
        case OpCode::GT:              return "GT";
        case OpCode::LE:              return "LE";
        case OpCode::GE:              return "GE";
        case OpCode::AND:             return "AND";
        case OpCode::OR:              return "OR";
        case OpCode::NOT:             return "NOT";
        case OpCode::JUMP:            return "JUMP";
        case OpCode::JUMP_IF_FALSE:   return "JUMP_IF_FALSE";
        case OpCode::JUMP_IF_TRUE:    return "JUMP_IF_TRUE";
        case OpCode::LOOP:            return "LOOP";
        case OpCode::REPEAT_INIT:     return "REPEAT_INIT";
        case OpCode::REPEAT_NEXT:     return "REPEAT_NEXT";
        case OpCode::RANGE_FOR_INIT:  return "RANGE_FOR_INIT";
        case OpCode::RANGE_FOR_NEXT:  return "RANGE_FOR_NEXT";
        case OpCode::MAKE_FUNCTION:   return "MAKE_FUNCTION";
        case OpCode::CALL:            return "CALL";
        case OpCode::CALL_METHOD:     return "CALL_METHOD";
        case OpCode::RETURN:          return "RETURN";
        case OpCode::BUILD_ARRAY:     return "BUILD_ARRAY";
        case OpCode::BUILD_DICT:      return "BUILD_DICT";
        case OpCode::BUILD_STRING:    return "BUILD_STRING";
        case OpCode::INDEX_GET:       return "INDEX_GET";
        case OpCode::INDEX_SET:       return "INDEX_SET";
        case OpCode::SLICE:           return "SLICE";
        case OpCode::LOAD_FIELD:      return "LOAD_FIELD";
        case OpCode::STORE_FIELD:     return "STORE_FIELD";
        case OpCode::JOSA:            return "JOSA";
        case OpCode::CONVERT:         return "CONVERT";
        case OpCode::CLASS_DEF:       return "CLASS_DEF";
        case OpCode::IMPORT:          return "IMPORT";
        case OpCode::TRY_PUSH:        return "TRY_PUSH";
        case OpCode::TRY_POP:         return "TRY_POP";
        case OpCode::THROW:           return "THROW";
        case OpCode::END_FINALLY:     return "END_FINALLY";
        case OpCode::AWAIT:           return "AWAIT";
        case OpCode::BUILD_PROMISE:   return "BUILD_PROMISE";
        case OpCode::PROMISE_RESOLVE: return "PROMISE_RESOLVE";
        case OpCode::PROMISE_REJECT:  return "PROMISE_REJECT";
        case OpCode::MATCH_PATTERN:   return "MATCH_PATTERN";
        case OpCode::MATCH_FAIL:      return "MATCH_FAIL";
        case OpCode::EVAL_NODE:       return "EVAL_NODE";
        case OpCode::POP:             return "POP";
        case OpCode::DUP:             return "DUP";
        case OpCode::PRINT:           return "PRINT";
        case OpCode::HALT:            return "HALT";
    }
    return "UNKNOWN";
}

const std::vector<uint8_t>& opCodeOperandWidths(OpCode op) {
    static const std::vector<uint8_t> none{};
    static const std::vector<uint8_t> u8{1};
    static const std::vector<uint8_t> u16{2};
    static const std::vector<uint8_t> u16u8{2, 1};
    static const std::vector<uint8_t> u16u16{2, 2};
    static const std::vector<uint8_t> u16u16u8{2, 2, 1};
    static const std::vector<uint8_t> u16u8u8u8{2, 1, 1, 1};

    switch (op) {
        case OpCode::LOAD_CONST:
        case OpCode::DECLARE:
        case OpCode::LOAD_NAME:
        case OpCode::STORE_NAME:
        case OpCode::JUMP:
        case OpCode::JUMP_IF_FALSE:
        case OpCode::JUMP_IF_TRUE:
        case OpCode::LOOP:
        case OpCode::REPEAT_NEXT:
        case OpCode::MAKE_FUNCTION:
        case OpCode::BUILD_ARRAY:
        case OpCode::BUILD_DICT:
        case OpCode::BUILD_STRING:
        case OpCode::LOAD_FIELD:
        case OpCode::STORE_FIELD:
        case OpCode::CONVERT:
        case OpCode::CLASS_DEF:
        case OpCode::MATCH_PATTERN:
        case OpCode::EVAL_NODE:
            return u16;
        case OpCode::CALL:
        case OpCode::SLICE:
            return u8;
        case OpCode::CALL_METHOD:
            return u16u8;
        case OpCode::IMPORT:
        case OpCode::TRY_PUSH:
            return u16u16;
        case OpCode::RANGE_FOR_NEXT:
            return u16u16u8;
        case OpCode::JOSA:
            return u16u8u8u8;
        default:
            return none;
    }
}

int opCodeOperandCount(OpCode op) {
    return static_cast<int>(opCodeOperandWidths(op).size());
}

size_t opCodeOperandBytes(OpCode op) {
    size_t total = 0;
    for (uint8_t width : opCodeOperandWidths(op)) {
        total += width;
    }
    return total;
}

bool isJumpOpCode(OpCode op) {
    return op == OpCode::JUMP ||
           op == OpCode::JUMP_IF_FALSE ||
           op == OpCode::JUMP_IF_TRUE ||
           op == OpCode::LOOP ||
           op == OpCode::REPEAT_NEXT ||
           op == OpCode::RANGE_FOR_NEXT ||
           op == OpCode::TRY_PUSH;
}

} // namespace bytecode
} // namespace kingsejong
