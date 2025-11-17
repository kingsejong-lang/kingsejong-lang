/**
 * @file Node.cpp
 * @brief AST 노드 기본 인터페이스 구현
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include "Node.h"

namespace kingsejong {
namespace ast {

std::string nodeTypeToString(NodeType type)
{
    switch (type)
    {
        // 프로그램
        case NodeType::PROGRAM:
            return "PROGRAM";

        // 문장
        case NodeType::EXPRESSION_STATEMENT:
            return "EXPRESSION_STATEMENT";
        case NodeType::VAR_DECLARATION:
            return "VAR_DECLARATION";
        case NodeType::ASSIGNMENT_STATEMENT:
            return "ASSIGNMENT_STATEMENT";
        case NodeType::RETURN_STATEMENT:
            return "RETURN_STATEMENT";
        case NodeType::IF_STATEMENT:
            return "IF_STATEMENT";
        case NodeType::WHILE_STATEMENT:
            return "WHILE_STATEMENT";
        case NodeType::FOR_STATEMENT:
            return "FOR_STATEMENT";
        case NodeType::REPEAT_STATEMENT:
            return "REPEAT_STATEMENT";
        case NodeType::RANGE_FOR_STATEMENT:
            return "RANGE_FOR_STATEMENT";
        case NodeType::BLOCK_STATEMENT:
            return "BLOCK_STATEMENT";
        case NodeType::IMPORT_STATEMENT:
            return "IMPORT_STATEMENT";
        case NodeType::TRY_STATEMENT:
            return "TRY_STATEMENT";
        case NodeType::THROW_STATEMENT:
            return "THROW_STATEMENT";

        // 표현식
        case NodeType::IDENTIFIER:
            return "IDENTIFIER";
        case NodeType::INTEGER_LITERAL:
            return "INTEGER_LITERAL";
        case NodeType::FLOAT_LITERAL:
            return "FLOAT_LITERAL";
        case NodeType::STRING_LITERAL:
            return "STRING_LITERAL";
        case NodeType::BOOLEAN_LITERAL:
            return "BOOLEAN_LITERAL";

        // 연산 표현식
        case NodeType::BINARY_EXPRESSION:
            return "BINARY_EXPRESSION";
        case NodeType::UNARY_EXPRESSION:
            return "UNARY_EXPRESSION";
        case NodeType::ASSIGNMENT_EXPRESSION:
            return "ASSIGNMENT_EXPRESSION";

        // KingSejong 특화 표현식
        case NodeType::JOSA_EXPRESSION:
            return "JOSA_EXPRESSION";
        case NodeType::RANGE_EXPRESSION:
            return "RANGE_EXPRESSION";

        // 함수 관련
        case NodeType::FUNCTION_LITERAL:
            return "FUNCTION_LITERAL";
        case NodeType::CALL_EXPRESSION:
            return "CALL_EXPRESSION";

        // 배열 관련
        case NodeType::ARRAY_LITERAL:
            return "ARRAY_LITERAL";
        case NodeType::INDEX_EXPRESSION:
            return "INDEX_EXPRESSION";

        // 딕셔너리 관련 (Phase 7.2)
        case NodeType::DICTIONARY_LITERAL:
            return "DICTIONARY_LITERAL";

        // 패턴 매칭
        case NodeType::MATCH_EXPRESSION:
            return "MATCH_EXPRESSION";
        case NodeType::LITERAL_PATTERN:
            return "LITERAL_PATTERN";
        case NodeType::WILDCARD_PATTERN:
            return "WILDCARD_PATTERN";
        case NodeType::BINDING_PATTERN:
            return "BINDING_PATTERN";
        case NodeType::ARRAY_PATTERN:
            return "ARRAY_PATTERN";

        // 클래스 관련 (Phase 7.1)
        case NodeType::CLASS_STATEMENT:
            return "CLASS_STATEMENT";
        case NodeType::FIELD_DECLARATION:
            return "FIELD_DECLARATION";
        case NodeType::METHOD_DECLARATION:
            return "METHOD_DECLARATION";
        case NodeType::CONSTRUCTOR_DECLARATION:
            return "CONSTRUCTOR_DECLARATION";
        case NodeType::MEMBER_ACCESS_EXPRESSION:
            return "MEMBER_ACCESS_EXPRESSION";
        case NodeType::THIS_EXPRESSION:
            return "THIS_EXPRESSION";
        case NodeType::NEW_EXPRESSION:
            return "NEW_EXPRESSION";

        default:
            return "UNKNOWN";
    }
}

} // namespace ast
} // namespace kingsejong
